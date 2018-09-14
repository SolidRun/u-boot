/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#ifndef OCTEONT_BGX_H
#define OCTEONT_BGX_H

#define    MAX_LMAC_PER_BGX			4
#define    MAX_BGX_CHANS_PER_LMAC		16
#define    MAX_DMAC_PER_LMAC			8
#define    MAX_FRAME_SIZE			9216

#define    MAX_DMAC_PER_LMAC_TNS_BYPASS_MODE	2

#define    MAX_LMAC	(CONFIG_MAX_BGX_PER_NODE * MAX_LMAC_PER_BGX)

#define    NODE_ID_MASK				0x300000000000
#define    NODE_ID(x)				((x & NODE_ID_MASK) >> 44)

/* Registers */
#define GSERX_CFG(x)		(0x000087E090000080ull + (x) * 0x1000000ull)
#define GSERX_SCRATCH(x)	(0x000087E090000020ull + (x) * 0x1000000ull)
#define GSERX_PHY_CTL(x)	(0x000087E090000000ull + (x) * 0x1000000ull)
#define GSERX_CFG_BGX		(1 << 2)
#define GSER_RX_EIE_DETSTS(x)	(0x000087e090000150ull + (x) * 0x1000000ull)
#define GSER_CDRLOCK		(8)
#define GSER_BR_RXX_CTL(x,y)	(0x000087e090000400ull + (x) * 0x1000000ull + (y) * 0x80) 
#define GSER_BR_RXX_CTL_RXT_SWM	(1 << 2)
#define GSER_BR_RXX_EER(x,y)	(0x000087e090000418ull + (x) * 0x1000000ull + (y) * 0x80)
#define GSER_BR_RXX_EER_RXT_ESV (1 << 14)
#define GSER_BR_RXX_EER_RXT_EER (1 << 15)
#define EER_RXT_ESV		(14)

#define BGX_CMRX_CFG			0x00
#define CMR_PKT_TX_EN				(1ull << 13)
#define CMR_PKT_RX_EN				(1ull << 14)
#define CMR_EN					(1ull << 15)
#define BGX_CMR_GLOBAL_CFG		0x08
#define CMR_GLOBAL_CFG_FCS_STRIP		(1ull << 6)
#define BGX_CMRX_RX_ID_MAP		0x60
#define BGX_CMRX_RX_STAT0		0x70
#define BGX_CMRX_RX_STAT1		0x78
#define BGX_CMRX_RX_STAT2		0x80
#define BGX_CMRX_RX_STAT3		0x88
#define BGX_CMRX_RX_STAT4		0x90
#define BGX_CMRX_RX_STAT5		0x98
#define BGX_CMRX_RX_STAT6		0xA0
#define BGX_CMRX_RX_STAT7		0xA8
#define BGX_CMRX_RX_STAT8		0xB0
#define BGX_CMRX_RX_STAT9		0xB8
#define BGX_CMRX_RX_STAT10		0xC0
#define BGX_CMRX_RX_BP_DROP		0xC8
#define BGX_CMRX_RX_DMAC_CTL		0x0E8
#define BGX_CMR_RX_DMACX_CAM		0x200
#define RX_DMACX_CAM_EN				(1ull << 48)
#define RX_DMACX_CAM_LMACID(x)			(x << 49)
#define RX_DMAC_COUNT				32
#define BGX_CMR_RX_STREERING		0x300
#define RX_TRAFFIC_STEER_RULE_COUNT		8
#define BGX_CMR_CHAN_MSK_AND		0x450
#define BGX_CMR_BIST_STATUS		0x460
#define BGX_CMR_RX_LMACS		0x468
#define BGX_CMRX_TX_STAT0		0x600
#define BGX_CMRX_TX_STAT1		0x608
#define BGX_CMRX_TX_STAT2		0x610
#define BGX_CMRX_TX_STAT3		0x618
#define BGX_CMRX_TX_STAT4		0x620
#define BGX_CMRX_TX_STAT5		0x628
#define BGX_CMRX_TX_STAT6		0x630
#define BGX_CMRX_TX_STAT7		0x638
#define BGX_CMRX_TX_STAT8		0x640
#define BGX_CMRX_TX_STAT9		0x648
#define BGX_CMRX_TX_STAT10		0x650
#define BGX_CMRX_TX_STAT11		0x658
#define BGX_CMRX_TX_STAT12		0x660
#define BGX_CMRX_TX_STAT13		0x668
#define BGX_CMRX_TX_STAT14		0x670
#define BGX_CMRX_TX_STAT15		0x678
#define BGX_CMRX_TX_STAT16		0x680
#define BGX_CMRX_TX_STAT17		0x688
#define BGX_CMR_TX_LMACS		0x1000

#define BGX_SPUX_CONTROL1		0x10000
#define SPU_CTL_LOW_POWER			(1ull << 11)
#define SPU_CTL_LOOPBACK                        (1ull << 14)
#define SPU_CTL_RESET				(1ull << 15)
#define BGX_SPUX_STATUS1		0x10008
#define SPU_STATUS1_RCV_LNK			(1ull << 2)
#define BGX_SPUX_STATUS2		0x10020
#define SPU_STATUS2_RCVFLT			(1ull << 10)
#define BGX_SPUX_BX_STATUS		0x10028
#define SPU_BX_STATUS_RX_ALIGN                  (1ull << 12)
#define BGX_SPUX_BR_STATUS1		0x10030
#define SPU_BR_STATUS_BLK_LOCK			(1ull << 0)
#define SPU_BR_STATUS_RCV_LNK			(1ull << 12)
#define BGX_SPUX_BR_PMD_CRTL		0x10068
#define SPU_PMD_CRTL_TRAIN_EN			(1ull << 1)
#define BGX_SPUX_BR_PMD_LP_CUP		0x10078
#define BGX_SPUX_BR_PMD_LD_CUP		0x10088
#define BGX_SPUX_BR_PMD_LD_REP		0x10090
#define BGX_SPUX_FEC_CONTROL		0x100A0
#define SPU_FEC_CTL_FEC_EN			(1ull << 0)
#define SPU_FEC_CTL_ERR_EN			(1ull << 1)
#define BGX_SPUX_AN_CONTROL		0x100C8
#define SPU_AN_CTL_AN_EN			(1ull << 12)
#define SPU_AN_CTL_XNP_EN			(1ull << 13)
#define SPU_AN_CTL_AN_RESTART			(1ull << 15)
#define BGX_SPUX_AN_STATUS		0x100D0
#define SPU_AN_STS_AN_COMPLETE			(1ull << 5)
#define BGX_SPUX_AN_ADV			0x100D8
#define BGX_SPUX_MISC_CONTROL		0x10218
#define SPU_MISC_CTL_INTLV_RDISP		(1ull << 10)
#define SPU_MISC_CTL_RX_DIS			(1ull << 12)
#define BGX_SPUX_INT			0x10220	/* +(0..3) << 20 */
#define BGX_SPUX_INT_W1S		0x10228
#define BGX_SPUX_INT_ENA_W1C		0x10230
#define BGX_SPUX_INT_ENA_W1S		0x10238
#define BGX_SPU_DBG_CONTROL		0x10300
#define SPU_DBG_CTL_AN_ARB_LINK_CHK_EN		(1ull << 18)
#define SPU_DBG_CTL_AN_NONCE_MCT_DIS		(1ull << 29)

#define BGX_SMUX_RX_INT			0x20000
#define BGX_SMUX_RX_JABBER		0x20030
#define BGX_SMUX_RX_CTL			0x20048
#define SMU_RX_CTL_STATUS			(3ull << 0)
#define BGX_SMUX_TX_APPEND		0x20100
#define SMU_TX_APPEND_FCS_D			(1ull << 2)
#define BGX_SMUX_TX_MIN_PKT		0x20118
#define BGX_SMUX_TX_INT			0x20140
#define BGX_SMUX_TX_CTL			0x20178
#define SMU_TX_CTL_DIC_EN			(1ull << 0)
#define SMU_TX_CTL_UNI_EN			(1ull << 1)
#define SMU_TX_CTL_LNK_STATUS			(3ull << 4)
#define BGX_SMUX_TX_THRESH		0x20180
#define BGX_SMUX_CTL			0x20200
#define SMU_CTL_RX_IDLE				(1ull << 0)
#define SMU_CTL_TX_IDLE				(1ull << 1)

#define BGX_GMP_PCS_MRX_CTL		0x30000
#define	PCS_MRX_CTL_RST_AN			(1ull << 9)
#define	PCS_MRX_CTL_PWR_DN			(1ull << 11)
#define	PCS_MRX_CTL_AN_EN			(1ull << 12)
#define PCS_MRX_CTL_LOOPBACK1                   (1ull << 14)
#define	PCS_MRX_CTL_RESET			(1ull << 15)
#define BGX_GMP_PCS_MRX_STATUS		0x30008
#define	PCS_MRX_STATUS_AN_CPT			(1ull << 5)
#define BGX_GMP_PCS_ANX_AN_RESULTS	0x30020
#define BGX_GMP_PCS_SGM_AN_ADV		0x30068
#define BGX_GMP_PCS_MISCX_CTL		0x30078
#define PCS_MISCX_CTL_DISP_EN			(1ull << 13)
#define PCS_MISC_CTL_GMX_ENO			(1ull << 11)
#define PCS_MISC_CTL_SAMP_PT_MASK		0x7Full
#define BGX_GMP_GMI_PRTX_CFG		0x38020
#define GMI_PORT_CFG_SPEED			(1ull << 1)
#define GMI_PORT_CFG_DUPLEX			(1ull << 2)
#define GMI_PORT_CFG_SLOT_TIME			(1ull << 3)
#define GMI_PORT_CFG_SPEED_MSB			(1ull << 8)
#define BGX_GMP_GMI_RXX_JABBER		0x38038
#define BGX_GMP_GMI_TXX_THRESH		0x38210
#define BGX_GMP_GMI_TXX_APPEND		0x38218
#define BGX_GMP_GMI_TXX_SLOT		0x38220
#define BGX_GMP_GMI_TXX_BURST		0x38228
#define BGX_GMP_GMI_TXX_MIN_PKT		0x38240
#define BGX_GMP_GMI_TXX_SGMII_CTL	0x38300

#define BGX_MSIX_VEC_0_29_ADDR		0x400000 /* +(0..29) << 4 */
#define BGX_MSIX_VEC_0_29_CTL		0x400008
#define BGX_MSIX_PBA_0			0x4F0000

/* MSI-X interrupts */
#define BGX_MSIX_VECTORS	30
#define BGX_LMAC_VEC_OFFSET	7
#define BGX_MSIX_VEC_SHIFT	4

#define CMRX_INT		0
#define SPUX_INT		1
#define SMUX_RX_INT		2
#define SMUX_TX_INT		3
#define GMPX_PCS_INT		4
#define GMPX_GMI_RX_INT		5
#define GMPX_GMI_TX_INT		6
#define CMR_MEM_INT		28
#define SPU_MEM_INT		29

#define LMAC_INTR_LINK_UP	(1 << 0)
#define LMAC_INTR_LINK_DOWN	(1 << 1)

/*  RX_DMAC_CTL configuration*/
enum MCAST_MODE {
		MCAST_MODE_REJECT,
		MCAST_MODE_ACCEPT,
		MCAST_MODE_CAM_FILTER,
		RSVD
};

#define BCAST_ACCEPT	1
#define CAM_ACCEPT	1

int octeontx_bgx_initialize(unsigned int bgx_idx, unsigned int node);
void bgx_add_dmac_addr(uint64_t dmac, int node, int bgx_idx, int lmac);
void bgx_get_count(int node, int *bgx_count);
int bgx_get_lmac_count(int node, int bgx);
void bgx_print_stats(int bgx_idx, int lmac);
void xcv_init_hw(void);
void xcv_setup_link(bool link_up, int link_speed);

#undef LINK_INTR_ENABLE

enum qlm_mode {
	QLM_MODE_SGMII,         /* SGMII, each lane independent */
	QLM_MODE_XAUI,      /* 1 XAUI or DXAUI, 4 lanes */
	QLM_MODE_RXAUI,     /* 2 RXAUI, 2 lanes each */
	QLM_MODE_XFI,       /* 4 XFI, 1 lane each */
	QLM_MODE_XLAUI,     /* 1 XLAUI, 4 lanes each */
	QLM_MODE_10G_KR,    /* 4 10GBASE-KR, 1 lane each */
	QLM_MODE_40G_KR4,   /* 1 40GBASE-KR4, 4 lanes each */
	QLM_MODE_QSGMII,    /* 4 QSGMII, each lane independent */
	QLM_MODE_RGMII,     /* 1 RGX */
};

struct phy_info {
	int mdio_bus;
	int phy_addr;
	bool autoneg_dis;
};

struct bgx_board_info {
	struct phy_info phy_info[MAX_LMAC_PER_BGX];
	bool lmac_reg[MAX_LMAC_PER_BGX];
	bool lmac_enable[MAX_LMAC_PER_BGX];
};

enum LMAC_TYPE {
	BGX_MODE_SGMII = 0, /* 1 lane, 1.250 Gbaud */
	BGX_MODE_XAUI = 1,  /* 4 lanes, 3.125 Gbaud */
	BGX_MODE_DXAUI = 1, /* 4 lanes, 6.250 Gbaud */
	BGX_MODE_RXAUI = 2, /* 2 lanes, 6.250 Gbaud */
	BGX_MODE_XFI = 3,   /* 1 lane, 10.3125 Gbaud */
	BGX_MODE_XLAUI = 4, /* 4 lanes, 10.3125 Gbaud */
	BGX_MODE_10G_KR = 3,/* 1 lane, 10.3125 Gbaud */
	BGX_MODE_40G_KR = 4,/* 4 lanes, 10.3125 Gbaud */
	BGX_MODE_RGMII = 5,
	BGX_MODE_QSGMII = 6,
	BGX_MODE_INVALID = 7,
};

#endif /* OCTEONT_BGX_H */
