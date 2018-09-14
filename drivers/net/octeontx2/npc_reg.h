/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


/* Register definitions */

/**
 * NPC AF General Configuration Register
 */
union cavm_npc_af_cfg {
	u64 u;
	struct npc_af_cfg_s {
		u64 rsvd_1_0:           2;
		u64 cclk_force:         1;
		u64 force_intf_clk_en:  1;
		u64 rsvd_63_4:          60;
	} s;
};

/**
 * NPC Interrupt-Timer Configuration Register
 */
union cavm_npc_af_active_pc {
	u64 u;
	struct npc_af_active_pc_s {
		u64 active_pc;                      
	} s;
};

/**
 * NPC AF Constants Register
 * This register contains constants for software discovery.
 */
union cavm_npc_af_const {
	u64 u;
	struct npc_af_const_s {
		u64 intfs:              4;
		u64 lids:               4;
		u64 kpus:               5;
		u64 rsvd_15_13:         3;
		u64 mcam_bank_width:    10;
		u64 rsvd_27_26:         2;
		u64 mcam_bank_depth:    16;
		u64 mcam_banks:         4;
		u64 match_stats:        16;
	} s;
};

/**
 * NPC AF Constants 1 Register
 * This register contains constants for software discovery.
 */
union cavm_npc_af_const1 {
	u64 u;
	struct npc_af_const1_s {
		u64 kpu_entries:        12;
		u64 pkinds:             8;
		u64 cpi_size:           16;
		u64 rsvd_63_36:         28;
	} s;
};

/**
 * NPC AF MCAM Scrub Control Register
 */
union cavm_npc_af_mcam_scrub_ctl {
	u64 u;
	struct npc_af_mcam_scrub_ctl_s {
		u64 ena:        1;
		u64 rsvd_7_1:   7;
		u64 lp_dis:     1;
		u64 rsvd_15_9:  7;
		u64 toth:       4;
		u64 rsvd_63_20: 44;
	} s;
};

/**
 * NPC AF KPU Configuration Registers
 */
union cavm_npc_af_kpux_cfg {
	u64 u;
	struct npc_af_kpux_cfg_s {
		u64 ena:        1;
		u64 rsvd_63_1:  63;
	} s;
};

/**
 * NPC AF Protocol Check Configuration Register
 */
union cavm_npc_af_pck_cfg {
	u64 u;
	struct npc_af_pck_cfg_s {
		u64 rsvd_0:             1;
		u64 iip4_cksum:         1;
		u64 oip4_cksum:         1;
		u64 rsvd_3:             1;
		u64 l3b:                1;
		u64 l3m:                1;
		u64 l2b:                1;
		u64 l2m:                1;
		u64 rsvd_23_8:          16;
		u64 iip4_cksum_errcode: 8;
		u64 oip4_cksum_errcode: 8;
		u64 rsvd_63_40:         24;
	} s;
};

/**
 * NPC AF Protocol Check Outer L2 Definition Register
 * Provides layer information used by the protocol checker to identify an
 * outer L2 header.
 */
union cavm_npc_af_pck_def_ol2 {
	u64 u;
	struct npc_af_pck_def_ol2_s {
		u64 ltype_mask:         4;
		u64 ltype_match:        4;
		u64 lid:                3;
		u64 rsvd_63_11:         53;
	} s;
};

/**
 * NPC AF Key Extract Layer Data Flags Configuration Register
 */
union cavm_npc_af_kex_ldatax_flags_cfg {
	u64 u;
	struct npc_af_kex_ldatax_flags_cfg_s {
		u64 lid:        3;
		u64 rsvd_63_3:  61;
	} s;
};

/**
 * NPC AF Interface Key Extract Configuration Registers
 */
union cavm_npc_af_intfx_kex_cfg {
	u64 u;
	struct npc_af_intfx_kex_cfg_s {
		u64 parse_nibble_ena:   31;
		u64 rsvd_31:            1;
		u64 keyw:               3;
		u64 rsvd_63_35:         29;
	} s;
};

/**
 * NPC AF Port Kind Channel Parse Index Definition Registers
 * These registers specify the layer information and algorithm to compute a
 * packet's channel parse index (CPI), which provides a port to channel adder
 * for calculating NPC_RESULT_S[CHAN]. There are two CPI definitions per port
 * kind, allowing the CPI computation to use two possible layer definitions in
 * the parsed packet, e.g. DiffServ DSCP from either IPv4 or IPv6 header. CPI
 * pseudocode: <pre> for (i = 0; i < 2; i++) {  cpi_def =
 * NPC_AF_PKIND()_CPI_DEF(i);  LX = LA, LB, ..., or LH as selected by
 * cpi_def[LID];   if (cpi_def[VALID]    && ((cpi_def[LTYPE_MATCH] &
 * cpi_def[LTYPE_MASK])       == (NPC_RESULT_S[LX[LTYPE]] &
 * cpi_def[LTYPE_MASK]))    && ((cpi_def[FLAGS_MATCH] & cpi_def[FLAGS_MASK])
 * == (NPC_RESULT_S[LX[FLAGS]] & cpi_def[FLAGS_MASK])))  {    // Found
 * matching layer    nibble_offset = (2*NPC_RESULT_S[LX[LPTR]]) +
 * cpi_def[ADD_OFFSET];    add_byte = byte at nibble_offset from start of
 * packet;    cpi_add = (add_byte & cpi_def[ADD_MASK]) >> cpi_def[ADD_SHIFT];
 * cpi = cpi_def[CPI_BASE] + cpi_add;    NPC_RESULT_S[CHAN] +=
 * NPC_AF_CPI(cpi)_CFG[PADD];    break;  } } </pre>
 */
union cavm_npc_af_pkindx_cpi_defx {
	u64 u;
	struct npc_af_pkindx_cpi_defx_s {
		u64 cpi_base:           10;
		u64 rsvd_11_10:         2;
		u64 add_shift:          3;
		u64 rsvd_15:            1;
		u64 add_mask:           8;
		u64 add_offset:         8;
		u64 flags_mask:         8;
		u64 flags_match:        8;
		u64 ltype_mask:         4;
		u64 ltype_match:        4;
		u64 lid:                3;
		u64 rsvd_62_59:         4;
		u64 ena:                1;
	} s;
};

/**
 * NPC AF KPU Entry CAM Registers
 * KPU comparison ternary data. The field values in NPC_AF_KPU()_ENTRY()_CAM()
 * are ternary, where each data bit of the search key matches as follows: _
 * [CAM(1)]<n>=0, [CAM(0)]<n>=0: Always match; search key data<n> don't care.
 * _ [CAM(1)]<n>=0, [CAM(0)]<n>=1: Match when search key data<n> == 0. _
 * [CAM(1)]<n>=1, [CAM(0)]<n>=0: Match when search key data<n> == 1. _
 * [CAM(1)]<n>=1, [CAM(0)]<n>=1: Reserved. The reserved combination is not
 * allowed. Hardware suppresses any write to CAM(0) or CAM(1) that would
 * result in the reserved combination for any CAM bit. Software must program a
 * default entry for each KPU, e.g. by programming each KPU's last entry {b}
 * (NPC_AF_KPU()_ENTRY({b})_CAM()) to always match all bits.
 */
union cavm_npc_af_kpux_entryx_camx {
	u64 u;
	struct npc_af_kpux_entryx_camx_s {
		u64 dp0_data:   16;
		u64 dp1_data:   16;
		u64 dp2_data:   16;
		u64 state:      8;
		u64 rsvd_63_56: 8;
	} s;
};

/**
 * NPC AF KPU Entry Action Data 0 Registers
 * When a KPU's search data matches a KPU CAM entry in
 * NPC_AF_KPU()_ENTRY()_CAM(), the corresponding entry action in
 * NPC_AF_KPU()_ENTRY()_ACTION0 and NPC_AF_KPU()_ENTRY()_ACTION1 specifies the
 * next state and operations to perform before exiting the KPU.
 */
union cavm_npc_af_kpux_entryx_action0 {
	u64 u;
	struct npc_af_kpux_entryx_action0_s {
		u64 var_len_shift:      3;
		u64 var_len_right:      1;
		u64 var_len_mask:       8;
		u64 var_len_offset:     8;
		u64 ptr_advance:        8;
		u64 capture_flags:      8;
		u64 capture_ltype:      4;
		u64 capture_lid:        3;
		u64 rsvd_43:            1;
		u64 next_state:         8;
		u64 parse_done:         1;
		u64 capture_ena:        1;
		u64 byp_count:          3;
		u64 rsvd_63_57:         7;
	} s;
};

/**
 * NPC AF KPU Entry Action Data 0 Registers
 * See NPC_AF_KPU()_ENTRY()_ACTION0.
 */
union cavm_npc_af_kpux_entryx_action1 {
	u64 u;
	struct npc_af_kpux_entryx_action1_s {
		u64 dp0_offset: 8;
		u64 dp1_offset: 8;
		u64 dp2_offset: 8;
		u64 errcode:    8;
		u64 errlev:     4;
		u64 rsvd_63_36: 28;
	} s;
};

/**
 * NPC AF KPU Entry Disable Registers
 * See NPC_AF_KPU()_ENTRY()_ACTION0.
 */
union cavm_npc_af_kpux_entry_disx {
	u64 u;
	struct npc_af_kpux_entry_disx_s {
		u64 dis;                            
	} s;
};

/**
 * NPC AF Channel Parse Index Table Registers
 */
union cavm_npc_af_cpix_cfg {
	u64 u;
	struct npc_af_cpix_cfg_s {
		u64 padd:       4;
		u64 rsvd_63_4:  60;
	} s;
};

/**
 * NPC AF Interface Layer Data Extract Configuration Registers
 * These registers control the extraction of layer data (LDATA) into the MCAM
 * search key for each interface. Up to two LDATA fields can be extracted per
 * layer (LID(0..7) indexed by NPC_LID_E), with up to 16 bytes per LDATA
 * field. For each layer, the corresponding NPC_LAYER_INFO_S[LTYPE] value in
 * NPC_RESULT_S is used as the LTYPE(0..15) index and select the associated
 * LDATA(0..1) registers. NPC_LAYER_INFO_S[LTYPE]=0x0 means the corresponding
 * layer not parsed (invalid), so software should keep
 * NPC_AF_INTF()_LID()_LT(0)_LD()_CFG[ENA] clear to disable extraction when
 * LTYPE is zero.
 */
union cavm_npc_af_intfx_lidx_ltx_ldx_cfg {
	u64 u;
	struct npc_af_intfx_lidx_ltx_ldx_cfg_s {
		u64 key_offset: 6;
		u64 flags_ena:  1;
		u64 ena:        1;
		u64 hdr_offset: 8;
		u64 bytesm1:    4;
		u64 rsvd_63_20: 44;
	} s;
};

/**
 * NPC AF Interface Layer Data Flags Configuration Registers
 * These registers control the extraction of layer data (LDATA) into the MCAM
 * search key for each interface based on the FLAGS<3:0> bits of two layers
 * selected by NPC_AF_KEX_LDATA()_FLAGS_CFG.
 */
union cavm_npc_af_intfx_ldatax_flagsx_cfg {
	u64 u;
	struct npc_af_intfx_ldatax_flagsx_cfg_s {
		u64 key_offset: 6;
		u64 rsvd_6:     1;
		u64 ena:        1;
		u64 hdr_offset: 8;
		u64 bytesm1:    4;
		u64 rsvd_63_20: 44;
	} s;
};

/**
 * NPC AF MCAM Entry Bank CAM Data Interface Registers
 * MCAM comparison ternary data interface word. The field values in
 * NPC_AF_MCAME()_BANK()_CAM()_INTF, NPC_AF_MCAME()_BANK()_CAM()_W0 and
 * NPC_AF_MCAME()_BANK()_CAM()_W1 are ternary, where each data bit of the
 * search key matches as follows: _ [CAM(1)]<n>=0, [CAM(0)]<n>=0: Always
 * match; search key data<n> don't care. _ [CAM(1)]<n>=0, [CAM(0)]<n>=1: Match
 * when search key data<n> == 0. _ [CAM(1)]<n>=1, [CAM(0)]<n>=0: Match when
 * search key data<n> == 1. _ [CAM(1)]<n>=1, [CAM(0)]<n>=1: Reserved. The
 * reserved combination is not allowed. Hardware suppresses any write to
 * CAM(0) or CAM(1) that would result in the reserved combination for any CAM
 * bit. When an interface is configured to use the NPC_MCAM_KEY_X1_S search
 * key format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X1), the four
 * banks of every MCAM entry are used as individual entries, each of which is
 * independently compared with the search key as follows: _
 * NPC_AF_MCAME()_BANK()_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X1_S[INTF]. _ NPC_AF_MCAME()_BANK()_CAM()_W0[MD] corresponds
 * to NPC_MCAM_KEY_X1_S[KW0]. _ NPC_AF_MCAME()_BANK()_CAM()_W1[MD] corresponds
 * to NPC_MCAM_KEY_X1_S[KW1]. When an interface is configured to use the
 * NPC_MCAM_KEY_X2_S search key format (NPC_AF_INTF()_KEX_CFG[KEYW] =
 * NPC_MCAMKEYW_E::X2), banks 0-1 of every MCAM entry are used as one
 * double-wide entry, banks 2-3 as a second double-wide entry, and each
 * double-wide entry is independently compared with the search key as follows:
 * _ NPC_AF_MCAME()_BANK(0,2)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X2_S[INTF]. _ NPC_AF_MCAME()_BANK(0,2)_CAM()_W0[MD]
 * corresponds to NPC_MCAM_KEY_X2_S[KW0]. _
 * NPC_AF_MCAME()_BANK(0,2)_CAM()_W1[MD] corresponds to
 * NPC_MCAM_KEY_X2_S[KW1]<47:0>. _ NPC_AF_MCAME()_BANK(1,3)_CAM()_INTF[INTF]
 * corresponds to NPC_MCAM_KEY_X2_S[INTF]. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W0[MD]<15:0> corresponds to
 * NPC_MCAM_KEY_X2_S[KW1]<63:48>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W0[MD]<63:16> corresponds to
 * NPC_MCAM_KEY_X2_S[KW2]<47:0>. _ NPC_AF_MCAME()_BANK(1,3)_CAM()_W1[MD]<15:0>
 * corresponds to NPC_MCAM_KEY_X2_S[KW2]<63:48>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W1[MD]<47:16> corresponds to
 * NPC_MCAM_KEY_X2_S[KW3]<31:0>. When an interface is configured to use the
 * NPC_MCAM_KEY_X4_S search key format (NPC_AF_INTF()_KEX_CFG[KEYW] =
 * NPC_MCAMKEYW_E::X4), the four banks of every MCAM entry are used as a
 * single quad-wide entry that is compared with the search key as follows: _
 * NPC_AF_MCAME()_BANK(0)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _ NPC_AF_MCAME()_BANK(0)_CAM()_W0[MD] corresponds
 * to NPC_MCAM_KEY_X4_S[KW0]. _ NPC_AF_MCAME()_BANK(0)_CAM()_W1[MD]
 * corresponds to NPC_MCAM_KEY_X4_S[KW1]<47:0>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _ NPC_AF_MCAME()_BANK(1)_CAM()_W0[MD]<15:0>
 * corresponds to NPC_MCAM_KEY_X4_S[KW1]<63:48>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W0[MD]<63:16> corresponds to
 * NPC_MCAM_KEY_X4_S[KW2]<47:0>. _ NPC_AF_MCAME()_BANK(1)_CAM()_W1[MD]<15:0>
 * corresponds to NPC_MCAM_KEY_X4_S[KW2]<63:48>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W1[MD]<47:16> corresponds to
 * NPC_MCAM_KEY_X4_S[KW3]<31:0>. _ NPC_AF_MCAME()_BANK(2)_CAM()_INTF[INTF]
 * corresponds to NPC_MCAM_KEY_X4_S[INTF]. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W0[MD]<31:0> corresponds to
 * NPC_MCAM_KEY_X4_S[KW3]<63:32>. _ NPC_AF_MCAME()_BANK(2)_CAM()_W0[MD]<63:32>
 * corresponds to NPC_MCAM_KEY_X4_S[KW4]<31:0>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W1[MD]<31:0> corresponds to
 * NPC_MCAM_KEY_X4_S[KW4]<63:32>. _ NPC_AF_MCAME()_BANK(2)_CAM()_W1[MD]<47:32>
 * corresponds to NPC_MCAM_KEY_X4_S[KW5]<15:0>. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _ NPC_AF_MCAME()_BANK(3)_CAM()_W0[MD]<47:0>
 * corresponds to NPC_MCAM_KEY_X4_S[KW5]<63:16>. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_W0[MD]<63:48> corresponds to
 * NPC_MCAM_KEY_X4_S[KW6]<15:0>. _ NPC_AF_MCAME()_BANK(3)_CAM()_W1[MD]
 * corresponds to NPC_MCAM_KEY_X4_S[KW6]<63:16>. Note that for the X2 and X4
 * formats, a wide entry will not match unless the INTF fields from the
 * associated two or four banks match the INTF value from the search key. For
 * the X1 and X2 formats, a match in a lower-numbered bank takes priority over
 * a match in any higher numbered banks. Within each bank, the lowest numbered
 * matching entry takes priority over any higher numbered entry.
 */
union cavm_npc_af_mcamex_bankx_camx_intf {
	u64 u;
	struct npc_af_mcamex_bankx_camx_intf_s {
		u64 intf:       2;
		u64 rsvd_63_2:  62;
	} s;
};

/**
 * NPC AF MCAM Entry Bank CAM Data Word 0 Registers
 * MCAM comparison ternary data word 0. See NPC_AF_MCAME()_BANK()_CAM()_INTF.
 */
union cavm_npc_af_mcamex_bankx_camx_w0 {
	u64 u;
	struct npc_af_mcamex_bankx_camx_w0_s {
		u64 md;                             
	} s;
};

/**
 * NPC AF MCAM Entry Bank Data Word 1 Registers
 * MCAM comparison ternary data word 1. See NPC_AF_MCAME()_BANK()_CAM()_INTF.
 */
union cavm_npc_af_mcamex_bankx_camx_w1 {
	u64 u;
	struct npc_af_mcamex_bankx_camx_w1_s {
		u64 md:         48;
		u64 rsvd_63_48: 16;
	} s;
};

/**
 * NPC AF MCAM Entry Bank Configuration Registers
 */
union cavm_npc_af_mcamex_bankx_cfg {
	u64 u;
	struct npc_af_mcamex_bankx_cfg_s {
		u64 ena:        1;
		u64 rsvd_63_1:  63;
	} s;
};

/**
 * NPC AF MCAM Entry Bank Statistics Action Registers
 * Used to optionally increment a NPC_AF_MATCH_STAT() counter when a packet
 * matches an MCAM entry. See also NPC_AF_MCAME()_BANK()_ACTION.
 */
union cavm_npc_af_mcamex_bankx_stat_act {
	u64 u;
	struct npc_af_mcamex_bankx_stat_act_s {
		u64 stat_sel:   9;
		u64 ena:        1;
		u64 rsvd_63_10: 54;
	} s;
};

/**
 * NPC AF Match Statistics Registers
 */
union cavm_npc_af_match_statx {
	u64 u;
	struct npc_af_match_statx_s {
		u64 count:      48;
		u64 rsvd_63_48: 16;
	} s;
};

/**
 * NPC AF MCAM Entry Bank Action Data Registers
 * Specifies a packet's match action captured in NPC_RESULT_S[ACTION]. When an
 * interface is configured to use the NPC_MCAM_KEY_X2_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2), *
 * NPC_AF_MCAME()_BANK(0)_ACTION/_TAG_ACT/_STAT_ACT are used if the search key
 * matches NPC_AF_MCAME()_BANK(0..1)_CAM()_W*. *
 * NPC_AF_MCAME()_BANK(2)_ACTION/_TAG_ACT/_STAT_ACT are used if the search key
 * matches NPC_AF_MCAME()_BANK(2..3)_CAM()_W*. *
 * NPC_AF_MCAME()_BANK(1,3)_ACTION/_TAG_ACT/_STAT_ACT are not used. When an
 * interface is configured to use the NPC_MCAM_KEY_X4_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4): *
 * NPC_AF_MCAME()_BANK(0)_ACTION/_TAG_ACT/_STAT_ACT are used if the search key
 * matches NPC_AF_MCAME()_BANK(0..3)_CAM()_W*. *
 * NPC_AF_MCAME()_BANK(1..3)_ACTION/_TAG_ACT/_STAT_ACT are not used.
 */
union cavm_npc_af_mcamex_bankx_action {
	u64 u;
	struct npc_af_mcamex_bankx_action_s {
		u64 action;                         
	} s;
};

/**
 * NPC AF MCAM Entry Bank VTag Action Data Registers
 * Specifies a packet's match Vtag action captured in
 * NPC_RESULT_S[VTAG_ACTION]. See also NPC_AF_MCAME()_BANK()_ACTION.
 */
union cavm_npc_af_mcamex_bankx_tag_act {
	u64 u;
	struct npc_af_mcamex_bankx_tag_act_s {
		u64 vtag_action;                    
	} s;
};

/**
 * NPC AF MCAM Bank Hit Registers
 */
union cavm_npc_af_mcam_bankx_hitx {
	u64 u;
	struct npc_af_mcam_bankx_hitx_s {
		u64 hit;                            
	} s;
};

/**
 * NPC AF Software Lookup Control Registers
 */
union cavm_npc_af_lkup_ctl {
	u64 u;
	struct npc_af_lkup_ctl_s {
		u64 intf:       2;
		u64 pkind:      6;
		u64 chan:       12;
		u64 hdr_sizem1: 8;
		u64 op:         3;
		u64 exec:       1;
		u64 rsvd_63_32: 32;
	} s;
};

/**
 * NPC AF Software Lookup Data Registers
 */
union cavm_npc_af_lkup_datax {
	u64 u;
	struct npc_af_lkup_datax_s {
		u64 data;                           
	} s;
};

/**
 * NPC AF Software Lookup Result Registers
 */
union cavm_npc_af_lkup_resultx {
	u64 u;
	struct npc_af_lkup_resultx_s {
		u64 data;                           
	} s;
};

/**
 * NPC AF Interface Statistics Registers
 * Statistics per interface. Index enumerated by NPC_INTF_E.
 */
union cavm_npc_af_intfx_stat {
	u64 u;
	struct npc_af_intfx_stat_s {
		u64 count:      48;
		u64 rsvd_63_48: 16;
	} s;
};

/**
 * NPC AF Debug Control Register
 * This register controls the capture of debug information in
 * NPC_AF_KPU()_DBG, NPC_AF_MCAM_DBG, NPC_AF_DBG_DATA() and
 * NPC_AF_DBG_RESULT().
 */
union cavm_npc_af_dbg_ctl {
	u64 u;
	struct npc_af_dbg_ctl_s {
		u64 continuous: 1;
		u64 lkup_dbg:   1;
		u64 intf_dbg:   4;
		u64 rsvd_63_6:  58;
	} s;
};

/**
 * NPC AF Debug Status Register
 * This register controls the capture of debug information in
 * NPC_AF_KPU()_DBG, NPC_AF_MCAM_DBG, NPC_AF_LKUP_DATA() and
 * NPC_AF_LKUP_RESULT().
 */
union cavm_npc_af_dbg_status {
	u64 u;
	struct npc_af_dbg_status_s {
		u64 done:       1;
		u64 rsvd_63_1:  63;
	} s;
};

/**
 * NPC AF KPU Debug Registers
 * This register contains information for the last packet/lookup for which
 * debug is enabled by NPC_AF_DBG_CTL[INTF_DBG,LKUP_DBG].
 */
union cavm_npc_af_kpux_dbg {
	u64 u;
	struct npc_af_kpux_dbg_s {
		u64 hit_entry:  8;
		u64 byp:        1;
		u64 rsvd_63_9:  55;
	} s;
};

/**
 * NPC AF KPU Error Control Registers
 * This register specifies values captured in NPC_RESULT_S[ERRLEV,ERRCODE]
 * when errors are detected by a KPU.
 */
union cavm_npc_af_kpux_err_ctl {
	u64 u;
	struct npc_af_kpux_err_ctl_s {
		u64 errlev:                     4;
		u64 dp_offset_errcode:          8;
		u64 ptr_advance_errcode:        8;
		u64 var_len_offset_errcode:     8;
		u64 rsvd_63_28:                 36;
	} s;
};

/**
 * NPC AF MCAM Debug Register
 * This register contains information for the last packet/lookup for which
 * debug is enabled by NPC_AF_DBG_CTL[INTF_DBG,LKUP_DBG].
 */
union cavm_npc_af_mcam_dbg {
	u64 u;
	struct npc_af_mcam_dbg_s {
		u64 hit_entry:  10;
		u64 rsvd_11_10: 2;
		u64 hit_bank:   2;
		u64 rsvd_15_14: 2;
		u64 miss:       1;
		u64 rsvd_63_17: 47;
	} s;
};

/**
 * NPC AF Debug Data Registers
 * This register contains packet header data for the last packet/lookup for
 * which debug information is captured by NPC_AF_DBG_CTL[INTF_DBG,LKUP_DBG].
 */
union cavm_npc_af_dbg_datax {
	u64 u;
	struct npc_af_dbg_datax_s {
		u64 data;                           
	} s;
};
