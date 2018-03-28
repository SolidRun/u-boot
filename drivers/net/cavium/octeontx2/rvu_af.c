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

int rvu_af_probe(struct udevice *dev)
{
	struct rvu_af *af_ptr = dev_get_priv(dev);
	size_t size;
	union cavm_rvu_af_addr_s func_addr;

	af_ptr->base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);
	debug("RVU AF BAR0 %p\n", af_ptr->base);
	af_ptr->bar2 = dm_pci_map_bar(dev, 2, &size, PCI_REGION_MEM);
	debug("RVU AF BAR2 %p\n", af_ptr->bar2);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NIXX(0);
	af_ptr->nix_af_base = af_ptr->base + func_addr.u;
	debug("RVU AF BAR0 NIX BASE %p\n", af_ptr->nix_af_base);
	//nix_af_init(af_ptr->nix_af_base);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NPA;
	af_ptr->npa_af_base = af_ptr->base + func_addr.u;
	debug("RVU AF BAR0 NPA BASE %p\n", af_ptr->npa_af_base);

	return 0;
}


static const struct udevice_id rvu_af_ids[] = {
        { .compatible = "cavium,rvu-af" },
        {}
};

U_BOOT_DRIVER(rvu_af) = {
        .name   = "rvu_af",
        .id     = UCLASS_MISC,
        .probe  = rvu_af_probe,
        .of_match = rvu_af_ids,
        .priv_auto_alloc_size = sizeof(struct rvu_af),
};

static struct pci_device_id rvu_af_supported[] = {
        { PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_OCTEONTX2_RVU_AF) },
        {}
};

U_BOOT_PCI_DEVICE(rvu_af, rvu_af_supported);
