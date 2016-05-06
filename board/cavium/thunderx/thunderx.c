// SPDX-License-Identifier: GPL-2.0+
/**
 * (C) Copyright 2014, Cavium Inc.
**/

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <asm/io.h>

#include <linux/compiler.h>

#include <cavium/atf.h>

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

#ifdef CONFIG_THUNDERX_VNIC
 #include <cavium/thunderx_smi.h>
 #include <cavium/thunderx_vnic.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define BOARD_TYPE "BOARD="

int board_init(void)
{
	ulong fdt_addr = (ulong)gd->fdt_blob;
	set_working_fdt_addr(fdt_addr);

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

	printf("Initializing\nNodes in system: %zd\n", node_count);

	gd->ram_size = 0;

	for (node = 0; node < node_count; node++) {
		dram_size = atf_dram_size(node);
		printf("Node %d: %zd MBytes of DRAM\n", node, dram_size >> 20);
		gd->ram_size += dram_size;
	}

	gd->ram_size -= MEM_BASE;

	*(unsigned long *)CPU_RELEASE_ADDR = 0;

	puts("DRAM size:");

	return 0;
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
	u64 val = readq(RST_SOFT_RST) | 1;

	writeq(val, RST_SOFT_RST);
}


/*
 * Board late initialization routine.
 */
int board_late_init(void)
{
	thunderx_parse_bdk_config();

	printf("Board type: %s\n", getenv("board"));

	return 0;
}

/*
 * Board specific ethernet initialization routine.
 */

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#if defined(CONFIG_THUNDERX_VNIC)
	struct nicpf* nicpf;
	unsigned int node;

#ifdef CONFIG_RANDOM_MACADDR
	unsigned char ethaddr[6];

	if (!eth_getenv_enetaddr("ethaddr", ethaddr)) {
		net_random_ethaddr(ethaddr);
		printf("Generating random MAC address: %pM\n", ethaddr);
		eth_setenv_enetaddr("ethaddr", ethaddr);
	}
#endif
#endif
#if defined(CONFIG_THUNDERX_SMI)
	thunderx_smi_initialize(bis, 0);
	thunderx_smi_initialize(bis, 1);
#endif

#if defined(CONFIG_THUNDERX_VNIC)
#define VNIC_PER_NODE 8

	for (node = 0; node < atf_node_count(); node++) {
		thunderx_bgx_initialize(0, node);
		thunderx_bgx_initialize(1, node);
	}

	for (node = 0; node < atf_node_count(); node++) {
		nicpf = nic_initialize(node);

		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 0, node);
		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 1, node);
		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 2, node);
		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 3, node);

		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 4, node);
		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 5, node);
		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 6, node);
		nicvf_initialize(nicpf, VNIC_PER_NODE * node + 7, node);
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
			writeq(~0ULL, CSR_PA(node, GTI_CWD_POKE(core)));
}

void hw_watchdog_disable(void)
{
	ssize_t node, core;

	for (node = 0; node < atf_node_count(); node++)
		for (core = 0; core < thunderx_core_count(); core++)
			writeq(0ULL, CSR_PA(node, GTI_CWD_WDOG(core)));
}
#endif
