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
#include <asm/arch/smc.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long fdt_base_addr;
extern void cgx_intf_shutdown(void);

void cleanup_env_ethaddr(void)
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
	u64 tmp_mac, board_mac_addr = fdt_get_board_mac_addr();
	static int board_mac_num;

	board_mac_num = fdt_get_board_mac_cnt();
	if ((!is_zero_ethaddr((u8 *)&board_mac_addr)) && board_mac_num) {
		tmp_mac = board_mac_addr;
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
	smc_disable_rvu_lfs(0);
}

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int board_init(void)
{
	return 0;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = smc_dram_size(0);
	gd->ram_size -= CONFIG_SYS_SDRAM_BASE;

	return 0;
}

/**
 * Board misc devices initialization routine.
 */
int misc_init_r(void)
{
	struct udevice *bus;

	/*
	 * Enumerate all known miscellaneous devices
	 * so CGX and RVU AF devices will be probed.
	 */
	for (uclass_first_device_check(UCLASS_MISC, &bus);
	     bus;
	     uclass_next_device_check(&bus)) {
		;
	}
	return 0;
}

#ifdef CONFIG_OCTEONTX_SERIAL_BOOTCMD
void board_init_serial_bootcmd(void)
{
	struct udevice *bootcmd_dev = NULL;
	int ret;
	char *stdinname = env_get("stdin");

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
}
#endif

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[32];
	char boardserial[150], boardrev[150];
	long val;

	/*
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of QLM can change.
	 */
	cleanup_env_ethaddr();

	snprintf(boardname, sizeof(boardname), "%s> ", fdt_get_board_model());
	env_set("prompt", boardname);
	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));

	if (fdt_get_board_revision()) {
		snprintf(boardrev, sizeof(boardrev), "%s",
			 fdt_get_board_revision());
		env_set("boardrev", boardrev);
	}

	if (fdt_get_board_serial()) {
		snprintf(boardserial, sizeof(boardserial), "%s",
			 fdt_get_board_serial());
		env_set("serial#", boardserial);
	}

	val = env_get_hex("disable_ooo", 0);
	smc_configure_ooo(val);

#ifdef CONFIG_OCTEONTX_SERIAL_BOOTCMD
	board_init_serial_bootcmd();
#endif
	env_save();
	return 0;
}

/*
 * Invoked before relocation, so limit to stack variables.
 */
int show_board_info(void)
{
	char *str = NULL;

	if (otx_is_soc(CN96XX))
		str = "CN96XX";
	if (otx_is_soc(CN95XX))
		str = "CN95XX";
	if (otx_is_soc(LOKI))
		str = "LOKI";
	if (otx_is_soc(CN98XX))
		str = "CN98XX";
	printf("OcteonTX2 %s ARM V8 Core\n", str);

	printf("Board: %s\n", fdt_get_board_model());

	return 0;
}

void acquire_flash_arb(bool acquire)
{
	union cpc_boot_ownerx ownerx;

	if (!acquire) {
		ownerx.u = readl(CPC_BOOT_OWNERX(3));
		ownerx.s.boot_req = 0;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
	} else {
		ownerx.u = 0;
		ownerx.s.boot_req = 1;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
		udelay(1);
		do {
			ownerx.u = readl(CPC_BOOT_OWNERX(3));
		} while (ownerx.s.boot_wait);
	}
}

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	writeq(~0ULL, GTI_CWD_POKEX);
}

void hw_watchdog_disable(void)
{
	writeq(0ULL, GTI_CWD_WDOGX);
}
#endif

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
	(void)smc_flsf_fw_booted();
	return 0;
}
#endif
