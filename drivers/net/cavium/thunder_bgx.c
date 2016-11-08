/*
 * Copyright (C) 2014 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <dm.h>
#include <pci.h>
#include <misc.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/errno.h>

#ifdef CONFIG_OF_LIBFDT
 #include <libfdt.h>
 #include <fdt_support.h>
#endif

#include <asm/arch/thunderx_smi.h>
#include <asm/arch/thunderx_vnic.h>

#include "nic_reg.h"
#include "nic.h"
#include "thunder_bgx.h"

static const phy_interface_t if_mode[] = {
	[QLM_MODE_SGMII]  = PHY_INTERFACE_MODE_SGMII,
	[QLM_MODE_RGMII]  = PHY_INTERFACE_MODE_RGMII,
	[QLM_MODE_QSGMII] = PHY_INTERFACE_MODE_QSGMII,
	[QLM_MODE_XAUI]   = PHY_INTERFACE_MODE_XAUI,
	[QLM_MODE_RXAUI]  = PHY_INTERFACE_MODE_RXAUI,
};

struct lmac {
	struct bgx		*bgx;
	int			dmac;
	u8			mac[6];
	bool			link_up;
	int			lmacid; /* ID within BGX */
	int			phy_addr; /* ID on board */
	struct eth_device	netdev;
	struct mii_dev		*mii_bus;
	struct phy_device	*phydev;
	unsigned int		last_duplex;
	unsigned int		last_link;
	unsigned int		last_speed;
	int			lane_to_sds;
	int			use_training;
	int			lmac_type;
	u8			qlm_mode;
	int			qlm;
};

struct bgx {
	u8			bgx_id;
	int			node;
	struct	lmac		lmac[MAX_LMAC_PER_BGX];
	int			lmac_count;
	u8			max_lmac;
	void __iomem		*reg_base;
	struct pci_dev		*pdev;
	bool			is_rgx;
};

struct bgx_board_info bgx_board_info[CONFIG_MAX_BGX];

struct bgx *bgx_vnic[CONFIG_MAX_BGX];
bool is_altpkg = 0;

/* APIs to read/write BGXX CSRs */
static uint64_t bgx_reg_read(struct bgx *bgx, uint8_t lmac, uint64_t offset)
{
	uint64_t addr = (uintptr_t)bgx->reg_base +
				((uint32_t)lmac << 20) + offset;

	return readq((void *)addr);
}

static void bgx_reg_write(struct bgx *bgx, uint8_t lmac,
			  uint64_t offset, uint64_t val)
{
	uint64_t addr = (uintptr_t)bgx->reg_base +
				((uint32_t)lmac << 20) + offset;

	writeq(val, (void *)addr);
}

static void bgx_reg_modify(struct bgx *bgx, uint8_t lmac,
			   uint64_t offset, uint64_t val)
{
	uint64_t addr = (uintptr_t)bgx->reg_base +
				((uint32_t)lmac << 20) + offset;

	writeq(val | bgx_reg_read(bgx, lmac, offset), (void *)addr);
}

static int bgx_poll_reg(struct bgx *bgx, uint8_t lmac,
			uint64_t reg, uint64_t mask, bool zero)
{
	int timeout = 200;
	uint64_t reg_val;

	while (timeout) {
		reg_val = bgx_reg_read(bgx, lmac, reg);
		if (zero && !(reg_val & mask))
			return 0;
		if (!zero && (reg_val & mask))
			return 0;
		mdelay(1);
		timeout--;
	}
	return 1;
}

static int gser_poll_reg(uint64_t reg, int bit, uint64_t mask, uint64_t expected_val, int timeout)
{
	uint64_t reg_val;
	debug("gser_poll_reg: reg = %#llx, mask = %#llx, expected_val = %#llx, bit = %d\n",
		reg, mask, expected_val, bit);
	while (timeout) {
		reg_val = readq(CSR_PA(0, reg)) >> bit;
		if ((reg_val & mask) == (expected_val))
			return 0;
		mdelay(1);
		timeout--;
	}
	return 1;
}

struct lmac *bgx_get_lmac(int node, int bgx_idx, int lmacid)
{
	struct bgx *bgx = bgx_vnic[(node * CONFIG_MAX_BGX_PER_NODE) + bgx_idx];

	if (bgx)
		return &bgx->lmac[lmacid];

	return NULL;
}

const u8 *bgx_get_lmac_mac(int node, int bgx_idx, int lmacid)
{
	struct bgx *bgx = bgx_vnic[(node * CONFIG_MAX_BGX_PER_NODE) + bgx_idx];

	if (bgx)
		return bgx->lmac[lmacid].mac;

	return NULL;
}

void bgx_set_lmac_mac(int node, int bgx_idx, int lmacid, const u8 *mac)
{
	struct bgx *bgx = bgx_vnic[(node * CONFIG_MAX_BGX_PER_NODE) + bgx_idx];

	if (!bgx)
		return;

	memcpy(bgx->lmac[lmacid].mac, mac, 6);
}

/* Return number of BGX present in HW */
void bgx_get_count(int node, int *bgx_count)
{
	int i;
	struct bgx *bgx;

	*bgx_count = 0;
	for (i = 0; i < CONFIG_MAX_BGX_PER_NODE; i++) {
		bgx = bgx_vnic[node * CONFIG_MAX_BGX_PER_NODE + i];
		debug("bgx_vnic[%u]: %p\n", node * CONFIG_MAX_BGX_PER_NODE + i, bgx);
		if (bgx)
			*bgx_count |= (1 << i);
	}
}

/* Return number of LMAC configured for this BGX */
int bgx_get_lmac_count(int node, int bgx_idx)
{
	struct bgx *bgx;

	bgx = bgx_vnic[(node * CONFIG_MAX_BGX_PER_NODE) + bgx_idx];
	if (bgx)
		return bgx->lmac_count;

	return 0;
}

void bgx_lmac_rx_tx_enable(int node, int bgx_idx, int lmacid, bool enable)
{
	struct bgx *bgx = bgx_vnic[(node * CONFIG_MAX_BGX_PER_NODE) + bgx_idx];
	u64 cfg;

	if (!bgx)
		return;

	cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	if (enable)
		cfg |= CMR_PKT_RX_EN | CMR_PKT_TX_EN;
	else
		cfg &= ~(CMR_PKT_RX_EN | CMR_PKT_TX_EN);
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);
}

static void bgx_flush_dmac_addrs(struct bgx *bgx, uint64_t lmac)
{
	uint64_t dmac = 0x00;
	uint64_t offset, addr;

	while (bgx->lmac[lmac].dmac > 0) {
		offset = ((bgx->lmac[lmac].dmac - 1) * sizeof(dmac)) +
			(lmac * MAX_DMAC_PER_LMAC * sizeof(dmac));
		addr = (uintptr_t)bgx->reg_base +
				BGX_CMR_RX_DMACX_CAM + offset;
		writeq(dmac, (void *)addr);
		bgx->lmac[lmac].dmac--;
	}
}

/* Configure BGX LMAC in internal loopback mode */
void bgx_lmac_internal_loopback(int node, int bgx_idx,
				int lmac_idx, bool enable)
{
	struct bgx *bgx;
	struct lmac *lmac;
	u64    cfg;

	bgx = bgx_vnic[(node * CONFIG_MAX_BGX_PER_NODE) + bgx_idx];
	if (!bgx)
		return;

	lmac = &bgx->lmac[lmac_idx];
	if (lmac->qlm_mode == QLM_MODE_SGMII) {
		cfg = bgx_reg_read(bgx, lmac_idx, BGX_GMP_PCS_MRX_CTL);
		if (enable)
			cfg |= PCS_MRX_CTL_LOOPBACK1;
		else
			cfg &= ~PCS_MRX_CTL_LOOPBACK1;
		bgx_reg_write(bgx, lmac_idx, BGX_GMP_PCS_MRX_CTL, cfg);
	} else {
		cfg = bgx_reg_read(bgx, lmac_idx, BGX_SPUX_CONTROL1);
		if (enable)
			cfg |= SPU_CTL_LOOPBACK;
		else
			cfg &= ~SPU_CTL_LOOPBACK;
		bgx_reg_write(bgx, lmac_idx, BGX_SPUX_CONTROL1, cfg);
	}
}

/* Return the DLM used for the BGX */
static int get_qlm_for_bgx(int node, int bgx_id, int index)
{
	int qlm = -1;
	uint64_t cfg;

	switch(bgx_id) {
		case 0:
			qlm = 2;
			cfg = readq(CSR_PA(node, GSERX_CFG(qlm))) & GSERX_CFG_BGX;
			break;
		case 1: 
			qlm = 3;
			cfg = readq(CSR_PA(node, GSERX_CFG(qlm))) & GSERX_CFG_BGX;
			break;
		case 2:
			qlm = 6;
			cfg = readq(CSR_PA(node, GSERX_CFG(qlm))) & GSERX_CFG_BGX;
			break;
		case 3:
			qlm = 4;
			cfg = readq(CSR_PA(node, GSERX_CFG(qlm))) & GSERX_CFG_BGX;
			break;
	}

	debug("get_qlm_for_bgx:qlm%d: cfg = %lld\n", qlm, cfg);

	/* Check if DLM is configured as BGX# */
	if (cfg) {
		if (readq(CSR_PA(node, GSERX_PHY_CTL(qlm))))
			return -1;
		return qlm;
	}
	return -1;
}

static int bgx_lmac_sgmii_init(struct bgx *bgx, int lmacid)
{
	u64 cfg;
	struct lmac *lmac;

	lmac = &bgx->lmac[lmacid];

	debug("bgx_lmac_sgmii_init: bgx_id = %d, lmacid = %d\n", bgx->bgx_id, lmacid);

	bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_TXX_THRESH, 0x30);
	/* max packet size */
	bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_RXX_JABBER, MAX_FRAME_SIZE);

	/* Disable frame alignment if using preamble */
	cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND);
	if (cfg & 1)
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_SGMII_CTL, 0);

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG, CMR_EN);

	/* PCS reset */
	bgx_reg_modify(bgx, lmacid, BGX_GMP_PCS_MRX_CTL, PCS_MRX_CTL_RESET);
	if (bgx_poll_reg(bgx, lmacid, BGX_GMP_PCS_MRX_CTL,
			 PCS_MRX_CTL_RESET, true)) {
		printf("BGX PCS reset not completed\n");
		return -1;
	}

	/* power down, reset autoneg, autoneg enable */
	cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_PCS_MRX_CTL);
	cfg &= ~PCS_MRX_CTL_PWR_DN;
	cfg |= (PCS_MRX_CTL_RST_AN | PCS_MRX_CTL_AN_EN);
	bgx_reg_write(bgx, lmacid, BGX_GMP_PCS_MRX_CTL, cfg);

	/* Disable disparity for QSGMII mode, to prevent propogation across
	   ports. */

	if (lmac->qlm_mode == QLM_MODE_QSGMII) {
		cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_PCS_MISCX_CTL);
		cfg &= ~PCS_MISCX_CTL_DISP_EN;
		bgx_reg_write(bgx, lmacid, BGX_GMP_PCS_MISCX_CTL, cfg);
		return 0; /* Skip checking AN_CPT */
	}

	if (lmac->qlm_mode == QLM_MODE_SGMII) {
		if (bgx_poll_reg(bgx, lmacid, BGX_GMP_PCS_MRX_STATUS,
			 PCS_MRX_STATUS_AN_CPT, false)) {
			printf("BGX AN_CPT not completed\n");
			return -1;
		}
	}

	return 0;
}

static int bgx_lmac_xaui_init(struct bgx *bgx, struct lmac *lmac)
{
	u64 cfg;
	int lmacid = lmac->lmacid;

	/* Reset SPU */
	bgx_reg_modify(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET);
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET, true)) {
		dev_err(&bgx->pdev->dev, "BGX SPU reset not completed\n");
		return -1;
	}

	/* Disable LMAC */
	cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	cfg &= ~CMR_EN;
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);

	bgx_reg_modify(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_LOW_POWER);
	/* Set interleaved running disparity for RXAUI */
	if (lmac->lmac_type == BGX_MODE_RXAUI)
		bgx_reg_modify(bgx, lmacid, BGX_SPUX_MISC_CONTROL,
			       SPU_MISC_CTL_INTLV_RDISP);

	/* Clear receive packet disable */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_MISC_CONTROL);
	cfg &= ~SPU_MISC_CTL_RX_DIS;
	bgx_reg_write(bgx, lmacid, BGX_SPUX_MISC_CONTROL, cfg);

	/* clear all interrupts */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_RX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SMUX_RX_INT, cfg);
	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_INT, cfg);
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SPUX_INT, cfg);

	if (lmac->use_training) {
		bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_LP_CUP, 0x00);
		bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_LD_CUP, 0x00);
		bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_LD_REP, 0x00);
		/* training enable */
		bgx_reg_modify(bgx, lmacid,
			       BGX_SPUX_BR_PMD_CRTL, SPU_PMD_CRTL_TRAIN_EN);
	}

	/* Append FCS to each packet */
	bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_APPEND, SMU_TX_APPEND_FCS_D);

	/* Disable forward error correction */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_FEC_CONTROL);
	cfg &= ~SPU_FEC_CTL_FEC_EN;
	bgx_reg_write(bgx, lmacid, BGX_SPUX_FEC_CONTROL, cfg);

	/* Disable autoneg */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_AN_CONTROL);
	cfg = cfg & ~(SPU_AN_CTL_AN_EN | SPU_AN_CTL_XNP_EN);
	bgx_reg_write(bgx, lmacid, BGX_SPUX_AN_CONTROL, cfg);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_AN_ADV);
	if (lmac->lmac_type == BGX_MODE_10G_KR)
		cfg |= (1 << 23);
	else if (lmac->lmac_type == BGX_MODE_40G_KR)
		cfg |= (1 << 24);
	else
		cfg &= ~((1 << 23) | (1 << 24));
	cfg = cfg & (~((1ULL << 25) | (1ULL << 22) | (1ULL << 12)));
	bgx_reg_write(bgx, lmacid, BGX_SPUX_AN_ADV, cfg);

	cfg = bgx_reg_read(bgx, 0, BGX_SPU_DBG_CONTROL);
	cfg &= ~SPU_DBG_CTL_AN_ARB_LINK_CHK_EN;
	bgx_reg_write(bgx, 0, BGX_SPU_DBG_CONTROL, cfg);

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG, CMR_EN);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_CONTROL1);
	cfg &= ~SPU_CTL_LOW_POWER;
	bgx_reg_write(bgx, lmacid, BGX_SPUX_CONTROL1, cfg);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_CTL);
	cfg &= ~SMU_TX_CTL_UNI_EN;
	cfg |= SMU_TX_CTL_DIC_EN;
	bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_CTL, cfg);

	/* take lmac_count into account */
	bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_THRESH, (0x100 - 1));
	/* max packet size */
	bgx_reg_modify(bgx, lmacid, BGX_SMUX_RX_JABBER, MAX_FRAME_SIZE);

	return 0;
}


int __rx_equalization(int qlm, int lane)
{
	int max_lanes = 2;
	int l;
	int fail = 0;

	/* Before completing Rx equalization wait for GSERx_RX_EIE_DETSTS[CDRLOCK] to be set
	   This ensures the rx data is valid */
	if (lane == -1) {
		if (gser_poll_reg(GSER_RX_EIE_DETSTS(qlm), GSER_CDRLOCK, 0xf, (1 << max_lanes) - 1, 100)) {
			printf("ERROR: DLM%d: CDR Lock not detected for 2 lanes\n", qlm);
			return -1;
		}
	} else {
		if (gser_poll_reg(GSER_RX_EIE_DETSTS(qlm), GSER_CDRLOCK, (0xf & (1 << lane)), (1 << lane), 100)) {
			printf("ERROR: DLM%d: CDR Lock not detected on %d lane\n", qlm, lane);
			return -1;
		}
	}

	for (l = 0; l < max_lanes; l++) {
		uint64_t rctl, reer;

		if ((lane != -1) && (lane != l))
			continue;

		/* Enable software control */
		rctl = readq(CSR_PA(0, GSER_BR_RXX_CTL(qlm, l)));
		rctl |= GSER_BR_RXX_CTL_RXT_SWM;
		writeq(CSR_PA(0, GSER_BR_RXX_CTL(qlm, l)), rctl);

		/* Clear the completion flag and initiate a new request */
		reer = readq(CSR_PA(0, GSER_BR_RXX_EER(qlm, l)));
		reer &= ~GSER_BR_RXX_EER_RXT_ESV;
		reer |= GSER_BR_RXX_EER_RXT_EER;
		writeq(CSR_PA(0, GSER_BR_RXX_EER(qlm, l)), reer);
	}

	/* Wait for RX equalization to complete */
	for (l = 0; l < max_lanes; l++) {
		uint64_t rctl, reer;

		if ((lane != -1) && (lane != l))
			continue;

		gser_poll_reg(GSER_BR_RXX_EER(qlm, l), EER_RXT_ESV, 1, 1, 200);
		reer = readq(CSR_PA(0, GSER_BR_RXX_EER(qlm, l)));

		/* Switch back to hardware control */
		rctl = readq(CSR_PA(0, GSER_BR_RXX_CTL(qlm, l)));
		rctl &= ~GSER_BR_RXX_CTL_RXT_SWM;
		writeq(CSR_PA(0, GSER_BR_RXX_CTL(qlm, l)), rctl);

		if (reer & GSER_BR_RXX_EER_RXT_ESV) {
			debug("Rx equalization completed on DLM%d lane%d, rxt_esm = 0x%llx\n",
				qlm, l, (reer & 0x3fff));
		} else {
			debug("Rx equalization timedout on DLM%d lane%d\n", qlm, l);
			fail = 1;
		}
	}

	return (fail) ? -1 : 0;
}

static int bgx_xaui_check_link(struct lmac *lmac)
{
	struct bgx *bgx = lmac->bgx;
	int lmacid = lmac->lmacid;
	int lmac_type = lmac->lmac_type;
	u64 cfg;

	if (lmac->use_training) {
		cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_INT);
		if (!(cfg & (1ull << 13))) {
			cfg = (1ull << 13) | (1ull << 14);
			bgx_reg_write(bgx, lmacid, BGX_SPUX_INT, cfg);
			cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_BR_PMD_CRTL);
			cfg |= (1ull << 0);
			bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_CRTL, cfg);
			return -1;
		}
	}

	/* wait for PCS to come out of reset */
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET, true)) {
		dev_err(&bgx->pdev->dev, "BGX SPU reset not completed\n");
		return -1;
	}

	if ((lmac_type == BGX_MODE_10G_KR) || (lmac_type == BGX_MODE_XFI) ||
	    (lmac_type == BGX_MODE_40G_KR) || (lmac_type == BGX_MODE_XLAUI)) {
		if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_BR_STATUS1,
				 SPU_BR_STATUS_BLK_LOCK, false)) {
			dev_err(&bgx->pdev->dev,
				"SPU_BR_STATUS_BLK_LOCK not completed\n");
			return -1;
		}
	} else {
		if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_BX_STATUS,
				 SPU_BX_STATUS_RX_ALIGN, false)) {
			dev_err(&bgx->pdev->dev,
				"SPU_BX_STATUS_RX_ALIGN not completed\n");
			return -1;
		}
	}

	/* Clear rcvflt bit (latching high) and read it back */
	if (bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS2) & SPU_STATUS2_RCVFLT)
		bgx_reg_modify(bgx, lmacid,
			       BGX_SPUX_STATUS2, SPU_STATUS2_RCVFLT);
	if (bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS2) & SPU_STATUS2_RCVFLT) {
		dev_err(&bgx->pdev->dev, "Receive fault, retry training\n");
		if (lmac->use_training) {
			cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_INT);
			if (!(cfg & (1ull << 13))) {
				cfg = (1ull << 13) | (1ull << 14);
				bgx_reg_write(bgx, lmacid, BGX_SPUX_INT, cfg);
				cfg = bgx_reg_read(bgx, lmacid,
						   BGX_SPUX_BR_PMD_CRTL);
				cfg |= (1ull << 0);
				bgx_reg_write(bgx, lmacid,
					      BGX_SPUX_BR_PMD_CRTL, cfg);
				return -1;
			}
		}
		return -1;
	}

	/* Wait for MAC RX to be ready */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_RX_CTL,
			 SMU_RX_CTL_STATUS, true)) {
		printf( "SMU RX link not okay\n");
		return -1;
	}

	/* Wait for BGX RX to be idle */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_CTL, SMU_CTL_RX_IDLE, false)) {
		dev_err(&bgx->pdev->dev, "SMU RX not idle\n");
		return -1;
	}

	/* Wait for BGX TX to be idle */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_CTL, SMU_CTL_TX_IDLE, false)) {
		dev_err(&bgx->pdev->dev, "SMU TX not idle\n");
		return -1;
	}

	/* Check for MAC RX faults */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_RX_CTL);
	/* 0 - Link is okay, 1 - Local fault, 2 - Remote fault */
	cfg &= SMU_RX_CTL_STATUS;
	if (!cfg)
		return 0;

	/* Rx local/remote fault seen.
	 * Do lmac reinit to see if condition recovers
	 */
	bgx_lmac_xaui_init(bgx, lmac);

	return -1;
}


int bgx_poll_for_link(int node, int bgx_idx, int lmacid)
{
	int ret;
	struct lmac *lmac = bgx_get_lmac(node, bgx_idx, lmacid);
	char mii_name[10];

	if (lmac == NULL) {
		printf("LMAC %d/%d/%d is disabled or doesn't exist\n",
		       node, bgx_idx, lmacid);
		return 0;
	}

	/* Receive link is latching low. Force it high and verify it */
	bgx_reg_modify(lmac->bgx, lmac->lmacid,
		       BGX_SPUX_STATUS1, SPU_STATUS1_RCV_LNK);
	bgx_poll_reg(lmac->bgx, lmac->lmacid, BGX_SPUX_STATUS1,
		     SPU_STATUS1_RCV_LNK, false);

	debug("%s: %d, lmac: %d/%d/%d %p\n",
	      __FILE__, __LINE__,
	      node, bgx_idx, lmacid, lmac);

	if ((lmac->qlm_mode == QLM_MODE_SGMII) ||
	    (lmac->qlm_mode == QLM_MODE_RGMII) ||
	    (lmac->qlm_mode == QLM_MODE_QSGMII)) {
		snprintf(mii_name, sizeof(mii_name), "txsmi%d",
			 bgx_board_info[bgx_idx].mdio_bus);

		debug("mii_name: %s\n", mii_name);

		lmac->mii_bus = miiphy_get_dev_by_name(mii_name);
		lmac->phy_addr = bgx_board_info[bgx_idx].phy_addr[lmacid];

		debug("lmac->mii_bus: %p\n",lmac->mii_bus);
		if (!lmac->mii_bus) {
			printf("MDIO device %s not found\n", mii_name);
			ret = -ENODEV;
			return ret;
		}

		lmac->phydev = phy_connect(lmac->mii_bus, lmac->phy_addr,
					   &lmac->netdev,
					   if_mode[lmac->qlm_mode]);

		if (!lmac->phydev) {
			printf("%s: No PHY device\n",
				lmac->netdev.name);
			return -1;
		}

		ret = phy_config(lmac->phydev);
		if (ret) {
			printf("%s: Could not initialize PHY %s\n",
				lmac->netdev.name, lmac->phydev->dev->name);
			return ret;
		}

		ret = phy_startup(lmac->phydev);
		debug("%s: %d\n", __FILE__, __LINE__);
		if (ret) {
			printf("%s: Could not initialize PHY %s\n",
				lmac->netdev.name, lmac->phydev->dev->name);
		}

#ifdef CONFIG_THUNDERX_XCV
		if (lmac->qlm_mode == QLM_MODE_RGMII)
			xcv_setup_link(lmac->phydev->link, lmac->phydev->speed);
#endif

		lmac->link_up = lmac->phydev->link;
		lmac->last_speed = lmac->phydev->speed;
		lmac->last_duplex = lmac->phydev->duplex;
	} else {
		u64 status1;
		u64 rx_ctl;
		status1 = bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SPUX_STATUS1);
		rx_ctl = bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SMUX_RX_CTL);

		debug("BGX%d LMAC%d BGX_SPUX_STATUS2: %lx\n",
		      bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SPUX_STATUS2));
		debug("BGX%d LMAC%d BGX_SPUX_STATUS1: %lx\n",
		      bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SPUX_STATUS1));
		debug("BGX%d LMAC%d BGX_SMUX_RX_CTL: %lx\n",
		      bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SMUX_RX_CTL));
		debug("BGX%d LMAC%d BGX_SMUX_TX_CTL: %lx\n",
		      bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SMUX_TX_CTL));

		if ((status1 & SPU_STATUS1_RCV_LNK) &&
		    ((rx_ctl & SMU_RX_CTL_STATUS) == 0)) {
			lmac->link_up = 1;
			if (lmac->lmac_type == 4)
				lmac->last_speed = 40000;
			else
				lmac->last_speed = 10000;
			lmac->last_duplex = 1;
		} else {
			lmac->link_up = 0;
			lmac->last_speed = 0;
			lmac->last_duplex = 0;
		}

		if (bgx_xaui_check_link(lmac)) {
				/* Errors, clear link_up state */
				lmac->link_up = 0;
				lmac->last_speed = 0;
				lmac->last_duplex = 0;
		}

		lmac->last_link = lmac->link_up;
	}

	printf("BGX%d:LMAC %u link %s\n", bgx_idx, lmacid,  (lmac->link_up) ? "up" : "down");

	return lmac->link_up;
}


static int bgx_lmac_enable(struct bgx *bgx, int8_t lmacid)
{
	struct lmac *lmac;
	uint64_t cfg;

	lmac = &bgx->lmac[lmacid];
	lmac->bgx = bgx;

	debug("bgx_lmac_enable: lmac: %p, lmacid = %d\n", lmac, lmacid);

	if ((lmac->qlm_mode == QLM_MODE_SGMII) ||
	    (lmac->qlm_mode == QLM_MODE_RGMII) ||
	    (lmac->qlm_mode == QLM_MODE_QSGMII)) {
		if (bgx_lmac_sgmii_init(bgx, lmacid)) {
			printf("bgx_lmac_sgmii_init failed\n");
			return -1;
		}
		cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1)); /* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_MIN_PKT, 60 - 1);
	} else {
		if (bgx_lmac_xaui_init(bgx, lmac))
			return -1;
		cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1)); /* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_MIN_PKT, 60 + 4);
	}

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG,
		       CMR_EN | CMR_PKT_RX_EN | CMR_PKT_TX_EN);

	return 0;
}

void bgx_lmac_disable(struct bgx *bgx, uint8_t lmacid)
{
	struct lmac *lmac;
	uint64_t cmrx_cfg;

	lmac = &bgx->lmac[lmacid];

	cmrx_cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	cmrx_cfg &= ~(1 << 15);
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cmrx_cfg);
	bgx_flush_dmac_addrs(bgx, lmacid);

	if (lmac->phydev)
		phy_shutdown(lmac->phydev);

	lmac->phydev = NULL;
}

/* Program BGXX_CMRX_CONFIG.{lmac_type,lane_to_sds} for each interface.
 * And the number of LMACs used by this interface. Each lmac can be in
 * programmed in a different mode, so parse each lmac one at a time. */
static void bgx_init_hw(struct bgx *bgx)
{
	struct lmac *lmac;
	int i, lmacid, lmac_count = 0;

	for (lmacid = 0; lmacid < bgx->max_lmac; lmacid++) {
		lmac = &bgx->lmac[lmacid];

		/* If QLM is not programmed, skip */
		if (lmac->qlm == -1)
			continue;

		switch (lmac->qlm_mode) {
		case QLM_MODE_SGMII:
			/* On EBB800, DLM0 and DLM1 has only one lane, so adjust the
			   lane_to_sds for 2nd port in BGX0 to DLM1, lane0. */
			if ((bgx->bgx_id == 0) && is_altpkg) {
				if (lmacid >= 2)
					continue;
				else if (lmacid == 1)
					lmac->lane_to_sds = lmacid + 1;
				else
					lmac->lane_to_sds = lmacid;
			} else
				lmac->lane_to_sds = lmacid;
			lmac->lmac_type = 0;
			lmac_count++;
			break;
		case QLM_MODE_XAUI:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 1;
			lmac->lane_to_sds = 0xE4;
			lmac_count = 1;
			break;
		case QLM_MODE_RXAUI:
			if (lmacid == 0) {
				lmac->lmac_type = 2;
				lmac->lane_to_sds = 0x4;
				lmac_count++;
			} else if (lmacid == 1) {
				struct lmac *tlmac;
				tlmac = &bgx->lmac[2];
				if (tlmac->qlm_mode == QLM_MODE_RXAUI) {
					lmac->lmac_type = 2;
					lmac->lane_to_sds = 0xe;
					lmac_count++;
				}
				continue;
			} else
				continue;
			break;
		case QLM_MODE_XFI:
			/* On EBB800, DLM0 and DLM1 has only one lane, so adjust the
			   lane_to_sds for 2nd port in BGX0 to DLM1 lane0. */
			if ((bgx->bgx_id == 0) && is_altpkg) {
				if (lmacid >= 2)
					continue;
				else if (lmacid == 1)
					lmac->lane_to_sds = lmacid + 1;
				else
					lmac->lane_to_sds = lmacid;
			} else
				lmac->lane_to_sds = lmacid;
			lmac->lmac_type = 3;
			lmac_count++;
			break;
		case QLM_MODE_XLAUI:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 4;
			lmac->lane_to_sds = 0xE4;
			lmac_count = 1;
			break;
		case QLM_MODE_10G_KR:
			if ((bgx->bgx_id == 0) && is_altpkg) {
				if (lmacid >= 2)
					continue;
				else if (lmacid == 1)
					lmac->lane_to_sds = lmacid + 1;
				else
					lmac->lane_to_sds = lmacid;
			} else
				lmac->lane_to_sds = lmacid;
			lmac->lmac_type = 3;
			lmac->use_training = 1;
			lmac_count++;
			break;
		case QLM_MODE_40G_KR4:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 4;
			lmac->lane_to_sds = 0xE4;
			lmac->use_training = 1;
			lmac_count = 1;
			break;
		case QLM_MODE_RGMII:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 5;
			lmac->lane_to_sds = 0xE4;
			lmac_count = 1;
			break;
		case QLM_MODE_QSGMII:
			if ((lmacid == 0) || (lmacid == 2)) {
				lmac_count = 4;
				for (i = 0; i < 4; i++) {
					struct lmac *l;
					l = &bgx->lmac[i];
					l->lmac_type = 6;
					l->qlm_mode = QLM_MODE_QSGMII;
					l->lane_to_sds = lmacid + i;
					bgx_reg_write(bgx, i, BGX_CMRX_CFG,
						(l->lmac_type << 8) | l->lane_to_sds);
				}
			}
			continue;
		default:
			lmac_count++;
			continue;
		}

		/* Initialize lmac_type and lane_to_sds */
		bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG,
				(lmac->lmac_type << 8) | lmac->lane_to_sds);
	}

	printf("BGX%d LMACs: %d\n", bgx->bgx_id, lmac_count);
	bgx->lmac_count = lmac_count;
	bgx_reg_write(bgx, 0, BGX_CMR_RX_LMACS, lmac_count);
	bgx_reg_write(bgx, 0, BGX_CMR_TX_LMACS, lmac_count);

	bgx_reg_modify(bgx, 0, BGX_CMR_GLOBAL_CFG, CMR_GLOBAL_CFG_FCS_STRIP);
	if (bgx_reg_read(bgx, 0, BGX_CMR_BIST_STATUS))
		printf("BGX%d BIST failed\n", bgx->bgx_id);

	/* Set the backpressure AND mask */
	for (i = 0; i < bgx->lmac_count; i++)
		bgx_reg_modify(bgx, 0, BGX_CMR_CHAN_MSK_AND,
			       ((1ULL << MAX_BGX_CHANS_PER_LMAC) - 1) <<
				(i * MAX_BGX_CHANS_PER_LMAC));

	/* Disable all MAC filtering */
	for (i = 0; i < RX_DMAC_COUNT; i++)
		bgx_reg_write(bgx, 0, BGX_CMR_RX_DMACX_CAM + (i * 8), 0x00);

	/* Disable MAC steering (NCSI traffic) */
	for (i = 0; i < RX_TRAFFIC_STEER_RULE_COUNT; i++)
		bgx_reg_write(bgx, 0, BGX_CMR_RX_STREERING + (i * 8), 0x00);
}

static void bgx_set_max_lmac(struct bgx *bgx)
{
	int lmac_type;
	bgx->max_lmac = MAX_LMAC_PER_BGX;

	/* Read LMACx type to figure out QLM mode
	 * This is configured by low level firmware
	 */
	lmac_type = bgx_reg_read(bgx, 0, BGX_CMRX_CFG);
	lmac_type = (lmac_type >> 8) & 0x07;

	switch (lmac_type) {
		case BGX_MODE_SGMII:
			bgx->max_lmac = 4;
			break;
		case BGX_MODE_XAUI:
			bgx->max_lmac = 1;
			break;
		case BGX_MODE_RXAUI:
			bgx->max_lmac = 2;
			break;
		case BGX_MODE_XFI:
			bgx->max_lmac = 4;
			break;
		case BGX_MODE_XLAUI:
			bgx->max_lmac = 1;
			break;
		case BGX_MODE_QSGMII:
			bgx->max_lmac = 4;
			break;
		default:
			break;
	}

	/* BGX3 is connected to DLM4 */
	if (bgx->bgx_id == 3 && bgx->max_lmac > 2)
		bgx->max_lmac = 2;

	debug("BGX:%d MAX_LMAC:%d\n",bgx->bgx_id, bgx->max_lmac);
}

/* BDK programs lmac_type for lmac0
 * let us program for others as well
 */
static void bgx_prog_lmac_type(struct bgx *bgx)
{
	int lmac_type;
	int lmacid;

	lmac_type = bgx_reg_read(bgx, 0, BGX_CMRX_CFG);

	for (lmacid = 1; lmacid < bgx->max_lmac && lmac_type != -1;
			lmacid++)
		bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG,lmac_type);
}

static void bgx_get_qlm_mode(struct bgx *bgx)
{
	struct lmac *lmac;
	int lmacid;

	/* Read LMACx type to figure out QLM mode
	 * This is configured by low level firmware
	 */
	for (lmacid = 0; lmacid < bgx->max_lmac; lmacid++) {
		int lmac_type;
		int train_en;

		lmac = &bgx->lmac[lmacid];

		if (lmac->qlm == -1)
			continue;

		lmac_type = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
		lmac->lmac_type = (lmac_type >> 8) & 0x07;
		debug("bgx_get_qlm_mode:%d:%d: lmac_type = %d\n", bgx->bgx_id,
				lmacid, lmac->lmac_type);

		train_en = (readq(CSR_PA(0, GSERX_SCRATCH(lmac->qlm))) & 0xf);

		switch(lmac->lmac_type) {
		case BGX_MODE_SGMII:
			if (bgx->is_rgx) {
				if (lmacid == 0) {
					lmac->qlm_mode = QLM_MODE_RGMII;
					printf("BGX%d LMAC%d mode: RGMII\n",
							bgx->bgx_id, lmacid);
				}
				continue;
			} else {
				if ((bgx->bgx_id == 0) && is_altpkg) {
					if (lmacid >= 2)
						continue;
				}
				lmac->qlm_mode = QLM_MODE_SGMII;
				printf("BGX%d QLM%d LMAC%d mode: SGMII\n",
						bgx->bgx_id, lmac->qlm, lmacid);
			}
			break;
		case BGX_MODE_XAUI:
			if ((bgx->bgx_id == 0) && is_altpkg)
				continue;
			lmac->qlm_mode = QLM_MODE_XAUI;
			printf("BGX%d QLM%d LMAC%d mode: XAUI\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case BGX_MODE_RXAUI:
			if ((bgx->bgx_id == 0) && is_altpkg)
				continue;
			lmac->qlm_mode = QLM_MODE_RXAUI;
			break;
		case BGX_MODE_XFI:
			if (((lmacid < 2) && (train_en & (1 << lmacid)))
			    || (train_en & (1 << (lmacid - 2)))) {
				lmac->qlm_mode = QLM_MODE_10G_KR;
				printf("BGX%d QLM%d LMAC%d mode: 10G_KR\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			} else {
				lmac->qlm_mode = QLM_MODE_XFI;
				printf("BGX%d QLM%d LMAC%d mode: XFI\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			}
			break;
		case BGX_MODE_XLAUI:
			if ((bgx->bgx_id == 0) && is_altpkg)
				continue;
			if (train_en) {
				lmac->qlm_mode = QLM_MODE_40G_KR4;
				if (lmacid != 0)
					break;
				printf("BGX%d QLM%d LMAC%d mode: 40G_KR4\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			} else {
				lmac->qlm_mode = QLM_MODE_XLAUI;
				if (lmacid != 0)
					break;
				printf("BGX%d QLM%d LMAC%d mode: XLAUI\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			}
		break;
		case BGX_MODE_QSGMII:
			lmac->qlm_mode = QLM_MODE_QSGMII;
			printf("BGX%d QLM%d LMAC%d mode: QSGMII\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			break;
		default:
			break;
		}
	}
}

void bgx_set_board_info(unsigned int bgx_id, unsigned int mdio_bus,
			unsigned int *phy_addr)
{
	unsigned int i;

	bgx_board_info[bgx_id].mdio_bus = mdio_bus;

	for (i = 0; i < MAX_LMAC_PER_BGX; i++)
		bgx_board_info[bgx_id].phy_addr[i] = phy_addr[i];
}

int thunderx_bgx_probe(struct udevice *dev)
{
	int err;
	struct bgx *bgx = dev_get_priv(dev);
	uint8_t lmac = 0;
	int bgx_idx, node;
	size_t size;

	bgx->reg_base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);

	debug("%s: bgx base = %llx\n", __func__, (uint64_t)bgx->reg_base);
	is_altpkg = alternate_pkg();

#ifdef CONFIG_THUNDERX_XCV
	/* Use FAKE BGX2 for RGX interface */
	if ((((uintptr_t)bgx->reg_base >> 24) & 0xf) == 0x8) {
		bgx->bgx_id = 2;
		bgx->is_rgx = true;
		for (lmac = 0; lmac < MAX_LMAC_PER_BGX; lmac++) {
			if (lmac == 0) {
				bgx->lmac[lmac].lmacid = 0;
				bgx->lmac[lmac].qlm = 0;
			} else {
				bgx->lmac[lmac].qlm = -1;
			}
		}
		xcv_init_hw();
		goto skip_qlm_config;
	}
#endif

	node = node_id(bgx->reg_base);
	bgx_idx = ((uintptr_t)bgx->reg_base >> 24) & 3;
	bgx->bgx_id = (node * CONFIG_MAX_BGX_PER_NODE) + bgx_idx;

	/* MAP configuration registers */
	for (lmac = 0; lmac < MAX_LMAC_PER_BGX; lmac++) {
		bgx->lmac[lmac].qlm = get_qlm_for_bgx(node, bgx_idx, lmac);;
		bgx->lmac[lmac].lmacid = lmac;
		debug("qlm[%d] =%d \n", lmac, bgx->lmac[lmac].qlm);
	}

	bgx_set_max_lmac(bgx);
	bgx_prog_lmac_type(bgx);

#ifdef CONFIG_THUNDERX_XCV
skip_qlm_config:
#endif
	bgx_vnic[bgx->bgx_id] = bgx;
	bgx_get_qlm_mode(bgx);
	debug("bgx_vnic[%u]: %p\n", bgx->bgx_id, bgx);

	bgx_init_hw(bgx);

	/* Enable all LMACs */
	for (lmac = 0; lmac < bgx->lmac_count; lmac++) {
		snprintf(bgx->lmac[lmac].netdev.name,
			 sizeof(bgx->lmac[lmac].netdev.name),
			 "lmac%d", lmac);

		err = bgx_lmac_enable(bgx, lmac);
		if (err) {
			printf("BGX%d failed to enable lmac%d\n",
				bgx->bgx_id, lmac);
		}
	}

	return 0;
}

static const struct misc_ops thunderx_bgx_ops = {
};

static const struct udevice_id thunderx_bgx_ids[] = {
	{ .compatible = "cavium,bgx" },
	{}
};

U_BOOT_DRIVER(thunderx_bgx) = {
	.name	= "thunderx_bgx",
	.id	= UCLASS_MISC,
	.probe	= thunderx_bgx_probe,
	.of_match = thunderx_bgx_ids,
	.ops	= &thunderx_bgx_ops,
	.priv_auto_alloc_size = sizeof(struct bgx),
};

static struct pci_device_id thunderx_bgx_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_THUNDER_BGX) },
#ifdef CONFIG_THUNDERX_XCV
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_THUNDER_RGX) },
#endif
	{}
};

U_BOOT_PCI_DEVICE(thunderx_bgx, thunderx_bgx_supported);
