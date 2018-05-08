/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <pci.h>
#include <memalign.h>
#include <watchdog.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/types.h>
#include <asm/arch/octeontx2.h>
#include "cavm-csrs-nix.h"
#include "cavm-csrs-npa.h"
#include "cavm-csrs-lmt.h"
#include "rvu_common.h"
#include "nix.h"
#include "lmt.h"
#include "cgx.h"

#define CAVM_NUMA_MAX_NODES	2	/** TODO: Move this elsewhere */
#define CAVM_MAX_GATHER		1	/** Maximum scatter/gather */

/** Offset from RVU PFVF BAR 2 */
#define CAVM_LMT_LMTLINE(x)		((x) * 0x8)
#define CAVM_LMT_LF_LMTCANCEL		(0x400)

static const int USE_SSO = 0;	/** Do not use SSO, use completion queues */
static const int MAX_MTU = 9212;/** Maximum packet size */
static const int MAX_CQS = 32;	/** Maximum of 32 completion queues */
static const int MAX_SQS = 32;	/** Maximum of 32 send queues */
static const int MAX_RQS = 32;	/** Maximum of 32 receive queues */
/** Size of RSS table (256) See NIX_AF_LFX_RSS_CFG[size] */
static const int RSS_SIZE = 0;
/** Each completion queue contains 256 entries, see NIC_CQ_CTX_S[qsize] */
static const unsigned int CQS_QSIZE = 2;
/** Number of CQ entries */
static const unsigned int CQ_ENTRIES = 16 << (2 /*CQS_QSIZE*/ * 2);
static const int AQ_RING_SIZE = 16 << (AQ_SIZE * 2);
/**
 * Each completion queue entry contains 512 bytes, see
 * NIXX_AF_LFX_CFG[xqe_size]
 */
static const int CQ_ENTRY_SIZE = 512;

struct nix_node_state {
	int next_free_lf;
	int next_free_sq;
	int next_free_rq;
	int next_free_cq;
	int next_free_rssi;
	int next_free_bpid;
};

/* Globals */
static struct nix_node_state global_node_state[CAVM_NUMA_MAX_NODES];
static const struct pci_device_id npc_devid = {

};

#if 0
static u64 npc_reg_read(struct nix_handle *nix, u64 offset)
{
	return readq(nix->npc_base + offset);
}

static void npc_reg_write(struct nix_handle *nix, u64 offset, u64 val)
{
	writeq(val, nix->npc_base + offset);
}
#endif

/**
 * NIX needs a lot of memory areas. Rather than handle all the failure cases,
 * we'll use a wrapper around alloc that prints an error if a memory
 * allocation fails.
 *
 * @param num_elements
 *                  Number of elements to allocate
 * @param elem_size Size of each element
 * @param msge      Text string to show when allocation fails
 *
 * @return A valid memory location or NULL on failure
 */
static void *nix_memalloc(int num_elements, size_t elem_size, const char *msg)
{
	size_t alloc_size = num_elements * elem_size;
	void *base = memalign(CONFIG_SYS_CACHELINE_SIZE, alloc_size);

	if (!base)
		printf("NIX: Memory alloc failed for %s (%d * %zu = %zu bytes)\n",
		       msg ? msg : __func__, num_elements, elem_size,
		       alloc_size);
	else
		memset(base, 0, alloc_size);

	return base;
}

static int npa_setup_pool(struct nix_handle *nix, u32 pool_id,
			  size_t buffer_size, u32 queue_length, void *buffers[])
{
	struct {
		union cavm_npa_lf_aura_op_free0 f0;
		union cavm_npa_lf_aura_op_free1 f1;
	} aura_descr;
	int index;

	for (index = 0; index < queue_length; index++) {
		buffers[index] = memalign(CONFIG_SYS_CACHELINE_SIZE,
					  buffer_size);
		if (!buffers[index]) {
			printf("%s: Out of memory allocating buffer %d, size: %zu\n",
			       __func__, index, buffer_size);
			return -ENOMEM;
		}

		/* Add the newly obtained pointer to the pool.  128 bit
		 * writes only.
		 */
		aura_descr.f0.s.addr = (u64)buffers[index];
		aura_descr.f1.u = 0;
		aura_descr.f1.s.fabs = 1;
		aura_descr.f1.s.aura = pool_id;
		cavm_st128(nix->npa->npa_base + CAVM_NPA_LF_AURA_OP_FREE0(),
			   aura_descr.f0.u, aura_descr.f1.u);
	}

	return 0;
}

static int npa_lf_setup(struct nix_handle *nix)
{
	struct npa_handle *npa = nix->npa;
	struct nix_af_handle *nix_af = nix->nix_af;
	union cavm_npa_aura_s aura_ctx[NPA_POOL_COUNT];
	union cavm_npa_pool_s pool_ctx[NPA_POOL_COUNT];
	union cavm_npa_af_const npa_af_const;
	int pool;
	int queue_len[NPA_POOL_COUNT];
	int buffer_size[NPA_POOL_COUNT];
	int stack_page_pointers;
	int stack_page_bytes;
	int err;
	int lf = 0;

	npa->aura_ctx = memalign(CONFIG_SYS_CACHELINE_SIZE,
				 NPA_AURA_HW_CTX_SIZE * NPA_POOL_COUNT);
	if (!npa->aura_ctx) {
		printf("%s: Out of memory for aura context\n", __func__);
		return -ENOMEM;
	}
	npa_af_const.u = npa_af_reg_read(nix->nix_af->npa_af,
					 CAVM_NPA_AF_CONST());
	stack_page_pointers = npa_af_const.s.stack_page_ptrs;
	stack_page_bytes = npa_af_const.s.stack_page_bytes;
	npa->rx_pool_stack_pages = (RQ_QLEN + stack_page_pointers - 1) /
							stack_page_pointers;
	npa->tx_pool_stack_pages = (SQ_QLEN + stack_page_pointers - 1) /
							stack_page_pointers;

	npa->pool_stack_pointers = stack_page_pointers;

	npa->q_len[NPA_POOL_RX] = RQ_QLEN;
	npa->q_len[NPA_POOL_TX] = SQ_QLEN;

	npa->buf_size[NPA_POOL_RX] = NIX_MAX_HW_MTU + CONFIG_SYS_CACHELINE_SIZE;
	npa->buf_size[NPA_POOL_TX] = NIX_MAX_HW_MTU + CONFIG_SYS_CACHELINE_SIZE;

	npa->stack_pages[NPA_POOL_RX] = npa->rx_pool_stack_pages;
	npa->stack_pages[NPA_POOL_TX] = npa->tx_pool_stack_pages;

	for (pool = 0; pool < NPA_POOL_COUNT; pool++) {
		npa->pool_ctx[pool] = memalign(CONFIG_SYS_CACHELINE_SIZE,
					       npa->stack_pages[pool] *
					       sizeof(union cavm_npa_pool_s));
		if (!npa->pool_ctx[pool]) {
			printf("%s: Out of memory for pool context\n",
			       __func__);
			return -ENOMEM;
		}
		npa->pool_stack[pool] = memalign(CONFIG_SYS_CACHELINE_SIZE,
						 npa->stack_pages[pool] *
						 stack_page_bytes);
		if (!npa->pool_stack[pool]){
			printf("%s: Out of memory for pool stack\n", __func__);
			return -ENOMEM;
		}
	}
	/* Set up the auras */
	for (pool = 0; pool < NPA_POOL_COUNT; pool++) {
		union cavm_npa_aura_s *aura = &aura_ctx[pool];
		union cavm_npa_pool_s *poo = &pool_ctx[pool];
		memset(aura, 0, sizeof(union cavm_npa_aura_s));
		aura->s.fc_ena = 0;
		aura->s.pool_addr = (u64)&npa->pool_ctx[pool];
		aura->s.shift = 64 - __builtin_clzll(npa->q_len[pool]) - 8;
		aura->s.count = npa->q_len[pool];
		aura->s.limit = npa->q_len[pool];
		aura->s.ena = 1;

		memset(poo, 0, sizeof(*poo));
		poo->s.fc_ena = 0;
		poo->s.stack_base = (u64)(&npa->pool_stack[pool]);
		poo->s.buf_size = npa->buf_size[pool];
		poo->s.stack_max_pages = npa->stack_pages[pool];
		poo->s.shift =
			64 - __builtin_clzll(npa->pool_stack_pointers) - 8;
		poo->s.ptr_start = 0;
		poo->s.ptr_end = (1ULL << 40) -  1;
		poo->s.ena = 1;
	}

	err = npa_lf_admin_setup(nix_af, lf, NPA_AURA_SIZE_DEFAULT,
				 aura_ctx, (dma_addr_t)&(npa->aura_ctx),
				 pool_ctx, NPA_POOL_COUNT);
	if (err) {
		printf("%s: Error setting up NPA LF admin for lf %d\n",
		       __func__, lf);
		return err;
	}

	npa->rx_buffers = calloc(queue_len[NPA_POOL_RX], sizeof(void *));
	if (!npa->rx_buffers) {
		printf("%s: Out of memory\n", __func__);
		return -ENOMEM;
	}

	npa->tx_buffers = calloc(queue_len[NPA_POOL_TX], sizeof(void *));
	if (!npa->tx_buffers) {
		printf("%s: Out of memory\n", __func__);
		return -ENOMEM;
	}

	for (pool = 0; pool < NPA_POOL_COUNT; pool++) {
		err = npa_setup_pool(nix, pool, buffer_size[pool],
				     queue_len[pool], pool == NPA_POOL_RX ?
				     npa->rx_buffers : npa->tx_buffers);
		if (err) {
			printf("%s: Error setting up pool %d\n",
			       __func__, pool);
			return err;
		}
	}
	return 0;
}

int npa_lf_shutdown(struct nix_handle *nix)
{
	struct npa_handle *npa = nix->npa;
	int err;
	int pool;

	err = npa_lf_admin_shutdown(nix->nix_af, nix->lf, NPA_POOL_COUNT);
	if (err) {
		printf("%s: Error %d shutting down NPA LF admin\n",
		       __func__, err);
		return err;
	}
	free(npa->aura_ctx);
	npa->aura_ctx = NULL;

	for (pool = 0; pool < NPA_POOL_COUNT; pool++) {
		free(npa->pool_ctx[pool]);
		npa->pool_ctx[pool] = NULL;
		free(npa->pool_stack[pool]);
		npa->pool_stack[pool] = NULL;
	}

	free(npa->rx_buffers);
	npa->rx_buffers = NULL;
	free(npa->tx_buffers);
	npa->tx_buffers = NULL;

	return 0;
}

int nix_rx_tx_iface_setup(struct nix_handle *nix)
{
	union cavm_nixx_af_rx_linkx_cfg link_cfg;

	link_cfg.u = 0;
	link_cfg.s.maxlen = NIX_MAX_HW_MTU;
	link_cfg.s.minlen = NIX_MIN_HW_MTU;
	nix_af_reg_write(nix->nix_af,
			 CAVM_NIXX_AF_RX_LINKX_CFG(nix->lmac->lmac_id),
			 link_cfg.u);

	return 0;
}

int nix_lf_setup(struct nix_handle *nix)
{
	union cavm_nix_rq_ctx_s rq;
	union cavm_nix_sq_ctx_s sq;
	union cavm_nix_cq_ctx_s cq[NIX_CQ_COUNT];
	int index;
	int err;
	bool admin_setup = false;

	nix->rq_ctx_base = memalign(CONFIG_SYS_CACHELINE_SIZE,
				    sizeof(union cavm_nix_rq_ctx_hw_s));
	if (!nix->rq_ctx_base) {
		printf("%s: Out of memory\n", __func__);
		return -ENOMEM;
	}
	memset(nix->rq_ctx_base, 0, sizeof(union cavm_nix_rq_ctx_hw_s));

	nix->sq_ctx_base = memalign(CONFIG_SYS_CACHELINE_SIZE,
				    sizeof(union cavm_nix_sq_ctx_hw_s));
	if (!nix->sq_ctx_base) {
		printf("%s: Out of memory\n", __func__);
		err = -ENOMEM;
		goto error;
	}
	memset(nix->sq_ctx_base, 0, sizeof(union cavm_nix_sq_ctx_hw_s));

	nix->cq_ctx_base = memalign(CONFIG_SYS_CACHELINE_SIZE,
				sizeof(union cavm_nix_cq_ctx_s) * NIX_CQ_COUNT);
	if (!nix->cq_ctx_base) {
		printf("%s: Out of memory\n", __func__);
		err = -ENOMEM;
		goto error;
	}
	memset(nix->cq_ctx_base, 0,
	       sizeof(union cavm_nix_cq_ctx_s) * NIX_CQ_COUNT);

	for (index = 0; index < NIX_CQ_COUNT; index++) {
		err = qmem_alloc(&nix->cq[index], NIX_CQE_SIZE_W64,
				 Q_COUNT(Q_SIZE_256));
		if (err) {
			printf("%s: Error allocating completion queue\n",
			       __func__);
			goto error;
		}
	}

	for (index = 0; index < NIX_CQ_COUNT; index++) {
		memset(&cq[index], 0, sizeof(union cavm_nix_cq_ctx_s));

		cq[index].s.qsize = Q_SIZE_256;
		cq[index].s.ena = 1;
		cq[index].s.caching = 1;
		cq[index].s.base = nix->cq[index].iova;
		cq[index].s.cint_idx = 0;
	}

	memset(&sq, 0, sizeof(union cavm_nix_sq_ctx_s));
	sq.s.cq = NIX_CQ_TX;
	sq.s.max_sqe_size = CAVM_NIX_MAXSQESZ_E_W16;
	sq.s.cq_ena = 1;
	sq.s.ena = 1;
	sq.s.sqb_aura = NPA_POOL_TX;
	sq.s.sqe_stype = CAVM_NIX_STYPE_E_STF;
	sq.s.default_chan = nix->lmac->lmac_id;

	err = nix_lf_admin_setup(nix->nix_af, nix->lf, nix->pf,
				 cq, (dma_addr_t)nix->cq_ctx_base, NIX_CQ_COUNT,
				 &rq, (dma_addr_t)nix->rq_ctx_base, 1,
				 &sq, (dma_addr_t)nix->sq_ctx_base, 1);
	if (err) {
		printf("%s: Error setting up LF\n", __func__);
		goto error;
	}
	admin_setup = true;

	memset(nix->send_descriptors, 0, sizeof(nix->send_descriptors));
	for (index = 0; index < SQ_QLEN; index++) {
		nix->send_descriptors[index].hdr.s.sqe_id = index;
		nix->free_send_descriptors[index] =
			&nix->send_descriptors[index];
	}

	nix->current_free_send_descriptor = 0;

	return 0;
error:
	if (admin_setup)
		nix_lf_admin_shutdown(nix->nix_af, nix->lf, NIX_CQ_COUNT, 1, 1);

	if (nix->rq_ctx_base)
		free(nix->rq_ctx_base);
	nix->rq_ctx_base = NULL;
	if (nix->rq_ctx_base)
		free(nix->rq_ctx_base);
	nix->rq_ctx_base = NULL;
	if (nix->sq_ctx_base)
		free(nix->sq_ctx_base);
	nix->sq_ctx_base = NULL;
	if (nix->cq_ctx_base)
		free(nix->cq_ctx_base);
	nix->cq_ctx_base = NULL;

	for (index = 0; index < NIX_CQ_COUNT; index++)
		qmem_free(&nix->cq[index]);

	return err;
}

int nix_lf_shutdown(struct nix_handle *nix)
{
	struct nix_af_handle *nix_af = nix->nix_af;
	int index;
	int err;

	err = nix_lf_admin_shutdown(nix_af, nix->lf, NIX_CQ_COUNT, 1, 1);
	if (err) {
		printf("%s: Error shutting down LF admin\n", __func__);
		return err;
	}

	if (nix->rq_ctx_base)
		free(nix->rq_ctx_base);
	nix->rq_ctx_base = NULL;
	if (nix->rq_ctx_base)
		free(nix->rq_ctx_base);
	nix->rq_ctx_base = NULL;
	if (nix->sq_ctx_base)
		free(nix->sq_ctx_base);
	nix->sq_ctx_base = NULL;
	if (nix->cq_ctx_base)
		free(nix->cq_ctx_base);
	nix->cq_ctx_base = NULL;

	for (index = 0; index < NIX_CQ_COUNT; index++)
		qmem_free(&nix->cq[index]);

	return 0;
}

struct nix_tx_descr *nix_alloc_send_descriptor(struct nix_handle *nix)
{
	if (nix->current_free_send_descriptor == SQ_QLEN)
		return NULL;

	return nix->free_send_descriptors[nix->current_free_send_descriptor++];
}

void nix_free_send_descriptor(struct nix_handle *nix,
			      struct nix_tx_descr *tx_descr)
{
	nix->free_send_descriptors[nix->current_free_send_descriptor] =
		tx_descr;
}

static inline void nix_write_lmt(struct nix_handle *nix, void *buffer,
				 int num_words)
{
	int i;
	u64 *ptr = buffer;

	for (i = 0; i < num_words; i++)
		writeq(ptr[i], nix->lmt_base + CAVM_LMT_LF_LMTLINEX(i));
}

static int nix_xmit(struct eth_device *netdev, void *pkt, int pkt_len)
{
	struct nix_handle *nix = netdev->priv;
	struct nix_tx_descr *tx_descr;
	const int descr_size = (sizeof(struct nix_tx_descr) + 15) / 16 - 1;
	s64 result;

	tx_descr = nix_alloc_send_descriptor(nix);
	if (!tx_descr) {
		printf("%s: Error: out of tx descriptors\n", __func__);
		return -1;
	}
	tx_descr->hdr.s.aura = 0xa5a5;
	tx_descr->hdr.s.df = 1;
	tx_descr->hdr.s.pnc = 1;
	tx_descr->hdr.s.sq = 0;
	tx_descr->hdr.s.total = pkt_len;
	tx_descr->hdr.s.sizem1 = descr_size;
	tx_descr->segments.s.segs = 1;
	tx_descr->segments.s.subdc = CAVM_NIX_SUBDC_E_SG;
	tx_descr->segments.s.seg1_size = pkt_len;
	tx_descr->segments.s.ld_type = CAVM_NIX_SENDLDTYPE_E_LDT;
	tx_descr->dev_addr = (dma_addr_t)pkt;
	tx_descr->host_addr = pkt;

	do {
		nix_write_lmt(nix, tx_descr, sizeof(*tx_descr) / sizeof(u64));
		__iowmb();
		result = cavm_lmt_submit((u64)(nix->nix_base +
					       CAVM_NIXX_LF_OP_SENDX(0)));
		WATCHDOG_RESET();
	} while (result == 0);

	return 0;
}

int nix_get_pf_num(const struct nix_handle *nix)
{
	return (((u64)(nix->nix_base)) >> 36) & 0x0f;
}

int nix_linear_link_number(const struct nix_handle *nix)
{
	return nix_get_pf_num(nix) - 1;
}

int npc_lf_setup(struct nix_handle *nix)
{
	struct nix_af_handle *nix_af = nix->nix_af;
	int link_num = nix_linear_link_number(nix);
	int err;

	err = npc_lf_admin_setup(nix_af, nix->lmac->cgx, link_num);
	if (err) {
		printf("%s: Error setting up npc lf admin\n", __func__);
		return err;
	}

	return 0;
}

int nix_rx_tx_completion(struct nix_handle *nix, uint queue_idx,
			 u32 *completion_type)
{
	union cavm_nixx_lf_cq_op_status op_status;
	union cavm_nix_cqe_hdr_s *completion;
	u32 head, tail;

	op_status.u =
		cavm_atomic_fetch_and_add64_nosync(nix->nix_base +
						   CAVM_NIXX_LF_CQ_OP_STATUS(),
						   (u64)queue_idx << 32);
	head = op_status.s.head;
	tail = op_status.s.tail;
	if (head != tail) {
		head &= (nix->cq[queue_idx].qsize - 1);
		tail &= (nix->cq[queue_idx].qsize - 1);
		completion = (union cavm_nix_cqe_hdr_s *)
					(nix->cq[queue_idx].base) + head;
		debug("%s: completion: %p (%d)\n", __func__, completion,
		      completion->s.cqe_type);
		*completion_type = completion->s.cqe_type;
	}
	return tail > head ?
	       tail - head : (nix->cq[queue_idx].qsize - head) + tail;
}

void *nix_dequeue_tx_packet(struct nix_handle *nix)
{
	u32 head, tail;
	union cavm_nixx_lf_cq_op_status op_status;
	union cavm_nix_cqe_hdr_s *completion;
	union cavm_nix_send_comp_s *send_comp;
	struct nix_tx_descr *tx_descr;
	void *packet = NULL;

	op_status.u =
		cavm_atomic_fetch_and_add64_nosync(nix->nix_base +
						   CAVM_NIXX_LF_CQ_OP_STATUS(),
						   NIX_CQ_TX << 32);
	head = op_status.s.head;
	tail = op_status.s.tail;

	if (head == tail)
		return NULL;

	head &= (nix->cq[NIX_CQ_TX].qsize - 1);

	completion = (union cavm_nix_cqe_hdr_s *)
			((void *)(nix->cq[NIX_CQ_TX].base) +
				head * sizeof(nix->cq[NIX_CQ_TX].entry_sz));

	debug("%s: completion: %p\n", __func__, completion);

	if (completion->s.cqe_type != CAVM_NIX_XQE_TYPE_E_SEND)
		return NULL;

	send_comp= (union cavm_nix_send_comp_s *)(completion + 1);

	tx_descr = &nix->send_descriptors[send_comp->s.sqe_id];

	debug("%s: tx descriptor: %p\n", __func__, tx_descr);

	packet = tx_descr->host_addr;

	nix_free_send_descriptor(nix, tx_descr);

	nix_pf_reg_write(nix, CAVM_NIXX_LF_CQ_OP_DOOR(), (NIX_CQ_TX << 32) | 1);

	return packet;
}

static int nix_recv(struct eth_device *netdev)
{
	return 0;
}

static int nix_xmmit(struct eth_device *netdev, void *pkt, int pkt_len)
{
	return 0;
}

int nix_dequeue_rx_packet(struct nix_handle *nix, void *buffer, int *buf_size)
{
	struct nix_rx_descr *rx_descr;
	union cavm_nixx_lf_cq_op_status op_status;
	u8 *ptr = (u8 *)buffer;
	u64 *addr;
	u32 head, tail;
	int seg;

	op_status.u =
		cavm_atomic_fetch_and_add64_nosync(nix->nix_base +
						   CAVM_NIXX_LF_CQ_OP_STATUS(),
						   NIX_CQ_RX << 32);
	head = op_status.s.head;
	tail = op_status.s.tail;

	if (head == tail)
		return -1;

	head &= (nix->cq[NIX_CQ_RX].qsize - 1);
	rx_descr = (struct nix_rx_descr *)(nix->cq[NIX_CQ_RX].base) + head;
	debug("%s: completion: %p\n", __func__, rx_descr);

	if (rx_descr->hdr.s.cqe_type != CAVM_NIX_XQE_TYPE_E_RX)
		return -1;

	addr = (dma_addr_t *)(rx_descr + 1);
	debug("%s: segs: %d (%d@0x%llx, %d@0x%llx, %d@0x%llx)\n", __func__,
	      rx_descr->rx_sg.s.segs, rx_descr->rx_sg.s.seg1_size, addr[0],
	      rx_descr->rx_sg.s.seg2_size, addr[1],
	      rx_descr->rx_sg.s.seg3_size, addr[2]);
	if (*buf_size < rx_descr->rx_sg.s.seg1_size +
			rx_descr->rx_sg.s.seg2_size +
			rx_descr->rx_sg.s.seg3_size) {
		debug("%s: Error: rx buffer size %d too small\n",
		      __func__, *buf_size);
		return -1;
	}

	memcpy(ptr, (void *)addr[0], rx_descr->rx_sg.s.seg1_size);
	ptr += rx_descr->rx_sg.s.seg1_size;
	if (rx_descr->rx_sg.s.seg2_size) {
		memcpy(ptr, (void *)addr[1], rx_descr->rx_sg.s.seg2_size);
		ptr += rx_descr->rx_sg.s.seg2_size;
	}
	if (rx_descr->rx_sg.s.seg3_size) {
		memcpy(ptr, (void *)addr[2], rx_descr->rx_sg.s.seg3_size);
		ptr += rx_descr->rx_sg.s.seg3_size;
	}

	for (seg = 0; seg < rx_descr->rx_sg.s.segs; seg++)
		cavm_st128(nix->npa->npa_base + CAVM_NPA_LF_AURA_OP_FREE0(),
			   addr[seg], (1ULL << 63) | NPA_POOL_RX);

	nix_pf_reg_write(nix, CAVM_NIXX_LF_CQ_OP_DOOR(), (NIX_CQ_RX << 32) | 1);

	return 0;
}

#if 0

static int nix_lf_alloc(struct nix_handle *nix)
{
	struct nix_node_state *state = &global_node_state[nix->hw->node];
	union cavm_nixx_af_const2 const2;
	union cvmx_rvu_pf_func rvu_pf_func;
	union cvmx_nixx_af_lfx_cfg lfx_cfg;
	union cavm_nix_af_lfx_cqs_cfg lfx_cqs_cfg;
	union cavm_nix_af_lfx_rqs_cfg rqs_cfg;
	union cavm_nix_af_lfx_rss_cfg rss_cfg;
	union cavm_nix_af_lfx_sqs_cfg sqs_cfg;
	union cavm_nix_af_lfx_tx_cfg tx_cfg;
	union cavm_nix_af_lfx_tx_cfg2 tx_cfg2;
	union cavm_nix_af_lfx_tx_parse_cfg tx_parse_cfg;
	int lf;
	void *cint_base = NULL, *cq_base = NULL, *qint_base = NULL;
	void *rq_base = NULL, *rss_base = NULL, *sqs_base = NULL;
	int retcode = -1;

	const2.u = nix_reg_read(nix, CAVM_NIX_AF_CONST2);

	if (state->next_free_lf >= const2.s.lfs) {
		printf("N%d NIX: Ran out of LFs\n", nix->hw->node);
		return -1;
	}
	lf = state->next_free_lf++;

	rvu_pf_func.u = 0;
	rvu_pf_func.s.pf = 0;
	rvu_pf_func.s.func = 0;
	lfx_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_LFX_CFG(lf));
	lfx_cfg.s.xqe_size = (CQ_ENTRY_SIZE == 128) ?
			CAVM_NIX_XQESZ_E_W16 : CAVM_NIX_XQESZ_E_W64;
	lfx_cfg.s.sso_pf_func = rvu_pf_func.s.func;
	lfx_cfg.s.npa_pf_func = rvu_pf_func.s.func;
	/* Allocate space for storing LF Completion Interrupts */
	cint_base = nix_memaloc(const2.s.cints,
				sizeof(union cavm_nix_cint_hw_s), __func__);
	if (!cint_base)
		goto error;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_CINTS_BASE(lf), (u64)cints_base);

	/* Allocate space for storing LF Completion Queues Admin */
	cq_base = nix_memalloc(MAX_CQS, sizeof(union cavm_nix_cq_ctx_s),
			       __func__);
	if (!cq_base)
		goto error;

	nix_reg_write(nix, CAVM_NIXX_AF_LFX_CQS_BASE(lf), (u64)cq_base);
	lfx_cqs_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_LFX_CQS_CFG(lf));
	lfx_cqs_cfg.s.max_queuesm1 = MAX_CQS - 1;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_CQS_CFG(lf), lfx_cqs_cfg.u);
	/* Allocate space for storing LF Queue Interrupts */
	qint_base = nix_memalloc(const2.s.qints,
				 sizeof(union cavm_nix_qint_hw_s), __func__);
	if (!qint_base)
		goto error;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_QINTS_BASE(lf),
		      (u64)qint_base);

	rq_base = nix_memalloc(MAX_RQS, sizeof(union cavm_nix_rq_ctx_s),
			       __func__);
	if (!rq_base)
		goto error;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_RQS_BASE(lf), (u64)rq_base);
	rqs_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_LFX_RQS_CFG(lf));
	rqs_cfg.s.max_queuesm1 = MAX_RQS - 1;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_RQS_CFG(lf), rqs_cfg.u);

	/* Allocate space for storing LF RSS tables */
	rss_base = nix_memalloc(256 << RSS_SIZE, sizeof(union cavm_nix_rsse_s),
				__func__);
	if (!rss_base)
		goto error;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_RSS_BASE(lf),
		      (u64)rss_base);
	rss_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_LFX_RSS_CFG(lf));
	rss_cfg.s.ena = 1;
	rss_cfg.s.size = RSS_SIZE;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_RSS_CFG(lf), rss_cfg.u);

	/* Allocate space for storing LF Send Queues */
	sqs_base = nix_memalloc(MAX_SQS, sizeof(union cavm_nix_sq_ctx_s),
				__func__);
	nix_reg_write(nix, CAVM_NIX_AF_LFX_SQS_BASE(lf), (u64)sqs_base);
	sqs_cfg.u nix_reg_read(nix, CAVM_NIX_AF_LFX_SQS_CFG(lf));
	sqs_cfg.s.queuesm1 = MAX_SQS - 1;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_SQS_CFG(lf), sqs_cfg.u);

	/* NIX AF Local Function Transmit Configuration Register */
	tx_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_LFX_TX_CFG(lf));
	tx_cfg.s.lock_ena = 1;
	tx_cfg.s.lock_viol_cqe_ena = 1;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_TX_CFG(lf), tx_cfg.u);
	tx_cfg2.u = nix_reg_read(nix, CAVM_NIX_AF_LFX_TX_CFG2(lf));
	tx_cfg2.s.lmt_ena = 1;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_TX_CFG2(lf), tx_cfg2.u);

	tx_parse_cfg.u = nix_reg_read(nix,
				      CAVM_NIX_AF_LFX_PARSE_CFG(lf));
	tx_parse_cfg.s.pkind = 1;
	nix_reg_write(nix, CAVM_NIX_AF_LFX_PARSE_CFG(lf),
		      tx_parse_cfg.u);

	nix->cint_base = cint_base;
	nix->cq_ctx_base = cq_base;
	nix->qint_base = qint_base;
	nix->rq_ctx_base = rq_base;
	nix->rss_base = rss_base;
	nix->sq_ctx_base = sqs_base;

	return 0;
error:

	if (cint_base)
		free(cint_base);
	if (cq_base)
		free(cq_base);
	if (qint_base)
		free(qint_base);
	if (rq_base)
		free(rq_base);
	if (rss_base)
		free(rss_base);
	if (sqs_base)
		free(sqs_base);
	return retcode;
}

/**
 * Issue a command to the NIX AF Admin Queue
 *
 * @param nix    nix handle
 * @param op     Operation
 * @param ctype  Context type
 * @param cindex Context index
 * @param resp   Result pointer
 *
 * @return	0 for success, -1 on failure
 */
static int nix_aq_issue_command(struct nix_handle *nix, enum nix_aq_instop_e op,
				enum nix_aq_ctype_e ctype,
				int cindex, void *resp)
{
	union cavm_nix_af_aq_status aq_status;
	union cavm_nix_aq_inst_s *aq_inst = nix->aq_base + aq_status.s.head_ptr;
	volatile union cavm_nix_aq_res_s *result = resp;
	int lf = nix->lf;

	aq_inst->u[0] = 0;
	aq_inst->u[1] = 0;
	aq_inst->s.op = op;
	aq_inst->s.ctype = ctype;
	aq_inst->s.lf = lf;
	aq_inst->s.cindex = cindex;
	aq_inst->s.doneint = 0;
	aq_inst->s.res_addr = resp;
	__iowmb();
	nix_reg_write(nix, CAVM_NIX_AF_AQ_DOOR, 1);

	/* Wait for completion */
	do {
		WATCHDOG_RESET();
	} while (result->s.compcode == 0);
	if (result->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
		printf("N%d, NIX: Admin Queue failed with code %d\n",
		       nix->hw->node, result->s.compcode);
		return -1;
	}
	return 0;
}

/**
 * Allocate and setup a new Completion Queue for use
 *
 * @param nix Handle for port to config
 *
 * @return Completion Queue number, or negative on failure
 */
static int nix_lf_alloc_cq(struct nix_handle *nix)
{
	struct nix_node_state *state = &global_node_state[nix->hw->node];
	struct nix_aq_cq_request aq_request
					__aligned(CONFIG_SYS_CACHELINE_SIZE);
	int cq = state->next_free_cq++;
	static const int cqe_size = 16 << (CQS_QSIZE * 2);
	void *cqe_mem = nix_memalloc(cqe_size, CQ_ENTRY_SIZE,
				     __func__ "CQ Data");
	int retcode;

	memset(&aq_request, 0, sizeof(aq_request));
	aq_request.cq.s.ena = 1;
	aq_request.cq.s.bpid = nix->pki_channel;
	aq_request.cq.s.substream = 0;	/* FIXME: Substream IDs? */
	aq_request.cq.s.drop_ena = 1;
	aq_request.cq.s.caching = 1;
	aq_request.cq.s.qsize = CQS_QSIZE;
	aq_request.cq.s.drop = 255 * 7 / 8;
	aq_request.cq.s.qint_idx = 0;
	aq_request.cq.s.cint_idx = 0;
	aq_request.cq.s.base = (u64)cqe_mem;

	retcode = nix_aq_issue_command(nix, CAVM_NIX_AQ_INSTOP_E_INIT,
				       CAVM_NIX_AQ_CTYPE_E_CQ, nix->lf, cq,
				       &aq_request.resp);
	if (retcode) {
		printf("%s: Error requesting completion queue\n", __func__);
		return -1;
	}
	debug("%s: CQ(%d) allocated, base %p\n", __func__, cq, cqe_mem);

	nix->cq = cq;
	nix->cqe_base = cqe_mem;
	return cq;
}

/**
 * Allocate and setup a new Receive Queue for use
 *
 * @param nix Handle for port to config
 *
 * @return Receive Queue number, or negative on failure
 */
static int nix_lf_alloc_rq(struct nix_handle *nix)
{
	struct nix_node_state *state = &global_node_state[nix->hw->node];
	struct nix_aq_rq_request aq_request __aligned(CONFIG_SYS_CACHELINE_SIZE);
	int cq = nix->cq;
	int rq;
	int retcode;

	if (state->next_free_rq >= MAX_RQS) {
		printf("%s: NIX: Ran out of Receive Queues\n", __func__);
		return -1;
	}
	rq = state->next_free_rq++;

	memset(&aq_request, 0, sizeof(aq_request));

	aq_request.rq.s.ena = 1;
	aq_request.rq.s.sso_ena = USE_SSO;
	aq_request.rq.s.ipsech_ena = 0;
	aq_request.rq.s.ena_wqwd = 1;
	aq_request.rq.s.cq = cq;
	aq_request.rq.s.substream = 0;	/* FIXME: Substream IDs? */
	aq_request.rq.s.wqe_aura = -1;	/* No WQE aura */
	aq_request.rq.s.spb_aura = CAVM_NPA_PACKET_POOL;	/* TODO */
	aq_request.rq.s.lpb_aura = CAVM_NPA_PACKET_POOL;	/* TODO */
	/* U-Boot doesn't use WQE group for anything */
	aq_request.rq.s.sso_grp = 0;
	aq_request.rq.s.sso_tt = CAVM_SSO_TT_E_ORDERED;		/* TODO */
	aq_request.rq.s.pb_caching = 1;
	aq_request.rq.s.wqe_caching = 1;
	aq_request.rq.s.xqe_drop_ena = 0;	/* Disable RED dropping */
	aq_request.rq.s.spb_drop_ena = 0;
	aq_request.rq.s.lpb_drop_ena = 0;
	aq_request.rq.s.spb_sizem1 =
		nix_npa_get_block_size(nix, CAVM_NPA_PACKET_POOL) / 8 - 1;
	aq_request.rq.s.sbp_ena = 1;
	aq_request.rq.s.lpb_sizem1 =
		nix_npa_get_block_size(nix, CAVM_NPA_PACKET_POOL) / 8 - 1;
	aq_request.rq.s.first_skip =
			(!USE_SSO) ? 0 : (CQ_ENTRY_SIZE == 128) ? 16 : 64;
	aq_request.rq.s.later_skip = 0;
	aq_request.rq.s.xqe_imm_copy = 0;
	aq_request.rq.s.xqe_hdr_split = 0;
	aq_request.rq.s.xqe_drop = 255;
	aq_request.rq.s.xqe_pass = 255;
	aq_request.rq.s.wqe_pool_drop = 0;	/* No WQE pool */
	aq_request.rq.s.wqe_pool_pass = 0;	/* No WQE pool */
	aq_request.rq.s.spb_aura_drop = 255;
	aq_request.rq.s.spb_aura_pass = 255;
	aq_request.rq.s.spb_pool_drop = 0;
	aq_request.rq.s.spb_pool_pass = 0;
	aq_request.rq.s.lpb_aura_drop = 255;
	aq_request.rq.s.lpb_aura_pass = 255;
	aq_request.rq.s.lpb_pool_drop = 0;
	aq_request.rq.s.lpb_pool_pass = 0;
	aq_request.rq.s.qint_idx = 0;
	retcode = nix_aq_issue_command(nix, CAVM_NIX_AQ_INSTOP_E_INIT,
				       CAVM_NIX_AQ_CTYPE_E_RQ, rq,
				       &aq_request.resp);

	debug("%s: RQ(%d) allocated\n", __func__, rq);
	if (retcode < 0)
		return retcode;

	nix->rq = rq;
	return rq;
}

/**
 * Setup SMQ -> TL4 -> TL3 -> TL2 -> TL1 -> MAC mapping
 *
 * @param nix     Handle to setup
 * @param nix_link_e NIX link number enumeration
 *
 * @return SMQ number, or negative on failure
 */
static int nix_af_setup_sq(struct nix_handle *nix, int nix_link_e)
{
	union cavm_nixx_af_tl1x_schedule tl1_sched;
	union cavm_nixx_af_tl2x_parent tl2_parent;
	union cavm_nixx_af_tl3x_parent tl3_parent;
	union cavm_nixx_af_tl3_tl2x_cfg tl3_tl2_cfg;
	union cavm_nixx_af_tl3_tl2x_linkx_cfg tl3_tl2_link_cfg;
	union cavm_nixx_af_tl4x_parent tl4_parent;
	union cavm_nixx_af_tl4x_sdp_link_cfg tl4_sdp_link_cfg;
	union cavm_nixx_af_smqx_cfg smq_cfg;
	union cavm_nixx_af_mdqx_schedule mdq_sched;
	union cavm_nixx_af_mdqx_parent mdq_parent;
	union cavm_npc_af_pkindx_action0 pkindx_action0;
	union cavm_npc_intfx_miss_act miss_act;
	int tl1_index = nix_link_e; /* NIX_LINK_E enum */
	int tl2_index = tl1_index;
	int tl3_index = tl2_index;
	int tl4_index = tl3_index;
	int smq_index = tl4_index;

	tl1_sched.u = nix_reg_read(nix,
				CAVM_NIX_AF_TL1X_SCHEDULE(tl1_index));
	tl1_sched.s.rr_quantum = MAX_MTU;
	nix_reg_write(nix, CAVM_NIX_AF_TL1X_SCHEDULE(tl1_index),
		      tl1_sched.u);
	tl2_parent.u = nix_reg_read(nix,
				CAVM_NIX_AF_TL2X_PARENT(tl2_index));
	tl2_parent.s.parent = tl1_index;
	nix_reg_write(nix, CAVM_NIX_AF_TL2X_PARENT(tl2_index),
		      tl2_parent.u);
	tl3_parent.u = nix_reg_read(nix,
				CAVM_NIX_AF_TL3X_PARENT(tl3_index));
	tl3_parent.s.parent = tl2_index;
	nix_reg_write(nix, CAVM_NIX_AF_TL3X_PARENT(tl3_index),
		      tl3_parent.u);
	tl3_tl2_cfg.u = nix_reg_read(nix,
				CAVM_NIX_AF_TL3_TL2X_CFG(tl3_index));
	tl3_tl2_cfg.s.express = 0;
	nix_reg_write(nix, CAVM_NIX_AF_TL3_TL2X_CFG(tl3_index),
		      tl3_tl2_cfg.u);

	if (nix_link_e != CAVM_NIX_LINK_E_SDP) {
		tl3_tl2_link_cfg.u =
			nix_reg_read(nix,
			  CAVM_NIX_AF_TL3_TL2X_LINKX_CFG(tl3_index,
								  nix_link_e));
		tl3_tl2_link_cfg.s.bp_ena = 1;
		tl3_tl2_link_cfg.s.ena = 1;
		tl3_tl2_link_cfg.s.relchan = 0;
		nix_reg_write(nix,
			      CAVM_NIX_AF_TL3_TL2X_LINKX_CFG(tl3_index,
								nix_link_e));
	}
	tl4_parent.u = nix_reg_read(nix,
				CAVM_NIX_AF_TL4X_PARENT(tl4_index));
	tl4_parent.s.parent = tl3_index;
	nix_reg_write(nix, CAVM_NIX_AF_TL4X_PARENT(tl4_index),
		      tl4_parent.u);
	tl4_sdp_link_cfg.u =
		nix_reg_read(nix,
			     CAVM_NIX_AF_TL4X_SDP_LINK_CFG(tl4_index));
	tl4_sdp_link_cfg.s.bp_ena = (nix_link_e == CAVM_NIX_LINK_E_SDP);
	tl4_sdp_link_cfg.s.ena = (nix_link_e == CAVM_NIX_LINK_E_SDP);
	tl4_sdp_link_cfg.s.relchan = nix->index;
	nix_reg_write(nix, CAVM_NIX_AF_TL4X_SDP_LINK_CFG_RBU_BAR0(tl4_index),
		      tl4_sdp_link_cfg.u);
	smq_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_SMQX_CFG(smq_index));
	smq_cfg.s.express = 0;
	smq_cfg.s.lf = nix->lf;
	smq_cfg.s.maxlen = MAX_MTU;
	smq_cfg.s.minlen = 60;
	nix_reg_write(nix, CAVM_NIX_AF_SMQX_CFG(smq_index), smq_cfg.u);
	mdq_sched.u = nix_reg_read(nix,
				CAVM_NIX_AF_MDQX_SCHEDULE(smq_index));
	mdq_sched.s.rr_quantum = MAX_MTU;
	nix_reg_write(nix, CAVM_NIX_AF_MDQX_SCHEDULE(smq_index),
		      mdq_sched.u);
	mdq_parent.u = nix_reg_read(nix,
				CAVM_NIX_AF_MDQX_PARENT(smq_index));
	mdq_parent.s.parent = tl4_index;
	nix_reg_write(nix, CAVM_NIX_AF_MDQX_PARENT(smq_index),
		      mdq_parent.u);
	pkindx_action0.u = npc_reg_read(nix,
				CAVM_NPC_AF_PKINDX_ACTION0_RBU_BAR0(nix->pknd));
	pkindx_action0.s.parse_done = 1;
	npc_reg_write(nix, CAVM_NPC_AF_PKINDX_ACTION0_RBU_BAR0(nix->pknd),
		      pkindx_action0.u);
	miss_act.u = npc_reg_read(nix, CAVM_NPC_AF_INTFX_MISS_ACT(
						CAVM_NPC_INTF_E_NIXX_RX(0)));
	miss_act.s.action = CAVM_NIX_RX_ACTIONOP_E_UCAST;
	npc_reg_write(nix,
		CAVM_NPC_AF_INTFX_MISS_ACT(CAVM_NPC_INTF_E_NIXX_RX(0)),
		      miss_act.u);
	miss_act.u = npc_reg_read(nix, CAVM_NPC_AF_INTFX_MISS_ACT(
						CAVM_NPC_INTF_E_NIXX_TX(0)));
	miss_act.s.action = CAVM_NIX_TX_ACTIONOP_E_UCAST_DEFAULT;
	npc_reg_write(nix,
		CAVM_NPC_AF_INTFX_MISS_ACT(CAVM_NPC_INTF_E_NIXX_TX(0)),
		      miss_act.u);

	return smq_index;
}

/**
 * Allocate and setup a new Send Queue for use
 *
 * @param nix     Handle for port to config
 * @param nix_link_e NIX link number enumeration
 *
 * @return Send Queue number, or negative on failure
 */
static int nix_lf_alloc_sq(struct nix_handle *nix, int nix_link_e)
{
    struct nix_node_state *state = &global_node_state[nix->hw->node];
    struct nix_aq_sq_request aq_request;
    int sq;
    int smq;

    if (state->next_free_sq >= MAX_SQS) {
        printf("%s NIX: Ran out of Send Queues\n", __func__);
        return -1;
    }
    sq = state->next_free_sq++;
    smq = nix_af_setup_sq(nix, sq, nix_link_e);

    memset(&aq_request, 0, sizeof(aq_request));

    aq_request.sq.s.ena = 1;
    aq_request.sq.s.cq_ena = !USE_SSO;
    aq_request.sq.s.max_sqe_size = CAVM_NIX_MAXSQESZ_E_W16;
    aq_request.sq.s.substream = 0; // FIXME: Substream IDs?
    aq_request.sq.s.sdp_mcast = 0;
    aq_request.sq.s.cq = nix->cq;
    aq_request.sq.s.cq_limit = 0;
    aq_request.sq.s.smq = smq;
    aq_request.sq.s.sso_ena = 1; /* Always allow a SQ to submit work */
    aq_request.sq.s.smq_rr_quantum = MAX_MTU / 4;
    aq_request.sq.s.default_chan = nix->pki_channel;
    aq_request.sq.s.sqe_stype = CAVM_NIX_STYPE_E_STP;
    aq_request.sq.s.qint_idx = 0;
    aq_request.sq.s.sqb_aura = CAVM_NPA_PKO_POOL;
    nix_aq_issue_command(nix, CAVM_NIX_AQ_INSTOP_E_INIT,
			 CAVM_NIX_AQ_CTYPE_E_SQ, sq, &aq_request.resp);
    nix->sq = sq;
    return sq;
}

/**
 * Setup the NPC MCAM to route incoming packets to the NIX
 *
 * @param rq     NIX receive queue
 */
static void nix_setup_mcam(struct nix_handle *nix)
{
	union cavm_npc_af_mcamex_bankx_camx_intf camx_intf;
	union cavm_npc_af_mcamex_bankx_camx_w0 camx_w0;
	union cavm_npc_af_mcamex_bankx_camx_w1 camx_w1;
	union cavm_npc_af_mcamex_bankx_cfg bankx_cfg;
	union cavm_nix_rx_action_s rx_action;
	union cavm_npc_intf_e_nixx_rx nixx_rx;
	union cavm_npc_intf_e_nixx_tx nixx_tx;
	union cavm_npc_af_mcamex_kex_cfg kex_cfg;
	u64 key0 = nix->pki_channel;
	int mcam = nix->pko_queue;
	int rq = nix->rq;
	int bank = 0;

	/* Setup receive direction MCAM match */
	/* First require interface direction to exactly match  */
	camx_intf.u = npc_reg_read(nix,
			CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(mcam,
								    bank, 0));
	/* Mask for bits that must be zero */
	camx_intf.s.intf = ~CAVM_NPC_INTF_E_NIXX_RX(0);
	npc_reg_write(nix, CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(mcam,
								       bank, 0),
		      camx_intf.u);
	/* Second set of bits to match, must be zero */
	camx_intf.u = npc_reg_read(nix,
			CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(mcam,
								    bank, 1));
	/* Mask for bits that must be zero */
	camx_intf.s.intf = CAVM_NPC_INTF_E_NIXX_RX(0);
	npc_reg_write(nix,
		      CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(mcam,
								  bank, 1),
		      camx_intf.u);

	/* Second require the first 12 bits of the key to match, the channel */
	camx_w0.u = npc_reg_read(nix,
			CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W0(mcam,
								  bank, 0));
	/* Mask for bits that must be zero */
	camx_w0.s.md = ~key0 & ~((~0x0ull) << 12);
	npc_reg_write(nix,
		      CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W0(mcam, bank, 0),
		      camx_w0.u);
	camx_w0.u = npc_reg_read(nix,
				 CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W0(mcam,
								       bank, 1));
	/* Mask for bits that must be one */
	camx_w0.s.md = key0;
	npc_reg_write(nix,
		      CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W0(mcam, bank, 1),
		      camx_w0.u);

	/* Third requires none of the other key bits to match */
	camx_w1.u = npc_reg_read(nix,
				 CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W1(mcam,
								       bank, 0));
	camx_w1.s.md = 0;
	npc_reg_write(nix,
		      CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W1(mcam, bank, 0),
		      camx_w1.u);
	camx_w1.u = npc_reg_read(nix,
			CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W1(mcam,
								  bank, 1));
	camx_w1.s.md = 0;
	npc_reg_write(nix,
		      CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W1(mcam, bank, 1),
		      camx_w1.u);

	/* Setup receive direction action */
	rx_action.u = 0;
	rx_action.s.op = CAVM_NIX_RX_ACTIONOP_E_UCAST;
	rx_action.s.pf_func = nix->lf;
	rx_action.s.index = nix->rq;
	rx_action.s.match_id = 0;
	rx_action.s.flow_key_alg = 0;
	npc_reg_write(nix, CAVM_NPC_AF_MCAMEX_BANKX_ACTION(mcam, bank),
		      rx_action.u);

	/* Enable the MCAM entry */
	bankx_cfg.u = npc_reg_read(nix,
				   CAVM_NPC_AF_MCAMEX_BANKX_CFG(mcam,
									 bank));
	bankx_cfg.s.ena = 1;
	npc_reg_write(nix, CAVM_NPC_AF_MCAMEX_BANKX_CFG(mcam, bank),
		      bankx_cfg.u);

	/* Program key size and nibbles to include */
	kex_cfg.u = npc_reg_read(nix, CAVM_NPC_AF_INTFX_KEX_CFG(
						CAVM_NPC_INTF_E_NIXX_RX(0)));
	kex_cfg.s.keyw = CAVM_NPC_MCAMKEYW_E_X1;
	kex_cfg.s.parse_nibble_ena = 0x7;
	npc_reg_write(nix,
		      CAVM_NPC_AF_INTFX_KEX_CFG(
					CAVM_NPC_INTF_E_NIXX_RX(0)),
		      kex_cfg.u);
	kex_cfg.u = npc_reg_read(nix, CAVM_NPC_AF_INTFX_KEX_CFG(
						CAVM_NPC_INTF_E_NIXX_TX(0)));
	kex_cfg.s.keyw = CAVM_NPC_MCAMKEYW_E_X1;
	kex_cfg.s.parse_nibble_ena = 0xfffffff;
	npc_reg_write(nix,
		      CAVM_NPC_AF_INTFX_KEX_CFG(
					CAVM_NPC_INTF_E_NIXX_TX(0)),
		      kex_cfg.u);
}

static void nix_assign_channel_bp(struct nix_handle *nix)
{
	int channel;
	int bpid = 0;


}

/**
 * Allocates the nix handle and performs low-level initialization
 *
 * @param	node		CPU node number
 * @param	nix_base	NIX BAR 0 base address
 * @param	nix2_base	NIX BAR 2 base address
 * @param	npc_base	NPC BAR 0 base address
 * @param	rvu_pf_func	RVU pf function number
 *
 * @return	Pointer to nix handle or NULL if failure
 */
static struct *nix_handle nix_init_handle(int node, void *nix_base,
					  void *nix2_base, void *npc_base,
					  int rvu_pf_func)
{
	union cavm_nixx_af_cfg af_cfg;
	union cavm_nixx_af_status af_status;
	union cavm_nixx_af_rx_cfg rx_cfg;
	union cavm_nixx_af_ndc_cfg ndc_cfg;
	union cavm_nixx_priv_lfx_cfg lfx_cfg;
	union cavm_nixx_af_rx_chanx_cfg chanx_cfg;
	struct nix_node_state *state = &global_node_state[node];
	struct nix_handle *nix = NULL;
	int channel;
	int bpid = 0;

	debug("%s(%d, %p, %p, %d)\n", __func__,
	      nix_base, npc_base, rvu_pf_func);
	nix = calloc(1, sizeof(*nix));
	if (!nix)
		goto out_of_mem;
	nix->hw = calloc(1, sizeof(struct hw_info));
	if (!nix->hw)
		goto out_of_mem;
	nix->nix_base = nix_base;
	nix->npc_base = npc_base;
	/* Fill the NPA pool */


	nix->aq_base = nix_memalloc(AQ_RING_SIZE,
				    sizeof(union c avm_nix_aq_inst_s),
				    __func__);
	debug("%s: aq base: %p\n", __func__, nix->aq_base);
	if (!nix->aq_base)
		goto out_of_mem;

#if 0	/* Don't do this in ASIM */
	af_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_CFG);
	af_cfg.s.calibrate_x2p = 1;
	nix_reg_write(nix, CAVM_NIX_AF_CFG, af_cfg.u);

	do {
		af_status.u = nix_reg_read(nix, CAVM_NIXX_AF_STATUS);
	} while (!af_status.s.calibrate_done);
	af_status.u = nix_reg_read(nix, CAVM_NIXX_AF_STATUS);
	if (!af_status.s.calibrate_status) {
		printf("N%d NIX: AF failed X2P calibration\n", node);
		goto error;
	}
#endif
	/* Enable channel backpressure */
	rx_cfg.u = nix_reg_read(nix, CAVM_NIX_AF_RX_CFG);
	rx_cfg.s.cbp_ena = 1;
	nix_reg_write(nix, CAVM_NIX_AF_RX_CFG, rx_cfg.u);

	/* Assign channel BP numbers sequentially */
	for (channel = 0; channel < 4096; channel++) {
		chanx_cfg.u = nix_reg_read(nix,
				CAVM_NIXX_AF_RX_CHANX_CFG(channel));
		if (!chanx_cfg.s.imp)
			continue;

		debug("%s: implementing back-pressure channel %d\n",
		      __func__, channel);
		chanx_cfg.s.bp_ena = 1;
		chanx_cfg.s.bpid = bpid;
		nix_reg_write(nix,
			      CAVM_NIXX_AF_RX_CHANX_CFG(channel),
			      chanx_cfg.u);
		if (bpid < 512) {
			bpid++;
		} else {
			printf("N%d NIX: Ran out of BPID at channel %d\n",
			       node, channel);
			break;
		}
	}

	/* Enable NDC cache */
	ndc_cfg.u = 0;
	nix_reg_write(nix, CAVM_NIX_AF_NDC_CFG(), ndc_cfg.u);

	/* Allocate MSIX space for NPA AF (not needed?) */
	/* Enable NIX */


	if (nix_lf_alloc(nix) < 0) {
		printf("%s: Could not allocate lf\n", __func__);
		goto error;
	}

	lfx_cfg.u = nix_reg_read(nix, CAVM_NIX_PRIV_LFX_CFG(nix->lf));
	lfx_cfg.s.ena = 1;
	lfx_cfg.s.pf_func = rvu_pf_func;
	lfx_cfg.s.slot = 0;
	nix_reg_write(nix, CAVM_NIX_PRIV_LFX_CFG(nix->lf), lfx_cfg.u);

	return nix;

out_of_mem:
	printf("%s(%d): Error: out of memory\n", __func__, node);
error:
	if (nix) {
		if (nix->hw)
			free(nix->hw);
		free(nix);
	}
	return NULL;
}

/**
 * Transmits an Ethernet packet
 *
 * @param	netdev		Ethernet device
 * @param	pkt		Pointer to packet data
 * @param	pkt_len		Length of packet
 *
 * @return	0 for success, -1 on error
 */
static int nix_xmit(struct eth_device *netdev, void *pkt, int pkt_len)
{
	struct nix_handle *nix = netdev->priv;
	union cavm_nix_op_q_wdata_s q_wdata_s;
	union cavm_nix_send_hdr_s send_hdr_s;
	union cavm_nixx_lf_sq_op_status sq_op_status;
	union cavm_nix_send_sg_s send_sg_s;
	volatile u64 *lmt_ptr;
	u64 depth;
	u64 io_address;
	s64 lmt_status;

	debug("%s(%s, %p, %d)\n", __func__, netdev->name, pkt, pkt_len);
	q_wdata_s.u = 0;
	q_wdata_s.q = nix->pko_queue;
	sq_op_status.u = cavm_atomic_fetch_and_add64_nosync(nix->reg2_base +
				CAVM_NIX_LF_SQ_OP_STATUS_RVU_BAR2, q_wdata_s.u);
	depth = sq_op_status.s.sqb_count;
	if (depth > 64)
		return -1;
	io_address = nix->reg_base + CAVM_NIX_LF_OP_SENDX(0);
	do {
		cavm_lmt_cancel(nix);
		lmt_ptr = cavm_lmt_store_ptr();

		send_hdr_s.u[0] = 0;
		send_hdr_s.u[1] = 0;
		send_hdr_s.s.total = pkt_len;
		send_hdr_s.s.df = 1;
		send_hdr_s.s.aura = CAVM_NPA_PACKET_POOL;
		send_hdr_s.s.sizem1 = 0;
		send_hdr_s.s.pnc = 0;	/* No completion posted */
		send_hdr_s.s.sq = nix->pko_queue;
		send_hdr_s.s.sqe_id = 0;
		/* Don't worry about TCP/UDP checksum support here */

		send_sg_s.u = 0;
		send_sg_s.s.seg1_size = pktlen;
		send_sg_s.s.ld_type = CAVM_NIX_SENDLDTYPE_E_LDD;
		send_sg_s.s.subdc = CAVM_NIX_SUBDC_E_SG;
		send_sg_s.s.segs = 1;
		debug("%s(%s): Sending packet, hdr: 0x%lx 0x%lx sg: 0x%lx, pkt: %p\n",
		      __func__, ethdev->name, send_hdr_s.u[0], send_hdr_s.u[1],
		      send_sg_s.u, pkt);
		*lmt_ptr++ = send_hdr_s.u[0];
		*lmt_ptr++ = send_hdr_s.u[1];
		*lmt_ptr++ = send_sg_s.u;
		*lmt_ptr++ = pkt;
		/* We only have one segment to worry about */
		__iowmb();
		lmt_status = cavm_lmt_submit(io_address);
		if (!lmt_status) {
			debug("%s: Error: unexpected lmt_status 0x%lx\n",
			      __func__, lmt_status);
		}
	} while (lmt_status == 0);

	nix->stats.tx.packets++;
	nix->stats.tx.octets += pkt_len;

	return 0;
}

static int nix_process_rx_complete(struct eth_device *netdev,
				   union cavm_nix_rx_parse_s *rx_parse)
{
	struct nix_handle *nix = netdev->priv;
	const union cavm_nix_rx_sg_s *rx_sg_ptr =
			(const union cavm_nix_rx_sg_s *)(rx_parse + 1);
	union cavm_nix_rx_sg_s rx_sg;
	int qwords;
	int num_segs;
	int segments;
	int segment_length;
	int pkt_len;
	void *pkt;
	u64 addr;

	qwords = rx_parse->s.desc_sizem1 + 1;
	while (qwords > 0) {
		pkt_len = rx_parse->s.pkt_lenm1 + 1;
		rx_sg.u = rx_sg_ptr->u;
		num_segs = rx_sg.s.segs;
		segment_length += rx_sg_ptr->s.seg1_size;
		addr = rx_sg_ptr[1].u;
		pkt = (void *)addr;
		prefetch(pkt);
		if (num_segs < 2)
			break;
		printf("%s(%s): Only one segment supported\n", __func__,
		       netdev->name);
		qwords -= 2;
	}
	net_process_received_packet(pkt, pkt_len);

	return pkt_len;
}
static int nix_recv(struct eth_device *netdev)
{
	struct nix_handle *nix = netdev->priv;
	struct nix_aq_cq_request aq_request;
	union cavm_nix_cqe_hdr_s *cq_next;
	union cavm_nixx_lf_cq_op_status cq_op_status;
	union cavm_nix_op_q_wdata_s q_wdata_s;
	union cavm_nix_cqe_hdr_s cq_hdr;
	union cavm_nix_rx_parse_s *rx_parse;
	s64 *cq_op_status_ptr;
	union cavm_nix_cqe_header_s *cq_ptr;
	union cavm_nixx_lf_cq_op_door op_door;
	const union cavm_nix_rx_sg_s *rx_sg_ptr;
	union cavm_nix_rx_sg_s rg_sg;
	u64 addr;
	int loc;
	int retcode;
	int pkt_len = 0;

	memset(&aq_request, 0, sizeof(aq_request));
	retcode = nix_aq_issue_command(nix, CAVM_NIX_AQ_INSTOP_E_READ,
				       CAVM_NIX_AQ_CTYPE_E_CQ,
				       &aq_request.resp);
	if (retcode) {
		printf("%s(%s): Error issuing command\n", __func__,
		       netdev->name);
		return -1;
	}
	cq_ptr = (void *)aq_request.cq.s.base;
	cq_op_status_ptr = (s64 *)(nix->reg2_base +
				   CAVM_NIX_LF_CQ_OP_STATUS_RVU_BAR2);
	q_wdata_s.u = 0;
	q_wdata_s.s.q = nix->cq;

		cq_op_status.u =
			cavm_atomic_fetch_and_add64_nosync(cq_op_status_ptr,
							   q_wdata_s.u);
	if (cq_op_status.s.head == cq_op_status.s.tail)
		return 0;

	loc = cq_op_status.s.head;
	cq_next = cq_ptr + loc;
	cq_hdr.u = cq->next.u;
	rx_parse = (const union cavm_nix_rx_parse_s *)(cq_next + 1);
	loc++;
	loc &= CQ_ENTRIES - 1;
	cq_next = cq_ptr + loc;
	prefetch(cq_next);
	if (cq_hdr.s.cqe_type == CAVM_NIX_XQE_TYPE_E_RX)
		pkt_len = nix_process_rx_complete(nix, rx_parse);
	else
		printf("%s(%s): Error: Unsupported CQ header type %d\n",
		       __func__, ethdev->name, cq_hdr.s.cqe_type);
	op_door.u = 0;
	op_door.s.cq = nix->cq;
	op_door.s.count = 1;
	nix_reg2_write(nix, CAVM_NIX_LF_CQ_OP_DOOR_RVU_BAR2, op_door.u);

	return pkt_len;
}

static int nix_open(struct eth_device *netdev)
{
	return 0;
}

static int nix_halt(struct eth_device *netdev)
{
	return 0;
}

int nix_initialize(struct udevice *pdev, int vf_num)
{
	struct eth_device *netdev;
	struct nix_handle *nix = NULL;
	struct udevice *npcdev;
	struct udevice *pci_bus;
	size_t size;
	void *reg_base;
	void *reg2_base;

	int retcode;

	netdev = calloc(1, sizeof(struct eth_device));
	if (!netdev) {
		retcode = -ENOMEM;
		goto fail;
	}

	reg_base = dm_pci_map_bar(pdev, 0, &size, PCI_REGION_MEM);
	reg2_base = dm_pci_map_bar(pdev, 2, &size, PCI_REGION_MEM);
	pci_bus =
	retcode = pci_find_device_id()
	netdev->halt = nix_halt;
	netdev->init = nix_open;
	netdev->send = nix_xmit;
	netdev->recv = nix_recv;


	retcode = eth_register(netdev);

fail:
	return retcode;

}
#endif
#if 0
int octeontx2_nix_probe(struct udevice *dev)
{
	int vf;
	void *nix_regs;
	void *nix_regs2;
	size_t size;

	nix_regs = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);
	nix_regs2 = dm_pci_map_bar(dev, 2, &size, PCI_REGION_MEM);

	debug("%s: %d, bar0: %p, bar2: %p\n", __func__, __LINE__,
	      nix_regs, nix_regs2);

	return 0;
}

static const struct misc_ops octeontx2_nix_ops = {
};

static const struct udevice_id octeontx2_nix_ids[] = {
	{ .compatible = "cavium,nix" },
};

U_BOOT_DRIVER(octeontx2_nix) = {
	.name	= "octeontx2_nix",
	.id	= UCLASS_MISC,
	.probe	= octeontx2_nix_probe,
	.of_match = octeontx2_nix_ids,
	.ops = &octeontx2_nix_ops,
	.priv_auto_alloc_size = sizeof(struct nix_handle),
};

static const struct pci_device_id octeontx2_nix_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_OCTEONTX2_RVU) },
	{}
};

U_BOOT_PCI_DEVICE(octeontx2_nix, octeontx2_nix_supported);
#endif
