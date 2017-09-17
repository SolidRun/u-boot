/**
 * (C) Copyright 2017, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#ifndef __THUNDERX_93XX_H__
#define __THUNDERX_93XX_H__


/** Thunder 93xx does not support NOR flash */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_SUPPORT_RAW_INITRD

#define CONFIG_SYS_BOOTM_LEN (16 << 20)

/* starting uboot and linux kernel from offset 40MB
 * below 40MB will be used for mailboxes of pko/pki
 * */

#define MEM_BASE			0x2800000

#define CONFIG_COREID_MASK             0xffffff

/** Low memory base address */
#define CONFIG_SYS_LOWMEM_BASE		MEM_BASE

/** IPS field of TCR_EL1 */
#define CONFIG_SYS_TCR_EL1_IPS_BITS	(5UL << 32)

/** IPS field of TCR_EL2 */
#define CONFIG_SYS_TCR_EL2_IPS_BITS	(5 << 16)

/** IPS field of TCR_EL3 */
#define CONFIG_SYS_TCR_EL3_IPS_BITS	(5 << 16)

#define CONFIG_SLT

/* Link Definitions */

/** Code start address */
#define CONFIG_SYS_TEXT_BASE		0x00500000

/** Stack starting address */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0xffff0)

#define CONFIG_BOARD_LATE_INIT

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(0x1800000)	/* 24MHz */

/** Memory test starting address */
#define CONFIG_SYS_MEMTEST_START	MEM_BASE

/** Memory test end address */
#define CONFIG_SYS_MEMTEST_END		(MEM_BASE + PHYS_SDRAM_1_SIZE)

/* Size of malloc() pool */

/** Heap size for U-Boot */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 64 * 1024 * 1024)

/**
 * This is defined in the board config header.  This specified memory area
 * will get subtracted from the top (end) of memory and won't get
 * "touched" at all by U-Boot.  By fixing up gd->ram_size the Linux kernel
 * should get passed the now "corrected" memory size and won't touch it
 * either.
 */
#define CONFIG_SYS_MEM_TOP_HIDE		0x1000000

/* Generic Interrupt Controller Definitions */

/** Base address of Global Interrupt Controller GICD registers */
#define GICD_BASE			(0x801000000000)
#define GICR_BASE			(0x801000002000)

#define CONFIG_BAUDRATE			115200

/* Net */
/** Enable ThunderX BGX networking support */
#define CONFIG_THUNDERX_BGX

/** Enable ThunderX SMI MDIO driver */
#define CONFIG_THUNDERX_SMI

/** Enable ThunderX VNIC driver support */
#define CONFIG_THUNDERX_VNIC

/** Generate a random MAC address if it is not already defined */
#define CONFIG_RANDOM_MACADDR

#ifndef CONFIG_RANDOM_MACADDR
/** If no random MAC address, use this address */
# define CONFIG_ETHADDR			aa:d3:31:40:11:00
#endif

/**
 * Only allow the Ethernet MAC address environment variable to be
 * overwritten once.
 */
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/** Maximum number of BGX interfaces per CPU node */
#define CONFIG_MAX_BGX_PER_NODE		4

/** Maximum total number of BGX interfaces across all nodes */
#define CONFIG_MAX_BGX			4

/** Command line configuration */
#define CONFIG_MENU

/*#define CONFIG_MENU_SHOW*/
/** Enable icache and dcache commands */
#define CONFIG_CMD_CACHE

#define CONFIG_CMD_DIAG

/** Enable env command */
#define CONFIG_CMD_ENV
#undef  CONFIG_CMD_IMLS

#define CONFIG_CMD_MII

/** Enable tftp command */
#define CONFIG_CMD_TFTP

/** Enable pci command */
#define CONFIG_CMD_PCI

/** Enable askenv command */
#define CONFIG_CMD_ASKENV

/** Enable environment flags support */
#define CONFIG_CMD_ENV_FLAGS

/** Enable grepenv command */
#define CONFIG_CMD_GREPENV

/**
 * Enable callback support when environment variables are set, or
 * changed
 */
#define CONFIG_CMD_ENV_CALLBACK

#define CONFIG_ENV_CALLBACK_LIST_STATIC "txsmi\\d?mode:smimode"

/* AHCI support Definitions */
#define CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
/** Maximum number of SATA devices per controller*/
# define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
/** Enable 48-bit SATA addressing */
# define CONFIG_LBA48
/** Enable libata, required for SATA */
# define CONFIG_LIBATA
/** Enable 64-bit addressing */
# define CONFIG_SYS_64BIT_LBA
#endif

#define CONFIG_SYS_SATA_MAX_DEVICE 2

#define CONFIG_SYS_NVME_MAX_DEVICE 64

/* PCIe */
/** Show devices found on PCI bus */
#define CONFIG_PCI_SCAN_SHOW
#undef CONFIG_PCI_ENUM_ONLY

/** Number of I2C buses */
#define CONFIG_SYS_MAX_I2C_BUS	2

/** I2C slave address */
#define CONFIG_SYS_I2C_THUNDERX_SLAVE_ADDR	0x77

/** I2C bus real-time clock resides on */
#define CONFIG_SYS_RTC_BUS_NUM 0

/** I2C address of real-time clcok */
#define CONFIG_SYS_I2C_RTC_ADDR 0x68

/** Enable Dallas or compabile DS1337 driver */
#define CONFIG_RTC_DS1337

#define CONFIG_DDR_SPD
#define CONFIG_SYS_SPD_ADDR_LIST 	\
	{ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 }

#define CONFIG_SYS_SPD_I2C_BUS 1

/***** SPI Defines *********/
#ifdef CONFIG_DM_SPI_FLASH
#define CONFIG_SF_DEFAULT_SPEED 12500000
#define CONFIG_SF_DEFAULT_MODE	0
#define CONFIG_SF_DEFAULT_BUS	0
#define CONFIG_SF_DEFAULT_CS	0
#endif
/**************************/
#define CONFIG_CMD_SAVES

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_PXE

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(MEM_BASE)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			(MEM_BASE)	  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		(0x80000000-MEM_BASE)	/* 2048 MB */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/** Enable XHCI PCI driver */
#define CONFIG_USB_XHCI_PCI
/** Maximum root ports for XHCI */
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS 2

/** Define some USB drivers for ethernet to USB dongles to work */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_ASIX88179

/* PCIe network controller drivers */
/** Intel E1000 network card */
#define CONFIG_E1000
#define CONFIG_E1000_SPI
/** Enable command for E1000 card */
#define CONFIG_CMD_E1000

/* #define E1000_DEBUG */

/* Initial environment variables */
#define UBOOT_IMG_HEAD_SIZE		0x40
/* C80000 - 0x40 */

/** Extra environment settings */
#define CONFIG_EXTRA_ENV_SETTINGS	\
					"kernel_addr=0x40080000\0"	\
					"smi0mode=0.0.0\0"		\
					"smi1mode=0.0.0\0"		\
					"autoload=0\0"

#define CONFIG_BOOTARGS			\
					"console=ttyAMA0,115200n8 " \
					"earlycon=pl011,0x87e028000000 " \
					"debug maxcpus=24 rootwait rw "\
					"root=/dev/sda2 coherent_pool=16M"

/** Store U-Boot version in "ver" environment variable */
#define CONFIG_VERSION_VARIABLE

/** Where the environment is located */

/* Use ATF based calls for env variables */
/*#define CONFIG_ATF*/

/* Use uboot SPI APIs for env variables */
#define CONFIG_SPI_ENV

/* Use early board init for pci_init call */
#define CONFIG_BOARD_EARLY_INIT_R

#if defined(CONFIG_ATF)
#define CONFIG_ENV_IS_IN_ATF
#define CONFIG_SYS_ENV_ATF_NOR
#elif defined(CONFIG_SPI_ENV)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SPI_MAX_HZ		12500000
#define CONFIG_ENV_SPI_MODE		0
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#endif

/** Size of environment in bytes */
#define CONFIG_ENV_SIZE			0x8000
/** Starting offset of the environment */
#define CONFIG_ENV_OFFSET		0xf00000

/** Maximum size of the BDK flat device-tree */
#define CONFIG_BDK_FDT_SIZE		0x20000

#define CONFIG_THUNDERX_PROMPT_SIZE	32

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/** Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 CONFIG_THUNDERX_PROMPT_SIZE + 16)\

#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		getenv("prompt")

/** Enable long help support */
#define CONFIG_SYS_LONGHELP
/** Enable editing of the command line */
#define CONFIG_CMDLINE_EDITING
/** Enable tab autocomplete on command line */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_FEATURE_COMMAND_EDITING
#define CONFIG_SYS_MAXARGS		64		/** max command args */
#define CONFIG_NO_RELOCATION		1

/** System PLL reference clock */
#define PLL_REF_CLK			50000000	/* 50 MHz */

#define NS_PER_REF_CLK_TICK		(1000000000/PLL_REF_CLK)

#define CONFIG_OCTEON_MMC
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	8191
#define CONFIG_SYS_MMC_SET_DEV

#define CONFIG_CMD_TIME

/** Maximum size of image supported for bootm (and bootable FIT images) */
#define CONFIG_SYS_BOOTM_LEN		0x10000000	/* 256MiB */

#endif /* __THUNDERX_93XX_H__ */
