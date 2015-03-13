/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <linux/compiler.h>

#ifdef CONFIG_OF_LIBFDT
 #include <libfdt.h>
 #include <fdt_support.h>
#endif

#if defined(CONFIG_OF_LIBFDT)

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	int err;

	int nodeoffset = fdt_path_offset(blob, "/cpus");
	char *coremask = getenv("coremask");

	if (coremask == NULL)
		return;

	if (nodeoffset < 0) {
		printf("WARNING: could not find /cpus: %s.\n",
		       fdt_strerror(nodeoffset));
		return;
	}

	err = fdt_setprop(blob, nodeoffset, "coremask",
			  coremask, sizeof(coremask));

	if (err < 0) {
		printf("WARNING: could not set %s: %s.\n", "coremask",
		       fdt_strerror(err));
		return;
	}
}

#endif

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

#endif
