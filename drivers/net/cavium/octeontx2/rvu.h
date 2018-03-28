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

#include "rvu_hw.h"

#define PCI_DEVICE_ID_OCTEONTX2_RVU_PF	0xA063
#define PCI_DEVICE_ID_OCTEONTX2_RVU_VF	0xA064
#define PCI_DEVICE_ID_OCTEONTX2_RVU_AF	0xA065

struct rvu_af {
	u8 pf_id;
	void __iomem *base;
	void __iomem *bar2;
	void __iomem *nix_af_base;
	void __iomem *npa_af_base;
};

struct rvu_pf {
	u8 pf_id;
	void __iomem *base;
	void __iomem *nix_base;
	void __iomem *npa_base;
};

#endif /* __RVU_H__ */

