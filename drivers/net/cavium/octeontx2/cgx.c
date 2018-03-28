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

#include "cgx_intf.h"


void enumerate_lmacs(void)
{

}

int cgx_probe(struct udevice *dev)
{
	struct cgx *p_cgx = dev_get_priv(dev);
	size_t size;

	p_cgx->base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);

	debug("CGX BAR %p\n", p_cgx->base);

	enumerate_lmacs();

	return 0;
}

int cgx_remove(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id cgx_ids[] = {
        { .compatible = "cavium,cgx" },
        {}
};

U_BOOT_DRIVER(cgx) = {
        .name   = "cgx",
        .id     = UCLASS_MISC,
        .probe  = cgx_probe,
	.remove	= cgx_remove,
        .of_match = cgx_ids,
        .priv_auto_alloc_size = sizeof(struct cgx),
};

static struct pci_device_id cgx_supported[] = {
        { PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_OCTEONTX2_CGX) },
        {}
};

U_BOOT_PCI_DEVICE(cgx, cgx_supported);
