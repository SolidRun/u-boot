/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2023 Josua Mayer <josua@solid-run.com>
 */

#include <asm/arch/ddr.h>

static struct dram_cfg_param ddr_ddrc_cfg[] = {
	{ 0x3d400064, 0x7a017c },
	{ 0x3d400138, 0x184 },
	{ 0x3d400200, 0x18 },
	{ 0x3d40021c, 0xf07 },
	{ 0x3d402064, 0xc0026 },
	{ 0x3d402138, 0x27 },
	{ 0x3d403064, 0x3000a },
	{ 0x3d403138, 0xa },
};

struct dram_timing_info dram_timing_patch_8gb_micron = {
	.ddrc_cfg = ddr_ddrc_cfg,
	.ddrc_cfg_num = ARRAY_SIZE(ddr_ddrc_cfg),
	.ddrphy_cfg = NULL,
	.ddrphy_cfg_num = 0,
	.fsp_msg = NULL,
	.fsp_msg_num = 0,
	.ddrphy_trained_csr = NULL,
	.ddrphy_trained_csr_num = 0,
	.ddrphy_pie = NULL,
	.ddrphy_pie_num = 0,
	.fsp_table = {},
};
