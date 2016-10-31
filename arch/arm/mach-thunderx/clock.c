/**
 * (C) Copyright 2016, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 **/

#include <common.h>
#include <asm/io.h>

#define RST_BOOT	0x87e006001600ll

union rst_boot {
	u64 u;
	struct {
		int rboot_pin:1;
		int rboot:1;
		int lboot:10;
		int lboot_ext23:6;
		int lboot_ext45:6;
		int reserved_24_29:6;
		int lboot_oci:3;
		int pnr_mul:6;
		int reserved_39_39:1;
		int c_mul:7;
		int reserved_47_54:8;
		int dis_scan:1;
		int dis_huk:1;
		int vrm_err:1;
		int jt_tstmode:1;
		int ckill_ppdis:1;
		int trusted_mode:1;
		int ejtagdis:1;
		int jtcsrdis:1;
		int chipkill:1;
	} s;
};

/**
 * Returns the I/O clock speed in Hz
 */
u64 thunderx_get_io_clock(void)
{
	union rst_boot rst_boot;

	rst_boot.u = readq(RST_BOOT);

	return rst_boot.s.pnr_mul * PLL_REF_CLK;
}

/**
 * Returns the core clock speed in Hz
 */
u64 thunderx_get_core_clock(void)
{
	union rst_boot rst_boot;

	rst_boot.u = readq(RST_BOOT);

	return rst_boot.s.c_mul * PLL_REF_CLK;
}
