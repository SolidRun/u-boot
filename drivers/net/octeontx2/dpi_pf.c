// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2022 Marvell.
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>
#include <pci_ids.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/arch/board.h>
#include <linux/delay.h>

#include "dpi_pf.h"
#include "npa_pf.h"

u64 read_bar4_reg(u64 offset)
{
	return readq(BAR4_DRAM_OFFSET + offset);
}

void write_bar4_reg(u64 val, u64 offset)
{
	writeq(val, (void *)(BAR4_DRAM_OFFSET + offset));
}

u64 read_dpi_vf_reg(struct dpi_pf *pf, u64 offset)
{
	return readq(pf->vf_base + offset);
}

void write_dpi_vf_reg(struct dpi_pf *pf, u64 offset, u64 val)
{
	writeq(val, pf->vf_base + offset);
}

u64 read_dpi_reg(struct dpi_pf *pf, u64 offset)
{
	return readq(pf->pf_base + offset);
}

void write_dpi_reg(struct dpi_pf *pf, u64 offset, u64 val)
{
	writeq(val, pf->pf_base + offset);
}

void mb_send_msg(struct dpi_pf *pf)
{
	union mbox_msg msg;
	unsigned long start = get_timer(0);
	int id;

	memset(&msg, 0, sizeof(union mbox_msg));
	msg.s.hdr.opcode = MBOX_TARGET_STATUS_CHANGE;

	pf->tx_mbox_id++;
	msg.s.hdr.id = pf->tx_mbox_id;
	id = msg.s.hdr.id;

	write_bar4_reg(msg.words[0], TARGET_MBOX_OFFSET);

	debug("mb msg sent %d\n", pf->tx_mbox_id);
	if (msg.s.hdr.req_ack) {
		while (read_bar4_reg(HOST_MBOX_ACK_REG) != id) {
			mdelay(MBOX_WAIT_MS);
			if (get_timer(start) > MBOX_TIMEOUT_MS) {
				printf("mbox ack fail\n");
				break;
			}
		}
	}
}

void mailbox_init(struct dpi_pf *pf)
{
	write_bar4_reg(TARGET_READY, TARGET_STATUS_REG);
	mb_send_msg(pf);
}

int pem_ep_link_sts(void)
{
	u64 reg, addr;

	addr = PEM_BASE + PEM_CFG_OFFSET;
	reg = readq(addr);
	if (reg & BIT(0))
		return 1;

	addr = PEM_BASE + PEM_DIAG_STS_OFFSET;
	reg = readq(addr);

	return (reg & BIT(16)) ? 0 : 1;
}

void pem_ep_bar4_init(struct dpi_pf *pf)
{
	u64 reg, addr, val;

	reg = BAR4_DRAM_OFFSET;
	val = ((reg >> 22) << 4) | 0x1;

	addr = PEM_BASE + PEM_BAR4_INDEX_OFFSET(8);

	writel(val, (void *)addr);

	addr = PEM_BASE + PEM_DIS_PORT_OFFSET;
	writel(0x1, (void *)addr);

	addr = BAR4_DRAM_OFFSET;
	writel(0xABCDABCD, (void *)addr);

	write_bar4_reg(TARGET_VERSION, TARGET_VERSION_REG);
}

void dpi_setup_queues(struct dpi_pf *pf)
{
	/* TXQ */
	struct hw_descq *descq;
	struct sw_descq *tq, *rq;
	int i, count;

	descq = (struct hw_descq *)(BAR4_DRAM_OFFSET + RX_DESCQ_OFFSET);
	if (descq->num_entries == 0 ||
	    (descq->num_entries & (descq->num_entries - 1)))
		printf("rx hw descq err line:%d\n", __LINE__);
	if (descq->cons_idx != 0)
		printf("rx hw descq err line:%d\n", __LINE__);
	if (descq->shadow_cons_idx_addr == 0)
		printf("rx hw descq err line:%d\n", __LINE__);
	if (descq->buf_size == 0)
		printf("rx hw descq err line:%d\n", __LINE__);

	tq = &pf->txqs;
	tq->priv = pf;
	tq->hw_descq = descq;
	tq->local_cons_idx = descq->cons_idx;
	tq->mask = descq->num_entries - 1;
	tq->num_entries = descq->num_entries;
	tq->shadow_cons_idx_ioremap_addr =
		 (u32 __iomem *)(descq->shadow_cons_idx_addr);
	debug("RX DQ entries %d remap addr %llx\n",
	      descq->num_entries, descq->shadow_cons_idx_addr);

	/* RXQ */
	descq = (struct hw_descq *)(BAR4_DRAM_OFFSET + TX_DESCQ_OFFSET);
	if (descq->num_entries == 0 ||
	    (descq->num_entries & (descq->num_entries - 1)))
		printf("tx hw descq err line:%d\n", __LINE__);
	if (descq->cons_idx != 0)
		printf("tx hw descq err line:%d\n", __LINE__);
	if (descq->shadow_cons_idx_addr == 0)
		printf("tx hw descq err line:%d\n", __LINE__);
	rq = &pf->rxqs;
	rq->mask = descq->num_entries - 1;
	rq->priv = pf;
	rq->num_entries = descq->num_entries;
	rq->hw_descq = descq;
	rq->shadow_cons_idx_ioremap_addr =
		 (u32 __iomem *)(descq->shadow_cons_idx_addr);
	rq->local_cons_idx = 0;
	rq->refill_prod_idx = 0;
	rq->dma_list = (u64 *)npa_memalloc(descq->num_entries, sizeof(u64),
					   "RX DescQ Buffer array");
	if (!rq->dma_list)
		printf("out of memory for buffer array\n");
	count = circq_space(0, rq->refill_prod_idx, rq->mask);
	for (i = 0; i < descq->num_entries; i++) {
		rq->dma_list[i] = (u64)npa_memalloc(1, RECV_BUF_SIZE,
						    "TX DQ Buffer");
		if (!rq->dma_list[i]) {
			printf("out of memory for buffer\n");
			break;
		}
	}
	rq->refill_prod_idx = i;
	__iowmb();
	debug("TX DQ entries %d remap addr %llx\n",
	      descq->num_entries, descq->shadow_cons_idx_addr);
}

void handle_host_status(struct dpi_pf *pf)
{
	u64 tgt_sts, host_sts;

	tgt_sts = read_bar4_reg(TARGET_STATUS_REG);
	host_sts = read_bar4_reg(HOST_STATUS_REG);

	switch (tgt_sts) {
	case TARGET_READY:
		if (host_sts == HOST_READY) {
			dpi_setup_queues(pf);
			write_bar4_reg(TARGET_RUNNING, TARGET_STATUS_REG);
			mb_send_msg(pf);
		}
		break;
	case TARGET_RUNNING:
		debug("Target state running\n");
		break;
	default:
		printf("Incorrect state transition cur_state:%llu\n", tgt_sts);
		break;
	}
}

void mb_check_msg(struct dpi_pf *pf)
{
	u64 word0;
	union mbox_msg msg;

	word0 = read_bar4_reg(HOST_MBOX_OFFSET);
	msg.words[0] = word0;

	if (pf->rx_mbox_id == msg.s.hdr.id)
		return;

	debug("MBOX msg rcvd %d\n", pf->rx_mbox_id);
	pf->rx_mbox_id = msg.s.hdr.id;
	switch (msg.s.hdr.opcode) {
	case MBOX_HOST_STATUS_CHANGE:
		handle_host_status(pf);
		if (msg.s.hdr.req_ack)
			write_bar4_reg(msg.s.hdr.id, TARGET_MBOX_ACK_REG);
		break;
	default:
		printf("Invalid MBOX msg rcvd\n");
		break;
	}
}

int dpi_queue_instr(struct dpi_pf *pf, struct dpi_instr *new_instr)
{
	struct npa_pf *npa_pf = dev_get_priv(pf->npa_dev);
	dpi_dma_ptr_t lptr = {0};
	dpi_dma_ptr_t hptr = {0};
	u64 dpi_cmd[DPI_DMA_CMD_BUF_SIZE] = {0};
	union dpi_dma_instr_hdr_s *header;
	struct dpi_instr_q *instr_q;
	int i, count = 8, icount;
	u64 *ptr, *cmds = &dpi_cmd[0];
	u64 new_buf;

	*(u64 *)new_instr->compaddr = 0xff;

	header = (union dpi_dma_instr_hdr_s *)&dpi_cmd[0];
	header->cn98xx.nfst = 1;
	header->cn98xx.nlst = 1;
	header->cn98xx.ptr = (u64)new_instr->compaddr;
	header->cn98xx.lport = 0;
	header->cn98xx.xtype = new_instr->xfer_dir;

	lptr.s.ptr = new_instr->localaddr;
	lptr.s.length = new_instr->dma_len;
	hptr.s.ptr = new_instr->hostaddr;
	hptr.s.length = new_instr->dma_len;

	dpi_cmd[4] = lptr.u[0];
	dpi_cmd[5] = lptr.u[1];
	dpi_cmd[6] = hptr.u[0];
	dpi_cmd[7] = hptr.u[1];

	instr_q = &pf->instrq;
	if (instr_q->index + count < instr_q->chunk_size_m1) {
		ptr = (u64 *)instr_q->instr_buf;

		ptr += instr_q->index;
		instr_q->index += count;
		while (count--)
			*ptr++ = *cmds++;
		__iowmb();

		debug("DPI Cmd\n");
		for (i = 0; i < 8; i++)
			debug(" word[%d] - %llx\n", i, dpi_cmd[i]);
		ptr = (u64 *)instr_q->instr_buf;
		ptr += (instr_q->index - 8);
		debug("DPI Inst\n");
		for (i = 0; i < 8; i++)
			debug(" word[%d] - %llx\n", i, ptr[i]);
	} else {
		u16 tmp;

		ptr = (u64 *)instr_q->instr_buf;
		new_buf = npa_pf_aura_op_alloc(npa_pf);

		icount = instr_q->chunk_size_m1 - instr_q->index;
		tmp = icount;
		ptr += instr_q->index;
		count -= icount;
		while (icount--)
			*ptr++ = *cmds++;

		*ptr++ = new_buf;
		*ptr = 0;

		debug("DPI Cmd\n");
		for (i = 0; i < 8; i++)
			debug(" word[%d] - %llx\n", i, dpi_cmd[i]);
		ptr = (u64 *)instr_q->instr_buf;
		ptr += (instr_q->index - tmp);
		debug("DPI Inst\n");
		for (i = 0; i < (instr_q->index - tmp); i++)
			debug(" word[%d] - %llx\n", i, ptr[i]);

		instr_q->instr_buf = new_buf;
		instr_q->index = count;
		ptr = (u64 *)instr_q->instr_buf;
		while (count--)
			*ptr++ = *cmds++;

		ptr = (u64 *)instr_q->instr_buf;
		debug("DPI Inst\n");
		for (i = 0; i < instr_q->index; i++)
			debug(" word[%d] - %llx\n", i, ptr[i]);
	}
	write_dpi_vf_reg(pf, DPI_VDMA_DBELL, 8);

	return 0;
}

int dpi_wait_for_sts(struct sw_descq *swq)
{
	unsigned long start;

	start = get_timer(0);
	while (true) {
		__iowmb();
		debug("%s comp data %llx\n", __func__, swq->comp_data);
		if (swq->comp_data != 0xff)
			return 0;
		if (get_timer(start) > COMP_TIMEOUT_MS) {
			printf("completion timeout\n");
			return 1;
		}
	}
}

int dpi_xmit(struct udevice *dev, void *pkt, int pkt_len)
{
	struct dpi_pf *dpi = dev_get_priv(dev);
	struct  hw_desc_ptr *ptr;
	u32 cons_idx, prod_idx;
	struct hw_descq  *descq;
	struct sw_descq  *tq;
	struct dpi_instr instr;
	u8 cnt, *tmp;

	/* Pad packet size upto 64 bytes */
	if (pkt_len < 64) {
		tmp = (u8 *)pkt;
		cnt = 64 - pkt_len;
		for (int i = 0; i < cnt; i++)
			tmp[pkt_len + i] = 0x0;
		pkt_len += cnt;
	}
	mb_check_msg(dpi);

	if (read_bar4_reg(HOST_STATUS_REG) != HOST_RUNNING) {
		printf("%s Host not running\n", __func__);
		return -1;
	}
	tq = &dpi->txqs;
	descq = tq->hw_descq;

	cons_idx = tq->local_cons_idx;
	prod_idx = descq->prod_idx;

	ptr = &descq->desc_arr[cons_idx];
	ptr->hdr.s_mgmt_net.total_len = pkt_len;
	ptr->hdr.s_mgmt_net.ptr_len   = pkt_len;
	ptr->hdr.s_mgmt_net.is_frag = 0;

	cons_idx = circq_inc(cons_idx, tq->mask);
	tq->local_cons_idx = cons_idx;

	instr.localaddr = (u64)pkt;
	instr.hostaddr = ptr->ptr;
	instr.compaddr = &(tq->comp_data);
	instr.xfer_dir = DPI_HDR_XTYPE_E_OUTBOUND;
	instr.dma_len = pkt_len;
	dpi_queue_instr(dpi, &instr);

	debug("DPI XMIT\n");
	debug("consid%d prodid%d\n", cons_idx, prod_idx);
	debug("pkt %llx len %d, host ptr %llx comp %llx\n",
	      instr.localaddr, instr.dma_len, instr.hostaddr,
	      (u64)instr.compaddr);

	if (dpi_wait_for_sts(tq)) {
		return -1;
	} else if (tq->comp_data) {
		printf("%s TX PKT comp error %llx\n", __func__,
		       tq->comp_data);
	} else {
#define DEBUG_PKT
#ifdef DEBUG_PKT
		debug("TX PKT Data\n");
		for (int i = 0; i < pkt_len; i++) {
			if (i && (i % 8 == 0))
				debug("\n");
			debug("%02x ", *((u8 *)pkt + i));
		}
		debug("\n");
#endif

		descq->cons_idx = cons_idx;
		__iowmb();
		instr.localaddr = (u64)&tq->local_cons_idx;
		instr.hostaddr = (u64)tq->shadow_cons_idx_ioremap_addr;
		instr.compaddr = &(tq->comp_data);
		instr.xfer_dir = DPI_HDR_XTYPE_E_OUTBOUND;
		instr.dma_len = 1;
		dpi_queue_instr(dpi, &instr);

		if (dpi_wait_for_sts(tq))
			return -1;
		if (tq->comp_data) {
			printf("%s TX Host update comp error %llx\n",
			       __func__, tq->comp_data);
		}
	}
	return 0;
}

int dpi_free_pkt(struct udevice *dev, uchar *pkt, int pkt_len)
{
	struct dpi_pf *dpi = dev_get_priv(dev);
	u32 cons_idx, prod_idx, mask;
	struct hw_descq  *descq;
	struct sw_descq  *rq;
	struct dpi_instr instr;
	int count;

	if (!pkt || !pkt_len)
		return -1;

	if (read_bar4_reg(HOST_STATUS_REG) != HOST_RUNNING) {
		debug("%s Host not running\n", __func__);
		return -1;
	}
	/* Clearing packet header */
	memset((void *)pkt, 0, 64);

	rq = &dpi->rxqs;
	descq = rq->hw_descq;
	prod_idx = rq->local_cons_idx;
	cons_idx = descq->cons_idx;
	mask = rq->mask;
	count = circq_depth(prod_idx, cons_idx, mask);
	if (!count)
		return 0;

	cons_idx = circq_inc(cons_idx, rq->mask);
	rq->cons_idx = cons_idx;

	__iowmb();
	descq->cons_idx = rq->cons_idx;

	instr.localaddr = (u64)&rq->cons_idx;
	instr.hostaddr = (u64)rq->shadow_cons_idx_ioremap_addr;
	instr.compaddr = &(rq->comp_data);
	instr.xfer_dir = DPI_HDR_XTYPE_E_OUTBOUND;
	instr.dma_len = 1;

	dpi_queue_instr(dpi, &instr);

	debug("%s RXQ cid %x lcid %x DQ cid %xx\n", __func__,
	      rq->cons_idx, rq->local_cons_idx, descq->cons_idx);

	if (dpi_wait_for_sts(rq))
		return -1;
	if (rq->comp_data) {
		printf("%s RX Host update comp error %llx\n",
		       __func__, rq->comp_data);
	}

	return 0;
}

int dpi_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct dpi_pf *dpi = dev_get_priv(dev);
	struct hw_desc_ptr *ptr;
	struct hw_descq  *descq;
	struct sw_descq  *rq;
	struct dpi_instr instr;
	u32 cons_idx, prod_idx, mask;
	void *pkt_buf;
	int count;

	mb_check_msg(dpi);

	if (read_bar4_reg(HOST_STATUS_REG) != HOST_RUNNING) {
		printf("%s Host not running\n", __func__);
		return -1;
	}
	rq = &dpi->rxqs;
	descq = rq->hw_descq;

	cons_idx = rq->local_cons_idx;
	prod_idx = descq->prod_idx;
	mask = rq->mask;
	count = circq_depth(prod_idx, cons_idx, mask);
	if (!count)
		return -EAGAIN;

	ptr = &descq->desc_arr[cons_idx];
	pkt_buf = (void *)rq->dma_list[cons_idx];

	instr.dma_len = ptr->hdr.s_mgmt_net.total_len;
	instr.hostaddr = ptr->ptr;
	instr.localaddr = rq->dma_list[cons_idx];
	instr.compaddr = &(rq->comp_data);
	instr.xfer_dir = DPI_HDR_XTYPE_E_INBOUND;

	dpi_queue_instr(dpi, &instr);

	debug("DPI RECV\n");
	debug("consid%d prodid%d\n", cons_idx, prod_idx);
	debug("pkt %llx len %d, host ptr %llx comp %llx\n",
	      instr.localaddr, instr.dma_len, instr.hostaddr,
	      (u64)instr.compaddr);

	if (dpi_wait_for_sts(rq))
		return -1;
	if (rq->comp_data) {
		printf("%s RX PKT comp error %llx\n",
		       __func__, rq->comp_data);
		return -1;
	} else {
		debug("RX PKT complete\n");
		pkt_buf = (void *)instr.localaddr;
		cons_idx = circq_inc(cons_idx, rq->mask);
		rq->local_cons_idx = cons_idx;
		*packetp = (uchar *)pkt_buf;

#define DEBUG_PKT
#ifdef DEBUG_PKT
		debug("RX PKT Data\n");
		for (int i = 0; i < instr.dma_len; i++) {
			if (i && (i % 8 == 0))
				debug("\n");
			debug("%02x ", *(u8 *)(pkt_buf + i));
		}
		debug("\n");
#endif

		return instr.dma_len;
	}
}

int dpi_setup_mac(struct udevice *dev)
{
	struct dpi_pf *dpi = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	if (memcmp(dpi->hw_addr, pdata->enetaddr, ARP_HLEN)) {
		memcpy(dpi->hw_addr, pdata->enetaddr, 6);
		eth_env_set_enetaddr_by_index("eth", dev_seq(dev),
					      pdata->enetaddr);
	}
	return 0;
}

void dpi_halt(struct udevice *dev)
{
	struct dpi_pf *dpi = dev_get_priv(dev);

	write_dpi_vf_reg(dpi, DPI_VDMA_EN, 0x0ULL);
}

int dpi_start(struct udevice *dev)
{
	struct dpi_pf *dpi = dev_get_priv(dev);

	if (pem_ep_link_sts()) {
		printf("%s PCIe Link down\n", __func__);
		return -1;
	}

	mb_check_msg(dpi);
	mdelay(10);
	if (read_bar4_reg(HOST_STATUS_REG) != HOST_RUNNING) {
		printf("%s Host not running\n", __func__);
		return -1;
	}

	write_dpi_vf_reg(dpi, DPI_VDMA_EN, 0x1ULL);

	return 0;
}

int dpi_pf_init(struct dpi_pf *dpi)
{
	struct eth_pdata *pdata = dev_get_plat(dpi->dev);
	u64 reg;
	u32 mps_val, mrrs_val, aura = NPA_POOL_INST;
	u16 npa_pf_func;
	struct npa_pf *npa_pf = dev_get_priv(dpi->npa_dev);
	u64 buf;
	char name[32];
	u8 mac[6];
	int i;

	pem_ep_bar4_init(dpi);
	mailbox_init(dpi);

	/* PF init */
	for (i = 0; i < DPI_MAX_ENGINES; i++) {
		write_dpi_reg(dpi, DPI_DMA_ENGX_BUF(i), DPI_ENG_BUF_BLKS(8));
		write_dpi_reg(dpi, DPI_DMA_ENGX_EN(i), 0x0ULL);
	}

	reg = (DPI_DMA_CONTROL_ZBWCSEN | DPI_DMA_CONTROL_PKT_EN |
	       DPI_DMA_CONTROL_LDWB | DPI_DMA_CONTROL_O_MODE);
	reg |= DPI_DMA_CONTROL_DMA_ENB(0x3fULL);
	write_dpi_reg(dpi, DPI_DMA_CONTROL, reg);
	write_dpi_reg(dpi, DPI_CTL, DPI_CTL_EN);
	write_dpi_reg(dpi, DPI_DMAX_QRST, 0x1ULL);

	mps_val = fls(128) - 8;
	mrrs_val = fls(128) - 8;
	for (int port = 0; port < DPI_EBUS_MAX_PORTS; port++) {
		reg = read_dpi_reg(dpi, DPI_EBUS_PORTX_CFG(port));
		reg &= ~(DPI_EBUS_PORTX_CFG_MRRS(0x7) |
			 DPI_EBUS_PORTX_CFG_MPS(0x7));
		reg |= (DPI_EBUS_PORTX_CFG_MPS(mps_val) |
			DPI_EBUS_PORTX_CFG_MRRS(mrrs_val));
		write_dpi_reg(dpi, DPI_EBUS_PORTX_CFG(port), reg);
	}

	/* VF queue init */
	reg = DPI_DMA_IBUFF_CSIZE_CSIZE((u64)(INST_CHUNK_SIZE / 8));
	write_dpi_reg(dpi, DPI_DMAX_IBUFF_CSIZE, reg);
	for (int i = 0; i < DPI_MAX_ENGINES; i++) {
		reg = 0;
		reg = read_dpi_reg(dpi, DPI_DMA_ENGX_EN(i));
		reg |= DPI_DMA_ENG_EN_QEN(0x1);
		write_dpi_reg(dpi, DPI_DMA_ENGX_EN(i), reg);
	}

	reg = read_dpi_reg(dpi, DPI_DMAX_IDS2);
	reg |= DPI_DMA_IDS2_INST_AURA(aura);
	write_dpi_reg(dpi, DPI_DMAX_IDS2, reg);

	npa_pf_func = (((npa_pf->pf_id & 0x3f) << 10) | 0x0);
	reg = read_dpi_reg(dpi, DPI_DMAX_IDS);
	reg |= DPI_DMA_IDS_DMA_NPA_PF_FUNC(npa_pf_func);
	write_dpi_reg(dpi, DPI_DMAX_IDS, reg);

	write_dpi_vf_reg(dpi, DPI_VDMA_EN, 0x0ULL);
	write_dpi_vf_reg(dpi, DPI_VDMA_REQQ_CTL, 0x0ULL);
	buf = npa_pf_aura_op_alloc(npa_pf);
	write_dpi_vf_reg(dpi, DPI_VDMA_SADDR, ((buf >> 7) << 7));

	dpi->instrq.instr_buf = buf;
	dpi->instrq.index = 0;
	dpi->instrq.chunk_size_m1 = (INST_CHUNK_SIZE >> 3) - 2;
	mb_check_msg(dpi);

	sprintf(name, "%s", "dpi-mac-addr");
	eth_env_get_enetaddr(name, mac);
	if (is_valid_ethaddr(mac)) {
		memcpy(pdata->enetaddr, mac, 6);
		memcpy(dpi->hw_addr, mac, 6);
	} else {
		net_random_ethaddr(dpi->hw_addr);
		memcpy(pdata->enetaddr, dpi->hw_addr, 6);
	}
	eth_env_set_enetaddr_by_index("eth", dev_seq(dpi->dev),
				      pdata->enetaddr);
	return 0;
}

static const struct eth_ops dpi_eth_ops = {
	.start			= dpi_start,
	.send			= dpi_xmit,
	.recv			= dpi_recv,
	.free_pkt		= dpi_free_pkt,
	.stop			= dpi_halt,
	.write_hwaddr		= dpi_setup_mac,
};

int dpi_pf_probe(struct udevice *dev)
{
	struct dpi_pf *dpi = dev_get_priv(dev);
	int err;
	struct udevice *ndev;

	dpi->pf_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0,
				      PCI_REGION_TYPE, PCI_REGION_MEM);
	dpi->vf_base = dpi->pf_base + BIT_ULL(33);
	dpi->dev = dev;

	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
				 PCI_DEVICE_ID_CAVIUM_NPA_PF, 0,
				 &ndev);
	if (err) {
		printf("%s NPA PF device not found\n", __func__);
		return err;
	}
	dpi->npa_dev = ndev;

	err = pci_sriov_init(dev, 1);
	if (err) {
		printf("%s: Error %d initializing VFs\n", __func__, err);
		return err;
	}

	err = dpi_pf_init(dpi);
	if (err)
		printf("%s: Error %d dpi init\n", __func__, err);

	return err;
}

int dpi_pf_remove(struct udevice *dev)
{
	struct dpi_pf *dpi = dev_get_priv(dev);
	int i;

	write_dpi_vf_reg(dpi, DPI_VDMA_EN, 0x0ULL);
	while (!(read_dpi_vf_reg(dpi, DPI_VDMA_SADDR) & BIT(63)))
		;
	write_dpi_reg(dpi, DPI_DMAX_QRST, 0x1ULL);

	write_bar4_reg(TARGET_GOING_DOWN, TARGET_STATUS_REG);
	mb_send_msg(dpi);

	for (i = 0; i < dpi->rxqs.num_entries; i++)
		free((void *)dpi->rxqs.dma_list[i]);
	for (i = 0; i < DPI_MAX_ENGINES; i++) {
		write_dpi_reg(dpi, DPI_DMA_ENGX_EN(i), 0x0ULL);
		write_dpi_reg(dpi, DPI_DMA_ENGX_BUF(i), 0);
	}
	write_dpi_reg(dpi, DPI_CTL, 0x0);
	write_dpi_reg(dpi, DPI_DMA_CONTROL, 0x0);

	write_bar4_reg(0x0, PEM_BASE + PEM_BAR4_INDEX_OFFSET(8));

	device_remove(dpi->npa_dev, DM_REMOVE_NORMAL);
	debug("%s: dpi pf down --\n", __func__);

	return 0;
}

U_BOOT_DRIVER(dpi_pf) = {
	.name   = "dpi_pf",
	.id     = UCLASS_ETH,
	.probe	= dpi_pf_probe,
	.remove = dpi_pf_remove,
	.ops    = &dpi_eth_ops,
	.priv_auto = sizeof(struct dpi_pf),
	.plat_auto = sizeof(struct eth_pdata),
};

static struct pci_device_id dpi_pf_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_DPI_PF) },
	{}
};

U_BOOT_PCI_DEVICE(dpi_pf, dpi_pf_supported);
