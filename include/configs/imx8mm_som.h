/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef __IMX8MM_SOM_H
#define __IMX8MM_SOM_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>
#include "imx_env.h"

#define UBOOT_ITB_OFFSET	0x57C00
#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CFG_MALLOC_F_ADDR	0x930000
/* For RAW image gives a error info not panic */

#endif

#define PHY_ANEG_TIMEOUT	20000

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS			\
	"scriptaddr=0x40400000\0"		\
	"pxefile_addr_r=0x40500000\0"		\
	"fdt_addr_r=0x40600000\0"		\
	"kernel_addr_r=0x40700000\0"		\
	"ramdisk_addr_r=0x44700000\0"		\
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"	\
	"console=ttymxc1,115200\0"		\
	"stdout=vidconsole,serial\0"		\
	"stderr=vidconsole,serial\0"		\
	"stdin=vidconsole,serial\0"		\
	BOOTENV

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x200000

#define CFG_SYS_SDRAM_BASE	0x40000000
#define PHYS_SDRAM		0x40000000
#define PHYS_SDRAM_SIZE		0x80000000 /* 2GB DDR */

#define CFG_FEC_MXC_PHYADDR	4

#define CFG_MXC_UART_BASE	UART_BASE_ADDR(2)

#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

#ifdef CONFIG_IMX_MATTER_TRUSTY
#define NS_ARCH_ARM64		1
#endif

#endif
