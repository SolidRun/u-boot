/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * (C) Copyright 2014, Cavium Inc.
**/

#ifndef __THUNDERX_88XX_H__
#define __THUNDERX_88XX_H__

#define CONFIG_REMAKE_ELF

#define CONFIG_THUNDERX

#define CONFIG_SPECIAL_SYNC_HANDLER

/*#define CONFIG_ARMV8_SWITCH_TO_EL1*/

#define CONFIG_SYS_GENERIC_BOARD

#define CONFIG_SYS_64BIT

#define MEM_BASE			0x00500000

#define CONFIG_SYS_LOWMEM_BASE		MEM_BASE

/* Link Definitions */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

#define CONFIG_BOARD_LATE_INIT

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP

/* SMP Spin Table Definitions */
#define CPU_RELEASE_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(0x1800000)	/* 24MHz */

#define CONFIG_SYS_MEMTEST_START	MEM_BASE
#define CONFIG_SYS_MEMTEST_END		(MEM_BASE + PHYS_SDRAM_1_SIZE)

/* Size of malloc() pool */

#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 64 * 1024 * 1024)
#define CONFIG_SYS_MEM_TOP_HIDE		0x1000000

#define CONFIG_AP_STACK_SIZE		65536
#define CONFIG_AP_STACK_ALIGN		16

/* PL011 Serial Configuration */

#define CONFIG_PL011_CLOCK		24000000

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			(0x801000000000)
#define GICR_BASE			(0x801000002000)
#define CONFIG_SYS_SERIAL0		0x87e024000000
#define CONFIG_SYS_SERIAL1		0x87e025000000

#define CONFIG_BAUDRATE			115200

/* Net */
#define CONFIG_THUNDER_BGX
#define CONFIG_THUNDER_SMI
#define CONFIG_RANDOM_MACADDR

#ifndef CONFIG_RANDOM_MACADDR
#define CONFIG_ETHADDR			aa:d3:31:40:11:00
#endif
#define CONFIG_OVERWRITE_ETHADDR_ONCE

#define CONFIG_PHYLIB

/* Command line configuration */
#define CONFIG_MENU

/*#define CONFIG_MENU_SHOW*/
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_BDI
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_RUN
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_SOURCE

#define CONFIG_CMD_I2C

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_TFTP
#define CONFIG_CMD_TFTPPUT
#define CONFIG_CMD_NFS

#define CONFIG_CMD_SAVEENV

#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_USB
#define CONFIG_CMD_PART
#define CONFIG_CMD_SATA

#define CONFIG_CMD_ATF

/* AHCI support Definitions */
#ifdef CONFIG_CMD_SATA
  #define CONFIG_SATA_AHCI
  #define CONFIG_SYS_SATA_MAX_DEVICE	16
  #define CONFIG_SATA_BASE_ADDR			0x810000000000
  #define CONFIG_LBA48
  #define CONFIG_LIBATA
  #define CONFIG_SYS_64BIT_LBA
#endif

#define CONFIG_CMD_ATF
#define CONFIG_CMD_VNIC

/* Partition systems */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUIDS

/* PCIe */
#define CONFIG_PCI
#define CONFIG_SYS_PCI_64BIT
#define CONFIG_PCI_SCAN_SHOW
#undef CONFIG_PCI_ENUM_ONLY
#define CONFIG_PCI_PNP
#define CONFIG_THUNDER_ECAMS 4

#define CONFIG_SYS_CACHELINE_SIZE 128

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_THUNDERX
#define CONFIG_SYS_I2C_THUNDERX_SPEED_0 100000
#define CONFIG_SYS_I2C_THUNDERX_SLAVE_0 0
#define CONFIG_SYS_I2C_THUNDERX_SPEED_1 100000
#define CONFIG_SYS_I2C_THUNDERX_SLAVE_1 0

#define CONFIG_DDR_SPD
#define CONFIG_SYS_SPD_ADDR_LIST {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57}
#define CONFIG_SYS_SPD_I2C_BUS 1

#define CONFIG_CMD_LOADB
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_SAVES

/* Partition systems */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUIDS

#define CONFIG_SYS_CACHELINE_SIZE 128

#define CONFIG_THUNDERX_VNIC

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(MEM_BASE)

/* Physical Memory Map */
#define PHYS_SDRAM_1			(MEM_BASE)	  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		(0x80000000-MEM_BASE)	/* 2048 MB */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Initial environment variables */
#define UBOOT_IMG_HEAD_SIZE		0x40
/* C80000 - 0x40 */
#define CONFIG_EXTRA_ENV_SETTINGS	\
					"kernel_addr=08007ffc0\0"	\
					"fdt_addr=0x94C00000\0"		\
					"fdt_high=0x9fffffff\0"		\
					"autoload=0\0"

/* Do not preserve environment */
#define CONFIG_ENV_SIZE			0x1000

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_THUNDERX
#define CONFIG_SYS_I2C_THUNDERX_SPEED_0	100000
#define CONFIG_SYS_I2C_THUNDERX_SLAVE_0	0
#define CONFIG_SYS_I2C_THUNDERX_SPEED_1	100000
#define CONFIG_SYS_I2C_THUNDERX_SLAVE_1	0


/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		64		/* max command args */
#define CONFIG_NO_RELOCATION		1
#define PLL_REF_CLK			50000000	/* 50 MHz */
#define NS_PER_REF_CLK_TICK		(1000000000/PLL_REF_CLK)

#endif /* __THUNDERX_88XX_H__ */
