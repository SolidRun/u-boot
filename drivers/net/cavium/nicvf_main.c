/*
 * Copyright (C) 2014 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <pci.h>
#include <net.h>
#include <misc.h>
#include <netdev.h>
#include <malloc.h>
#include <asm/io.h>

#include <asm/arch/thunderx_vnic.h>

#include "nic_reg.h"
#include "nic.h"
#include "nicvf_queues.h"
#include "thunder_bgx.h"

#define ETH_ALEN 6


/* Register read/write APIs */
void nicvf_reg_write(struct nicvf *nic, uint64_t offset, uint64_t val)
{
	writeq(val, nic->reg_base + offset);
}

uint64_t nicvf_reg_read(struct nicvf *nic, uint64_t offset)
{
	return readq(nic->reg_base + offset);
}

void nicvf_queue_reg_write(struct nicvf *nic, uint64_t offset,
			   uint64_t qidx, uint64_t val)
{
	void *addr = nic->reg_base + offset;

	writeq(val, (void *)(addr + (qidx << NIC_Q_NUM_SHIFT)));
}

uint64_t nicvf_queue_reg_read(struct nicvf *nic, uint64_t offset, uint64_t qidx)
{
	void *addr = nic->reg_base + offset;

	return readq((void *)(addr + (qidx << NIC_Q_NUM_SHIFT)));
}

static void  nicvf_handle_mbx_intr(struct nicvf *nic);

/* VF -> PF mailbox communication */
static void nicvf_write_to_mbx(struct nicvf *nic, union nic_mbx *mbx)
{
	u64 *msg = (u64 *)mbx;

	nicvf_reg_write(nic, NIC_VF_PF_MAILBOX_0_1 + 0, msg[0]);
	nicvf_reg_write(nic, NIC_VF_PF_MAILBOX_0_1 + 8, msg[1]);
}

int nicvf_send_msg_to_pf(struct nicvf *nic, union nic_mbx *mbx)
{
	int timeout = NIC_PF_VF_MBX_TIMEOUT;
	int sleep = 10;

	nic->pf_acked = false;
	nic->pf_nacked = false;

	nicvf_write_to_mbx(nic, mbx);

	nic_handle_mbx_intr(nic->nicpf, nic->vf_id);

	/* Wait for previous message to be acked, timeout 2sec */
	while (!nic->pf_acked) {
		if (nic->pf_nacked)
			return -1;
		mdelay(sleep);
		nicvf_handle_mbx_intr(nic);

		if (nic->pf_acked)
			break;
		timeout -= sleep;
		if (!timeout) {
			printf("PF didn't ack to mbox msg %d from VF%d\n",
			       (mbx->msg.msg & 0xFF), nic->vf_id);
			return -1;
		}
	}

	return 0;
}


/* Checks if VF is able to comminicate with PF
* and also gets the VNIC number this VF is associated to.
*/
static int nicvf_check_pf_ready(struct nicvf *nic)
{
	union nic_mbx mbx = {};

	mbx.msg.msg = NIC_MBOX_MSG_READY;
	if (nicvf_send_msg_to_pf(nic, &mbx)) {
		printf("PF didn't respond to READY msg\n");
		return 0;
	}

	return 1;
}

static void  nicvf_handle_mbx_intr(struct nicvf *nic)
{
	union nic_mbx mbx = {};
	u64 *mbx_data;
	u64 mbx_addr;
	int i;

	mbx_addr = NIC_VF_PF_MAILBOX_0_1;
	mbx_data = (u64 *)&mbx;

	for (i = 0; i < NIC_PF_VF_MAILBOX_SIZE; i++) {
		*mbx_data = nicvf_reg_read(nic, mbx_addr);
		mbx_data++;
		mbx_addr += sizeof(u64);
	}

	debug("Mbox message: msg: 0x%x\n", mbx.msg.msg);
	switch (mbx.msg.msg) {
	case NIC_MBOX_MSG_READY:
		nic->pf_acked = true;
		nic->vf_id = mbx.nic_cfg.vf_id & 0x7F;
		nic->tns_mode = mbx.nic_cfg.tns_mode & 0x7F;
		nic->node = mbx.nic_cfg.node_id;
		if (!nic->set_mac_pending)
			memcpy(nic->netdev->enetaddr,
					mbx.nic_cfg.mac_addr, 6);
		nic->loopback_supported = mbx.nic_cfg.loopback_supported;
		nic->link_up = false;
		nic->duplex = 0;
		nic->speed = 0;
		break;
	case NIC_MBOX_MSG_ACK:
		nic->pf_acked = true;
		break;
	case NIC_MBOX_MSG_NACK:
		nic->pf_nacked = true;
		break;
	case NIC_MBOX_MSG_BGX_LINK_CHANGE:
		nic->pf_acked = true;
		nic->link_up = mbx.link_status.link_up;
		nic->duplex = mbx.link_status.duplex;
		nic->speed = mbx.link_status.speed;
		if (nic->link_up) {
			printf("%s: Link is Up %d Mbps %s\n",
				    nic->netdev->name, nic->speed,
				    nic->duplex == 1 ?
				"Full duplex" : "Half duplex");
		} else {
			printf("%s: Link is Down\n",
				    nic->netdev->name);
		}
		break;
	default:
		printf("Invalid message from PF, msg 0x%x\n", mbx.msg.msg);
		break;
	}

	nicvf_clear_intr(nic, NICVF_INTR_MBOX, 0);
}

static int nicvf_hw_set_mac_addr(struct nicvf *nic, struct eth_device *netdev)
{
	union nic_mbx mbx = {};

	mbx.mac.msg = NIC_MBOX_MSG_SET_MAC;
	mbx.mac.vf_id = nic->vf_id;
	memcpy(mbx.mac.mac_addr, netdev->enetaddr, 6);

	return nicvf_send_msg_to_pf(nic, &mbx);
}

static void nicvf_config_cpi(struct nicvf *nic)
{
	union nic_mbx mbx = {};

	mbx.cpi_cfg.msg = NIC_MBOX_MSG_CPI_CFG;
	mbx.cpi_cfg.vf_id = nic->vf_id;
	mbx.cpi_cfg.cpi_alg = nic->cpi_alg;
	mbx.cpi_cfg.rq_cnt = nic->qs->rq_cnt;

	nicvf_send_msg_to_pf(nic, &mbx);
}


static int nicvf_init_resources(struct nicvf *nic)
{
	int err;

	nic->num_qs = 1;

	/* Enable Qset */
	nicvf_qset_config(nic, true);

	/* Initialize queues and HW for data transfer */
	err = nicvf_config_data_transfer(nic, true);

	if (err) {
		printf("Failed to alloc/config VF's QSet resources\n");
		return err;
	}
	return 0;
}

void nicvf_free_pkt(struct nicvf *nic, void *pkt)
{
	free(pkt);
}

static void nicvf_snd_pkt_handler(struct nicvf *nic,
				  struct cmp_queue *cq,
				  void *cq_desc, int cqe_type)
{
	struct cqe_send_t *cqe_tx;
	struct snd_queue *sq;
	struct sq_hdr_subdesc *hdr;

	cqe_tx = (struct cqe_send_t *)cq_desc;
	sq = &nic->qs->sq[cqe_tx->sq_idx];

	hdr = (struct sq_hdr_subdesc *)GET_SQ_DESC(sq, cqe_tx->sqe_ptr);
	if (hdr->subdesc_type != SQ_DESC_TYPE_HEADER)
		return;

	nicvf_check_cqe_tx_errs(nic, cq, cq_desc);
	nicvf_put_sq_desc(sq, hdr->subdesc_cnt + 1);
}

static int nicvf_rcv_pkt_handler(struct nicvf *nic,
				 struct cmp_queue *cq, void *cq_desc,
				 void **ppkt, int cqe_type)
{
	void *pkt;

	size_t pkt_len;
	struct cqe_rx_t *cqe_rx = (struct cqe_rx_t *)cq_desc;
	int err = 0;

	/* Check for errors */
	err = nicvf_check_cqe_rx_errs(nic, cq, cq_desc);
	if (err && !cqe_rx->rb_cnt)
		return -1;

	pkt = nicvf_get_rcv_pkt(nic, cq_desc, &pkt_len);
	if (!pkt) {
		debug("Packet not received\n");
		return -1;
	}

	if (pkt)
		*ppkt = pkt;

	return pkt_len;
}

int nicvf_cq_handler(struct nicvf *nic, void **ppkt, int *pkt_len)
{
	int cq_qnum = 0;
	int processed_sq_cqe = 0;
	int processed_rq_cqe = 0;
	int processed_cqe = 0;

	unsigned long cqe_count, cqe_head;
	struct queue_set *qs = nic->qs;
	struct cmp_queue *cq = &qs->cq[cq_qnum];
	struct cqe_rx_t *cq_desc;

	/* Get num of valid CQ entries expect next one to be SQ completion */
	cqe_count = nicvf_queue_reg_read(nic, NIC_QSET_CQ_0_7_STATUS, cq_qnum);
	cqe_count &= 0xFFFF;
	if (!cqe_count)
		return 0;

	/* Get head of the valid CQ entries */
	cqe_head = nicvf_queue_reg_read(nic, NIC_QSET_CQ_0_7_HEAD, cq_qnum) >> 9;
	cqe_head &= 0xFFFF;

	if (cqe_count) {
		/* Get the CQ descriptor */
		cq_desc = (struct cqe_rx_t *)GET_CQ_DESC(cq, cqe_head);
		cqe_head++;
		cqe_head &= (cq->dmem.q_len - 1);
		/* Initiate prefetch for next descriptor */
		prefetch((struct cqe_rx_t *)GET_CQ_DESC(cq, cqe_head));

		switch (cq_desc->cqe_type) {
		case CQE_TYPE_RX:
			debug("%s: Got Rx CQE\n", nic->netdev->name);
			*pkt_len = nicvf_rcv_pkt_handler(nic, cq, cq_desc, ppkt, CQE_TYPE_RX);
			processed_rq_cqe++;
			break;
		case CQE_TYPE_SEND:
			debug("%s: Got Tx CQE\n", nic->netdev->name);
			nicvf_snd_pkt_handler(nic, cq, cq_desc, CQE_TYPE_SEND);
			processed_sq_cqe++;
			break;
		default:
			debug("%s: Got CQ type %u\n", nic->netdev->name, cq_desc->cqe_type);
			break;
		}
		processed_cqe++;
	}


	/* Dequeue CQE */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_DOOR,
			      cq_qnum, processed_cqe);

	asm volatile ("dsb sy");

	return (processed_sq_cqe | processed_rq_cqe)  ;
}

/* Qset error interrupt handler
 *
 * As of now only CQ errors are handled
 */
void nicvf_handle_qs_err(struct nicvf *nic)
{
	struct queue_set *qs = nic->qs;
	int qidx;
	uint64_t status;

	/* Check if it is CQ err */
	for (qidx = 0; qidx < qs->cq_cnt; qidx++) {
		status = nicvf_queue_reg_read(nic, NIC_QSET_CQ_0_7_STATUS,
					      qidx);
		if (!(status & CQ_ERR_MASK))
			continue;
		/* Process already queued CQEs and reconfig CQ */
		nicvf_sq_disable(nic, qidx);
		nicvf_cmp_queue_config(nic, qs, qidx, true);
		nicvf_sq_free_used_descs(nic->netdev, &qs->sq[qidx], qidx);
		nicvf_sq_enable(nic, &qs->sq[qidx], qidx);
	}
}

static int nicvf_xmit(struct eth_device *netdev, void *pkt, int pkt_len)
{
	struct nicvf *nic = netdev->priv;
	int ret = 0;
	int rcv_len = 0;
	unsigned int timeout = 5000;
	void *rpkt = NULL;

	if (!nicvf_sq_append_pkt(nic, pkt, pkt_len)) {
		printf("VF%d: TX ring full\n", nic->vf_id);
		return -1;
	}

	/* check and update CQ for pkt sent */
	while (!ret && timeout--) {
		ret = nicvf_cq_handler(nic, &rpkt, &rcv_len);
		if (!ret)
		{
			debug("%s: %d, Not sent\n", __FUNCTION__, __LINE__);
			udelay(10);
		}
	}

	return 0;
}

static int nicvf_recv(struct eth_device *netdev)
{
	struct nicvf *nic = netdev->priv;
	void *pkt;
	int pkt_len = 0;
#ifdef DEBUG
	u8 *dpkt;
	int i, j;
#endif

	nicvf_cq_handler(nic, &pkt, &pkt_len);

	if (pkt_len) {
#ifdef DEBUG
		dpkt = pkt;
		printf("RX packet contents:\n");
		for (i = 0; i < 8; i++) {
			puts("\t");
			for (j = 0; j < 10; j++) {
				printf("%02x ", dpkt[i * 10 + j]);
			}
			puts("\n");
		}
#endif
		net_process_received_packet(pkt, pkt_len);
		nicvf_refill_rbdr(nic);
	}

	return pkt_len;
}

void nicvf_stop(struct eth_device *netdev)
{
	struct nicvf *nic = netdev->priv;

	if (!nic->open)
		return;

	/* Free resources */
	nicvf_config_data_transfer(nic, false);

	/* Disable HW Qset */
	nicvf_qset_config(nic, false);

	nic->open = false;
}

int nicvf_open(struct eth_device *netdev, bd_t *bis)
{
	int err;
	struct nicvf *nic = netdev->priv;

	nicvf_hw_set_mac_addr(nic, netdev);

	/* Configure CPI alorithm */
	nic->cpi_alg = CPI_ALG_NONE;
	nicvf_config_cpi(nic);

	/* Initialize the queues */
	err = nicvf_init_resources(nic);
	if (err)
		return -1;

	if (!nicvf_check_pf_ready(nic)) {
		return -1;
	}

	nic->open = true;

	/* Make sure queue initialization is written */
	asm volatile("dsb sy");

	return 0;
}

int nicvf_initialize(struct udevice *pdev, int vf_num)
{
	struct eth_device *netdev = NULL;
	struct nicvf *nicvf = NULL;
	int    ret;
	size_t size;

	netdev = calloc(1, sizeof(struct eth_device));

	if (!netdev) {
		ret = -ENOMEM;
		goto fail;
	}

	nicvf = calloc(1, sizeof(struct nicvf));

	if (!nicvf) {
		ret = -ENOMEM;
		goto fail;
	}

	netdev->priv = nicvf;
	nicvf->netdev = netdev;
	nicvf->nicpf = dev_get_priv(pdev);
	nicvf->vf_id = vf_num;


	/* Enable TSO support */
	nicvf->hw_tso = true;

	nicvf->reg_base = dm_pci_map_bar(pdev, 9, &size, PCI_REGION_MEM);

	nicvf->reg_base += size * nicvf->vf_id;

	debug("nicvf->reg_base: %p\n", nicvf->reg_base);

	if (!nicvf->reg_base) {
		printf("Cannot map config register space, aborting\n");
		ret = -1;
		goto fail;
	}

	ret = nicvf_set_qset_resources(nicvf);
	if (ret)
		return -1;

	snprintf(netdev->name, sizeof(netdev->name), "vnic%u", nicvf->vf_id);

	netdev->halt = nicvf_stop;
	netdev->init = nicvf_open;
	netdev->send = nicvf_xmit;
	netdev->recv = nicvf_recv;

	if (!eth_getenv_enetaddr_by_index("eth", nicvf->vf_id, netdev->enetaddr)) {
		eth_getenv_enetaddr("ethaddr", netdev->enetaddr);
		netdev->enetaddr[5] += nicvf->vf_id;
	}

	ret = eth_register(netdev);

	if (!ret)
		return 0;

	printf("Failed to register netdevice\n");

fail:
	if (nicvf)
		free(nicvf);
	if(netdev)
		free(netdev);
	return ret;
}

int thunderx_vnic_probe(struct udevice *dev)
{
	void *regs;
	size_t size;

	regs = dm_pci_map_bar(dev, 9, &size, PCI_REGION_MEM);

	debug("%s: %d, regs: %p\n", __FUNCTION__, __LINE__, regs);

	return 0;
}

static const struct misc_ops thunderx_vnic_ops = {
};

static const struct udevice_id thunderx_vnic_ids[] = {
	{ .compatible = "cavium,vnic" },
	{}
};

U_BOOT_DRIVER(thunderx_vnic) = {
	.name	= "thunderx_vnic",
	.id	= UCLASS_MISC,
	.probe	= thunderx_vnic_probe,
	.of_match = thunderx_vnic_ids,
	.ops	= &thunderx_vnic_ops,
	.priv_auto_alloc_size = sizeof(struct nicvf),
};

static struct pci_device_id thunderx_vnic_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_THUNDER_NIC_VF_1) },
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_THUNDER_NIC_VF_2) },
	{}
};

U_BOOT_PCI_DEVICE(thunderx_vnic, thunderx_vnic_supported);
