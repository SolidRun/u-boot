/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <asm/arch/soc.h>

/** Reg offsets */
#define CPC_BOOT_OWNERX(a)	0x86D0000001C0ULL + (8 * (a))

extern unsigned long fdt_base_addr;

/** Structure definitions */
/**
 * Register (NCB32b) cpc_boot_owner#
 *
 * CPC Boot Owner Registers These registers control an external arbiter
 * for the boot device (SPI/eMMC) across multiple external devices. There
 * is a register for each requester: _ \<0\> - SCP          - reset on
 * SCP reset _ \<1\> - MCP          - reset on MCP reset _ \<2\> - AP
 * Secure    - reset on core reset _ \<3\> - AP Nonsecure - reset on core
 * reset  These register is only writable to the corresponding
 * requestor(s) permitted with CPC_PERMIT.
 */
union cpc_boot_ownerx {
	u32 u;
	struct cpc_boot_ownerx_s {
		u32 boot_req		: 1;
		u32 reserved_1_7	: 7;
		u32 boot_wait		: 1;
		u32 reserved_9_31	: 23;
	} s;
};

union rst_cold_data2_s {
	u64 u;
	struct rst_cold_data2_s_s {
		u64 fail_safe_boot_delaying : 1; /**< [  0:  0] Check if firmware has a delay for interaction with user or other valid reasons. */
		u64 reserved_1 : 1;
		u64 scp_to_ebf_vrm_problem : 1; /**< [  2:  2] SCP sets to tell EBF to not proceed if the AVS bus is not working. */
		u64 ebf_to_scp_vrm_problem : 1; /**< [  3:  3] EBF sets to tell SCP to not proceed if the PMBus configuration failed. */
		u64 cust : 8;                   /**< [ 11:  4] For customer use. */
		u64 vdd_ddr_dig_voltage : 1;    /**  [ 12: 12] Check for vdd_ddr_dig_voltage set*/
		u64 reserved_13_42 : 30;
		u64 ebf_boot_status : 1;  /**< [  43:  43] EBF boot status */
		u64 ebf_boot_error : 2;  /**< [  45:  44] EBF boot stage error */
		u64 atf_bl1_boot_status : 1;  /**< [  46:  46] ATF bl1 boot status */
		u64 atf_bl1_boot_error : 2;  /**< [  48:  47] ATF bl1 boot stage error */
		u64 atf_bl2_boot_status : 1;  /**< [  49:  49] ATF bl2 boot status */
		u64 atf_bl2_boot_error : 2;  /**< [  51:  50] ATF bl2 boot stage error */
		u64 atf_bl31_boot_status : 1;  /**< [  52:  52] ATF bl31 boot status */
		u64 atf_bl31_boot_error : 2;  /**< [  54:  53] ATF bl31 boot stage error */
		u64 atf_bl32_boot_status : 1;  /**< [  55:  55] ATF bl32 boot status */
		u64 atf_bl32_boot_error : 2;  /**< [  57:  56] ATF bl32 boot stage error */
		u64 uboot_boot_status : 1;  /**< [  58:  58] U-boot boot status */
		u64 uboot_boot_error : 2;  /**< [  60:  59] U-boot boot stage error */
		u64 linux_boot_status : 1;  /**< [  61:  61] Linux boot status */
		u64 linux_boot_error : 2;  /**< [  63:  62] Linux boot stage error */
	} s;
	/* struct rst_cold_data2_s_s cn; */
};

static inline uint64_t RST_COLD_DATAX(uint64_t a)
	__attribute__ ((pure, always_inline));

static inline uint64_t RST_COLD_DATAX(uint64_t a)
{
	if (a <= 5)
		return 0x87e0060017c0ULL + 8ULL * ((a) & 0x7);

	return 0;
}

/** Function definitions */
void mem_map_fill(u64 rvu_addr, u64 rvu_size);
void board_fdt_get_rsvd_size(u64 *addr, u64 *size);
int fdt_get_board_mac_cnt(bool *use_id);
u64 fdt_get_board_mac_addr(bool use_id, u8 id);
const char *fdt_get_run_platform(void);
const char *fdt_get_board_model(void);
const char *fdt_get_board_serial(void);
const char *fdt_get_board_revision(void);
void cn10k_board_get_mac_addr(u8 index, u8 *mac_addr);
void eth_intf_shutdown(void);
void init_sh_fwdata(void);
void board_get_env_spi_bus_cs(int *bus, int *cs);
void board_get_env_offset(int *offset, const char *property);
struct udevice;
void board_get_spi_bus_cs(struct udevice *dev, int *bus, int *cs);
int board_acquire_flash_arb(bool acquire);
int fdt_get_switch_config(void);
int fdt_get_switch_reset(void);
#endif /* __BOARD_H__ */
