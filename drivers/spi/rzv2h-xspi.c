// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar Gen3 RPC QSPI driver
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/of_access.h>
#include <dt-structs.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/errno.h>
#include <spi.h>
#include <wait_bit.h>

/* xSPI Wrapper Configuration Register */
#define XSPI_WRAPCFG		0x0000
#define XSPI_WRAPCFG_DSSFTCS0(val)	(((val) & 0x1f) << 8)
#define XSPI_WRAPCFG_DSSFTCS1(val)	(((val) & 0x1f) << 24)

/* xSPI Common Configuration Register */
#define XSPI_COMCFG		0x0004
#define XSPI_COMCFG_OEASTEX	BIT(16)
#define XSPI_COMCFG_OENEGEX	BIT(17)
#define XSPI_COMCFG_ARBMD(val)  (((val) & 0x03) << 0)

/* xSPI Bridge Configuration Register */
#define XSPI_BMCFG		0x0008
#define XSPI_BMCFG_WRMD		BIT(0)
#define XSPI_BMCFG_MWRCOMB	BIT(7)
#define XSPI_BMCFG_MWRSIZE(val)	(((val) & 0xff) << 8)
#define XSPI_BMCFG_PREEN	BIT(16)
#define XSPI_BMCFG_CMBTIM(val)	(((val) & 0xff) << 24)

/* xSPI Command Map Configuration Register 0 CS(0/1) */
#define XSPI_CMCFG0CS0		0x0010
#define XSPI_CMCFG0CS1		0x0020
#define XSPI_CMCFG2CS(val)	      (0x0018 + 0x0010 * (val))
#define XSPI_CMCFG0_FFMT(val)		(((val) & 0x03) << 0)
#define XSPI_CMCFG0_ADDSIZE(val)	(((val) & 0x03) << 2)
#define XSPI_CMCFG0_WPBSTMD	BIT(4)
#define XSPI_CMCFG0_ARYAMD	BIT(5)
#define XSPI_CMCFG0_ADDRPEN(val)	(((val) & 0xff) << 16)
#define XSPI_CMCFG0_ADDRPCD(val)	(((val) & 0xff) << 24)

/* xSPI Command Map Configuration Register 1 CS(0/1) */
#define XSPI_CMCFG1CS0		0x0014
#define XSPI_CMCFG1CS1		0x0024
#define XSPI_CMCFG1_RDCMD(val)	(((val) & 0xffff) << 0)
#define XSPI_CMCFG1_RDCMD_UPPER_BYTE(val)	(((val) & 0xff) << 8)
#define XSPI_CMCFG1_RDLATE(val)	(((val) & 0x1f) << 16)

/* xSPI Command Map Configuration Register 2 CS(0/1) */
#define XSPI_CMCFG2CS0		0x0018
#define XSPI_CMCFG2CS1		0x0028
#define XSPI_CMCFG2_WRCMD(val)	(((val) & 0xffff) << 0)
#define XSPI_CMCFG2_WRCMD_UPPER(val)	(((val) & 0xff) << 8)
#define XSPI_CMCFG2_WRLATE(val)	(((val) & 0x1f) << 16)

/* xSPI Link I/O Configuration Register CS(0/1) */
#define XSPI_LIOCFGCS0		0x0050
#define XSPI_LIOCFGCS1		0x0054
#define XSPI_LIOCFG_PRTMD(val)	(((val) & 0x3ff) << 0)
#define XSPI_LIOCFG_LATEMV	BIT(10)
#define XSPI_LIOCFG_WRMSKMD	BIT(11)
#define XSPI_LIOCFG_CSMIN(val)	(((val) & 0x0f) << 16)
#define XSPI_LIOCFG_CSASTEX	BIT(20)
#define XSPI_LIOCFG_CSNEGEX	BIT(21)
#define XSPI_LIOCFG_SDRDRV	BIT(22)
#define XSPI_LIOCFG_SDRSMPMD	BIT(23)
#define XSPI_LIOCFG_SDRSMPSFT(val)	(((val) & 0x0f) << 24)
#define XSPI_LIOCFG_DDRSMPEX(val)	(((val) & 0x0f) << 28)

/* xSPI Bridge Map Control Register 0 */
#define XSPI_BMCTL0		0x0060
#define XSPI_BMCTL0_CS0ACC(val)	(((val) & 0x03) << 0)
#define XSPI_BMCTL0_CS1ACC(val)	(((val) & 0x03) << 2)

/* xSPI Bridge Map Control Register 1 */
#define XSPI_BMCTL1		0x0064
#define XSPI_BMCTL1_MWRPUSH	BIT(8)
#define XSPI_BMCTL1_PBUFCLR	BIT(10)

/* xSPI Command Manual Control Register 0 */
#define XSPI_CDCTL0		0x0070
#define XSPI_CDCTL0_TRREQ	BIT(0)
#define XSPI_CDCTL0_PERMD	BIT(1)
#define XSPI_CDCTL0_CSSEL	BIT(3)
#define XSPI_CDCTL0_TRNUM(val)	(((val) & 0x03) << 4)
#define XSPI_CDCTL0_PERITV(val)	(((val) & 0x1f) << 16)
#define XSPI_CDCTL0_PERREP(val)	(((val) & 0x0f) << 24)

/* xSPI Command Manual Control Register 1 */
#define XSPI_CDCTL1		0x0074

/* xSPI Command Manual Control Register 2 */
#define XSPI_CDCTL2		0x0078

/* xSPI Command Manual Type Buf 0/1/2/3 */
#define XSPI_CDTBUF0		0x0080
#define XSPI_CDTBUF1		0x0090
#define XSPI_CDTBUF2		0x00A0
#define XSPI_CDTBUF3		0x00B0
#define XSPI_CDTBUF_CMDSIZE(val)	(((val) & 0x03) << 0)
#define XSPI_CDTBUF_ADDSIZE(val)	(((val) & 0x07) << 2)
#define XSPI_CDTBUF_DATASIZE(val)	(((val) & 0x0f) << 5)
#define XSPI_CDTBUF_LATE(val)		(((val) & 0x1f) << 9)
#define XSPI_CDTBUF_TRTYPE	BIT(15)
#define XSPI_CDTBUF_CMD(val)		(((val) & 0xffff) << 16)
#define XSPI_CDTBUF_CMD_FIELD(val)	(((val) & 0xff) << 24)
#define XSPI_CDTBUF_EXCMD_FIELD(val)	(((val) & 0xff) << 16)

/* xSPI Command Manual Address Buff 0/1/2/3 */
#define XSPI_CDABUF0		0x0084
#define XSPI_CDABUF1		0x0094
#define XSPI_CDABUF2		0x00A4
#define XSPI_CDABUF3		0x00B4

/* xSPI Command Manual Data 0/1 Buf 0/1/2/3 */
#define XSPI_CDD0BUF0		0x0088
#define XSPI_CDD1BUF0		0x008c
#define XSPI_CDD0BUF1		0x0098
#define XSPI_CDD0BUF2		0x00A8
#define XSPI_CDD0BUF3		0x00B8

/* xSPI Command Calibration Control Register 0 CS(0/1) */
#define XSPI_CCCTL0CS0		0x0130
#define XSPI_CCCTL0CS1		0x0150
#define XSPI_CCCTL0_CAEN	BIT(0)
#define XSPI_CCCTL0_CANOWR	BIT(1)
#define XSPI_CCCTL0_CAITV(val)		(((val) & 0x1f) << 8)
#define XSPI_CCCTL0_CASFTSTA(val)	(((val) & 0x1f) << 16)
#define XSPI_CCCTL0_CASFTEND(val)	(((val) & 0x1f) << 24)

/* xSPI Command Calibration Control Register 1 CS(0/1) */
#define XSPI_CCCTL1CS0		0x0134
#define XSPI_CCCTL1CS1		0x0154
#define XSPI_CCCTL1_CACMDSIZE(val)	(((val) & 0x03) << 0)
#define XSPI_CCCTL1_CAADDSIZE(val)	(((val) & 0x07) << 2)
#define XSPI_CCCTL1_CADARASIZE(val)	(((val) & 0x0f) << 5)
#define XSPI_CCCTL1_CAWRLATE(val)	(((val) & 0x1f) << 16)
#define XSPI_CCCTL1_CARDLATE(val)	(((val) & 0x1f) << 24)

/* xSPI Common Status Register */
#define XSPI_COMSTT		0x0184
#define XSPI_COMSTT_MEMACC	BIT(0)
#define XSPI_COMSTT_PBUFNE	BIT(4)
#define XSPI_COMSTT_WRBUFNE	BIT(6)

/* xSPI Interrupt Status Register */
#define XSPI_INTS		0x0190
#define XSPI_INTS_CMDCMP	BIT(0)
#define XSPI_INTS_PATCPM	BIT(1)
#define XSPI_INTS_INICPM	BIT(2)
#define XSPI_INTS_PERTO		BIT(3)
#define XSPI_INTS_DSTOCS0	BIT(4)
#define XSPI_INTS_DSTOCS1	BIT(5)

/* xSPI Interrupt Clear Register */
#define XSPI_INTC		0x0194
#define XSPI_INTC_CMDCMPC	BIT(0)
#define XSPI_INTC_CPATCMPC	BIT(1)
#define XSPI_INTC_INICMPC	BIT(2)
#define XSPI_INTC_PERTOC	BIT(3)
#define XSPI_INTC_DSTOCS0C	BIT(4)
#define XSPI_INTC_DSTOCS1C	BIT(5)

/* xSPI Interrupt Enable Register */
#define XSPI_INTE		0x0198
#define XSPI_INTE_CMDCMPE	BIT(0)
#define XSPI_INTE_PATCMPE	BIT(1)
#define XSPI_INTE_INICMPE	BIT(2)
#define XSPI_INTE_PERTOE	BIT(3)
#define XSPI_INTE_DSTOCS0E	BIT(4)
#define XSPI_INTE_DSTOCS1E	BIT(5)

/* Maximum data size of MWRSIZE*/
#define MWRSIZE_MAX		64

/* Initial value */
#define XSPI_CCCTL0CS0_INITIAL  0x1F000000

/* xSPI Protocol mode */
#define PROTO_1S_1S_1S		0x000
#define PROTO_1S_4S_4S		0x090
#define PROTO_4S_4S_4S		0x092

DECLARE_GLOBAL_DATA_PTR;

struct xspi_plat {
	fdt_addr_t	regs;
	fdt_addr_t	extr;
	s32		freq;   /* Default clock freq, -1 for none */
};

struct xspi_priv {
	fdt_addr_t	regs;
	fdt_addr_t	extr;
	struct clk	clk;

	u8		cmdcopy[8];
	u32		cmdlen;
	bool		cmdstarted;
};

static int xspi_wait_cmdcmp(struct udevice *dev)
{
	struct xspi_priv *priv = dev_get_priv(dev->parent);

	return wait_for_bit_le32((void *)priv->regs + XSPI_INTS, XSPI_INTS_CMDCMP,
				true, 1000, false);
}

static int xspi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct xspi_priv *priv = dev_get_priv(bus);

	writel(XSPI_LIOCFG_PRTMD(PROTO_1S_1S_1S) | XSPI_LIOCFG_CSMIN(0)
		| XSPI_LIOCFG_CSASTEX | XSPI_LIOCFG_CSNEGEX,
					priv->regs + XSPI_LIOCFGCS0);

	writel(XSPI_CCCTL0CS0_INITIAL, priv->regs + XSPI_CCCTL0CS0);
	writel(0, priv->regs + XSPI_CDCTL0);

	return 0;
}

static int xspi_release_bus(struct udevice *dev)
{
	/* This is a SPI NOR controller, do nothing. */
	return 0;
}

static int xspi_xfer(struct udevice *dev, unsigned int bitlen,
			const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct xspi_priv *priv = dev_get_priv(bus);
	u32 wlen = dout ? (bitlen / 8) : 0;
	u32 rlen = din ? (bitlen / 8) : 0;
	u32 bmcfg, offset, cdtbuf, cdctl0, cmcfg0cs0, cmcfg1cs0;
	int ret = 0;

	if (!priv->cmdstarted) {
		if (!wlen || rlen)
			BUG();
		memcpy(priv->cmdcopy, dout, wlen);
		priv->cmdlen = wlen;

		/* Command transfer start */
		priv->cmdstarted = true;
		if (!(flags & SPI_XFER_END))
			return 0;
	}

	offset = (priv->cmdcopy[1] << 16) | (priv->cmdcopy[2] << 8) |
		(priv->cmdcopy[3] << 0);
	cdtbuf = 0;
	cdctl0 = 0;
	cmcfg0cs0 = 0;
	bmcfg = 0;
	cmcfg1cs0 = 0;

	xspi_claim_bus(dev);

	if (wlen || (!rlen && !wlen) || flags == SPI_XFER_ONCE) {
		if (wlen && flags == SPI_XFER_END)
			cdtbuf |= XSPI_CDTBUF_DATASIZE(0x4);

		cdtbuf |= XSPI_CDTBUF_TRTYPE;

		if (priv->cmdlen >= 1) {	/* Command(1) */
			cdtbuf |= XSPI_CDTBUF_CMD_FIELD(priv->cmdcopy[0])
					| XSPI_CDTBUF_CMDSIZE(0x1);
		} else {
			cdtbuf = 0;
		}

		writel(0, priv->regs + XSPI_CDABUF0);

		if (priv->cmdlen >= 4) {	/* Address(3) */
			writel(offset, priv->regs + XSPI_CDABUF0);
			cdtbuf |= XSPI_CDTBUF_ADDSIZE(0x3);
		} else {
			writel(0, priv->regs + XSPI_CDABUF0);
			cdtbuf |= XSPI_CDTBUF_ADDSIZE(0);
		}
		if (priv->cmdlen >= 5)		/* Dummy(n) */
			cdtbuf |= XSPI_CDTBUF_LATE(8 * (priv->cmdlen - 4));
		else
			cdtbuf |= XSPI_CDTBUF_LATE(0);

		writel(XSPI_LIOCFG_PRTMD(PROTO_1S_1S_1S), priv->regs + XSPI_LIOCFGCS0);

		if (wlen && flags == SPI_XFER_END) {
			u32 *datout = (u32 *)dout;
			u32 data;
			u32 nbytes = 0;

			cdctl0 |= XSPI_CDCTL0_TRREQ;
			cdtbuf &= ~XSPI_CDTBUF_DATASIZE(0xf);
			switch (wlen) {
			case 1:
				cdtbuf |= XSPI_CDTBUF_DATASIZE(0x1);
				data = (*datout & 0x0ff);
				nbytes = 1;
			break;
			case 2:
				cdtbuf |= XSPI_CDTBUF_DATASIZE(0x2);
				data = (*datout & 0x0ffff);
				nbytes = 2;
			break;
			case 3:
				cdtbuf |= XSPI_CDTBUF_DATASIZE(0x3);
				data = (*datout & 0x0ffffff);
				nbytes = 3;
			break;
			default:
				cdtbuf |= XSPI_CDTBUF_DATASIZE(0x4);
				data = *datout;
				nbytes = 4;
			break;
			}

			writel(cdtbuf, priv->regs + XSPI_CDTBUF0);
			writel(data, priv->regs + XSPI_CDD0BUF0);
			writel(cdctl0, priv->regs + XSPI_CDCTL0);

			ret = xspi_wait_cmdcmp(dev);
			if (ret)
				goto err;
			ret = nbytes;
			writel(XSPI_INTC_CMDCMPC, priv->regs + XSPI_INTC);

		} else {
			writel(cdtbuf, priv->regs + XSPI_CDTBUF0);
			writel(0, priv->regs + XSPI_CDD0BUF0);
			writel(XSPI_CDCTL0_TRREQ, priv->regs + XSPI_CDCTL0);
			ret = xspi_wait_cmdcmp(dev);
			if (ret)
				goto err;
			writel(XSPI_INTC_CMDCMPC, priv->regs + XSPI_INTC);
		}
	} else {
		/* Read ID address of spi-flash using manual mode */
		if (priv->cmdlen < 2) {
			u8 *dinout = (u8 *)din;

			writel(0, priv->regs + XSPI_CDD0BUF0);
			writel(0, priv->regs + XSPI_CDD1BUF0);

			cdtbuf |= XSPI_CDTBUF_CMD_FIELD(priv->cmdcopy[0])
				| XSPI_CDTBUF_DATASIZE(0x6) | XSPI_CDTBUF_CMDSIZE(0x1);
			writel(cdtbuf, priv->regs + XSPI_CDTBUF0);
			writel(XSPI_CDCTL0_TRREQ, priv->regs + XSPI_CDCTL0);
			if (rlen <= 4) {
				ret = xspi_wait_cmdcmp(dev);
				if (ret)
					goto err;
				memcpy_fromio(dinout, (void *)(priv->regs + XSPI_CDD0BUF0), rlen);
				dinout = dinout + rlen;
				writel(XSPI_INTC_CMDCMPC, priv->regs + XSPI_INTC);
			} else {
				ret = xspi_wait_cmdcmp(dev);
				if (ret)
					goto err;
				memcpy_fromio(dinout, (void *)(priv->regs + XSPI_CDD0BUF0), 4);
				dinout = dinout + 4;
				memcpy_fromio(dinout, (void *)(priv->regs + XSPI_CDD1BUF0),
						rlen - 4);
				writel(XSPI_INTC_CMDCMPC, priv->regs + XSPI_INTC);
			}
		} else {
			/* Read data of spi-flash using memory maping mode */
			writel(XSPI_CCCTL1_CAADDSIZE(0), priv->regs + XSPI_CCCTL1CS0);
			writel(XSPI_LIOCFG_PRTMD(PROTO_1S_1S_1S)
				| XSPI_LIOCFG_CSMIN(0), priv->regs + XSPI_LIOCFGCS0);
			cmcfg0cs0 |= XSPI_CMCFG0_FFMT(0) | XSPI_CMCFG0_WPBSTMD;

			if (priv->cmdlen >= 1)		/* Command(1) */
				cmcfg1cs0 |= XSPI_CMCFG1_RDCMD_UPPER_BYTE(priv->cmdcopy[0]);
			else
				cmcfg1cs0 = 0;

			if (priv->cmdlen >= 4)		/* Address(3) */
				cmcfg0cs0 |= XSPI_CMCFG0_ADDSIZE(0x2);

			if (priv->cmdlen >= 5) {	/* Dummy(n) */
				cmcfg1cs0 |= XSPI_CMCFG1_RDLATE(8 * (priv->cmdlen - 4));
			} else {
				cmcfg1cs0 |= XSPI_CMCFG1_RDLATE(0);
			}

			bmcfg |= 0 | XSPI_BMCFG_MWRCOMB | XSPI_BMCFG_MWRSIZE(0x0f)
							| XSPI_BMCFG_PREEN;
			writel(bmcfg, priv->regs + XSPI_BMCFG);

			writel(cmcfg0cs0, priv->regs + XSPI_CMCFG0CS0);
			writel(cmcfg1cs0, priv->regs + XSPI_CMCFG1CS0);

			writel(XSPI_BMCTL0_CS0ACC(0x01), priv->regs + XSPI_BMCTL0);

			if (rlen)
				memcpy_fromio(din, (void *)(priv->extr + offset), rlen);
			else
				readl(priv->extr);     /* Dummy read */
		}
	}

err:
	priv->cmdstarted = false;
	return ret;
}

static int xspi_set_speed(struct udevice *bus, uint speed)
{
	/* This is a SPI NOR controller, do nothing. */
	return 0;
}

static int xspi_set_mode(struct udevice *bus, uint mode)
{
	/* This is a SPI NOR controller, do nothing. */
	return 0;
}

static int xspi_bind(struct udevice *parent)
{
	const void *fdt = gd->fdt_blob;
	ofnode node;
	int ret, off;
	/*
	 * Check if there are any SPI NOR child nodes, if so, bind as
	 * this controller will be operated in SPI mode.
	 */
	dev_for_each_subnode(node, parent) {
		off = ofnode_to_offset(node);
		ret = fdt_node_check_compatible(fdt, off, "spi-flash");
		if (!ret)
			return 0;
		ret = fdt_node_check_compatible(fdt, off, "jedec,spi-nor");
		if (!ret)
			return 0;
	}

	return -ENODEV;
}

static int xspi_probe(struct udevice *dev)
{
	struct xspi_plat *plat = dev_get_plat(dev);
	struct xspi_priv *priv = dev_get_priv(dev);

	priv->regs = plat->regs;
	priv->extr = plat->extr;
#if CONFIG_IS_ENABLED(CLK)
	clk_enable(&priv->clk);
#endif
	return 0;
}

static int xspi_of_to_plat(struct udevice *bus)
{
	struct xspi_plat *plat = dev_get_plat(bus);

	plat->regs = dev_read_addr_index(bus, 0);
	plat->extr = dev_read_addr_index(bus, 1);

#if CONFIG_IS_ENABLED(CLK)
	struct xspi_priv *priv = dev_get_priv(bus);
	int ret;

	ret = clk_get_by_index(bus, 0, &priv->clk);
	if (ret < 0) {
		printf("%s: Could not get clock for %s: %d\n",
			__func__, bus->name, ret);
		return ret;
	}
#endif

	plat->freq = dev_read_u32_default(bus, "spi-max-freq", 50000000);
	return 0;
}

static const struct dm_spi_ops xspi_ops = {
	.xfer		= xspi_xfer,
	.set_speed	= xspi_set_speed,
	.set_mode	= xspi_set_mode,
	.release_bus	= xspi_release_bus,
};

static const struct udevice_id xspi_ids[] = {
	{ .compatible = "renesas,xspi-r9a09g057" },
	{ .compatible = "renesas,xspi-r9a09g056" },
	{ .compatible = "renesas,xspi-r9a09g047" },
	{ }
};

U_BOOT_DRIVER(rzv2h_xspi) = {
	.name		= "rzv2h_xspi",
	.id		= UCLASS_SPI,
	.of_match	= xspi_ids,
	.ops		= &xspi_ops,
	.of_to_plat	= xspi_of_to_plat,
	.plat_auto	= sizeof(struct xspi_plat),
	.priv_auto	= sizeof(struct xspi_priv),
	.bind		= xspi_bind,
	.probe		= xspi_probe,
};
