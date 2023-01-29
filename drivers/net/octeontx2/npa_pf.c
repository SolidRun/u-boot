// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2022 Marvell
 */

#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>
#include <pci_ids.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/arch/board.h>
#include "npa_pf.h"

static int initialized;

u64 npa_pf_aura_op_alloc(struct npa_pf *npa_pf)
{
	u64 aura_id = NPA_POOL_INST;
	struct npa *npa = npa_pf->npa;
	union npa_lf_aura_op_allocx op_allocx;

	op_allocx.u = atomic_fetch_and_add64_nosync(npa->npa_base +
			NPA_LF_AURA_OP_ALLOCX(0), aura_id);
	return op_allocx.s.addr;
}

void *npa_memalloc(int num_elements, size_t elem_size, const char *msg)
{
	size_t alloc_size = num_elements * elem_size;
	void *base = memalign(CONFIG_SYS_CACHELINE_SIZE, alloc_size);

	if (!base)
		printf("NPA: Mem alloc failed for %s (%d * %zu = %zu bytes)\n",
		       msg ? msg : __func__, num_elements, elem_size,
		       alloc_size);
	else
		memset(base, 0, alloc_size);

	debug("NPA: Memory alloc for %s (%d * %zu = %zu bytes) at %p\n",
	      msg ? msg : __func__, num_elements, elem_size, alloc_size, base);
	return base;
}

int npa_pf_init(struct npa_pf *npa_pf)
{
	union npa_af_const npa_af_const;
	union npa_aura_s *aura;
	union npa_pool_s *pool;
	union rvu_func_addr_s block_addr;
	struct npa *npa;
	struct rvu_af *rvu_af;
	int idx = NPA_POOL_INST;
	int stack_page_pointers;
	int stack_page_bytes;
	int err;

	npa = (struct npa *)calloc(1, sizeof(struct npa));
	if (!npa) {
		printf("%s: out of memory for npa instance\n", __func__);
		return -ENOMEM;
	}
	npa_pf->npa = npa;
	rvu_af = dev_get_priv(npa_pf->afdev);

	block_addr.u = 0;
	block_addr.s.block = RVU_BLOCK_ADDR_E_NPA;
	npa->npa_base = npa_pf->pf_base + block_addr.u;
	npa->npa_af = rvu_af->nix_af[0]->npa_af;
	npa->lf = npa_pf->lf_id;

	npa_af_const.u = npa_af_reg_read(npa->npa_af, NPA_AF_CONST());
	stack_page_pointers = npa_af_const.s.stack_page_ptrs;
	stack_page_bytes = npa_af_const.s.stack_page_bytes;

	npa->stack_pages[NPA_POOL_INST] =
		(INST_QLEN + stack_page_pointers - 1) /	stack_page_pointers;
	npa->pool_stack_pointers = stack_page_pointers;

	npa->q_len[NPA_POOL_INST] = INST_QLEN;

	npa->buf_size[NPA_POOL_INST] = 1024;

	npa->aura_ctx = npa_memalloc(NPA_POOL_COUNT,
				     sizeof(union npa_aura_s),
				     "aura context");
	if (!npa->aura_ctx) {
		printf("%s: Out of memory for aura context\n", __func__);
		return -ENOMEM;
	}

	npa->pool_ctx[idx] = npa_memalloc(1, sizeof(union npa_pool_s),
					  "pool context");
	if (!npa->pool_ctx[idx]) {
		printf("%s: Out of memory for pool context\n",
		       __func__);
		return -ENOMEM;
	}
	npa->pool_stack[idx] = npa_memalloc(npa->stack_pages[idx],
					    stack_page_bytes,
					    "pool stack");
	if (!npa->pool_stack[idx]) {
		printf("%s: Out of memory for pool stack\n", __func__);
		return -ENOMEM;
	}

	err = npa_lf_admin_setup(npa, npa->lf, (dma_addr_t)npa->aura_ctx);
	if (err) {
		printf("%s: Error setting up NPA LF admin for lf %d\n",
		       __func__, npa->lf);
		return err;
	}

	/* Set up the auras */
	aura = npa->aura_ctx + (idx * sizeof(union npa_aura_s));
	pool = npa->pool_ctx[idx];
	debug("%s aura %p pool %p\n", __func__, aura, pool);
	memset(aura, 0, sizeof(union npa_aura_s));
	aura->s.fc_ena = 0;
	aura->s.pool_addr = (u64)npa->pool_ctx[idx];
	debug("%s aura.s.pool_addr %llx pool_addr %p\n", __func__,
	      aura->s.pool_addr, npa->pool_ctx[idx]);
	aura->s.shift = 64 - __builtin_clzll(npa->q_len[idx]) - 8;
	aura->s.count = npa->q_len[idx];
	aura->s.limit = npa->q_len[idx];
	aura->s.ena = 1;
	err = npa_attach_aura(npa->npa_af, npa->lf, aura, idx);
	if (err)
		return err;

	memset(pool, 0, sizeof(*pool));
	pool->s.fc_ena = 0;
	pool->s.nat_align = 0;
	pool->s.stack_base = (u64)(npa->pool_stack[idx]);
	debug("%s pool.s.stack_base %llx stack_base %p\n", __func__,
	      pool->s.stack_base, npa->pool_stack[idx]);
	pool->s.buf_size =
		npa->buf_size[idx] / CONFIG_SYS_CACHELINE_SIZE;
	pool->s.stack_max_pages = npa->stack_pages[idx];
	pool->s.shift =
		64 - __builtin_clzll(npa->pool_stack_pointers) - 8;
	pool->s.ptr_start = 0;
	pool->s.ptr_end = (1ULL << 40) -  1;
	pool->s.ena = 1;
	err = npa_attach_pool(npa->npa_af, npa->lf, pool, idx);
	if (err)
		return err;

	npa->buffers[idx] = npa_memalloc(npa->q_len[idx],
					 sizeof(void *),
					 "buffers");
	if (!npa->buffers[idx]) {
		printf("%s: Out of memory\n", __func__);
		return -ENOMEM;
	}

	err = npa_setup_pool(npa, idx, npa->buf_size[idx],
			     npa->q_len[idx], npa->buffers[idx]);
	if (err) {
		printf("%s: Error setting up pool %d\n",
		       __func__, idx);
		return err;
	}
	return 0;
}

int npa_pf_shutdown(struct npa_pf *npa_pf)
{
	int err, pool;
	struct npa *npa = npa_pf->npa;

	err = npa_lf_admin_shutdown(npa->npa_af, npa->lf, 1);
	if (err) {
		printf("%s: Error %d shutting down NPA LF admin\n",
		       __func__, err);
		return err;
	}
	free(npa->aura_ctx);
	npa->aura_ctx = NULL;

	free(npa->pool_ctx[pool]);
	npa->pool_ctx[pool] = NULL;
	free(npa->pool_stack[pool]);
	npa->pool_stack[pool] = NULL;

	return 0;
}

int npa_pf_probe(struct udevice *dev)
{
	struct npa_pf *npa_pf = dev_get_priv(dev);
	int err;
	char name[16];
	struct rvu_af *rvu_af;
	union npa_priv_lfx_cfg npa_lfx_cfg;
	union rvu_priv_pfx_npa_cfg rvu_pfx_npa_cfg;
	void __iomem *npa_af_base;

	/* Init only one NPA PF */
	if (initialized)
		return 0;

	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
				 PCI_DEVICE_ID_CAVIUM_RVU_AF, 0,
				 &npa_pf->afdev);
	if (err) {
		printf("%s RVU AF device not found\n", __func__);
		return 0;
	}
	rvu_af = dev_get_priv(npa_pf->afdev);
	npa_af_base = rvu_af->nix_af[0]->npa_af->npa_af_base;

	npa_pf->pf_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_2, 0, 0,
					 PCI_REGION_TYPE, PCI_REGION_MEM);
	npa_pf->dev = dev;

	npa_pf->pf_id = ((u64)npa_pf->pf_base >> 36) & 0xf;
	npa_pf->lf_id = NPA_PF_LF_ID;

	/* Allocate LF to NPA PF */
	rvu_pfx_npa_cfg.u = 0;
	rvu_pfx_npa_cfg.s.has_lf = 1;
	writeq(rvu_pfx_npa_cfg.u,
	       rvu_af->af_base + RVU_PRIV_PFX_NPA_CFG(npa_pf->pf_id));

	npa_lfx_cfg.u = 0;
	npa_lfx_cfg.s.ena = 1;
	npa_lfx_cfg.s.pf_func = (((npa_pf->pf_id & 0x3f) << 10) | 0x0);
	npa_lfx_cfg.s.slot = 0;
	writeq(npa_lfx_cfg.u,
	       npa_af_base + NPA_PRIV_LFX_CFG(npa_pf->lf_id));

	err = npa_pf_init(npa_pf);
	if (err)
		printf("%s: Error %d initializing npa_pf\n", __func__, err);

	/*
	 * modify device name to include index/sequence number,
	 * for better readability, this is 1:1 mapping with eth0/1/2.. names.
	 */
	sprintf(name, "npa_pf#%d", dev_seq(dev));
	device_set_name(dev, name);
	debug("%s: name: %s\n", __func__, dev->name);

	initialized = 1;
	return err;
}

int npa_pf_remove(struct udevice *dev)
{
	struct npa_pf *npa_pf = dev_get_priv(dev);

	npa_pf_shutdown(npa_pf);

	debug("%s: npa pf%d down --\n", __func__,  npa_pf->pf_id);

	return 0;
}

U_BOOT_DRIVER(npa_pf) = {
	.name   = "npa_pf",
	.id     = UCLASS_MISC,
	.probe	= npa_pf_probe,
	.remove = npa_pf_remove,
	.priv_auto = sizeof(struct npa_pf),
};

static struct pci_device_id npa_pf_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_NPA_PF) },
	{}
};

U_BOOT_PCI_DEVICE(npa_pf, npa_pf_supported);
