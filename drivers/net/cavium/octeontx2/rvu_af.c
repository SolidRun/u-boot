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
#include <errno.h>
#include <linux/list.h>
#include <asm/io.h>
#include <asm/arch/octeontx2.h>
#include "cavm-csrs-rvu.h"
#include "rvu.h"
#include "rvu_common.h"
#include "nix.h"

static LIST_HEAD(nix_af_list);

/**
 * Given the PF base address, return the NIX AF
 *
 * @param nix_pf_base		NIX PF base address
 *
 * @return	nix_af handle or NULL if not found.
 */
struct nix_af_handle *nix_get_af(u64 nix_pf_base)
{
	struct nix_af_handle *nix_af;
	static const u64 mask = ~(0xffffffffff);
	nix_pf_base &= mask;

	list_for_each_entry(nix_af, &nix_af_list, nix_af_list) {
		if (((u64)(nix_af->nix_af_base) & mask) == (nix_pf_base & mask))
			return nix_af;
	}
	debug("%s: No NIX AF found for address 0x%llx\n", __func__,
	      nix_pf_base);
	return NULL;
}

/**
 * Allocates an admin queue for instructions and results
 *
 * @param	aq	admin queue to allocate for
 * @param	qsize	Number of entries in the queue
 * @param	inst_size	Size of each instruction
 * @param	res_size	Size of each result
 *
 * @return	-ENOMEM on error, 0 on success
 */
int cavm_rvu_aq_alloc(struct admin_queue *aq, unsigned qsize,
		      size_t inst_size, size_t res_size)
{
	int err;

	err = qmem_alloc(&aq->inst, qsize, inst_size);
	if (err)
		return err;
	err = qmem_alloc(&aq->res, qsize, res_size);
	if (err)
		qmem_free(&aq->inst);

	return err;
}

/**
 * Frees an admin queue
 *
 * @param	aq	Admin queue to free
 */
void cavm_rvu_aq_free(struct admin_queue *aq)
{
	qmem_free(&aq->inst);
	qmem_free(&aq->res);
	memset(aq, 0, sizeof(*aq));
}

int cavm_rvu_af_probe(struct udevice *dev)
{
	struct rvu_af *af_ptr = dev_get_priv(dev);
	struct nix_af_handle *nix_af;
	size_t size;
	union cavm_rvu_af_addr_s func_addr;
	static int instance = 0;

	debug("%s(%s) instance: %d\n", __func__, dev->name, instance);
	af_ptr->base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);
	debug("RVU AF BAR0 %p\n", af_ptr->base);
	af_ptr->bar2 = dm_pci_map_bar(dev, 2, &size, PCI_REGION_MEM);
	debug("RVU AF BAR2 %p\n", af_ptr->bar2);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NIXX(0);
	af_ptr->nix_af_base = af_ptr->base + func_addr.u;
	debug("RVU AF BAR0 NIX BASE %p\n", af_ptr->nix_af_base);
	af_ptr->nix_af_bar2 = af_ptr->bar2 + func_addr.u;
	debug("RVU AF BAR2 NIX BASE %p\n", af_ptr->nix_af_bar2);
	//nix_af_init(af_ptr->nix_af_base);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NPA;
	af_ptr->npa_af_base = af_ptr->base + func_addr.u;
	debug("RVU AF BAR0 NPA BASE %p\n", af_ptr->npa_af_base);
	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NPC;
	af_ptr->npc_af_base = af_ptr->base + func_addr.u;
	debug("RVU AF BAR0 NPC BASE %p\n", af_ptr->npc_af_base);

	debug("%s: Initializing nix instance %d\n", __func__, instance);
	nix_af = nix_af_initialize(instance++, dev,
				   af_ptr->nix_af_base, 0,
				   af_ptr->npa_af_base);
	if (!nix_af) {
		printf("%s: Error: could not initialize NIX AF\n", __func__);
		return -1;
	}
	debug("%s: Adding list, nix_af: %p\n", __func__, nix_af);
	list_add(&nix_af->nix_af_list, &nix_af_list);
	af_ptr->nix_af = nix_af;
	debug("%s: Done\n", __func__);

	return 0;
}

static const struct udevice_id rvu_af_ids[] = {
        { .compatible = "cavium,rvu-af" },
        {}
};

U_BOOT_DRIVER(rvu_af) = {
        .name   = "rvu_af",
        .id     = UCLASS_MISC,
        .probe  = cavm_rvu_af_probe,
        .of_match = rvu_af_ids,
        .priv_auto_alloc_size = sizeof(struct rvu_af),
};

static struct pci_device_id rvu_af_supported[] = {
        { PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_OCTEONTX2_RVU_AF) },
        {}
};

U_BOOT_PCI_DEVICE(rvu_af, rvu_af_supported);
