/**
 * ThunderX SPI U_BOOT_DRIVER
 *
 * Copyright (C) 2016 Cavium, Inc.
 *
 * SPDX-LICENSE-IDENTIFIER:	GPL-2.0+
 *
 * SPI/MPI driver for Cavium ThunderX series of SoCs
 */
#include <common.h>
#include <spi.h>
#include <asm/io.h>
#include <malloc.h>
#include <dm.h>
#include <asm/arch/clock.h>
#include <asm/unaligned.h>
#include <watchdog.h>

#define THUNDERX_SPI_MAX_BYTES		9
#define THUNDERX_SPI_MAX_CLOCK_HZ	50000000

#define THUNDERX_SPI_NUM_CS		4

#define THUNDERX_SPI_CS_VALID(cs)	((cs) < THUNDERX_SPI_NUM_CS)

#define MPI_CFG				0x1000
#define MPI_STS				0x1008
#define MPI_TX				0x1010
#define MPI_WIDE_DAT			0x1040
#define MPI_DAT(X)			(0x1080 + ((X) << 3))

union mpi_cfg {
	uint64_t u;
	struct mpi_cfg_s {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
		uint64_t		:35;

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
		uint64_t 		:2;	/** Reserved */
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
		uint64_t		:2;
		uint64_t cshi		:1;
		uint64_t idleclks	:2;
		uint64_t tritx		:1;
		uint64_t cslate		:1;
		uint64_t csena0		:1;
		uint64_t csena1		:1;
		uint64_t csena2		:1;
		uint64_t csena3		:1;
		uint64_t clkdiv		:13;
		uint64_t 		:35;	/** Reserved */
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
		uint64_t reserved_13_63	:51;
		uint64_t rxnum		:5;	/** Number of bytes */
		uint64_t reserved_2_7	:6;
		uint64_t mpi_intr	:1;	/** Transaction done int */
		uint64_t busy		:1;	/** SPI engine busy */
#else /* Word 0 - Little Endian */
		uint64_t busy		:1;
		uint64_t mpi_intr	:1;
		uint64_t reserved_2_7	:6;
		uint64_t rxnum		:5;
		uint64_t reserved_13_63	:51;
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

/** Local driver data structure */
struct thunderx_spi {
	void *baseaddr;		/** Register base address */
	u32 clkdiv;		/** Clock divisor for device speed */
};

void *thunderx_spi_get_baseaddr(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct thunderx_spi *priv = dev_get_priv(bus);

	return priv->baseaddr;
}

static union mpi_cfg thunderx_spi_set_mpicfg(struct udevice *dev)
{
	struct dm_spi_slave_platdata *slave = dev_get_parent_platdata(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct thunderx_spi *priv = dev_get_priv(bus);
	union mpi_cfg mpi_cfg;
	uint max_speed = slave->max_hz;
	bool cpha, cpol;

	if (!max_speed)
		max_speed = 12500000;
	if (max_speed > THUNDERX_SPI_MAX_CLOCK_HZ)
		max_speed = THUNDERX_SPI_MAX_CLOCK_HZ;

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
static void thunderx_spi_wait_ready(struct udevice *dev)
{
	void *baseaddr = thunderx_spi_get_baseaddr(dev);
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
static int thunderx_spi_claim_bus(struct udevice *dev)
{
	void *baseaddr = thunderx_spi_get_baseaddr(dev);
	union mpi_cfg mpi_cfg;

	debug("%s(%s)\n", __func__, dev->name);
	if (!THUNDERX_SPI_CS_VALID(spi_chip_select(dev)))
		return -EINVAL;

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
static int thunderx_spi_release_bus(struct udevice *dev)
{
	void *baseaddr = thunderx_spi_get_baseaddr(dev);
	union mpi_cfg mpi_cfg;

	debug("%s(%s)\n", __func__, dev->name);
	if (!THUNDERX_SPI_CS_VALID(spi_chip_select(dev)))
		return -EINVAL;

	mpi_cfg.u = readq(baseaddr + MPI_CFG);
	mpi_cfg.s.enable = 0;
	writeq(mpi_cfg.u, baseaddr + MPI_CFG);

	return 0;
}

static int thunderx_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *dout, void *din, unsigned long flags)
{
	void *baseaddr = thunderx_spi_get_baseaddr(dev);
	union mpi_tx mpi_tx;
	union mpi_cfg mpi_cfg;
	uint64_t wide_dat = 0;
	int len = bitlen / 8;
	int i;
	const uint8_t *tx_data = dout;
	uint8_t *rx_data = din;
	int cs = spi_chip_select(dev);

	if (!THUNDERX_SPI_CS_VALID(cs))
		return -EINVAL;

	debug("%s(%s, %u, %p, %p, 0x%lx), cs: %d\n",
	      __func__, dev->name, bitlen, dout, din, flags, cs);

	mpi_cfg = thunderx_spi_set_mpicfg(dev);

	if (mpi_cfg.u != readq(baseaddr + MPI_CFG))
		writeq(mpi_cfg.u, baseaddr + MPI_CFG);

	/* Start by writing and reading 8 bytes at a time.  While we can support
	 * up to 10, it's easier to just use 8 with the MPI_WIDE_DAT register.
	 */
	while (len > 8) {
		if (tx_data) {
			wide_dat = get_unaligned((uint64_t *)tx_data);
			debug("  tx: %016llx\n", (unsigned long long)wide_dat);
			tx_data += 8;
			writeq(wide_dat, baseaddr + MPI_WIDE_DAT);
		}
		mpi_tx.u = 0;
		mpi_tx.s.csid = cs;
		mpi_tx.s.leavecs = 1;
		mpi_tx.s.txnum = tx_data ? 8 : 0;
		mpi_tx.s.totnum = 8;
		writeq(mpi_tx.u, baseaddr + MPI_TX);

		thunderx_spi_wait_ready(dev);

		if (rx_data) {
			wide_dat = readq(baseaddr + MPI_WIDE_DAT);
			debug("  rx: %016llx\n", (unsigned long long)wide_dat);
			*(uint64_t *)rx_data = wide_dat;
			rx_data += 8;
		}
		len -= 8;
	}

	/* Write and read the rest of the data */
	if (tx_data)
		for (i = 0; i < len; i++) {
			debug("  tx: %02x\n", *tx_data);
			writeq(*tx_data++, baseaddr + MPI_DAT(i));
		}

	mpi_tx.u = 0;
	mpi_tx.s.csid = cs;
	mpi_tx.s.leavecs = !(flags & SPI_XFER_END);
	mpi_tx.s.txnum = tx_data ? len : 0;
	mpi_tx.s.totnum = len;

	writeq(mpi_tx.u, baseaddr + MPI_TX);

	thunderx_spi_wait_ready(dev);

	if (rx_data) {
		for (i = 0; i < len; i++) {
			*rx_data = readq(baseaddr + MPI_DAT(i)) & 0xff;
			debug("  rx: %02x\n", *rx_data);
			rx_data++;
		}
	}

	return 0;
}

/**
 * Set the speed of the SPI bus
 *
 * @param	bus	bus to set
 * @param	max_hz	maximum speed supported
 */
static int thunderx_spi_set_speed(struct udevice *bus, uint max_hz)
{
	struct thunderx_spi *priv = dev_get_priv(bus);

	debug("%s(%s, %u, %llu)\n", __func__, bus->name, max_hz, thunderx_get_io_clock());
	if (max_hz > THUNDERX_SPI_MAX_CLOCK_HZ)
		max_hz = THUNDERX_SPI_MAX_CLOCK_HZ;
	priv->clkdiv = (thunderx_get_io_clock()) / (2 * max_hz);
	debug("%s %d\n", __func__, priv->clkdiv);

	return 0;
}

static int thunderx_spi_set_mode(struct udevice *bus, uint mode)
{
	/* We don't set it here */
	return 0;
}

static int thunderx_pci_spi_probe(struct udevice *dev)
{
	struct thunderx_spi *priv = dev_get_priv(dev);
	pci_dev_t bdf = dm_pci_get_bdf(dev);
	size_t size;

	debug("SPI PCI device: %x\n", bdf);
	dev->req_seq = PCI_FUNC(bdf);
	priv->baseaddr = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);

	debug("SPI bus %s %d at %p\n",dev->name, dev->seq, priv->baseaddr);

	return 0;
}

static const struct dm_spi_ops thunderx_spi_ops = {
	.claim_bus	= thunderx_spi_claim_bus,
	.release_bus	= thunderx_spi_release_bus,
	.xfer		= thunderx_spi_xfer,
	.set_speed	= thunderx_spi_set_speed,
	.set_mode	= thunderx_spi_set_mode,
};

static const struct udevice_id thunderx_spi_ids[] = {
	{ .compatible	= "cavium,thunder-8890-spi" },
	{ .compatible	= "cavium,thunder-8190-spi" },
	{ }
};

U_BOOT_DRIVER(thunderx_pci_spi) = {
	.name			= "spi_thunderx",
	.id			= UCLASS_SPI,
	.of_match 		= thunderx_spi_ids,
	.probe			= thunderx_pci_spi_probe,
	.priv_auto_alloc_size 	= sizeof(struct thunderx_spi),
	.ops			= &thunderx_spi_ops,
};

