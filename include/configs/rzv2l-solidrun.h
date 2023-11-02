/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 Renesas Electronics Corporation
 */

#ifndef __RZV2L_SOLIDRUN_H
#define __RZV2L_SOLIDRUN_H

#include <asm/arch/rmobile.h>

#define CONFIG_REMAKE_ELF

#ifdef CONFIG_SPL
#define CONFIG_SPL_TARGET	"spl/u-boot-spl.scif"
#endif

/* boot option */

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* Generic Interrupt Controller Definitions */
/* RZ/V2L use GIC-v3 */
#define CONFIG_GICV3
#define GICD_BASE	0x11900000
#define GICR_BASE	0x11960000

/* console */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 38400 }

/* PHY needs a longer autoneg timeout */
#define PHY_ANEG_TIMEOUT		20000

/* MEMORY */
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE

/* SDHI clock freq */
#define CONFIG_SH_SDHI_FREQ		133000000

#define DRAM_RSV_SIZE			0x08000000
#define CONFIG_SYS_SDRAM_BASE		(0x40000000 + DRAM_RSV_SIZE)
#define CONFIG_SYS_SDRAM_SIZE		(0x80000000u - DRAM_RSV_SIZE) //total 2GB
#define CONFIG_SYS_LOAD_ADDR		0x58000000
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR // Default load address for tfpt,bootp...
#define CONFIG_VERY_BIG_RAM
#define CONFIG_MAX_MEM_MAPPED		(0x80000000u - DRAM_RSV_SIZE)

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(1 * 1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)
#define CONFIG_SYS_BOOTM_LEN		(64 << 20)

/* The HF/QSPI layout permits up to 1 MiB large bootloader blob */
#define CONFIG_BOARD_SIZE_LIMIT		1048576

#define BOOT_TARGET_DEVICES(func) \
        func(MMC, mmc, 0) \
        func(USB, usb, 0) \
        func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define KERNEL_ADDR_R       __stringify(0x48000000)
#define KERNEL_COMP_ADDR_R  __stringify(0x4a000000)
#define FDT_ADDR_R          __stringify(0x4c000000)
#define SCRIPT_ADDR_R       __stringify(0x4c100000)
#define PXEFILE_ADDR_R      __stringify(0x4c200000)
#define RAMDISK_ADDR_R      __stringify(0x4c800000)
#define KERNEL_COMP_SIZE    __stringify(0xb00000)

#define CONFIG_EXTRA_ENV_SETTINGS \
    "kernel_addr_r=" KERNEL_ADDR_R "\0" \
    "fdt_addr_r=" FDT_ADDR_R "\0" \
    "ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
    "scriptaddr=" SCRIPT_ADDR_R "\0" \
    "pxefile_addr_r=" PXEFILE_ADDR_R "\0" \
    "fdt_high=0xffffffffffffffff\0"	\
    "initrd_high=0xffffffffffffffff\0" \
    "fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
    "kernel_comp_addr_r=" KERNEL_COMP_ADDR_R "\0" \
    "kernel_comp_size=" KERNEL_COMP_SIZE "\0" \
BOOTENV

/* For board */
/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

#endif /* __RZV2L_DEV_H */
