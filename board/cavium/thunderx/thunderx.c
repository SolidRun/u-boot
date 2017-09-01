// SPDX-License-Identifier: GPL-2.0+
/**
 * (C) Copyright 2014, Cavium Inc.
**/
#define DEBUG
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <asm/io.h>

#include <linux/compiler.h>

#include <asm/arch/atf.h>

#include <libfdt.h>
#include <fdt_support.h>
#include <cavium/thunderx_fdt.h>
#include <cavium/atf.h>
#include <asm/armv8/mmu.h>

#if !CONFIG_IS_ENABLED(OF_CONTROL)
#include <dm/platform_data/serial_pl01x.h>

static const struct pl01x_serial_platdata serial0 = {
	.base = CONFIG_SYS_SERIAL0,
	.type = TYPE_PL011,
	.clock = 0,
	.skip_init = true,
};

U_BOOT_DEVICE(thunderx_serial0) = {
	.name = "serial_pl01x",
	.platdata = &serial0,
};

static const struct pl01x_serial_platdata serial1 = {
	.base = CONFIG_SYS_SERIAL1,
	.type = TYPE_PL011,
	.clock = 0,
	.skip_init = true,
};

U_BOOT_DEVICE(thunderx_serial1) = {
	.name = "serial_pl01x",
	.platdata = &serial1,
};
#endif
#include <asm/arch/thunderx_fdt.h>
#include <asm/arch/atf.h>
#include <dm/util.h>

#include "cavm-arch.h"

DECLARE_GLOBAL_DATA_PTR;

#define BOARD_TYPE "BOARD="

char thunderx_prompt[CONFIG_THUNDERX_PROMPT_SIZE] = "ThunderX> ";
extern unsigned long fdt_base_addr;

#ifdef CONFIG_BOARD_EARLY_INIT_R
extern void eth_common_init(void);
int board_early_init_r(void)
{
	eth_common_init();
	pci_init();
	return 0;
}
#endif

int board_init(void)
{
	ulong fdt_addr = (ulong)fdt_base_addr;
	set_working_fdt_addr(fdt_addr);
	thunderx_parse_phy_info();

	return 0;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	ssize_t node_count = atf_node_count();
	ssize_t dram_size;
	int node;

	debug("Initializing\nNodes in system: %zd\n", node_count);

	gd->ram_size = 0;

	for (node = 0; node < node_count; node++) {
		dram_size = atf_dram_size(node);
		debug("Node %d: %zd MBytes of DRAM\n", node, dram_size >> 20);
		gd->ram_size += dram_size;
	}

	gd->ram_size -= MEM_BASE;

	return 0;
}

/**
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
	u64 val = readq(CAVM_RST_SOFT_RST) | 1;

	writeq(val, CAVM_RST_SOFT_RST);
}

/*
 * Return board alternative package
 */
bool alternate_pkg(void)
{
	u64 val = readq(CAVM_MIO_FUS_DAT2);
	int altpkg;

	altpkg = (val >> 22) & 0x3;

	/* Figure out alternative pkg by reading chip_id
	   or lmc_mode32 on 81xx */
	if (CAVIUM_IS_MODEL(CAVIUM_CN81XX)
	    && (altpkg || ((val >> 30) & 0x1)))
		return 2;
	return altpkg;
}

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[32];
	const char *board, *str;
	int len, node;

	debug("%s()\n", __func__);

	/*
	 * Now that pci_init initializes env device.
	 * Try to set environment variables
	 */
	thunderx_parse_bdk_config();
	thunderx_parse_mac_addr();

	board = getenv("board");

	/* some times simulator fails to load environment
	 * from flash, try to read it from devicetree
	 * until it is fixed
	 */
	if (board == NULL) {
		node = fdt_path_offset(gd->fdt_blob, "/cavium,bdk");
		str = fdt_getprop(gd->fdt_blob, node, "BOARD-MODEL", &len);
		debug("fdt: BOARD-MODEL str %s len %d\n", str, len);
		if (str) {
			strncpy(boardname, str, sizeof(boardname));
			setenv("board", boardname);
		}
		board = getenv("board");
	}

	if (board != NULL && !getenv("prompt")) {
		snprintf(thunderx_prompt, sizeof(thunderx_prompt), "%s> ",
			 board);
		printf("Set prompt to \"%s\"\n", thunderx_prompt);
		setenv("prompt", thunderx_prompt);
	} else if (getenv("prompt")) {
		printf("%s: prompt already set to \"%s\"\n",
		       __func__, getenv("prompt"));
	} else {
		printf("%s: board is NULL\n", __func__);
	}

	printf("Board type: %s\n", getenv("board"));
#ifdef DEBUG
	dm_dump_all();
#endif

	return 0;
}

/*
 * Board specific ethernet initialization routine.
 */

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_RANDOM_MACADDR
	unsigned char ethaddr[6];

	if (!eth_getenv_enetaddr("ethaddr", ethaddr)) {
		net_random_ethaddr(ethaddr);
		printf("Generating random MAC address: %pM\n", ethaddr);
		eth_setenv_enetaddr("ethaddr", ethaddr);
	}
#endif

	rc = pci_eth_init(bis);

	return rc;
}

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	ssize_t node, core;

	for (node = 0; node < atf_node_count(); node++)
		for (core = 0; core < thunderx_core_count(); core++)
			writeq(~0ULL, CSR_PA(node, CAVM_GTI_CWD_POKE(core)));
}

void hw_watchdog_disable(void)
{
	ssize_t node, core;

	for (node = 0; node < atf_node_count(); node++)
		for (core = 0; core < thunderx_core_count(); core++)
			writeq(0ULL, CSR_PA(node, CAVM_GTI_CWD_WDOG(core)));
}
#endif
