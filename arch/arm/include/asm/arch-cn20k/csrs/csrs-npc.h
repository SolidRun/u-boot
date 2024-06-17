/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2022 Marvell
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_NPC_H__
#define __CSRS_NPC_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * NPC.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration npc_ctype_e
 *
 * NPC Channel Type Enumeration Enumerates the NPC channel CTYPEs.
 */
#define NPC_EM_CTYPE_E_EM_CTYPEX(a) (0 + (a))

/**
 * Enumeration npc_errlev_e
 *
 * NPC Error Level Enumeration Enumerates the lowest protocol layer
 * containing an error.
 */
#define NPC_ERRLEV_E_LA (1)
#define NPC_ERRLEV_E_LB (2)
#define NPC_ERRLEV_E_LC (3)
#define NPC_ERRLEV_E_LD (4)
#define NPC_ERRLEV_E_LE (5)
#define NPC_ERRLEV_E_LF (6)
#define NPC_ERRLEV_E_LG (7)
#define NPC_ERRLEV_E_LH (8)
#define NPC_ERRLEV_E_NIX (0xf)
#define NPC_ERRLEV_E_RX(a) (9 + (a))
#define NPC_ERRLEV_E_RE (0)

/**
 * Enumeration npc_exact_opc_e
 *
 * NPC MCAM Search Key Width Enumeration
 */
#define NPC_EXACT_OPC_E_INVAL (0)
#define NPC_EXACT_OPC_E_RESERVED (3)
#define NPC_EXACT_OPC_E_VAL_CAM (1)
#define NPC_EXACT_OPC_E_VAL_MEM (2)

/**
 * Enumeration npc_intf_e
 *
 * NPC Interface Enumeration Enumerates the NPC interfaces.
 */
#define NPC_INTF_E_NIXX_RX(a) (0 + 2 * (a))
#define NPC_INTF_E_NIXX_RX1(a) (2 + 0 * (a))
#define NPC_INTF_E_NIXX_TX(a) (1 + 2 * (a))

/**
 * Enumeration npc_lid_e
 *
 * NPC Layer ID Enumeration Enumerates layers parsed by NPC.
 */
#define NPC_LID_E_LA (0)
#define NPC_LID_E_LB (1)
#define NPC_LID_E_LC (2)
#define NPC_LID_E_LD (3)
#define NPC_LID_E_LE (4)
#define NPC_LID_E_LF (5)
#define NPC_LID_E_LG (6)
#define NPC_LID_E_LH (7)

/**
 * Enumeration npc_lkupop_e
 *
 * NPC Lookup Operation Enumeration Enumerates the lookup operation for
 * NPC_AF_LKUP_CTL[OP].
 */
#define NPC_LKUPOP_E_KEY (1)
#define NPC_LKUPOP_E_PKT (0)

/**
 * Enumeration npc_mcamkeyw_e
 *
 * NPC MCAM Search Key Width Enumeration
 */
#define NPC_MCAMKEYW_E_DYNAMIC (0)
#define NPC_MCAMKEYW_E_X1 (0)
#define NPC_MCAMKEYW_E_X2 (1)
#define NPC_MCAMKEYW_E_X4 (2)

/**
 * Enumeration npc_ptype_e
 *
 * NPC Port Kind Type Enumeration Enumerates the NPC pkind PTYPEs.
 */
#define NPC_PTYPE_E_PTYPEX(a) (0 + (a))

/**
 * Register (RVU_PF_BAR0) npc_af_blk_rst
 *
 * NPC AF Block Reset Register
 */
union npc_af_blk_rst {
	u64 u;
	struct npc_af_blk_rst_s {
		u64 rst                              : 1;
		u64 reserved_1_62                    : 62;
		u64 busy                             : 1;
	} s;
	/* struct npc_af_blk_rst_s cn; */
};

static inline u64 NPC_AF_BLK_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_BLK_RST(void)
{
	return 0x40;
}

/**
 * Register (RVU_PF_BAR0) npc_af_const
 *
 * NPC AF Constants Register This register contains constants for
 * software discovery.
 */
union npc_af_const {
	u64 u;
	struct npc_af_const_s {
		u64 intfs                            : 4;
		u64 lids                             : 4;
		u64 kpus                             : 5;
		u64 reserved_13_15                   : 3;
		u64 mcam_bank_width                  : 10;
		u64 reserved_26_27                   : 2;
		u64 mcam_bank_depth                  : 16;
		u64 mcam_banks                       : 4;
		u64 match_stats                      : 16;
	} s;
	/* struct npc_af_const_s cn; */
};

static inline u64 NPC_AF_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_CONST(void)
{
	return 0x20;
}

/**
 * Register (RVU_PF_BAR0) npc_af_dbg_ctl
 *
 * NPC AF Debug Control Register This register controls the capture of
 * debug information in NPC_AF_KPU()_DBG, NPC_AF_MCAM_DBG,
 * NPC_AF_DBG_DATA() and NPC_AF_DBG_RESULT().
 */
union npc_af_dbg_ctl {
	u64 u;
	struct npc_af_dbg_ctl_s {
		u64 continuous                       : 1;
		u64 lkup_dbg                         : 1;
		u64 intf_dbg                         : 4;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct npc_af_dbg_ctl_s cn; */
};

static inline u64 NPC_AF_DBG_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_DBG_CTL(void)
{
	return 0x3000000;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_kex_cfg
 *
 * NPC AF Interface Key Extract Configuration Registers
 */
union npc_af_intfx_kex_cfg {
	u64 u;
	struct npc_af_intfx_kex_cfg_s {
		u64 parse_nibble_ena                 : 23;
		u64 reserved_23_31                   : 9;
		u64 keyw                             : 3;
		u64 reserved_35_39                   : 5;
		u64 exact_nibble_ena                 : 4;
		u64 reserved_44_63                   : 20;
	} s;
};

static inline u64 NPC_AF_INTFX_KEX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_KEX_CFG(u64 a)
{
	return 0x1010 + 0x100 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_miss_act
 *
 * NPC AF Interface MCAM Miss Action Data Registers When a combination of
 * NPC_AF_MCAME()_BANK()_CAM()_* and NPC_AF_MCAME()_BANK()_CFG_EXT[ENA]
 * yields an MCAM miss for a packet, this register specifies the packet's
 * match action captured in NPC_RESULT_S[ACTION].
 */
union npc_af_intfx_miss_act {
	u64 u;
	struct npc_af_intfx_miss_act_s {
		u64 action                           : 64;
	} s;
	/* struct npc_af_intfx_miss_act_s cn; */
};

static inline u64 NPC_AF_INTFX_MISS_ACT(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_MISS_ACT(u64 a, u64 b)
{
	return 0x1a00000 + 0x40 * a + 0x10 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_cfg
 *
 * NPC AF KPU Configuration Registers
 */
union npc_af_kpux_cfg {
	u64 u;
	struct npc_af_kpux_cfg_s {
		u64 ena                              : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npc_af_kpux_cfg_s cn; */
};

static inline u64 NPC_AF_KPUX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_CFG(u64 a)
{
	return 0x500 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_action#_ext
 *
 * NPC AF MCAM Entry Bank Action Data Registers
 * Specifies a packet's match action captured in NPC_RESULT_S[ACTION].
 *
 * When an interface is configured to use the NPC_MCAM_KEY_X2_S search key
 * format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2),
 * * NPC_AF_MCAME()_BANK(0)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are used if the search key
 * matches NPC_AF_MCAME()_BANK(0..1)_CAM()_W*_EXT.
 * * NPC_AF_MCAME()_BANK(2)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are used if the search key
 * matches NPC_AF_MCAME()_BANK(2..3)_CAM()_W*_EXT.
 * * NPC_AF_MCAME()_BANK(1,3)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are not used.
 *
 * When an interface is configured to use the NPC_MCAM_KEY_X4_S search key
 * format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4):
 * * NPC_AF_MCAME()_BANK(0)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are used if the search key
 * matches NPC_AF_MCAME()_BANK(0..3)_CAM()_W*_EXT.
 * * NPC_AF_MCAME()_BANK(1..3)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are not used.
 */
union npc_af_mcamex_bankx_actionx_ext {
    u64 u;
    struct npc_af_mcamex_bankx_actionx_ext_s {
#if __BYTE_ORDER == __BIG_ENDIAN /* Word 0 - Big Endian */
	u64 action                : 64; /**< [ 63:  0](R/W) Match action.
	Format is NIX_RX_ACTION_S for RX packet, NIX_TX_ACTION_S for TX packet. */
#else /* Word 0 - Little Endian */
	u64 action                : 64; /**< [ 63:  0](R/W) Match action.
	Format is NIX_RX_ACTION_S for RX packet, NIX_TX_ACTION_S for TX packet. */
#endif /* Word 0 - End */
    } s;
    /* struct npc_af_mcamex_bankx_actionx_ext_s cn; */
};

typedef union npc_af_mcamex_bankx_actionx_ext npc_af_mcamex_bankx_actionx_ext_t;

static inline uint64_t NPC_AF_MCAMEX_BANKX_ACTIONX_EXT(u64 a, u64 b, u64 c)
	 __attribute__ ((pure, always_inline));
static inline uint64_t NPC_AF_MCAMEX_BANKX_ACTIONX_EXT(u64 a, u64 b, u64 c)
{
	return 0x8000060 + 0x100 * a + 0x400000 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_action_ext
 *
 * NPC AF MCAM Entry Bank Action Data Registers Specifies a packet's
 * match action captured in NPC_RESULT_S[ACTION].  When an interface is
 * configured to use the NPC_MCAM_KEY_X2_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2), *
 * NPC_AF_MCAME()_BANK(0)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are used
 * if the search key matches NPC_AF_MCAME()_BANK(0..1)_CAM()_W*_EXT. *
 * NPC_AF_MCAME()_BANK(2)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are used
 * if the search key matches NPC_AF_MCAME()_BANK(2..3)_CAM()_W*_EXT. *
 * NPC_AF_MCAME()_BANK(1,3)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are not
 * used.  When an interface is configured to use the NPC_MCAM_KEY_X4_S
 * search key format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4):
 * * NPC_AF_MCAME()_BANK(0)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are
 * used if the search key matches NPC_AF_MCAME()_BANK(0..3)_CAM()_W*_EXT.
 * * NPC_AF_MCAME()_BANK(1..3)_ACTION_EXT/_TAG_ACT_EXT/_STAT_ACT_EXT are
 * not used.
 */
union npc_af_mcamex_bankx_action_ext {
	u64 u;
	struct npc_af_mcamex_bankx_action_ext_s {
		u64 action                           : 64;
	} s;
	/* struct npc_af_mcamex_bankx_action_ext_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_ACTION_EXT(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_ACTION_EXT(u64 a, u64 b, u64 c)
{
	return 0x8000040 + 0x100 * a + 0x400000 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_intf_ext
 *
 * NPC AF MCAM Entry Bank CAM Data Interface Registers MCAM comparison
 * ternary data interface word. The field values in
 * NPC_AF_MCAME()_BANK()_CAM()_INTF_EXT,
 * NPC_AF_MCAME()_BANK()_CAM()_W0_EXT and
 * NPC_AF_MCAME()_BANK()_CAM()_W1_EXT are ternary, where  each data bit
 * of the search key matches as follows: _ [CAM(1)]\<n\>=0,
 * [CAM(0)]\<n\>=0: Always match; search key data\<n\> don't care. _
 * [CAM(1)]\<n\>=0, [CAM(0)]\<n\>=1: Match when search key data\<n\> ==
 * 0. _ [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=0: Match when search key data\<n\>
 * == 1. _ [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=1: Reserved.  The reserved
 * combination is not allowed. Hardware suppresses any write to CAM(0) or
 * CAM(1) that would result in the reserved combination for any CAM bit.
 * The reset value for all non-reserved fields in
 * NPC_AF_MCAME()_BANK()_CAM()_INTF_EXT,
 * NPC_AF_MCAME()_BANK()_CAM()_W0_EXT and
 * NPC_AF_MCAME()_BANK()_CAM()_W1_EXT is all zeros for CAM(1) and all
 * ones for CAM(0), matching a search key of all zeros.  When an
 * interface is configured to use the NPC_MCAM_KEY_X1_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X1), the four banks of
 * every MCAM entry are used as individual entries, each of which is
 * independently compared with the search key as follows: _
 * NPC_AF_MCAME()_BANK()_CAM()_INTF_EXT[INTF] corresponds to
 * NPC_MCAM_KEY_X1_S[INTF]. _ NPC_AF_MCAME()_BANK()_CAM()_INTF_EXT[CTYPE]
 * corresponds to NPC_MCAM_KEY_X1_S[CTYPE]. _
 * NPC_AF_MCAME()_BANK()_CAM()_W0_EXT[MD] corresponds to
 * NPC_MCAM_KEY_X1_S[KW0]. _ NPC_AF_MCAME()_BANK()_CAM()_W1_EXT[MD]
 * corresponds to NPC_MCAM_KEY_X1_S[KW1].  When an interface is
 * configured to use the NPC_MCAM_KEY_X2_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2), banks 0-1 of every
 * MCAM entry are used as one double-wide entry, banks 2-3 as a second
 * double-wide entry, and each double-wide entry is independently
 * compared with the search key as follows: _
 * NPC_AF_MCAME()_BANK(0,2)_CAM()_INTF_EXT[INTF] corresponds to
 * NPC_MCAM_KEY_X2_S[INTF]. _
 * NPC_AF_MCAME()_BANK(0,2)_CAM()_INTF_EXT[CTYPE] corresponds to
 * NPC_MCAM_KEY_X2_S[CTYPE]. _ NPC_AF_MCAME()_BANK(0,2)_CAM()_W0_EXT[MD]
 * corresponds to NPC_MCAM_KEY_X2_S[KW0]. _
 * NPC_AF_MCAME()_BANK(0,2)_CAM()_W1_EXT[MD] corresponds to
 * NPC_MCAM_KEY_X2_S[KW1]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_INTF_EXT[INTF] corresponds to
 * NPC_MCAM_KEY_X2_S[INTF]. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_INTF_EXT[CTYPE] corresponds to
 * NPC_MCAM_KEY_X2_S[CTYPE]. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W0_EXT[MD]\<15:0\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW1]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W0_EXT[MD]\<63:16\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW2]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W1_EXT[MD]\<15:0\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW2]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W1_EXT[MD]\<47:16\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW3]\<31:0\>.  When an interface is configured to
 * use the NPC_MCAM_KEY_X4_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4), the four banks of
 * every MCAM entry are used as a single quad-wide entry that is compared
 * with the search key as follows: _
 * NPC_AF_MCAME()_BANK(0)_CAM()_INTF_EXT[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _
 * NPC_AF_MCAME()_BANK(0)_CAM()_INTF_EXT[CTYPE] corresponds to
 * NPC_MCAM_KEY_X4_S[CTYPE]. _ NPC_AF_MCAME()_BANK(0)_CAM()_W0_EXT[MD]
 * corresponds to NPC_MCAM_KEY_X4_S[KW0]. _
 * NPC_AF_MCAME()_BANK(0)_CAM()_W1_EXT[MD] corresponds to
 * NPC_MCAM_KEY_X4_S[KW1]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_INTF_EXT[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_INTF_EXT[CTYPE] corresponds to
 * NPC_MCAM_KEY_X4_S[CTYPE]. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W0_EXT[MD]\<15:0\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW1]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W0_EXT[MD]\<63:16\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW2]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W1_EXT[MD]\<15:0\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW2]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W1_EXT[MD]\<47:16\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW3]\<31:0\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_INTF_EXT[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_INTF_EXT[CTYPE] corresponds to
 * NPC_MCAM_KEY_X4_S[CTYPE]. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W0_EXT[MD]\<31:0\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW3]\<63:32\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W0_EXT[MD]\<63:32\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW4]\<31:0\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W1_EXT[MD]\<31:0\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW4]\<63:32\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W1_EXT[MD]\<47:32\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW5]\<15:0\>. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_INTF_EXT[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_INTF_EXT[CTYPE] corresponds to
 * NPC_MCAM_KEY_X4_S[CTYPE]. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_W0_EXT[MD]\<47:0\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW5]\<63:16\>. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_W0_EXT[MD]\<63:48\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW6]\<15:0\>. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_W1_EXT[MD] corresponds to
 * NPC_MCAM_KEY_X4_S[KW6]\<63:16\>.  Note that for the X2 and X4 formats,
 * a wide entry will not match unless the INTF,CTYPE fields from the
 * associated two or four banks match the INTF,CTYPE value pair from the
 * search key.  For the X1 and X2 formats, a match in a lower-numbered
 * bank takes priority over a match in any higher numbered banks. Within
 * each bank, the lowest numbered matching entry takes priority over any
 * higher numbered entry.
 */
union npc_af_mcamex_bankx_camx_intf_ext {
	u64 u;
	struct npc_af_mcamex_bankx_camx_intf_ext_s {
		u64 intf                             : 2;
		u64 reserved_2_7                     : 6;
		u64 ctype                            : 5;
		u64 reserved_13_15                   : 3;
		u64 keyw			     : 3;
		u64 reserved_19_63		     : 45;
	} s;
	/* struct npc_af_mcamex_bankx_camx_intf_ext_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_INTF_EXT(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_INTF_EXT(u64 a, u64 b, u64 c)
{
	return 0x8000000 + 0x100 * a + 0x400000 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_w0_ext
 *
 * NPC AF MCAM Entry Bank CAM Data Word 0 Registers MCAM comparison
 * ternary data word 0. See NPC_AF_MCAME()_BANK()_CAM()_INTF_EXT.
 */
union npc_af_mcamex_bankx_camx_w0_ext {
	u64 u;
	struct npc_af_mcamex_bankx_camx_w0_ext_s {
		u64 md                               : 64;
	} s;
	/* struct npc_af_mcamex_bankx_camx_w0_ext_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W0_EXT(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W0_EXT(u64 a, u64 b, u64 c)
{
	return 0x8000010 + 0x100 * a + 0x400000 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_w1_ext
 *
 * NPC AF MCAM Entry Bank Data Word 1 Registers MCAM comparison ternary
 * data word 1. See NPC_AF_MCAME()_BANK()_CAM()_INTF_EXT.
 */
union npc_af_mcamex_bankx_camx_w1_ext {
	u64 u;
	struct npc_af_mcamex_bankx_camx_w1_ext_s {
		u64 md                               : 64;
	} s;
	/* struct npc_af_mcamex_bankx_camx_w1_ext_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W1_EXT(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W1_EXT(u64 a, u64 b, u64 c)
{
	return 0x8000020 + 0x100 * a + 0x400000 * b + 8 * c;
}

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W2_EXT(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W2_EXT(u64 a, u64 b, u64 c)
{
	return 0x8000030 + 0x100 * a + 0x400000 * b + 8 * c;
}

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W3_EXT(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W3_EXT(u64 a, u64 b, u64 c)
{
	return 0x8000040 + 0x100 * a + 0x400000 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cfg_ext
 *
 * NPC AF MCAM Entry Bank Configuration Registers
 */
union npc_af_mcamex_bankx_cfg_ext {
	u64 u;
	struct npc_af_mcamex_bankx_cfg_ext_s {
		u64 ena                              : 1;
		u64 reserved_1_7                     : 7;
		u64 pri				     : 7;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct npc_af_mcamex_bankx_cfg_ext_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CFG_EXT(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CFG_EXT(u64 a, u64 b)
{
	return 0x8000050 + 0x100 * a + 0x400000 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pkind#_action0
 *
 * NPC AF Port Kind Action Data 0 Registers NPC_AF_PKIND()_ACTION0 and
 * NPC_AF_PKIND()_ACTION1 specify the initial parse state and operations
 * to perform before entering KPU 0.
 */
union npc_af_pkindx_action0 {
	u64 u;
	struct npc_af_pkindx_action0_s {
		u64 var_len_shift                    : 3;
		u64 var_len_right                    : 1;
		u64 var_len_mask                     : 8;
		u64 var_len_offset                   : 8;
		u64 ptr_advance                      : 8;
		u64 capture_flags                    : 4;
		u64 reserved_32_35		     : 4;
		u64 capture_ltype                    : 4;
		u64 capture_lid                      : 3;
		u64 reserved_43                      : 1;
		u64 next_state                       : 8;
		u64 parse_done                       : 1;
		u64 capture_ena                      : 1;
		u64 byp_count                        : 3;
		u64 reserved_57_63                   : 7;
	} s;
	/* struct npc_af_pkindx_action0_s cn; */
};

static inline u64 NPC_AF_PKINDX_ACTION0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PKINDX_ACTION0(u64 a)
{
	return 0x80000 + 0x40 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pkind#_action1
 *
 * NPC AF Port Kind Action Data 1 Registers NPC_AF_PKIND()_ACTION0 and
 * NPC_AF_PKIND()_ACTION1 specify the initial parse state and operations
 * to perform before entering KPU 0.
 */
union npc_af_pkindx_action1 {
	u64 u;
	struct npc_af_pkindx_action1_s {
		u64 dp0_offset                       : 8;
		u64 dp1_offset                       : 8;
		u64 dp2_offset                       : 8;
		u64 errcode                          : 8;
		u64 errlev                           : 4;
		u64 reserved_36_63                   : 28;
	} s;
	/* struct npc_af_pkindx_action1_s cn; */
};

static inline u64 NPC_AF_PKINDX_ACTION1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PKINDX_ACTION1(u64 a)
{
	return 0x80008 + 0x40 * a;
}

#endif /* __CSRS_NPC_H__ */
