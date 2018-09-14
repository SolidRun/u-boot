/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX_CLOCK_H__

/** System PLL reference clock */
#define PLL_REF_CLK                     50000000        /* 50 MHz */
#define NS_PER_REF_CLK_TICK             (1000000000/PLL_REF_CLK)

/** Address of RST_BOOT register */
#define RST_BOOT	0x87e006001600ll

/**
 * Register (RSL) rst_boot
 *
 * RST Boot Register
 */
union cavm_rst_boot {
	u64 u;
	struct cavm_rst_boot_s {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		u64 chipkill:1;
		u64 jtcsrdis:1;
		u64 ejtagdis:1;
		u64 trusted_mode:1;
		u64 ckill_ppdis:1;
		u64 jt_tstmode:1;
		u64 vrm_err:1;
		u64 dis_huk:1;
		u64 dis_scan:1;
		u64 reserved_47_54:8;
		u64 c_mul:7;
		u64 reserved_39:1;
		u64 pnr_mul:6;
		u64 lboot_oci:3;
		u64 lboot_pf_flr:4;
		u64 lboot_ckill:1;
		u64 lboot_jtg:1;
		u64 lboot_ext45:6;
		u64 lboot_ext23:6;
		u64 lboot:10;
		u64 rboot:1;
		u64 rboot_pin:1;
#else /* Word 0 - Little Endian */
		u64 rboot_pin:1;
		u64 rboot:1;
		u64 lboot:10;
		u64 lboot_ext23:6;
		u64 lboot_ext45:6;
		u64 lboot_jtg:1;
		u64 lboot_ckill:1;
		u64 lboot_pf_flr:4;
		u64 lboot_oci:3;
		u64 pnr_mul:6;
		u64 reserved_39:1;
		u64 c_mul:7;
		u64 reserved_47_54:8;
		u64 dis_scan:1;
		u64 dis_huk:1;
		u64 vrm_err:1;
		u64 jt_tstmode:1;
		u64 ckill_ppdis:1;
		u64 trusted_mode:1;
		u64 ejtagdis:1;
		u64 jtcsrdis:1;
		u64 chipkill:1;
#endif /* Word 0 - End */
	} s;
	struct cavm_rst_boot_cn81xx {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		u64 chipkill:1;
		u64 jtcsrdis:1;
		u64 ejtagdis:1;
		u64 trusted_mode:1;
		u64 ckill_ppdis:1;
		u64 jt_tstmode:1;
		u64 vrm_err:1;
		u64 dis_huk:1;
		u64 dis_scan:1;
		u64 reserved_47_54:8;
		u64 c_mul:7;
		u64 reserved_39:1;
		u64 pnr_mul:6;
		u64 lboot_oci:3;
		u64 reserved_26_29:4;
		u64 lboot_ckill:1;
		u64 lboot_jtg:1;
		u64 lboot_ext45:6;
		u64 lboot_ext23:6;
		u64 lboot:10;
		u64 rboot:1;
		u64 rboot_pin:1;
#else /* Word 0 - Little Endian */
		u64 rboot_pin:1;
		u64 rboot:1;
		u64 lboot:10;
		u64 lboot_ext23:6;
		u64 lboot_ext45:6;
		u64 lboot_jtg:1;
		u64 lboot_ckill:1;
		u64 reserved_26_29:4;
		u64 lboot_oci:3;
		u64 pnr_mul:6;
		u64 reserved_39:1;
		u64 c_mul:7;
		u64 reserved_47_54:8;
		u64 dis_scan:1;
		u64 dis_huk:1;
		u64 vrm_err:1;
		u64 jt_tstmode:1;
		u64 ckill_ppdis:1;
		u64 trusted_mode:1;
		u64 ejtagdis:1;
		u64 jtcsrdis:1;
		u64 chipkill:1;
#endif /* Word 0 - End */
	} cn81xx;
	struct cavm_rst_boot_cn88xx {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		u64 chipkill:1;
		u64 jtcsrdis:1;
		u64 ejtagdis:1;
		u64 trusted_mode:1;
		u64 ckill_ppdis:1;
		u64 jt_tstmode:1;
		u64 vrm_err:1;
		u64 dis_huk:1;
		u64 dis_scan:1;
		u64 reserved_47_54:8;
		u64 c_mul:7;
		u64 reserved_39:1;
		u64 pnr_mul:6;
		u64 lboot_oci:3;
		u64 reserved_26_29:4;
		u64 reserved_24_25:2;
		u64 lboot_ext45:6;
		u64 lboot_ext23:6;
		u64 lboot:10;
		u64 rboot:1;
		u64 rboot_pin:1;
#else /* Word 0 - Little Endian */
		u64 rboot_pin:1;
		u64 rboot:1;
		u64 lboot:10;
		u64 lboot_ext23:6;
		u64 lboot_ext45:6;
		u64 reserved_24_25:2;
		u64 reserved_26_29:4;
		u64 lboot_oci:3;
		u64 pnr_mul:6;
		u64 reserved_39:1;
		u64 c_mul:7;
		u64 reserved_47_54:8;
		u64 dis_scan:1;
		u64 dis_huk:1;
		u64 vrm_err:1;
		u64 jt_tstmode:1;
		u64 ckill_ppdis:1;
		u64 trusted_mode:1;
		u64 ejtagdis:1;
		u64 jtcsrdis:1;
		u64 chipkill:1;
#endif /* Word 0 - End */
	} cn88xx;
	struct cavm_rst_boot_cn83xx {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		u64 chipkill:1;
		u64 jtcsrdis:1;
		u64 ejtagdis:1;
		u64 trusted_mode:1;
		u64 ckill_ppdis:1;
		u64 jt_tstmode:1;
		u64 vrm_err:1;
		u64 dis_huk:1;
		u64 dis_scan:1;
		u64 reserved_47_54:8;
		u64 c_mul:7;
		u64 reserved_39:1;
		u64 pnr_mul:6;
		u64 lboot_oci:3;
		u64 lboot_pf_flr:4;
		u64 lboot_ckill:1;
		u64 lboot_jtg:1;
		u64 lboot_ext45:6;
		u64 lboot_ext23:6;
		u64 lboot:10;
		u64 rboot:1;
		u64 rboot_pin:1;
#else /* Word 0 - Little Endian */
		u64 rboot_pin:1;
		u64 rboot:1;
		u64 lboot:10;
		u64 lboot_ext23:6;
		u64 lboot_ext45:6;
		u64 lboot_jtg:1;
		u64 lboot_ckill:1;
		u64 lboot_pf_flr:4;
		u64 lboot_oci:3;
		u64 pnr_mul:6;
		u64 reserved_39:1;
		u64 c_mul:7;
		u64 reserved_47_54:8;
		u64 dis_scan:1;
		u64 dis_huk:1;
		u64 vrm_err:1;
		u64 jt_tstmode:1;
		u64 ckill_ppdis:1;
		u64 trusted_mode:1;
		u64 ejtagdis:1;
		u64 jtcsrdis:1;
		u64 chipkill:1;
#endif /* Word 0 - End */
	} cn83xx;
};

/**
 * Returns the I/O clock speed in Hz
 */
u64 octeontx_get_io_clock(void);

/**
 * Returns the core clock speed in Hz
 */
u64 octeontx_get_core_clock(void);

#endif /* __OCTEONTX_CLOCK_H__ */
