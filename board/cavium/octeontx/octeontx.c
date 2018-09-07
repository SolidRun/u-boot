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
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/arch/octeontx.h>
#include <asm/arch/atf.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;
extern unsigned long fdt_base_addr;
extern void eth_common_init(void);

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int misc_init_r(void)
{
	struct udevice *bus;

	eth_common_init();

	/*
	 * Enumerate all miscellaneous devices.
	 * So BGX/NIC/vNIC devices will be enumerated too.
	 */
	for (uclass_first_device(UCLASS_MISC, &bus);
	     bus;
	     uclass_next_device(&bus)) {
		;
	}
	return 0;
}

int board_init(void)
{
	octeontx_parse_board_info();
	octeontx_parse_phy_info();
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
//	octeontx_parse_mac_addr();

	debug("bdt.type %s\n", p_cavm_bdt->type);
	snprintf(boardname, sizeof(boardname), "%s> ", p_cavm_bdt->type);
	env_set("prompt", boardname);
	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));
	return 0;
}

void octeontx_board_get_ethaddr(int bgx, int lmac, unsigned char *eth)
{
	const void *fdt = gd->fdt_blob;
	const char *mac = NULL;
	int offset = 0, node, len;
	int subnode, i = 0;
	char bgxname[24];

	offset = fdt_node_offset_by_compatible(fdt, -1, "pci-bridge");
	if (offset < 0) {
		printf("%s couldn't find mrml bridge node in fdt\n",
			 __func__);
		return;
	}
	if (bgx == 2 && CAVIUM_IS_MODEL(CN81XX)) {
		snprintf(bgxname, sizeof(bgxname), "rgx%d", 0);
		lmac = 0;
	} else
		snprintf(bgxname, sizeof(bgxname), "bgx%d", bgx);

	node = fdt_subnode_offset(fdt, offset, bgxname);

	fdt_for_each_subnode(subnode, fdt, node) {
		if (i++ != lmac)
			continue;
		/* check for local-mac-address */
		mac = fdt_getprop(fdt, subnode,
				       "local-mac-address", &len);
		debug("%s mac %pM\n", __func__, mac);
		memcpy(eth, mac, ARP_HLEN);
		debug("%s mac %pM\n", __func__, eth);
		return;
	}
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
