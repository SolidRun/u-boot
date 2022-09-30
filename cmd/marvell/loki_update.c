// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <asm/arch/smc.h>

static int do_sec_update(struct cmd_tbl *cmdtp, int flag, int argc,
			 char * const argv[])
{
	char *env1, *env2;
	int addr, size;
	int ret;

	if (argc == 1) {
		env1 = env_get("fileaddr");
		env2 = env_get("filesize");
		if (!env1 || !env2) {
			printf("Missing env variables fileaddr/filesize\n");
			return CMD_RET_USAGE;
		}
	} else if (argc == 3) {
		env1 = argv[1];
		env2 = argv[2];
	} else {
		return CMD_RET_USAGE;
	}

	ret = strict_strtoul(env1, 16, &addr);
	if (ret)
		return -1;

	ret = strict_strtoul(env2, 16, &size);
	if (ret)
		return -1;

	return smc_update(addr, size);
}

U_BOOT_CMD(
	sec_update, 1, 1, do_sec_update, "Updates Boot Image",
	"sec_update [image_address] [image_size]"
);
