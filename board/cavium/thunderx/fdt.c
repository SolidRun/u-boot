/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <linux/compiler.h>

#include <libfdt.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <asm/arch/atf.h>

#ifdef CONFIG_THUNDERX_VNIC
# include <asm/arch/thunderx_vnic.h>
#endif

#define MAX_LMAC_PER_BGX 4

DECLARE_GLOBAL_DATA_PTR;

static void thunderx_parse_phy_address(const void *fdt, int node)
{
	char bgxname[32];
	int bgx_id, phy_id;
	unsigned int phy_addr[MAX_LMAC_PER_BGX] = {-1, -1, -1, -1}, mdio_bus = 0;
	const char *buffer;
	uint32_t val;

	for (bgx_id = 0; bgx_id < CONFIG_MAX_BGX_PER_NODE; bgx_id++) {
		for (phy_id = 0; phy_id < MAX_LMAC_PER_BGX; phy_id++)   {
			snprintf(bgxname, sizeof(bgxname),
				 "PHY-ADDRESS.N0.BGX%d.P%d", bgx_id, phy_id);
			buffer = fdt_getprop(fdt, node, bgxname, NULL);
			if (buffer != NULL) {
				val = simple_strtoul(buffer, NULL, 16);
				mdio_bus = (val >> 8) & 0xF;
				phy_addr[phy_id] = val & 0xFF;
			} else {
				debug("Err: cannot retrieve phy address from fdt:%d:%d\n", bgx_id, phy_id);
			}
		}
		bgx_set_board_info(bgx_id, mdio_bus, &phy_addr[0]);
	}
}

void thunderx_parse_bdk_config(void)
{
	char boardname[32];
	const char *str;
#ifdef CONFIG_THUNDERX_BGX
	uint64_t val64;
	uint8_t ethaddr[6];
	uint8_t mac_addr[6] = { 0, 0, 0, 0, 0, 0};
	int i, shift;
#endif
	int node;
	int ret = 0, len = sizeof(boardname);

	debug("%s: ENTER\n", __func__);
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

	node = fdt_path_offset(gd->fdt_blob, "/cavium,bdk");
	if (node < 0) {
		printf("%s: /cavium,bdk is missing from device tree: %s\n",
		       __func__, fdt_strerror(node));
		return;
	}

	debug("fdt:size %d\n", fdt_totalsize(gd->fdt_blob));
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MODEL", &len);
	debug("fdt: BOARD-MODEL str %s len %d\n", str, len);
	if (str) {
		strncpy(boardname, str, sizeof(boardname));
		setenv("board", boardname);
	} else {
		printf("Error: cannot retrieve board type from fdt\n");
	}

#ifdef CONFIG_THUNDERX_BGX
	thunderx_parse_phy_address(gd->fdt_blob, node);
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MAC-ADDRESS", &len);
	if (!str) {
		printf("%s: BOARD-MAC-ADDRESS missing from device tree\n",
		       __func__);
		return;
	}
	val64 = simple_strtoull(str, NULL, 16);
	for (i = 0, shift = 40; i < 6; i++, shift -= 8)
		mac_addr[i] = (val64 >> shift) & 0xff;

	if (!eth_getenv_enetaddr("ethaddr", ethaddr)) {
		char addr_str[18];
		snprintf(addr_str, sizeof(addr_str), "%pM", mac_addr);
		debug("Setting ethaddr to \"%s\"\n", addr_str);
		setenv_force("ethaddr", addr_str);
	} else {
		memcpy(mac_addr, ethaddr, sizeof(mac_addr));
	}
	printf("Board MAC address: %pM\n", mac_addr);
#endif
}

int arch_fixup_memory_node(void *blob)
{
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	/* remove "cavium, bdk" node from DT */
	int ret = 0, offset;

	ret = fdt_check_header(blob);
	if (ret < 0) {
		printf("ERROR: %s\n", fdt_strerror(ret));
		return ret;
	}

	if (blob != NULL) {
		offset = fdt_path_offset(blob, "/cavium,bdk");
		if(offset < 0) {
			printf("ERROR: FDT BDK node not found\n");
			return offset;
		}

		/* delete node */
		ret = fdt_del_node(blob, offset);
		if (ret < 0) {
			printf("WARNING : could not remove cavium, bdk node\n");
			return ret;
		}

		debug("%s deleted 'cavium,bdk' node\n", __FUNCTION__);
	}

	return 0;
}


#define NODENAME_BUFLEN 32

int ft_getcore(void *blob, char *id)
{
	int nodeoffset;
	char nodename[NODENAME_BUFLEN];

	snprintf(nodename, sizeof(nodename), "/cpus/cpu@%s", id);

	nodeoffset = fdt_path_offset(blob, nodename);

	if (nodeoffset < 0) {
		printf("WARNING: could not find %s: %s.\n", nodename,
		       fdt_strerror(nodeoffset));
	}

	return nodeoffset;
}

void ft_coreenable(bd_t *bd, char *id, int enable)
{
	int err;
	void *blob = working_fdt;

	int nodeoffset = ft_getcore(blob, id);

	if (nodeoffset < 0)
		return;

	err = fdt_setprop_u32(blob, nodeoffset, "enabled", enable);

	if (err < 0) {
		printf("WARNING: could not set %s: %s.\n", "enabled",
		       fdt_strerror(err));
		return;
	}
}

int ft_corestatus(bd_t *bd, char *id)
{
	const u32 *enabled;
	void *blob = working_fdt;

	int nodeoffset = ft_getcore(blob, id);

	if (nodeoffset < 0)
		return -1;

	enabled = fdt_getprop(blob, nodeoffset, "enabled", NULL);

	if (enabled == NULL)
		return 1;
	else
		return *enabled;
}

static int parse_argv(const char *s)
{
	if (strncmp(s, "en", 2) == 0)
		return 1;
	else if (strncmp(s, "di", 2) == 0)
		return 0;

	return -1;
}

int do_cpucore(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int enable;

	switch (argc) {
	case 3:			/* enable / disable	*/
		enable = parse_argv(argv[2]);

		if (enable < 0)
			return CMD_RET_USAGE;
		else
			ft_coreenable(NULL, argv[1], enable);

		break;
	case 2:			/* get status */
		printf("CPU Core %s is %s\n", argv[1],
		       ft_corestatus(NULL, argv[1]) ? "ENABLED" : "DISABLED");
		return 0;
	default:
		return CMD_RET_USAGE;
	}
	return 0;
}

U_BOOT_CMD(
	cpucore,   3,   1,     do_cpucore,
	"enable or disable a CPU core",
	"id [enable, disable]\n"
	"    - enable or disable a CPU core"
);

