/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <common.h>
#include <phy.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/io.h>

enum thunderx_smi_mode {
	CLAUSE22 = 0,
	CLAUSE45 = 1,
};

enum {
	SMI_OP_C22_WRITE = 0,
	SMI_OP_C22_READ = 1,

	SMI_OP_C45_ADDR = 0,
	SMI_OP_C45_WRITE = 1,
	SMI_OP_C45_PRIA = 2,
	SMI_OP_C45_READ = 3,
};

union smi_x_clk {
	u64 u;
	struct smi_x_clk_s {
		int phase:8;
		int sample:4;
		int preamble:1;
		int clk_idle:1;
		int reserved_14_14:1;
		int sample_mode:1;
		int sample_hi:5;
		int reserved_21_23:3;
		int mode:1;
	} s;
};

union smi_x_cmd {
	u64 u;
	struct smi_x_cmd_s {
		int reg_adr:5;
		int reserved_5_7:3;
		int phy_adr:5;
		int reserved_13_15:3;
		int phy_op:2;
	} s;
};

union smi_x_wr_dat {
	u64 u;
	struct smi_x_wr_dat_s {
		int dat:16;
		int val:1;
		int pending:1;
	} s;
};

union smi_x_rd_dat {
	u64 u;
	struct smi_x_rd_dat_s {
		int dat:16;
		int val:1;
		int pending:1;
	} s;
};

union smi_x_en {
	u64 u;
	struct smi_x_en_s {
		int en:1;
	} s;
};

#define SMI_X_RD_DAT(x)	(0x87E005003810ull + (x) * 0x80ull)
#define SMI_X_WR_DAT(x)	(0x87E005003808ull + (x) * 0x80ull)
#define SMI_X_CMD(x)	(0x87E005003800ull + (x) * 0x80ull)
#define SMI_X_CLK(x)	(0x87E005003818ull + (x) * 0x80ull)
#define SMI_X_EN(x)	(0x87E005003820ull + (x) * 0x80ull)

struct thunderx_priv {
	int bus_num;
	enum thunderx_smi_mode mode;
};

#define MDIO_TIMEOUT 256

void thunderx_smi_setmode(struct mii_dev *bus, enum thunderx_smi_mode mode)
{
	struct thunderx_priv *priv = bus->priv;
	union smi_x_clk smix_clk;

	if (priv->mode != mode) {
		smix_clk.u = readq(SMI_X_CLK(priv->bus_num));

		smix_clk.s.mode = mode;
		smix_clk.s.preamble = 1;

		writeq(smix_clk.u, SMI_X_CLK(priv->bus_num));

		priv->mode = mode;
	}
}

int thunderx_c45_addr(struct mii_dev *bus, int addr, int devad, int regnum)
{
	struct thunderx_priv *priv = bus->priv;

	union smi_x_cmd smix_cmd;
	union smi_x_wr_dat smix_wr_dat;
	unsigned long timeout = MDIO_TIMEOUT;

	thunderx_smi_setmode(bus, CLAUSE45);

	smix_wr_dat.u = 0;
	smix_wr_dat.s.dat = regnum;

	writeq(smix_wr_dat.u, SMI_X_WR_DAT(priv->bus_num));

	smix_cmd.u = 0;
	smix_cmd.s.phy_op = SMI_OP_C45_ADDR;
	smix_cmd.s.phy_adr = addr;
	smix_cmd.s.reg_adr = devad;

	writeq(smix_cmd.u, SMI_X_CMD(priv->bus_num));

	do {
		smix_wr_dat.u = readq(SMI_X_WR_DAT(priv->bus_num));
		udelay(1);
		timeout--;
	} while (smix_wr_dat.s.pending && timeout);

	return timeout == 0;
}

int thunderx_phy_read(struct mii_dev *bus, int addr, int devad, int regnum)
{
	struct thunderx_priv *priv = bus->priv;
	union smi_x_cmd smix_cmd;
	union smi_x_rd_dat smix_rd_dat;
	unsigned long timeout = MDIO_TIMEOUT;
	int ret;

	enum thunderx_smi_mode mode = (devad < 0) ? CLAUSE22 : CLAUSE45;

	debug("RD: Mode: %u, index: %u, addr: %d, devad: %d, reg: %d\n",
	      mode, priv->bus_num, addr, devad, regnum);

	thunderx_smi_setmode(bus, mode);

	if (mode == CLAUSE45) {
		ret = thunderx_c45_addr(bus, addr, devad, regnum);

		if (ret)
			return 0;
	}

	smix_cmd.u = 0;
	smix_cmd.s.phy_adr = addr;
	smix_cmd.s.reg_adr = regnum;

	if (mode == CLAUSE45)
		smix_cmd.s.phy_op = SMI_OP_C45_READ;
	else
		smix_cmd.s.phy_op = SMI_OP_C22_READ;

	writeq(smix_cmd.u, SMI_X_CMD(priv->bus_num));

	do {
		smix_rd_dat.u = readq(SMI_X_RD_DAT(priv->bus_num));
		udelay(1);
		timeout--;
	} while (smix_rd_dat.s.pending && timeout);

	debug("SMIX_RD_DAT: %lx\n", (unsigned long)smix_rd_dat.u);

	return smix_rd_dat.s.dat;
}

int thunderx_phy_write(struct mii_dev *bus, int addr, int devad, int regnum,
		       u16 value)
{
	struct thunderx_priv *priv = bus->priv;
	union smi_x_cmd smix_cmd;
	union smi_x_wr_dat smix_wr_dat;
	unsigned long timeout = MDIO_TIMEOUT;
	int ret;

	enum thunderx_smi_mode mode = (devad < 0) ? CLAUSE22 : CLAUSE45;

	debug("WR: Mode: %u, index: %u, addr: %d, devad: %d, reg: %d\n",
	      mode, priv->bus_num, addr, devad, regnum);

	if (mode == CLAUSE45) {
		ret = thunderx_c45_addr(bus, addr, devad, regnum);

		if (ret)
			return ret;
	}

	smix_wr_dat.u = 0;
	smix_wr_dat.s.dat = value;

	writeq(smix_wr_dat.u, SMI_X_WR_DAT(priv->bus_num));

	smix_cmd.u = 0;
	smix_cmd.s.phy_adr = addr;
	smix_cmd.s.reg_adr = regnum;

	if (mode == CLAUSE45)
		smix_cmd.s.phy_op = SMI_OP_C45_READ;
	else
		smix_cmd.s.phy_op = SMI_OP_C22_READ;

	writeq(smix_cmd.u, SMI_X_CMD(priv->bus_num));

	do {
		smix_wr_dat.u = readq(SMI_X_WR_DAT(priv->bus_num));
		udelay(1);
		timeout--;
	} while (smix_wr_dat.s.pending && timeout);

	debug("SMIX_WR_DAT: %lx\n", (unsigned long)smix_wr_dat.u);

	return timeout == 0;
}

int thunderx_smi_reset(struct mii_dev *bus)
{
	struct thunderx_priv *priv = bus->priv;

	union smi_x_en smi_en;

	smi_en.s.en = 0;
	writeq(smi_en.u, SMI_X_EN(priv->bus_num));

	smi_en.s.en = 1;
	writeq(smi_en.u, SMI_X_EN(priv->bus_num));

	thunderx_smi_setmode(bus, CLAUSE22);

	return 0;
}

int thunderx_smi_initialize(bd_t *bis, unsigned int index)
{
	struct mii_dev *bus = mdio_alloc();
	struct thunderx_priv *priv = malloc(sizeof(*priv));

	if (!bus || !priv) {
		printf("Failed to allocate ThunderX MDIO bus # %u\n", index);
		return -1;
	}

	bus->read = thunderx_phy_read;
	bus->write = thunderx_phy_write;
	bus->reset = thunderx_smi_reset;

	bus->priv = priv;

	priv->bus_num = index;
	priv->mode = CLAUSE22;

	/* use given name or generate its own unique name */
	snprintf(bus->name, MDIO_NAME_LEN, "thunderx%d", priv->bus_num);

	return mdio_register(bus);
}
