/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX_MMC_H__
#define __OCTEONTX_MMC_H__

#include <mmc.h>
#include <dm/device.h>

/* NOTE: this file is used by both Octeon and OcteonTX */

/*
 * Card Command Classes (CCC)
 */
#define CCC_BASIC		BIT(0)	/* (0) Basic protocol functions */
					/* (CMD0,1,2,3,4,7,9,10,12,13,15) */
					/* (and for SPI, CMD58,59) */
#define CCC_STREAM_READ		BIT(1)	/* (1) Stream read commands */
					/* (CMD11) */
#define CCC_BLOCK_READ		BIT(2)	/* (2) Block read commands */
					/* (CMD16,17,18) */
#define CCC_STREAM_WRITE	BIT(3)	/* (3) Stream write commands */
					/* (CMD20) */
#define CCC_BLOCK_WRITE		BIT(4)	/* (4) Block write commands */
					/* (CMD16,24,25,26,27) */
#define CCC_ERASE		BIT(5)	/* (5) Ability to erase blocks */
					/* (CMD32,33,34,35,36,37,38,39) */
#define CCC_WRITE_PROT		BIT(6)	/* (6) Able to write protect blocks */
					/* (CMD28,29,30) */
#define CCC_LOCK_CARD		BIT(7)	/* (7) Able to lock down card */
					/* (CMD16,CMD42) */
#define CCC_APP_SPEC		BIT(8)	/* (8) Application specific */
					/* (CMD55,56,57,ACMD*) */
#define CCC_IO_MODE		BIT(9)	/* (9) I/O mode */
					/* (CMD5,39,40,52,53) */
#define CCC_SWITCH		BIT(10)	/* (10) High speed switch */
					/* (CMD6,34,35,36,37,50) */
					/* (11) Reserved */
					/* (CMD?) */

/*
 * NOTE: This was copied from the Linux kernel.
 *
 * MMC status in R1, for native mode (SPI bits are different)
 * Type
 *	e:error bit
 *	s:status bit
 *	r:detected and set for the actual command response
 *	x:detected and set during command execution. the host must poll
 *	    the card by sending status command in order to read these bits.
 * Clear condition
 *	a:according to the card state
 *	b:always related to the previous command. Reception of
 *	    a valid command will clear it (with a delay of one command)
 *	c:clear by read
 */
#define R1_OUT_OF_RANGE		BIT(31)		/* er, c */
#define R1_ADDRESS_ERROR	BIT(30)		/* erx, c */
#define R1_BLOCK_LEN_ERROR	BIT(29)		/* er, c */
#define R1_ERASE_SEQ_ERROR	BIT(28)		/* er, c */
#define R1_ERASE_PARAM          BIT(27)		/* ex, c */
#define R1_WP_VIOLATION		BIT(26)		/* erx, c */
#define R1_CARD_IS_LOCKED	BIT(25)		/* sx, a */
#define R1_LOCK_UNLOCK_FAILED	BIT(24)		/* erx, c */
#define R1_COM_CRC_ERROR	BIT(23)		/* er, b */
/*#define R1_ILLEGAL_COMMAND	BIT(22)*/		/* er, b */
#define R1_CARD_ECC_FAILED	BIT(21)		/* ex, c */
#define R1_CC_ERROR		BIT(20)		/* erx, c */
#define R1_ERROR		BIT(19)		/* erx, c */
#define R1_UNDERRUN		BIT(18)		/* ex, c */
#define R1_OVERRUN		BIT(17)		/* ex, c */
#define R1_CID_CSD_OVERWRITE	BIT(16)		/* erx, c, CID/CSD overwrite */
#define R1_WP_ERASE_SKIP	BIT(15)		/* sx, c */
#define R1_CARD_ECC_DISABLED	BIT(14)		/* sx, a */
#define R1_ERASE_RESET		BIT(13)		/* sr, c */
#define R1_STATUS(x)		(x & 0xFFFFE000)
#define R1_CURRENT_STATE(x)	((x & 0x00001E00) >> 9) /* sx, b (4 bits) */
#define R1_READY_FOR_DATA	BIT(8)		/* sx, a */
#define R1_SWITCH_ERROR		BIT(7)		/* sx, c */
/*#define R1_APP_CMD		BIT(5)*/		/* sr, c */

#define R1_BLOCK_READ_MASK	R1_OUT_OF_RANGE |	\
				R1_ADDRESS_ERROR |	\
				R1_BLOCK_LEN_ERROR |	\
				R1_CARD_IS_LOCKED |	\
				R1_COM_CRC_ERROR |	\
				R1_ILLEGAL_COMMAND |	\
				R1_CARD_ECC_FAILED |	\
				R1_CC_ERROR |		\
				R1_ERROR
#define R1_BLOCK_WRITE_MASK	R1_OUT_OF_RANGE |	\
				R1_ADDRESS_ERROR |	\
				R1_BLOCK_LEN_ERROR |	\
				R1_WP_VIOLATION |	\
				R1_CARD_IS_LOCKED |	\
				R1_COM_CRC_ERROR |	\
				R1_ILLEGAL_COMMAND |	\
				R1_CARD_ECC_FAILED |	\
				R1_CC_ERROR |		\
				R1_ERROR |		\
				R1_UNDERRUN |		\
				R1_OVERRUN

/**
 * Flag indicates that CMD23 is supported.  This is required for multi-block
 * hardware transfers to work.
 */
#define OCTEON_MMC_FLAG_SD_CMD23	1

#ifdef __mips
# define OCTEON_EMM_BASE_ADDR		0x0x1180000000000
#endif

/** Maximum supported MMC slots */
#define OCTEONTX_MAX_MMC_SLOT		3

#define OCTEONTX_MMC_NAME_LEN		32

struct mmc;
struct mmc_config;
struct octeontx_mmc_host;
struct octeontx_mmc_slot;
struct gpio_desc;

struct octeontx_mmc_slot {
	char		name[OCTEONTX_MMC_NAME_LEN];	/** Name of device */
	u8		ext_csd[512];	/** Extended CSD register */
	struct mmc	*mmc;		/** Ptr back to mmc structure */
	struct octeontx_mmc_host *host;	/** Host controller data structure */
#if defined(CONFIG_ARCH_OCTEONTX2)
	u64		timing_tapps;	/** picoseconds per tap delay */
#endif
	struct mmc_config cfg;		/** Slot configuration */
	struct gpio_desc power_gpio;	/** Power/reset GPIO line (usually 8) */
	struct gpio_desc cd_gpio;	/** Card detect GPIO */
	struct gpio_desc wp_gpio;	/** Write-protect GPIO */
	int		power_delay;	/** Time in usec to wait for power */
	int		bus_id;		/** BUS ID of device */
	int		of_offset;	/** Device tree node */
	int		clk_period;	/** Clock period */
	int		bus_max_width;	/** Bus width 1, 4 or 8 */
	int		power_class;	/** Power class for device */
	int		flags;
	int		cmd_clk_skew;	/** Clock skew for cmd in SCLK */
	int		dat_clk_skew;	/** Clock skew for data in SCLK */
	int		power_gpio_of_offset;	/** Offset of power node */
	/**
	 * Register bus-width value where:
	 * 0: 1-bit
	 * 1: 4-bit
	 * 2: 8-bit
	 * 5: 4-bit DDR
	 * 6: 8-bit DDR
	 * All other values are reserved.
	 */
	u8		bus_width;
	bool		non_removable:1;/** True if device is not removable */
	/** True if have extended CSD register */
	bool		have_ext_csd:1;
	bool		sector_mode:1;	/** Sector or byte mode */
	bool		powered:1;	/** True if powered on */
	/** True if power GPIO is active high */
	bool		power_active_high:1;
	bool		ro_inverted:1;	/** True if write-protect is inverted */
	bool		cd_inverted:1;	/** True if card-detect is inverted */
	bool		disable_ddr:1;	/** True to disable DDR */
#if defined(CONFIG_ARCH_OCTEONTX2)
	bool		is_asim:1;	/** True if we're running in ASIM */
#endif
};

struct octeontx_mmc_host {
	void		*base_addr;	/** Base address of device */
	pci_dev_t	pdev;		/** PCI device */
	u64		sclock;		/** SCLK in hz */
	int		of_offset;	/** Device tree node */
	int		cur_slotid;	/** Current slot to use */
	int		last_slotid;	/** last slot in use */
	int		max_width;	/** Maximum width hardware supports */
#ifdef __mips
	int		node;		/** OCX node for Octeon (MIPS) */
	bool		use_ndf;	/** Use MIO_NDF_DMA or MIO_EMM_DMA. */
#endif
	bool		initialized;	/** True if initialized */
	struct udevice  *dev;		/** Device host is associated with */
	/** Slots associated with host controller */
	struct octeontx_mmc_slot slots[OCTEONTX_MAX_MMC_SLOT];
};

/**
 * Returns the card write protect status
 *
 * @param mmc	pointer to mmc data structure
 * @return 1 if card is write protected, 0 otherwise
 */
int octeontx_mmc_getwp(struct udevice *dev);

/**
 * Gets the card-detect status
 *
 * @param mmc	pointer to mmc data structure
 *
 * @return	1 if card is detected, false if not detected.
 */
int octeontx_mmc_getcd(struct udevice *dev);

#endif /* __OCTEONTX_MMC_H__ */
