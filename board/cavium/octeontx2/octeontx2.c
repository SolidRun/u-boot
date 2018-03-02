/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/arch/octeontx2.h>
#include <asm/arch/atf.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;
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
	octeontx2_parse_board_info();
	printf("Board: %s\n", p_cavm_bdt->type);
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

	gd->ram_size -= CONFIG_SYS_SDRAM_BASE;

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

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[20];

	debug("%s()\n", __func__);
	/*
	 * Now that pci_init initializes env device.
	 * Try to validate ethaddr env variables
	 */

	debug("bdt.type %s\n", p_cavm_bdt->type);
	snprintf(boardname, sizeof(boardname), "%s> ", p_cavm_bdt->type);
	env_set("prompt", boardname);
	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));
	return 0;
}

/*
 * Board specific ethernet initialization routine.
 */

int board_eth_init(bd_t *bis)
{
	int rc = 0;
	unsigned char ethaddr[6];

	if (!eth_env_get_enetaddr("ethaddr", ethaddr)) {
		net_random_ethaddr(ethaddr);
		printf("Generating random MAC address: %pM\n", ethaddr);
		eth_env_set_enetaddr("ethaddr", ethaddr);
	}

	rc = pci_eth_init(bis);

	return rc;
}

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	writeq(~0ULL, CAVM_GTI_CWD_POKEX);
}

void hw_watchdog_disable(void)
{
	writeq(0ULL, CAVM_GTI_CWD_WDOGX);
}
#endif
