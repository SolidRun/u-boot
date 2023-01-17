/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2023 Josua Mayer <josua@solid-run.com>
 */

#include <asm/arch/ddr.h>

#include "lpddr4_timing.h"

static inline void swapu(unsigned int *a, unsigned int *b) {
	unsigned int c;

	c = *a;
	*a = *b;
	*b = c;
}

static void dram_cfg_param_patch_apply(struct dram_cfg_param *const config, size_t const size, struct dram_cfg_param *const patch, size_t const length)
{
	size_t i, j;

	for (i = 0; i < size; i++)
		for (j = 0; j < length; j++)
			if (config[i].reg == patch[j].reg)
				swapu(&config[i].val, &patch[j].val);
}

static void dram_fsp_msg_patch_apply(struct dram_fsp_msg *const msg, size_t const size, struct dram_fsp_msg *const patch, size_t const length)
{
	size_t i;

	for (i = 0; i < size && i < length; i++) {
		if (patch[i].drate)
			swapu(&msg[i].drate, &patch[i].drate);
		if (patch->fw_type != -1)
			swapu(&msg[i].fw_type, &patch[i].fw_type);
		dram_cfg_param_patch_apply(msg->fsp_cfg, msg->fsp_cfg_num, patch->fsp_cfg, patch->fsp_cfg_num);
    }
}

void timing_patch_apply(struct dram_timing_info *const info, struct dram_timing_info *const patch)
{
	size_t i;
	dram_cfg_param_patch_apply(info->ddrc_cfg, info->ddrc_cfg_num, patch->ddrc_cfg, patch->ddrc_cfg_num);
	dram_cfg_param_patch_apply(info->ddrphy_cfg, info->ddrphy_cfg_num, patch->ddrphy_cfg, patch->ddrphy_cfg_num);
	dram_fsp_msg_patch_apply(info->fsp_msg, info->fsp_msg_num, patch->fsp_msg, patch->fsp_msg_num);
	dram_cfg_param_patch_apply(info->ddrphy_trained_csr, info->ddrphy_trained_csr_num, patch->ddrphy_trained_csr, patch->ddrphy_trained_csr_num);
	dram_cfg_param_patch_apply(info->ddrphy_pie, info->ddrphy_pie_num, patch->ddrphy_pie, patch->ddrphy_pie_num);
	for (i = 0; i < ARRAY_SIZE(info->fsp_table); i++)
		if (patch->fsp_table[i])
			swapu(&info->fsp_table[i], &patch->fsp_table[i]);
}
