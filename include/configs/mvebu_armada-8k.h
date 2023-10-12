/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_MVEBU_ARMADA_8K_H
#define _CONFIG_MVEBU_ARMADA_8K_H

#define CONFIG_DEFAULT_CONSOLE		"console=ttyS0,115200 "\
					"earlycon=uart8250,mmio32,0xf0512000"

#include <configs/mvebu_armada-common.h>

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

#define CONFIG_BOARD_EARLY_INIT_R

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_NAND_ONFI_DETECTION

#define CONFIG_USB_MAX_CONTROLLER_COUNT (3 + 3)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x5400000\0"	\
	"pxefile_addr_r=0x5500000\0"	\
	"fdt_addr_r=0x5600000\0"	\
	"kernel_addr_r=0x5700000\0"	\
	"ramdisk_addr_r=0x8700000\0"	\
	"fdtfile=marvell/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	CONFIG_DEFAULT_CONSOLE "\0"\
	BOOTENV

#include <config_distro_bootcmd.h>

/* RTC configuration */
#ifdef CONFIG_MARVELL_RTC
#define ERRATA_FE_3124064
#endif

#endif /* _CONFIG_MVEBU_ARMADA_8K_H */
