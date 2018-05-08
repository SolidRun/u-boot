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
#include <asm/io.h>
#include <errno.h>
#include <asm/types.h>
#include <asm/arch/octeontx2.h>
#include "cavm-csrs-rvu.h"
#include "cavm-csrs-npa.h"
#include "rvu_common.h"
#include "rvu.h"
#include "cgx.h"
#include "nix.h"

static int pfid = 1;

int rvu_add_nix(struct rvu_pf *rvu)
{
	struct nix_af_handle *nix_af = nix_get_af((u64)(rvu->nix_base));
	struct eth_device *netdev;
	struct nix_handle *nix;
	struct cgx *cgx;
	struct lmac *lmac;
	struct nix_lf_alloc_req req;
	struct nix_lf_alloc_rsp res;
	int err;

	if (!nix_af) {
		printf("%s: Error: Could not find NIX AF for PF base %p\n",
		       __func__, rvu->nix_base);
		return -1;
	}
	if (rvu->pf < 1) {
		printf("%s: Error: RVU PF %d invalid\n", __func__, rvu->pf);
		return -1;
	}
	lmac = cgx_get_lmac(rvu->pf - 1);
	if (!lmac) {
		printf("%s: Error: No LMAC found for pf %d\n",
		       __func__, rvu->pf);
		return -1;
	}
	cgx = lmac->cgx;
	memset(&req, 0, sizeof(req));
	memset(&res, 0, sizeof(res));
	req.rq_cnt = 1;
	req.sq_cnt = 1;
	req.cq_cnt = 1;
	req.rss_grps = 1;
	req.xqe_sz = CAVM_NIX_XQESZ_E_W16;
	req.npa_func = rvu->pf;

	debug("%s: Allocating nix lf\n", __func__);
	nix = cavm_nix_lf_alloc(nix_af, rvu->dev, rvu->pf, rvu->pf,
				rvu->nix_base,
				rvu->npc_base, rvu->lmt_base,
				cgx->cgx_id, lmac->lmac_id, &req, &res);
	if (!nix) {
		printf("%s: Error allocating lf for pf %d\n",
		       __func__, rvu->pf);
		return -1;
	}

	netdev = calloc(sizeof(*netdev), 1);
	if (!netdev) {
		printf("%s: Out of memory!\n", __func__);
		/* TODO: call cleanup function */
		return -1;
	}
	rvu->nix = nix;
	netdev->priv = rvu;
	snprintf(netdev->name, sizeof(netdev->name), "eth%u", nix->nic_id);
	netdev->halt = NULL;	/* TODO */
	netdev->init = NULL;	/* TODO */
	netdev->send = NULL;	/* TODO */
	netdev->recv = NULL;	/* TODO */

	if (!eth_env_get_enetaddr_by_index("eth", nix->nic_id,
					   netdev->enetaddr)) {
		eth_env_get_enetaddr("ethaddr", netdev->enetaddr);
		netdev->enetaddr[5] += nix->nic_id;
	}
	err = eth_register(netdev);
	if (!err)
		return 0;

	printf("Failed to register Ethernet device %s\n", netdev->name);

	return err;
}

int rvu_pf_probe(struct udevice *dev)
{
	struct rvu_pf *pf_ptr = dev_get_priv(dev);
	size_t size;
	union cavm_rvu_func_addr_s func_addr;
	int nix_af_pf;
	int err;

	debug("%s: name: %s\n", __func__, dev->name);

	pf_ptr->dev = dev;
	pf_ptr->base = dm_pci_map_bar(dev, 2, &size, PCI_REGION_MEM);
	pf_ptr->pf_id = pfid++;
	pf_ptr->pf = ((u64)(pf_ptr->base) >> 36) & 0x0f;

	debug("RVU PF BAR2 RVU BASE %p, pf: %u\n", pf_ptr->base, pf_ptr->pf);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NIXX(0);
	pf_ptr->nix_base = pf_ptr->base + func_addr.u;
	debug("RVU PF BAR2 NIX BASE %p\n", pf_ptr->nix_base);
	//nix_lf_init(pf_ptr->pf_id, pf_ptr->nix_base);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NPA;
	pf_ptr->npa_base = pf_ptr->base + func_addr.u;
	debug("RVU PF BAR2 NPA BASE %p\n", pf_ptr->npa_base);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_NPC;
	pf_ptr->npc_base = pf_ptr->base + func_addr.u;
	debug("RVU PF BAR2 NPC BASE %p\n", pf_ptr->npc_base);

	func_addr.u = 0;
	func_addr.s.block = CAVM_RVU_BLOCK_ADDR_E_LMT;
	pf_ptr->lmt_base = pf_ptr->base + func_addr.u;
	debug("RVU PF BAR2 LMT BASE %p\n", pf_ptr->lmt_base);

	err = rvu_add_nix(pf_ptr);

	if (err)
		printf("%s: Error %d adding nix\n", __func__, err);

	return err;
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


