/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2023 Josua Mayer <josua@solid-run.com>
 */

#ifndef __IMX8MPSR_LPDDR4_TIMING_H__
#define __IMX8MPSR_LPDDR4_TIMING_H__

/*
 * timings for 1GB Samsung / Micron
 */
extern struct dram_timing_info dram_timing_1gb_samsung_micron;

/*
 * timings for additional variants, as patch against 1GB Samsung
 * - 2GB Samsung
 */
extern struct dram_timing_info dram_timing_patch_2gb_samsung;

/*
 * timings for 3GB Micron
 */
extern struct dram_timing_info dram_timing_3gb_micron;

/*
 * timings for 4GB Samsung / Micron
 */
extern struct dram_timing_info dram_timing_4gb_samsung_micron;

/*
 * timings for additional variants, as patch against 4GB Samsung / Micron
 * - 8GB Micron
 */
extern struct dram_timing_info dram_timing_patch_8gb_micron;

/*
 * patch function to apply timing differences in-place
 *
 * Note: uses patch to store original values, execute again to revert!
 */
void timing_patch_apply(struct dram_timing_info *const info, struct dram_timing_info *const patch);

#endif /* __IMX8MPSR_LPDDR4_TIMING_H__ */
