/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/arch/octeontx.h>
#include <asm/arch/atf.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;
struct cavm_bdt g_cavm_bdt;

extern unsigned long fdt_base_addr;
extern void eth_common_init(void);

void octeontx_cleanup_ethaddr(void)
{
	char ename[32];
	for (int i = 0; i < 20; i++) {
		sprintf(ename, i ? "eth%daddr" : "ethaddr", i);
		if(env_get(ename))
			env_set_force(ename, NULL);
	}
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
	if (bgx == 2 && is_board_model(CN81XX)) {
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
		if(mac) {
			debug("%s mac %pM\n", __func__, mac);
			memcpy(eth, mac, ARP_HLEN);
		} else
			memset(eth, 0, ARP_HLEN);
		debug("%s eth %pM\n", __func__, eth);
		return;
	}
}

int octeontx_board_has_pmp(void)
{
	if ((strcasecmp(g_cavm_bdt.type, "sff8104") == 0) ||
		(strcasecmp(g_cavm_bdt.type, "nas8104") == 0))
		return 1;

	return 0;
}

void octeontx_parse_board_info(void)
{
	const char *str;
	int node;
	int ret = 0, len = 16;
	u64 midr, val;

	debug("%s: ENTER\n", __func__);

	asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (midr));

	g_cavm_bdt.prod_id = (midr >> 4) & 0xff;

	val = readq(CAVM_MIO_FUS_DAT2);
	g_cavm_bdt.alt_pkg = (val >> 22) & 0x3;
	if ((g_cavm_bdt.prod_id == CN81XX) &&
		(g_cavm_bdt.alt_pkg || ((val >> 30) & 0x1)))
		g_cavm_bdt.alt_pkg = 2;

	if (!gd->fdt_blob) {
		printf("ERROR: %s: no valid device tree found\n", __func__);
		return;
	}

	debug("%s: fdt blob at %p\n", __func__, gd->fdt_blob);
	ret = fdt_check_header(gd->fdt_blob);
	if (ret < 0) {
		printf("fdt: %s\n", fdt_strerror(ret));
		return;
	}
	debug("fdt:size %d\n", fdt_totalsize(gd->fdt_blob));

	node = fdt_path_offset(gd->fdt_blob, "/cavium,bdk");
	if (node < 0) {
		printf("%s: /cavium,bdk is missing from device tree: %s\n",
		       __func__, fdt_strerror(node));
		return;
	}
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MODEL", &len);
	debug("fdt: BOARD-MODEL str %s len %d\n", str, len);
	if (str) {
		strlcpy(g_cavm_bdt.type, str, sizeof(g_cavm_bdt.type));
		debug("fdt: BOARD-MODEL bdt.type %s \n", g_cavm_bdt.type);
	} else {
		printf("Error: cannot retrieve board type from fdt\n");
	}
}

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
	for (uclass_first_device_check(UCLASS_MISC, &bus);
	     bus;
	     uclass_next_device_check(&bus)) {
		;
	}
	return 0;
}

int board_init(void)
{
	octeontx_parse_phy_info();
	/* populates cavm_bdt structure */
	octeontx_parse_board_info();

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
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[20];

	debug("%s()\n", __func__);

	/*
	 * Now that pci_init initializes env device.
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of QLM can change.
	 */
	octeontx_cleanup_ethaddr();

	debug("bdt.type %s\n", g_cavm_bdt.type);
	snprintf(boardname, sizeof(boardname), "%s> ", g_cavm_bdt.type);
	env_set("prompt", boardname);
	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));
	return 0;
}

/*
 * Invoked before relocation, so limit to stack variables.
 */
int show_board_info(void)
{
	const char *str;
	int node, prod_id;
	int ret = 0, len = 16;
	u64 midr;

	asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (midr));

	prod_id = (midr >> 4) & 0xff;

	if (!gd->fdt_blob) {
		printf("ERROR: %s: no valid device tree found\n", __func__);
		return ret;
	}

	debug("%s: fdt blob at %p\n", __func__, gd->fdt_blob);
	ret = fdt_check_header(gd->fdt_blob);
	if (ret < 0) {
		printf("fdt: %s\n", fdt_strerror(ret));
		return ret;
	}
	debug("fdt:size %d\n", fdt_totalsize(gd->fdt_blob));

	node = fdt_path_offset(gd->fdt_blob, "/cavium,bdk");
	if (node < 0) {
		printf("%s: /cavium,bdk is missing from device tree: %s\n",
		       __func__, fdt_strerror(node));
		return ret;
	}
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MODEL", &len);
	debug("fdt: BOARD-MODEL str %s len %d\n", str, len);
	if (!str) {
		printf("Error: cannot retrieve board type from fdt\n");
	}

	if (prod_id == CN81XX)
		printf("OcteonTX CN81XX ARM V8 Core\n");
	if (prod_id == CN83XX)
		printf("OcteonTX CN83XX ARM V8 Core\n");

	printf("Board: %s\n", str);
	return 0;
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
