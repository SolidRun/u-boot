// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2021 Marvell
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <net.h>

extern int eth_intf_set_mode(struct udevice *ethdev, int mode, int port);
extern int eth_intf_get_mode(struct udevice *ethdev, int port);
extern int eth_intf_ecp_dump(struct udevice *ethdev, int port, int lmac_id);
extern int eth_intf_set_fec(struct udevice *ethdev, int type);
extern int eth_intf_get_fec(struct udevice *ethdev);
extern void nix_print_mac_info(struct udevice *dev);

static int do_ethlist(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct udevice *dev;

	for (uclass_find_first_device(UCLASS_ETH, &dev); dev;
	     uclass_find_next_device(&dev)) {
		printf("eth%d [%s]", dev->seq, dev->name);
		if (!strncmp(dev->name, "rvu_", 4))
			nix_print_mac_info(dev);
		printf("\n");
	}
	return 0;
}

U_BOOT_CMD(
	ethlist, 1, 1, do_ethlist, "Display ethernet interface list",
	"Prints all detected ethernet interfaces with below format\n"
	"ethX [device name] [LMAC info for RVU PF devices]\n"
);

static int do_ethparam_common(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	const char *cmd;
	char *endp;
	const char *devname;
	int ret = CMD_RET_USAGE, arg, port;
	struct udevice *dev = NULL;

	if (argc < 2)
		return ret;

	cmd = argv[0];

	devname = argv[1];
	dev = eth_get_dev_by_name(devname);
	if (!dev) {
		printf("device interface %s not found\n", devname);
		return CMD_RET_FAILURE;
	}
	if (strncmp(dev->name, "rvu_", 4)) {
		printf("Invalid eth interface choose RVU PF device\n");
		return CMD_RET_FAILURE;
	}

	if (strcmp(cmd, "set_fec") == 0) {
		if (argc < 3)
			return CMD_RET_FAILURE;
		arg = simple_strtol(argv[2], &endp, 0);
		if (arg < 0 || arg > 2)
			return ret;
		ret = eth_intf_set_fec(dev, arg);
	} else if (strcmp(cmd, "get_fec") == 0) {
		ret = eth_intf_get_fec(dev);
	} else if (strcmp(cmd, "get_mode") == 0) {
		if (argc < 3)
			port = -1;
		else
			port = simple_strtol(argv[2], &endp, 0);

		ret = eth_intf_get_mode(dev, port);
	} else if (strcmp(cmd, "set_mode") == 0) {
		if (argc < 3)
			return CMD_RET_FAILURE;
		arg = simple_strtol(argv[2], &endp, 0);
		if (arg < 0)
			return ret;

		if (argc < 4)
			port = -1;
		else
			port = simple_strtol(argv[3], &endp, 0);

		ret = eth_intf_set_mode(dev, arg, port);
	} else if (strcmp(cmd, "ecp_dump") == 0) {
		int lmac_id = 0;

		if (argc < 3)
			return CMD_RET_USAGE;

		port = simple_strtol(argv[2], &endp, 0);
		if (port < 0)
			return CMD_RET_USAGE;

		if (argc > 3)
			lmac_id = simple_strtol(argv[3], &endp, 0);

		if (lmac_id < 0)
			return CMD_RET_USAGE;

		ret = eth_intf_ecp_dump(dev, port, lmac_id);
	}
	return (ret == 0) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

U_BOOT_CMD(
	set_fec, 3, 1, do_ethparam_common,
	"Modify fec type for selected ethernet interface",
	"Example - set_fec <ethX> [type]\n"
	"Set FEC type for any of RVU PF based network interfaces\n"
	"- where type - 0 [NO FEC] 1 [BASER_FEC] 2 [RS_FEC]\n"
	"Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(
	get_fec, 2, 1, do_ethparam_common,
	"Display fec type for selected ethernet interface",
	"Example - get_fec <ethX>\n"
	"Get FEC type for any of RVU PF based network interfaces\n"
	"Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(get_mode, 3, 1, do_ethparam_common,
	   "Display Interface mode for selected ethernet interface",
	   "Example - get_mode <ethX> [portm#]\n"
	   "Use 'ethlist' command to display network interface names\n\n"

	   "For NIX connected interfaces, the target interface\n"
	   "to query mode from is selected by providing <ethX>\n"
	   "parameter.\n\n"

	   "For Ethernet Ports connected to BPHY, u-boot doesn't\n"
	   "register ethX interface. Thus to query mode use one\n"
	   "of the registered ethX interface connected to NIX\n"
	   "for sending message to ATF, and pass PORTM# index\n"
	   "of BPHY interface to query mode from.\n\n"

	   "Example 1 - get mode from eth1 (NIX connected port):\n"
	   "get_mode eth1\n\n"

	   "Example 2 - get mode from PORTM10 (BPHY connected port or:\n"
	   "NIX connected port Ex: Multi-LMAC modes)\n"
	   "get_mode eth0 10\n"
);

U_BOOT_CMD(
	ecp_dump, 4, 1, do_ethparam_common,
	"Dump ECP state log tables for a given PORTM",
	"<ethX> <portm#> [lmac#]\n\n"

	"Parameters:\n"
	"ethX:   Registered network interface for sending the message to ATF\n"
	"\tUse 'ethlist' command to display network interface names\n"
	"portm#: Port index for which to dump the ECP state tables\n"
	"lmac#:  (Optional) - provide it only for multi-lmac ports (QSGMII, USGMII etc)\n"
	"\tto indicate particular lmac within the port for which the tables should be dumped\n"
);

/* Mode Encoding for command help should be in compliant
 * with eth_mode_t defined in eth_intf.h
 * FIXME: Only added modes that are supported by ATF
 */
U_BOOT_CMD(set_mode, 4, 1, do_ethparam_common,
	"Modify Interface mode for selected ethernet interface",
	"Example - set_mode <ethX> mode [portm_idx]\n"
	"Change mode of selected network interface\n"
	"\n"
	"mode encoding -\n\n"
	"Ethernet modes:\n"
	"    0 - SGMII\n"
	"    1 - 1G-X\n"
	"    3 - 10G_C2C\n"
	"    4 - 10G_C2M\n"
	"    5 - 10G_KR\n"
	"    7 - 25G_C2C\n"
	"    8 - 25G_C2M\n"
	"   10 - 25G_CR\n"
	"   11 - 25G_KR\n"
	"   12 - 40G_C2C\n"
	"   13 - 40G_C2M\n"
	"   14 - 40G_CR4\n"
	"   15 - 40G_KR4\n"
	"   17 - 50G_1_C2C\n"
	"   18 - 50G_1_C2M\n"
	"   20 - 50G_CR\n"
	"   21 - 50G_KR\n"
	"   23 - 100G_4_C2C\n"
	"   24 - 100G_4_C2M\n"
	"   25 - 100G_CR4\n"
	"   26 - 100G_KR4\n"
	"   27 - 50G_2_C2C\n"
	"   28 - 50G_2_C2M\n"
	"   29 - 50G_CR2\n"
	"   30 - 50G_KR2\n"
	"   31 - 100G_2_C2C\n"
	"   32 - 100G_2_C2M\n"
	"   33 - 100G_CR2\n"
	"   34 - 100G_KR2\n"
	"   35 - SFI_1G\n"
	"   36 - 25G_CR_C\n"
	"   37 - 25G_KR_C\n"
	"   42 - 2500BASE-X\n"
	"CPRI modes:\n"
	"   84 - CPRI_2_4G\n"
	"   85 - CPRI_3_1G\n"
	"   86 - CPRI_4_9G\n"
	"   87 - CPRI_6_1G\n"
	"   88 - CPRI_9_8G\n"
	"   89 - CPRI_2_4G_TEST\n"
	"   90 - CPRI_3_1G_TEST\n"
	"   91 - CPRI_4_9G_TEST\n"
	"   92 - CPRI_6_1G_TEST\n"
	"   93 - CPRI_9_8G_TEST\n"
	"   94 - CPRI_12_3G_TEST\n"
	"   95 - CPRI_19_7G_TEST\n"

	"Use 'ethlist' command to display network interface names\n"
);

