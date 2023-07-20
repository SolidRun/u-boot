/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef __ETH_INTF_H__
#define __ETH_INTF_H__

#define ETH_FIRMWARE_MAJOR_VER		1
#define ETH_FIRMWARE_MINOR_VER		2

/* ETH error types. set for cmd response status as ETH_STAT_FAIL */
enum eth_error_type {
	ETH_ERR_NONE = 0,
	ETH_ERR_LMAC_NOT_ENABLED,
	ETH_ERR_LMAC_MODE_INVALID,
	ETH_ERR_REQUEST_ID_INVALID,
	ETH_ERR_PREV_ACK_NOT_CLEAR,
	ETH_ERR_PHY_LINK_DOWN,		/* = 5 */
	ETH_ERR_PCS_RESET_FAIL,
	ETH_ERR_AN_CPT_FAIL,
	ETH_ERR_TX_NOT_IDLE,
	ETH_ERR_RX_NOT_IDLE,
	ETH_ERR_SPUX_BR_BLKLOCK_FAIL,	/* = 10 */
	ETH_ERR_SPUX_RX_ALIGN_FAIL,
	ETH_ERR_SPUX_TX_FAULT,
	ETH_ERR_SPUX_RX_FAULT,
	ETH_ERR_SPUX_RESET_FAIL,
	ETH_ERR_SPUX_AN_RESET_FAIL,	/* = 15 */
	ETH_ERR_SPUX_USX_AN_RESET_FAIL,
	ETH_ERR_SMUX_RX_LINK_NOT_OK,
	ETH_ERR_PCS_LINK_FAIL,
	ETH_ERR_TRAINING_FAIL,
	ETH_ERR_RX_EQU_FAIL,		/* = 20 */
	ETH_ERR_SPUX_BER_FAIL,
	ETH_ERR_SPUX_RSFEC_ALGN_FAIL,
	ETH_ERR_SPUX_MARKER_LOCK_FAIL,
	ETH_ERR_SET_FEC_INVALID,
	ETH_ERR_SET_FEC_FAIL,		/* = 25 */
	ETH_ERR_MODULE_INVALID,
	ETH_ERR_MODULE_NOT_PRESENT,
	ETH_ERR_SPEED_CHANGE_INVALID,
	ETH_ERR_SERDES_RX_NO_SIGNAL,
	ETH_ERR_SERDES_CPRI_PARAM_INVALID,	/* = 30 */
	ETH_ERR_ECP_LINK_REQ_FAIL,
	ETH_ERR_LPCS_INTERNAL_LBK_INVALID,
	/* FIXME : add more error types when adding support for new modes */
};

/* LINK speed types */
enum eth_link_speed {
	ETH_LINK_NONE,
	ETH_LINK_10M,
	ETH_LINK_100M,
	ETH_LINK_1G,
	ETH_LINK_2HG,	/* 2.5 Gbps */
	ETH_LINK_5G,
	ETH_LINK_10G,
	ETH_LINK_20G,
	ETH_LINK_25G,
	ETH_LINK_40G,
	ETH_LINK_50G,
	ETH_LINK_80G,
	ETH_LINK_100G,
	ETH_LINK_MAX,
};

/* REQUEST ID types. Input to firmware */
enum eth_cmd_id {
	ETH_CMD_NONE = 0,
	ETH_CMD_GET_FW_VER,
	ETH_CMD_GET_MAC_ADDR,
	ETH_CMD_SET_MTU,
	ETH_CMD_GET_LINK_STS,		/* optional to user */
	ETH_CMD_LINK_BRING_UP,		/* = 5 */
	ETH_CMD_LINK_BRING_DOWN,
	ETH_CMD_INTERNAL_LBK,
	ETH_CMD_EXTERNAL_LBK,
	ETH_CMD_HIGIG,
	ETH_CMD_LINK_STAT_CHANGE,	/* = 10 */
	ETH_CMD_MODE_CHANGE,		/* hot plug support */
	ETH_CMD_INTF_SHUTDOWN,
	ETH_CMD_GET_MKEX_SIZE,
	ETH_CMD_GET_MKEX_PROFILE,
	ETH_CMD_GET_FWD_BASE,		/* get base address of shared FW data */
	ETH_CMD_GET_LINK_MODES,		/* Supported Link Modes */
	ETH_CMD_SET_LINK_MODE,
	ETH_CMD_GET_SUPPORTED_FEC,
	ETH_CMD_SET_FEC,
	ETH_CMD_GET_AN,			/* = 20 */  /* Not Implemented */
	ETH_CMD_SET_AN,				    /* Not Implemented */
	ETH_CMD_GET_ADV_LINK_MODES,
	ETH_CMD_GET_ADV_FEC,
	ETH_CMD_GET_PHY_MOD_TYPE, /* line-side modulation type: NRZ or PAM4 */
	ETH_CMD_SET_PHY_MOD_TYPE,	/* = 25 */
	ETH_CMD_RESERVED1,
	ETH_CMD_RESERVED2,
	ETH_CMD_GET_PHY_FEC_STATS,
	ETH_CMD_RESERVED3,
	ETH_CMD_AN_LOOPBACK,	/* = 30 */
	ETH_CMD_GET_PERSIST_IGNORE,
	ETH_CMD_SET_PERSIST_IGNORE,
	ETH_CMD_SET_MAC_ADDR,
	ETH_CMD_SET_PTP_MODE,
	ETH_CMD_CPRI_MODE_CHANGE, /* Only supported for T9x */	/* = 35 */
	ETH_CMD_CPRI_TX_CONTROL, /* Only supported for T9x */
	ETH_CMD_LOOP_SERDES,
	ETH_CMD_TUNE_SERDES,
	ETH_CMD_LEQ_ADAPT_SERDES, /* Only supported for T9x */
	ETH_CMD_DFE_ADAPT_SERDES, /* Only supported for T9x */	/* = 40 */
	ETH_CMD_DO_CMU_RESET,	/* Only supported for T9x */
	ETH_CMD_CPRI_MISC,      /* Only supported for T9x */
	ETH_CMD_LINK_TIMEOUT,
	ETH_CMD_GET_PORT_MODE,	/* Only supported for cn10k */
	ETH_CMD_ECP_DUMP_STATE, /* Only supported for cn10k */  /* = 45 */
	ETH_CMD_STOP_TIMERS, /* For UEFI SystemReady test */
};

/* async event ids */
enum eth_evt_id {
	ETH_EVT_NONE,
	ETH_EVT_LINK_CHANGE,
};

/* event types - cause of interrupt */
enum eth_evt_type {
	ETH_EVT_ASYNC,
	ETH_EVT_CMD_RESP
};

enum eth_stat {
	ETH_STAT_SUCCESS,
	ETH_STAT_FAIL
};

enum eth_cmd_own {
	/* default ownership with kernel/uefi/u-boot */
	ETH_OWN_NON_SECURE_SW,
	/* set by kernel/uefi/u-boot after posting a new request to ATF */
	ETH_OWN_FIRMWARE,
};

/* Supported LINK MODE enums
 * Each link mode is a bit mask of these
 * enums which are represented as bits
 */
typedef enum {
	ETH_MODE_SGMII_BIT = 0,
	ETH_MODE_1000_BASEX_BIT,
	ETH_MODE_QSGMII_BIT,
	ETH_MODE_10G_C2C_BIT,
	ETH_MODE_10G_C2M_BIT,
	ETH_MODE_10G_KR_BIT,		/* = 5 */
	ETH_MODE_20G_C2C_BIT,
	ETH_MODE_25G_C2C_BIT,
	ETH_MODE_25G_C2M_BIT,
	ETH_MODE_25G_2_C2C_BIT,
	ETH_MODE_25G_CR_BIT,		/* = 10 */
	ETH_MODE_25G_KR_BIT,
	ETH_MODE_40G_C2C_BIT,
	ETH_MODE_40G_C2M_BIT,
	ETH_MODE_40G_CR4_BIT,
	ETH_MODE_40G_KR4_BIT,		/* = 15 */
	ETH_MODE_40GAUI_C2C_BIT,
	ETH_MODE_50G_C2C_BIT,		/* single lane 50G */
	ETH_MODE_50G_C2M_BIT,
	ETH_MODE_50G_4_C2C_BIT,
	ETH_MODE_50G_CR_BIT,		/* = 20 */
	ETH_MODE_50G_KR_BIT,
	ETH_MODE_80GAUI_C2C_BIT,
	ETH_MODE_100G_C2C_BIT,		/* four lanes 100G */
	ETH_MODE_100G_C2M_BIT,
	ETH_MODE_100G_CR4_BIT,		/* = 25 */
	ETH_MODE_100G_KR4_BIT,
	ETH_MODE_50GAUI_2_C2C_BIT,	/* two lanes 50G */
	ETH_MODE_50GAUI_2_C2M_BIT,
	ETH_MODE_50GBASE_CR2_C_BIT,
	ETH_MODE_50GBASE_KR2_C_BIT,	/* = 30 */
	ETH_MODE_100GAUI_2_C2C_BIT,	/* two lanes 100G */
	ETH_MODE_100GAUI_2_C2M_BIT,
	ETH_MODE_100GBASE_CR2_BIT,
	ETH_MODE_100GBASE_KR2_BIT,
	ETH_MODE_SFI_1G_BIT,		/* = 35 */
	ETH_MODE_25GBASE_CR_C_BIT,
	ETH_MODE_25GBASE_KR_C_BIT,	/* = 37 */
	/* Add new ethernet modes here */
	ETH_MODE_MAX_BIT = 41,       /* = 41 */
	/* The below modes are applicable only for T103/T102 */
	ETH_MODE_2500_BASEX_BIT = 42,    /* Start from 42 to indicate Mode group 1 */
	ETH_MODE_5000_BASEX_BIT,
	ETH_MODE_O_USGMII_BIT,
	ETH_MODE_Q_USGMII_BIT,		/* = 45 */
	ETH_MODE_2_5G_USXGMII_BIT,
	ETH_MODE_5G_USXGMII_BIT,
	ETH_MODE_10G_SXGMII_BIT,
	ETH_MODE_10G_DXGMII_BIT,
	ETH_MODE_10G_QXGMII_BIT,	/* = 50 */
	/* Add new ethernet modes here */
	ETH_MODE_MAX_GROUP2_BIT,        /* = 83 */
} eth_mode_t;

/* Supported CPRI modes */
typedef enum {
	ETH_MODE_CPRI_2_4G_BIT,
	ETH_MODE_CPRI_3_1G_BIT,
	ETH_MODE_CPRI_4_9G_BIT,
	ETH_MODE_CPRI_6_1G_BIT,
	ETH_MODE_CPRI_9_8G_BIT,

	ETH_MODE_CPRI_2_4G_TEST_BIT,
	ETH_MODE_CPRI_3_1G_TEST_BIT,
	ETH_MODE_CPRI_4_9G_TEST_BIT,
	ETH_MODE_CPRI_6_1G_TEST_BIT,
	ETH_MODE_CPRI_9_8G_TEST_BIT,
	ETH_MODE_CPRI_12_3G_TEST_BIT,
	ETH_MODE_CPRI_19_7G_TEST_BIT,
} eth_cpri_mode_t;

typedef enum {
	MODE_GROUP_ETH0,		/* Groups 0 and 1 are reserved for ethernet */
	MODE_GROUP_ETH1,		/* Groups 0 and 1 are reserved for ethernet */
	MODE_GROUP_CPRI = 2,
} mode_group_t;

#define ETH_ALL_SUPPORTED_MODES 0xFFFFFFFFFFFFFFFF

/* scratchx(0) CSR used for ATF->non-secure SW communication.
 * This acts as the status register
 * Provides details on command ack/status, link status, error details
 */

/* CAUTION : below structures are placed in order based on the bit positions
 * For any updates/new bitfields, corresponding structures needs to be updated
 */
struct eth_evt_sts_s {			/* start from bit 0 */
	u64 ack:1;
	u64 evt_type:1;		/* eth_evt_type */
	u64 stat:1;		/* eth_stat */
	u64 id:6;			/* eth_evt_id/eth_cmd_id */
	u64 reserved:55;
};

/* all the below structures are in the same memory location of SCRATCHX(0)
 * value can be read/written based on command ID
 */

/* Resp to command IDs with command status as ETH_STAT_FAIL
 * Not applicable for commands :
 *	ETH_CMD_LINK_BRING_UP/DOWN/ETH_EVT_LINK_CHANGE
 *	check struct eth_lnk_sts_s comments
 */
struct eth_err_sts_s {			/* start from bit 9 */
	u64 reserved1:9;
	u64 type:10;		/* eth_error_type */
	u64 reserved2:35;
};

/* Resp to cmd ID as ETH_CMD_GET_FW_VER with cmd status as ETH_STAT_SUCCESS */
struct eth_ver_s {			/* start from bit 9 */
	u64 reserved1:9;
	u64 major_ver:4;
	u64 minor_ver:4;
	u64 reserved2:47;
};

/* Resp to cmd ID as ETH_CMD_GET_MAC_ADDR with cmd status as ETH_STAT_SUCCESS
 * Returns each byte of MAC address in a separate bit field
 */
struct eth_mac_addr_s {			/* start from bit 9 */
	u64 reserved1:9;
	u64 addr_0:8;
	u64 addr_1:8;
	u64 addr_2:8;
	u64 addr_3:8;
	u64 addr_4:8;
	u64 addr_5:8;
	u64 reserved2:7;
};

#ifdef NT_FW_CONFIG
struct eth_mcam_profile_addr_s {
	u64 reserved1:9; /* start from bit 9 */
	u64 mcam_addr:55;
};

struct eth_mcam_profile_sz_s {
	u64 reserved1:9; /* start from bit 9 */
	u64 mcam_sz:55;
};
#endif

/* Resp to cmd ID - ETH_CMD_LINK_BRING_UP/DOWN, event ID ETH_EVT_LINK_CHANGE
 * status can be either ETH_STAT_FAIL or ETH_STAT_SUCCESS
 * In case of ETH_STAT_FAIL, it indicates ETH configuration failed when
 * processing link up/down/change command. Both err_type and current link status
 * will be updated
 * In case of ETH_STAT_SUCCESS, err_type will be ETH_ERR_NONE and current
 * link status will be updated
 */
struct eth_lnk_sts_s {
	u64 reserved1:9;
	u64 link_up:1;
	u64 full_duplex:1;
	u64 speed:4;	/* eth_link_speed */
	u64 err_type:10;
	u64 an:1;		/* Current AN state : enabled/disabled */
	u64 fec:2;		/* Current FEC type if enabled, if not 0 */
	u64 lmac_type:8;	/* Share the current port info if required */
	u64 mode:8;	/* eth_mode_t enum integer value */
	u64 mode_group_idx:2; /* mode_grp_idx : group 0 or 1 depending on the mode */
	u64 reserved2:18;
};

struct sh_fwd_base_s {
	u64 reserved1:9;
	u64 addr:55;
};

struct eth_link_modes_s {
	u64 reserved1:9;
	u64 modes:55;
};

/* Resp to cmd ID - ETH_CMD_GET_ADV_FEC/ETH_CMD_GET_SUPPORTED_FEC
 * FEC : 2 bits
 * For CN9XX, below are the possible FEC types
 *
 * typedef enum cgx_fec_type {
 *     CGX_FEC_NONE = 0,
 *     CGX_FEC_BASE_R = 1,
 *     CGX_FEC_RS = 2,
 *     CGX_FEC_BASE_R_RS = 3
 * } fec_type_t;
 *
 *  For CN10K, below are the possible FEC types
 *
 * typedef enum {
 *	PORTM_FEC_DISABLED = 0,
 *	PORTM_FEC_BASER = 1,
 *	PORTM_FEC_RS = 2,
 *	PORTM_FEC_BASER_RS = 3,
 * } cn10k_portm_fec_t;
 */
struct eth_fec_types_s {
	u64 reserved1:9;
	u64 fec:2;
	u64 reserved2:53;
};

/* Resp to cmd ID - ETH_CMD_GET_AN */
struct eth_get_an_s {
	u64 reserved1:9;
	u64 an:1;
	u64 reserved2:54;
};

/* Resp to cmd ID - ETH_CMD_GET_PHY_MOD_TYPE */
struct eth_get_phy_mod_type_s {
	u64 reserved1:9;
	u64 mod:1;		/* 0=NRZ, 1=PAM4 */
	u64 reserved2:54;
};

/* Resp to cmd ID - ETH_CMD_GET_PERSIST_IGNORE */
struct eth_get_flash_ignore_s {
	u64 reserved1:9;
	u64 ignore:1;
	u64 reserved2:54;
};

struct eth_get_port_mode_s {
	uint64_t reserved1:9;
	uint64_t mode_group_idx:2;	/* mode_group_t */
	uint64_t mode:6;		/* eth_mode_t or eth_cpri_mode_t depending on
					 * the mode_group_idx
					 */
	uint64_t reserved2:47;
};

union eth_rsp_sts {
	/* Fixed, applicable for all commands/events */
	struct eth_evt_sts_s evt_sts;
	/* response to ETH_CMD_LINK_BRINGUP/DOWN/LINK_CHANGE */
	struct eth_lnk_sts_s link_sts;
	/* response to ETH_CMD_GET_PORT_MODE */
	struct eth_get_port_mode_s port_mode;
	/* response to ETH_CMD_GET_FW_VER */
	struct eth_ver_s ver;
	/* response to ETH_CMD_GET_MAC_ADDR */
	struct eth_mac_addr_s mac_s;
	/* response to ETH_CMD_GET_FWD_BASE */
	struct sh_fwd_base_s fwd_base_s;
	/* response if evt_status = CMD_FAIL */
	struct eth_err_sts_s err;
	/* response to ETH_CMD_GET_SUPPORTED_FEC */
	struct eth_fec_types_s supported_fec;
	/* response to ETH_CMD_GET_LINK_MODES */
	struct eth_link_modes_s supported_modes;
	/* response to ETH_CMD_GET_ADV_LINK_MODES */
	struct eth_link_modes_s adv_modes;
	/* response to ETH_CMD_GET_ADV_FEC */
	struct eth_fec_types_s adv_fec;
	/* response to ETH_CMD_GET_AN */
	struct eth_get_an_s an;
	/* response to ETH_CMD_GET_PHY_MOD_TYPE */
	struct eth_get_phy_mod_type_s phy_mod_type;
	/* response to ETH_CMD_GET_PERSIST_IGNORE */
	struct eth_get_flash_ignore_s persist;
#ifdef NT_FW_CONFIG
	/* response to ETH_CMD_GET_MKEX_SIZE */
	struct eth_mcam_profile_sz_s prfl_sz;
	/* response to ETH_CMD_GET_MKEX_PROFILE */
	struct eth_mcam_profile_addr_s prfl_addr;
#endif
};

union eth_scratchx0 {
	u64 u;
	union eth_rsp_sts s;
};

/* scratchx(1) CSR used for non-secure SW->ATF communication
 * This CSR acts as a command register
 */
struct eth_cmd {			/* start from bit 2 */
	u64 reserved1:2;
	u64 id:6;			/* eth_request_id */
	u64 reserved2:56;
};

/* all the below structures are in the same memory location of SCRATCHX(1)
 * corresponding arguments for command Id needs to be updated
 */

/* Any command using enable/disable as an argument need
 * to pass the option via this structure.
 * Ex: Loopback, HiGig...
 */
struct eth_ctl_args {			/* start from bit 8 */
	u64 reserved1:8;
	u64 enable:1;
	u64 reserved2:55;
};

/* command argument to be passed for cmd ID - ETH_CMD_SET_MTU */
struct eth_mtu_args {
	u64 reserved1:8;
	u64 size:16;
	u64 reserved2:40;
};

/* command argument to be passed for cmd ID - CGX_CMD_LINK_BRINGUP */
struct cgx_link_bringup_args {         /* start from bit 8 */
	uint64_t reserved1:8;
	uint64_t timeout:14;            /* in ms */
	uint64_t rx_tx_dis:1;		/* Argument to not enable Rx/Tx when link is up */
	uint64_t reserved2:41;
};

/* command argument to be passed for cmd ID - ETH_CMD_MODE_CHANGE */
struct eth_mode_change_args {
	u64 reserved1:8;
	u64 speed:4; /* eth_link_speed enum */
	u64 duplex:1; /* 0 - full duplex, 1 - half duplex */
	u64 an:1;	/* 0 - disable AN, 1 - enable AN */
	uint64_t use_portm_idx:1;
	uint64_t portm_idx:5;
	/* This field categorize the mode ID range to accomodate more modes.
	* To specify mode ID range of 0 - 41, this field will be 0.
	* To specify mode ID range of 42 - 83, this field will be 1 and so.
	* mode ID will be still mentioned as 1 << (0 - 41). But the mode_group_idx
	* decides the actual mode range
	*/
	u64 mode_group_idx:2;
	u64 mode:42;	/* (1 << eth_mode_t) enum */
};

struct eth_get_port_mode_args {
	uint64_t reserved1:8;
	uint64_t portm_idx:5;
	uint64_t reserved2:51;
};

struct eth_ecp_dump_state_args {
	uint64_t reserved1:8;
	uint64_t portm_idx:8;
	uint64_t lmac_id:4;     /* only required for multi-lmac ports */
	uint64_t reserved2:44;
};

/* command argument to be passed for cmd ID - ETH_CMD_LINK_CHANGE */
struct eth_link_change_args {		/* start from bit 8 */
	u64 reserved1:8;
	u64 link_up:1;
	u64 full_duplex:1;
	u64 speed:4;		/* eth_link_speed */
	u64 reserved2:50;
};

/* command argument to be passed for cmd ID - ETH_CMD_CPRI_MODE_CHANGE */
struct cpri_mode_change_args {
	u64 reserved1:8;
	u64 gserc_idx:4; /* GSERC index 0 - 4 */
	u64 lane_idx:4;  /* lane index 0 - 1 */
	u64 rate:16; /* 9830/4915/2458/6144/3072 */
	u64 disable_leq:1;
	u64 disable_dfe:1;
	u64 reserved2:30;
};

/* command argument to be passed for cmd ID - ETH_CMD_CPRI_TX_CONTROL */
struct cpri_mode_tx_ctrl_args {
	u64 reserved1:8;
	u64 gserc_idx:4;	/* GSERC index 0 - 4 */
	u64 lane_idx:4;	/* lane index 0 - 1 */
	u64 enable:1; /* 0 - disable, 1 - enable */
	u64 reserved2:47;
};

/* command argument to be passed for cmd ID - CGX_CMD_CPRI_MISC */
struct cpri_mode_misc_args {
	u64 reserved1:8;
	u64 gserc_idx:4;	/* GSERC index 0 - 4 */
	u64 lane_idx:4;	/* lane index 0 - 1 */
	u64 flags:2;	/* 0 - RX Eq
				   1 - RX State machine reset */
	u64 reserved2:46;
};

/* command argument to be passed for cmd ID - ETH_CMD_LEQ_ADAPT_SERDES */
struct gser_leq_adapt {
	u64 reserved1:8;
	u64 ifg_start:5;
	u64 hfg_sqi_start:5;
	u64 mbf_start:4;
	u64 mbg_start:4;
	u64 apg_start:3;
	u64 enable:1;	/* 0 - disable, 1 - enable */
	u64 reserved2:34;
};

/* command argument to be passed for cmd ID - ETH_CMD_SET_LINK_MODE */
struct eth_set_mode_args {
	u64 reserved1:8;
	u64 mode:56; /* Bitmask of eth_mode_t enum */
};

/*
 * Resp to cmd ID - ETH_CMD_GET_ADV_FEC/ETH_CMD_GET_SUPPORTED_FEC
 * FEC : 2 bits
 *
 *  For CN9XX, below are the possible FEC types
 *
 * typedef enum cgx_fec_type {
 *     CGX_FEC_NONE = 0,
 *     CGX_FEC_BASE_R = 1,
 *     CGX_FEC_RS = 2,
 *     CGX_FEC_BASE_R_RS = 3
 * } fec_type_t;
 *
 *  For CN10K, below are the possible FEC types
 *
 * typedef enum {
 *	PORTM_FEC_DISABLED = 0,
 *	PORTM_FEC_BASER = 1,
 *	PORTM_FEC_RS = 2,
 *	PORTM_FEC_BASER_RS = 3,
 * } cn10k_portm_fec_t;
 */
/* command argument to be passed for cmd ID - ETH_CMD_SET_FEC */
struct eth_set_fec_args {
	u64 reserved1:8;
	u64 fec:2;
	u64 reserved2:54;
};

/* command argument to be passed for cmd ID - ETH_CMD_DO_CMU_RESET */
struct eth_do_cmu_reset {
	u64 reserved1:8;
	u64 cgx:3;
	u64 reserved2:53;
};

/* command argument to be passed for cmd ID - ETH_CMD_SET_PHY_MOD_TYPE */
struct eth_set_phy_mod_args {
	u64 reserved1:8;
	u64 mod:1;		/* 0=NRZ, 1=PAM4 */
	u64 reserved2:55;
};

/* command argument to be passed for cmd ID - ETH_CMD_SET_PERSIST_IGNORE */
struct eth_set_flash_ignore_args {
	u64 reserved1:8;
	u64 ignore:1;
	u64 reserved2:55;
};

/* command argument to be passed for cmd ID - ETH_CMD_SET_MAC_ADDR */
struct eth_mac_addr_args {
	u64 reserved1:8;
	u64 addr:48;
	u64 pf_id:8;
};

/* command argument to be passed for cmd ID - ETH_CMD_PRBS */
struct eth_prbs_args {
	u64 reserved1:8; /* start from bit 8 */
	u64 lane:8;
	u64 qlm:8;
	u64 stop_on_error:1;
	u64 mode:8;
	u64 time:31;
};

/* command argument to be passed for cmd ID - ETH_CMD_DISPLAY_EYE or
 * ETH_CMD_DISPLAY_SERDES
 */
struct eth_display_args {
	u64 reserved1:8; /* start from bit 8 */
	u64 qlm:8;
	u64 lane:47;
};

/* Resp to cmd ID - ETH_CMD_SERDES_LOOP
 * flags : 3 bits
 *    if 0 : disable serdes loopback
 *    if 1 : FEA serdes loopback
 *    if 2 : NED serdes loopback
 *    if 3 : NEA serdes loopback
 *    if 4 : FED serdes loopback
 */
struct eth_gser_loop {
	u64 reserved1:8;
	u64 flags:3;
	u64 reserved2:53;
};

/* Configure TX tuning parameters */
struct eth_gser_tune {
	u64 reserved1:8;
	u64 portm_idx:8;
	u64 tx_main:8;
	u64 tx_pre:8;
	u64 tx_post:8;
	u64 tx_pre2:8;
	u64 reserved2:16;
};

union eth_cmd_s {
	u64 own_status:2;			/* eth_cmd_own */
	struct eth_cmd cmd;
	struct eth_ctl_args cmd_args;
	struct eth_mtu_args mtu_size;
	struct eth_link_change_args lnk_args;	/* Input to ETH_CMD_LINK_CHANGE */
	struct cgx_link_bringup_args lnk_bringup;
	struct eth_set_mode_args mode_args;
	struct eth_mode_change_args mode_change_args;
	struct eth_get_port_mode_args port_mode_args;
	struct eth_ecp_dump_state_args ecp_dump_state_args;
	struct eth_set_fec_args fec_args;
	struct eth_do_cmu_reset cmu_args;
	struct eth_set_phy_mod_args phy_mod_args;
	struct eth_set_flash_ignore_args persist_args;
	struct eth_mac_addr_args mac_args;
	struct cpri_mode_change_args cpri_change_args;
	struct cpri_mode_tx_ctrl_args cpri_tx_ctrl_args;
	struct cpri_mode_misc_args cpri_misc_args;
	struct gser_leq_adapt leq_adt;
	/* any other arg for command id * like : mtu, dmac filtering control */
	struct eth_prbs_args prbs_args;
	struct eth_display_args dsp_eye_args;
	struct eth_display_args dsp_serdes_args;
	struct eth_gser_loop gser_loop;
	struct eth_gser_tune gser_tune;
};

union eth_scratchx1 {
	u64 u;
	union eth_cmd_s s;
};

#endif /* __ETH_INTF_H__ */
