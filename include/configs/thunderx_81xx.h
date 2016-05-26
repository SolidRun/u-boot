/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#ifndef __THUNDERX_81XX_H__
#define __THUNDERX_81XX_H__

#define CONFIG_SPECIAL_SYNC_HANDLER

/*#define CONFIG_ARMV8_SWITCH_TO_EL1*/

#define CONFIG_SYS_64BIT

#define CONFIG_SYS_NO_FLASH

#define CONFIG_SUPPORT_RAW_INITRD

#define CONFIG_SYS_BOOTM_LEN (16 << 20)

#define CONFIG_SYS_VSNPRINTF

#define CONFIG_IDENT_STRING	\
	" for Cavium Thunder CN81XX ARM v8 Core"
#define CONFIG_BOOTP_VCI_STRING		"Diagnostics"

#define MEM_BASE			0x00500000

#define CONFIG_COREID_MASK             0xffffff

#define CONFIG_SYS_FULL_VA

#define CONFIG_SYS_LOWMEM_BASE		MEM_BASE

#define CONFIG_SYS_MEM_MAP		{{0x000000000000UL, 0x40000000000UL, \
					  PTL2_MEMTYPE(MT_NORMAL) |	     \
					  PTL2_BLOCK_NON_SHARE},	     \
					 {0x800000000000UL, 0x40000000000UL, \
					  PTL2_MEMTYPE(MT_DEVICE_NGNRNE) |   \
					  PTL2_BLOCK_NON_SHARE},	     \
					 {0x840000000000UL, 0x40000000000UL, \
					  PTL2_MEMTYPE(MT_DEVICE_NGNRNE) |   \
					  PTL2_BLOCK_NON_SHARE},	     \
					 {0x880000000000UL, 0x40000000000UL, \
					  PTL2_MEMTYPE(MT_DEVICE_NGNRNE) |   \
					  PTL2_BLOCK_NON_SHARE},	     \
					 {0xffffffffffffUL, 0xfffffffffffUL, \
					  PTL2_MEMTYPE(MT_DEVICE_NGNRNE) |   \
					  PTL2_BLOCK_NON_SHARE},	     \
					}

#define CONFIG_SYS_MEM_MAP_SIZE		4

#define CONFIG_COREID_MASK		0xffffff
#define CONFIG_SYS_FULL_VA

#define CONFIG_SYS_VA_BITS		48
#define CONFIG_SYS_PTL2_BITS		42
#define CONFIG_SYS_BLOCK_SHIFT		29
#define CONFIG_SYS_PTL1_ENTRIES		64
#define CONFIG_SYS_PTL2_ENTRIES		8192

#define CONFIG_SYS_PGTABLE_SIZE		\
	((CONFIG_SYS_PTL1_ENTRIES + \
	  CONFIG_SYS_MEM_MAP_SIZE * CONFIG_SYS_PTL2_ENTRIES) * 8)
#define CONFIG_SYS_TCR_EL1_IPS_BITS	(5UL << 32)
#define CONFIG_SYS_TCR_EL2_IPS_BITS	(5 << 16)
#define CONFIG_SYS_TCR_EL3_IPS_BITS	(5 << 16)

#define CONFIG_SLT

/* Link Definitions */
#define CONFIG_SYS_TEXT_BASE		0x00500000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

#define CONFIG_BOARD_LATE_INIT

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT
/* #define CONFIG_OF_BOARD_SETUP */

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

#define CONFIG_PL01X_SERIAL
#define CONFIG_PL011_CLOCK		24000000
#define CONFIG_CONS_INDEX		1

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			(0x801000000000)
#define GICR_BASE			(0x801000002000)
#define CONFIG_SYS_SERIAL0		0x87e028000000
#define CONFIG_SYS_SERIAL1		0x87e029000000

#define CONFIG_BAUDRATE			115200

/* Net */
#define CONFIG_THUNDERX_BGX
#define CONFIG_THUNDERX_SMI
#define CONFIG_THUNDERX_VNIC
#define CONFIG_RANDOM_MACADDR

#ifndef CONFIG_RANDOM_MACADDR
#define CONFIG_ETHADDR			aa:d3:31:40:11:00
#endif
#define CONFIG_OVERWRITE_ETHADDR_ONCE

#define CONFIG_MAX_BGX_PER_NODE		2
#define CONFIG_MAX_BGX			2

#define CONFIG_PHYLIB_10G


/* Command line configuration */
#define CONFIG_MENU

/*#define CONFIG_MENU_SHOW*/
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ENV
#undef  CONFIG_CMD_IMLS
#define CONFIG_CMD_BOOTI

#define CONFIG_CMD_MII
#define CONFIG_CMD_TFTP

#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_PART
#define CONFIG_CMD_SATA

#define CONFIG_CMD_PCI

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_ENV_FLAGS
#define CONFIG_CMD_GREPENV
#define CONFIG_CMD_ENV_CALLBACK

#define CONFIG_CMD_DATE

#define CONFIG_ENV_CALLBACK_LIST_STATIC "txsmi\\d?mode:smimode"

/* AHCI support Definitions */
#ifdef CONFIG_CMD_SATA
  #define CONFIG_SATA_AHCI
  #define CONFIG_SYS_SATA_MAX_DEVICE	2
  #define CONFIG_LBA48
  #define CONFIG_LIBATA
  #define CONFIG_SYS_64BIT_LBA
#endif

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
#define CONFIG_SYS_PCI_ARI
#define CONFIG_THUNDERX_ECAMS 1
#define CONFIG_THUNDERX_RCS 3

#define CONFIG_SYS_CACHELINE_SIZE 128

#define CONFIG_SYS_I2C_THUNDERX
#define CONFIG_SYS_RTC_BUS_NUM 0
#define CONFIG_SYS_I2C_RTC_ADDR 0x68
#define CONFIG_RTC_DS1337

#define CONFIG_DDR_SPD
#define CONFIG_SYS_SPD_ADDR_LIST {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57}
#define CONFIG_SYS_SPD_I2C_BUS 1

#define CONFIG_CMD_SAVES

/* Partition systems */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUIDS

#define CONFIG_SYS_CACHELINE_SIZE 128

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_PXE_CLIENTARCH	0x100

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(MEM_BASE)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			(MEM_BASE)	  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		(0x80000000-MEM_BASE)	/* 2048 MB */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#define CONFIG_USB_XHCI_PCI
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS 2



/* PCIe network controller drivers */
#define CONFIG_E1000
#define CONFIG_E1000_SPI
#define CONFIG_CMD_E1000

/* #define E1000_DEBUG */

#define CONFIG_RTL8169

/* Initial environment variables */
#define UBOOT_IMG_HEAD_SIZE		0x40
/* C80000 - 0x40 */
#define CONFIG_EXTRA_ENV_SETTINGS	\
					"kernel_addr=04007ffc0\0"	\
					"fdt_addr=0x54C00000\0"		\
					"fdt_high=0x5fffffff\0"		\
					"smi0mode=0.0.0\0"		\
					"smi1mode=0.0.0\0"		\
					"autoload=0\0"

#define CONFIG_BOOTARGS			\
					"console=ttyAMA0,115200n8 " \
					"earlycon=pl011,0x87e028000000 " \
					"debug maxcpus=4 rootwait rw "\
					"root=/dev/sda2 coherent_pool=16M"
#define CONFIG_BOOTDELAY		5

#define CONFIG_ENV_IS_IN_ATF
#define CONFIG_SYS_ENV_ATF_NOR
#define CONFIG_ENV_SIZE			0x80000
#define CONFIG_ENV_OFFSET		0xf00000

#define CONFIG_BDK_FDT_SIZE		0x20000

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_FEATURE_COMMAND_EDITING
#define CONFIG_SYS_MAXARGS		64		/* max command args */
#define CONFIG_NO_RELOCATION		1
#define PLL_REF_CLK			50000000	/* 50 MHz */
#define NS_PER_REF_CLK_TICK		(1000000000/PLL_REF_CLK)

#endif /* __THUNDERX_81XX_H__ */
