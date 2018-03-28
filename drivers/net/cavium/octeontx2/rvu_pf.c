/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <errno.h>

#include "rvu.h"

static int pfid = 1;

int rvu_pf_probe(struct udevice *dev)
{
	struct rvu_pf *pf_ptr = dev_get_priv(dev);
	size_t size;
	union cavm_rvu_func_addr_s func_addr;

	pf_ptr->base = dm_pci_map_bar(dev, 2, &size, PCI_REGION_MEM);
	pf_ptr->pf_id = pfid++;

	debug("RVU PF BAR2 %p\n", pf_ptr->base);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NIXX(0);
	pf_ptr->nix_base = pf_ptr->base + func_addr.u;
	debug("RVU PF BAR2 NIX BASE %p\n", pf_ptr->nix_base);
	//nix_lf_init(pf_ptr->pf_id, pf_ptr->nix_base);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NPA;
	pf_ptr->npa_base = pf_ptr->base + func_addr.u;
	debug("RVU PF BAR2 NPA BASE %p\n", pf_ptr->npa_base);

	return 0;
}


static const struct udevice_id rvu_pf_ids[] = {
        { .compatible = "cavium,rvu-pf" },
        {}
};

U_BOOT_DRIVER(rvu_pf) = {
        .name   = "rvu_pf",
        .id     = UCLASS_MISC,
        .probe  = rvu_pf_probe,
        .of_match = rvu_pf_ids,
        .priv_auto_alloc_size = sizeof(struct rvu_pf),
};

static struct pci_device_id rvu_pf_supported[] = {
        { PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_OCTEONTX2_RVU_PF) },
        {}
};

U_BOOT_PCI_DEVICE(rvu_pf, rvu_pf_supported);


