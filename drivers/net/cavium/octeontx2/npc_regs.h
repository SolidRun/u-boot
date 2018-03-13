/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This file defines the NPC registers for the Cavium OcteonTX2.
 */

#ifndef __NPC_REG_H__
#define __NPC_REG_H__

/**
 * Enumeration npc_intf_e
 *
 * NPC Interface Enumeration
 * Enumerates the NPC interfaces.
 */
/**
 * Enumeration npc_errlev_e
 *
 * NPC Error Level Enumeration
 * Enumerates the lowest protocol layer containing an error.
 */
#define CAVM_NPC_ERRLEV_E_LA (1)
#define CAVM_NPC_ERRLEV_E_LB (2)
#define CAVM_NPC_ERRLEV_E_LC (3)
#define CAVM_NPC_ERRLEV_E_LD (4)
#define CAVM_NPC_ERRLEV_E_LE (5)
#define CAVM_NPC_ERRLEV_E_LF (6)
#define CAVM_NPC_ERRLEV_E_LG (7)
#define CAVM_NPC_ERRLEV_E_LH (8)
#define CAVM_NPC_ERRLEV_E_NIX (0xf)
#define CAVM_NPC_ERRLEV_E_RX(a) (0 + (a))
#define CAVM_NPC_ERRLEV_E_RE (0)

/**
 * Enumeration npc_intf_e
 *
 * NPC Interface Enumeration
 * Enumerates the NPC interfaces.
 */
#define CAVM_NPC_INTF_E_NIXX_RX(a) (0 + 2 * (a))
#define CAVM_NPC_INTF_E_NIXX_TX(a) (1 + 2 * (a))

/**
 * Enumeration npc_lid_e
 *
 * NPC Layer ID Enumeration
 * Enumerates layers parsed by NPC.
 */
#define CAVM_NPC_LID_E_LA (0)
#define CAVM_NPC_LID_E_LB (1)
#define CAVM_NPC_LID_E_LC (2)
#define CAVM_NPC_LID_E_LD (3)
#define CAVM_NPC_LID_E_LE (4)
#define CAVM_NPC_LID_E_LF (5)
#define CAVM_NPC_LID_E_LG (6)
#define CAVM_NPC_LID_E_LH (7)

/**
 * Enumeration npc_lkupop_e
 *
 * NPC Lookup Operation Enumeration
 * Enumerates the lookup operation for NPC_AF_LKUP_CTL[OP].
 */
#define CAVM_NPC_LKUPOP_E_KEY (1)
#define CAVM_NPC_LKUPOP_E_PKT (0)

/**
 * Enumeration npc_mcamkeyw_e
 *
 * NPC MCAM Search Key Width Enumeration
 */
#define CAVM_NPC_MCAMKEYW_E_X1 (0)
#define CAVM_NPC_MCAMKEYW_E_X2 (1)
#define CAVM_NPC_MCAMKEYW_E_X4 (2)

#define CAVM_NPC_AF_CFG			(0x0000)
#define CAVM_NPC_AF_ACTIVE_PC		(0x0010)
#define CAVM_NPC_AF_CONST		(0x0020)
#define CAVM_NPC_AF_CONST1		(0x0030)
#define CAVM_NPC_AF_BLK_RST		(0x0040)
#define CAVM_NPC_AF_DV_FC_SCRATCH	(0x0060)
#define CAVM_NPC_AF_MCAM_SCRUB_CTL	(0x00a0)
#define CAVM_NPC_AF_KCAM_SCRUB_CTL	(0x00b0)

#define CAVM_NPC_AF_ECO0		(0x0200)
#define CAVM_NPC_AF_KPUX_CFG(x)		(0x0500 + ((x) & 0xf) * 0x8)
#define CAVM_NPC_AF_PCK_CFG		(0x0600)
#define CAVM_NPC_AF_PCK_DEF_OL2		(0x0610)
#define CAVM_NPC_AF_PCK_DEF_OIP4	(0x0620)
#define CAVM_NPC_AF_PCK_DEF_OIP6	(0x0630)
#define CAVM_NPC_AF_PCK_DEF_IIP4	(0x0640)
#define CAVM_NPC_AF_KEX_LDATAX_FLAGS_CFG(x)	(0x0800 + ((x) & 0x1) * 0x8)
#define CAVM_NPC_AF_INTFX_CFG(x)	(0x1000 + ((x) & 0x1) * 0x100)
#define CAVM_NPC_AF_INTFX_KEX_CFG	(0x1010 + ((x) & 0x1) * 0x100)

#define CAVM_NPC_AF_PKINDX_ACTION0(x)			\
		(0x0080000 + ((x) & 0x3f) * 0x40ll)
#define CAVM_NPC_AF_PKINDX_ACTION1(x)			\
		(0x0080008 + ((x) & 0x3f) * 0x40ll)
#define CAVM_NPC_AF_PKINDX_CPI_DEFX(a, b)		\
		(0x0080020 + ((a) & 0x3f) * 0x40 + ((b) & 0x1) * 0x8)

#define CAVM_NPC_AF_KPUX_ENTRYX_CAMX(a, b, c)		\
		(0x100000 + ((a) & 0xf) * 0x4000 +	\
		 ((b) & 0x7f) * 0x40 + ((c) & 0x1) * 0x8)
#define CAVM_NPC_AF_KPUX_ENTRYX_ACTION0(a, b)		\
		(0x100020 + ((a) & 0xf) * 0x4000 + ((b) & 0x3f) * 0x40)
#define CAVM_NPC_AF_KPUX_ENTRYX_ACTION1(a, b)		\
		(0x100028 + ((a) & 0xf) * 0x4000 + ((b) & 0x3f) * 0x40)
#define CAVM_NPC_AF_KPUX_ENTRY_DISX(a, b)		\
		(0x180000 + ((a) & 0xf) + ((b) & 0x1) * 0x8)

#define CAVM_NPC_AF_CPIX_CFG(x)		(0x200000 + ((x) & 0x3ff) * 0x8)

#define CAVM_NPC_AF_INTFX_LIDX_LTX_LDX_CFG(a, b, c, d)	\
		(0x900000 + ((a) & 0x1) * 0x10000 +	\
		 ((b) & 0x7) * 0x1000 + ((c) & 0xf) * 0x20 + ((d) & 0x1) * 0x8)
#define CAVM_NPC_AF_INTFX_LDATAX_FLAGSX_CFG(a, b, c)	\
		(0x980000 + ((a) & 0x1) * 0x10000 +	\
		 ((b) & 0x1) * 0x1000 + ((c) & 0xf) * 0x8)
#define CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(a, b, c)	\
		(0x1000000 + 0x400 * ((a) & 0x3ff) +	\
		 0x40 * ((b) & 0x3) + 8 * ((c) & 0x1))
#define CAVM__NPC_AF_MCAMEX_BANKX_CAMX_W0(a, b, c)	\
		(0x1000010 + 0x400 * ((a) & 0x3ff) +	\
		 0x40 * ((b) & 0x3) + 8 * ((c) & 0x1))
#define CAVM__NPC_AF_MCAMEX_BANKX_CAMX_W1(a, b, c)	\
		(0x1000020 + 0x400 * ((a) & 0x3ff) +	\
		 0x40 * ((b) & 0x3) + 8 * ((c) & 0x1))
#define CAVM_NPC_AF_INTFX_MISS_ACT(x)			\
		(0x1a00000 + ((x) & 0x1) * 0x10)
/**
 * Register (RVU_PF_BAR0) npc_af_pkind#_action0
 *
 * NPC AF Port Kind Action Data 0 Registers
 * NPC_AF_PKIND()_ACTION0 and NPC_AF_PKIND()_ACTION1 specify the initial parse state and
 * operations to perform before entering KPU 0.
 */
union cavm_npc_af_pkindx_action0 {
	u64 u;
	struct cavm_npc_af_pkindx_action0_s {
		u64 var_len_shift         : 3;  /**< [  2:  0](R/W) Variable length shift size in bits. See [VAR_LEN_OFFSET]. */
		u64 var_len_right         : 1;  /**< [  3:  3](R/W) Variable length shift direction.
                                                                 0 = Left shift.
                                                                 1 = Right shift. */
		u64 var_len_mask          : 8;  /**< [ 11:  4](R/W) Variable length mask. See [VAR_LEN_OFFSET]. */
		u64 var_len_offset        : 8;  /**< [ 19: 12](R/W) Variable length byte offset. When [VAR_LEN_MASK] is nonzero, byte offset
                                                                 from current header pointer of the packet byte that supplies a variable
                                                                 pointer advance value.

                                                                 The pointer advance algorithm is as follows:

                                                                 \<pre\>
                                                                 var_len_byte = packet byte at (NPC_RESULT_S[EOH_PTR] + [VAR_LEN_OFFSET]);
                                                                 masked_var_len_byte = var_len_byte & [VAR_LEN_MASK];

                                                                 if ([VAR_LEN_RIGHT])
                                                                    var_len_advance = masked_var_len_byte \>\> [VAR_LEN_SHIFT];
                                                                 else
                                                                    var_len_advance = masked_var_len_byte \<\< [VAR_LEN_SHIFT];

                                                                 NPC_RESULT_S[EOH_PTR] += ([PTR_ADVANCE] + var_len_advance);
                                                                 \</pre\>

                                                                 NPC_RESULT_S[EOH_PTR] must always be even. Therefore,
                                                                 [VAR_LEN_SHIFT], [VAR_LEN_RIGHT] and [VAR_LEN_MASK] must produce an
                                                                 even var_len_advance value. */
		u64 ptr_advance           : 8;  /**< [ 27: 20](R/W) Pointer advance. Fixed value added to NPC_RESULT_S[EOH_PTR]. Must be
                                                                 even. See also [VAR_LEN_OFFSET]. */
		u64 capture_flags         : 8;  /**< [ 35: 28](R/W) Capture flags. When nonzero, specifies which NPC_LAYER_INFO_S[FLAGS] bits
                                                                 to set in the approprate layer within
                                                                 NPC_RESULT_S, as follows:
                                                                 _ NPC_LAYER_INFO_S[FLAGS] |= [CAPTURE_FLAGS]

                                                                 Note that flags are captured irrespective of the [CAPTURE_ENA] value. */
		u64 capture_ltype         : 4;  /**< [ 39: 36](R/W) Capture layer type. When [CAPTURE_ENA] is set, specifies
                                                                 NPC_LAYER_INFO_S[LTYPE] value captured in the approprate layer within
                                                                 NPC_RESULT_S. */
		u64 capture_lid           : 3;  /**< [ 42: 40](R/W) Capture layer ID. Specifies the layer for which information is captured in
                                                                 NPC_RESULT_S. Enumerated by NPC_LID_E. */
		u64 reserved_43           : 1;
		u64 next_state            : 8;  /**< [ 51: 44](R/W) Search value for ternary comparison with the next KPU's
                                                                 NPC_AF_KPU()_ENTRY()_CAM()[STATE]. */
		u64 parse_done            : 1;  /**< [ 52: 52](R/W) Parse done. When set, terminate parse after this KPU and bypass subsequent KPUs. */
		u64 capture_ena           : 1;  /**< [ 53: 53](R/W) Layer capture enable. When set, layer information is captured in
                                                                 NPC_RESULT_S. When clear, layer information is not captured by the KPU. */
		u64 byp_count             : 3;  /**< [ 56: 54](R/W) Bypass count. When nonzero, specifies the number of enabled KPUs to be
                                                                 bypassed. For example, if the bypass count is two in the matching entry for
                                                                 KPU 3, NPC_AF_KPU(4,6,7)_CFG[ENA] = 1 and NPC_AF_KPU(5)_CFG[ENA] = 0, then:
                                                                 * KPUs 4 and 6 are bypassed.
                                                                 * The matching entry's [NEXT_STATE] and
                                                                 NPC_AF_KPU()_ENTRY()_ACTION1[DP*_OFFSET] in KPU 3 are used for the lookup in
                                                                 KPU 7. */
		u64 reserved_57_63        : 7;
	} s;
	/* struct cavm_npc_af_pkindx_action0_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_miss_act
 *
 * NPC AF Interface MCAM Miss Action Data Registers
 * When a combination of NPC_AF_MCAME()_BANK()_CAM()_* and
 * NPC_AF_MCAME()_BANK()_CFG[ENA] yields an MCAM miss for a packet, this
 * register specifies the packet's match action captured in NPC_RESULT_S[ACTION].
 */
union cavm_npc_af_intfx_miss_act {
	u64 u;
	struct cavm_npc_af_intfx_miss_act_s {
		u64 action                : 64; /**< [ 63:  0](R/W) Match action. Format is NIX_RX_ACTION_S for RX packet, NIX_TX_ACTION_S for
                                                                 TX packet. */
	} s;
	/* struct cavm_npc_af_intfx_miss_act_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_intf
 *
 * NPC AF MCAM Entry Bank CAM Data Interface Registers
 * MCAM comparison ternary data interface word. The field values in
 * NPC_AF_MCAME()_BANK()_CAM()_INTF, NPC_AF_MCAME()_BANK()_CAM()_W0 and
 * NPC_AF_MCAME()_BANK()_CAM()_W1 are ternary, where  each data bit of the
 * search key matches as follows:
 * _ [CAM(1)]\<n\>=0, [CAM(0)]\<n\>=0: Always match; search key data\<n\> don't care.
 * _ [CAM(1)]\<n\>=0, [CAM(0)]\<n\>=1: Match when search key data\<n\> == 0.
 * _ [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=0: Match when search key data\<n\> == 1.
 * _ [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=1: Reserved.
 *
 * The reserved combination is not allowed. Hardware suppresses any write to
 * CAM(0) or CAM(1) that would result in the reserved combination for any CAM bit.
 *
 * When an interface is configured to use the NPC_MCAM_KEY_X1_S search key
 * format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X1), the four banks
 * of every MCAM entry are used as individual entries, each of which is
 * independently compared with the search key as follows:
 * _ NPC_AF_MCAME()_BANK()_CAM()_INTF[INTF] corresponds to NPC_MCAM_KEY_X1_S[INTF].
 * _ NPC_AF_MCAME()_BANK()_CAM()_W0[MD] corresponds to NPC_MCAM_KEY_X1_S[KW0].
 * _ NPC_AF_MCAME()_BANK()_CAM()_W1[MD] corresponds to NPC_MCAM_KEY_X1_S[KW1].
 *
 * When an interface is configured to use the NPC_MCAM_KEY_X2_S search key
 * format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2), banks 0-1 of
 * every MCAM entry are used as one double-wide entry, banks 2-3 as a second
 * double-wide entry, and each double-wide entry is independently compared
 * with the search key as follows:
 * _ NPC_AF_MCAME()_BANK(0,2)_CAM()_INTF[INTF] corresponds to NPC_MCAM_KEY_X2_S[INTF].
 * _ NPC_AF_MCAME()_BANK(0,2)_CAM()_W0[MD] corresponds to NPC_MCAM_KEY_X2_S[KW0].
 * _ NPC_AF_MCAME()_BANK(0,2)_CAM()_W1[MD] corresponds to NPC_MCAM_KEY_X2_S[KW1\<47:0\>].
 * _ NPC_AF_MCAME()_BANK(1,3)_CAM()_INTF[INTF] corresponds to NPC_MCAM_KEY_X2_S[INTF].
 * _ NPC_AF_MCAME()_BANK(1,3)_CAM()_W0[MD\<15:0\>] corresponds to NPC_MCAM_KEY_X2_S[KW1\<63:48\>].
 * _ NPC_AF_MCAME()_BANK(1,3)_CAM()_W0[MD\<63:16\>] corresponds to NPC_MCAM_KEY_X2_S[KW2\<47:0\>].
 * _ NPC_AF_MCAME()_BANK(1,3)_CAM()_W1[MD\<15:0\>] corresponds to NPC_MCAM_KEY_X2_S[KW2\<63:48\>].
 * _ NPC_AF_MCAME()_BANK(1,3)_CAM()_W1[MD\<47:16\>] corresponds to NPC_MCAM_KEY_X2_S[KW3\<31:0\>].
 *
 * When an interface is configured to use the NPC_MCAM_KEY_X4_S search key
 * format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4), the four banks of every MCAM entry
 * are used as a single quad-wide entry that is compared with the search key as follows:
 * _ NPC_AF_MCAME()_BANK(0)_CAM()_INTF[INTF] corresponds to NPC_MCAM_KEY_X4_S[INTF].
 * _ NPC_AF_MCAME()_BANK(0)_CAM()_W0[MD] corresponds to NPC_MCAM_KEY_X4_S[KW0].
 * _ NPC_AF_MCAME()_BANK(0)_CAM()_W1[MD] corresponds to NPC_MCAM_KEY_X4_S[KW1\<47:0\>].
 * _ NPC_AF_MCAME()_BANK(1)_CAM()_INTF[INTF] corresponds to NPC_MCAM_KEY_X4_S[INTF].
 * _ NPC_AF_MCAME()_BANK(1)_CAM()_W0[MD\<15:0\>] corresponds to NPC_MCAM_KEY_X4_S[KW1\<63:48\>].
 * _ NPC_AF_MCAME()_BANK(1)_CAM()_W0[MD\<63:16\>] corresponds to NPC_MCAM_KEY_X4_S[KW2\<47:0\>].
 * _ NPC_AF_MCAME()_BANK(1)_CAM()_W1[MD\<15:0\>] corresponds to NPC_MCAM_KEY_X4_S[KW2\<63:48\>].
 * _ NPC_AF_MCAME()_BANK(1)_CAM()_W1[MD\<47:16\>] corresponds to NPC_MCAM_KEY_X4_S[KW3\<31:0\>].
 * _ NPC_AF_MCAME()_BANK(2)_CAM()_INTF[INTF] corresponds to NPC_MCAM_KEY_X4_S[INTF].
 * _ NPC_AF_MCAME()_BANK(2)_CAM()_W0[MD\<31:0\>] corresponds to NPC_MCAM_KEY_X4_S[KW3\<63:32\>].
 * _ NPC_AF_MCAME()_BANK(2)_CAM()_W0[MD\<63:32\>] corresponds to NPC_MCAM_KEY_X4_S[KW4\<31:0\>].
 * _ NPC_AF_MCAME()_BANK(2)_CAM()_W1[MD\<31:0\>] corresponds to NPC_MCAM_KEY_X4_S[KW4\<63:32\>].
 * _ NPC_AF_MCAME()_BANK(2)_CAM()_W1[MD\<47:32\>] corresponds to NPC_MCAM_KEY_X4_S[KW5\<15:0\>].
 * _ NPC_AF_MCAME()_BANK(3)_CAM()_INTF[INTF] corresponds to NPC_MCAM_KEY_X4_S[INTF].
 * _ NPC_AF_MCAME()_BANK(3)_CAM()_W0[MD\<47:0\>] corresponds to NPC_MCAM_KEY_X4_S[KW5\<63:16\>].
 * _ NPC_AF_MCAME()_BANK(3)_CAM()_W0[MD\<63:48\>] corresponds to NPC_MCAM_KEY_X4_S[KW6\<15:0\>].
 * _ NPC_AF_MCAME()_BANK(3)_CAM()_W1[MD] corresponds to NPC_MCAM_KEY_X4_S[KW6\<63:16\>]
 *
 * Note that for the X2 and X4 formats, a wide entry will not match unless
 * the INTF fields from the associated two or four banks match the INTF
 * value from the search key.
 *
 * For the X1 and X2 formats, a match in a lower-numbered bank takes priority over
 * a match in any higher numbered banks. Within each bank, the lowest numbered
 * matching entry takes priority over any higher numbered entry.
 */
union cavm_npc_af_mcamex_bankx_camx_intf {
	u64 u;
	struct cavm_npc_af_mcamex_bankx_camx_intf_s {
		u64 intf                  : 2;  /**< [  1:  0](R/W) NPC interface. Enumerated by NPC_INTF_E. */
		u64 reserved_2_63         : 62;
	} s;
	/* struct cavm_npc_af_mcamex_bankx_camx_intf_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_w0
 *
 * NPC AF MCAM Entry Bank CAM Data Word 0 Registers
 * MCAM comparison ternary data word 0. See NPC_AF_MCAME()_BANK()_CAM()_INTF.
 */
union cavm_npc_af_mcamex_bankx_camx_w0 {
	u64 u;
	struct cavm_npc_af_mcamex_bankx_camx_w0_s {
		u64 md                    : 64; /**< [ 63:  0](R/W) Match data. */
	} s;
	/* struct cavm_npc_af_mcamex_bankx_camx_w0_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_w1
 *
 * NPC AF MCAM Entry Bank Data Word 1 Registers
 * MCAM comparison ternary data word 1. See NPC_AF_MCAME()_BANK()_CAM()_INTF.
 */
union cavm_npc_af_mcamex_bankx_camx_w1 {
	u64 u;
	struct cavm_npc_af_mcamex_bankx_camx_w1_s {
		u64 md                    : 48; /**< [ 47:  0](R/W) Match data. */
		u64 reserved_48_63        : 16;
	} s;
	/* struct cavm_npc_af_mcamex_bankx_camx_w1_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cfg
 *
 * NPC AF MCAM Entry Bank Configuration Registers
 */
union cavm_npc_af_mcamex_bankx_cfg {
	u64 u;
	struct cavm_npc_af_mcamex_bankx_cfg_s {
		u64 ena                   : 1;  /**< [  0:  0](R/W) Enable entry. When clear, the entry is disabled and may be safely modified
                                                                 by software. */
		u64 reserved_1_63         : 63;
	} s;
	/* struct cavm_npc_af_mcamex_bankx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_kex_cfg
 *
 * NPC AF Interface Key Extract Configuration Registers
 */
union cavm_npc_af_intfx_kex_cfg {
	u64 u;
	struct cavm_npc_af_intfx_kex_cfg_s {
		u64 parse_nibble_ena      : 31; /**< [ 30:  0](R/W) Parse key extract nibble enable. Enable bit for each nibble in
                                                                 NPC_PARSE_KEX_S to be included in the MCAM search key: bit 0 for
                                                                 NPC_PARSE_KEX_S[CHAN\<3:0\>], bit 1 for NPC_PARSE_KEX_S[CHAN\<7:4\>], ..., bit
                                                                 30 for NPC_PARSE_KEX_S[LH\<11..8\>]. The extracted nibbles are concatenated
                                                                 and and written to NPC_MCAM_KEY_X*_S, with the first extracted nibble
                                                                 written to the least significant nibble of the key (NPC_MCAM_KEY_X*_S[KW0\<3:0\>]). */
		u64 reserved_31           : 1;
		u64 keyw                  : 3;  /**< [ 34: 32](R/W) MCAM search key width selection for the interface. Enumerated by NPC_MCAMKEYW_E. */
		u64 reserved_35_63        : 29;
	} s;
	/* struct cavm_npc_af_intfx_kex_cfg_s cn; */
};

#endif /* __NPC_REG_H__ */
