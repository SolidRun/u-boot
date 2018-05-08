/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __RVU_H__
#define __RVU_H__

/*#include "rvu_hw.h"*/

#define PCI_DEVICE_ID_OCTEONTX2_RVU_PF	0xA063
#define PCI_DEVICE_ID_OCTEONTX2_RVU_VF	0xA064
#define PCI_DEVICE_ID_OCTEONTX2_RVU_AF	0xA065

struct nix_af_handle;

struct rvu_af {
	struct udevice *dev;
	u8 pf_id;
	void __iomem *base;
	void __iomem *bar2;
	void __iomem *nix_af_base;
	void __iomem *nix_af_bar2;
	void __iomem *npa_af_base;
	void __iomem *npc_af_base;
	struct rvu_hwinfo *hw;
	struct nix_af_handle *nix_af;
};

struct rvu_pf {
	struct udevice *dev;
	void __iomem *base;
	void __iomem *nix_base;
	void __iomem *npa_base;
	void __iomem *npc_base;
	void __iomem *lmt_base;
	struct rvu_hwinfo *hw;
	struct nix_handle *nix;
	u8 pf_id;
	u8 pf;
};

/**
 * Given the PF base address, return the NIX AF
 *
 * @param nix_pf_base		NIX PF base address
 *
 * @return	nix_af handle or NULL if not found.
 */
struct nix_af_handle *nix_get_af(u64 nix_pf_base);

#endif /* __RVU_H__ */

