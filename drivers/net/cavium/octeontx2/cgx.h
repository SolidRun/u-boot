/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __CGX_H__
#define __CGX_H__

#define PCI_DEVICE_ID_OCTEONTX2_CGX	0xA059

#define CGX_FIRWARE_MAJOR_VER		1
#define CGX_FIRWARE_MINOR_VER		0
#define MAX_LMAC_PER_CGX		4
#define CGX_PER_NODE 			3

/* Register offsets */
#define CGX_CMR_SCRATCH0	0x87e0e0001050
#define CGX_CMR_SCRATCH1	0x87e0e0001058

#define CGX_SHIFT(x)		(0x1000000 * (x & 0x3))
#define CMR_SHIFT(x)		(0x40000 * (x & 0x3))

enum lmac_type {
	LMAC_MODE_SGMII		= 0,
	LMAC_MODE_XAUI		= 1,
	LMAC_MODE_RXAUI		= 2,
	LMAC_MODE_10G_R		= 3,
	LMAC_MODE_40G_R		= 4,
	LMAC_MODE_QSGMII	= 6,
	LMAC_MODE_25G_R		= 7,
	LMAC_MODE_50G_R		= 8,
	LMAC_MODE_100G_R	= 9,
	LMAC_MODE_USXGMII	= 10,
};

struct lmac_priv {
	u8 enable:1;
	u8 full_duplex:1;
	u8 speed:4;
	u8 mode:1;
	u8 rsvd:1;
	u8 mac_addr[6];
};

struct cgx;
struct nix_handle;
struct nix_af_handle;

struct lmac {
	struct cgx	*cgx;
	struct nix_handle *nix;
	char		name[16];
	enum lmac_type	lmac_type;
	bool		cmd_pend;
	u8		instance;
	u8		lmac_id;
	u32		linear_link_number;
};

struct cgx {
	struct nix_af_handle	*nix_af;
	void __iomem		*reg_base;
	struct udevice		*dev;
	struct lmac		*lmac_idmap[MAX_LMAC_PER_CGX];
	struct list_head	cgx_list;
	u8			instance;
	u8			cgx_id;
	u8			lmac_count;
};

int cgx_get_cgx_cnt(void);
int cgx_get_lmac_cnt(void *cgxd);
/**
 * Given an LMAC instance number, return the lmac
 *
 * @param instance	instance to find
 *
 * @return	pointer to lmac data structure or NULL if not found
 */
struct lmac *cgx_get_lmac(int instance);
void *cgx_get_pdata(int cgx_id);
int cgx_set_pkind(void *cgxd, u8 lmac_id, int pkind);
u64 cgx_get_channel_number(struct cgx *cgx, int linear_link_number);
/**
 * Given a linear link number, get the cgx and lmac
 *
 * @param	linear_link_number	Linear link number
 * @param[out]	cgx_id			cgx_id number
 * @parma[out]	lmac_id			lmac_id number
 *
 * @return 0 for success or -1 if not found
 */
int cgx_get_identifiers(int linear_link_number, int *cgx_id, int *lmac_id);

#endif /* __CGX_H__ */
