/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <command.h>
#include <dm.h>
#include <asm/arch/smc.h>
#include <efi_loader.h>

static int do_efi_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	char filesize[64];
	char *endp;
	u64 addr = 0, size = 0;
	int ret = CMD_RET_USAGE;

	if (argc < 2)
		return ret;

	addr = simple_strtoull(argv[1], &endp, 16);
	if (!addr)
		return ret;

#ifdef CONFIG_TARGET_CN20K_A
	u64 dev_flags = 0;
	const char *str;

	efi_get_boot_device_name(&str);
	/* Load image from boot device */
	efi_get_boot_device_mode(str, &dev_flags);
	ret = smc_load_efi_img(addr, &size, dev_flags);
#else
	ret = smc_load_efi_img(addr, &size);
#endif
	if (ret)
		return CMD_RET_FAILURE;

	snprintf(filesize, sizeof(filesize), "%llx", size);
	env_set("filesize", filesize);
	printf("Loaded EFI App image at 0x%llx with 0x%llx bytes\n", addr,
	       size);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	efiload, 2, 1, do_efi_cmd, "Load EFI Application",
	"Load EFI Application image from secure SPI flash to memory\n"
	"efiload [Load address]\n"
	"    - loads EFI_IMAGE1[efi_app1.efi]\n"
	" Loaded image size will be printed out\n"
	"\n"
	"Example - efiload $loadaddr\n"
);
