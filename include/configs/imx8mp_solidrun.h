/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 * Copyright 2022 SolidRun Ltd.
 */

#ifndef __IMX8MP_SOLIDRUN_H
#define __IMX8MP_SOLIDRUN_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>
#include "imx_env.h"

#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_DISTRO_DEFAULTS
#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2)

#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

#define JH_ROOT_DTB    "imx8mp-hummingboard-pulse-root.dtb"

#define JAILHOUSE_ENV \
	"jh_clk= \0 " \
	"jh_root_dtb=" JH_ROOT_DTB "\0" \
	"jh_mmcboot=setenv fdtfile ${jh_root_dtb};" \
		"setenv jh_clk clk_ignore_unused mem=1920MB; " \
			   "if run loadimage; then " \
				   "run mmcboot; " \
			   "else run jh_netboot; fi; \0" \
	"jh_netboot=setenv fdtfile ${jh_root_dtb}; setenv jh_clk clk_ignore_unused mem=1920MB; run netboot; \0 "

#define CFG_MFG_ENV_SETTINGS \
	CFG_MFG_ENV_SETTINGS_DEFAULT \
	"initrd_addr=0x43800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"emmc_dev=2\0"\
	"sd_dev=1\0"

/*
 * Load Adresses (different from imx8mp_evk.h):
 * - 1MB for boot-script
 * - 1MB for pxe
 * - 1MB for DTB
 * - 1MB for DTB Overlays
 * - 56MB for compressed kernel
 * - 192MB for uncompressed kernel
 * - 761MB for ramdisk (1GB SoM DDR, more otherwise)
 */
#define SCRIPT_ADDR_R		__stringify(0x40400000)
#define PXEFILE_ADDR_R		__stringify(0x40500000)
#define FDT_ADDR_R		__stringify(0x40600000)
#define FDTOVERLAY_ADDR_R	__stringify(0x40700000)
#define KERNEL_COMP_ADDR_R	__stringify(0x40800000)
#define KERNEL_COMP_SIZE	__stringify(0x03800000)
#define KERNEL_ADDR_R		__stringify(0x44000000)
#define RAMDISK_ADDR_R		__stringify(0x50000000)
#define FDT_RELOCATION_LIMIT	__stringify(0xffffffff)

#define CFG_EXTRA_ENV_SETTINGS						\
	CFG_MFG_ENV_SETTINGS						\
	JAILHOUSE_ENV							\
	BOOTENV								\
	"scriptaddr=" SCRIPT_ADDR_R "\0"				\
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0"				\
	"fdt_addr_r=" FDT_ADDR_R "\0"					\
	"fdtoverlay_addr_r=" FDTOVERLAY_ADDR_R "\0"			\
	"fdt_high=" FDT_RELOCATION_LIMIT "\0"				\
	"kernel_comp_addr_r=" KERNEL_COMP_ADDR_R "\0"			\
	"kernel_comp_size=" KERNEL_COMP_SIZE "\0"			\
	"kernel_addr_r=" KERNEL_ADDR_R "\0"				\
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0"				\
	"stdout=vidconsole,serial\0"					\
	"stderr=vidconsole,serial\0"					\
	"stdin=vidconsole,serial\0"					\
	"console=ttymxc1,115200\0" 					\
	"findfdt=setenv fdtfile freescale/imx8mp-$carrier_name.dtb\0"	\
	"fdtfile=undefined\0" \

/* Link Definitions */
#define CFG_SYS_INIT_RAM_ADDR		0x40000000
#define CFG_SYS_INIT_RAM_SIZE		0x80000

/* Totally 8GB DDR */
#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0xC0000000	/* 3 GB */
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		0x140000000	/* 5 GB */

/* Some general LPDDR4 macros */
#define LPDDR4_SAMSUNG_MANID		0x1

#define CFG_MXC_UART_BASE		UART2_BASE_ADDR

#define CFG_SYS_NAND_BASE		0x20000000

#define CFG_SYS_FSL_USDHC_NUM		2

#ifdef CONFIG_ANDROID_SUPPORT
#include "imx8mp_evk_android.h"
#endif

#endif
