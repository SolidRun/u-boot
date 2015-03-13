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
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <asm/io.h>

#include <cavium/thunderx_vnic.h>

#include "nic_reg.h"
#include "nic.h"
#include "nicvf_queues.h"
#include "thunder_bgx.h"

#define ETH_ALEN 6

/* Register read/write APIs */
void nicvf_reg_write(struct nicvf *nicvf, uint64_t offset, uint64_t val)
{
	uint64_t addr = nicvf->reg_base + offset;

	writeq(val, (void *)addr);
}

uint64_t nicvf_reg_read(struct nicvf *nicvf, uint64_t offset)
{
	uint64_t addr = nicvf->reg_base + offset;

	return readq((void *)addr);
}

void nicvf_queue_reg_write(struct nicvf *nicvf, uint64_t offset,
			   uint64_t qidx, uint64_t val)
{
	uint64_t addr = nicvf->reg_base + offset;

	writeq(val, (void *)(addr + (qidx << NIC_Q_NUM_SHIFT)));
}

uint64_t nicvf_queue_reg_read(struct nicvf *nicvf, uint64_t offset, uint64_t qidx)
{
	uint64_t addr = nicvf->reg_base + offset;

	return readq((void *)(addr + (qidx << NIC_Q_NUM_SHIFT)));
}

/* VF -> PF mailbox communication */
static bool pf_ready_to_rcv_msg;
static bool pf_acked;
static bool pf_nacked;

int nicvf_lock_mbox(struct nicvf *nicvf)
{
	int timeout = NIC_PF_VF_MBX_TIMEOUT;
	int sleep = 10;
	uint64_t lock, mbx_addr;

	mbx_addr = NIC_VF_PF_MAILBOX_0_7 + NIC_PF_VF_MBX_LOCK_OFFSET;
	lock = NIC_PF_VF_MBX_LOCK_VAL(nicvf_reg_read(nicvf, mbx_addr));
	while (lock) {
		mdelay(sleep);
		lock = NIC_PF_VF_MBX_LOCK_VAL(nicvf_reg_read(nicvf, mbx_addr));
		timeout -= sleep;
		if (!timeout) {
			printf("VF%d Couldn't lock mailbox\n", nicvf->vf_id);
			return 0;
		}
	}
	nicvf_reg_write(nicvf, mbx_addr, NIC_PF_VF_MBX_LOCK_SET(lock));
	return 1;
}

void nicvf_release_mbx(struct nicvf *nicvf)
{
	uint64_t mbx_addr, lock;

	mbx_addr = NIC_VF_PF_MAILBOX_0_7 + NIC_PF_VF_MBX_LOCK_OFFSET;
	lock = nicvf_reg_read(nicvf, mbx_addr);
	nicvf_reg_write(nicvf, mbx_addr, NIC_PF_VF_MBX_LOCK_CLEAR(lock));
}

static void  nicvf_handle_mbx_intr(struct nicvf *nicvf);

int nicvf_send_msg_to_pf(struct nicvf *nicvf, struct nic_mbx *mbx)
{
	int i, timeout = NIC_PF_VF_MBX_TIMEOUT;
	int sleep = 10;
	uint64_t *msg;
	uint64_t mbx_addr;

	if (!nicvf_lock_mbox(nicvf))
		return -1;

	pf_acked = false;
	pf_nacked = false;
	mbx->mbx_trigger_intr = 1;
	msg = (uint64_t *)mbx;

	mbx_addr = nicvf->reg_base + NIC_VF_PF_MAILBOX_0_7;

	for (i = 0; i < NIC_PF_VF_MAILBOX_SIZE; i++)
		writeq(*(msg + i), (void *)(mbx_addr + (i * 8)));

	nicvf_release_mbx(nicvf);

	nic_handle_mbx_intr(nicvf->nicpf, nicvf->vf_id);

	/* Wait for previous message to be acked, timeout 2sec */
	while (!pf_acked) {
		if (pf_nacked)
			return -1;
		mdelay(sleep);
		nicvf_handle_mbx_intr(nicvf);

		if (pf_acked)
			break;
		timeout -= sleep;
		if (!timeout) {
			printf("PF didn't ack to mbox msg %d from VF%d\n",
			       (mbx->msg & 0xFF), nicvf->vf_id);
			return -1;
		}
	}

	return 0;
}

/* Checks if VF is able to comminicate with PF
* and also gets the VNIC number this VF is associated to.
*/
static int nicvf_check_pf_ready(struct nicvf *nicvf)
{
	int timeout = 5000, sleep = 20;
	uint64_t mbx_addr = NIC_VF_PF_MAILBOX_0_7;

	pf_ready_to_rcv_msg = false;

	nicvf_reg_write(nicvf, mbx_addr, NIC_PF_VF_MSG_READY);

	mbx_addr += (NIC_PF_VF_MAILBOX_SIZE - 1) * 8;
	nicvf_reg_write(nicvf, mbx_addr, 1ULL);

	nic_handle_mbx_intr(nicvf->nicpf, nicvf->vf_id);

	while (!pf_ready_to_rcv_msg) {
		mdelay(sleep);
		nicvf_handle_mbx_intr(nicvf);

		if (pf_ready_to_rcv_msg)
			break;
		timeout -= sleep;
		if (!timeout) {
			printf("PF didn't respond to READY msg\n");
			return 0;
		}
	}
	return 1;
}

static void nicvf_handle_mbx_intr(struct nicvf *nicvf)
{
	struct nic_mbx mbx = { };
	uint64_t *mbx_data;
	uint64_t mbx_addr;
	int i;

	mbx_addr = NIC_VF_PF_MAILBOX_0_7;
	mbx_data = (uint64_t *)&mbx;

	for (i = 0; i < NIC_PF_VF_MAILBOX_SIZE; i++) {
		*mbx_data = nicvf_reg_read(nicvf, mbx_addr);
		asm volatile("dsb sy");

		mbx_data++;
		mbx_addr += NIC_PF_VF_MAILBOX_SIZE;
	}

	debug("Mbox message from PF, msg 0x%x\n", mbx.msg);

	switch (mbx.msg & NIC_PF_VF_MBX_MSG_MASK) {
	case NIC_PF_VF_MSG_READY:
		pf_ready_to_rcv_msg = true;
		nicvf->vf_id = mbx.data.nic_cfg.vf_id & 0x7F;
		nicvf->tns_mode = mbx.data.nic_cfg.tns_mode & 0x7F;
		nicvf->node = mbx.data.nic_cfg.node_id;

		debug("MAC: %pM\n", nicvf->netdev->enetaddr);
		break;
	case NIC_PF_VF_MSG_ACK:
		pf_acked = true;
		break;
	case NIC_PF_VF_MSG_NACK:
		pf_nacked = true;
		break;
	default:
		printf("Invalid message from PF, msg 0x%x\n", mbx.msg);
		break;
	}
	nicvf_clear_intr(nicvf, NICVF_INTR_MBOX, 0);
}

static int nicvf_hw_set_mac_addr(struct nicvf *nicvf, struct eth_device *netdev)
{
	struct nic_mbx mbx = { };
	int i;

	mbx.msg = NIC_PF_VF_MSG_SET_MAC;
	mbx.data.mac.vf_id = nicvf->vf_id;
	for (i = 0; i < ETH_ALEN; i++)
		mbx.data.mac.addr =
		    (mbx.data.mac.addr << 8) | netdev->enetaddr[i];

	return nicvf_send_msg_to_pf(nicvf, &mbx);
}

void nicvf_config_cpi(struct nicvf *nicvf)
{
	struct nic_mbx mbx = { };

	mbx.msg = NIC_PF_VF_MSG_CPI_CFG;
	mbx.data.cpi_cfg.vf_id = nicvf->vf_id;
	mbx.data.cpi_cfg.cpi_alg = nicvf->cpi_alg;
	mbx.data.cpi_cfg.rq_cnt = nicvf->qs->rq_cnt;

	nicvf_send_msg_to_pf(nicvf, &mbx);
}

static int nicvf_init_resources(struct nicvf *nicvf)
{
	int err;

	nicvf->num_qs = 1;

	/* Enable Qset */
	nicvf_qset_config(nicvf, true);

	/* Initialize queues and HW for data transfer */
	err = nicvf_config_data_transfer(nicvf, true);
	if (err) {
		printf("Failed to alloc/config VF's QSet resources\n");
		return err;
	}
	return 0;
}

void nicvf_free_pkt(struct nicvf *nicvf, void *pkt)
{
	free(pkt);
}

static void nicvf_snd_pkt_handler(struct nicvf *nicvf,
				  struct cmp_queue *cq,
				  void *cq_desc, int cqe_type)
{
	struct cqe_send_t *cqe_tx;
	struct snd_queue *sq;
	struct sq_hdr_subdesc *hdr;

	cqe_tx = (struct cqe_send_t *)cq_desc;
	sq = &nicvf->qs->sq[cqe_tx->sq_idx];

	hdr = (struct sq_hdr_subdesc *)GET_SQ_DESC(sq, cqe_tx->sqe_ptr);
	if (hdr->subdesc_type != SQ_DESC_TYPE_HEADER)
		return;

	debug("%s Qset #%d SQ #%d SQ ptr #%d subdesc count %d\n",
	      __func__, cqe_tx->sq_qs, cqe_tx->sq_idx,
	      cqe_tx->sqe_ptr, hdr->subdesc_cnt);

	nicvf_check_cqe_tx_errs(nicvf, cq, cq_desc);
	nicvf_put_sq_desc(sq, hdr->subdesc_cnt + 1);
}

static int nicvf_rcv_pkt_handler(struct nicvf *nicvf,
				 struct cmp_queue *cq, void *cq_desc,
				 void **ppkt, int cqe_type)
{
	void *pkt;

	size_t pkt_len;
	struct cqe_rx_t *cqe_rx = (struct cqe_rx_t *)cq_desc;
	int err = 0;

	/* Check for errors */
	err = nicvf_check_cqe_rx_errs(nicvf, cq, cq_desc);
	if (err && !cqe_rx->rb_cnt)
		return -1;

	pkt = nicvf_get_rcv_pkt(nicvf, cq_desc, &pkt_len);
	if (!pkt) {
		debug("Packet not received\n");
		return -1;
	}

	if (pkt)
		*ppkt = pkt;

	return pkt_len;
}

int nicvf_cq_handler(struct nicvf *nicvf, void **ppkt, int *pkt_len)
{
	int cq_qnum = 0;
	int processed_sq_cqe = 0;
	int processed_rq_cqe = 0;
	int processed_cqe = 0;

	unsigned long cqe_count, cqe_head;
	struct queue_set *qs = nicvf->qs;
	struct cmp_queue *cq = &qs->cq[cq_qnum];
	struct cqe_rx_t *cq_desc;

	/* Get num of valid CQ entries expect next one to be SQ completion */
	cqe_count = nicvf_queue_reg_read(nicvf, NIC_QSET_CQ_0_7_STATUS, cq_qnum);
	cqe_count &= 0xFFFF;
	if (!cqe_count)
		return 0;

	/* Get head of the valid CQ entries */
	cqe_head =
	    nicvf_queue_reg_read(nicvf, NIC_QSET_CQ_0_7_HEAD, cq_qnum) >> 9;
	cqe_head &= 0xFFFF;

	if (cqe_count) {
		/* Get the CQ descriptor */
		cq_desc = (struct cqe_rx_t *)GET_CQ_DESC(cq, cqe_head);

		switch (cq_desc->cqe_type) {
		case CQE_TYPE_RX:
			debug("%s: Got Rx CQE\n", nicvf->netdev->name);
			*pkt_len =
			    nicvf_rcv_pkt_handler(nicvf, cq, cq_desc, ppkt,
						  CQE_TYPE_RX);
			processed_rq_cqe++;
			break;
		case CQE_TYPE_SEND:
			debug("%s: Got Tx CQE\n", nicvf->netdev->name);
			nicvf_snd_pkt_handler(nicvf, cq, cq_desc, CQE_TYPE_SEND);
			processed_sq_cqe++;
			break;
		default:
			debug("%s: Got CQ type %u\n", nicvf->netdev->name,
			      cq_desc->cqe_type);
			break;
		}
		processed_cqe++;
		cqe_head++;
		cqe_head &= (cq->dmem.q_len - 1);
	}

	/* Dequeue CQE */
	nicvf_queue_reg_write(nicvf, NIC_QSET_CQ_0_7_DOOR,
			      cq_qnum, processed_cqe);

	asm volatile ("dsb sy");

	return processed_sq_cqe;
}

/* Qset error interrupt handler
 *
 * As of now only CQ errors are handled
 */
void nicvf_handle_qs_err(struct nicvf *nicvf)
{
	struct queue_set *qs = nicvf->qs;
	int qidx;
	uint64_t status;

	/* Check if it is CQ err */
	for (qidx = 0; qidx < qs->cq_cnt; qidx++) {
		status = nicvf_queue_reg_read(nicvf, NIC_QSET_CQ_0_7_STATUS,
					      qidx);
		if (!(status & CQ_ERR_MASK))
			continue;
		/* Process already queued CQEs and reconfig CQ */
		nicvf_sq_disable(nicvf, qidx);
		nicvf_cmp_queue_config(nicvf, qs, qidx, true);
		nicvf_sq_free_used_descs(nicvf->netdev, &qs->sq[qidx], qidx);
		nicvf_sq_enable(nicvf, &qs->sq[qidx], qidx);

		nicvf_enable_intr(nicvf, NICVF_INTR_CQ, qidx);
	}
}

static int nicvf_xmit(struct eth_device *netdev, void *pkt, int pkt_len)
{
	struct nicvf *nicvf = netdev->priv;
	int ret = 0;
	int rcv_len = 0;
	unsigned int timeout = 5000;
	void *rpkt = NULL;

	if (!nicvf_sq_append_pkt(nicvf, pkt, pkt_len)) {
		printf("VF%d: TX ring full\n", nicvf->vf_id);
		return -1;
	}

	/* check and update CQ for pkt sent */
	while (!ret && timeout--) {
		ret = nicvf_cq_handler(nicvf, &rpkt, &rcv_len);
		if (!ret)
			debug("%s: %d, Not sent\n", __func__, __LINE__);
		udelay(1);
	}

	return 0;
}

static int nicvf_recv(struct eth_device *netdev)
{
	struct nicvf *nicvf = netdev->priv;
	void *pkt;
	int pkt_len = 0;
#ifdef DEBUG
	u8 *dpkt;
	int i, j;
#endif

	nicvf_cq_handler(nicvf, &pkt, &pkt_len);

	if (pkt_len) {
#ifdef DEBUG
		dpkt = pkt;
		printf("RX packet contents:\n");
		for (i = 0; i < 8; i++) {
			puts("\t");
			for (j = 0; j < 10; j++)
				printf("%02x ", dpkt[i * 10 + j]);
			puts("\n");
		}
#endif
		net_process_received_packet(pkt, pkt_len);
		nicvf_refill_rbdr(nicvf);
	}

	return pkt_len;
}

void nicvf_stop(struct eth_device *netdev)
{
	struct nicvf *nicvf = netdev->priv;

	/* Free resources */
	nicvf_config_data_transfer(nicvf, false);

	/* Disable HW Qset */
	nicvf_qset_config(nicvf, false);
}

int nicvf_open(struct eth_device *netdev, bd_t *bis)
{
	int err;
	struct nicvf *nicvf = netdev->priv;
	struct nicpf *nicpf = nicvf->nicpf;
	int vnic = nicvf->vf_id;

	int bgx, lmac;

	bgx = NIC_GET_BGX_FROM_VF_LMAC_MAP(nicpf->vf_lmac_map[vnic]);
	lmac = NIC_GET_LMAC_FROM_VF_LMAC_MAP(nicpf->vf_lmac_map[vnic]);

	bgx_lmac_enable(bgx, lmac);
	nicvf_hw_set_mac_addr(nicvf, netdev);

	/* Configure CPI alorithm */
	nicvf->cpi_alg = CPI_ALG_NONE;
	nicvf_config_cpi(nicvf);

	/* Initialize the queues */
	err = nicvf_init_resources(nicvf);
	if (err)
		return -1;

	if (!nicvf_check_pf_ready(nicvf))
		return -1;

	/* Make sure queue initialization is written */
	asm volatile ("dsb sy");

	return 0;
}

int nicvf_initialize(struct nicpf *nicpf, int vf_num, unsigned int node)
{
	struct eth_device *netdev = NULL;
	struct nicvf *nicvf = NULL;
	int    err;

	netdev = malloc(sizeof(struct eth_device));

	if (!netdev) {
		err = -1;
		goto fail;
	}

	nicvf = malloc(sizeof(struct nicvf));

	if (!nicvf) {
		err = -1;
		goto fail;
	}

	netdev->priv = nicvf;
	nicvf->netdev = netdev;
	nicvf->nicpf = nicpf;
	nicvf->vf_id = vf_num;

	/* MAP VF's configuration registers */
	nicvf->reg_base = CSR_PA(NIC_VFX_BAR0(vf_num), node);
	if (!nicvf->reg_base) {
		printf("Cannot map config register space, aborting\n");
		err = -1;
		goto fail;
	}

	err = nicvf_set_qset_resources(nicvf);
	if (err)
		return -1;

	snprintf(netdev->name, sizeof(netdev->name), "vnic%u", vf_num);

	netdev->halt = nicvf_stop;
	netdev->init = nicvf_open;
	netdev->send = nicvf_xmit;
	netdev->recv = nicvf_recv;

	if (!eth_getenv_enetaddr_by_index("eth", vf_num, netdev->enetaddr)) {
		eth_getenv_enetaddr("ethaddr", netdev->enetaddr);
		netdev->enetaddr[5] += vf_num;
	}

	err = eth_register(netdev);

	if (err) {
		printf("Failed to register netdevice\n");
		return -1;
	}

	return 0;
fail:
	if (nicvf)
		free(nicvf);
	if (netdev)
		free(netdev);
	return err;
}
