/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <common.h>
#include <net.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/types.h>
#include <asm/arch/octeontx2.h>
#include "cgx.h"
#include "nix.h"

extern struct udevice *rvu_af_dev;

int rvu_pf_init(struct rvu_pf *rvu)
{
	struct nix *nix;
	struct eth_pdata *pdata = dev_get_platdata(rvu->dev);

	debug("%s: Allocating nix lf\n", __func__);
	nix = nix_lf_alloc(rvu->dev);
	if (!nix) {
		printf("%s: Error allocating lf for pf %d\n",
		       __func__, rvu->pfid);
		return -1;
	}
	rvu->nix = nix;

	/* to make post_probe happy */
	memcpy(pdata->enetaddr, nix->lmac->mac_addr, 6);
	eth_env_set_enetaddr_by_index("eth", rvu->dev->seq, pdata->enetaddr);

	return 0;
}

static const struct eth_ops nix_eth_ops = {
	.start			= nix_lf_init,
	.send			= nix_lf_xmit,
	.recv			= nix_lf_recv,
	.free_pkt		= nix_lf_free_pkt,
	.stop			= nix_lf_halt,
	.write_hwaddr		= nix_lf_setup_mac,
	.read_rom_hwaddr	= nix_lf_read_rom_mac,
};

int rvu_pf_probe(struct udevice *dev)
{
	struct rvu_pf *rvu = dev_get_priv(dev);
	size_t size;
	int err;

	debug("%s: name: %s\n", __func__, dev->name);

	rvu->pf_base = dm_pci_map_bar(dev, 2, &size, PCI_REGION_MEM);
	rvu->pfid = dev->seq + 1;
	rvu->dev = dev;
	if (!rvu_af_dev) {
		printf("%s: Error: Could not find RVU AF device\n",
		       __func__);
		return -1;
	}
	rvu->afdev = rvu_af_dev;

	debug("RVU PF %u BAR2 %p\n", rvu->pfid, rvu->pf_base);

	rvu_get_lfid_for_pf(rvu->pfid, &rvu->nix_lfid,
				 &rvu->npa_lfid);

	err = rvu_pf_init(rvu);
	if (err)
		printf("%s: Error %d adding nix\n", __func__, err);

	return err;
}

int rvu_pf_remove(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id rvu_pf_ids[] = {
        { .compatible = "cavium,rvu-pf" },
        {}
};

U_BOOT_DRIVER(rvu_pf) = {
        .name   = "rvu_pf",
        .id     = UCLASS_ETH,
        .of_match = rvu_pf_ids,
        .probe  = rvu_pf_probe,
        .remove = rvu_pf_remove,
	.ops    = &nix_eth_ops,
        .priv_auto_alloc_size = sizeof(struct rvu_pf),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static struct pci_device_id rvu_pf_supported[] = {
        { PCI_VDEVICE(CAVIUM, PCI_DEVID_OCTEONTX2_RVU_PF) },
        {}
};

U_BOOT_PCI_DEVICE(rvu_pf, rvu_pf_supported);

