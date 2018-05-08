/* This file is auto-generated. Do not edit */

/***********************license start***************
 * Copyright (c) 2003-2018  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export
 * control laws, including the U.S. Export Administration Act and its
 * associated regulations, and may be subject to export or import regulations
 * in other countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM NETWORKS MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
 * REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
 * DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
 * PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT,
 * QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

#ifndef __NPC_HW_H__
#define __NPC_HW_H__

/* Register offsets */

#define CAVM_NPC_AF_CFG                                   (0x0ull)
#define CAVM_NPC_AF_ACTIVE_PC                             (0x10ull)
#define CAVM_NPC_AF_CONST                                 (0x20ull)
#define CAVM_NPC_AF_CONST1                                (0x30ull)
#define CAVM_NPC_AF_BLK_RST                               (0x40ull)
#define CAVM_NPC_AF_MCAM_SCRUB_CTL                        (0xa0ull)
#define CAVM_NPC_AF_KCAM_SCRUB_CTL                        (0xb0ull)
#define CAVM_NPC_AF_KPUX_CFG(a)                           \
	(0x500ull | (u64)(a) << 3)
#define CAVM_NPC_AF_PCK_CFG                               (0x600ull)
#define CAVM_NPC_AF_PCK_DEF_OL2                           (0x610ull)
#define CAVM_NPC_AF_PCK_DEF_OIP4                          (0x620ull)
#define CAVM_NPC_AF_PCK_DEF_OIP6                          (0x630ull)
#define CAVM_NPC_AF_PCK_DEF_IIP4                          (0x640ull)
#define CAVM_NPC_AF_KEX_LDATAX_FLAGS_CFG(a)               \
	(0x800ull | (u64)(a) << 3)
#define CAVM_NPC_AF_INTFX_KEX_CFG(a)                      \
	(0x1010ull | (u64)(a) << 8)
#define CAVM_NPC_AF_PKINDX_ACTION0(a)                     \
	(0x80000ull | (u64)(a) << 6)
#define CAVM_NPC_AF_PKINDX_ACTION1(a)                     \
	(0x80008ull | (u64)(a) << 6)
#define CAVM_NPC_AF_PKINDX_CPI_DEFX(a, b)                 \
	(0x80020ull | (u64)(a) << 6 | (u64)(b) << 3)
#define CAVM_NPC_AF_KPUX_ENTRYX_CAMX(a, b, c)             \
	(0x100000ull | (u64)(a) << 14 | (u64)(b) << 6 | (u64)(c) << 3)
#define CAVM_NPC_AF_KPUX_ENTRYX_ACTION0(a, b)             \
	(0x100020ull | (u64)(a) << 14 | (u64)(b) << 6)
#define CAVM_NPC_AF_KPUX_ENTRYX_ACTION1(a, b)             \
	(0x100028ull | (u64)(a) << 14 | (u64)(b) << 6)
#define CAVM_NPC_AF_KPUX_ENTRY_DISX(a, b)                 \
	(0x180000ull | (u64)(a) << 6 | (u64)(b) << 3)
#define CAVM_NPC_AF_CPIX_CFG(a)                           \
	(0x200000ull | (u64)(a) << 3)
#define CAVM_NPC_AF_INTFX_LIDX_LTX_LDX_CFG(a, b, c, d)    \
	(0x900000ull | (u64)(a) << 16 | (u64)(b) << 12 | (u64)(c) << 5 | \
	(u64)(d) << 3)
#define CAVM_NPC_AF_INTFX_LDATAX_FLAGSX_CFG(a, b, c)      \
	(0x980000ull | (u64)(a) << 16 | (u64)(b) << 12 | (u64)(c) << 3)
#define CAVM_NPC_AF_MCAMEX_BANKX_CAMX_INTF(a, b, c)       \
	(0x1000000ull | (u64)(a) << 10 | (u64)(b) << 6 | (u64)(c) << 3)
#define CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W0(a, b, c)         \
	(0x1000010ull | (u64)(a) << 10 | (u64)(b) << 6 | (u64)(c) << 3)
#define CAVM_NPC_AF_MCAMEX_BANKX_CAMX_W1(a, b, c)         \
	(0x1000020ull | (u64)(a) << 10 | (u64)(b) << 6 | (u64)(c) << 3)
#define CAVM_NPC_AF_MCAMEX_BANKX_CFG(a, b)                \
	(0x1800000ull | (u64)(a) << 8 | (u64)(b) << 4)
#define CAVM_NPC_AF_MCAMEX_BANKX_STAT_ACT(a, b)           \
	(0x1880000ull | (u64)(a) << 8 | (u64)(b) << 4)
#define CAVM_NPC_AF_MATCH_STATX(a)                        \
	(0x1880008ull | (u64)(a) << 8)
#define CAVM_NPC_AF_INTFX_MISS_STAT_ACT(a)                \
	(0x1880040ull + (u64)(a) * 0x8)
#define CAVM_NPC_AF_MCAMEX_BANKX_ACTION(a, b)             \
	(0x1900000ull | (u64)(a) << 8 | (u64)(b) << 4)
#define CAVM_NPC_AF_MCAMEX_BANKX_TAG_ACT(a, b)            \
	(0x1900008ull | (u64)(a) << 8 | (u64)(b) << 4)
#define CAVM_NPC_AF_INTFX_MISS_ACT(a)                     \
	(0x1a00000ull | (u64)(a) << 4)
#define CAVM_NPC_AF_INTFX_MISS_TAG_ACT(a)                 \
	(0x1b00008ull | (u64)(a) << 4)
#define CAVM_NPC_AF_MCAM_BANKX_HITX(a, b)                 \
	(0x1c80000ull | (u64)(a) << 8 | (u64)(b) << 4)
#define CAVM_NPC_AF_LKUP_CTL                              (0x2000000ull)
#define CAVM_NPC_AF_LKUP_DATAX(a)                         \
	(0x2000200ull | (u64)(a) << 4)
#define CAVM_NPC_AF_LKUP_RESULTX(a)                       \
	(0x2000400ull | (u64)(a) << 4)
#define CAVM_NPC_AF_INTFX_STAT(a)                         \
	(0x2000800ull | (u64)(a) << 4)
#define CAVM_NPC_AF_DBG_CTL                               (0x3000000ull)
#define CAVM_NPC_AF_DBG_STATUS                            (0x3000010ull)
#define CAVM_NPC_AF_KPUX_DBG(a)                           \
	(0x3000020ull | (u64)(a) << 8)
#define CAVM_NPC_AF_IKPU_ERR_CTL                          (0x3000080ull)
#define CAVM_NPC_AF_KPUX_ERR_CTL(a)                       \
	(0x30000a0ull | (u64)(a) << 8)
#define CAVM_NPC_AF_MCAM_DBG                              (0x3001000ull)
#define CAVM_NPC_AF_DBG_DATAX(a)                          \
	(0x3001400ull | (u64)(a) << 4)
#define CAVM_NPC_AF_DBG_RESULTX(a)                        \
	(0x3001800ull | (u64)(a) << 4)


/* Enum offsets */

#define CAVM_NPC_INTF_NIX0_RX    (0x0ull)
#define CAVM_NPC_INTF_NIX0_TX    (0x1ull)

#define CAVM_NPC_ERRLEV_RE       (0x0ull)
#define CAVM_NPC_ERRLEV_LA       (0x1ull)
#define CAVM_NPC_ERRLEV_LB       (0x2ull)
#define CAVM_NPC_ERRLEV_LC       (0x3ull)
#define CAVM_NPC_ERRLEV_LD       (0x4ull)
#define CAVM_NPC_ERRLEV_LE       (0x5ull)
#define CAVM_NPC_ERRLEV_LF       (0x6ull)
#define CAVM_NPC_ERRLEV_LG       (0x7ull)
#define CAVM_NPC_ERRLEV_LH       (0x8ull)
#define CAVM_NPC_ERRLEV_NIX      (0xfull)
#define CAVM_NPC_ERRLEV_R9       (0x9ull)
#define CAVM_NPC_ERRLEV_R10      (0xaull)
#define CAVM_NPC_ERRLEV_R11      (0xbull)
#define CAVM_NPC_ERRLEV_R12      (0xcull)
#define CAVM_NPC_ERRLEV_R13      (0xdull)
#define CAVM_NPC_ERRLEV_R14      (0xeull)

#define CAVM_NPC_LKUPOP_PKT      (0x0ull)
#define CAVM_NPC_LKUPOP_KEY      (0x1ull)

#define CAVM_NPC_LID_LA          (0x0ull)
#define CAVM_NPC_LID_LB          (0x1ull)
#define CAVM_NPC_LID_LC          (0x2ull)
#define CAVM_NPC_LID_LD          (0x3ull)
#define CAVM_NPC_LID_LE          (0x4ull)
#define CAVM_NPC_LID_LF          (0x5ull)
#define CAVM_NPC_LID_LG          (0x6ull)
#define CAVM_NPC_LID_LH          (0x7ull)

#define CAVM_NPC_MCAMKEYW_X1     (0x0ull)
#define CAVM_NPC_MCAMKEYW_X2     (0x1ull)
#define CAVM_NPC_MCAMKEYW_X4     (0x2ull)


/* Structures definitions */

/**
 * NPC Layer Parse Information Structure
 * This structure specifies the format of NPC_RESULT_S[LA,LB,...,LH].
 */
union cavm_npc_layer_info_s {
	u32 u;
	struct npc_layer_info_s_s {
	
		u32 lptr:       8;
		u32 flags:      8;
		u32 ltype:      4;
		u32 rsvd_31_20: 12;
	} s;
};

/**
 * NPC Layer MCAM Search Key Extract Structure
 * This structure specifies the format of each of the
 * NPC_PARSE_KEX_S[LA,LB,...,LH] fields. It contains the subset of
 * NPC_LAYER_INFO_S fields that can be included in the MCAM search key. See
 * NPC_PARSE_KEX_S and NPC_AF_INTF()_KEX_CFG.
 */
union cavm_npc_layer_kex_s {
	u16 u;
	struct npc_layer_kex_s_s {
	
		u16 flags:      8;
		u16 ltype:      4;
		u16 rsvd_15_12: 4;
	} s;
};

/**
 * NPC MCAM Search Key X1 Structure
 * This structure specifies the MCAM search key format used by an interface
 * when NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X1.
 */
union cavm_npc_mcam_key_x1_s {
	u64 u[3];
	struct npc_mcam_key_x1_s_s {
		/* Word 0 */
		u64 intf:               2;
		u64 rsvd_63_2:          62;
		u64 kw0;               	/* Word 1 */
		/* Word 2 */
		u64 kw1:                48;
		u64 rsvd_191_176:       16;
	} s;
};

/**
 * NPC MCAM Search Key X2 Structure
 * This structure specifies the MCAM search key format used by an interface
 * when NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2.
 */
union cavm_npc_mcam_key_x2_s {
	u64 u[5];
	struct npc_mcam_key_x2_s_s {
		/* Word 0 */
		u64 intf:               2;
		u64 rsvd_63_2:          62;
		u64 kw0;               	/* Word 1 */
		u64 kw1;               	/* Word 2 */
		u64 kw2;               	/* Word 3 */
		/* Word 4 */
		u64 kw3:                32;
		u64 rsvd_319_288:       32;
	} s;
};

/**
 * NPC MCAM Search Key X4 Structure
 * This structure specifies the MCAM search key format used by an interface
 * when NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4.
 */
union cavm_npc_mcam_key_x4_s {
	u64 u[8];
	struct npc_mcam_key_x4_s_s {
		/* Word 0 */
		u64 intf:       2;
		u64 rsvd_63_2:  62;
		u64 kw0;               	/* Word 1 */
		u64 kw1;               	/* Word 2 */
		u64 kw2;               	/* Word 3 */
		u64 kw3;               	/* Word 4 */
		u64 kw4;               	/* Word 5 */
		u64 kw5;               	/* Word 6 */
		u64 kw6;               	/* Word 7 */
	} s;
};

/**
 * NPC Parse Key Extract Structure
 * This structure contains the subset of NPC_RESULT_S fields that can be
 * included in the MCAM search key. See NPC_AF_INTF()_KEX_CFG.
 */
union cavm_npc_parse_kex_s {
	u64 u[2];
	struct npc_parse_kex_s_s {
		/* Word 0 */
		u64 chan:               12;
		u64 errlev:             4;
		u64 errcode:            8;
		u64 l2m:                1;
		u64 l2b:                1;
		u64 l3m:                1;
		u64 l3b:                1;
		u64 la:                 12;
		u64 lb:                 12;
		u64 lc:                 12;
		/* Word 1 */
		u64 ld:                 12;
		u64 le:                 12;
		u64 lf:                 12;
		u64 lg:                 12;
		u64 lh:                 12;
		u64 rsvd_127_124:       4;
	} s;
};

/**
 * NPC Result Structure
 * This structure contains a packet's parse and flow identification
 * information.
 */
union cavm_npc_result_s {
	u64 u[6];
	struct npc_result_s_s {
		/* Word 0 */
		u64 intf:               2;
		u64 pkind:              6;
		u64 chan:               12;
		u64 errlev:             4;
		u64 errcode:            8;
		u64 l2m:                1;
		u64 l2b:                1;
		u64 l3m:                1;
		u64 l3b:                1;
		u64 eoh_ptr:            8;
		u64 rsvd_63_44:         20;
		u64 action;            	/* Word 1 */
		u64 vtag_action;       	/* Word 2 */
		/* Word 3 */
		u64 la:                 20;
		u64 lb:                 20;
		u64 lc:                 20;
		u64 rsvd_255_252:       4;
		/* Word 4 */
		u64 ld:                 20;
		u64 le:                 20;
		u64 lf:                 20;
		u64 rsvd_319_316:       4;
		/* Word 5 */
		u64 lg:                 20;
		u64 lh:                 20;
		u64 rsvd_383_360:       24;
	} s;
};

#endif /* __NPC_HW_H__ */
