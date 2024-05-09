// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2022 Marvell.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <log.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <asm/io.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

struct ecam_cfg {
	unsigned long base;
	u8 domain;
	u8 bus;
	u8 dev;
	u8 func;
} g_ecam_cfg;
struct ecam_cfg bridges[32];

static unsigned long get_ecam_addr(void)
{
	return (g_ecam_cfg.base + (g_ecam_cfg.bus << 20) +
		(g_ecam_cfg.dev << 15) + (g_ecam_cfg.func << 12));
}

static void do_pci_dump_dev(unsigned long addr)
{
	u32 reg;
	u32 sec_bus;

	reg = readl(addr);
	if (reg != 0xffffffff) {
		if (g_ecam_cfg.domain == 0 && g_ecam_cfg.bus == 1) {
			printf("%02x:%02x.%02x\t%x:%x\n",
			       g_ecam_cfg.bus,
			       (g_ecam_cfg.func >> 3 & 0x1f),
			       (g_ecam_cfg.func & 0x7),
			       reg & 0xffff,
			       reg >> 16);
		} else {
			printf("%02x:%02x.%02x\t%x:%x\n",
			       g_ecam_cfg.bus, g_ecam_cfg.dev,
			       g_ecam_cfg.func,
			       reg & 0xffff,
			       reg >> 16);
		}
	}
	bridges[g_ecam_cfg.dev].bus = 0;
	sec_bus = 0;
	if (((reg >> 16) == 0xa002) ||
	    ((reg >> 16) == 0xa02d)) {
		sec_bus = readl(addr + 0x18);
		sec_bus = (sec_bus >> 8) & 0xff;
		if (sec_bus != 0) {
			bridges[g_ecam_cfg.dev].bus = sec_bus;
			debug("sec %d\n", sec_bus);
		}
	}
}

static void do_pci_scan_bus(void)
{
	int maxdev = 32;

	if (g_ecam_cfg.domain > 2)
		maxdev = 1;
	if (g_ecam_cfg.domain == 0 && g_ecam_cfg.bus == 1) {
		for (int dev = 0; dev < 255; dev++) {
			g_ecam_cfg.func = dev;
			g_ecam_cfg.dev = 0;
			do_pci_dump_dev(get_ecam_addr());
		}
	} else {
		for (int dev = 0; dev < maxdev; dev++) {
			g_ecam_cfg.dev = dev;
			g_ecam_cfg.func = 0;
			do_pci_dump_dev(get_ecam_addr());
		}
	}
	g_ecam_cfg.dev = 0;
	g_ecam_cfg.func = 0;
}

static void do_pci_scan_domain(void)
{
	for (int i = 0; i < 32; i++)
		bridges[i].bus = 0;

	g_ecam_cfg.bus = 0;
	do_pci_scan_bus();

	/* Scan Bridges */
	for (int i = 1; i < 256; i++) {
		g_ecam_cfg.bus = i;
		do_pci_scan_bus();
	}
}

static struct udevice *get_pci_udevice(int dmn)
{
	int ret, i;
	struct udevice *dev = NULL;
	ofnode node;
	u32 dmn_val;
	char compat_str[3][64] = { "pci-host-ecam-generic",
				   "cavium,pci-host-thunder-pem",
				   "cavium,pci-host-octeontx-ecam"};

	for (i = 0; i < 3; i++) {
		node = ofnode_by_compatible(ofnode_null(),
					    compat_str[i]);
		while (ofnode_valid(node)) {
			ret = ofnode_read_u32(node, "linux,pci-domain",
					      &dmn_val);
			if (!ret && dmn_val == dmn) {
				ret = uclass_find_device_by_ofnode(UCLASS_PCI,
								   node, &dev);
				if (!ret)
					return dev;
			}

			node = ofnode_by_compatible(node, compat_str[i]);
		}
	}
	return dev;
}

static void do_pci_domain_list(int domain)
{
	struct udevice *root = NULL;

	root = get_pci_udevice(domain);
	if (!root) {
		printf("Domain not found\n");
		return;
	}
	g_ecam_cfg.base = dev_read_addr(root);
	printf("\nDomain %d\n", domain);
	g_ecam_cfg.domain = domain;
	do_pci_scan_domain();
}

static int do_pcilist(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	const char *cmd;
	char *endp;
	int ret = CMD_RET_USAGE, arg;

	if (argc < 2)
		return ret;

	cmd = argv[0];
	arg = simple_strtol(argv[1], &endp, 0);
	if (arg < 0 || arg > 9)
		return ret;

	do_pci_domain_list(arg);
	return 0;
}

U_BOOT_CMD(pcilist, 2, 1, do_pcilist, "Display pci device list",
	   "Prints pci device list with below format for specific domain\n"
	   "Bus:Dev:Func\tVendorID:DeviceID\n"
);
