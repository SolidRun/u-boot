/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2023 Josua Mayer <josua@solid-run.com>
 */

#include <asm/arch/ddr.h>

static struct dram_cfg_param ddr_ddrc_cfg[] = {
	{ 0x3d400064, 0xf008c },
	{ 0x3d400070, 0x7027f90 },
	{ 0x3d400074, 0x790 },
	{ 0x3d400100, 0x2028042a },
	{ 0x3d40011c, 0x502 },
	{ 0x3d400138, 0x94 },
	{ 0x3d400180, 0x3e8001e },
	{ 0x3d4000f4, 0x799 },
	{ 0x3d400108, 0x9121b1c },
	{ 0x3d400208, 0x0 },
	{ 0x3d400218, 0x7070707 },
	{ 0x3d402064, 0x3000e },
	{ 0x3d402100, 0xa040105 },
	{ 0x3d40211c, 0x302 },
	{ 0x3d402138, 0xf },
	{ 0x3d402180, 0x640004 },
	{ 0x3d4020f4, 0x599 },
	{ 0x3d403064, 0x30004 },
	{ 0x3d40311c, 0x302 },
	{ 0x3d403138, 0x4 },
	{ 0x3d403180, 0x190004 },
	{ 0x3d4030f4, 0x599 },
};

static struct dram_cfg_param ddr_phy_pie[] = {
	{ 0x2000b, 0x465 },
	{ 0x12000b, 0x70 },
	{ 0x22000b, 0x1c },
};

struct dram_timing_info dram_timing_patch_2gb_samsung = {
	.ddrc_cfg = ddr_ddrc_cfg,
	.ddrc_cfg_num = ARRAY_SIZE(ddr_ddrc_cfg),
	.ddrphy_cfg = NULL,
	.ddrphy_cfg_num = 0,
	.fsp_msg = NULL,
	.fsp_msg_num = 0,
	.ddrphy_trained_csr = NULL,
	.ddrphy_trained_csr_num = 0,
	.ddrphy_pie = ddr_phy_pie,
	.ddrphy_pie_num = ARRAY_SIZE(ddr_phy_pie),
	.fsp_table = {},
};
