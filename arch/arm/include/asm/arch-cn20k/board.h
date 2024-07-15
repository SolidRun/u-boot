/* SPDX-License-Identifier: BSD-2-Clause-Patent
 * https://spdx.org/licenses
 *
 * Copyright (C) 2024 Marvell
 *
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
 * is a register for each requester:
 * _ \<0\> - SCP          - reset on SCP reset
 * _ \<1\> - MCP          - reset on MCP reset
 * _ \<2\> - AP Secure    - reset on core reset
 * _ \<3\> - AP Nonsecure - reset on core reset
 * These register is only writable to the corresponding
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

/** Function definitions */
void mem_map_fill(u64 rvu_addr, u64 rvu_size);
void board_fdt_get_rsvd_size(u64 *addr, u64 *size);
const char *fdt_get_run_platform(void);
const char *fdt_get_board_model(void);
const char *fdt_get_board_serial(void);
const char *fdt_get_board_revision(void);
int fdt_get_board_mac_cnt(void);
u64 fdt_get_board_mac_addr(u8 id);
void board_get_env_spi_bus_cs(int *bus, int *cs);
void board_get_env_offset(int *offset, const char *property);
void board_get_spi_bus_cs(struct udevice *dev, int *bus, int *cs);
void cn20k_board_get_mac_addr(u8 index, u8 *mac_addr);
void init_sh_fwdata(void);
struct udevice;
int board_acquire_flash_arb(bool acquire);
void eth_intf_shutdown(void);
#endif /* __BOARD_H__ */
