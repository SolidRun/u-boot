/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * Written By:
 * Konstantin Porotchkin <kostap@marvell.com>
 *
 * Based on :
 *   - pci_endpoint/pcie-cadence-ep.c
 * and Linux driver:
 *   - drivers/pci/endpoint/pcie-armada-dw-ep.c
 */

#ifndef PCIE_DW_EP_MVEBU_H
#define PCIE_DW_EP_MVEBU_H

#include <linux/bitops.h>

#define PCIE_GLOBAL_CTRL		0x0
#define PCIE_GLOBAL_CTRL_CRS_EN		BIT(9)
#define PCIE_GLOBAL_CTRL_TYPE_OFF	4
#define PCIE_GLOBAL_CTRL_TYPE_MASK	0xF
#define PCIE_GLOBAL_CTRL_TYPE_RC	(0x4)

#define PCIE_ATU_VIEWPORT		0x900
#define PCIE_ATU_REGION_INBOUND		(0x1 << 31)
#define PCIE_ATU_REGION_OUTBOUND	(0x0 << 31)
#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_CR1			0x904
#define PCIE_ATU_CR1_FUNC_OFF		20
#define PCIE_ATU_CR1_FUNC_MASK		0x1F
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_CR2			0x908
#define PCIE_ATU_CR2_REGION_EN		(0x1 << 31)
#define PCIE_ATU_CR2_BAR_EN		(0x1 << 30)
#define PCIE_ATU_CR2_FUNC_EN		(0x1 << 19)
#define PCIE_ATU_CR2_BAR_OFF		8
#define PCIE_ATU_LOWER_BASE		0x90C
#define PCIE_ATU_UPPER_BASE		0x910
#define PCIE_ATU_LIMIT			0x914
#define PCIE_ATU_LOWER_TARGET		0x918
#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)
#define PCIE_ATU_UPPER_TARGET		0x91C

#define PCIE_SRIOV_DEVID_OFFSET		0x192

#define PCIE_RESBAR_EXT_CAP_HDR_REG	0x25c
#define PCIE_RESBAR_EXT_CAP_REG(bar)			\
	({ typeof(bar) _BAR_ = (bar);			\
		PCIE_RESBAR_EXT_CAP_HDR_REG + 4 +	\
		(((_BAR_) / 2 + (_BAR_) % 2) & 0x3) * 8; })
#define PCIE_RESBAR_EXT_CAP_REG_MASK	0x000fffff
#define PCIE_RESBAR_EXT_CAP_REG_SHIFT	4

#define PCIE_BAR_IS_RESIZABLE(bar)		\
	({ typeof(bar) _BAR_ = (bar);		\
		(_BAR_) == 5 || (_BAR_) == 4 ||	\
		(_BAR_) == 2 || (_BAR_) == 0; })

#define MAX_ATU_REGIONS			16
#define MAX_ATU_SIZE			(4ul * SZ_1G)

#define  BAR_ENABLE_OFFSET		0
#define  BAR_ENABLE_MASK		BIT(BAR_ENABLE_OFFSET)

struct armada_pcie_ep {
	void		*regs;
	void		*shadow_regs;
	void		*lm_regs;
	void		*pl_regs;	/*port logical register only PF0*/
};

#define cfg_space_addr(func_id)			(0x1000 * (func_id))

#define cfg_func_base(ep, func_id, off)		\
	((ep)->regs + cfg_space_addr(func_id) + (off))

#define cfg_shadow_func_base(ep, func_id, off)	\
	((ep)->shadow_regs + cfg_space_addr(func_id) + (off))

#define get_out_region_idx(func_id, id)		((func_id) + (id))
#define get_in_region_idx(func_id, bar)		((func_id) + (bar))

#endif /* PCIE_DW_EP_MVEBU_H */
