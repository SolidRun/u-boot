// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/arch/octeontx2.h>
#include <asm/arch/atf.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

struct cavm_bdt g_cavm_bdt;
static int board_mac_num;
u64 board_mac_addr;

extern unsigned long fdt_base_addr;
extern void cgx_intf_shutdown(void);
extern void eth_common_init(void);
extern ssize_t atf_configure_ooo(unsigned int val);

void octeontx2_cleanup_ethaddr(void)
{
	char ename[32];

	for (int i = 0; i < 20; i++) {
		sprintf(ename, i ? "eth%daddr" : "ethaddr", i);
		if (env_get(ename))
			env_set(ename, NULL);
	}
}

void octeontx2_board_get_mac_addr(u8 index, u8 *mac_addr)
{
	u64 tmp_mac = board_mac_addr;

	if ((!is_zero_ethaddr((u8 *)&board_mac_addr)) && board_mac_num) {
		tmp_mac += index;
		tmp_mac = swab64(tmp_mac) >> 16;
		memcpy(mac_addr, (u8 *)&tmp_mac, ARP_HLEN);
		board_mac_num--;
	} else {
		memset(mac_addr, 0, ARP_HLEN);
	}
	debug("%s mac %pM\n", __func__, mac_addr);
}

void board_get_env_spi_bus_cs(int *bus, int *cs)
{
	const void *blob = gd->fdt_blob;
	int env_bus, env_cs;
	int node, preg;

	env_bus = -1;
	env_cs = -1;
	node = fdt_node_offset_by_compatible(blob, -1, "spi-flash");
	while (node > 0) {
		if (fdtdec_get_bool(blob, node, "u-boot,env")) {
			env_cs = fdtdec_get_int(blob, node, "reg", -1);
			preg = fdtdec_get_int(blob,
					      fdt_parent_offset(blob, node),
					      "reg", -1);
			/* SPI node will have PCI addr, so map it */
			if (preg == 0x3000)
				env_bus = 0;
			if (preg == 0x3800)
				env_bus = 1;
			debug("\n Env SPI [bus:cs] [%d:%d]\n",
			      env_bus, env_cs);
			break;
		}
		node = fdt_node_offset_by_compatible(blob, node, "spi-flash");
	}
	if (env_bus == -1)
		debug("\'u-boot,env\' property not found in fdt\n");

	*bus = env_bus;
	*cs = env_cs;
}

void octeontx_parse_board_info(void)
{
	const char *str;
	int node;
	int ret = 0, len = 16;
	u64 midr;

	debug("%s: ENTER\n", __func__);

	asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (midr));

	g_cavm_bdt.prod_id = (midr >> 4) & 0xff;

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
		debug("fdt: BOARD-MODEL bdt.type %s\n", g_cavm_bdt.type);
	} else {
		printf("Error: cannot retrieve board type from fdt\n");
	}
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MAC-ADDRESS-NUM", &len);
	if (str) {
		board_mac_num = simple_strtol(str, NULL, 10);
		if (!board_mac_num)
			board_mac_num = simple_strtol(str, NULL, 16);
		debug("fdt: MAC_NUM %d\n", board_mac_num);
	} else {
		printf("Error: cannot retrieve mac num prop from fdt\n");
	}
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MAC-ADDRESS-NUM-OVERRIDE",
			  &len);
	if (str) {
		if (simple_strtol(str, NULL, 10) >= 0)
			board_mac_num = simple_strtol(str, NULL, 10);
		debug("fdt: MAC_NUM %d\n", board_mac_num);
	} else {
		printf("Error: cannot retrieve macnum override prop\n");
	}
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MAC-ADDRESS", &len);
	if (str) {
		board_mac_addr = simple_strtoul(str, NULL, 16);
		debug("%s mac %llx\n", __func__, board_mac_addr);
	} else {
		board_mac_addr = 0;
	}
}

void board_quiesce_devices(void)
{
	struct uclass *uc_dev;
	int ret;

	/* Removes all RVU PF devices */
	ret = uclass_get(UCLASS_ETH, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove rvu pf devices\n");

#ifdef CONFIG_OCTEONTX2_CGX_INTF
	/* Bring down all cgx lmac links */
	cgx_intf_shutdown();
#endif

	/* Removes all CGX and RVU AF devices */
	ret = uclass_get(UCLASS_MISC, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove misc (cgx/rvu_af) devices\n");

	/* SMC call - removes all LF<->PF mappings */
	atf_disable_rvu_lfs(0);
}

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int board_init(void)
{
	octeontx_parse_board_info();
	return 0;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = 0;

	gd->ram_size = atf_dram_size(0);
	debug("Node0: %lld MBytes of DRAM\n", gd->ram_size >> 20);

	gd->ram_size -= CONFIG_SYS_SDRAM_BASE;

	return 0;
}

/**
 * Board misc devices initialization routine.
 */
int misc_init_r(void)
{
	struct udevice *bus;

	eth_common_init();
	/*
	 * Enumerate all known miscellaneous devices.
	 * Enumeration has the side-effect of probing them,
	 * so CGX and RVU AF devices will get enumerated.
	 */
	for (uclass_first_device_check(UCLASS_MISC, &bus);
	     bus;
	     uclass_next_device_check(&bus)) {
		;
	}
	return 0;
}

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[20];
	long val;
#ifdef CONFIG_OCTEONTX_SERIAL_BOOTCMD
	struct udevice *bootcmd_dev = NULL;
	int ret;
	char *stdinname = env_get("stdin");
#endif

	debug("%s()\n", __func__);

	/*
	 * Now that pci_init initializes env device.
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of QLM can change.
	 */
	octeontx2_cleanup_ethaddr();

	debug("bdt.type %s\n", g_cavm_bdt.type);
	snprintf(boardname, sizeof(boardname), "%s> ", g_cavm_bdt.type);
	env_set("prompt", boardname);
	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));

	val = env_get_hex("disable_ooo", 0);
	atf_configure_ooo(val);

#ifdef CONFIG_OCTEONTX_SERIAL_BOOTCMD
	if (!stdinname) {
		env_set("stdin", "serial");
		stdinname = env_get("stdin");
	}

	/* This will cause the pci-bootcmd driver to be probed. */
	ret = uclass_get_device_by_name(UCLASS_SERIAL, "pci-bootcmd",
					&bootcmd_dev);
	if (!ret && bootcmd_dev)
		debug("%s: %s found!\n", __func__, bootcmd_dev->name);
#if CONFIG_IS_ENABLED(CONSOLE_MUX)
	if (stdinname && bootcmd_dev) {
		char iomux_name[64];

		snprintf(iomux_name, sizeof(iomux_name),
			 "%s,%s", stdinname, bootcmd_dev->name);
		ret = iomux_doenv(stdin, iomux_name);
		if (ret)
			printf("%s: Error adding %s to stdin\n",
			       __func__, iomux_name);
	}
#endif
#endif
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
	if (!str)
		printf("Error: cannot retrieve board type from fdt\n");

	if (prod_id == CN96XX)
		printf("OcteonTX2 CN96XX ARM V8 Core\n");
	if (prod_id == CN95XX)
		printf("OcteonTX2 CN95XX ARM V8 Core\n");
	if (prod_id == LOKI)
		printf("OcteonTX2 LOKI ARM V8 Core\n");
	if (prod_id == CN98XX)
		printf("OcteonTX2 CN98XX ARM V8 Core\n");

	printf("Board: %s\n", str);
	return 0;
}

void acquire_flash_arb(bool acquire)
{
	union cavm_cpc_boot_ownerx ownerx;

	if (!acquire) {
		ownerx.u = readl(CAVM_CPC_BOOT_OWNERX(3));
		ownerx.s.boot_req = 0;
		writel(ownerx.u, CAVM_CPC_BOOT_OWNERX(3));
	} else {
		ownerx.u = 0;
		ownerx.s.boot_req = 1;
		writel(ownerx.u, CAVM_CPC_BOOT_OWNERX(3));
		udelay(1);
		do {
			ownerx.u = readl(CAVM_CPC_BOOT_OWNERX(3));
		} while (ownerx.s.boot_wait);
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

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
	(void)atf_flsf_fw_booted();
	return 0;
}
#endif
