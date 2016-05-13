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
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/errno.h>

#ifdef CONFIG_OF_LIBFDT
 #include <libfdt.h>
 #include <fdt_support.h>
#endif

#include <cavium/thunderx_smi.h>
#include <cavium/thunderx_vnic.h>

#include "nic_reg.h"
#include "nic.h"
#include "thunder_bgx.h"

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
	struct	lmac		lmac[MAX_LMAC_PER_BGX];
	int			lmac_count;
	void __iomem		*reg_base;
	struct pci_dev		*pdev;
};

struct bgx_board_info bgx_board_info[CONFIG_MAX_BGX];

struct bgx *bgx_vnic[CONFIG_MAX_BGX];

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
	int qlm = (bgx_id) ? 2 : 0;
	uint64_t cfg1, cfg2;

	cfg1 = readq(CSR_PA(node, GSERX_CFG(qlm))) & GSERX_CFG_BGX;
	cfg2 = readq(CSR_PA(node, GSERX_CFG(qlm+1))) & GSERX_CFG_BGX;
	debug("get_qlm_for_bgx:qlm%d: cfg1 = %lld, cfg2 = %lld\n", qlm, cfg1, cfg2);

	/* Check if both DLMs are configured as BGX# */
	if (cfg2) {
		if (cfg1) {
			if (readq(CSR_PA(node, GSERX_PHY_CTL(qlm)))
			    && readq(CSR_PA(node, GSERX_PHY_CTL(qlm+1))))
				return -1;
			if (index >= 2)
				return qlm+1;
			else
				return qlm;
		} else { /* Only 2nd DLM is used */
			if (readq(CSR_PA(node, GSERX_PHY_CTL(qlm + 1))))
				return -1;
			return qlm + 1;
		}
	} else if (cfg1) {
		if (readq(CSR_PA(node, GSERX_PHY_CTL(qlm))))
			return -1;
		if (index < 2)
			return qlm;
		else
			return -1;
	}
	return -1;
}

static int bgx_lmac_sgmii_init(struct bgx *bgx, int lmacid)
{
	u64 cfg;

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

	if (bgx_poll_reg(bgx, lmacid, BGX_GMP_PCS_MRX_STATUS,
			 PCS_MRX_STATUS_AN_CPT, false)) {
		printf("BGX AN_CPT not completed\n");
		return -1;
	}

	return 0;
}

static int bgx_lmac_xaui_init(struct bgx *bgx, int lmacid, int lmac_type)
{
	u64 cfg;
	struct lmac *lmac;

	lmac = &bgx->lmac[lmacid];

	/* Reset SPU */
	bgx_reg_modify(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET);
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET, true)) {
		printf("BGX SPU reset not completed\n");
		return -1;
	}

	/* Disable LMAC */
	cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	cfg &= ~CMR_EN;
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);

	bgx_reg_modify(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_LOW_POWER);
	/* Set interleaved running disparity for RXAUI */
	if (lmac->qlm_mode != QLM_MODE_RXAUI)
		bgx_reg_modify(bgx, lmacid,
			       BGX_SPUX_MISC_CONTROL, SPU_MISC_CTL_RX_DIS);
	else
		bgx_reg_modify(bgx, lmacid, BGX_SPUX_MISC_CONTROL,
			       SPU_MISC_CTL_RX_DIS | SPU_MISC_CTL_INTLV_RDISP);

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
	cfg = cfg & ~(SPU_AN_CTL_XNP_EN);
	if (lmac->use_training)
		cfg = cfg | (SPU_AN_CTL_AN_EN);
	else
		cfg = cfg & ~(SPU_AN_CTL_AN_EN);
	bgx_reg_write(bgx, lmacid, BGX_SPUX_AN_CONTROL, cfg);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_AN_ADV);
	if (lmac->qlm_mode == QLM_MODE_10G_KR)
		cfg |= (1 << 23);
	else if (lmac->qlm_mode == QLM_MODE_40G_KR4)
		cfg |= (1 << 24);
	else
		cfg &= ~((1 << 23) | (1 << 24));
	cfg = cfg & (~((1ULL << 25) | (1ULL << 22) | (1ULL << 12)));
	bgx_reg_write(bgx, lmacid, BGX_SPUX_AN_ADV, cfg);

	cfg = bgx_reg_read(bgx, 0, BGX_SPU_DBG_CONTROL);
	if (lmac->use_training)
		cfg |= SPU_DBG_CTL_AN_ARB_LINK_CHK_EN;
	else
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
			debug("ERROR: DLM%d: CDR Lock not detected for 2 lanes\n", qlm);
			return -1;
		}
	} else {
		if (gser_poll_reg(GSER_RX_EIE_DETSTS(qlm), GSER_CDRLOCK, (0xf & (1 << lane)), (1 << lane), 100)) {
			debug("ERROR: DLM%d: CDR Lock not detected on %d lane\n", qlm, lane);
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

		gser_poll_reg(GSER_BR_RXX_CTL(qlm, l), EER_RXT_ESV, 1, 1, 200);
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

	bgx_reg_modify(bgx, lmacid, BGX_SPUX_MISC_CONTROL, SPU_MISC_CTL_RX_DIS);
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

	/* Perform RX Equalization. Applies to non-KR interfaces for speeds 
	   >= 6.25Gbps. */ 
	if (!lmac->use_training) {
		int index;
		switch (lmac->lmac_type) {
		default:
		case 0: // SGMII
		case 1: // XAUI
			/* Nothing to do */
			break;
		case 4: // XLAUI
			if (__rx_equalization(lmac->qlm, -1) ||
				__rx_equalization(lmac->qlm+1, -1))
				printf("BGX%d:%d: Waiting for RX Equalization on DLM%d/DLM%d\n",
					bgx->bgx_id, lmacid, lmac->qlm, lmac->qlm+1);
			break;
		case 2: // RXAUI
			if (lmacid > 1)
				index = lmacid - 2;
			else
				index = lmacid;
			if (__rx_equalization(lmac->qlm, index))
				printf("BGX%d:%d: Waiting for RX Equalization on DLM%d\n",
					bgx->bgx_id, index, lmac->qlm);
			break;
		case 3: // XFI
			if (__rx_equalization(lmac->qlm, lmacid))
				printf("BGX%d:%d: Waiting for RX Equalization on DLM%d\n",
					bgx->bgx_id, lmacid, lmac->qlm);
			break;
		}
	}

	/* wait for PCS to come out of reset */
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET, true)) {
		printf("BGX SPU reset not completed\n");
		return -1;
	}

	if ((lmac_type == 3) || (lmac_type == 4)) {
		if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_BR_STATUS1,
				 SPU_BR_STATUS_BLK_LOCK, false)) {
			printf("SPU_BR_STATUS_BLK_LOCK not completed\n");
			return -1;
		}
	} else {
		if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_BX_STATUS,
				 SPU_BX_STATUS_RX_ALIGN, false)) {
			printf("SPU_BX_STATUS_RX_ALIGN not completed\n");
			return -1;
		}
	}

	/* Clear rcvflt bit (latching high) and read it back */
	bgx_reg_modify(bgx, lmacid, BGX_SPUX_STATUS2, SPU_STATUS2_RCVFLT);
	if (bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS2) & SPU_STATUS2_RCVFLT) {
		printf("Receive fault, retry training\n");
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
		printf("SMU RX not idle\n");
		return -1;
	}

	/* Wait for BGX TX to be idle */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_CTL, SMU_CTL_TX_IDLE, false)) {
		printf("SMU TX not idle\n");
		return -1;
	}

	if (bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS2) & SPU_STATUS2_RCVFLT) {
		printf("Receive fault\n");
		return -1;
	}

	/* Receive link is latching low. Force it high and verify it */
	if (!(bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS1 & SPU_STATUS1_RCV_LNK)))
		bgx_reg_modify(bgx, lmacid, BGX_SPUX_STATUS1, SPU_STATUS1_RCV_LNK);
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_STATUS1,
			 SPU_STATUS1_RCV_LNK, false)) {
		printf("SPU receive link down\n");
		return -1;
	}

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_MISC_CONTROL);
	cfg &= ~SPU_MISC_CTL_RX_DIS;
	bgx_reg_write(bgx, lmacid, BGX_SPUX_MISC_CONTROL, cfg);
	return 0;
}

void bgx_poll_for_link(int node, int bgx_idx, int lmacid)
{
	int ret;
	struct lmac *lmac = bgx_get_lmac(node, bgx_idx, lmacid);

	if (lmac == NULL) {
		printf("LMAC %d/%d/%d is disabled or doesn't exist\n",
		       node, bgx_idx, lmacid);
		return;
	}

	debug("%s: %d, lmac: %p\n", __FILE__, __LINE__, lmac);

	if (lmac->qlm_mode == QLM_MODE_SGMII){
		debug("%s: %d, phydev: %p\n", __FILE__, __LINE__, lmac->phydev);
		if (!lmac->phydev) {
			printf("%s: No PHY device\n",
				lmac->netdev.name);
			return;
		}
		ret = phy_startup(lmac->phydev);
		debug("%s: %d\n", __FILE__, __LINE__);
		if (ret) {
			printf("%s: Could not initialize PHY %s\n",
				lmac->netdev.name, lmac->phydev->dev->name);
		}
	} else {
		u64 status1;
		u64 tx_ctl;
		u64 rx_ctl;
		status1 = bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SPUX_STATUS1);
		tx_ctl = bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SMUX_TX_CTL);
		rx_ctl = bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SMUX_RX_CTL);

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
		    ((tx_ctl & SMU_TX_CTL_LNK_STATUS) == 0) &&
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

		if (lmac->last_link != lmac->link_up) {
			lmac->last_link = lmac->link_up;
			if (lmac->link_up)
				bgx_xaui_check_link(lmac);
		}
	}

	printf("LMAC %u link %s\n", lmacid,  (lmac->link_up) ? "up" : "down");
}

static int bgx_lmac_enable(struct bgx *bgx, int8_t lmacid)
{
	struct lmac *lmac;
	uint64_t cfg;
	int ret;

	lmac = &bgx->lmac[lmacid];
	lmac->bgx = bgx;

	debug("bgx_lmac_enable: lmac: %p, lmacid = %d\n", lmac, lmacid);

	if (lmac->qlm_mode == QLM_MODE_SGMII) {
		if (bgx_lmac_sgmii_init(bgx, lmacid)) {
			debug("bgx_lmac_sgmii_init failed\n");
			return -1;
		}
		cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1)); /* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_MIN_PKT, 60 - 1);
	} else {
		if (bgx_lmac_xaui_init(bgx, lmacid, lmac->lmac_type))
			return -1;
		cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1)); /* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_MIN_PKT, 60 + 4);
	}

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG,
		       CMR_EN | CMR_PKT_RX_EN | CMR_PKT_TX_EN);

	
	if (lmac->qlm_mode == QLM_MODE_SGMII) {
		lmac->phydev = phy_connect(lmac->mii_bus, lmac->phy_addr,
					   &lmac->netdev, PHY_INTERFACE_MODE_SGMII);

		if (!lmac->phydev)
			return -1;

		ret = phy_config(lmac->phydev);
		if (ret) {
			printf("%s: Could not initialize PHY %s\n",
				lmac->netdev.name, lmac->phydev->dev->name);
			return ret;
		}
	}

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

	for (lmacid = 0; lmacid < MAX_LMAC_PER_BGX; lmacid++) {
		lmac = &bgx->lmac[lmacid];

		/* If QLM is not programmed, skip */
		if (lmac->qlm == -1)
			continue;

		switch (lmac->qlm_mode) {
		case QLM_MODE_SGMII:
			lmac->lmac_type = 0;
			lmac->lane_to_sds = lmacid;
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
			lmac->lmac_type = 2;
			lmac->lane_to_sds = (lmacid) ? 0xE : 0x4;
			lmac_count++;
			break;
		case QLM_MODE_XFI:
			lmac->lmac_type = 3;
			lmac->lane_to_sds = lmacid;
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
			lmac->lmac_type = 3;
			lmac->lane_to_sds = lmacid;
			lmac->use_training = 1;
			lmac_count++;
			break;
		case QLM_MODE_40G_KR4:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 4;
			lmac->lane_to_sds = 0xE4;
			lmac->use_training = 1;
			lmac_count = 4;
			break;
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

static void bgx_get_qlm_mode(struct bgx *bgx)
{
	struct lmac *lmac;
	int lmacid;

	/* Read LMACx type to figure out QLM mode
	 * This is configured by low level firmware
	 */
	for (lmacid = 0; lmacid < MAX_LMAC_PER_BGX; lmacid++) {
		int lmac_type;
		int train_en;
		int index = (lmacid < 2) ? 0 : 2;

		lmac = &bgx->lmac[lmacid];

		if (lmac->qlm == -1)
			continue;

		lmac_type = bgx_reg_read(bgx, index, BGX_CMRX_CFG);
		lmac->lmac_type = (lmac_type >> 8) & 0x07;
		debug("bgx_get_qlm_mode:%d:%d: lmac_type = %d\n", bgx->bgx_id,
				lmacid, lmac->lmac_type);

		train_en = (readq(CSR_PA(0, GSERX_SCRATCH(lmac->qlm))) & 0xf);

	switch(lmac->lmac_type) {
		case 0:
			lmac->qlm_mode = QLM_MODE_SGMII;
			printf("BGX%d QLM%d LMAC%d mode: SGMII\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case 1:
			lmac->qlm_mode = QLM_MODE_XAUI;
			printf("BGX%d QLM%d LMAC%d mode: XAUI\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case 2:
			lmac->qlm_mode = QLM_MODE_RXAUI;
			printf("BGX%d QLM%d LMAC%d mode: RXAUI\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case 3:
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
		case 4:
			if (((lmacid < 2) && (train_en & (1 << lmacid))) ||
			    (train_en & (1 << (lmacid - 2)))) {
				lmac->qlm_mode = QLM_MODE_40G_KR4;
				printf("BGX%d QLM%d LMAC%d mode: 40G_KR4\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			} else {
				lmac->qlm_mode = QLM_MODE_XLAUI;
				printf("BGX%d QLM%d LMAC%d mode: XLAUI\n",
					bgx->bgx_id, lmac->qlm, lmacid);
			}
		break;
		default:
			break;
		}
	}
}

int thunderx_bgx_initialize(unsigned int bgx_idx, unsigned int node)
{
	int err;
	struct bgx *bgx = NULL;
	uint8_t lmac = 0;
	char mii_name[10];
	int qlm[4] = {-1};

	for (lmac = 0; lmac < MAX_LMAC_PER_BGX; lmac+=2) {
		qlm[lmac] = get_qlm_for_bgx(node, bgx_idx, lmac);
		qlm[lmac+1] = qlm[lmac];
	}

	/* A BGX can take 1 or 2 DLMs, if both the DLMs are not configured
	   as BGX, then return, nothing to initialize */
	if ((qlm[0] == -1) && (qlm[2] == -1))
		return -ENODEV;

	bgx = malloc(sizeof(struct bgx));

	/* MAP configuration registers */
	bgx->reg_base = (void *)CSR_PA(node, BGXX_PF_BAR0(bgx_idx));
	bgx->bgx_id = bgx_idx + node * CONFIG_MAX_BGX_PER_NODE;
	for (lmac = 0; lmac < MAX_LMAC_PER_BGX; lmac++) {
		bgx->lmac[lmac].qlm = qlm[lmac];
		bgx->lmac[lmac].lmacid = lmac;
	}

	bgx_vnic[bgx->bgx_id] = bgx;
	bgx_get_qlm_mode(bgx);
	debug("bgx_vnic[%u]: %p\n", bgx->bgx_id, bgx);

	snprintf(mii_name, sizeof(mii_name), "thunderx%d",
		 bgx_board_info[bgx_idx].mdio_bus);

	debug("mii_name: %s\n", mii_name);

	bgx_init_hw(bgx);

	/* Enable all LMACs */
	for (lmac = 0; lmac < bgx->lmac_count; lmac++) {
		if (bgx->lmac[lmac].qlm_mode == QLM_MODE_SGMII) {
			bgx->lmac[lmac].mii_bus = miiphy_get_dev_by_name(mii_name);
			bgx->lmac[lmac].phy_addr = bgx_board_info[bgx_idx].phy_addr[lmac];
			debug("bgx->lmac[lmac].mii_bus: %p\n", bgx->lmac[lmac].mii_bus);
			if (!bgx->lmac[lmac].mii_bus) {
				printf("MDIO device %s not found\n", mii_name);
				err = -ENODEV;
				goto error;
			}
		}

		err = bgx_lmac_enable(bgx, lmac);
		if (err) {
			printf("BGX%d failed to enable lmac%d\n",
				bgx->bgx_id, lmac);
		}
	}

	return 0;
error:
	bgx_vnic[bgx->bgx_id] = NULL;
	free(bgx);
	return err;
}

void bgx_set_board_info(unsigned int bgx_id, unsigned int mdio_bus,
			unsigned int *phy_addr)
{
	unsigned int i;

	bgx_board_info[bgx_id].mdio_bus = mdio_bus;

	for (i = 0; i < MAX_LMAC_PER_BGX; i++)
		bgx_board_info[bgx_id].phy_addr[i] = phy_addr[i];
}
