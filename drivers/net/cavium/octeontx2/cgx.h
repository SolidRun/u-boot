/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

//#include "cavm-csrs-cgx.h"

#define LMAC_PER_CGX 4
#define CGX_PER_NODE 3
#define PCI_DEVICE_ID_OCTEONTX2_CGX	0xA059

struct lmac_priv {
	u8 enable:1;
	u8 full_duplex:1;
	u8 speed:4;
	u8 mode:1;
	u8 rsvd:1;
	u8 mac_addr[6];
};

struct cgx_priv {
	u8 enable;
	struct lmac_priv lmac[LMAC_PER_CGX];
};

struct cgx {
	void *__iomem base;
	struct cgx_priv cgx;
};
