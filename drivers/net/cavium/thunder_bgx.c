/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <config.h>
#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <asm/io.h>

#include <cavium/thunderx_smi.h>
#include <cavium/thunderx_vnic.h>

#ifdef CONFIG_OF_LIBFDT
 #include <libfdt.h>
 #include <fdt_support.h>
#endif

#include "nic_reg.h"
#include "nic.h"
#include "thunder_bgx.h"

struct lmac {
	struct bgx *bgx;
	int dmac;
	bool link_up;
	int lmacid;		/* ID within BGX */
	int lmacid_bd;		/* ID on board */
	struct eth_device netdev;
	struct mii_dev *mii_bus;
	struct phy_device *phydev;
	unsigned int last_duplex;
	unsigned int last_link;
	unsigned int last_speed;
	bool is_sgmii;
} lmac;

struct bgx {
	uint8_t bgx_id;
	struct lmac lmac[MAX_LMAC_PER_BGX];
	int lmac_count;
	enum lmac_type lmac_type;
	int lane_to_sds;
	int use_training;
	uint64_t reg_base;
	struct pci_dev *pdev;

	enum qlm_mode qlm_mode;
} bgx;

int lmac_count = 0;	/* Total no of LMACs in system */
struct bgx *bgx_vnic[MAX_BGX_THUNDER];

static int bgx_lmac_xaui_link_init(struct lmac *lmac);

/* Register read/write APIs */
static uint64_t bgx_reg_read(struct bgx *bgx, uint8_t lmac, uint64_t offset)
{
	uint64_t addr = bgx->reg_base + ((uint32_t) lmac << 20) + offset;

	return readq((void *)addr);
}

static void bgx_reg_write(struct bgx *bgx, uint8_t lmac,
			  uint64_t offset, uint64_t val)
{
	uint64_t addr = bgx->reg_base + ((uint32_t) lmac << 20) + offset;

	writeq(val, (void *)addr);
}

static void bgx_reg_modify(struct bgx *bgx, uint8_t lmac,
			   uint64_t offset, uint64_t val)
{
	uint64_t addr = bgx->reg_base + ((uint32_t) lmac << 20) + offset;

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

/**
 * To remove unwanted nodes from fdt .
 *
 *  @param fdt_key - key to preserve.
 *  fdt_key of formate < bgx, qlm-type >.
 *  All non-matching keys are removed
 *
 * */
static void bgx_fdt_patch(const char *fdt_key)
{
	const char *trim_name = "qlm-mode";
	void *fdt = (void *)FDT_ADDR_HACK;

	int offset, next_offset;
	char qlm[32];
	char *mode;
	int qlm_key_len;
	int rc;

	strncpy(qlm, fdt_key, sizeof(qlm));
	mode = qlm;
	strsep(&mode, ",");
	qlm_key_len = strlen(qlm);

	if (!fdt || fdt_check_header(fdt) != 0) {
		printf("%s: Invalid device tree\n", __func__);
		return;
	}

	/* Prune out the unwanted parts based on the QLM mode.  */
	for (offset = fdt_next_node(fdt, 0, NULL);
		 offset >= 0; offset = next_offset) {
		int len;
		const char *val;

		next_offset = fdt_next_node(fdt, offset, NULL);
		val = fdt_getprop(fdt, offset, trim_name, &len);
		if (!val)
			continue;

		if (strncmp(val, qlm, qlm_key_len) != 0)
			continue; /* Not this QLM. */

		if (!fdt_stringlist_contains(val, len, fdt_key)) {
			debug("Key \"%s\" does not match \"%s\"\n",
			      val, fdt_key);
			/* This QLM, but wrong mode.  Delete it. */
			debug("fdt trimming matching key %s\n", fdt_key);
			next_offset = fdt_parent_offset(fdt, offset);
			rc = fdt_nop_node(fdt, offset);
			if (rc) {
				printf("Error %d noping node in device tree\n",
				       rc);
			}
		}
	}
}

/* Return number of BGX present in HW */
void bgx_get_count(int node, int *bgx_count)
{
	int i;
	struct bgx *bgx;

	*bgx_count = 0;
	for (i = 0; i < MAX_BGX_PER_CN88XX; i++) {
		bgx = bgx_vnic[node * MAX_BGX_PER_CN88XX + i];
		debug("bgx_vnic[%u]: %p\n", node * MAX_BGX_PER_CN88XX + i, bgx);
		if (bgx)
			*bgx_count |= (1 << i);
	}
}

/* Return number of LMAC configured for this BGX */
int bgx_get_lmac_count(int node, int bgx_idx)
{
	struct bgx *bgx;

	bgx = bgx_vnic[(node * MAX_BGX_PER_CN88XX) + bgx_idx];
	if (bgx)
		return bgx->lmac_count;

	return 0;
}

static void bgx_lmac_change_link_state(struct lmac *lmac)
{
	struct bgx *bgx = lmac->bgx;
	uint64_t cmr_cfg;
	uint64_t port_cfg = 0;
	uint64_t misc_ctl = 0;

	cmr_cfg = bgx_reg_read(bgx, lmac->lmacid, BGX_CMRX_CFG);
	cmr_cfg &= ~CMR_EN;
	bgx_reg_write(bgx, lmac->lmacid, BGX_CMRX_CFG, cmr_cfg);

	port_cfg = bgx_reg_read(bgx, lmac->lmacid, BGX_GMP_GMI_PRTX_CFG);
	misc_ctl = bgx_reg_read(bgx, lmac->lmacid, BGX_GMP_PCS_MISCX_CTL);

	if (lmac->link_up) {
		misc_ctl &= ~PCS_MISC_CTL_GMX_ENO;
		port_cfg &= ~GMI_PORT_CFG_DUPLEX;
		port_cfg |= (lmac->last_duplex << 2);
	} else {
		misc_ctl |= PCS_MISC_CTL_GMX_ENO;
	}

	switch (lmac->last_speed) {
	case 10:
		port_cfg &= ~GMI_PORT_CFG_SPEED;	/* speed 0 */
		port_cfg |= GMI_PORT_CFG_SPEED_MSB;	/* speed_msb 1 */
		port_cfg &= ~GMI_PORT_CFG_SLOT_TIME;	/* slottime 0 */
		misc_ctl &= ~PCS_MISC_CTL_SAMP_PT_MASK;
		misc_ctl |= 50;	/* samp_pt */
		bgx_reg_write(bgx, lmac->lmacid, BGX_GMP_GMI_TXX_SLOT, 64);
		bgx_reg_write(bgx, lmac->lmacid, BGX_GMP_GMI_TXX_BURST, 0);
		break;
	case 100:
		port_cfg &= ~GMI_PORT_CFG_SPEED;	/* speed 0 */
		port_cfg &= ~GMI_PORT_CFG_SPEED_MSB;	/* speed_msb 0 */
		port_cfg &= ~GMI_PORT_CFG_SLOT_TIME;	/* slottime 0 */
		misc_ctl &= ~PCS_MISC_CTL_SAMP_PT_MASK;
		misc_ctl |= 5;	/* samp_pt */
		bgx_reg_write(bgx, lmac->lmacid, BGX_GMP_GMI_TXX_SLOT, 64);
		bgx_reg_write(bgx, lmac->lmacid, BGX_GMP_GMI_TXX_BURST, 0);
		break;
	case 1000:
		port_cfg |= GMI_PORT_CFG_SPEED;	/* speed 1 */
		port_cfg &= ~GMI_PORT_CFG_SPEED_MSB;	/* speed_msb 0 */
		port_cfg |= GMI_PORT_CFG_SLOT_TIME;	/* slottime 1 */
		misc_ctl &= ~PCS_MISC_CTL_SAMP_PT_MASK;
		misc_ctl |= 1;	/* samp_pt */
		bgx_reg_write(bgx, lmac->lmacid, BGX_GMP_GMI_TXX_SLOT, 512);
		if (lmac->last_duplex)
			bgx_reg_write(bgx, lmac->lmacid,
				      BGX_GMP_GMI_TXX_BURST, 0);
		else
			bgx_reg_write(bgx, lmac->lmacid,
				      BGX_GMP_GMI_TXX_BURST, 8192);
		break;
	default:
		break;
	}
	bgx_reg_write(bgx, lmac->lmacid, BGX_GMP_PCS_MISCX_CTL, misc_ctl);
	bgx_reg_write(bgx, lmac->lmacid, BGX_GMP_GMI_PRTX_CFG, port_cfg);

	port_cfg = bgx_reg_read(bgx, lmac->lmacid, BGX_GMP_GMI_PRTX_CFG);

	/* renable lmac */
	cmr_cfg |= CMR_EN;
	bgx_reg_write(bgx, lmac->lmacid, BGX_CMRX_CFG, cmr_cfg);
}

static void bgx_lmac_handler(struct eth_device *netdev)
{
	struct lmac *lmac = container_of(netdev, struct lmac, netdev);
	struct phy_device *phydev = lmac->phydev;
	int link_changed = 0;

	if (!lmac)
		return;

	if (!phydev->link && lmac->last_link)
		link_changed = -1;

	if (phydev->link &&
	    (lmac->last_duplex != phydev->duplex ||
	     lmac->last_link != phydev->link ||
	     lmac->last_speed != phydev->speed)) {
		link_changed = 1;
	}

	lmac->last_link = phydev->link;
	lmac->last_speed = phydev->speed;
	lmac->last_duplex = phydev->duplex;

	if (!link_changed)
		return;

	if (link_changed > 0) {
		printf("LMAC%d: Link is up - %d/%s\n", lmac->lmacid_bd,
		       phydev->speed,
		       DUPLEX_FULL == phydev->duplex ? "Full" : "Half");
		lmac->link_up = true;
	} else {
		lmac->link_up = false;
		printf("LMAC%d: Link is down\n", lmac->lmacid_bd);
	}

	if (lmac->is_sgmii) {
		bgx_lmac_change_link_state(lmac);
	} else {
		if (!lmac->link_up)
			bgx_lmac_xaui_link_init(lmac);
	}
}

static void bgx_flush_dmac_addrs(struct bgx *bgx, uint64_t lmac)
{
	uint64_t dmac = 0x00;
	uint64_t offset, addr;

	while (bgx->lmac[lmac].dmac > 0) {
		offset = ((bgx->lmac[lmac].dmac - 1) * sizeof(dmac)) +
		    (lmac * MAX_DMAC_PER_LMAC * sizeof(dmac));
		addr = bgx->reg_base + BGX_CMR_RX_DMACX_CAM + offset;
		writeq(dmac, (void *)addr);
		bgx->lmac[lmac].dmac--;
	}
}

static int bgx_lmac_sgmii_init(struct bgx *bgx, int lmacid)
{
	uint64_t cfg;

	bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_TXX_THRESH, 0x30);
	/* max packet size */
	bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_RXX_JABBER, 9216);

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
	uint64_t cfg;

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
	bgx_reg_modify(bgx, lmacid, BGX_SPUX_MISC_CONTROL, SPU_MISC_CTL_RX_DIS);

	/* clear all interrupts */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_RX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SMUX_RX_INT, cfg);
	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_INT, cfg);
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SPUX_INT, cfg);

	if (bgx->use_training) {
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
	if (bgx->lmac_type == BGX_MODE_10G_KR)
		cfg |= (1 << 23);
	else if (bgx->lmac_type == BGX_MODE_40G_KR)
		cfg |= (1 << 24);
	else
		cfg &= ~((1 << 23) | (1 << 24));
	cfg = cfg & (~((1ULL << 25) | (1ULL << 22) | (1ULL << 12)));
	bgx_reg_write(bgx, lmacid, BGX_SPUX_AN_ADV, cfg);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPU_DBG_CONTROL);
	cfg &= ~SPU_DBG_CTL_AN_ARB_LINK_CHK_EN;
	bgx_reg_write(bgx, lmacid, BGX_SPU_DBG_CONTROL, cfg);

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
	bgx_reg_modify(bgx, lmacid, BGX_SMUX_RX_JABBER, 9216);

	return 0;
}

static int bgx_lmac_xaui_link_init(struct lmac *lmac)
{
	struct bgx *bgx = lmac->bgx;
	uint64_t cfg;
	int lmacid = lmac->lmacid;
	int tx_link_ok, rx_link_ok, rcv_link;
	int lmac_type = bgx->lmac_type;

	tx_link_ok = bgx_reg_read(bgx, lmacid,
				  BGX_SMUX_TX_CTL) & SMU_TX_CTL_LNK_STATUS;
	rx_link_ok = bgx_reg_read(bgx, lmacid,
				  BGX_SMUX_RX_CTL) & SMU_RX_CTL_STATUS;
	rcv_link = bgx_reg_read(bgx, lmacid,
				BGX_SPUX_STATUS1) & SPU_STATUS1_RCV_LNK;

	if ((tx_link_ok == 0) && (rx_link_ok == 0) && rcv_link)
		return 0;

	bgx_reg_modify(bgx, lmacid, BGX_SPUX_MISC_CONTROL, SPU_MISC_CTL_RX_DIS);
	if (bgx->use_training) {
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
		printf("BGX SPU reset not completed\n");
		return -1;
	}

	if ((lmac_type == BGX_MODE_10G_KR) || (lmac_type == BGX_MODE_XFI) ||
	    (lmac_type == BGX_MODE_40G_KR) || (lmac_type == BGX_MODE_XLAUI)) {
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
		if (bgx->use_training) {
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
		printf("SMU RX link not okay\n");
		return -1;
	}

	/* Wait for BGX RX to be idle */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_CTL,
			 SMU_CTL_RX_IDLE, false)) {
		printf("SMU RX not idle\n");
		return -1;
	}

	/* Wait for BGX TX to be idle */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_CTL,
			 SMU_CTL_TX_IDLE, false)) {
		printf("SMU TX not idle\n");
		return -1;
	}

	if (bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS2) & SPU_STATUS2_RCVFLT) {
		printf("Receive fault\n");
		return -1;
	}

	/* Receive link is latching low. Force it high and verify it */
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

int bgx_lmac_enable(int bgxid, int8_t lmacid)
{
	struct lmac *lmac;
	struct bgx *bgx = bgx_vnic[bgxid];
	uint64_t cfg;
	int ret;

	lmac = &bgx->lmac[lmacid];
	lmac->bgx = bgx;

	debug("lmac: %p", lmac);

	if (bgx->lmac_type == BGX_MODE_SGMII) {
		lmac->is_sgmii = 1;
		if (bgx_lmac_sgmii_init(bgx, lmacid))
			return -1;
	} else {
		lmac->is_sgmii = 0;
		if (bgx_lmac_xaui_init(bgx, lmacid, bgx->lmac_type))
			return -1;
	}

	if (lmac->is_sgmii) {
		cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1));	/* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_MIN_PKT, 60 - 1);
	} else {
		cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1));	/* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_MIN_PKT, 60 + 4);
	}

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG,
		       CMR_EN | CMR_PKT_RX_EN | CMR_PKT_TX_EN);

	if (!lmac->is_sgmii) {
		printf("%s: Only SGMII is implemented currently\n", __func__);
		return 1;
	}

	lmac->phydev = phy_connect(lmac->mii_bus, lmac->lmacid,
				   &lmac->netdev, PHY_INTERFACE_MODE_SGMII);

	if (!lmac->phydev)
		return -1;

	ret = phy_config(lmac->phydev);
	if (ret) {
		printf("%s: Could not initialize PHY %s\n",
		       lmac->netdev.name, lmac->phydev->dev->name);
		return ret;
	}

	ret = phy_startup(lmac->phydev);
	if (ret) {
		printf("%s: Could not initialize PHY %s\n",
		       lmac->netdev.name, lmac->phydev->dev->name);
		return ret;
	}

	bgx_lmac_handler(&lmac->netdev);

	return 0;
}

void bgx_lmac_disable(int bgxid, uint8_t lmacid)
{
	struct lmac *lmac;
	struct bgx *bgx = bgx_vnic[bgxid];
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

static int bgx_lmac_count(struct bgx *bgx)
{
	const char *boardtype = getenv("board");
	int lmac_count = 0;

	if (!boardtype)
		return lmac_count;

	if (!strcmp(boardtype, "ebb8800")) {
		switch (bgx->qlm_mode) {
		case QLM_MODE_SGMII:
		case QLM_MODE_XFI_4X1:
		case QLM_MODE_10G_KR_4X1:
			lmac_count = 4;
		break;
		case QLM_MODE_XAUI_1X4:
		case QLM_MODE_XLAUI_1X4:
		case QLM_MODE_40G_KR4_1X4:
			lmac_count = 1;
		break;
		case QLM_MODE_RXAUI_2X2:
			lmac_count = 2;
		break;
		}
	} else if (!strcmp(boardtype, "crb_1s")) {
		switch (bgx->bgx_id) {
		case 0:
			lmac_count = 1;
			break;
		case 1:
			lmac_count = 2;
			break;
		}
	} else if (!strcmp(boardtype, "crb_2s")) {
		lmac_count = 1;
	} else {
		printf("Unsupported board\n");
		lmac_count = 0;
	}

	return lmac_count;
}

static void bgx_set_num_ports(struct bgx *bgx)
{
	switch (bgx->qlm_mode) {
	case QLM_MODE_SGMII:
		bgx->lmac_type = BGX_MODE_SGMII;
		bgx->lane_to_sds = 0;
		break;
	case QLM_MODE_XAUI_1X4:
		bgx->lmac_type = BGX_MODE_XAUI;
		bgx->lane_to_sds = 0xE4;
		break;
	case QLM_MODE_RXAUI_2X2:
		bgx->lmac_type = BGX_MODE_RXAUI;
		bgx->lane_to_sds = 0xE4;
		break;
	case QLM_MODE_XFI_4X1:
		bgx->lmac_type = BGX_MODE_XFI;
		bgx->lane_to_sds = 0;
		break;
	case QLM_MODE_XLAUI_1X4:
		bgx->lmac_type = BGX_MODE_XLAUI;
		bgx->lane_to_sds = 0xE4;
		break;
	case QLM_MODE_10G_KR_4X1:
		bgx->lmac_type = BGX_MODE_10G_KR;
		bgx->lane_to_sds = 0;
		bgx->use_training = 1;
		break;
	case QLM_MODE_40G_KR4_1X4:
		bgx->lmac_type = BGX_MODE_40G_KR;
		bgx->lane_to_sds = 0xE4;
		bgx->use_training = 1;
		break;
	default:
		break;
	}

	bgx->lmac_count = bgx_lmac_count(bgx);

	printf("BGX%d LMACs: %d\n", bgx->bgx_id, bgx->lmac_count);
}

/* Most of this will eventually need to be set by bootloader */
static void bgx_init_hw(struct bgx *bgx)
{
	int i;

	bgx_set_num_ports(bgx);

	bgx_reg_modify(bgx, 0, BGX_CMR_GLOBAL_CFG, CMR_GLOBAL_CFG_FCS_STRIP);
	if (bgx_reg_read(bgx, 0, BGX_CMR_BIST_STATUS))
		printf("BGX%d BIST failed\n", bgx->bgx_id);

	/* Set lmac type and lane2serdes mapping */
	for (i = 0; i < bgx->lmac_count; i++) {
		if (bgx->lmac_type == BGX_MODE_RXAUI) {
			if (i)
				bgx->lane_to_sds = 0x0e;
			else
				bgx->lane_to_sds = 0x04;
			bgx_reg_write(bgx, i, BGX_CMRX_CFG,
				      (bgx->lmac_type << 8) | bgx->lane_to_sds);
			continue;
		}
		bgx_reg_write(bgx, i, BGX_CMRX_CFG,
			      (bgx->lmac_type << 8) | (bgx->lane_to_sds + i));
		bgx->lmac[i].lmacid_bd = lmac_count;
		lmac_count++;
	}

	bgx_reg_write(bgx, 0, BGX_CMR_TX_LMACS, bgx->lmac_count);
	bgx_reg_write(bgx, 0, BGX_CMR_RX_LMACS, bgx->lmac_count);

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
	int lmac_type;
	int train_en;
	char fdt_key[20];

	/* Read LMAC0 type to figure out QLM mode
	 * This is configured by low level firmware
	 */

	lmac_type = bgx_reg_read(bgx, 0, BGX_CMRX_CFG);
	lmac_type = (lmac_type >> 8) & 0x07;

	train_en = bgx_reg_read(bgx, 0, BGX_SPUX_BR_PMD_CRTL) &
	    SPU_PMD_CRTL_TRAIN_EN;

	switch (lmac_type) {
	case BGX_MODE_SGMII:
		sprintf(fdt_key, "%d,sgmii", bgx->bgx_id);
		bgx_fdt_patch(fdt_key);
		bgx->qlm_mode = QLM_MODE_SGMII;
		printf("BGX%d QLM mode: SGMII\n", bgx->bgx_id);
		break;
	case BGX_MODE_XAUI:
		printf(fdt_key, "%d,xaui", bgx->bgx_id);
		bgx_fdt_patch(fdt_key);
		bgx->qlm_mode = QLM_MODE_XAUI_1X4;
		printf("BGX%d QLM mode: XAUI\n", bgx->bgx_id);
		break;
	case BGX_MODE_RXAUI:
		sprintf(fdt_key, "%d,rxaui", bgx->bgx_id);
		bgx_fdt_patch(fdt_key);
		bgx->qlm_mode = QLM_MODE_RXAUI_2X2;
		printf("BGX%d QLM mode: RXAUI\n", bgx->bgx_id);
		break;
	case BGX_MODE_XFI:
		if (!train_en) {
			sprintf(fdt_key, "%d,xfi", bgx->bgx_id);
			bgx_fdt_patch(fdt_key);
			bgx->qlm_mode = QLM_MODE_XFI_4X1;
			printf("BGX%d QLM mode: XFI\n", bgx->bgx_id);
		} else {
			sprintf(fdt_key, "%d,xfi-10g-kr", bgx->bgx_id);
			bgx_fdt_patch(fdt_key);
			bgx->qlm_mode = QLM_MODE_10G_KR_4X1;
			printf("BGX%d QLM mode: 10G_KR\n", bgx->bgx_id);
		}
		break;
	case BGX_MODE_XLAUI:
		if (!train_en) {
			sprintf(fdt_key, "%d,xlaui", bgx->bgx_id);
			bgx_fdt_patch(fdt_key);
			bgx->qlm_mode = QLM_MODE_XLAUI_1X4;
			printf("BGX%d QLM mode: XLAUI\n", bgx->bgx_id);
		} else {
			sprintf(fdt_key, "%d,xlaui-40g-kr", bgx->bgx_id);
			bgx_fdt_patch(fdt_key);
			bgx->qlm_mode = QLM_MODE_40G_KR4_1X4;
			printf("BGX%d QLM mode: 40G_KR4\n", bgx->bgx_id);
		}
		break;
	}
}

int thunderx_bgx_initialize(unsigned int bgx_idx,
			    unsigned int smi_idx, unsigned int node)
{
	int err;
	struct bgx *bgx = NULL;
	uint8_t lmac = 0;
	char mii_name[10];

	bgx = malloc(sizeof(struct bgx));

	/* MAP configuration registers */
	bgx->reg_base = CSR_PA(BGXX_PF_BAR0(bgx_idx), node);
	bgx->bgx_id = bgx_idx + node * MAX_BGX_PER_CN88XX;

	bgx_vnic[bgx->bgx_id] = bgx;
	debug("bgx_vnic[%u]: %p\n", bgx->bgx_id, bgx);

	snprintf(mii_name, sizeof(mii_name), "thunderx%d", smi_idx);

	for (lmac = 0; lmac < 4; lmac++) {
		bgx->lmac[lmac].lmacid_bd = bgx->bgx_id;
		bgx->lmac[lmac].lmacid = lmac + bgx->bgx_id * 4;
		bgx->lmac[lmac].mii_bus = miiphy_get_dev_by_name(mii_name);

		bgx->lmac_count++;
	}

	bgx_get_qlm_mode(bgx);

	bgx_init_hw(bgx);

	return 0;

	free(bgx);
	return err;
}
