/**
 * (C) Copyright 2016, Cavium, Inc. <support@cavium.com>
 * Aaron Williams, <aaron.williams@cavium.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 **/

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

/**
 * Returns the I/O clock speed in Hz
 */
u64 thunderx_get_io_clock(void)
{
	union cavm_rst_boot rst_boot;

	rst_boot.u = readq(RST_BOOT);

	return rst_boot.s.pnr_mul * PLL_REF_CLK;
}

/**
 * Returns the core clock speed in Hz
 */
u64 thunderx_get_core_clock(void)
{
	union cavm_rst_boot rst_boot;

	rst_boot.u = readq(RST_BOOT);

	return rst_boot.s.c_mul * PLL_REF_CLK;
}
