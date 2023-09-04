/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the SolidRun mx6 based boards
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __MX6CUBOXI_CONFIG_H
#define __MX6CUBOXI_CONFIG_H

#include <config_distro_defaults.h>
#include "mx6_common.h"

#include "imx6_spl.h"

#define CONFIG_IMX_THERMAL

#define CONFIG_HW_WATCHDOG
#define CONFIG_IMX_WATCHDOG

#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)
#define CONFIG_MXC_UART

/* MMC Configs */
#ifdef CONFIG_CMD_MMC
#define CONFIG_SUPPORT_EMMC_BOOT
#endif

#define CONFIG_SYS_FSL_USDHC_NUM	2

#ifdef CONFIG_SPL_BOOT_DEVICE_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC3_BASE_ADDR
#else
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR
#endif

/* SATA Configuration */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE      1
#define CONFIG_DWC_AHSATA_PORT_ID       0
#define CONFIG_DWC_AHSATA_BASE_ADDR     SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#endif
#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SYS_SCSI_MAX_SCSI_ID 1
#endif

/* Ethernet Configuration */
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_FEC_MXC_PHYADDR		0
#define CONFIG_PHY_ATHEROS
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define FDT_SEQ_MACADDR_FROM_ENV

/* Framebuffer */
#define CONFIG_VIDEO_IPUV3
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP

/* USB */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_PREBOOT \
	"if hdmidet; then " \
		"usb start; "		       \
		"setenv stdin  serial,usbkbd; "\
		"setenv stdout serial,vga; "   \
		"setenv stderr serial,vga; "   \
	"else " \
		"setenv stdin  serial; " \
		"setenv stdout serial; " \
		"setenv stderr serial; " \
	"fi;"

/* Command definition */

#define CONFIG_MXC_UART_BASE	UART1_BASE
#define CONSOLE_DEV	"ttymxc0"

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	"som_rev=undefined\0" \
	"has_emmc=undefined\0" \
	"fdtfile=undefined\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdt_addr=0x18000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0"  \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"ramdiskaddr=0x13000000\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_high=0xffffffff\0" \
	"ip_dyn=yes\0" \
	"is_cbi=no\0" \
	"console=" CONSOLE_DEV ",115200\0" \
	"bootm_size=0x10000000\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"finduuid=part uuid mmc 0:1 uuid\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"findfdt="\
		"if test ${board_rev} = MX6Q; then " \
			"setenv fdtprefix imx6q; fi; " \
		"if test ${board_rev} = MX6DL; then " \
			"setenv fdtprefix imx6dl; fi; " \
		"if test ${som_rev} = V15; then " \
			"setenv fdtsuffix -som-v15; fi; " \
		"if test ${is_cbi} = yes; then " \
			"setenv cbisuffix -cbi; fi; " \
		"if test ${has_emmc} = yes; then " \
			"setenv emmcsuffix -emmc; fi; " \
		"if test ${board_name} = HUMMINGBOARD2 ; then " \
			"setenv fdtfile ${fdtprefix}-hummingboard2${cbisuffix}${emmcsuffix}${fdtsuffix}.dtb; fi; " \
		"if test ${board_name} = HUMMINGBOARD ; then " \
			"setenv fdtfile ${fdtprefix}-hummingboard${emmcsuffix}${fdtsuffix}.dtb; fi; " \
		"if test ${board_name} = CUBOXI ; then " \
			"setenv fdtfile ${fdtprefix}-cubox-i${emmcsuffix}${fdtsuffix}.dtb; fi; " \
		"if test ${fdtfile} = undefined; then " \
			"echo WARNING: Could not determine dtb to use; fi; \0" \
	BOOTENV

#ifdef CONFIG_CMD_MMC
#ifdef CONFIG_SPL_BOOT_DEVICE_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 1) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0) func(MMC, mmc, 1)
#endif
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#if defined(CONFIG_CMD_PXE)
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#if defined(CONFIG_CMD_SATA)
#define BOOT_TARGET_DEVICES_SATA(func) func(SATA, sata, 0)
#else
#define BOOT_TARGET_DEVICES_SATA(func)
#endif

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
        BOOT_TARGET_DEVICES_MMC(func) \
        BOOT_TARGET_DEVICES_USB(func) \
        BOOT_TARGET_DEVICES_SATA(func) \
        BOOT_TARGET_DEVICES_PXE(func) \
        BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>

#else
#define CONFIG_EXTRA_ENV_SETTINGS
#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_CMD_NET
#define CONFIG_TFTP_TSIZE
#endif

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS           1
#define CONFIG_SYS_SDRAM_BASE          MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define CONFIG_ENV_SIZE			(8 * 1024)
#define CONFIG_ENV_OFFSET		(SZ_1M - CONFIG_ENV_SIZE)

#ifdef CONFIG_ENV_IS_IN_MMC
#ifdef CONFIG_SPL_BOOT_DEVICE_MMC
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		1
#else
#define CONFIG_SYS_MMC_ENV_DEV		0
#endif
#endif

/* Reboot after 60 sec if bootcmd fails */
/*
#define CONFIG_RESET_TO_RETRY
#define CONFIG_BOOT_RETRY_TIME 60

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_ENV
*/

#endif                         /* __MX6CUBOXI_CONFIG_H */
