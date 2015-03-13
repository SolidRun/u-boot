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

#include "nic_reg.h"
#include "nic.h"
#include "q_struct.h"
#include "nicvf_queues.h"

struct rcv_buffer {
	void *alloc;
	void *data;
};

static int nicvf_poll_reg(struct nicvf *nic, int qidx,
			  uint64_t reg, int bit_pos, int bits, int val)
{
	uint64_t bit_mask;
	uint64_t reg_val;
	int timeout = 10;

	bit_mask = (1ULL << bits) - 1;
	bit_mask = (bit_mask << bit_pos);

	while (timeout) {
		reg_val = nicvf_queue_reg_read(nic, reg, qidx);
		if (((reg_val & bit_mask) >> bit_pos) == val)
			return 0;
		udelay(2000);
		timeout--;
	}
	printf("Poll on reg 0x%llx failed\n", reg);
	return 1;
}

static int nicvf_alloc_q_desc_mem(struct nicvf *nic, struct q_desc_mem *dmem,
				  int q_len, int desc_size, int align_bytes)
{
	dmem->q_len = q_len;
	dmem->size = (desc_size * q_len) + align_bytes;
	dmem->unalign_base = malloc(dmem->size);
	if (!dmem->unalign_base)
		return -1;

	dmem->dma = (uintptr_t) dmem->unalign_base;
	dmem->phys_base = NICVF_ALIGNED_ADDR((uint64_t) dmem->dma, align_bytes);
	dmem->base = (void *)((u8 *)dmem->unalign_base +
			      (dmem->phys_base - dmem->dma));
	return 0;
}

static void nicvf_free_q_desc_mem(struct nicvf *nic, struct q_desc_mem *dmem)
{
	if (!dmem)
		return;

	free(dmem->unalign_base);
	dmem->unalign_base = NULL;
	dmem->base = NULL;
}

static int nicvf_alloc_rcv_buffer(struct nicvf *nic,
				  uint64_t buf_len, void **rbuf)
{
	uintptr_t buf = 0;
	void *alloc;
	struct rcv_buffer *bufdata;

	buf_len += NICVF_RCV_BUF_ALIGN_BYTES + sizeof(void *);

	alloc = malloc(buf_len);

	buf = (uintptr_t) alloc;

	if (!buf) {
		printf("Failed to allocate new rcv buffer\n");
		return -1;
	}

	/* Reserve bytes for storing skb address */
	buf += sizeof(void *);

	/* Align buffer addr to cache line i.e 128 bytes */
	buf += NICVF_RCV_BUF_ALIGN_LEN(buf);

	/* Store skb address */
	bufdata = (struct rcv_buffer *)(buf - sizeof(*bufdata));

	bufdata->alloc = alloc;
	bufdata->data = (void *)(buf);

	/* Return buffer address */
	*rbuf = bufdata->data;

	return 0;
}

static void *nicvf_rb_ptr_to_pkt(struct nicvf *nic, uintptr_t rb_ptr)
{
	struct rcv_buffer *bufdata;
	void *pkt;

	bufdata = (struct rcv_buffer *)(rb_ptr - sizeof(*bufdata));

	pkt = bufdata->data;
	return pkt;
}

static void *nicvf_rb_ptr_to_buf(struct nicvf *nic, uintptr_t rb_ptr)
{
	struct rcv_buffer *bufdata;
	void *alloc;

	bufdata = (struct rcv_buffer *)(rb_ptr - sizeof(*bufdata));

	alloc = bufdata->alloc;
	return alloc;
}

static int nicvf_init_rbdr(struct nicvf *nic, struct rbdr *rbdr,
			   int ring_len, int buf_size)
{
	int idx;
	void *rbuf;
	struct rbdr_entry_t *desc;

	if (nicvf_alloc_q_desc_mem(nic, &rbdr->dmem, ring_len,
				   sizeof(struct rbdr_entry_t),
				   NICVF_RCV_BUF_ALIGN_BYTES)) {
		printf("Unable to allocate memory for rcv buffer ring\n");
		return -1;
	}

	rbdr->desc = rbdr->dmem.base;
	/* Buffer size has to be in multiples of 128 bytes */
	rbdr->buf_size = buf_size;
	rbdr->enable = true;
	rbdr->thresh = RBDR_THRESH;

	for (idx = 0; idx < ring_len; idx++) {
		if (nicvf_alloc_rcv_buffer(nic, rbdr->buf_size, &rbuf))
			return -1;

		desc = GET_RBDR_DESC(rbdr, idx);
		desc->buf_addr = (uintptr_t) rbuf >> NICVF_RCV_BUF_ALIGN;
		flush_dcache_range((uintptr_t) desc,
				   (uintptr_t) desc + sizeof(desc));
	}
	return 0;
}

static void nicvf_free_rbdr(struct nicvf *nic, struct rbdr *rbdr)
{
	int head, tail;
	void *pkt;
	uint64_t buf_addr;
	struct rbdr_entry_t *desc;

	if (!rbdr)
		return;

	rbdr->enable = false;
	if (!rbdr->dmem.base)
		return;

	head = 0;
	tail = rbdr->dmem.q_len - 1;

	/* Free SKBs */
	while (head != tail) {
		desc = GET_RBDR_DESC(rbdr, head);

		invalidate_dcache_range((uintptr_t) desc,
					(uintptr_t) desc + sizeof(desc));
		buf_addr = desc->buf_addr << NICVF_RCV_BUF_ALIGN;
		pkt = nicvf_rb_ptr_to_buf(nic, buf_addr);

		free(pkt);

		head++;
		head &= (rbdr->dmem.q_len - 1);
	}
	/* Free SKB of tail desc */
	desc = GET_RBDR_DESC(rbdr, tail);

	invalidate_dcache_range((uintptr_t) desc,
				(uintptr_t) desc + sizeof(desc));

	buf_addr = desc->buf_addr << NICVF_RCV_BUF_ALIGN;

	pkt = nicvf_rb_ptr_to_buf(nic, buf_addr);

	invalidate_dcache_range(buf_addr, buf_addr + rbdr->buf_size);

	free(pkt);

	/* Free RBDR ring */
	nicvf_free_q_desc_mem(nic, &rbdr->dmem);
}

/* Refill receive buffer descriptors with new buffers.
 * This runs in softirq context .
 */
void nicvf_refill_rbdr(struct nicvf *nic)
{
	struct queue_set *qs = nic->qs;
	int rbdr_idx = qs->rbdr_cnt;
	int qcount;
	struct rbdr *rbdr;

refill:
	if (!rbdr_idx)
		return;
	rbdr_idx--;
	rbdr = &qs->rbdr[rbdr_idx];
	/* Check if it's enabled */
	if (!rbdr->enable)
		goto next_rbdr;

	/* check if valid descs reached or crossed threshold level */
	qcount = nicvf_queue_reg_read(nic, NIC_QSET_RBDR_0_1_STATUS0, rbdr_idx);
	qcount &= 0x7FFFF;
	if (qcount > rbdr->thresh)
		goto next_rbdr;

	/* Notify HW */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_DOOR,
			      rbdr_idx, rbdr->thresh);
next_rbdr:
	if (rbdr_idx)
		goto refill;
}

/* TBD: how to handle full packets received in CQ
 * i.e conversion of buffers into SKBs
 */
static int nicvf_init_cmp_queue(struct nicvf *nic,
				struct cmp_queue *cq, int q_len)
{
	if (nicvf_alloc_q_desc_mem(nic, &cq->dmem, q_len,
				   CMP_QUEUE_DESC_SIZE,
				   NICVF_CQ_BASE_ALIGN_BYTES)) {
		printf("Unable to allocate memory for completion queue\n");
		return -1;
	}
	cq->desc = cq->dmem.base;
	cq->thresh = CMP_QUEUE_CQE_THRESH;
	cq->intr_timer_thresh = CMP_QUEUE_TIMER_THRESH;

	return 0;
}

static void nicvf_free_cmp_queue(struct nicvf *nic, struct cmp_queue *cq)
{
	if (!cq)
		return;
	if (!cq->dmem.base)
		return;

	nicvf_free_q_desc_mem(nic, &cq->dmem);
}

static int nicvf_init_snd_queue(struct nicvf *nic,
				struct snd_queue *sq, int q_len)
{
	if (nicvf_alloc_q_desc_mem(nic, &sq->dmem, q_len,
				   SND_QUEUE_DESC_SIZE,
				   NICVF_SQ_BASE_ALIGN_BYTES)) {
		printf("Unable to allocate memory for send queue\n");
		return -1;
	}

	sq->desc = sq->dmem.base;
	sq->skbuff = calloc(q_len, sizeof(uint64_t));
	sq->head = 0;
	sq->tail = 0;
	sq->free_cnt = q_len - 1;
	sq->thresh = SND_QUEUE_THRESH;

	return 0;
}

static void nicvf_free_snd_queue(struct nicvf *nic, struct snd_queue *sq)
{
	if (!sq)
		return;
	if (!sq->dmem.base)
		return;

	free(sq->skbuff);
	nicvf_free_q_desc_mem(nic, &sq->dmem);
}

static void nicvf_reclaim_snd_queue(struct nicvf *nic,
				    struct queue_set *qs, int qidx)
{
	/* Disable send queue */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, 0);
	/* Check if SQ is stopped */
	if (nicvf_poll_reg(nic, qidx, NIC_QSET_SQ_0_7_STATUS, 21, 1, 0x01))
		return;
	/* Reset send queue */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, NICVF_SQ_RESET);
}

static void nicvf_reclaim_rcv_queue(struct nicvf *nic,
				    struct queue_set *qs, int qidx)
{
	struct nic_mbx mbx = { };

	/* Make sure all packets in the pipeline are written back into mem */

	mbx.msg = NIC_PF_VF_MSG_RQ_SW_SYNC;
	mbx.data.rq.cfg = 0;
	nicvf_send_msg_to_pf(nic, &mbx);
}

static void nicvf_reclaim_cmp_queue(struct nicvf *nic,
				    struct queue_set *qs, int qidx)
{
	/* Disable timer threshold (doesn't get reset upon CQ reset */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG2, qidx, 0);
	/* Disable completion queue */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, 0);
	/* Reset completion queue */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, NICVF_CQ_RESET);
}

static void nicvf_reclaim_rbdr(struct nicvf *nic, struct rbdr *rbdr, int qidx)
{
	uint64_t tmp;
	int timeout = 10;

	/* Save head and tail pointers for feeing up buffers */
	rbdr->head = nicvf_queue_reg_read(nic,
					  NIC_QSET_RBDR_0_1_HEAD, qidx) >> 3;
	rbdr->tail = nicvf_queue_reg_read(nic,
					  NIC_QSET_RBDR_0_1_TAIL, qidx) >> 3;

	/* Disable RBDR */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG, qidx, 0);
	if (nicvf_poll_reg(nic, qidx, NIC_QSET_RBDR_0_1_STATUS0, 62, 2, 0x00))
		return;
	while (1) {
		tmp = nicvf_queue_reg_read(nic,
					   NIC_QSET_RBDR_0_1_PREFETCH_STATUS,
					   qidx);
		if ((tmp & 0xFFFFFFFF) == ((tmp >> 32) & 0xFFFFFFFF))
			break;
		udelay(2000);
		timeout--;
		if (!timeout) {
			printf("Failed polling on prefetch status\n");
			return;
		}
	}
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG,
			      qidx, NICVF_RBDR_RESET);

	if (nicvf_poll_reg(nic, qidx, NIC_QSET_RBDR_0_1_STATUS0, 62, 2, 0x02))
		return;
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG, qidx, 0x00);
	if (nicvf_poll_reg(nic, qidx, NIC_QSET_RBDR_0_1_STATUS0, 62, 2, 0x00))
		return;
}

static void nicvf_rcv_queue_config(struct nicvf *nic, struct queue_set *qs,
				   int qidx, bool enable)
{
	struct nic_mbx mbx = { };
	struct rcv_queue *rq;

	union {
		uintptr_t u;
		struct rq_cfg s;
	} rq_cfg;

	rq = &qs->rq[qidx];
	rq->enable = enable;

	/* Disable receive queue */
	nicvf_queue_reg_write(nic, NIC_QSET_RQ_0_7_CFG, qidx, 0);

	if (!rq->enable) {
		nicvf_reclaim_rcv_queue(nic, qs, qidx);
		return;
	}

	rq->cq_qs = qs->vnic_id;
	rq->cq_idx = qidx;
	rq->start_rbdr_qs = qs->vnic_id;
	rq->start_qs_rbdr_idx = qs->rbdr_cnt - 1;
	rq->cont_rbdr_qs = qs->vnic_id;
	rq->cont_qs_rbdr_idx = qs->rbdr_cnt - 1;
	rq->caching = 0;

	/* Send a mailbox msg to PF to config RQ */
	mbx.msg = NIC_PF_VF_MSG_RQ_CFG;
	mbx.data.rq.qs_num = qs->vnic_id;
	mbx.data.rq.rq_num = qidx;
	mbx.data.rq.cfg = (rq->caching << 26) | (rq->cq_qs << 19) |
	    (rq->cq_idx << 16) | (rq->cont_rbdr_qs << 9) |
	    (rq->cont_qs_rbdr_idx << 8) |
	    (rq->start_rbdr_qs << 1) | (rq->start_qs_rbdr_idx);
	nicvf_send_msg_to_pf(nic, &mbx);

	mbx.msg = NIC_PF_VF_MSG_RQ_BP_CFG;
	mbx.data.rq.cfg = (1ULL << 63) | (1ULL << 62) | (qs->vnic_id << 0);
	nicvf_send_msg_to_pf(nic, &mbx);

	/* RQ drop config
	 * Enable CQ drop to reserve sufficient CQEs for all tx packets
	 */
	mbx.msg = NIC_PF_VF_MSG_RQ_DROP_CFG;
	mbx.data.rq.cfg = (1ULL << 62) | (RQ_CQ_DROP << 8);
	nicvf_send_msg_to_pf(nic, &mbx);

	nicvf_queue_reg_write(nic, NIC_QSET_RQ_GEN_CFG, qidx, 0x00);

	/* Enable Receive queue */
	rq_cfg.s.ena = 1;
	rq_cfg.s.tcp_ena = 0;

	nicvf_queue_reg_write(nic, NIC_QSET_RQ_0_7_CFG, qidx, rq_cfg.u);
}

void nicvf_cmp_queue_config(struct nicvf *nic, struct queue_set *qs,
			    int qidx, bool enable)
{
	struct cmp_queue *cq;
	union {
		uint64_t u;
		struct cq_cfg s;
	} cq_cfg;

	cq = &qs->cq[qidx];
	cq->enable = enable;

	if (!cq->enable) {
		nicvf_reclaim_cmp_queue(nic, qs, qidx);
		return;
	}

	/* Reset completion queue */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, NICVF_CQ_RESET);

	if (!cq->enable)
		return;

	/* Set completion queue base address */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_BASE,
			      qidx, (uint64_t) (cq->dmem.phys_base));

	/* Enable Completion queue */
	cq_cfg.s.ena = 1;
	cq_cfg.s.reset = 0;
	cq_cfg.s.caching = 0;
	cq_cfg.s.qsize = CMP_QSIZE;
	cq_cfg.s.avg_con = 0;
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx, cq_cfg.u);

	/* Set threshold value for interrupt generation */
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_THRESH, qidx, cq->thresh);
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG2,
			      qidx, cq->intr_timer_thresh);
}

static void nicvf_snd_queue_config(struct nicvf *nic, struct queue_set *qs,
				   int qidx, bool enable)
{
	struct nic_mbx mbx = { };
	struct snd_queue *sq;

	union {
		uint64_t u;
		struct sq_cfg s;
	} sq_cfg;

	sq = &qs->sq[qidx];
	sq->enable = enable;

	if (!sq->enable) {
		nicvf_reclaim_snd_queue(nic, qs, qidx);
		return;
	}

	/* Reset send queue */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, NICVF_SQ_RESET);

	sq->cq_qs = qs->vnic_id;
	sq->cq_idx = qidx;

	/* Send a mailbox msg to PF to config SQ */
	mbx.msg = NIC_PF_VF_MSG_SQ_CFG;
	mbx.data.sq.qs_num = qs->vnic_id;
	mbx.data.sq.sq_num = qidx;
	mbx.data.sq.cfg = (sq->cq_qs << 3) | sq->cq_idx;
	nicvf_send_msg_to_pf(nic, &mbx);

	/* Set queue base address */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_BASE,
			      qidx, (uint64_t) (sq->dmem.phys_base));

	/* Enable send queue  & set queue size */
	sq_cfg.s.ena = 1;
	sq_cfg.s.reset = 0;
	sq_cfg.s.ldwb = 0;
	sq_cfg.s.qsize = SND_QSIZE;
	sq_cfg.s.tstmp_bgx_intf = 0;
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, sq_cfg.u);

	/* Set threshold value for interrupt generation */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_THRESH, qidx, sq->thresh);
}

static void nicvf_rbdr_config(struct nicvf *nic, struct queue_set *qs,
			      int qidx, bool enable)
{
	struct rbdr *rbdr;

	union {
		uint64_t u;
		struct rbdr_cfg s;
	} rbdr_cfg;

	rbdr = &qs->rbdr[qidx];
	nicvf_reclaim_rbdr(nic, rbdr, qidx);
	if (!enable)
		return;

	/* Set descriptor base address */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_BASE,
			      qidx, (uint64_t) (rbdr->dmem.phys_base));

	/* Enable RBDR  & set queue size */
	/* Buffer size should be in multiples of 128 bytes */
	rbdr_cfg.s.ena = 1;
	rbdr_cfg.s.reset = 0;
	rbdr_cfg.s.ldwb = 0;
	rbdr_cfg.s.qsize = RBDR_SIZE;
	rbdr_cfg.s.avg_con = 0;
	rbdr_cfg.s.lines = rbdr->buf_size / 128;
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_CFG, qidx, rbdr_cfg.u);

	/* Notify HW */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_DOOR,
			      qidx, qs->rbdr_len - 1);

	/* Set threshold value for interrupt generation */
	nicvf_queue_reg_write(nic, NIC_QSET_RBDR_0_1_THRESH,
			      qidx, rbdr->thresh - 1);
}

void nicvf_qset_config(struct nicvf *nic, bool enable)
{
	struct nic_mbx mbx = { };
	struct queue_set *qs = nic->qs;
	struct qs_cfg *qs_cfg;

	if (!qs) {
		printf("Qset is still not allocated, don't init queues\n");
		return;
	}

	qs->enable = enable;
	qs->vnic_id = nic->vf_id;

	/* Send a mailbox msg to PF to config Qset */
	mbx.msg = NIC_PF_VF_MSG_QS_CFG;
	mbx.data.qs.num = qs->vnic_id;

	mbx.data.qs.cfg = 0;
	qs_cfg = (struct qs_cfg *)&mbx.data.qs.cfg;
	if (qs->enable) {
		qs_cfg->ena = 1;
#ifdef __BIG_ENDIAN
		qs_cfg->be = 1;
#endif
		qs_cfg->vnic = qs->vnic_id;
	}

	nicvf_send_msg_to_pf(nic, &mbx);
}

static void nicvf_free_resources(struct nicvf *nic)
{
	int qidx;
	struct queue_set *qs = nic->qs;

	/* Free receive buffer descriptor ring */
	for (qidx = 0; qidx < qs->rbdr_cnt; qidx++)
		nicvf_free_rbdr(nic, &qs->rbdr[qidx]);

	/* Free completion queue */
	for (qidx = 0; qidx < qs->cq_cnt; qidx++)
		nicvf_free_cmp_queue(nic, &qs->cq[qidx]);

	/* Free send queue */
	for (qidx = 0; qidx < qs->sq_cnt; qidx++)
		nicvf_free_snd_queue(nic, &qs->sq[qidx]);
}

static int nicvf_alloc_resources(struct nicvf *nic)
{
	int qidx;
	struct queue_set *qs = nic->qs;

	/* Alloc receive buffer descriptor ring */
	for (qidx = 0; qidx < qs->rbdr_cnt; qidx++) {
		if (nicvf_init_rbdr(nic, &qs->rbdr[qidx], qs->rbdr_len,
				    RCV_BUFFER_LEN))
			goto alloc_fail;
	}

	/* Alloc send queue */
	for (qidx = 0; qidx < qs->sq_cnt; qidx++) {
		if (nicvf_init_snd_queue(nic, &qs->sq[qidx], qs->sq_len))
			goto alloc_fail;
	}

	/* Alloc completion queue */
	for (qidx = 0; qidx < qs->cq_cnt; qidx++) {
		if (nicvf_init_cmp_queue(nic, &qs->cq[qidx], qs->cq_len))
			goto alloc_fail;
	}

	return 0;
alloc_fail:
	nicvf_free_resources(nic);
	return -1;
}

int nicvf_set_qset_resources(struct nicvf *nic)
{
	struct queue_set *qs;

	qs = malloc(sizeof(struct queue_set));
	if (!qs)
		return -1;
	nic->qs = qs;

	/* Set count of each queue */
	qs->rbdr_cnt = RBDR_CNT;
	qs->rq_cnt = RCV_QUEUE_CNT;
	qs->sq_cnt = SND_QUEUE_CNT;
	qs->cq_cnt = CMP_QUEUE_CNT;

	/* Set queue lengths */
	qs->rbdr_len = RCV_BUF_COUNT;
	qs->sq_len = SND_QUEUE_LEN;
	qs->cq_len = CMP_QUEUE_LEN;
	return 0;
}

int nicvf_config_data_transfer(struct nicvf *nic, bool enable)
{
	bool disable = false;
	struct queue_set *qs = nic->qs;
	int qidx;

	if (!qs)
		return 0;

	if (enable) {
		if (nicvf_alloc_resources(nic))
			return -1;

		for (qidx = 0; qidx < qs->sq_cnt; qidx++)
			nicvf_snd_queue_config(nic, qs, qidx, enable);
		for (qidx = 0; qidx < qs->cq_cnt; qidx++)
			nicvf_cmp_queue_config(nic, qs, qidx, enable);
		for (qidx = 0; qidx < qs->rbdr_cnt; qidx++)
			nicvf_rbdr_config(nic, qs, qidx, enable);
		for (qidx = 0; qidx < qs->rq_cnt; qidx++)
			nicvf_rcv_queue_config(nic, qs, qidx, enable);
	} else {
		for (qidx = 0; qidx < qs->rq_cnt; qidx++)
			nicvf_rcv_queue_config(nic, qs, qidx, disable);
		for (qidx = 0; qidx < qs->rbdr_cnt; qidx++)
			nicvf_rbdr_config(nic, qs, qidx, disable);
		for (qidx = 0; qidx < qs->sq_cnt; qidx++)
			nicvf_snd_queue_config(nic, qs, qidx, disable);
		for (qidx = 0; qidx < qs->cq_cnt; qidx++)
			nicvf_cmp_queue_config(nic, qs, qidx, disable);

		nicvf_free_resources(nic);
	}

	return 0;
}

/* Get a free desc from send queue
 * @qs:   Qset from which to get a SQ descriptor
 * @qnum: SQ number (0...7) in the Qset
 *
 * returns descriptor ponter & descriptor number
 */
static int nicvf_get_sq_desc(struct queue_set *qs, int qnum, void **desc)
{
	int qentry;
	struct snd_queue *sq = &qs->sq[qnum];

	if (!sq->free_cnt)
		return 0;

	qentry = sq->tail++;
	sq->free_cnt--;

	sq->tail &= (sq->dmem.q_len - 1);
	*desc = GET_SQ_DESC(sq, qentry);
	return qentry;
}

void nicvf_put_sq_desc(struct snd_queue *sq, int desc_cnt)
{
	while (desc_cnt--) {
		sq->free_cnt++;
		sq->head++;
		sq->head &= (sq->dmem.q_len - 1);
	}
}

void nicvf_sq_enable(struct nicvf *nic, struct snd_queue *sq, int qidx)
{
	uint64_t sq_cfg;

	sq_cfg = nicvf_queue_reg_read(nic, NIC_QSET_SQ_0_7_CFG, qidx);
	sq_cfg |= NICVF_SQ_EN;
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, sq_cfg);
	/* Ring doorbell so that H/W restarts processing SQEs */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_DOOR, qidx, 0);
}

void nicvf_sq_disable(struct nicvf *nic, int qidx)
{
	uint64_t sq_cfg;

	sq_cfg = nicvf_queue_reg_read(nic, NIC_QSET_SQ_0_7_CFG, qidx);
	sq_cfg &= ~NICVF_SQ_EN;
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_CFG, qidx, sq_cfg);
}

void nicvf_sq_free_used_descs(struct eth_device *netdev, struct snd_queue *sq,
			      int qidx)
{
	uint64_t head;
	void *pkt;
	struct nicvf *nic = netdev->priv;
	struct sq_hdr_subdesc *hdr;

	head = nicvf_queue_reg_read(nic, NIC_QSET_SQ_0_7_HEAD, qidx) >> 4;

	while (sq->head != head) {
		hdr = (struct sq_hdr_subdesc *)GET_SQ_DESC(sq, sq->head);
		if (hdr->subdesc_type != SQ_DESC_TYPE_HEADER) {
			nicvf_put_sq_desc(sq, 1);
			continue;
		}
		pkt = (void *)sq->skbuff[sq->head];
		free(pkt);
		nicvf_put_sq_desc(sq, hdr->subdesc_cnt + 1);
	}
}

/* Add SQ HEADER subdescriptor.
 * First subdescriptor for every send descriptor.
 */
struct sq_hdr_subdesc *nicvf_sq_add_hdr_subdesc(struct queue_set *qs,
						int sq_num, int subdesc_cnt,
						void *pkt, size_t pkt_len)
{
	int qentry;
	void *desc;
	struct snd_queue *sq;
	struct sq_hdr_subdesc *hdr;

	sq = &qs->sq[sq_num];
	qentry = nicvf_get_sq_desc(qs, sq_num, &desc);
	sq->skbuff[qentry] = (uintptr_t) pkt;

	hdr = (struct sq_hdr_subdesc *)desc;

	memset(hdr, 0, SND_QUEUE_DESC_SIZE);
	hdr->subdesc_type = SQ_DESC_TYPE_HEADER;
	hdr->post_cqe = 1;
	hdr->subdesc_cnt = subdesc_cnt;
	hdr->tot_len = pkt_len;

	return hdr;
}

/* SQ GATHER subdescriptor
 * Must follow HDR descriptor
 */
static void nicvf_sq_add_gather_subdesc(struct nicvf *nic, struct queue_set *qs,
					int sq_num, void *pkt, size_t pkt_len)
{
	void *desc;
	struct sq_gather_subdesc *gather;

	nicvf_get_sq_desc(qs, sq_num, &desc);
	gather = (struct sq_gather_subdesc *)desc;

	memset(gather, 0, SND_QUEUE_DESC_SIZE);
	gather->subdesc_type = SQ_DESC_TYPE_GATHER;
	gather->ld_type = NIC_SEND_LD_TYPE_E_LDD;
	gather->size = pkt_len;
	gather->addr = (uintptr_t) pkt;

	flush_dcache_range(gather->addr, gather->addr + gather->size);
}

/* Append an skb to a SQ for packet transfer. */
int nicvf_sq_append_pkt(struct nicvf *nic, void *pkt, size_t pkt_size)
{
	int subdesc_cnt;
	int sq_num;
	struct queue_set *qs = nic->qs;
	struct snd_queue *sq;

	sq_num = 0;
	sq = &qs->sq[sq_num];

	subdesc_cnt = 2;
	if (subdesc_cnt > sq->free_cnt)
		goto append_fail;

	/* Add SQ header subdesc */
	nicvf_sq_add_hdr_subdesc(qs, sq_num, subdesc_cnt - 1, pkt, pkt_size);

	/* Add SQ gather subdesc */
	nicvf_sq_add_gather_subdesc(nic, qs, sq_num, pkt, pkt_size);

	/* make sure all memory stores are done before ringing doorbell */
	asm volatile ("dsb sy");

	/* Inform HW to xmit new packet */
	nicvf_queue_reg_write(nic, NIC_QSET_SQ_0_7_DOOR, sq_num, subdesc_cnt);
	return 1;

append_fail:
	printf("Not enough SQ descriptors to xmit pkt\n");
	return 0;
}

static unsigned frag_num(unsigned i)
{
#ifdef __BIG_ENDIAN
	return (i & ~3) + 3 - (i & 3);
#else
	return i;
#endif
}

void *nicvf_get_rcv_pkt(struct nicvf *nic, void *cq_desc, size_t *pkt_len)
{
	int frag;
	int payload_len = 0;
	void *pkt = NULL;
	struct cqe_rx_t *cqe_rx;
	struct rbdr *rbdr;
	struct rcv_queue *rq;
	struct queue_set *qs = nic->qs;
	uint16_t *rb_lens = NULL;
	uint64_t *rb_ptrs = NULL;

	cqe_rx = (struct cqe_rx_t *)cq_desc;

	rq = &qs->rq[cqe_rx->rq_idx];
	rbdr = &qs->rbdr[rq->start_qs_rbdr_idx];
	rb_lens = cq_desc + (3 * sizeof(uint64_t));	/* Use offsetof */
	rb_ptrs = cq_desc + (6 * sizeof(uint64_t));

	debug("%s rb_cnt %d rb0_ptr %llx rb0_sz %d\n",
	      __func__, cqe_rx->rb_cnt, cqe_rx->rb0_ptr, cqe_rx->rb0_sz);

	for (frag = 0; frag < cqe_rx->rb_cnt; frag++) {
		payload_len = rb_lens[frag_num(frag)];

		invalidate_dcache_range((uintptr_t) (*rb_ptrs),
					(uintptr_t) (*rb_ptrs) +
					rbdr->buf_size);

		/* First fragment */
		*rb_ptrs = *rb_ptrs - cqe_rx->align_pad;
		pkt = nicvf_rb_ptr_to_pkt(nic, *rb_ptrs);

		invalidate_dcache_range((uintptr_t) pkt,
					(uintptr_t) pkt + payload_len);

		if (cqe_rx->align_pad)
			pkt += cqe_rx->align_pad;

		/* Next buffer pointer */
		rb_ptrs++;

		*pkt_len = payload_len;
	}
	return pkt;
}

/* Clear interrupt */
void nicvf_clear_intr(struct nicvf *nic, int int_type, int q_idx)
{
	uint64_t reg_val = 0;

	switch (int_type) {
	case NICVF_INTR_CQ:
		reg_val = ((1ULL << q_idx) << NICVF_INTR_CQ_SHIFT);
		break;
	case NICVF_INTR_SQ:
		reg_val = ((1ULL << q_idx) << NICVF_INTR_SQ_SHIFT);
		break;
	case NICVF_INTR_RBDR:
		reg_val = ((1ULL << q_idx) << NICVF_INTR_RBDR_SHIFT);
		break;
	case NICVF_INTR_PKT_DROP:
		reg_val = (1ULL << NICVF_INTR_PKT_DROP_SHIFT);
		break;
	case NICVF_INTR_TCP_TIMER:
		reg_val = (1ULL << NICVF_INTR_TCP_TIMER_SHIFT);
		break;
	case NICVF_INTR_MBOX:
		reg_val = (1ULL << NICVF_INTR_MBOX_SHIFT);
		break;
	case NICVF_INTR_QS_ERR:
		reg_val |= (1ULL << NICVF_INTR_QS_ERR_SHIFT);
		break;
	default:
		printf("Failed to clear interrupt: unknown type\n");
		break;
	}

	nicvf_reg_write(nic, NIC_VF_INT, reg_val);
}

void nicvf_update_rq_stats(struct nicvf *nic, int rq_idx)
{
	struct rcv_queue *rq;

#define GET_RQ_STATS(reg) \
	nicvf_reg_read(nic, NIC_QSET_RQ_0_7_STAT_0_1 |\
			    (rq_idx << NIC_Q_NUM_SHIFT) | (reg << 3))

	rq = &nic->qs->rq[rq_idx];
	rq->stats.bytes = GET_RQ_STATS(RQ_SQ_STATS_OCTS);
	rq->stats.pkts = GET_RQ_STATS(RQ_SQ_STATS_PKTS);
}

void nicvf_update_sq_stats(struct nicvf *nic, int sq_idx)
{
	struct snd_queue *sq;

#define GET_SQ_STATS(reg) \
	nicvf_reg_read(nic, NIC_QSET_SQ_0_7_STAT_0_1 |\
			    (sq_idx << NIC_Q_NUM_SHIFT) | (reg << 3))

	sq = &nic->qs->sq[sq_idx];
	sq->stats.bytes = GET_SQ_STATS(RQ_SQ_STATS_OCTS);
	sq->stats.pkts = GET_SQ_STATS(RQ_SQ_STATS_PKTS);
}

/* Check for errors in the receive cmp.queue entry */
int nicvf_check_cqe_rx_errs(struct nicvf *nic,
			    struct cmp_queue *cq, void *cq_desc)
{
	struct cqe_rx_t *cqe_rx;
	struct cmp_queue_stats *stats = &cq->stats;

	cqe_rx = (struct cqe_rx_t *)cq_desc;
	if (!cqe_rx->err_level && !cqe_rx->err_opcode) {
		stats->rx.errop.good++;
		return 0;
	}

	switch (cqe_rx->err_level) {
	case CQ_ERRLVL_MAC:
		stats->rx.errlvl.mac_errs++;
		break;
	case CQ_ERRLVL_L2:
		stats->rx.errlvl.l2_errs++;
		break;
	case CQ_ERRLVL_L3:
		stats->rx.errlvl.l3_errs++;
		break;
	case CQ_ERRLVL_L4:
		stats->rx.errlvl.l4_errs++;
		break;
	}

	switch (cqe_rx->err_opcode) {
	case CQ_RX_ERROP_RE_PARTIAL:
		stats->rx.errop.partial_pkts++;
		break;
	case CQ_RX_ERROP_RE_JABBER:
		stats->rx.errop.jabber_errs++;
		break;
	case CQ_RX_ERROP_RE_FCS:
		stats->rx.errop.fcs_errs++;
		break;
	case CQ_RX_ERROP_RE_TERMINATE:
		stats->rx.errop.terminate_errs++;
		break;
	case CQ_RX_ERROP_RE_RX_CTL:
		stats->rx.errop.bgx_rx_errs++;
		break;
	case CQ_RX_ERROP_PREL2_ERR:
		stats->rx.errop.prel2_errs++;
		break;
	case CQ_RX_ERROP_L2_FRAGMENT:
		stats->rx.errop.l2_frags++;
		break;
	case CQ_RX_ERROP_L2_OVERRUN:
		stats->rx.errop.l2_overruns++;
		break;
	case CQ_RX_ERROP_L2_PFCS:
		stats->rx.errop.l2_pfcs++;
		break;
	case CQ_RX_ERROP_L2_PUNY:
		stats->rx.errop.l2_puny++;
		break;
	case CQ_RX_ERROP_L2_MAL:
		stats->rx.errop.l2_hdr_malformed++;
		break;
	case CQ_RX_ERROP_L2_OVERSIZE:
		stats->rx.errop.l2_oversize++;
		break;
	case CQ_RX_ERROP_L2_UNDERSIZE:
		stats->rx.errop.l2_undersize++;
		break;
	case CQ_RX_ERROP_L2_LENMISM:
		stats->rx.errop.l2_len_mismatch++;
		break;
	case CQ_RX_ERROP_L2_PCLP:
		stats->rx.errop.l2_pclp++;
		break;
	case CQ_RX_ERROP_IP_NOT:
		stats->rx.errop.non_ip++;
		break;
	case CQ_RX_ERROP_IP_CSUM_ERR:
		stats->rx.errop.ip_csum_err++;
		break;
	case CQ_RX_ERROP_IP_MAL:
		stats->rx.errop.ip_hdr_malformed++;
		break;
	case CQ_RX_ERROP_IP_MALD:
		stats->rx.errop.ip_payload_malformed++;
		break;
	case CQ_RX_ERROP_IP_HOP:
		stats->rx.errop.ip_hop_errs++;
		break;
	case CQ_RX_ERROP_L3_ICRC:
		stats->rx.errop.l3_icrc_errs++;
		break;
	case CQ_RX_ERROP_L3_PCLP:
		stats->rx.errop.l3_pclp++;
		break;
	case CQ_RX_ERROP_L4_MAL:
		stats->rx.errop.l4_malformed++;
		break;
	case CQ_RX_ERROP_L4_CHK:
		stats->rx.errop.l4_csum_errs++;
		break;
	case CQ_RX_ERROP_UDP_LEN:
		stats->rx.errop.udp_len_err++;
		break;
	case CQ_RX_ERROP_L4_PORT:
		stats->rx.errop.bad_l4_port++;
		break;
	case CQ_RX_ERROP_TCP_FLAG:
		stats->rx.errop.bad_tcp_flag++;
		break;
	case CQ_RX_ERROP_TCP_OFFSET:
		stats->rx.errop.tcp_offset_errs++;
		break;
	case CQ_RX_ERROP_L4_PCLP:
		stats->rx.errop.l4_pclp++;
		break;
	case CQ_RX_ERROP_RBDR_TRUNC:
		stats->rx.errop.pkt_truncated++;
		break;
	}

	return 1;
}

/* Check for errors in the send cmp.queue entry */
int nicvf_check_cqe_tx_errs(struct nicvf *nic,
			    struct cmp_queue *cq, void *cq_desc)
{
	struct cqe_send_t *cqe_tx;
	struct cmp_queue_stats *stats = &cq->stats;

	cqe_tx = (struct cqe_send_t *)cq_desc;
	switch (cqe_tx->send_status) {
	case CQ_TX_ERROP_GOOD:
		stats->tx.good++;
		return 0;
		break;
	case CQ_TX_ERROP_DESC_FAULT:
		stats->tx.desc_fault++;
		break;
	case CQ_TX_ERROP_HDR_CONS_ERR:
		stats->tx.hdr_cons_err++;
		break;
	case CQ_TX_ERROP_SUBDC_ERR:
		stats->tx.subdesc_err++;
		break;
	case CQ_TX_ERROP_IMM_SIZE_OFLOW:
		stats->tx.imm_size_oflow++;
		break;
	case CQ_TX_ERROP_DATA_SEQUENCE_ERR:
		stats->tx.data_seq_err++;
		break;
	case CQ_TX_ERROP_MEM_SEQUENCE_ERR:
		stats->tx.mem_seq_err++;
		break;
	case CQ_TX_ERROP_LOCK_VIOL:
		stats->tx.lock_viol++;
		break;
	case CQ_TX_ERROP_DATA_FAULT:
		stats->tx.data_fault++;
		break;
	case CQ_TX_ERROP_TSTMP_CONFLICT:
		stats->tx.tstmp_conflict++;
		break;
	case CQ_TX_ERROP_TSTMP_TIMEOUT:
		stats->tx.tstmp_timeout++;
		break;
	case CQ_TX_ERROP_MEM_FAULT:
		stats->tx.mem_fault++;
		break;
	case CQ_TX_ERROP_CK_OVERLAP:
		stats->tx.csum_overlap++;
		break;
	case CQ_TX_ERROP_CK_OFLOW:
		stats->tx.csum_overflow++;
		break;
	}

	return 1;
}
