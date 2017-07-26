/*
 * Copyright (C) 2017 NXP Semiconductors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <linux/compat.h>
#include <dm.h>
#include <dm/device.h>
#include <pci.h>
#include <memalign.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <console.h>
#include <asm/armv8/mmu.h>
#include "nvme.h"

#define NVME_Q_DEPTH		2
#define NVME_AQ_DEPTH		2
#define SQ_SIZE(depth)		(depth * sizeof(struct nvme_command))
#define CQ_SIZE(depth)		(depth * sizeof(struct nvme_completion))
#define ADMIN_TIMEOUT		60
#define MAX_PRP_POOL		512

struct nvme_info *nvme_info;

/*
 *An NVM Express queue. Each device has at least two(one for admin
 *commands and one for I/O commands).
 */
struct nvme_queue {
	struct nvme_dev *dev;
	struct nvme_command *sq_cmds;
	struct nvme_completion *cqes;
	wait_queue_head_t sq_full;
	u32 __iomem *q_db;
	u16 q_depth;
	s16 cq_vector;
	u16 sq_head;
	u16 sq_tail;
	u16 cq_head;
	u16 qid;
	u8 cq_phase;
	u8 cqe_seen;
	unsigned long cmdid_data[];
};

static int nvme_wait_ready(struct nvme_dev *dev, u64 cap, bool enabled)
{
	u32 bit = enabled ? NVME_CSTS_RDY : 0;
	while ((readl(&dev->bar->csts) & NVME_CSTS_RDY) != bit)
		udelay(10000);

	return 0;
}

static int nvme_setup_prps(struct nvme_dev *dev, u64 *prp2,
		int total_len, u64 dma_addr)
{
	u32 page_size = dev->page_size;
	int offset = dma_addr & (page_size - 1);
	u64 *prp_pool;
	int length = total_len;
	int i, nprps;
	length -= (page_size - offset);

	if (length <= 0) {
		*prp2 = 0;
		return 0;
	}

	if (length)
		dma_addr += (page_size - offset);

	if (length <= page_size) {
		*prp2 = dma_addr;
		return 0;
	}

	nprps = DIV_ROUND_UP(length, page_size);

	if (nprps > dev->prp_entry_num) {
		free(dev->prp_pool);
		dev->prp_pool = malloc(nprps << 3);
		if (!dev->prp_pool) {
			printf("Error: malloc prp_pool fail\n");
			return -ENOMEM;
		}
		dev->prp_entry_num = nprps;
	}
	prp_pool = dev->prp_pool;
	i = 0;
	while (nprps) {
		if (i == ((page_size >> 3) - 1)) {
			*(prp_pool + i) = cpu_to_le64((u64)prp_pool +
					page_size);
			i = 0;
			prp_pool += page_size;
		}
		*(prp_pool + i++) = cpu_to_le64(dma_addr);
		dma_addr += page_size;
		nprps--;
	}
	*prp2 = (u64)dev->prp_pool;
	return 0;
}

static __le16 get_cmdid(void)
{
	static unsigned short cmdid;
	return cpu_to_le16((cmdid < USHRT_MAX) ? cmdid++ : 0);
}

static u16 read_completion_status(struct nvme_queue *nvmeq, u16 index)
{
	u64 start = (u64)&nvmeq->cqes[index];
	u64 stop = start + sizeof(struct nvme_completion);
	invalidate_dcache_range(start, stop);
	return le16_to_cpu(readw(&(nvmeq->cqes[index].status)));
}

/**
 * nvme_submit_cmd() - Copy a command into a queue and ring the doorbell
 * @nvmeq: The queue to use
 * @cmd: The command to send
 */
static int nvme_submit_cmd(struct nvme_queue *nvmeq, struct nvme_command *cmd)
{
	u16 tail = nvmeq->sq_tail;
	flush_dcache_all();
	memcpy(&nvmeq->sq_cmds[tail], cmd, sizeof(*cmd));
	flush_dcache_range((ulong)&nvmeq->sq_cmds[tail],
			   (ulong)&nvmeq->sq_cmds[tail] + sizeof(*cmd));
	if (++tail == nvmeq->q_depth)
		tail = 0;
	writel(tail, nvmeq->q_db);
	nvmeq->sq_tail = tail;

	return 0;
}

static int nvme_submit_sync_cmd(struct nvme_queue *nvmeq,
		struct nvme_command *cmd, u32 *result, unsigned timeout)
{
	u16 head = nvmeq->cq_head;
	u16 phase = nvmeq->cq_phase;
	u16 status;
	ulong start_time;
	ulong timeout_us = timeout * 100000;

	cmd->common.command_id = get_cmdid();
	nvme_submit_cmd(nvmeq, cmd);

	start_time = timer_get_us();

	for (;;) {
		status = read_completion_status(nvmeq, head);
		if ((status & 0x01) == phase)
			break;
		if (timeout_us > 0 && (timer_get_us() - start_time)
		    >= timeout_us)
			return -ETIMEDOUT;
	}
	status >>= 1;
	if (status) {
		printf("ERROR: status = %d, phase = %d, head = %d\n",
		       status, phase, head);
		status = 0;
		if (++head == nvmeq->q_depth) {
			head = 0;
			phase = !phase;
		}
		writel(head, nvmeq->q_db + nvmeq->dev->db_stride);
		nvmeq->cq_head = head;
		nvmeq->cq_phase = phase;
		return -1;
	}

	if (result)
		*result = le32_to_cpu(readl(&(nvmeq->cqes[head].result)));

	if (++head == nvmeq->q_depth) {
		head = 0;
		phase = !phase;
	}
	writel(head, nvmeq->q_db + nvmeq->dev->db_stride);
	nvmeq->cq_head = head;
	nvmeq->cq_phase = phase;

	return status;
}

static int nvme_submit_admin_cmd(struct nvme_dev *dev,
		struct nvme_command *cmd, u32 *result)
{
	return nvme_submit_sync_cmd(dev->queues[0], cmd, result, ADMIN_TIMEOUT);
}

static struct nvme_queue *nvme_alloc_queue(struct nvme_dev *dev,
		int qid, int depth)
{
	struct nvme_queue *nvmeq = malloc(sizeof(*nvmeq));
	if (!nvmeq)
		return NULL;
	memset(nvmeq, 0, sizeof(*nvmeq));

	nvmeq->cqes = (void *)memalign(4096, CQ_SIZE(depth));
	if (!nvmeq->cqes)
		goto free_nvmeq;
	memset((void *)nvmeq->cqes, 0, CQ_SIZE(depth));

	nvmeq->sq_cmds = (void *)memalign(4096, SQ_SIZE(depth));
	if (!nvmeq->sq_cmds)
		goto free_queue;
	memset((void *)nvmeq->sq_cmds, 0, SQ_SIZE(depth));

	nvmeq->dev = dev;

	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	nvmeq->q_depth = depth;
	nvmeq->qid = qid;
	dev->queue_count++;
	dev->queues[qid] = nvmeq;

	return nvmeq;
 free_queue:
	free((void *)nvmeq->cqes);
 free_nvmeq:
	free(nvmeq);
	return NULL;
}

static int adapter_delete_queue(struct nvme_dev *dev, u8 opcode, u16 id)
{
	struct nvme_command c;
	memset(&c, 0, sizeof(c));
	c.delete_queue.opcode = opcode;
	c.delete_queue.qid = cpu_to_le16(id);
	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int adapter_delete_sq(struct nvme_dev *dev, u16 sqid)
{
	return adapter_delete_queue(dev, nvme_admin_delete_sq, sqid);
}

static int adapter_delete_cq(struct nvme_dev *dev, u16 cqid)
{
	return adapter_delete_queue(dev, nvme_admin_delete_cq, cqid);
}

static int nvme_enable_ctrl(struct nvme_dev *dev, u64 cap)
{
	dev->ctrl_config &= ~NVME_CC_SHN_MASK;
	dev->ctrl_config |= NVME_CC_ENABLE;
	writel(cpu_to_le32(dev->ctrl_config), &dev->bar->cc);
	return nvme_wait_ready(dev, cap, true);
}

static int nvme_disable_ctrl(struct nvme_dev *dev, u64 cap)
{
	dev->ctrl_config &= ~NVME_CC_SHN_MASK;
	dev->ctrl_config &= ~NVME_CC_ENABLE;
	writel(cpu_to_le32(dev->ctrl_config), &dev->bar->cc);
	return nvme_wait_ready(dev, cap, false);
}

static void nvme_free_queue(struct nvme_queue *nvmeq)
{
	free((void *)nvmeq->cqes);
	free(nvmeq->sq_cmds);
	free(nvmeq);
}

static void nvme_free_queues(struct nvme_dev *dev, int lowest)
{
	int i;
	for (i = dev->queue_count - 1; i >= lowest; i--) {
		struct nvme_queue *nvmeq = dev->queues[i];
		dev->queue_count--;
		dev->queues[i] = NULL;
		nvme_free_queue(nvmeq);
	}
}

static void nvme_init_queue(struct nvme_queue *nvmeq, u16 qid)
{
	struct nvme_dev *dev = nvmeq->dev;
	nvmeq->sq_tail = 0;
	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	memset((void *)nvmeq->cqes, 0, CQ_SIZE(nvmeq->q_depth));
	flush_dcache_range((u64)nvmeq->cqes,
			   (u64)nvmeq->cqes + CQ_SIZE(nvmeq->q_depth));
	dev->online_queues++;
}

static int nvme_configure_admin_queue(struct nvme_dev *dev)
{
	int result;
	u32 aqa;
	u64 cap = readq(&dev->bar->cap);
	struct nvme_queue *nvmeq;
	unsigned page_shift = PAGE_SHIFT;
	unsigned dev_page_min = NVME_CAP_MPSMIN(cap) + 12;
	unsigned dev_page_max = NVME_CAP_MPSMAX(cap) + 12;
	if (page_shift < dev_page_min) {
		dev_err(&dev->pci_dev->dev,
			"Minimum device page size (%u) too large for host (%u)\n",
			1 << dev_page_min,
			1 << page_shift);
		return -ENODEV;
	}

	if (page_shift > dev_page_max) {
		dev_info(&dev->pci_dev->dev,
			 "Device maximum page size (%u) smaller than host (%u); enabling work-around\n",
			 1 << dev_page_max,
			 1 << page_shift);
		page_shift = dev_page_max;
	}

	result = nvme_disable_ctrl(dev, cap);
	if (result < 0)
		return result;

	nvmeq = dev->queues[0];
	if (!nvmeq) {
		nvmeq = nvme_alloc_queue(dev, 0, NVME_AQ_DEPTH);
		if (!nvmeq)
			return -ENOMEM;
	}

	aqa = nvmeq->q_depth - 1;
	aqa |= aqa << 16;
	aqa |= aqa << 16;

	dev->page_size = 1 << page_shift;

	dev->ctrl_config = NVME_CC_CSS_NVM;
	dev->ctrl_config |= (page_shift - 12) << NVME_CC_MPS_SHIFT;
	dev->ctrl_config |= NVME_CC_ARB_RR | NVME_CC_SHN_NONE;
	dev->ctrl_config |= NVME_CC_IOSQES | NVME_CC_IOCQES;

	writel(aqa, &dev->bar->aqa);
	writeq((u64)nvmeq->sq_cmds, &dev->bar->asq);
	writeq((u64)nvmeq->cqes, &dev->bar->acq);

	result = nvme_enable_ctrl(dev, cap);
	if (result)
		goto free_nvmeq;

	nvmeq->cq_vector = 0;

	nvme_init_queue(dev->queues[0], 0);
	return result;

 free_nvmeq:
	nvme_free_queues(dev, 0);
	return result;
}

static int adapter_alloc_cq(struct nvme_dev *dev,
		u16 qid, struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_CQ_IRQ_ENABLED;
	memset(&c, 0, sizeof(c));
	c.create_cq.opcode = nvme_admin_create_cq;
	c.create_cq.prp1 = cpu_to_le64(nvmeq->cqes);
	c.create_cq.cqid = cpu_to_le16(qid);
	c.create_cq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_cq.cq_flags = cpu_to_le16(flags);
	c.create_cq.irq_vector = cpu_to_le16(nvmeq->cq_vector);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int adapter_alloc_sq(struct nvme_dev *dev,
		u16 qid, struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_SQ_PRIO_MEDIUM;
	memset(&c, 0, sizeof(c));
	c.create_sq.opcode = nvme_admin_create_sq;
	c.create_sq.prp1 = cpu_to_le64(nvmeq->sq_cmds);
	c.create_sq.sqid = cpu_to_le16(qid);
	c.create_sq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_sq.sq_flags = cpu_to_le16(flags);
	c.create_sq.cqid = cpu_to_le16(qid);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int nvme_identify(struct nvme_dev *dev, unsigned nsid,
		unsigned cns, dma_addr_t dma_addr)
{
	struct nvme_command c;
	memset(&c, 0, sizeof(c));
	u32 page_size = dev->page_size;
	int offset = dma_addr & (page_size - 1);
	int length = sizeof(struct nvme_id_ctrl);

	c.identify.opcode = nvme_admin_identify;
	c.identify.nsid = cpu_to_le32(nsid);
	c.identify.prp1 = cpu_to_le64(dma_addr);

	length -= (page_size - offset);
	if (length <= 0) {
		c.identify.prp2 = 0;
	} else {
		dma_addr += (page_size - offset);
		c.identify.prp2 = dma_addr;
	}

	c.identify.cns = cpu_to_le32(cns);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static __maybe_unused int nvme_get_features(struct nvme_dev *dev, unsigned fid,
		unsigned nsid, dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;
	memset(&c, 0, sizeof(c));

	c.features.opcode = nvme_admin_get_features;
	c.features.nsid = cpu_to_le32(nsid);
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);

	return nvme_submit_admin_cmd(dev, &c, result);
}

static __maybe_unused int nvme_set_features(struct nvme_dev *dev, unsigned fid,
		unsigned dword11, dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;
	memset(&c, 0, sizeof(c));

	c.features.opcode = nvme_admin_set_features;
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);
	c.features.dword11 = cpu_to_le32(dword11);

	return nvme_submit_admin_cmd(dev, &c, result);
}

static int nvme_create_queue(struct nvme_queue *nvmeq, int qid)
{
	struct nvme_dev *dev = nvmeq->dev;
	int result;

	nvmeq->cq_vector = qid - 1;
	result = adapter_alloc_cq(dev, qid, nvmeq);
	if (result < 0)
		goto release_cq;

	result = adapter_alloc_sq(dev, qid, nvmeq);
	if (result < 0)
		goto release_sq;

	nvme_init_queue(nvmeq, qid);
	return result;

 release_sq:
	adapter_delete_sq(dev, qid);
 release_cq:
	adapter_delete_cq(dev, qid);
	return result;
}

static int set_queue_count(struct nvme_dev *dev, int count)
{
	int status;
	u32 result;
	u32 q_count = (count - 1) | ((count - 1) << 16);

	status = nvme_set_features(dev, NVME_FEAT_NUM_QUEUES,
			q_count, 0, &result);

	if (status < 0)
		return status;
	if (status > 1)
		return 0;

	return min(result & 0xffff, result >> 16) + 1;
}

static void nvme_create_io_queues(struct nvme_dev *dev)
{
	unsigned int i;

	for (i = dev->queue_count; i <= dev->max_qid; i++)
		if (!nvme_alloc_queue(dev, i, dev->q_depth))
			break;

	for (i = dev->online_queues; i <= dev->queue_count - 1; i++)
		if (nvme_create_queue(dev->queues[i], i))
			break;
}

static int nvme_setup_io_queues(struct nvme_dev *dev)
{
	int nr_io_queues;
	int result;

	nr_io_queues = 1;
	result = set_queue_count(dev, nr_io_queues);
	if (result <= 0)
		return result;

	if (result < nr_io_queues)
		nr_io_queues = result;

	dev->max_qid = nr_io_queues;

	/* Free previously allocated queues*/
	nvme_free_queues(dev, nr_io_queues + 1);
	nvme_create_io_queues(dev);

	return 0;
}

static int nvme_get_info_from_identify(struct nvme_dev *dev)
{
	u16 vendor, device;
	struct nvme_id_ctrl buf, *ctrl = &buf;
	int ret;
	int shift = NVME_CAP_MPSMIN(readq(&dev->bar->cap)) + 12;

	ret = nvme_identify(dev, 0, 1, (dma_addr_t)ctrl);
	if (ret)
		return -EIO;

	dev->nn = le32_to_cpu(ctrl->nn);
	dev->vwc = ctrl->vwc;
	memcpy(dev->serial, ctrl->sn, sizeof(ctrl->sn));
	memcpy(dev->model, ctrl->mn, sizeof(ctrl->mn));
	memcpy(dev->firmware_rev, ctrl->fr, sizeof(ctrl->fr));
	if (ctrl->mdts)
		dev->max_transfer_shift = (ctrl->mdts + shift);

	dm_pci_read_config16(dev->pdev, PCI_VENDOR_ID, &vendor);
	dm_pci_read_config16(dev->pdev, PCI_DEVICE_ID, &device);
	if ((vendor == PCI_VENDOR_ID_INTEL) &&
	    (device == 0x0953) && ctrl->vs[3]) {
		unsigned int max_transfer_shift;
		dev->stripe_size = (ctrl->vs[3] + shift);
		max_transfer_shift = (ctrl->vs[3] + 18);
		if (dev->max_transfer_shift) {
			dev->max_transfer_shift = min(max_transfer_shift,
						       dev->max_transfer_shift);
		} else {
			dev->max_transfer_shift = max_transfer_shift;
		}
	}
	return 0;
}

int init_nvme(struct udevice *udev)
{
	int ret;
	struct nvme_dev *ndev = dev_get_priv(udev);
	u32 val;
	u64 cap;
	size_t size;

	ndev->pdev = udev;

	ndev->instance = trailing_strtol(udev->name);

	INIT_LIST_HEAD(&ndev->namespaces);
	ndev->bar = dm_pci_map_bar(udev, 0, &size, PCI_REGION_MEM);
	if (readl(&ndev->bar->csts) == -1) {
		ret = -ENODEV;
		printf("Error: %s: Out of Memory!\n", udev->name);
		goto free_nvme;
	}

	ndev->queues = malloc(2 * sizeof(struct nvme_queue));
	if (!ndev->queues) {
		ret = -ENOMEM;
		printf("Error: %s: Out of Memory!\n", udev->name);
		goto free_nvme;
	}
	memset(ndev->queues, 0, sizeof(2 * sizeof(struct nvme_queue)));

	ndev->prp_pool = malloc(MAX_PRP_POOL);
	if (!ndev->prp_pool) {
		ret = -ENOMEM;
		printf("Error: %s: Out of Memory!\n", udev->name);
		goto free_nvme;
	}
	ndev->prp_entry_num = MAX_PRP_POOL >> 3;

	/* Try to enable I/O accesses and bus-mastering */
	val = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	dm_pci_write_config32(udev, PCI_COMMAND, val);

	/* Print a debug message with the IO base address */
	dm_pci_read_config32(udev, PCI_BASE_ADDRESS_0, &val);

	/* Make sure it worked */
	dm_pci_read_config32(udev, PCI_COMMAND, &val);
	if (!(val & PCI_COMMAND_MEMORY)) {
		printf("Can't enable I/O memory\n");
		ret = -ENOSPC;
		goto free_queue;
	}

	if (!(val & PCI_COMMAND_MASTER)) {
		printf("Can't enable bus-mastering\n");
		ret = -EPERM;
		goto free_queue;
	}

	if (readl(&ndev->bar->csts) == -1) {
		ret = -ENODEV;
		goto free_queue;
	}

	cap = readq(&ndev->bar->cap);
	ndev->q_depth = min_t(int, NVME_CAP_MQES(cap) + 1, NVME_Q_DEPTH);
	ndev->db_stride = 1 << NVME_CAP_STRIDE(cap);
	ndev->dbs = ((void __iomem *)ndev->bar) + 4096;

	ret = nvme_configure_admin_queue(ndev);
	if (ret)
		goto free_queue;

	ret = nvme_setup_io_queues(ndev);
	if (ret)
		goto free_queue;

	nvme_get_info_from_identify(ndev);
	ndev->blk_dev_start = nvme_info->ns_num;
	list_add(&ndev->node, &nvme_info->dev_list);
	return 0;
 free_queue:
	free((void *)ndev->queues);
 free_nvme:
	return ret;
}

static void print_optional_admin_cmd(u16 oacs, int devnum)
{
	printf("Blk device %d: Optional Admin Command Support:\n",
	       devnum);
	printf("\tNamespace Management/Attachment: %s\n",
	       oacs & 0x08 ? "yes" : "no");
	printf("\tFirmware Commit/Image download: %s\n",
	       oacs & 0x04 ? "yes" : "no");
	printf("\tFormat NVM: %s\n",
	       oacs & 0x02 ? "yes" : "no");
	printf("\tSecurity Send/Receive: %s\n",
	       oacs & 0x01 ? "yes" : "no");
}

static void print_optional_nvm_cmd(u16 oncs, int devnum)
{
	printf("Blk device %d: Optional NVM Command Support:\n",
	       devnum);
	printf("\tReservation: %s\n",
	       oncs & 0x10 ? "yes" : "no");
	printf("\tSave/Select field in the Set/Get features: %s\n",
	       oncs & 0x08 ? "yes" : "no");
	printf("\tWrite Zeroes: %s\n",
	       oncs & 0x04 ? "yes" : "no");
	printf("\tDataset Management: %s\n",
	       oncs & 0x02 ? "yes" : "no");
	printf("\tWrite Uncorrectable: %s\n",
	       oncs & 0x01 ? "yes" : "no");
}

static void print_format_nvme_attributes(u8 fna, int devnum)
{
	printf("Blk device %d: Format NVM Attributes:\n", devnum);
	printf("\tSupport Cryptographic Erase: %s\n",
	       fna & 0x04 ? "yes" : "No");
	printf("\tSupport erase a particular namespace: %s\n",
	       fna & 0x02 ? "No" : "Yes");
	printf("\tSupport format a particular namespace: %s\n",
	       fna & 0x01 ? "No" : "Yes");
}

static void print_format(struct nvme_lbaf *lbaf)
{
	u8 str[][10] = {"Best", "Better", "Good", "Degraded"};
	printf("\t\tMetadata Size: %d\n", le16_to_cpu(lbaf->ms));
	printf("\t\tLBA Data Size: %d\n", 1 << lbaf->ds);
	printf("\t\tRelative Performance: %s\n", str[lbaf->rp & 0x03]);
}

static void print_formats(struct nvme_id_ns *id, struct nvme_ns *ns)
{
	int i;
	printf("Blk device %d: LBA Format Support:\n", ns->devnum);
	for (i = 0; i < id->nlbaf; i++) {
		printf("\tLBA Foramt %d Support: ", i);
		if (i == ns->flbas)
			printf("(current)\n");
		else
			printf("\n");
		print_format(id->lbaf + i);
	}
}

static void print_data_protect_cap(u8 dpc, int devnum)
{
	printf("Blk device %d: End-to-End Data", devnum);
	printf("Protect Capabilities:\n");
	printf("\tAs last eight bytes: %s\n",
	       dpc & 0x10 ? "yes" : "No");
	printf("\tAs first eight bytes: %s\n",
	       dpc & 0x08 ? "yes" : "No");
	printf("\tSupport Type3: %s\n",
	       dpc & 0x04 ? "yes" : "No");
	printf("\tSupport Type2: %s\n",
	       dpc & 0x02 ? "yes" : "No");
	printf("\tSupport Type1: %s\n",
	       dpc & 0x01 ? "yes" : "No");
}

static void print_metadata_cap(u8 mc, int devnum)
{
	printf("Blk device %d: Metadata capabilities:\n", devnum);
	printf("\tAs part of a separate buffer: %s\n",
	       mc & 0x02 ? "yes" : "No");
	printf("\tAs part of an extended data LBA: %s\n",
	       mc & 0x01 ? "yes" : "No");
}

int nvme_print_info(struct udevice *udev)
{
	struct nvme_ns *ns = dev_get_priv(udev);
	struct nvme_dev *dev = ns->dev;
	struct nvme_id_ns buf_ns, *id = &buf_ns;
	struct nvme_id_ctrl buf_ctrl, *ctrl = &buf_ctrl;

	if (nvme_identify(dev, 0, 1, (dma_addr_t)ctrl))
		return -EIO;

	print_optional_admin_cmd(le16_to_cpu(ctrl->oacs), ns->devnum);
	print_optional_nvm_cmd(le16_to_cpu(ctrl->oncs), ns->devnum);
	print_format_nvme_attributes(ctrl->fna, ns->devnum);

	if (nvme_identify(dev, ns->ns_id, 0, (dma_addr_t)id))
		return -EIO;

	print_formats(id, ns);
	print_data_protect_cap(id->dpc, ns->devnum);
	print_metadata_cap(id->mc, ns->devnum);
	return 0;
}

static ulong nvme_write(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer)
{
	struct nvme_ns *ns = dev_get_priv(udev);
	struct nvme_dev *dev = ns->dev;
	struct nvme_command c;
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	int status;
	u64 prp2;
	u64 total_len = blkcnt << desc->log2blksz;
	u64 temp_len = total_len;

	u64 slba = blknr;
	u16 lbas = 1 << (dev->max_transfer_shift - ns->lba_shift);
	u64 total_lbas = blkcnt;

	c.rw.opcode = nvme_cmd_write;
	c.rw.flags = 0;
	c.rw.nsid = cpu_to_le32(ns->ns_id);
	c.rw.control = 0;
	c.rw.dsmgmt = 0;
	c.rw.reftag = 0;
	c.rw.apptag = 0;
	c.rw.appmask = 0;
	c.rw.metadata = 0;

	while (total_lbas) {
		if (total_lbas < lbas) {
			lbas = (u16)total_lbas;
			total_lbas = 0;
		} else {
			total_lbas -= lbas;
		}

		if (nvme_setup_prps
		   (dev, &prp2, lbas << ns->lba_shift, (u64)buffer))
			return -EIO;
		c.rw.slba = cpu_to_le64(slba);
		slba += lbas;
		c.rw.length = cpu_to_le16(lbas - 1);
		c.rw.prp1 = cpu_to_le64(buffer);
		c.rw.prp2 = cpu_to_le64(prp2);
		status = nvme_submit_sync_cmd(dev->queues[1],
				&c, NULL, NVME_IO_TIMEOUT);
		if (status)
			break;
		temp_len -= lbas << ns->lba_shift;
		buffer += lbas << ns->lba_shift;
	}
	return (total_len - temp_len) >> desc->log2blksz;
}

static ulong nvme_read(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
		void *buffer)
{
	struct nvme_ns *ns = dev_get_priv(udev);
	struct nvme_dev *dev = ns->dev;
	struct nvme_command c;
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	int status;
	u64 prp2;
	u64 total_len = blkcnt << desc->log2blksz;
	u64 temp_len = total_len;

	u64 slba = blknr;
	u16 lbas = 1 << (dev->max_transfer_shift - ns->lba_shift);
	u64 total_lbas = blkcnt;

	c.rw.opcode = nvme_cmd_read;
	c.rw.flags = 0;
	c.rw.nsid = cpu_to_le32(ns->ns_id);
	c.rw.control = 0;
	c.rw.dsmgmt = 0;
	c.rw.reftag = 0;
	c.rw.apptag = 0;
	c.rw.appmask = 0;
	c.rw.metadata = 0;

	while (total_lbas) {
		if (total_lbas < lbas) {
			lbas = (u16)total_lbas;
			total_lbas = 0;
		} else {
			total_lbas -= lbas;
		}

		if (nvme_setup_prps
		   (dev, &prp2, lbas << ns->lba_shift, (u64)buffer))
			return -EIO;
		c.rw.slba = cpu_to_le64(slba);
		slba += lbas;
		c.rw.length = cpu_to_le16(lbas - 1);
		c.rw.prp1 = cpu_to_le64(buffer);
		c.rw.prp2 = cpu_to_le64(prp2);
		status = nvme_submit_sync_cmd(dev->queues[1],
				&c, NULL, NVME_IO_TIMEOUT);
		if (status)
			break;
		temp_len -= lbas << ns->lba_shift;
		buffer += lbas << ns->lba_shift;
	}
	return (total_len - temp_len) >> desc->log2blksz;
}

struct pci_device_id nvme_supported[] = {
		{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x0953) },
		{}
};

int __nvme_initialize(void)
{
	int i, ret = 0;
	struct udevice *pdev;
	struct udevice *ndev;
	struct udevice *nsdev;

	for (i = 0; ; i++) {
		ret = pci_find_device_id(nvme_supported, i, &pdev);
		if (ret)
			break;
		for (uclass_first_device(UCLASS_NVME, &ndev);
		     ndev;
		     uclass_next_device(&ndev)) {
			for (device_find_first_child(ndev, &nsdev);
			     nsdev;
			     device_find_next_child(&nsdev)) {
				printf(" %s ", nsdev->name);
			}
		}
	}
	printf("\n");

	return ret;
}

int nvme_initialize(void) __attribute__((weak, alias("__nvme_initialize")));

static int nvme_blk_probe(struct udevice *udev)
{
	struct nvme_dev *ndev = dev_get_priv(udev->parent);
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	struct nvme_ns *ns = dev_get_priv(udev);
	u8 flbas;
	u16 vendor;
	struct nvme_id_ns buf, *id = &buf;

	memset(ns, 0, sizeof(*ns));
	ns->dev = ndev;
	ns->ns_id = desc->devnum - ndev->blk_dev_start + 1;
	if (nvme_identify(ndev, ns->ns_id, 0, (dma_addr_t)id))
		return -EIO;

	flbas = id->flbas & NVME_NS_FLBAS_LBA_MASK;
	ns->flbas = flbas;
	ns->lba_shift = id->lbaf[flbas].ds;
	ns->mode_select_num_blocks = le64_to_cpu(id->nuse);
	ns->mode_select_block_len = 1 << ns->lba_shift;
	list_add(&ns->list, &ndev->namespaces);

	desc->lba = ns->mode_select_num_blocks;
	desc->log2blksz = ns->lba_shift;
	desc->blksz = 1 << ns->lba_shift;
	desc->bdev = udev;
	dm_pci_read_config16(ndev->pdev, PCI_VENDOR_ID, &vendor);
	memcpy(desc->product, ndev->serial, sizeof(ndev->serial));
	sprintf(desc->vendor, "0x%.4x", vendor);
	memcpy(desc->revision, ndev->firmware_rev, sizeof(ndev->firmware_rev));
	part_init(desc);
	return 0;
}

static const struct blk_ops nvme_blk_ops = {
	.read	= nvme_read,
	.write	= nvme_write,
};

static int nvme_bind(struct udevice *udev)
{
	char name[20];
	sprintf(name, "nvme#%d", nvme_info->ndev_num++);
	return device_set_name(udev, name);
}

int nvme_probe(struct udevice *udev)
{
	return init_nvme(udev);
}

U_BOOT_DRIVER(nvme_blk) = {
	.name			= "nvme-blk",
	.id			= UCLASS_BLK,
	.ops			= &nvme_blk_ops,
	.probe			= nvme_blk_probe,
	.priv_auto_alloc_size	= sizeof(struct nvme_ns),
};

U_BOOT_DRIVER(nvme) = {
	.name		= "nvme",
	.id		= UCLASS_NVME,
	.bind		= nvme_bind,
	.probe		= nvme_probe,
	.priv_auto_alloc_size	= sizeof(struct nvme_dev),
};

U_BOOT_PCI_DEVICE(nvme, nvme_supported);
