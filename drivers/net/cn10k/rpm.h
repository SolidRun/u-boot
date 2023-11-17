/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __RPM_H__
#define __RPM_H__

#include <asm/arch/csrs/csrs-rpm.h>

#define MAX_RPM			9

#define RPM_BAR(x)		RPM_BAR_E_RPMX_PF_BAR0(x)

/* MACSEC Block */
#define MCS_BASE		0x87e080000000

#define MCS_MIL_GLOBAL		0x80000
#define MCS_MIL_GLOBAL_CLB_X2P	BIT_ULL(5)

#define MCS_MIL_RX_GBL_STS	0x800c8
#define MCS_MIL_RX_GBL_CLB_STS	GENMASK_ULL(16, 1)
#define MCS_MIL_RX_GBL_CLB_DONE	BIT_ULL(0)

#define MCS_LINK_LMACX_CFG(x)	0x90000 + (0x800 * (x))

enum lmac_type {
	LMAC_MODE_SGMII		= 0,
	LMAC_MODE_XAUI		= 1,
	LMAC_MODE_RXAUI		= 2,
	LMAC_MODE_10G_R		= 3,
	LMAC_MODE_40G_R		= 4,
	LMAC_MODE_QSGMII	= 6,
	LMAC_MODE_25G_R		= 7,
	LMAC_MODE_50G_R		= 8,
	LMAC_MODE_100G_R	= 9,
	LMAC_MODE_USXGMII	= 10,
	LMAC_MODE_USGMII	= 11,
	LMAC_MODE_MAX		= 12,
};

enum p2x_select {
	P2X1_NIX0 = 1,
	P2X2_NIX1 = 2,
};

extern char lmac_type_to_str[][8];

extern char lmac_speed_to_str[][8];

struct lmac_priv {
	u8 enable:1;
	u8 full_duplex:1;
	u8 speed:4;
	u8 mode:1;
	u8 rsvd:1;
	u8 mac_addr[6];
};

struct rpm;
struct nix;
struct nix_af;

struct lmac {
	struct rpm	*rpm;
	struct nix	*nix;
	char		name[16];
	enum lmac_type	lmac_type;
	enum p2x_select p2x_sel;
	bool		init_pend;
	u8		instance;
	u8		lmac_id;
	u8		pknd;
	u8		link_num;
	u32		chan_num;
	u8		mac_addr[6];
};

struct rpm {
	struct nix_af		*nix_af;
	void __iomem		*reg_base;
	struct udevice		*dev;
#define LMAC_LIMIT 8
	struct lmac		*lmac[LMAC_LIMIT];
	u8			rpm_id;
	u8			lmac_count;
	u8			max_lmac;
	u8			is_v2;
};

static inline void rpm_write(struct rpm *rpm, u64 offset, u64 val)
{
	writeq(val, rpm->reg_base + offset);
}

static inline u64 rpm_read(struct rpm *rpm, u64 offset)
{
	return readq(rpm->reg_base + offset);
}

/* SH FWDATA Structure Definitions */
struct sfp_eeprom_s {
#define SFP_EEPROM_SIZE 256
	u16 sff_id;
	u8 buf[SFP_EEPROM_SIZE];
	u64 reserved;
};

struct phy_s {
	struct {
		u64 can_change_mod_type : 1;
		u64 mod_type            : 1;
		u64 has_fec_stats       : 1;
	} misc;
	struct {
		u32 rsfec_corr_cws;
		u32 rsfec_uncorr_cws;
		u32 brfec_corr_blks;
		u32 brfec_uncorr_blks;
	} fec_stats;
};

struct eth_lmac_fwdata_s {
	/* RO to kernel. FW to set rw_valid as 0 when updating this struct
	 * indicating data is invalid. After copying the data, this bit needs
	 * to be set as 1. only when this bit is 1, kernel should
	 * read this info
	 */
	u16 rw_valid;
	u64 supported_fec;
	u64 supported_an;
	u64 supported_link_modes;
	/* only applicable if AN is supported */
	u64 advertised_fec;
	u64 advertised_link_modes_own:1; /* eth_cmd_own */
	u64 advertised_link_modes:63; /* RO to firmware */
	/* Only applicable if SFP/QSFP slot is present */
	struct sfp_eeprom_s sfp_eeprom;
	struct phy_s phy;
	/* LMAC type updated with CSR macro CAVM_RPM_LMAC_TYPES_E_* */
	u32 lmac_type;
	u32 portm_idx;
	u64 mgmt_port:1;
#define LMAC_FWDATA_RESERVED_MEM 1019
	u64 reserved[LMAC_FWDATA_RESERVED_MEM];

};

/* sh_fwdata to be synced with linux/drivers/net/ethernet/marvell/octeontx2/af/rvu.h */
struct sh_fwdata {
#define SH_FWDATA_HEADER_MAGIC	0xCFDA	/*Custom Firmware Data*/
#define SH_FWDATA_VERSION	0x0001
	u32 header_magic;
	u32 version;		/* version id */

	/* MAC address */
#define PF_MACNUM_MAX	32
#define VF_MACNUM_MAX	256
	u64 pf_macs[PF_MACNUM_MAX];
	u64 vf_macs[VF_MACNUM_MAX];
	u64 sclk;	/* In MHZ */
	u64 coreclk; /* In MHZ */
	u64 mcam_addr;
	u64 mcam_sz;
	u64 rvu_af_msixtr_base;
	u32 ptp_ext_clk_rate;
	u32 ptp_ext_tstamp;
 #define FWDATA_RESERVED_MEM 1022
	u64 reserved[FWDATA_RESERVED_MEM];
	/* Do not add new fields below this line */
#define ETH_MAX		9
#define ETH_LMACS_MAX	4
#define ETH_LMACS_USX   8       /* Applicable for CN10KB */
	union {
		struct eth_lmac_fwdata_s eth_fw_data[ETH_MAX][ETH_LMACS_MAX];
		struct eth_lmac_fwdata_s eth_fw_data_usx[ETH_MAX][ETH_LMACS_USX];
	};
};

/**
 * Given an LMAC/PF instance number, return the lmac
 * Per design, each PF has only one LMAC mapped.
 *
 * @param instance	instance to find
 *
 * @return	pointer to lmac data structure or NULL if not found
 */
struct lmac *nix_get_rpm_lmac(int lmac_instance);

int rpm_lmac_set_chan(struct lmac *lmac);
int rpm_lmac_set_pkind(struct lmac *lmac, u8 lmac_id, int pkind);
int rpm_lmac_internal_loopback(struct lmac *lmac, int lmac_id, bool enable);
int rpm_lmac_rx_tx_enable(struct lmac *lmac, int lmac_id, bool enable);
int rpm_lmac_link_enable(struct lmac *lmac, int lmac_id, bool enable,
			 u64 *status);
int rpm_lmac_link_status(struct lmac *lmac, int lmac_id, u64 *status);
void rpm_lmac_mac_filter_setup(struct lmac *lmac);

int eth_intf_get_link_sts(u8 rpm, u8 lmac, u64 *lnk_sts);
int eth_intf_link_up_dwn(u8 rpm, u8 lmac, u8 up_dwn, u64 *lnk_sts);
int eth_intf_get_mac_addr(u8 rpm, u8 lmac, u8 *mac);
int eth_intf_set_macaddr(struct udevice *dev);
int eth_intf_prbs(u8 qlm, u8 mode, u32 time, u8 lane);
int eth_intf_display_eye(u8 qlm, u8 lane);
int eth_intf_display_serdes(u8 qlm, u8 lane);

#endif /* __RPM_H__ */
