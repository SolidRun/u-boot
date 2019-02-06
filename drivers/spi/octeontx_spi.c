/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <spi.h>
#include <asm/io.h>
#include <malloc.h>
#include <dm.h>
#include <asm/arch/clock.h>
#include <asm/unaligned.h>
#include <watchdog.h>

#if defined(CONFIG_ARCH_OCTEONTX2)
#include <asm/arch/octeontx2.h>
#define USE_TBI_CLK
#endif

#define OCTEONTX_SPI_MAX_BYTES		9
#define OCTEONTX_SPI_MAX_CLOCK_HZ	50000000

#define OCTEONTX2_TBI_CLK		100000000

#define OCTEONTX_SPI_NUM_CS		4

#define OCTEONTX_SPI_CS_VALID(cs)	((cs) < OCTEONTX_SPI_NUM_CS)

#define MPI_CFG				0x1000
#define MPI_STS				0x1008
#define MPI_TX				0x1010
#define MPI_XMIT			0x1018
#define MPI_WIDE_DAT			0x1040
#define MPI_IO_CTL			0x1048
#define MPI_DAT(X)			(0x1080 + ((X) << 3))
#define MPI_WIDE_BUF(X)			(0x1800 + ((X) << 3))
#define MPI_CYA_CFG			0x2000
#define MPI_CLKEN			0x2080

union mpi_cfg {
	uint64_t u;
	struct mpi_cfg_s {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		uint64_t		:14;
		uint64_t tb100_en	:1;
		uint64_t		:1;
		uint64_t cs_espi_en	:4;
		uint64_t		:8;
		uint64_t iomode		:2;
		uint64_t		:2;
		uint64_t legacy_dis	:1;
		uint64_t		:2;
		uint64_t clkdiv		:13;	/** clock divisor */
		uint64_t csena3		:1;	/** cs enable 3. */
		uint64_t csena2		:1;	/** cs enable 2 */
		uint64_t csena1		:1;	/** cs enable 1 */
		uint64_t csena0		:1;	/** cs enable 0 */
		/**
		 * 0 = SPI_CSn asserts 1/2 coprocessor-clock cycle before
		 *     transaction
		 * 1 = SPI_CSn asserts coincident with transaction
		 */
		uint64_t cslate		:1;
		/**
		 * Tristate TX.  Set to 1 to tristate SPI_DO when not
		 * transmitting.
		 */
		uint64_t tritx		:1;
		/**
		 * When set, guarantees idle coprocessor-clock cycles between
		 * commands.
		 */
		uint64_t idleclks	:2;
		/**
		 * SPI_CSn_L high.  1 = SPI_CSn_L is asserted high,
		 * 0 = SPI_CS_n asserted low.
		 */
		uint64_t cshi		:1;
		uint64_t rsvd		:1;	/** Reserved */
		uint64_t cs_sticky	:1;	/** cs sticky bit */
		/** 0 = shift MSB first, 1 = shift LSB first */
		uint64_t lsbfirst	:1;
		/**
		 * Wire-or DO and DI.
		 * 0 = SPI_DO and SPI_DI are separate wires (SPI).  SPI_DO pin
		 *     is always driven.
		 * 1 = SPI_DO/DI is all from SPI_DO pin (MPI).  SPI_DO pin is
		 *     tristated when not transmitting.  If WIREOR = 1, SPI_DI
		 *     pin is not used by the MPI/SPI engine.
		 */
		uint64_t wireor		:1;
		/**
		 * Clock control.
		 * 0 = Clock idles to value given by IDLELO after completion of
		 *     MPI/SPI transaction.
		 * 1 = Clock never idles, requires SPI_CSn_L
		 *     deassertion/assertion between commands.
		 */
		uint64_t clk_cont	:1;
		/**
		 * Clock idle low/clock invert
		 * 0 = SPI_CLK idles high, first transition is high-to-low.
		 *     This correspondes to SPI Block Guide options CPOL = 1,
		 *     CPHA = 0.
		 * 1 = SPI_CLK idles low, first transition is low-to-high.  This
		 *     corresponds to SPI Block Guide options CPOL = 0, CPHA = 0.
		 */
		uint64_t idlelo		:1;
		/** MPI/SPI enable, 0 = pins are tristated, 1 = pins driven */
		uint64_t enable		:1;
#else /* Word 0 - Little Endian */
		uint64_t enable		:1;
		uint64_t idlelo		:1;
		uint64_t clk_cont	:1;
		uint64_t wireor		:1;
		uint64_t lsbfirst	:1;
		uint64_t cs_sticky	:1;
		uint64_t rsvd		:1;
		uint64_t cshi		:1;
		uint64_t idleclks	:2;
		uint64_t tritx		:1;
		uint64_t cslate		:1;
		uint64_t csena0		:1;
		uint64_t csena1		:1;
		uint64_t csena2		:1;
		uint64_t csena3		:1;
		uint64_t clkdiv		:13;
		uint64_t		:2;
		uint64_t legacy_dis	:1;
		uint64_t		:2;
		uint64_t iomode		:2;
		uint64_t		:8;
		uint64_t cs_espi_en	:4;
		uint64_t		:1;
		uint64_t tb100_en	:1;
		uint64_t		:14;
#endif /* Word 0 - End */
	} s;
	/* struct mpi_cfg_s cn; */
};

/**
 * Register (NCB) mpi_dat#
 *
 * MPI/SPI Data Registers
 */
union mpi_dat {
	uint64_t u;
	struct mpi_datx_s {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		uint64_t reserved_8_63	:56;
		/**< [  7:  0](R/W/H) Data to transmit/receive. */
		uint64_t data		:8;
#else /* Word 0 - Little Endian */
		uint64_t data		:8;
		uint64_t reserved_8_63	:56;
#endif /* Word 0 - End */
	} s;
	/* struct mpi_datx_s cn; */
};

/**
 * Register (NCB) mpi_sts
 *
 * MPI/SPI STS Register
 */
union mpi_sts {
	uint64_t u;
	struct mpi_sts_s {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		uint64_t reserved_40_63	:24;
		uint64_t crc		:8;
		uint64_t		:5;
		uint64_t crc_err	:1;
		uint64_t		:6;
		uint64_t rxnum		:5;	/** Number of bytes */
		uint64_t reserved_2_7	:6;
		uint64_t mpi_intr	:1;	/** Transaction done int */
		uint64_t busy		:1;	/** SPI engine busy */
#else /* Word 0 - Little Endian */
		uint64_t busy		:1;
		uint64_t mpi_intr	:1;
		uint64_t reserved_2_7	:6;
		uint64_t rxnum		:5;
		uint64_t		:6;
		uint64_t crc_err	:1;
		uint64_t		:5;
		uint64_t crc		:8;
		uint64_t reserved_40_63	:24;
#endif /* Word 0 - End */
	} s;
	/* struct mpi_sts_s cn; */
};

/**
 * Register (NCB) mpi_tx
 *
 * MPI/SPI Transmit Register
 */
union mpi_tx {
	uint64_t u;
	struct mpi_tx_s {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		uint64_t		:42;	/* Reserved */
		uint64_t csid 		:2;	/** Which CS to assert */
		uint64_t		:3;	/* Reserved */
		uint64_t leavecs	:1;	/** Leave CSn asserted */
		uint64_t 		:3;	/* Reserved */
		uint64_t txnum		:5;	/** Number of words to tx */
		uint64_t		:3;	/* Reserved */
		uint64_t totnum		:5;	/** Total bytes to shift */
#else /* Word 0 - Little Endian */
		uint64_t totnum		:5;
		uint64_t 		:3;
		uint64_t txnum		:5;
		uint64_t		:3;
		uint64_t leavecs	:1;
		uint64_t		:3;
		uint64_t csid		:2;
		uint64_t		:42;
#endif /* Word 0 - End */
	} s;
	/* struct mpi_tx_s cn; */
};

#if defined(CONFIG_ARCH_OCTEONTX2)
/**
 * Register (NCB) mpi#_xmit
 *
 * MPI/SPI Transmit Register
 */
union mpi_xmit {
	u64 u;
	struct mpi_xmit_s {
		u64 totnum		: 11;
		u64 reserved_11_19	: 9;
		u64 txnum		: 11;
		u64 reserved_31_59	: 29;
		u64 leavecs		: 1;
		u64 csid		: 2;
		u64 reserved_63		: 1;
	} s;
};
#endif

/** Local driver data structure */
struct octeontx_spi {
	void *baseaddr;		/** Register base address */
	u32 clkdiv;		/** Clock divisor for device speed */
	bool is_otx2;		/** Gen 2 SoC */
};

static union mpi_cfg octeontx_spi_set_mpicfg(struct udevice *dev)
{
	struct dm_spi_slave_platdata *slave = dev_get_parent_platdata(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct octeontx_spi *priv = dev_get_priv(bus);
	union mpi_cfg mpi_cfg;
	uint max_speed = slave->max_hz;
	bool cpha, cpol;

	if (!max_speed)
		max_speed = 12500000;
	if (max_speed > OCTEONTX_SPI_MAX_CLOCK_HZ)
		max_speed = OCTEONTX_SPI_MAX_CLOCK_HZ;

	debug ("\n slave params %d %d %d \n", slave->cs,
		slave->max_hz, slave->mode);
	cpha = !!(slave->mode & SPI_CPHA);
	cpol = !!(slave->mode & SPI_CPOL);

	mpi_cfg.u = 0;
	mpi_cfg.s.clkdiv = priv->clkdiv & 0x1fff;
	mpi_cfg.s.cshi = !!(slave->mode & SPI_CS_HIGH);
	mpi_cfg.s.lsbfirst = !!(slave->mode & SPI_LSB_FIRST);
	mpi_cfg.s.wireor = !!(slave->mode & SPI_3WIRE);
	mpi_cfg.s.idlelo = cpha != cpol;
	mpi_cfg.s.cslate = cpha;
	mpi_cfg.s.enable = 1;
	mpi_cfg.s.csena0 = 1;
	mpi_cfg.s.csena1 = 1;
	mpi_cfg.s.csena2 = 1;
	mpi_cfg.s.csena3 = 1;

	debug("\n mpi_cfg %llx\n",mpi_cfg.u);
	return mpi_cfg;
}

/**
 * Wait until the SPI bus is ready
 *
 * @param	dev	SPI device to wait for
 */
static void octeontx_spi_wait_ready(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeontx_spi *priv = dev_get_priv(bus);
	void *baseaddr = priv->baseaddr;
	union mpi_sts mpi_sts;

	do {
		mpi_sts.u = readq(baseaddr + MPI_STS);
		WATCHDOG_RESET();
	} while (mpi_sts.s.busy);
	debug("%s(%s)\n", __func__, dev->name);
}
/**
 * Claim the bus for a slave device
 *
 * @param	dev	SPI bus
 *
 * @return	0 for success, -EINVAL if chip select is invalid
 */
static int octeontx_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeontx_spi *priv = dev_get_priv(bus);
	void *baseaddr = priv->baseaddr;
	union mpi_cfg mpi_cfg;

	debug("\n\n%s(%s)\n", __func__, dev->name);
	if (!OCTEONTX_SPI_CS_VALID(spi_chip_select(dev)))
		return -EINVAL;

#if defined(CONFIG_ARCH_OCTEONTX2)
	acquire_flash_arb(true);
#endif

	mpi_cfg.u = readq(baseaddr + MPI_CFG);
	mpi_cfg.s.tritx = 0;
	mpi_cfg.s.enable = 1;
	writeq(mpi_cfg.u, baseaddr + MPI_CFG);

	return 0;
}

/**
 * Release the bus to a slave device
 *
 * @param	dev	SPI bus
 *
 * @return	0 for success, -EINVAL if chip select is invalid
 */
static int octeontx_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeontx_spi *priv = dev_get_priv(bus);
	void *baseaddr = priv->baseaddr;
	union mpi_cfg mpi_cfg;

	debug("%s(%s)\n\n", __func__, dev->name);
	if (!OCTEONTX_SPI_CS_VALID(spi_chip_select(dev)))
		return -EINVAL;

#if defined(CONFIG_ARCH_OCTEONTX2)
	acquire_flash_arb(false);
#endif

	mpi_cfg.u = readq(baseaddr + MPI_CFG);
	mpi_cfg.s.enable = 0;
	writeq(mpi_cfg.u, baseaddr + MPI_CFG);

	return 0;
}

#if defined(CONFIG_ARCH_OCTEONTX)
static int octeontx_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeontx_spi *priv = dev_get_priv(bus);
	void *baseaddr = priv->baseaddr;
	union mpi_tx mpi_tx;
	union mpi_cfg mpi_cfg;
	uint64_t wide_dat = 0;
	int len = bitlen / 8;
	int i;
	const uint8_t *tx_data = dout;
	uint8_t *rx_data = din;
	int cs = spi_chip_select(dev);

	if (!OCTEONTX_SPI_CS_VALID(cs))
		return -EINVAL;

	debug("\n %s(%s, %u, %p, %p, 0x%lx), cs: %d\n",
	      __func__, dev->name, bitlen, dout, din, flags, cs);

	mpi_cfg = octeontx_spi_set_mpicfg(dev);

	if (mpi_cfg.u != readq(baseaddr + MPI_CFG))
		writeq(mpi_cfg.u, baseaddr + MPI_CFG);

	debug("\n mpi_cfg upd %llx\n", mpi_cfg.u);

	/* Start by writing and reading 8 bytes at a time.  While we can support
	 * up to 10, it's easier to just use 8 with the MPI_WIDE_DAT register.
	 */
	while (len > 8) {
		if (tx_data) {
			wide_dat = get_unaligned((uint64_t *)tx_data);
			debug("  tx: %016llx \t", (unsigned long long)wide_dat);
			tx_data += 8;
			writeq(wide_dat, baseaddr + MPI_WIDE_DAT);
		}

		mpi_tx.u = 0;
		mpi_tx.s.csid = cs;
		mpi_tx.s.leavecs = 1;
		mpi_tx.s.txnum = tx_data ? 8 : 0;
		mpi_tx.s.totnum = 8;
		writeq(mpi_tx.u, baseaddr + MPI_TX);

		octeontx_spi_wait_ready(dev);

		debug("\n ");

		if (rx_data) {
			wide_dat = readq(baseaddr + MPI_WIDE_DAT);
			debug("  rx: %016llx\t", (unsigned long long)wide_dat);
			*(uint64_t *)rx_data = wide_dat;
			rx_data += 8;
		}
		len -= 8;
	}

	debug("\n ");

	/* Write and read the rest of the data */
	if (tx_data) {
		for (i = 0; i < len; i++) {
			debug("  tx: %02x\n", *tx_data);
			writeq(*tx_data++, baseaddr + MPI_DAT(i));
		}
	}
	mpi_tx.u = 0;
	mpi_tx.s.csid = cs;
	mpi_tx.s.leavecs = !(flags & SPI_XFER_END);
	mpi_tx.s.txnum = tx_data ? len : 0;
	mpi_tx.s.totnum = len;
	writeq(mpi_tx.u, baseaddr + MPI_TX);

	octeontx_spi_wait_ready(dev);

	debug("\n ");

	if (rx_data) {
		for (i = 0; i < len; i++) {
			*rx_data = readq(baseaddr + MPI_DAT(i)) & 0xff;
			debug("  rx: %02x\n", *rx_data);
			rx_data++;
		}
	}

	return 0;
}
#endif

#if defined(CONFIG_ARCH_OCTEONTX2)
static int octeontx_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeontx_spi *priv = dev_get_priv(bus);
	void *baseaddr = priv->baseaddr;
	union mpi_xmit mpi_xmit;
	union mpi_cfg mpi_cfg;
	u64 wide_dat = 0;
	int len = bitlen / 8;
	int rem;
	int i;
	const u8 *tx_data = dout;
	u8 *rx_data = din;
	int cs = spi_chip_select(dev);

	if (!OCTEONTX_SPI_CS_VALID(cs))
		return -EINVAL;

	debug("\n %s(%s, %u, %p, %p, 0x%lx), cs: %d\n",
	      __func__, dev->name, bitlen, dout, din, flags, cs);

	mpi_cfg = octeontx_spi_set_mpicfg(dev);

	mpi_cfg.s.legacy_dis = 1;
	mpi_cfg.s.cs_sticky = 1;
#ifdef USE_TBI_CLK
	mpi_cfg.s.tb100_en = 1;
#endif
	mpi_cfg.s.iomode = 0;
	if (flags & SPI_XFER_DUAL)
		mpi_cfg.s.iomode = 2;
	if (flags & SPI_XFER_QUAD)
		mpi_cfg.s.iomode = 3;

	if (mpi_cfg.u != readq(baseaddr + MPI_CFG))
		writeq(mpi_cfg.u, baseaddr + MPI_CFG);

	debug("\n mpi_cfg upd %llx\n\n", mpi_cfg.u);

	/* Start by writing or reading 1024 bytes at a time. */
	while (len > 1024) {
		if (tx_data) {
			/* 8 bytes per iteration */
			for (i = 0; i < 128; i++) {
				wide_dat = get_unaligned((uint64_t *)tx_data);
				debug("  tx: %016llx \t",
				      (unsigned long long)wide_dat);
				if ((i % 4) == 3)
					debug("\n");
				tx_data += 8;
				writeq(wide_dat, baseaddr + MPI_WIDE_BUF(i));
			}
		}

		mpi_xmit.u = 0;
		mpi_xmit.s.csid = cs;
		mpi_xmit.s.leavecs = 1;
		mpi_xmit.s.txnum = tx_data ? 1024 : 0;
		mpi_xmit.s.totnum = 1024;
		writeq(mpi_xmit.u, baseaddr + MPI_XMIT);

		octeontx_spi_wait_ready(dev);

		debug("\n ");

		if (rx_data) {
			/* 8 bytes per iteration */
			for (i = 0; i < 128; i++) {
				wide_dat = readq(baseaddr + MPI_WIDE_BUF(i));
				debug("  rx: %016llx\t",
				      (unsigned long long)wide_dat);
				if ((i % 4) == 3)
					debug("\n");
				*(uint64_t *)rx_data = wide_dat;
				rx_data += 8;
			}
		}
		len -= 1024;
	}

	if (tx_data) {
		rem = len % 8;
		/* 8 bytes per iteration */
		for (i = 0; i < len / 8; i++) {
			wide_dat = get_unaligned((uint64_t *)tx_data);
			debug("  tx: %016llx \t",
			      (unsigned long long)wide_dat);
			if ((i % 4) == 3)
				debug("\n");
			tx_data += 8;
			writeq(wide_dat, baseaddr + MPI_WIDE_BUF(i));
		}
		if (rem) {
			memcpy(&wide_dat, tx_data, rem);
			debug("  rtx: %016llx\t", wide_dat);
			writeq(wide_dat, baseaddr + MPI_WIDE_BUF(i));
		}
	}

	mpi_xmit.u = 0;
	mpi_xmit.s.csid = cs;
	mpi_xmit.s.leavecs = !(flags & SPI_XFER_END);
	mpi_xmit.s.txnum = tx_data ? len : 0;
	mpi_xmit.s.totnum = len;
	writeq(mpi_xmit.u, baseaddr + MPI_XMIT);

	octeontx_spi_wait_ready(dev);

	debug("\n ");

	if (rx_data) {
		rem = len % 8;
		/* 8 bytes per iteration */
		for (i = 0; i < len / 8; i++) {
			wide_dat = readq(baseaddr + MPI_WIDE_BUF(i));
			debug("  rx: %016llx\t",
			      (unsigned long long)wide_dat);
			if ((i % 4) == 3)
				debug("\n");
			*(uint64_t *)rx_data = wide_dat;
			rx_data += 8;
		}
		if (rem) {
			wide_dat = readq(baseaddr + MPI_WIDE_BUF(i));
			debug("  rrx: %016llx\t",
			      (unsigned long long)wide_dat);
			memcpy(rx_data, &wide_dat, rem);
			rx_data += rem;
		}
	}

	return 0;
}
#endif

/**
 * Set the speed of the SPI bus
 *
 * @param	bus	bus to set
 * @param	max_hz	maximum speed supported
 */
static int octeontx_spi_set_speed(struct udevice *bus, uint max_hz)
{
	struct octeontx_spi *priv = dev_get_priv(bus);
	u64 refclk = octeontx_get_io_clock();
	u32 calc_hz;

	debug("%s(%s, %u, %llu)\n", __func__, bus->name, max_hz, refclk);

	if (max_hz > OCTEONTX_SPI_MAX_CLOCK_HZ)
		max_hz = OCTEONTX_SPI_MAX_CLOCK_HZ;

#ifdef USE_TBI_CLK
	refclk = OCTEONTX2_TBI_CLK;
#endif
	priv->clkdiv = refclk / (2 * max_hz);
	while (1) {
		calc_hz = refclk / (2 * priv->clkdiv);
		if (calc_hz <= max_hz)
			break;
		priv->clkdiv += 1;
	}
	if (priv->clkdiv > 8191)
		return -1;

	debug("%s %d\n", __func__, priv->clkdiv);

	return 0;
}

static int octeontx_spi_set_mode(struct udevice *bus, uint mode)
{
	/* We don't set it here */
	return 0;
}

static int octeontx_pci_spi_probe(struct udevice *dev)
{
	struct octeontx_spi *priv = dev_get_priv(dev);
	pci_dev_t bdf = dm_pci_get_bdf(dev);
	size_t size;

	debug("SPI PCI device: %x\n", bdf);
	dev->req_seq = PCI_FUNC(bdf);
	priv->baseaddr = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);

	debug("SPI bus %s %d at %p\n",dev->name, dev->seq, priv->baseaddr);

	return 0;
}

static const struct dm_spi_ops octeontx_spi_ops = {
	.claim_bus	= octeontx_spi_claim_bus,
	.release_bus	= octeontx_spi_release_bus,
	.xfer		= octeontx_spi_xfer,
	.set_speed	= octeontx_spi_set_speed,
	.set_mode	= octeontx_spi_set_mode,
};

static const struct udevice_id octeontx_spi_ids[] = {
	{ .compatible	= "cavium,thunder-8890-spi" },
	{ .compatible	= "cavium,thunder-8190-spi" },
	{ }
};

U_BOOT_DRIVER(octeontx_pci_spi) = {
	.name			= "spi_octeontx",
	.id			= UCLASS_SPI,
	.of_match 		= octeontx_spi_ids,
	.probe			= octeontx_pci_spi_probe,
	.priv_auto_alloc_size 	= sizeof(struct octeontx_spi),
	.ops			= &octeontx_spi_ops,
};

