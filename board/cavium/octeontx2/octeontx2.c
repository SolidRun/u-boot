/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <common.h>
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
extern unsigned long fdt_base_addr;
extern void cgx_intf_shutdown(void);

void board_quiesce_devices(void)
{
	ssize_t node_count = atf_node_count();
	int node;
	struct uclass *uc_dev;
	int ret;

	/* Removes all RVU PF devices */
	ret = uclass_get(UCLASS_ETH, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove rvu pf devices\n");

	/* Bring down all cgx lmac links */
	cgx_intf_shutdown();

	/* Removes all CGX and RVU AF devices */
	ret = uclass_get(UCLASS_MISC, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove misc (cgx/rvu_af) devices\n");

	/* SMC call - removes all LF<->PF mappings */
	for (node = 0; node < node_count; node++) {
		atf_disable_rvu_lfs(node);
	}
}

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r(void)
{
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
	u64 val = readq(CAVM_RST_CHIP_DOM_W1S) | 1;

	writeq(val, CAVM_RST_CHIP_DOM_W1S);
}

/**
 * Board misc devices initialization routine.
 */
void board_misc_init(void)
{
	struct udevice *bus;

	/*
	 * Enumerate all known miscellaneous devices.
	 * Enumeration has the side-effect of probing them,
	 * so CGX and RVU AF devices will get enumerated.
	 */
	for (uclass_first_device(UCLASS_MISC, &bus);
	     bus;
	     uclass_next_device(&bus)) {
		;
	}
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

	board_misc_init();

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
