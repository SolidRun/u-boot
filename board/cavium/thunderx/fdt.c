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
#include <fdt_support.h>
#include <cavium/atf.h>

#ifdef CONFIG_THUNDERX_VNIC
 #include <cavium/thunderx_vnic.h>
#endif

#define MAX_LMAC_PER_BGX 4

static const void *get_prop_value(void *fdt, const char *prop_name, int *len)
{
	int depth = 0, node;

	node = fdt_next_node(fdt, 0, &depth);
	while (node >= 0) {
		int prop_off;
		prop_off = fdt_first_property_offset(fdt, node);
		while (prop_off >= 0) {
			const char *name;
			const void *val = fdt_getprop_by_offset(fdt, prop_off,
								&name, len);
			if (strcmp(name, prop_name) == 0)
				return val;
			prop_off = fdt_next_property_offset(fdt, prop_off);
		}
		node = fdt_next_node(fdt, node, &depth);
	}
	return NULL;
}

static void thunderx_parse_phy_address(void *fdt)
{
	char bgxname[32];
	const char *str;
	int len = 0, bgx_id, phy_id;
	unsigned int phy_addr[MAX_LMAC_PER_BGX] = {0}, mdio_bus = 0;
	unsigned long buffer = 0x0;

	for (bgx_id = 0; bgx_id < CONFIG_MAX_BGX_PER_NODE; bgx_id++) {
		for (phy_id = 0; phy_id < MAX_LMAC_PER_BGX; phy_id++)   {
			snprintf(bgxname, sizeof(bgxname),
				 "PHY-ADDRESS.N0.BGX%d.P%d", bgx_id, phy_id);
			str = get_prop_value(fdt, bgxname, &len);
			debug("fdt: str %s len %d\n", str, len);
			if (str) {
				buffer = simple_strtoul(str, NULL, 16);
				mdio_bus = (buffer >> 8) & 0xF;
				phy_addr[phy_id] = buffer & 0xFF;
			} else {
				printf("Err: cannot retrieve phy address from fdt\n");
			}
		}
		bgx_set_board_info(bgx_id, mdio_bus, &phy_addr[0]);
	}
}

void thunderx_parse_bdk_config(void)
{
	char boardname[32];
	const char *str;
	void *fdt = (void *)CONFIG_BDK_FDT_START;
	int ret = 0, len = 32;

	atf_get_bdk_fdt(fdt, CONFIG_BDK_FDT_SIZE);
	if (fdt != NULL) {
		ret = fdt_check_header(fdt);
		if (ret < 0) {
			printf("fdt: %s\n", fdt_strerror(ret));
		} else {
			debug("fdt:size %d\n", fdt_totalsize(fdt));
			str = get_prop_value(fdt, "BOARD-MODEL", &len);
			debug("fdt: str %s len %d\n", str, len);
			if (str) {
				strncpy(boardname, str, len);
				setenv("board", boardname);
			} else {
				printf("Err: cannot retrieve board type from fdt\n");
			}
			thunderx_parse_phy_address(fdt);
		}
	}

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

