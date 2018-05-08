/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#define DEBUG
#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <pci.h>
#include <memalign.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/log2.h>
#include <asm/arch/octeontx2.h>
#include "cavm-csrs-lmt.h"
#include "cavm-csrs-nix.h"
#include "cavm-csrs-npa.h"
#include "cavm-csrs-npc.h"
#include "cavm-csrs-rvu.h"
#include "rvu_common.h"
#include "rvu.h"
#include "nix_af.h"
#include "nix.h"
#include "lmt.h"
#include "cgx.h"

static const int USE_SSO = 0;	/** Do not use SSO, use completion queues */
static const int MAX_MTU = 9212;/** Maximum packet size */
static const int MAX_CQS = 32;	/** Maximum of 32 completion queues */
static const int MAX_SQS = 32;	/** Maximum of 32 send queues */
static const int MAX_RQS = 32;	/** Maximum of 32 receive queues */
/** Size of RSS table (256) See NIX_AF_LFX_RSS_CFG[size] */
static const int RSS_SIZE = 0;
/** Each completion queue contains 256 entries, see NIC_CQ_CTX_S[qsize] */
static const int CQS_QSIZE = Q_SIZE_256;
/** Number of CQ entries */
static const int AQ_RING_SIZE = 16 << (AQ_SIZE * 2);
/**
 * Each completion queue entry contains 512 bytes, see
 * NIXX_AF_LFX_CFG[xqe_size]
 */
static const int CQ_ENTRY_SIZE = 64;

struct nix_aq_cq_request {
	union cavm_nix_aq_res_s	resp	ALIGNED;
	union cavm_nix_cq_ctx_s	cq	ALIGNED;
};

struct nix_aq_rq_request {
	union cavm_nix_aq_res_s	resp	ALIGNED;
	union cavm_nix_rq_ctx_s	rq	ALIGNED;
};

struct nix_aq_sq_request {
	union cavm_nix_aq_res_s	resp	ALIGNED;
	union cavm_nix_sq_ctx_s	sq	ALIGNED;
};

static struct nix_af_state {
	int next_free_lf;
	int next_free_sq;
	int next_free_rq;
	int next_free_cq;
	int next_free_rssi;
	int next_free_bpid;
} af_state;

static int nix_lf_alloc_cq(struct nix_af_handle *nix_af,
			   struct nix_handle *nix);

static struct nix_af_handle nix_afs[MAX_NIX];
static int nix_afs_references[MAX_NIX] = {0,};
static LIST_HEAD(nix_list);
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
void *nix_memalloc(int num_elements, size_t elem_size, const char *msg)
{
	size_t alloc_size = num_elements * elem_size;
	void *base = memalign(CONFIG_SYS_CACHELINE_SIZE, alloc_size);

	if (!base)
		printf("NIX: Memory alloc failed for %s (%d * %zd = %zd bytes)\n",
		       msg ? msg : __func__, num_elements, elem_size,
		       alloc_size);
	else
		memset(base, 0, alloc_size);

	return base;
}

int nix_get_af_num(u64 base_addr)
{
	return (base_addr >> 28) & 0;
}

struct nix_handle *nix_get_pdata(int nic_id)
{
	struct nix_handle *nix;

	list_for_each_entry(nix, &nix_list, nix_list) {
		if (nix->nic_id == nic_id)
			return nix;
	}
	return NULL;
}

int nix_get_nix_cnt(void)
{
	struct nix_handle *nix;
	int count = 0;

	list_for_each_entry(nix, &nix_list, nix_list)
		count++;

	return count;
}

static int npa_setup_admin(struct nix_af_handle *nix_af)
{
	int err;
	struct npa_af_handle *npa = nix_af->npa_af;
	union cavm_npa_af_gen_cfg npa_cfg;
	union cavm_npa_af_ndc_cfg ndc_cfg;
	union cavm_npa_af_aq_cfg aq_cfg;

	err = cavm_rvu_aq_alloc(&npa->aq, Q_COUNT(AQ_SIZE),
				sizeof(union cavm_npa_aq_inst_s),
				sizeof(union cavm_npa_aq_res_s));
	if (err) {
		printf("%s: Error %d allocating admin queue\n", __func__, err);
		return err;
	}
	debug("%s: NPA admin queue allocated at %p\n", __func__,
	      npa->aq.inst.base);

	/* Set little Endian */
	npa_cfg.u = npa_af_reg_read(nix_af->npa_af, CAVM_NPA_AF_GEN_CFG());
	npa_cfg.s.af_be = 0;
	npa_af_reg_write(nix_af->npa_af, CAVM_NPA_AF_GEN_CFG(), npa_cfg.u);
	/* Enable NDC cache */
	ndc_cfg.u = npa_af_reg_read(nix_af->npa_af, CAVM_NPA_AF_NDC_CFG());
	ndc_cfg.s.ndc_bypass = 0;
	npa_af_reg_write(nix_af->npa_af, CAVM_NPA_AF_NDC_CFG(), ndc_cfg.u);
	/* Set up queue size */
	aq_cfg.u = npa_af_reg_read(nix_af->npa_af, CAVM_NPA_AF_AQ_CFG());
	aq_cfg.s.qsize = AQ_SIZE;
	npa_af_reg_write(nix_af->npa_af, CAVM_NPA_AF_AQ_CFG(), aq_cfg.u);
	/* Set up queue base address */
	npa_af_reg_write(npa, CAVM_NPA_AF_AQ_BASE(), npa->aq.inst.iova);

	return 0;
}

static int npa_attach_aura(struct nix_af_handle *nix_af, int lf,
			   const union cavm_npa_aura_s *desc, u32 aura_id)
{
	struct npa_af_handle *npa = nix_af->npa_af;
	union cavm_npa_aq_inst_s *inst;
	volatile union cavm_npa_aq_res_s *res;
	union cavm_npa_af_aq_status aq_stat;
	union cavm_npa_aura_s *context;
	u64 head;
	ulong start;

	debug("%s(%p, %d, %p, %u)\n", __func__, nix_af, lf, desc, aura_id);
	aq_stat.u = npa_af_reg_read(npa, CAVM_NPA_AF_AQ_STATUS());
	head = aq_stat.s.head_ptr;
	inst = (union cavm_npa_aq_inst_s *)(npa->aq.inst.base) + head;
	res = (volatile union cavm_npa_aq_res_s *)(npa->aq.res.base);

	memset(inst, 0, sizeof(*inst));
	inst->s.lf = lf;
	inst->s.doneint = 0;
	inst->s.ctype = CAVM_NPA_AQ_CTYPE_E_AURA;
	inst->s.op = CAVM_NPA_AQ_INSTOP_E_INIT;
	inst->s.res_addr = npa->aq.res.iova;
	inst->s.cindex = aura_id;

	context = (union cavm_npa_aura_s *)(npa->aq.res.base +
						CONFIG_SYS_CACHELINE_SIZE);
	memset(npa->aq.res.base, 0, npa->aq.res.entry_sz);
	memcpy(context, desc, sizeof(union cavm_npa_aura_s));
	__iowmb();
	npa_af_reg_write(npa, CAVM_NPA_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	while ((res->s.compcode == CAVM_NPA_AQ_COMP_E_NOTDONE) &&
	       (get_timer(start) < 1000))
		WATCHDOG_RESET();
	if (res->s.compcode != CAVM_NPA_AQ_COMP_E_GOOD) {
		printf("%s: Error: result 0x%x not good\n",
		       __func__, res->s.compcode);
		return -1;
	}

	return 0;
}

static int npa_attach_pool(struct nix_af_handle *nix_af, int lf,
			   const union cavm_npa_pool_s *desc, u32 pool_id)
{
	union cavm_npa_aq_inst_s *inst;
	volatile union cavm_npa_aq_res_s *res;
	union cavm_npa_af_aq_status aq_stat;
	struct npa_af_handle *npa = nix_af->npa_af;
	union cavm_npa_aura_s *context;
	u64 head;
	ulong start;

	debug("%s(%p, %d, %p, %u)\n", __func__, nix_af, lf, desc, pool_id);
	aq_stat.u = npa_af_reg_read(npa, CAVM_NPA_AF_AQ_STATUS());
	head = aq_stat.s.head_ptr;

	inst = (union cavm_npa_aq_inst_s *)(npa->aq.inst.base) + head;
	res = (union cavm_npa_aq_res_s *)(npa->aq.res.base);

	memset(inst, 0, sizeof(*inst));
	inst->s.cindex = pool_id;
	inst->s.lf = lf;
	inst->s.doneint = 0;
	inst->s.ctype = CAVM_NPA_AQ_CTYPE_E_POOL;
	inst->s.op = CAVM_NPA_AQ_INSTOP_E_INIT;
	inst->s.res_addr = npa->aq.res.iova;

	context = (union cavm_npa_aura_s *)(npa->aq.res.base +
						CONFIG_SYS_CACHELINE_SIZE);
	memset(npa->aq.res.base, 0, npa->aq.res.entry_sz);
	memcpy(context, desc, sizeof(union cavm_npa_aura_s));
	__iowmb();
	npa_af_reg_write(npa, CAVM_NPA_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	while ((res->s.compcode == CAVM_NPA_AQ_COMP_E_NOTDONE) &&
	       (get_timer(start) < 1000))
		WATCHDOG_RESET();

	if (res->s.compcode != CAVM_NPA_AQ_COMP_E_GOOD) {
		printf("%s: Error: result 0x%x not good\n",
		       __func__, res->s.compcode);
		return -1;
	}

	return 0;
}

int npa_lf_admin_setup(struct nix_af_handle *nix_af, int lf,
		       u32 aura_size,
		       const union cavm_npa_aura_s *aura_ctx,
		       dma_addr_t auras_dev_addr,
		       const union cavm_npa_pool_s *pool_ctx,
		       u32 pool_cnt)
{
	union cavm_npa_af_lf_rst lf_rst;
	union cavm_npa_af_const af_const;
	union cavm_npa_af_lfx_auras_cfg auras_cfg;
	struct npa_af_handle *npa = nix_af->npa_af;
	int index;
	int err;

	debug("%s(%p, %d, %u, %p, 0x%llx, %p, %u)\n", __func__, nix_af, lf,
	      aura_size, aura_ctx, auras_dev_addr, pool_ctx, pool_cnt);
	lf_rst.u = 0;
	lf_rst.s.exec = 1;
	lf_rst.s.lf = lf;
	npa_af_reg_write(npa, CAVM_NPA_AF_LF_RST(), lf_rst.u);

	do {
		lf_rst.u = npa_af_reg_read(npa, CAVM_NPA_AF_LF_RST());
		WATCHDOG_RESET();
	} while (lf_rst.s.exec);

	/* TODO: remove this */
	af_const.u = npa_af_reg_read(npa, CAVM_NPA_AF_CONST());
	debug("%s: %d NPA local functions\n", __func__, af_const.s.lfs);
	/* Set Aura size and enable caching of contexts */
	auras_cfg.u = npa_af_reg_read(npa, CAVM_NPA_AF_LFX_AURAS_CFG(lf));
	auras_cfg.s.loc_aura_size = aura_size;
	auras_cfg.s.caching = 1;
	auras_cfg.s.rmt_aura_size = 0;
	auras_cfg.s.rmt_aura_offset = 0;
	auras_cfg.s.rmt_lf = 0;
	npa_af_reg_write(npa, CAVM_NPA_AF_LFX_AURAS_CFG(lf), auras_cfg.u);
	/* Configure aura HW context base */
	npa_af_reg_write(npa, CAVM_NPA_AF_LFX_LOC_AURAS_BASE(lf),
			 auras_dev_addr);

	/* Set up the auras */
	for (index = 0; index < pool_cnt; index++) {
		err = npa_attach_aura(nix_af, lf, &aura_ctx[index], index);
		if (err)
			return err;
		err = npa_attach_pool(nix_af, lf, &pool_ctx[index], index);
		if (err)
			return err;
	}
	return 0;
}

int npa_lf_admin_shutdown(struct nix_af_handle *nix_af, int lf, u32 pool_count)
{
	int pool_id;
	u32 head;
	union cavm_npa_aq_inst_s *inst;
	volatile union cavm_npa_aq_res_s *res;
	union cavm_npa_aura_s *aura_ctx;
	union cavm_npa_pool_s *pool_ctx;
	union cavm_npa_af_aq_status aq_stat;
	union cavm_npa_af_lf_rst lf_rst;
	struct npa_af_handle *npa = nix_af->npa_af;
	ulong start;

	for (pool_id = 0; pool_id < pool_count; pool_id++) {
		aq_stat.u = npa_af_reg_read(npa, CAVM_NPA_AF_AQ_STATUS());
		head = aq_stat.s.head_ptr;
		inst = (union cavm_npa_aq_inst_s *)(npa->aq.inst.base) + head;
		res = npa->aq.res.base;

		memset(inst, 0, sizeof(*inst));
		inst->s.cindex = pool_id;
		inst->s.lf = lf;
		inst->s.doneint = 0;
		inst->s.ctype = CAVM_NPA_AQ_CTYPE_E_POOL;
		inst->s.op = CAVM_NPA_AQ_INSTOP_E_WRITE;
		inst->s.res_addr = npa->aq.res.iova;

		/* Context base address */
		pool_ctx = (union cavm_npa_pool_s *)
				(npa->aq.res.base + CONFIG_SYS_CACHELINE_SIZE);
		memset(npa->aq.res.base, 0, npa->aq.res.entry_sz);
		pool_ctx[0].s.ena = 0;
		pool_ctx[1].s.ena = 1;	/* Write mask */
		__iowmb();

		npa_af_reg_write(npa, CAVM_NPA_AF_AQ_DOOR(), 1);

		start = get_timer(0);
		while ((res->s.compcode == CAVM_NPA_AQ_COMP_E_NOTDONE) &&
		       (get_timer(start) < 1000))
			WATCHDOG_RESET();

		if (res->s.compcode != CAVM_NPA_AQ_COMP_E_GOOD) {
			printf("%s: Error: result 0x%x not good\n",
			       __func__, res->s.compcode);
			return -1;
		}
	}

	for (pool_id = 0; pool_id < pool_count; pool_id++) {
		aq_stat.u = npa_af_reg_read(npa, CAVM_NPA_AF_AQ_STATUS());
		head = aq_stat.s.head_ptr;
		inst = (union cavm_npa_aq_inst_s *)(npa->aq.inst.base) + head;
		res = npa->aq.res.base;

		memset(inst, 0, sizeof(*inst));
		inst->s.cindex = pool_id;
		inst->s.lf = lf;
		inst->s.doneint = 0;
		inst->s.ctype = CAVM_NPA_AQ_CTYPE_E_AURA;
		inst->s.op = CAVM_NPA_AQ_INSTOP_E_WRITE;
		inst->s.res_addr = npa->aq.res.iova;

		/* Context base address */
		aura_ctx = (union cavm_npa_aura_s *)
				(npa->aq.res.base + CONFIG_SYS_CACHELINE_SIZE);
		memset(npa->aq.res.base, 0, npa->aq.res.entry_sz);
		aura_ctx[0].s.ena = 0;
		aura_ctx[1].s.ena = 1;	/* Write mask */
		__iowmb();

		npa_af_reg_write(npa, CAVM_NPA_AF_AQ_DOOR(), 1);

		start = get_timer(0);
		while ((res->s.compcode == CAVM_NPA_AQ_COMP_E_NOTDONE) &&
		       (get_timer(start) < 1000))
			WATCHDOG_RESET();

		if (res->s.compcode != CAVM_NPA_AQ_COMP_E_GOOD) {
			printf("%s: Error: result 0x%x not good\n",
			       __func__, res->s.compcode);
			return -1;
		}
	}

	/* Reset the LF */
	lf_rst.u = 0;
	lf_rst.s.exec = 1;
	lf_rst.s.lf = lf;
	npa_af_reg_write(npa, CAVM_NPA_AF_LF_RST(), lf_rst.u);

	do {
		lf_rst.u = npa_af_reg_read(npa, CAVM_NPA_AF_LF_RST());
		WATCHDOG_RESET();
	} while (lf_rst.s.exec);

	return 0;
}

static int nix_af_setup(struct nix_af_handle *nix_af)
{
	int err;
	union cavm_nixx_af_cfg af_cfg;
	union cavm_nixx_af_ndc_cfg ndc_cfg;
	union cavm_nixx_af_aq_cfg aq_cfg;

	debug("%s(%p)\n", __func__, nix_af);
	err = cavm_rvu_aq_alloc(&nix_af->aq, Q_COUNT(AQ_SIZE),
				sizeof(union cavm_nix_aq_inst_s),
				sizeof(union cavm_nix_aq_res_s));
	if (err) {
		printf("%s: Error allocating nix admin queue\n", __func__);
		return err;
	}

	/* Put in LE mode */
	af_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_CFG());
	if (af_cfg.s.force_cond_clk_en ||
	    af_cfg.s.calibrate_x2p || af_cfg.s.force_intf_clk_en) {
		    printf("%s: Error: Invalid NIX_AF_CFG value 0x%llx\n",
			   __func__, af_cfg.u);
		    return -1;
	}
	af_cfg.s.af_be = 0;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_CFG(), af_cfg.u);

	/* Enable NDC cache */
	ndc_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_NDC_CFG());
	ndc_cfg.s.ndc_ign_pois = 0;
	ndc_cfg.s.byp_sq = 0;
	ndc_cfg.s.byp_sqb = 0;
	ndc_cfg.s.byp_cqs = 0;
	ndc_cfg.s.byp_cints = 0;
	ndc_cfg.s.byp_dyno = 0;
	ndc_cfg.s.byp_mce = 0;
	ndc_cfg.s.byp_rqc = 0;
	ndc_cfg.s.byp_rsse = 0;
	ndc_cfg.s.byp_mc_data = 0;
	ndc_cfg.s.byp_mc_wqe = 0;
	ndc_cfg.s.byp_mr_data = 0;
	ndc_cfg.s.byp_mr_wqe = 0;
	ndc_cfg.s.byp_qints = 0;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_NDC_CFG(), ndc_cfg.u);

	/* Set up queue size */
	aq_cfg.u = 0;
	aq_cfg.s.qsize = AQ_SIZE;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_CFG(), aq_cfg.u);

	/* Set up queue base address */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_BASE(), nix_af->aq.inst.iova);

	return 0;
}

static int nix_attach_receive_queue(struct nix_af_handle *nix_af, int lf,
				    union cavm_nix_rq_ctx_s *desc, u32 index)
{
	union cavm_nix_aq_inst_s *inst;
	volatile union cavm_nix_aq_res_s *res;
	union cavm_nix_rq_ctx_s *context;
	union cavm_nixx_af_aq_status aq_status;
	ulong start;

	debug("%s(%p, %d, %p, %u)\n", __func__, nix_af, lf, desc, index);
	aq_status.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_AQ_STATUS());

	inst = (union cavm_nix_aq_inst_s *)(nix_af->aq.inst.base) +
						aq_status.s.head_ptr;
	memset(inst, 0, sizeof(*inst));
	inst->s.cindex = index;
	inst->s.doneint = 0;
	inst->s.ctype = CAVM_NIX_AQ_CTYPE_E_RQ;
	inst->s.op = CAVM_NIX_AQ_INSTOP_E_INIT;
	inst->s.res_addr = nix_af->aq.res.iova;

	res = nix_af->aq.res.base;

	/* Context base address */
	context = (union cavm_nix_rq_ctx_s *)
			(nix_af->aq.res.base + CONFIG_SYS_CACHELINE_SIZE);
	memset((void *)res, 0, nix_af->aq.res.entry_sz);
	memcpy(context, desc, sizeof(*context));
	__iowmb();

	/* Submit the aura context to HW */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	while ((res->s.compcode == CAVM_NIX_AQ_COMP_E_NOTDONE) &&
	       (get_timer(start) < 1000))
		WATCHDOG_RESET();
	if (res->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
		printf("%s: Bad result code 0x%x\n",
		       __func__, res->s.compcode);
		return -1;
	}
	return 0;
}

static int nix_attach_send_queue(struct nix_af_handle *nix_af, int lf,
				 const union cavm_nix_sq_ctx_s *desc,
				 u32 index)
{
	union cavm_nix_aq_inst_s *inst;
	volatile union cavm_nix_aq_res_s *res;
	union cavm_nix_sq_ctx_s *context;
	union cavm_nixx_af_aq_status aq_stat;
	ulong start;

	debug("%s(%p, %d, %p, %u)\n", __func__, nix_af, lf, desc, index);
	res = (union cavm_nix_aq_res_s *)nix_af->aq.res.base;
	aq_stat.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_AQ_STATUS());
	inst = (union cavm_nix_aq_inst_s *)(nix_af->aq.inst.base) +
							aq_stat.s.head_ptr;
	memset(inst, 0, sizeof(*inst));
	inst->s.cindex = index;
	inst->s.lf = lf;
	inst->s.doneint = 0;
	inst->s.ctype = CAVM_NIX_AQ_CTYPE_E_SQ;
	inst->s.op = CAVM_NIX_AQ_INSTOP_E_INIT;
	inst->s.res_addr = nix_af->aq.res.iova;

	context = (union cavm_nix_sq_ctx_s *)
			(nix_af->aq.res.base + CONFIG_SYS_CACHELINE_SIZE);
	memset((void *)res, 0, nix_af->aq.res.entry_sz);
	memcpy(context, desc, sizeof(*context));
	__iowmb();

	/* Now submit the aura context to HW */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	while ((res->s.compcode == CAVM_NIX_AQ_COMP_E_NOTDONE) &&
	       (get_timer(start) < 1000))
		WATCHDOG_RESET();
	if (res->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
		printf("%s: Bad result code 0x%x\n",
		       __func__, res->s.compcode);
		return -1;
	}
	return 0;
}

static int nix_attach_completion_queue(struct nix_af_handle *nix_af, int lf,
				       const union cavm_nix_cq_ctx_s *desc,
				       u32 index)
{
	union cavm_nix_aq_inst_s *inst;
	volatile union cavm_nix_aq_res_s *res;
	union cavm_nix_cq_ctx_s *context;
	union cavm_nixx_af_aq_status aq_stat;
	ulong start;

	debug("%s(%p, %d, %p, %u)\n", __func__, nix_af, lf, desc, index);
	aq_stat.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_AQ_STATUS());
	inst = (union cavm_nix_aq_inst_s *)(nix_af->aq.inst.base) +
							aq_stat.s.head_ptr;
	res = (union cavm_nix_aq_res_s *)nix_af->aq.res.base;
	memset(inst, 0, sizeof(*inst));
	inst->s.cindex = index;
	inst->s.lf = lf;
	inst->s.doneint = 0;
	inst->s.ctype = CAVM_NIX_AQ_CTYPE_E_CQ;
	inst->s.op = CAVM_NIX_AQ_INSTOP_E_INIT;
	inst->s.res_addr = nix_af->aq.res.iova;

	context = (union cavm_nix_cq_ctx_s *)
			(nix_af->aq.res.base + CONFIG_SYS_CACHELINE_SIZE);
	memset((void *)res, 0, nix_af->aq.res.entry_sz);
	memcpy(context, desc, sizeof(*context));
	__iowmb();

	/* Now submit the aura context to HW */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	while ((res->s.compcode == CAVM_NIX_AQ_COMP_E_NOTDONE) &&
	       (get_timer(start) < 1000))
		WATCHDOG_RESET();
	if (res->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
		printf("%s: Bad result code 0x%x\n",
		       __func__, res->s.compcode);
		return -1;
	}
	return 0;
}

int nix_lf_admin_setup(struct nix_af_handle *nix_af, int lf, int pf,
		       union cavm_nix_cq_ctx_s *cq_descriptors,
		       dma_addr_t cq_dev_addr,
		       u32 cq_count,
		       union cavm_nix_rq_ctx_s *rq_descriptors,
		       dma_addr_t rq_dev_addr,
		       u32 rq_count,
		       union cavm_nix_sq_ctx_s *sq_descriptors,
		       dma_addr_t sq_dev_addr,
		       u32 sq_count)
{
	union cavm_nixx_af_lfx_rqs_cfg rqs_cfg;
	union cavm_nixx_af_lfx_rqs_cfg sqs_cfg;
	union cavm_nixx_af_lfx_rqs_cfg cqs_cfg;
	union cavm_nixx_af_lfx_tx_cfg2 tx_cfg2;
	union cavm_nixx_af_lfx_cfg lfx_cfg;
	union cavm_nixx_af_smqx_cfg smqx_cfg;
	u32 index;
	int err;

	debug("%s(%p, %d, %d, %p, 0x%llx, %u, %p, 0x%llx, %u, %p, 0x%llx, %u)\n",
	      __func__, nix_af, lf, pf, cq_descriptors, cq_dev_addr, cq_count,
	      rq_descriptors, rq_dev_addr, rq_count,
	      sq_descriptors, sq_dev_addr, sq_count);
	/* Request queues */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RQS_BASE(lf), rq_dev_addr);
	rqs_cfg.u = 0;
	rqs_cfg.s.caching = 1;
	rqs_cfg.s.max_queuesm1 = rq_count - 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RQS_CFG(lf), rqs_cfg.u);

	/* Send queues */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_SQS_BASE(lf), sq_dev_addr);
	sqs_cfg.u = 0;
	sqs_cfg.s.caching = 1;
	sqs_cfg.s.max_queuesm1 = sq_count - 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_SQS_CFG(lf), sqs_cfg.u);

	/* Completion queues */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CQS_BASE(lf), cq_dev_addr);
	cqs_cfg.u = 0;
	cqs_cfg.s.caching = 1;
	cqs_cfg.s.max_queuesm1 = cq_count - 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CQS_CFG(lf), cqs_cfg.u);

	/* Enable LMTST for this NIX LF */
	tx_cfg2.u = 0;
	tx_cfg2.s.lmt_ena = 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_TX_CFG2(lf), tx_cfg2.u);

	/* Use 16-word XQEs, write the NPA PF|LF number only */
	lfx_cfg.u = 0;
	lfx_cfg.s.npa_pf_func = pf << 10;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CFG(lf), lfx_cfg.u);

	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RX_CFG(lf), 0);

	for (index = 0; index < cq_count; index++) {
		err = nix_attach_completion_queue(nix_af, lf,
						  &cq_descriptors[index],
						  index);
		if (err) {
			printf("%s: Error attaching completion queue %d\n",
			       __func__, index);
			return err;
		}
	}

	for (index = 0; index < rq_count; index++) {
		err = nix_attach_receive_queue(nix_af, lf,
					       &rq_descriptors[index], index);
		if (err) {
			printf("%s: Error attaching receive queue %d\n",
			       __func__, index);
			return err;
		}
	}

	for (index = 0; index < sq_count; index++) {
		err = nix_attach_send_queue(nix_af, lf,
					    &sq_descriptors[index], index);
		if (err) {
			printf("%s: Error attaching send queue %d\n",
			       __func__, index);
			return err;
		}
	}

	smqx_cfg.u = 0;
	/* Disable shaper control for packets */
	smqx_cfg.s.desc_shp_ctl_dis = 1;
	smqx_cfg.s.minlen = NIX_MIN_HW_MTU;
	smqx_cfg.s.maxlen = NIX_MIN_HW_MTU;
	smqx_cfg.s.lf = lf;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_SMQX_CFG(lf), smqx_cfg.u);

	return 0;
}

int nix_lf_admin_shutdown(struct nix_af_handle *nix_af, int lf,
			  u32 cq_count, u32 rq_count, u32 sq_count)
{
	union cavm_nixx_af_rx_sw_sync sw_sync;
	union cavm_nix_aq_inst_s *inst;
	volatile union cavm_nix_aq_res_s *res;
	union cavm_nix_cq_ctx_s *cq_context;
	union cavm_nix_rq_ctx_s *rq_context;
	union cavm_nix_sq_ctx_s *sq_context;
	union cavm_nixx_af_aq_status aq_status;
	union cavm_nixx_af_lf_rst lf_rst;
	ulong start;
	int index;

	/* Flush all tx packets */
	sw_sync.u = 0;
	sw_sync.s.ena = 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_RX_SW_SYNC(), sw_sync.u);

	do {
		sw_sync.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_RX_SW_SYNC());
		WATCHDOG_RESET();
	} while (sw_sync.s.ena);

	for (index = 0; index < rq_count; index++) {
		aq_status.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_AQ_STATUS());
		inst = (union cavm_nix_aq_inst_s *)(nix_af->aq.inst.base) +
							aq_status.s.head_ptr;
		inst->s.cindex = index;
		inst->s.lf = lf;
		inst->s.doneint = 0;
		inst->s.ctype = CAVM_NIX_AQ_CTYPE_E_RQ;
		inst->s.op = CAVM_NPA_AQ_INSTOP_E_WRITE;
		inst->s.res_addr = nix_af->aq.res.iova;

		res = (union cavm_nix_aq_res_s *)(nix_af->aq.res.base);
		rq_context = nix_af->aq.res.base + CONFIG_SYS_CACHELINE_SIZE;

		memset((void *)res, 0, sizeof(*res));

		rq_context[0].s.ena = 0;	/* Context */
		rq_context[1].s.ena = 1;	/* Mask */
		__iowmb();

		nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_DOOR(), 1);
		start = get_timer(0);
		while ((res->s.compcode == CAVM_NIX_AQ_COMP_E_NOTDONE) &&
		       (get_timer(start) < 1000))
			WATCHDOG_RESET();
		if (res->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
			printf("%s: Bad result code 0x%x\n",
			       __func__, res->s.compcode);
			return -1;
		}
	}

	for (index = 0; index < rq_count; index++) {
		aq_status.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_AQ_STATUS());
		inst = (union cavm_nix_aq_inst_s *)(nix_af->aq.inst.base) +
							aq_status.s.head_ptr;
		inst->s.cindex = index;
		inst->s.lf = lf;
		inst->s.doneint = 0;
		inst->s.ctype = CAVM_NIX_AQ_CTYPE_E_SQ;
		inst->s.op = CAVM_NPA_AQ_INSTOP_E_WRITE;
		inst->s.res_addr = nix_af->aq.res.iova;

		res = nix_af->aq.res.base;
		sq_context = nix_af->aq.res.base + CONFIG_SYS_CACHELINE_SIZE;

		memset((void *)res, 0, sizeof(*res));

		sq_context[0].s.ena = 0;	/* Context */
		sq_context[1].s.ena = 1;	/* Mask */
		__iowmb();

		nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_DOOR(), 1);
		start = get_timer(0);
		while ((res->s.compcode == CAVM_NIX_AQ_COMP_E_NOTDONE) &&
		       (get_timer(start) < 1000))
			WATCHDOG_RESET();
		if (res->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
			printf("%s: Bad result code 0x%x\n",
			       __func__, res->s.compcode);
			return -1;
		}
	}

	for (index = 0; index < rq_count; index++) {
		aq_status.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_AQ_STATUS());
		inst = (union cavm_nix_aq_inst_s *)(nix_af->aq.inst.base) +
							aq_status.s.head_ptr;
		inst->s.cindex = index;
		inst->s.lf = lf;
		inst->s.doneint = 0;
		inst->s.ctype = CAVM_NIX_AQ_CTYPE_E_CQ;
		inst->s.op = CAVM_NPA_AQ_INSTOP_E_WRITE;
		inst->s.res_addr = nix_af->aq.res.iova;

		res = nix_af->aq.res.base;
		cq_context = nix_af->aq.res.base + CONFIG_SYS_CACHELINE_SIZE;

		memset((void *)res, 0, sizeof(*res));

		cq_context[0].s.ena = 0;	/* Context */
		cq_context[1].s.ena = 1;	/* Mask */
		__iowmb();

		nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_DOOR(), 1);
		start = get_timer(0);
		while ((res->s.compcode == CAVM_NIX_AQ_COMP_E_NOTDONE) &&
		       (get_timer(start) < 1000))
			WATCHDOG_RESET();
		if (res->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
			printf("%s: Bad result code 0x%x\n",
			       __func__, res->s.compcode);
			return -1;
		}
	}

	/* Reset the LF */
	lf_rst.u = 0;
	lf_rst.s.lf = lf;
	lf_rst.s.exec = 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LF_RST(), lf_rst.u);

	do {
		lf_rst.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_LF_RST());
		WATCHDOG_RESET();
	} while (lf_rst.s.exec);

	return 0;
}

struct nix_af_handle *nix_af_initialize(int instance, struct udevice *dev,
					void *bar0_ptr, void *bar2_ptr,
					void *npa_bar0_ptr)
{
	struct nix_af_handle *nix_af;
	struct npa_af_handle *npa;
	int err;

	if (instance >= MAX_NIX)
		return NULL;

	nix_af = &nix_afs[instance];
	debug("%s(%d, %s, %p, %p, %p): nix_af: %p\n",
	      __func__, instance, dev->name, bar0_ptr, bar2_ptr, npa_bar0_ptr,
	      nix_af);

	if (nix_afs_references[instance]++) {
		debug("%s: NIX %d already initialized\n", __func__, instance);
		return nix_af;
	}

	nix_af->index = instance;
	nix_af->dev = dev;
	nix_af->nix_af_base = bar0_ptr;
	if (!nix_af->npa_af) {
		nix_af->npa_af = calloc(1, sizeof(struct npa_af_handle));
		if (!nix_af->npa_af) {
			printf("%s: out of memory\n", __func__);
			goto error;
		}
	}
	npa = nix_af->npa_af;
	npa->npa_base = npa_bar0_ptr;

	debug("%s: Setting up npa admin\n", __func__);
	err = npa_setup_admin(nix_af);
	if (err) {
		printf("%s: Error %d setting up NPA admin\n", __func__, err);
		goto error;
	}
	debug("%s: Setting up nix af\n", __func__);
	err = nix_af_setup(nix_af);
	if (err) {
		printf("%s: Error %d setting up NIX admin\n", __func__, err);
		goto error;
	}
	debug("%s: nix_af: %p\n", __func__, nix_af);
	return nix_af;

error:
	nix_afs_references[instance]--;
	if (nix_af->npa_af) {
		free(nix_af->npa_af);
		memset(nix_af, 0, sizeof(*nix_af));
	}
	return NULL;
}

static int nix_af_shutdown(struct nix_af_handle *nix_af)
{
	union cavm_nixx_af_blk_rst blk_rst;

	if (!nix_afs_references[nix_af->index])
		return 0;
	if (--nix_afs_references[nix_af->index])
		return 0;

	blk_rst.u = 0;
	blk_rst.s.rst = 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_BLK_RST(), blk_rst.u);

	/* Wait for reset to complete */
	do {
		blk_rst.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_BLK_RST());
		WATCHDOG_RESET();
	} while (blk_rst.s.busy);

	blk_rst.u = 0;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_BLK_RST(), blk_rst.u);
	cavm_rvu_aq_free(&nix_af->npa_af->aq);
	cavm_rvu_aq_free(&nix_af->aq);

	return 0;
}

static int nix_aq_init(struct nix_af_handle *nix_af)
{
	union cavm_nixx_af_cfg cfg;
	union cavm_nixx_af_ndc_cfg ndc_cfg;
	union cavm_nixx_af_aq_cfg aq_cfg;
	int err;

	if (nix_af->aq.inst.base)
		return 0;

	debug("%s(%p)\n", __func__, nix_af);
	nix_af->xqe_size = CAVM_NIX_XQESZ_E_W16;

	/* Set admin queue endianess */
	cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_CFG());
	cfg.s.af_be = 0;	/* Force little-endian */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_CFG(), cfg.u);

	/* Do not bypass NDC cache */
	ndc_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_NDC_CFG());
	ndc_cfg.s.ndc_ign_pois = 0;
	ndc_cfg.s.byp_sq = 0;
	ndc_cfg.s.byp_sqb = 0;
	ndc_cfg.s.byp_cqs = 0;
	ndc_cfg.s.byp_cints = 0;
	ndc_cfg.s.byp_dyno = 0;
	ndc_cfg.s.byp_mce = 0;
	ndc_cfg.s.byp_rqc = 0;
	ndc_cfg.s.byp_rsse = 0;
	ndc_cfg.s.byp_mc_data = 0;
	ndc_cfg.s.byp_mc_wqe = 0;
	ndc_cfg.s.byp_mr_data = 0;
	ndc_cfg.s.byp_mr_wqe = 0;
	ndc_cfg.s.byp_qints = 0;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_NDC_CFG(), ndc_cfg.u);

	/* Result structure can be followed by RQ/SQ/CQ context at
	 * res + 128 bytes and a write mask at RES + 256 bytes depending on
	 * the operation type.  Alloc sufficient result memory for all
	 * operations.
	 */
	err = cavm_rvu_aq_alloc(&nix_af->aq, Q_COUNT(AQ_SIZE),
				sizeof(union cavm_nix_aq_inst_s),
				ALIGN(sizeof(union cavm_nix_aq_res_s),
				      CONFIG_SYS_CACHELINE_SIZE) + 256);
	if (err)
		return err;

	aq_cfg.u = 0;
	aq_cfg.s.qsize = AQ_SIZE;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_CFG(), aq_cfg.u);
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_BASE(), nix_af->aq.inst.iova);

	return 0;
}

/**
 * Issue a command to the NIX AF Admin Queue
 *
 * @param nix    nix handle
 * @param lf     Logical function number for command
 * @param op     Operation
 * @param ctype  Context type
 * @param cindex Context index
 * @param resp   Result pointer
 *
 * @return	0 for success, -EBUSY on failure
 */
static int nix_aq_issue_command(struct nix_af_handle *nix_af,
				int lf,
				int op,
				int ctype,
				int cindex, union cavm_nix_aq_res_s *resp)
{
	union cavm_nixx_af_aq_status aq_status;
	union cavm_nix_aq_inst_s *aq_inst;
	volatile union cavm_nix_aq_res_s *result = resp;
	ulong start;

	debug("%s(%p, 0x%x, 0x%x, 0x%x, 0x%x, %p)\n", __func__, nix_af, lf,
	      op, ctype, cindex, resp);
	memset(result, 0, sizeof(*result));
	aq_status.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_AQ_STATUS());
	aq_inst = (union cavm_nix_aq_inst_s *)(nix_af->aq.inst.base) +
						aq_status.s.head_ptr;
	aq_inst->u[0] = 0;
	aq_inst->u[1] = 0;
	aq_inst->s.op = op;
	aq_inst->s.ctype = ctype;
	aq_inst->s.lf = lf;
	aq_inst->s.cindex = cindex;
	aq_inst->s.doneint = 0;
	aq_inst->s.res_addr = (u64)resp;
	debug("%s: inst@%p: 0x%llx 0x%llx\n", __func__, aq_inst,
	      aq_inst->u[0], aq_inst->u[1]);
	__iowmb();

	/* Ring doorbell and wait for result */
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_AQ_DOOR(), 1);

	start = get_timer(0);
	/* Wait for completion */
	do {
		WATCHDOG_RESET();
	} while (result->s.compcode == 0 && get_timer(start) < 2);

	if (result->s.compcode != CAVM_NIX_AQ_COMP_E_GOOD) {
		printf("NIX: Admin Queue failed or timed out with code %d after %ld ms\n",
		       result->s.compcode, get_timer(start));
		return -EBUSY;
	}
	return 0;
}

static void nix_get_cgx_lmac_id(u8 map, u8 *cgx_id, u8 *lmac_id)
{
	*cgx_id = (map >> 4) & 0xf;
	*lmac_id = (map & 0xf);
}

int nix_af_get_pf_num(const struct nix_af_handle *nix)
{
	return (((u64)(nix->nix_af_base)) >> 36) & 0x0f;
}

int npc_lf_admin_setup(struct nix_af_handle *nix_af,
		       struct cgx *cgx, u64 link_num)
{
	union cavm_npc_af_const af_const;
	union cavm_npc_af_pkindx_action0 action0;
	union cavm_npc_af_pkindx_action1 action1;
	union cavm_npc_af_intfx_kex_cfg kex_cfg;
	union cavm_npc_af_mcamex_bankx_camx_intf camx_intf;
	union cavm_npc_af_mcamex_bankx_camx_w0 camx_w0;
	union cavm_npc_af_mcamex_bankx_cfg bankx_cfg;
	union cavm_nix_rx_action_s rx_action;
	union cavm_nix_tx_action_s tx_action;
	int pf = nix_af_get_pf_num(nix_af);
	u32 kpus;
	int pkind = link_num;
	int index;

	debug("%s(%p, 0x%llx)\n", __func__, nix_af, link_num);
	if (!cgx) {
		printf("%s: No CGX data found for link number 0x%llx\n",
		       __func__, link_num);
		return -1;
	}
	af_const.u = npc_af_reg_read(nix_af, CAVM_NPC_AF_CONST());
	kpus = af_const.s.kpus;

	action0.u = 0;
	action0.s.parse_done = 1;
	npc_af_reg_write(nix_af, CAVM_NPC_AF_PKINDX_ACTION0(pkind), action0.u);

	action1.u = 0;
	npc_af_reg_write(nix_af, CAVM_NPC_AF_PKINDX_ACTION1(pkind), action1.u);

	kex_cfg.u = 0;
	kex_cfg.s.parse_nibble_ena = 0x07;
	npc_af_reg_write(nix_af,
			 CAVM_NPC_AF_INTFX_KEX_CFG(CAVM_NPC_INTF_E_NIXX_RX(0)),
			 kex_cfg.u);

	camx_intf.u = 0;
	camx_intf.s.intf = ~CAVM_NPC_INTF_E_NIXX_RX(0);
	npc_af_reg_write(nix_af,
			 CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(pkind, 0, 0),
			 camx_intf.u);

	camx_intf.u = 0;
	camx_intf.s.intf = CAVM_NPC_INTF_E_NIXX_RX(0);
	npc_af_reg_write(nix_af,
			 CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(pkind, 0, 1),
			 camx_intf.u);

	camx_w0.u = 0;
	camx_w0.s.md = ~cgx_get_channel_number(cgx, link_num);
	npc_af_reg_write(nix_af,
			 CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W0(pkind, 0, 0),
			 camx_w0.u);

	camx_w0.u = 0;
	camx_w0.s.md = cgx_get_channel_number(cgx, link_num);
	npc_af_reg_write(nix_af,
			 CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W0(pkind, 0, 1),
			 camx_w0.u);

	npc_af_reg_write(nix_af, CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W1(pkind, 0, 0),
			 0);

	npc_af_reg_write(nix_af, CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W1(pkind, 0, 1),
			 0);

	bankx_cfg.u = 0;
	bankx_cfg.s.ena = 1;
	npc_af_reg_write(nix_af, CAVM_NPC_AF_MCAMEX_BANKX_CFG(pkind, 0),
			 bankx_cfg.u);

	rx_action.u = 0;
	rx_action.s.pf_func = pf << 10;
	rx_action.s.op = CAVM_NIX_RX_ACTIONOP_E_UCAST;
	npc_af_reg_write(nix_af, CAVM_NPC_AF_MCAMEX_BANKX_ACTION(pkind, 0),
			 rx_action.u);

	for (index = 0; index < kpus; index++)
		npc_af_reg_write(nix_af, CAVM_NPC_AF_KPUX_CFG(index), 0);

	rx_action.u = 0;
	rx_action.s.pf_func = pf << 10;
	rx_action.s.op = CAVM_NIX_RX_ACTIONOP_E_DROP;
	npc_af_reg_write(nix_af,
			 CAVM_NPC_AF_INTFX_MISS_ACT(CAVM_NPC_INTF_E_NIXX_RX(0)),
			 rx_action.u);
	tx_action.u = 0;
	tx_action.s.op = CAVM_NIX_TX_ACTIONOP_E_UCAST_DEFAULT;
	npc_af_reg_write(nix_af,
			 CAVM_NPC_AF_INTFX_MISS_ACT(CAVM_NPC_INTF_E_NIXX_TX(0)),
			 tx_action.u);

	return 0;
}

static int nix_interface_init(struct nix_af_handle *nix_af,
			      struct nix_handle *nix, u16 pcifunc,
			      int type, int nixlf)
{
	union cavm_rvu_pf_func_s pf_func;
	struct cgx *cgx = nix->lmac->cgx;
	int err;
	int pkind, pf, lmac_cnt;
	u64 tx_credit;
	u16 link;
	u8 cgx_id = nix->lmac->cgx->cgx_id;
	u8 lmac_id = nix->lmac->lmac_id;

	pf = rvu_get_pf(pcifunc);

	debug("%s(%p, %p, 0x%x, 0x%x, 0x%x) pf: 0x%x\n", __func__, nix_af, nix,
	      pcifunc, type, nixlf, pf);

	switch (type) {
	case NIX_INTF_TYPE_CGX:
		pkind = npc_get_pkind(nix_af, pf);
		if (pkind < 0) {
			printf("%s: Error: invalid pkind 0x%x for pf 0x%x\n",
			       __func__, pkind, pf);
			return -EINVAL;
		}
		cgx_set_pkind(cgx, lmac_id, pkind);
		link = NIX_LINK_CGX_LMAC(cgx_id, lmac_id);
		break;
	case NIX_INTF_TYPE_LBK:
		link = NIX_LINK_LBK(0);
		break;
	}
	return 0;
}

struct nix_handle *cavm_nix_lf_alloc(struct nix_af_handle *nix_af,
				     struct udevice *dev,
				     u16 pcifunc,
				     u16 nix_lf,
				     void __iomem *nix_base,
				     void __iomem *npc_base,
				     void __iomem *lmt_base,
				     int cgx_id, int lmac_id,
				     struct nix_lf_alloc_req *req,
				     struct nix_lf_alloc_rsp *rsp)
{
	union cavm_nixx_af_sq_const sq_const;
	union cavm_nixx_af_const2 af_const2;
	union cavm_nixx_af_const3 af_const3;
	union cavm_nixx_af_lfx_rqs_cfg rqs_cfg;
	union cavm_nixx_af_lfx_sqs_cfg sqs_cfg;
	union cavm_nixx_af_lfx_cqs_cfg cqs_cfg;
	union cavm_nixx_af_lfx_rss_cfg rss_cfg;
	union cavm_nixx_af_lfx_cints_cfg cints_cfg;
	union cavm_nixx_af_lfx_qints_cfg qints_cfg;
	union cavm_nixx_af_lfx_rss_grpx rss_grp;
	union cavm_nixx_af_lfx_tx_cfg2 tx_cfg2;
	union cavm_nixx_af_lfx_cfg lfx_cfg;
	int idx, hwctx_size;
	int qints;
	struct nix_handle *nix;
	int err;
	static int instance = 0;

	debug("%s(%p, %s, 0x%x, 0x%x, %p, %p, %p, 0x%x, 0x%x, %p, %p)\n",
	      __func__, nix_af, dev->name, pcifunc, nix_lf, nix_base, npc_base,
	      lmt_base, cgx_id, lmac_id, req, rsp);

	if (!nix_lf)
		return NULL;

	if (!req->rq_cnt || !req->sq_cnt || !req->cq_cnt)
		return NULL;

	af_const3.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_CONST3());
	hwctx_size = 1ULL << af_const3.s.rq_ctx_log2bytes;

	nix = (struct nix_handle *)calloc(1, sizeof(*nix));
	if (!nix) {
		printf("%s: Out of memory\n", __func__);
		return NULL;
	}
	nix->nic_id = instance++;
	nix->nix_af = nix_af;
	nix->nix_base = nix_base;
	nix->npc_base = npc_base;
	nix->lmt_base = lmt_base;
	nix->lf = nix_lf;
	nix->dev = dev;
	nix->pf = pcifunc;
	nix->lmac = cgx_get_lmac(pcifunc - 1);
	if (!nix->lmac) {
		printf("%s: Error: could not find lmac for pf %d\n",
		       __func__, nix->pf);
		free(nix);
		return NULL;
	}

	/* Alloc NIX RQ HW context memory and config base */
	err = qmem_alloc(&(nix->rq), req->rq_cnt, hwctx_size);
	if (err)
		goto error;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RQS_BASE(nix_lf),
			 nix->rq.iova);

	/* Set caching and queue count in HW */
	rqs_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_LFX_RQS_CFG(nix_lf));
	rqs_cfg.s.caching = 1;
	rqs_cfg.s.max_queuesm1 = req->rq_cnt - 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RQS_CFG(nix_lf), rqs_cfg.u);

	/* Alloc NIX SQ HW context memory and config the base */
	hwctx_size = 1ULL << af_const3.s.sq_ctx_log2bytes;
	err = qmem_alloc(&(nix->sq), req->sq_cnt, hwctx_size);
	if (err)
		goto error;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_SQS_BASE(nix_lf),
			 nix->sq.iova);
	sqs_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_LFX_SQS_CFG(nix_lf));
	sqs_cfg.s.caching = 1;
	sqs_cfg.s.max_queuesm1 = req->sq_cnt - 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_SQS_CFG(nix_lf), sqs_cfg.u);

	/* Alloc NIX CQ HW context memory and config the base */
	hwctx_size = 1ULL << af_const3.s.cq_ctx_log2bytes;
	for (idx = 0; idx < NIX_CQ_COUNT; idx++) {
		err = qmem_alloc(&(nix->cq[idx]), req->cq_cnt, hwctx_size);
		if (err)
			goto error;
	}
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CQS_BASE(nix_lf),
			 nix->cq[NIX_CQ_TX].iova);
	cqs_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_LFX_CQS_CFG(nix_lf));
	cqs_cfg.s.caching = 1;
	cqs_cfg.s.max_queuesm1 = req->cq_cnt - 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CQS_CFG(nix_lf), cqs_cfg.u);

	/* Alloc NIX RSS HW context memory and config the base */
	hwctx_size = 1ULL << af_const3.s.rsse_log2bytes;
	err = qmem_alloc(&(nix->rss), req->rss_sz * req->rss_grps, hwctx_size);
	if (err)
		goto error;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RSS_BASE(nix_lf),
			 nix->rss.iova);
	rss_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_LFX_RSS_CFG(nix_lf));
	rss_cfg.s.ena = 1;
	rss_cfg.s.size = ilog2(req->rss_sz) / 256;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RSS_CFG(nix_lf), rss_cfg.u);

	for (idx = 0; idx < req->rss_grps; idx++) {
		rss_grp.u = 0;
		rss_grp.s.sizem1 = 0x7;
		rss_grp.s.offset = req->rss_sz * idx;
		nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_RSS_GRPX(nix_lf, idx),
				 rss_grp.u);
	}

	/* Alloc memory for CQints HW contextxs */
	af_const2.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_CONST2());
	qints = af_const2.s.cints;
	hwctx_size = 1ULL << af_const3.s.cint_log2bytes;
	err = qmem_alloc(&nix->cq_ints, qints, hwctx_size);
	if (err)
		goto error;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CINTS_BASE(nix_lf),
			 nix->cq_ints.iova);
	cints_cfg.u = nix_af_reg_read(nix_af,
				      CAVM_NIXX_AF_LFX_CINTS_CFG(nix_lf));
	cints_cfg.s.caching = 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CINTS_CFG(nix_lf),
			 cints_cfg.u);

	/* Alloc memory for Qints HW contexts */
	qints = af_const2.s.qints;
	hwctx_size = 1ULL << af_const3.s.qint_log2bytes;
	err = qmem_alloc(&(nix->qints), qints, hwctx_size);
	if (err)
		goto error;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_QINTS_BASE(nix_lf),
			 nix->qints.iova);
	qints_cfg.u = nix_af_reg_read(nix_af,
				      CAVM_NIXX_AF_LFX_QINTS_CFG(nix_lf));
	qints_cfg.s.caching = 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_QINTS_CFG(nix_lf),
			 qints_cfg.u);

	/* Enable LMTST for this NIX LF */
	tx_cfg2.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_LFX_TX_CFG2(nix_lf));
	tx_cfg2.s.lmt_ena = 1;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_TX_CFG2(nix_lf), tx_cfg2.u);

	lfx_cfg.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_LFX_CFG(nix_lf));
	lfx_cfg.s.xqe_size = req->xqe_sz;
	lfx_cfg.s.npa_pf_func = pcifunc;
	lfx_cfg.s.sso_pf_func = pcifunc;
	nix_af_reg_write(nix_af, CAVM_NIXX_AF_LFX_CFG(nix_lf), lfx_cfg.u);

	/* Config Rx pkt length, csum checks and apad enable/disable */

	sq_const.u = nix_af_reg_read(nix_af, CAVM_NIXX_AF_SQ_CONST());
	rsp->sqb_size = sq_const.s.sqb_size;
	rsp->chan_base = CAVM_NIX_CHAN_E_CGXX_LMACX_CHX(cgx_id, lmac_id, 0);
	rsp->chan_cnt = 1;

	err = nix_lf_alloc_cq(nix_af, nix);
	if (err) {
		printf("%s: Error %d allocating completion queue for pf %d\n",
		       __func__, err, pcifunc);
		return NULL;
	}
#if 0
	rsp->lso_tsov4_idx = NIX_LSO_FORMAT_IDX_TSOV4;
	rsp->lso_tsov6_idx = NIX_LSO_FORMAT_IDX_TSOV6;
#endif
	list_add(&nix->nix_list, &nix_list);

	return nix;

error:
	qmem_free(&(nix->rq));
	qmem_free(&(nix->sq));
	for (idx = 0; idx < NIX_CQ_COUNT; idx++)
		qmem_free(&(nix->cq[idx]));
	qmem_free(&(nix->rss));
	qmem_free(&(nix->cq_ints));
	qmem_free(&(nix->qints));
	return NULL;
}

/**
 * Allocate and setup a new Completion Queue for use
 *
 * @param nix_af	Handle for admin function
 * @param nix		Handle for pf
 *
 * @return Completion Queue number, or negative on failure
 */
static int nix_lf_alloc_cq(struct nix_af_handle *nix_af, struct nix_handle *nix)
{
	struct nix_aq_cq_request aq_request ALIGNED;
	int cq = af_state.next_free_cq++;
	int err;

	err = cavm_rvu_aq_alloc(&nix_af->aq, Q_COUNT(AQ_SIZE),
			sizeof(union cavm_nix_aq_inst_s),
			ALIGN(sizeof(union cavm_nix_aq_res_s), 128) + 256);

	if (err) {
		printf("%s: Error %d allocating completion queue\n",
		       __func__, err);
		return err;
	}

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
	aq_request.cq.s.base = nix->cq[NIX_CQ_TX].iova;

	err = nix_aq_issue_command(nix_af, CAVM_NIX_AQ_INSTOP_E_INIT,
				   CAVM_NIX_AQ_CTYPE_E_CQ, nix->lf, cq,
				   &aq_request.resp);
	if (err) {
		printf("%s: Error requesting completion queue\n", __func__);
		return err;
	}
	debug("%s: CQ(%d) allocated, base %p, %p\n", __func__, cq,
	      nix->cq[NIX_CQ_TX].base, nix->cq[NIX_CQ_RX].base);

	nix->cq_idx = cq;

	return nix->cq_idx;
}
#if 0
int nix_af_setup_tx_resources(struct nix_af_handle *nix)
{
	struct rvu_hwinfo *hw = nix->hw;
	struct nix_txsch *txsch;
	union cavm_nixx_af_const af_const;
	union cavm_nixx_af_tl1_const tl_const;
	u64 reg;
	int err, lvl;

	/* Set number of links of each type */
	af_const.u = nix_af_reg_read(nix, CAVM_NIXX_AF_CONST());
	hw->cgx = af_const.s.num_cgx;
	hw->lmac_per_cgx = af_const.s.cgx_lmacs;
	hw->cgx_links = hw->cgx * hw->lmac_per_cgx;
	hw->lbk_links = 1;
	hw->sdp_links = 1;

	for (lvl = 0; lvl < NIX_TXSCH_LVL_CNT; lvl++) {
		txsch = &hw->txsch[lvl];
		txsch->lvl = lvl;
		switch (lvl) {
		case NIX_TXSCH_LVL_SMQ:
			reg = CAVM_NIXX_AF_MDQ_CONST();
			break;
		case NIX_TXSCH_LVL_TL4:
			reg = CAVM_NIXX_AF_TL4_CONST();
			break;
		case NIX_TXSCH_LVL_TL3:
			reg = CAVM_NIXX_AF_TL3_CONST();
			break;
		case NIX_TXSCH_LVL_TL2:
			reg = CAVM_NIXX_AF_TL2_CONST();
			break;
		case NIX_TXSCH_LVL_TL1:
			reg = CAVM_NIXX_AF_TL1_CONST();
			break;
		}
		tl_const.u = nix_af_reg_read(reg);
		txsch->rsrc.max = tl_const.s.count;
		err = rvu_alloc_bitmap(&txsch->rsrc);
		if (err)
			return err;

		/* Allocate mem for scheduler to PF/VF pcifunc mapping info */
		txsch->pfvf_map = calloc(txsch->rsrc.max, sizeof(u16));
		if (!txsch->pfvf_map)
			return -ENOMEM;
	}
	return 0;
}
#endif
