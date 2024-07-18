/* SPDX-License-Identifier: BSD-2-Clause-Patent
 * https://spdx.org/licenses
 *
 * Copyright (C) 2024 Marvell
 *
 */

#ifndef __SMC_H__
#define __SMC_H__

#include <asm/arch/smc-id.h>

ssize_t smc_dram_size(unsigned int node);
ssize_t smc_disable_rvu_lfs(unsigned int node);

/*
 * Get RVU Reserved Memory Region Info
 *
 * Return:
 *	x0:
 *		0 -- Success
 *	x1 - region start address
 *	x2 - region size
 */
int smc_rvu_rsvd_reg_info(u64 *reg_addr, u64 *reg_size);

#endif	// __SMC_H__
