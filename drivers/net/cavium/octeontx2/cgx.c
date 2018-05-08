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
#include <linux/list.h>
#include <asm/arch/octeontx2.h>

#include "cavm-csrs-cgx.h"
#include "cgx_intf.h"
#include "cgx.h"

static LIST_HEAD(cgx_list);

static inline struct lmac *lmac_pdata(u8 lmac_id, struct cgx *cgx)
{
	if (!cgx || lmac_id > MAX_LMAC_PER_CGX)
		return NULL;

	return cgx->lmac_idmap[lmac_id];
}

int cgx_get_cgx_cnt(void)
{
	struct cgx *cgx_dev;
	int count = 0;

	list_for_each_entry(cgx_dev, &cgx_list, cgx_list)
		count++;

	return count;
}

int cgx_get_lmac_cnt(void *cgxd)
{
	struct cgx *cgx = cgxd;

	if (!cgx)
		return -ENODEV;

	return cgx->lmac_count;
}

void *cgx_get_pdata(int cgx_id)
{
	struct cgx *cgx_dev;

	list_for_each_entry(cgx_dev, &cgx_list, cgx_list) {
		if (cgx_dev->cgx_id == cgx_id)
			return cgx_dev;
	}
	return NULL;
}

/**
 * Given an LMAC instance number, return the lmac
 *
 * @param instance	instance to find
 *
 * @return	pointer to lmac data structure or NULL if not found
 */
struct lmac *cgx_get_lmac(int instance)
{
	struct cgx *cgx;
	int i;

	list_for_each_entry(cgx, &cgx_list, cgx_list) {
		for (i = 0; i < MAX_LMAC_PER_CGX; i++) {
			if (cgx->lmac_idmap[i] &&
			    cgx->lmac_idmap[i]->instance == instance)
				return cgx->lmac_idmap[i];
		}
	}
	return NULL;
}

static void cgx_write(struct cgx *cgx, u64 lmac, u64 offset, u64 val)
{
	writeq(val, cgx->reg_base + (lmac << 18) + offset);
}

static u64 cgx_read(struct cgx *cgx, u64 lmac, u64 offset)
{
	return readq(cgx->reg_base + (lmac << 18) + offset);
}

int cgx_set_pkind(void *cgxd, u8 lmac_id, int pkind)
{
	struct cgx *cgx = cgxd;

	if (!cgx || lmac_id >= cgx->lmac_count)
		return -ENODEV;

	cgx_write(cgx, lmac_id, CAVM_CGXX_CMRX_RX_ID_MAP(0), (pkind & 0x3f));
	return 0;
}

/**
 * Given a linear link number, get the cgx and lmac
 *
 * @param	linear_link_number	Linear link number
 * @param[out]	cgx_id			cgx_id number
 * @parma[out]	lmac_id			lmac_id number
 *
 * @return 0 for success or -1 if not found
 */
int cgx_get_identifiers(int linear_link_number, int *cgx_id, int *lmac_id)
{
	int index = 0;
	struct cgx *cgx;

	for (cgx = cgx_get_pdata(index); cgx; index++) {
		if (linear_link_number < cgx->lmac_count) {
			*cgx_id = cgx->cgx_id;
			*lmac_id = linear_link_number;
			return 0;
		} else {
			linear_link_number -= cgx->lmac_count;
		}
	}
	return -1;
}

int cgx_channel_number(int linear_link_number)
{
	int cgx_id, lmac_id, err;

	err = cgx_get_identifiers(linear_link_number, &cgx_id, &lmac_id);
	if (err)
		return -1;
	else
		return (0x800 + 0x100 * cgx_id + 0x10 * lmac_id + 0);
}

int cgx_link_number(int linear_link_number)
{
	int cgx_id, lmac_id, err;

	err = cgx_get_identifiers(linear_link_number, &cgx_id, &lmac_id);
	if (err)
		return -1;
	else
		return (4 * cgx_id + lmac_id);
}

u64 cgx_get_channel_number(struct cgx *cgx, int linear_link_number)
{
	int lmac_id;
	if (!cgx->lmac_idmap[linear_link_number]) {
		printf("%s: Invalid link number %d for cgx %d\n", __func__,
		       linear_link_number, cgx->cgx_id);
		return 0;
	}

	lmac_id = cgx->lmac_idmap[linear_link_number]->lmac_id;
	return (0x800 + 0x100 * cgx->cgx_id + 0x10 * lmac_id + 0);
}

int cgx_lmac_internal_loopback(void *cgxd, int lmac_id, bool enable)
{
	struct cgx *cgx = cgxd;
	union cavm_cgxx_cmrx_config cmrx_cfg;
	union cavm_cgxx_gmp_pcs_mrx_control mrx_control;
	union cavm_cgxx_spux_control1 spux_control1;
	enum lmac_type lmac_type;

	if (!cgx || lmac_id >= cgx->lmac_count)
		return -ENODEV;

	cmrx_cfg.u = cgx_read(cgx, lmac_id, CAVM_CGXX_CMRX_CONFIG(0));
	lmac_type = cmrx_cfg.s.lmac_type;
	if (lmac_type == LMAC_MODE_SGMII || lmac_type == LMAC_MODE_QSGMII) {
		mrx_control.u = cgx_read(cgx, lmac_id,
					 CAVM_CGXX_GMP_PCS_MRX_CONTROL(0));
		mrx_control.s.loopbck1 = enable ? 1 : 0;
		cgx_write(cgx, lmac_id, CAVM_CGXX_GMP_PCS_MRX_CONTROL(0),
			  mrx_control.u);
	} else {
		spux_control1.u = cgx_read(cgx, lmac_id,
					   CAVM_CGXX_SPUX_CONTROL1(0));
		spux_control1.s.loopbck = enable ? 1 : 0;
		cgx_write(cgx, lmac_id, CAVM_CGXX_SPUX_CONTROL1(0),
			  spux_control1.u);
	}
	return 0;
}

int cgx_lmac_rx_tx_enable(void *cgxd, int lmac_id, bool enable)
{
	struct cgx *cgx = cgxd;
	union cavm_cgxx_cmrx_config cmrx_config;

	if (!cgx || lmac_id >= cgx->lmac_count)
		return -ENODEV;

	cmrx_config.u = cgx_read(cgx, lmac_id, CAVM_CGXX_CMRX_CONFIG(0));
	cmrx_config.s.enable =
		cmrx_config.s.data_pkt_rx_en =
		cmrx_config.s.data_pkt_tx_en = enable ? 1 : 0;
	cgx_write(cgx, lmac_id, CAVM_CGXX_CMRX_CONFIG(0), cmrx_config.u);
	return 0;
}

static int cgx_lmac_init(struct cgx *cgx)
{
	struct lmac *lmac;
	int i;
	union cavm_cgxx_cmrx_config cmrx_cfg;
	static int instance = 0;

	cgx->lmac_count = cgx_read(cgx, 0, CAVM_CGXX_CMR_RX_LMACS());
	debug("%s: Found %d lmacs for cgx %d@%p\n", __func__, cgx->lmac_count,
	      cgx->cgx_id, cgx->reg_base);
	if (cgx->lmac_count > MAX_LMAC_PER_CGX)
		cgx->lmac_count = MAX_LMAC_PER_CGX;

	for (i = 0; i < cgx->lmac_count; i++) {
		lmac = calloc(1, sizeof(*lmac));
		if (!lmac)
			return -ENOMEM;
		lmac->instance = instance++;
		snprintf(lmac->name, sizeof(lmac->name), "cgx_fwi_%d_%d",
			 cgx->cgx_id, i);
		/* Get LMAC type */
		cmrx_cfg.u = cgx_read(cgx, i, CAVM_CGXX_CMRX_CONFIG(0));
		lmac->lmac_type = cmrx_cfg.s.lmac_type;

		lmac->lmac_id = i;
		lmac->cgx = cgx;
		cgx->lmac_idmap[i] = lmac;
		debug("%s: mapping id %d to lmac %p (%s), lmac type: %d\n",
		      __func__, i, lmac, lmac->name, lmac->lmac_type);
	}
	return 0;
}

void enumerate_lmacs(void)
{

}

int cgx_probe(struct udevice *dev)
{
	struct cgx *cgx = dev_get_priv(dev);
	size_t size;
	int err;
	static int instance = 0;

	cgx->reg_base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);
	cgx->dev = dev;
	cgx->cgx_id = ((u64)(cgx->reg_base) >> 24) & 0x7;
	cgx->instance = instance++;

	debug("CGX BAR %p, id: %d, instance: %d\n",
	      cgx->reg_base, cgx->cgx_id, cgx->instance);

	/*enumerate_lmacs();*/

	err = cgx_lmac_init(cgx);
	if (!err)
		list_add(&cgx->cgx_list, &cgx_list);

	return err;
}

U_BOOT_DRIVER(cgx) = {
        .name   = "cgx",
        .id     = UCLASS_MISC,
        .probe  = cgx_probe,
        .priv_auto_alloc_size = sizeof(struct cgx),
};

static struct pci_device_id cgx_supported[] = {
        { PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_OCTEONTX2_CGX) },
        {}
};

U_BOOT_PCI_DEVICE(cgx, cgx_supported);
