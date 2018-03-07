/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This file defines the NIX registers for the Cavium OcteonTX2.
 */

#ifndef __NIX_REG_H__
#define __NIX_REG_H__

/**
 * Enumeration nix_af_int_vec_e
 *
 * NIX Admin Function Interrupt Vector Enumeration
 * Enumerates the NIX AF MSI-X interrupt vectors.
 */
#define CAVM_NIX_AF_INT_VEC_E_AF_ERR (3)
#define CAVM_NIX_AF_INT_VEC_E_AQ_DONE (2)
#define CAVM_NIX_AF_INT_VEC_E_GEN (1)
#define CAVM_NIX_AF_INT_VEC_E_POISON (4)
#define CAVM_NIX_AF_INT_VEC_E_RVU (0)

/**
 * Enumeration nix_aq_comp_e
 *
 * NIX Completion Enumeration
 * Enumerates the values of NIX_AQ_RES_S[COMPCODE].
 */
#define CAVM_NIX_AQ_COMP_E_CTX_FAULT (4)
#define CAVM_NIX_AQ_COMP_E_CTX_POISON (3)
#define CAVM_NIX_AQ_COMP_E_GOOD (1)
#define CAVM_NIX_AQ_COMP_E_LOCKERR (5)
#define CAVM_NIX_AQ_COMP_E_NOTDONE (0)
#define CAVM_NIX_AQ_COMP_E_SQB_ALLOC_FAIL (6)
#define CAVM_NIX_AQ_COMP_E_SWERR (2)

/**
 * Enumeration nix_aq_ctype_e
 *
 * NIX Context Type Enumeration
 * Enumerates NIX_AQ_INST_S[CTYPE] values.
 */
#define CAVM_NIX_AQ_CTYPE_E_CQ (2)
#define CAVM_NIX_AQ_CTYPE_E_DYNO (5)
#define CAVM_NIX_AQ_CTYPE_E_MCE (3)
#define CAVM_NIX_AQ_CTYPE_E_RQ (0)
#define CAVM_NIX_AQ_CTYPE_E_RSS (4)
#define CAVM_NIX_AQ_CTYPE_E_SQ (1)

/**
 * Enumeration nix_aq_instop_e
 *
 * NIX Admin Queue Opcode Enumeration
 * Enumerates NIX_AQ_INST_S[OP] values.
 */
#define CAVM_NIX_AQ_INSTOP_E_INIT (1)
#define CAVM_NIX_AQ_INSTOP_E_LOCK (4)
#define CAVM_NIX_AQ_INSTOP_E_NOP (0)
#define CAVM_NIX_AQ_INSTOP_E_READ (3)
#define CAVM_NIX_AQ_INSTOP_E_UNLOCK (5)
#define CAVM_NIX_AQ_INSTOP_E_WRITE (2)

/**
 * Enumeration nix_chan_e
 *
 * NIX Channel Number Enumeration
 * Enumerates the receive and transmit channels, and values of
 * NIX_RX_PARSE_S[CHAN], NIX_SQ_CTX_S[DEFAULT_CHAN]. CNXXXX implements a subset of
 * these channels. Specifically, only channels for links enumerated by NIX_LINK_E
 * are implemented.
 *
 * Internal:
 * P2X/X2P channel enumeration for t9x.
 */
#define CAVM_NIX_CHAN_E_CGXX_LMACX_CHX(a,b,c) (0x800 + 0x100 * (a) + 0x10 * (b) + (c))
#define CAVM_NIX_CHAN_E_LBKX_CHX(a,b) (0 + 0x100 * (a) + (b))
#define CAVM_NIX_CHAN_E_RX(a) (0 + 0x100 * (a))
#define CAVM_NIX_CHAN_E_SDP_CHX(a) (0x700 + (a))

/**
 * Enumeration nix_colorresult_e
 *
 * NIX Color Result Enumeration
 * Enumerates the values of NIX_MEM_RESULT_S[COLOR], NIX_AF_TL1()_MD_DEBUG1[COLOR]
 * and NIX_AF_TL1()_MD_DEBUG2[COLOR].
 */
#define CAVM_NIX_COLORRESULT_E_GREEN (0)
#define CAVM_NIX_COLORRESULT_E_RED_DROP (3)
#define CAVM_NIX_COLORRESULT_E_RED_SEND (2)
#define CAVM_NIX_COLORRESULT_E_YELLOW (1)

/**
 * Enumeration nix_cqerrint_e
 *
 * NIX Completion Queue Interrupt Enumeration
 * Enumerates the bit index of NIX_CQ_CTX_S[CQ_ERR_INT,CQ_ERR_INT_ENA].
 */
#define CAVM_NIX_CQERRINT_E_CQE_FAULT (2)
#define CAVM_NIX_CQERRINT_E_DOOR_ERR (0)
#define CAVM_NIX_CQERRINT_E_WR_FULL (1)

/**
 * Enumeration nix_lf_int_vec_e
 *
 * NIX Local Function Interrupt Vector Enumeration
 * Enumerates the NIX MSI-X interrupt vectors per LF.
 */
#define CAVM_NIX_LF_INT_VEC_E_CINTX(a) (0x40 + (a))
#define CAVM_NIX_LF_INT_VEC_E_ERR_INT (0x81)
#define CAVM_NIX_LF_INT_VEC_E_GINT (0x80)
#define CAVM_NIX_LF_INT_VEC_E_POISON (0x82)
#define CAVM_NIX_LF_INT_VEC_E_QINTX(a) (0 + (a))

/**
 * Enumeration nix_link_e
 *
 * NIX Link Number Enumeration
 * Enumerates the receive and transmit links, and LINK index of
 * NIX_AF_RX_LINK()_CFG, NIX_AF_RX_LINK()_WRR_CFG,
 * NIX_AF_TX_LINK()_NORM_CREDIT, NIX_AF_TX_LINK()_EXPR_CREDIT,
 * NIX_AF_TX_LINK()_HW_XOFF and NIX_AF_TX_LINK()_SW_XOFF,
 * NIX_AF_TL3_TL2()_LINK()_CFG.
 */
#define CAVM_NIX_LINK_E_CGXX_LMACX(a,b) (0 + 4 * (a) + (b))
#define CAVM_NIX_LINK_E_LBKX(a) (0xc + (a))
#define CAVM_NIX_LINK_E_MC (0xe)
#define CAVM_NIX_LINK_E_SDP (0xd)

/**
 * Enumeration nix_lsoalg_e
 *
 * NIX Large Send Offload Algorithm Enumeration
 * Enumerates NIX_AF_LSO_FORMAT()_FIELD()[ALG] values. Specifies algorithm for
 * modifying the associated LSO packet field.
 */
#define CAVM_NIX_LSOALG_E_ADD_OFFSET (3)
#define CAVM_NIX_LSOALG_E_ADD_PAYLEN (2)
#define CAVM_NIX_LSOALG_E_ADD_SEGNUM (1)
#define CAVM_NIX_LSOALG_E_NOP (0)
#define CAVM_NIX_LSOALG_E_TCP_FLAGS (4)

/**
 * Enumeration nix_maxsqesz_e
 *
 * NIX Maximum SQE Size Enumeration
 * Enumerates the values of NIX_SQ_CTX_S[MAX_SQE_SIZE].
 */
#define CAVM_NIX_MAXSQESZ_E_W16 (0)
#define CAVM_NIX_MAXSQESZ_E_W8 (1)

/**
 * Enumeration nix_mdtype_e
 *
 * NIX Meta Descriptor Type Enumeration
 * Enumerates values of NIX_AF_MDQ()_MD_DEBUG[MD_TYPE].
 */
#define CAVM_NIX_MDTYPE_E_FLUSH (1)
#define CAVM_NIX_MDTYPE_E_PMD (2)
#define CAVM_NIX_MDTYPE_E_RSVD (0)

/**
 * Enumeration nix_mnqerr_e
 *
 * NIX Meta-Descriptor Enqueue Error Enumeration
 * Enumerates NIX_LF_MNQ_ERR_DBG[ERRCODE] values.
 */
#define CAVM_NIX_MNQERR_E_CQ_QUERY_ERR (6)
#define CAVM_NIX_MNQERR_E_LSO_ERR (5)
#define CAVM_NIX_MNQERR_E_MAXLEN_ERR (8)
#define CAVM_NIX_MNQERR_E_MAX_SQE_SIZE_ERR (7)
#define CAVM_NIX_MNQERR_E_SQB_FAULT (2)
#define CAVM_NIX_MNQERR_E_SQB_POISON (3)
#define CAVM_NIX_MNQERR_E_SQ_CTX_FAULT (0)
#define CAVM_NIX_MNQERR_E_SQ_CTX_POISON (1)
#define CAVM_NIX_MNQERR_E_TOTAL_ERR (4)

/**
 * Enumeration nix_ndc_rx_port_e
 *
 * NIX Receive NDC Port Enumeration
 * Enumerates NIX receive NDC (NDC_IDX_E::NIX()_RX) ports and the PORT index of
 * NDC_AF_PORT()_RT()_RW()_REQ_PC and NDC_AF_PORT()_RT()_RW()_LAT_PC.
 */
#define CAVM_NIX_NDC_RX_PORT_E_AQ (0)
#define CAVM_NIX_NDC_RX_PORT_E_CINT (2)
#define CAVM_NIX_NDC_RX_PORT_E_CQ (1)
#define CAVM_NIX_NDC_RX_PORT_E_MC (3)
#define CAVM_NIX_NDC_RX_PORT_E_PKT (4)
#define CAVM_NIX_NDC_RX_PORT_E_RQ (5)

/**
 * Enumeration nix_ndc_tx_port_e
 *
 * NIX Transmit NDC Port Enumeration
 * Enumerates NIX transmit NDC (NDC_IDX_E::NIX()_TX) ports and the PORT index of
 * NDC_AF_PORT()_RT()_RW()_REQ_PC and NDC_AF_PORT()_RT()_RW()_LAT_PC.
 */
#define CAVM_NIX_NDC_TX_PORT_E_DEQ (3)
#define CAVM_NIX_NDC_TX_PORT_E_DMA (4)
#define CAVM_NIX_NDC_TX_PORT_E_ENQ (1)
#define CAVM_NIX_NDC_TX_PORT_E_LMT (0)
#define CAVM_NIX_NDC_TX_PORT_E_MNQ (2)
#define CAVM_NIX_NDC_TX_PORT_E_XQE (5)

/**
 * Enumeration nix_re_opcode_e
 *
 * NIX Receive Error Opcode Enumeration
 * Enumerates NIX_RX_PARSE_S[ERRCODE] values when NIX_RX_PARSE_S[ERRLEV] =
 * NPC_ERRLEV_E::RE.
 */
#define CAVM_NIX_RE_OPCODE_E_OL2_LENMISM (0x12)
#define CAVM_NIX_RE_OPCODE_E_OVERSIZE (0x11)
#define CAVM_NIX_RE_OPCODE_E_RE_DMAPKT (0xf)
#define CAVM_NIX_RE_OPCODE_E_RE_FCS (7)
#define CAVM_NIX_RE_OPCODE_E_RE_FCS_RCV (8)
#define CAVM_NIX_RE_OPCODE_E_RE_JABBER (2)
#define CAVM_NIX_RE_OPCODE_E_RE_NONE (0)
#define CAVM_NIX_RE_OPCODE_E_RE_PARTIAL (1)
#define CAVM_NIX_RE_OPCODE_E_RE_RX_CTL (0xb)
#define CAVM_NIX_RE_OPCODE_E_RE_SKIP (0xc)
#define CAVM_NIX_RE_OPCODE_E_RE_TERMINATE (9)
#define CAVM_NIX_RE_OPCODE_E_UNDERSIZE (0x10)

/**
 * Enumeration nix_redalg_e
 *
 * NIX Red Algorithm Enumeration
 * Enumerates the different algorithms of NIX_SEND_EXT_S[SHP_RA].
 */
#define CAVM_NIX_REDALG_E_DISCARD (3)
#define CAVM_NIX_REDALG_E_SEND (1)
#define CAVM_NIX_REDALG_E_STALL (2)
#define CAVM_NIX_REDALG_E_STD (0)

/**
 * Enumeration nix_rqint_e
 *
 * NIX Receive Queue Interrupt Enumeration
 * Enumerates the bit index of NIX_RQ_CTX_S[RQ_INT,RQ_INT_ENA].
 */
#define CAVM_NIX_RQINT_E_DROP (0)
#define CAVM_NIX_RQINT_E_RX(a) (0 + (a))
#define CAVM_NIX_RQINT_E_RED (1)

/**
 * Enumeration nix_rx_actionop_e
 *
 * NIX Receive Action Opcode Enumeration
 * Enumerates the values of NIX_RX_ACTION_S[OP].
 */
#define CAVM_NIX_RX_ACTIONOP_E_DROP (0)
#define CAVM_NIX_RX_ACTIONOP_E_MCAST (3)
#define CAVM_NIX_RX_ACTIONOP_E_MIRROR (6)
#define CAVM_NIX_RX_ACTIONOP_E_PF_FUNC_DROP (5)
#define CAVM_NIX_RX_ACTIONOP_E_RSS (4)
#define CAVM_NIX_RX_ACTIONOP_E_UCAST (1)
#define CAVM_NIX_RX_ACTIONOP_E_UCAST_IPSEC (2)

/**
 * Enumeration nix_rx_mcop_e
 *
 * NIX Receive Multicast/Mirror Opcode Enumeration
 * Enumerates the values of NIX_RX_MCE_S[OP].
 */
#define CAVM_NIX_RX_MCOP_E_RQ (0)
#define CAVM_NIX_RX_MCOP_E_RSS (1)

/**
 * Enumeration nix_rx_perrcode_e
 *
 * NIX Receive Protocol Error Code Enumeration
 * Enumerates NIX_RX_PARSE_S[ERRCODE] values when NIX_RX_PARSE_S[ERRLEV] =
 * NPC_ERRLEV_E::NIX.
 */
#define CAVM_NIX_RX_PERRCODE_E_BUFS_OFLOW (0xa)
#define CAVM_NIX_RX_PERRCODE_E_DATA_FAULT (8)
#define CAVM_NIX_RX_PERRCODE_E_IL3_LEN (0x20)
#define CAVM_NIX_RX_PERRCODE_E_IL4_CHK (0x22)
#define CAVM_NIX_RX_PERRCODE_E_IL4_LEN (0x21)
#define CAVM_NIX_RX_PERRCODE_E_IL4_PORT (0x23)
#define CAVM_NIX_RX_PERRCODE_E_MCAST_FAULT (4)
#define CAVM_NIX_RX_PERRCODE_E_MCAST_POISON (6)
#define CAVM_NIX_RX_PERRCODE_E_MEMOUT (9)
#define CAVM_NIX_RX_PERRCODE_E_MIRROR_FAULT (5)
#define CAVM_NIX_RX_PERRCODE_E_MIRROR_POISON (7)
#define CAVM_NIX_RX_PERRCODE_E_NPC_RESULT_ERR (2)
#define CAVM_NIX_RX_PERRCODE_E_OL3_LEN (0x10)
#define CAVM_NIX_RX_PERRCODE_E_OL4_CHK (0x12)
#define CAVM_NIX_RX_PERRCODE_E_OL4_LEN (0x11)
#define CAVM_NIX_RX_PERRCODE_E_OL4_PORT (0x13)

/**
 * Enumeration nix_send_status_e
 *
 * NIX Send Completion Status Enumeration
 * Enumerates values of NIX_SEND_COMP_S[STATUS] and NIX_LF_SEND_ERR_DBG[ERRCODE].
 * Internal:
 * TODO.
 */
#define CAVM_NIX_SEND_STATUS_E_DATA_FAULT (0x16)
#define CAVM_NIX_SEND_STATUS_E_DATA_POISON (0x17)
#define CAVM_NIX_SEND_STATUS_E_GOOD (0)
#define CAVM_NIX_SEND_STATUS_E_INVALID_SUBDC (0x14)
#define CAVM_NIX_SEND_STATUS_E_JUMP_FAULT (7)
#define CAVM_NIX_SEND_STATUS_E_JUMP_POISON (8)
#define CAVM_NIX_SEND_STATUS_E_LOCK_VIOL (0x21)
#define CAVM_NIX_SEND_STATUS_E_NPC_DROP_ACTION (0x20)
#define CAVM_NIX_SEND_STATUS_E_NPC_MCAST_ABORT (0x24)
#define CAVM_NIX_SEND_STATUS_E_NPC_MCAST_CHAN_ERR (0x23)
#define CAVM_NIX_SEND_STATUS_E_NPC_UCAST_CHAN_ERR (0x22)
#define CAVM_NIX_SEND_STATUS_E_NPC_VTAG_PTR_ERR (0x25)
#define CAVM_NIX_SEND_STATUS_E_NPC_VTAG_SIZE_ERR (0x26)
#define CAVM_NIX_SEND_STATUS_E_SEND_CRC_ERR (0x10)
#define CAVM_NIX_SEND_STATUS_E_SEND_EXT_ERR (6)
#define CAVM_NIX_SEND_STATUS_E_SEND_HDR_ERR (5)
#define CAVM_NIX_SEND_STATUS_E_SEND_IMM_ERR (0x11)
#define CAVM_NIX_SEND_STATUS_E_SEND_MEM_ERR (0x13)
#define CAVM_NIX_SEND_STATUS_E_SEND_MEM_FAULT (0x27)
#define CAVM_NIX_SEND_STATUS_E_SEND_SG_ERR (0x12)
#define CAVM_NIX_SEND_STATUS_E_SQB_FAULT (3)
#define CAVM_NIX_SEND_STATUS_E_SQB_POISON (4)
#define CAVM_NIX_SEND_STATUS_E_SQ_CTX_FAULT (1)
#define CAVM_NIX_SEND_STATUS_E_SQ_CTX_POISON (2)
#define CAVM_NIX_SEND_STATUS_E_SUBDC_ORDER_ERR (0x15)

/**
 * Enumeration nix_sendcrcalg_e
 *
 * NIX Send CRC Algorithm Enumeration
 * Enumerates the CRC algorithm used, see NIX_SEND_CRC_S[ALG].
 */
#define CAVM_NIX_SENDCRCALG_E_CRC32 (0)
#define CAVM_NIX_SENDCRCALG_E_CRC32C (1)
#define CAVM_NIX_SENDCRCALG_E_ONES16 (2)

/**
 * Enumeration nix_sendl3type_e
 *
 * NIX Send Layer 3 Header Type Enumeration
 * Enumerates values of NIX_SEND_HDR_S[OL3TYPE], NIX_SEND_HDR_S[IL3TYPE].
 * Internal:
 * Encoding matches DPDK TX IP types:
 * \<pre\>
 * PKT_TX_IP_CKSUM      (1ULL \<\< 54)
 * PKT_TX_IPV4          (1ULL \<\< 55)
 * PKT_TX_IPV6          (1ULL \<\< 56)
 *
 * PKT_TX_OUTER_IP_CKSUM(1ULL \<\< 58)
 * PKT_TX_OUTER_IPV4    (1ULL \<\< 59)
 * PKT_TX_OUTER_IPV6    (1ULL \<\< 60)
 * \</pre\>
 */
#define CAVM_NIX_SENDL3TYPE_E_IP4 (2)
#define CAVM_NIX_SENDL3TYPE_E_IP4_CKSUM (3)
#define CAVM_NIX_SENDL3TYPE_E_IP6 (4)
#define CAVM_NIX_SENDL3TYPE_E_NONE (0)

/**
 * Enumeration nix_sendl4type_e
 *
 * NIX Send Layer 4 Header Type Enumeration
 * Enumerates values of NIX_SEND_HDR_S[OL4TYPE], NIX_SEND_HDR_S[IL4TYPE].
 * Internal:
 * Encoding matches DPDK TX L4 types.
 * \<pre\>
 * PKT_TX_L4_NO_CKSUM   (0ULL \<\< 52)  // Disable L4 cksum of TX pkt.
 * PKT_TX_TCP_CKSUM     (1ULL \<\< 52)  // TCP cksum of TX pkt. computed by nic.
 * PKT_TX_SCTP_CKSUM    (2ULL \<\< 52)  // SCTP cksum of TX pkt. computed by nic.
 * PKT_TX_UDP_CKSUM     (3ULL \<\< 52)  // UDP cksum of TX pkt. computed by nic.
 * \</pre\>
 */
#define CAVM_NIX_SENDL4TYPE_E_NONE (0)
#define CAVM_NIX_SENDL4TYPE_E_SCTP_CKSUM (2)
#define CAVM_NIX_SENDL4TYPE_E_TCP_CKSUM (1)
#define CAVM_NIX_SENDL4TYPE_E_UDP_CKSUM (3)

/**
 * Enumeration nix_sendldtype_e
 *
 * NIX Send Load Type Enumeration
 * Enumerates the load transaction types for reading segment bytes specified by
 * NIX_SEND_SG_S[LD_TYPE] and NIX_SEND_JUMP_S[LD_TYPE].
 *
 * Internal:
 * The hardware implementation treats undefined encodings as LDD load type.
 */
#define CAVM_NIX_SENDLDTYPE_E_LDD (0)
#define CAVM_NIX_SENDLDTYPE_E_LDT (1)
#define CAVM_NIX_SENDLDTYPE_E_LDWB (2)

/**
 * Enumeration nix_sendmemalg_e
 *
 * NIX Memory Modify Algorithm Enumeration
 * Enumerates the different algorithms for modifying memory; see
 * NIX_SEND_MEM_S[ALG]. mbufs_freed is the number of gather buffers freed to NPA
 * for the send descriptor. See NIX_SEND_HDR_S[DF] and NIX_SEND_SG_S[I].
 */
#define CAVM_NIX_SENDMEMALG_E_ADD (8)
#define CAVM_NIX_SENDMEMALG_E_ADDLEN (0xa)
#define CAVM_NIX_SENDMEMALG_E_ADDMBUF (0xc)
#define CAVM_NIX_SENDMEMALG_E_SET (0)
#define CAVM_NIX_SENDMEMALG_E_SETRSLT (2)
#define CAVM_NIX_SENDMEMALG_E_SETTSTMP (1)
#define CAVM_NIX_SENDMEMALG_E_SUB (9)
#define CAVM_NIX_SENDMEMALG_E_SUBLEN (0xb)
#define CAVM_NIX_SENDMEMALG_E_SUBMBUF (0xd)

/**
 * Enumeration nix_sendmemdsz_e
 *
 * NIX Memory Data Size Enumeration
 * Enumerates the datum size for modifying memory; see NIX_SEND_MEM_S[DSZ].
 */
#define CAVM_NIX_SENDMEMDSZ_E_B16 (2)
#define CAVM_NIX_SENDMEMDSZ_E_B32 (1)
#define CAVM_NIX_SENDMEMDSZ_E_B64 (0)
#define CAVM_NIX_SENDMEMDSZ_E_B8 (3)

/**
 * Enumeration nix_sqint_e
 *
 * NIX Send Queue Interrupt Enumeration
 * Enumerates the bit index of NIX_SQ_CTX_S[SQ_INT,SQ_INT_ENA].
 */
#define CAVM_NIX_SQINT_E_LMT_ERR (0)
#define CAVM_NIX_SQINT_E_MNQ_ERR (1)
#define CAVM_NIX_SQINT_E_SEND_ERR (2)
#define CAVM_NIX_SQINT_E_SQB_ALLOC_FAIL (3)

/**
 * Enumeration nix_sqoperr_e
 *
 * NIX SQ Operation Error Enumeration
 * Enumerates NIX_LF_SQ_OP_ERR_DBG[ERRCODE] values.
 */
#define CAVM_NIX_SQOPERR_E_MAX_SQE_SIZE_ERR (4)
#define CAVM_NIX_SQOPERR_E_SQB_FAULT (7)
#define CAVM_NIX_SQOPERR_E_SQB_NULL (6)
#define CAVM_NIX_SQOPERR_E_SQE_OFLOW (5)
#define CAVM_NIX_SQOPERR_E_SQ_CTX_FAULT (1)
#define CAVM_NIX_SQOPERR_E_SQ_CTX_POISON (2)
#define CAVM_NIX_SQOPERR_E_SQ_DISABLED (3)
#define CAVM_NIX_SQOPERR_E_SQ_OOR (0)

/**
 * Enumeration nix_stat_lf_rx_e
 *
 * NIX Local Function Receive Statistics Enumeration
 * Enumerates the last index of NIX_AF_LF()_RX_STAT() and NIX_LF_RX_STAT().
 */
#define CAVM_NIX_STAT_LF_RX_E_RX_BCAST (2)
#define CAVM_NIX_STAT_LF_RX_E_RX_DROP (4)
#define CAVM_NIX_STAT_LF_RX_E_RX_DROP_OCTS (5)
#define CAVM_NIX_STAT_LF_RX_E_RX_DRP_BCAST (8)
#define CAVM_NIX_STAT_LF_RX_E_RX_DRP_L3BCAST (0xa)
#define CAVM_NIX_STAT_LF_RX_E_RX_DRP_L3MCAST (0xb)
#define CAVM_NIX_STAT_LF_RX_E_RX_DRP_MCAST (9)
#define CAVM_NIX_STAT_LF_RX_E_RX_ERR (7)
#define CAVM_NIX_STAT_LF_RX_E_RX_FCS (6)
#define CAVM_NIX_STAT_LF_RX_E_RX_MCAST (3)
#define CAVM_NIX_STAT_LF_RX_E_RX_OCTS (0)
#define CAVM_NIX_STAT_LF_RX_E_RX_UCAST (1)

/**
 * Enumeration nix_stat_lf_tx_e
 *
 * NIX Local Function Transmit Statistics Enumeration
 * Enumerates the index of NIX_AF_LF()_TX_STAT() and NIX_LF_TX_STAT().
 * These statistics do not account for packet replication due to
 * NIX_TX_ACTION_S[OP] = NIX_TX_ACTIONOP_E::MCAST.
 */
#define CAVM_NIX_STAT_LF_TX_E_TX_BCAST (1)
#define CAVM_NIX_STAT_LF_TX_E_TX_DROP (3)
#define CAVM_NIX_STAT_LF_TX_E_TX_MCAST (2)
#define CAVM_NIX_STAT_LF_TX_E_TX_OCTS (4)
#define CAVM_NIX_STAT_LF_TX_E_TX_UCAST (0)

/**
 * Enumeration nix_stype_e
 *
 * NIX SQB Caching Type Enumeration
 * Enumerates the values of NIX_SQ_CTX_S[SQE_STYPE].
 */
#define CAVM_NIX_STYPE_E_STF (0)
#define CAVM_NIX_STYPE_E_STP (2)
#define CAVM_NIX_STYPE_E_STT (1)

/**
 * Enumeration nix_subdc_e
 *
 * NIX Subdescriptor Operation Enumeration
 * Enumerates send and receive subdescriptor codes. The codes differentiate
 * subdescriptors within a NIX send or receive descriptor, excluding NIX_SEND_HDR_S for
 * send and NIX_CQE_HDR_S/NIX_WQE_HDR_S for receive, which are determined by their
 * position as the first subdescriptor, and NIX_RX_PARSE_S, which is determined by its
 * position as the second subdescriptor.
 */
#define CAVM_NIX_SUBDC_E_CRC (2)
#define CAVM_NIX_SUBDC_E_EXT (1)
#define CAVM_NIX_SUBDC_E_IMM (3)
#define CAVM_NIX_SUBDC_E_JUMP (6)
#define CAVM_NIX_SUBDC_E_MEM (5)
#define CAVM_NIX_SUBDC_E_NOP (0)
#define CAVM_NIX_SUBDC_E_SG (4)
#define CAVM_NIX_SUBDC_E_SOD (0xf)
#define CAVM_NIX_SUBDC_E_WORK (7)

/**
 * Enumeration nix_tx_actionop_e
 *
 * NIX Transmit Action Opcode Enumeration
 * Enumerates the values of NIX_TX_ACTION_S[OP].
 */
#define CAVM_NIX_TX_ACTIONOP_E_DROP (0)
#define CAVM_NIX_TX_ACTIONOP_E_DROP_VIOL (5)
#define CAVM_NIX_TX_ACTIONOP_E_MCAST (3)
#define CAVM_NIX_TX_ACTIONOP_E_UCAST_CHAN (2)
#define CAVM_NIX_TX_ACTIONOP_E_UCAST_DEFAULT (1)

/**
 * Enumeration nix_tx_vtagop_e
 *
 * NIX Transmit Vtag Opcode Enumeration
 * Enumerates the values of NIX_TX_VTAG_ACTION_S[VTAG0_OP,VTAG1_OP].
 */
#define CAVM_NIX_TX_VTAGOP_E_INSERT (1)
#define CAVM_NIX_TX_VTAGOP_E_NOP (0)
#define CAVM_NIX_TX_VTAGOP_E_REPLACE (2)

/**
 * Enumeration nix_txlayer_e
 *
 * NIX Transmit Layer Enumeration
 * Enumerates the values of NIX_AF_LSO_FORMAT()_FIELD()[LAYER].
 */
#define CAVM_NIX_TXLAYER_E_IL3 (2)
#define CAVM_NIX_TXLAYER_E_IL4 (3)
#define CAVM_NIX_TXLAYER_E_OL3 (0)
#define CAVM_NIX_TXLAYER_E_OL4 (1)

/**
 * Enumeration nix_vtagsize_e
 *
 * NIX Vtag Size Enumeration
 * Enumerates the values of NIX_AF_TX_VTAG_DEF()_CTL[SIZE] and NIX_AF_LF()_RX_VTAG_TYPE()[SIZE].
 */
#define CAVM_NIX_VTAGSIZE_E_T4 (0)
#define CAVM_NIX_VTAGSIZE_E_T8 (1)

/**
 * Enumeration nix_xqe_type_e
 *
 * NIX WQE/CQE Type Enumeration
 * Enumerates the values of NIX_WQE_HDR_S[WQE_TYPE], NIX_CQE_HDR_S[CQE_TYPE].
 */
#define CAVM_NIX_XQE_TYPE_E_INVALID (0)
#define CAVM_NIX_XQE_TYPE_E_RX (1)
#define CAVM_NIX_XQE_TYPE_E_RX_IPSECD (4)
#define CAVM_NIX_XQE_TYPE_E_RX_IPSECH (3)
#define CAVM_NIX_XQE_TYPE_E_RX_IPSECS (2)
#define CAVM_NIX_XQE_TYPE_E_SEND (8)

/**
 * Enumeration nix_xqesz_e
 *
 * NIX WQE/CQE Size Enumeration
 * Enumerates the values of NIX_AF_LF()_CFG[XQE_SIZE].
 */
#define CAVM_NIX_XQESZ_E_W16 (1)
#define CAVM_NIX_XQESZ_E_W64 (0)

/* Hardware PF_BAR0 register offsets */
#define CAVM_NIX_AF_AQ_CFG			(0x0000)
#define CAVM_NIX_AF_STATUS			(0x0010)
#define CAVM_NIX_AF_NDC_CFG			(0x0018)
#define CAVM_NIX_AF_CONST			(0x0020)
#define CAVM_NIX_AF_CONST1			(0x0028)
#define CAVM_NIX_AF_CONST2			(0x0030)
#define CAVM_NIX_AF_SQ_CONST			(0x0040)
#define CAVM_NIX_AF_CQ_CONST			(0x0048)
#define CAVM_NIX_AF_RQ_CONST			(0x0050)
#define CAVM_NIX_AF_PSE_CONST			(0x0060)
#define CAVM_NIX_AF_TL1_CONST			(0x0070)
#define CAVM_NIX_AF_TL2_CONST			(0x0078)
#define CAVM_NIX_AF_TL3_CONST			(0x0080)
#define CAVM_NIX_AF_TL4_CONST			(0x0088)
#define CAVM_NIX_AF_MDQ_CONST			(0x0090)
#define CAVM_NIX_AF_MC_MIRROR_CONST		(0x0098)
#define CAVM_NIX_AF_ACTIVE_CYCLES_PC		(0x00a0)
#define CAVM_NIX_AF_LSO_CFG			(0x00a8)
#define CAVM_NIX_AF_BLK_RST			(0x00b0)
#define CAVM_NIX_AF_TX_TSTMP_CFG		(0x00c0)
#define CAVM_NIX_AF_RX_CFG			(0x00d0)
#define CAVM_NIX_AF_AVG_DELAY			(0x00e0)
#define CAVM_NIX_AF_CINT_DELAY			(0x00f0)
#define CAVM_NIX_AF_RX_MCAST_BASE		(0x0100)
#define CAVM_NIX_AF_RX_MCAST_CFG		(0x0110)
#define CAVM_NIX_AF_RX_MCAST_BUF_BASE		(0x0120)
#define CAVM_NIX_AF_RX_MCAST_BUF_CFG		(0x0130)
#define CAVM_NIX_AF_RX_MIRROR_BUF_BASE		(0x0140)
#define CAVM_NIX_AF_RX_MIRROR_BUF_CFG		(0x0148)
#define CAVM_NIX_AF_LF_RST			(0x0150)
#define CAVM_NIX_AF_GEN_INT			(0x0160)
#define CAVM_NIX_AF_GEN_INT_W1S			(0x0168)
#define CAVM_NIX_AF_GEN_INT_ENA_W1S		(0x0170)
#define CAVM_NIX_AF_GEN_INT_ENA_W1C		(0x0178)
#define CAVM_NIX_AF_ERR_INT			(0x0180)
#define CAVM_NIX_AF_ERR_INT_W1S			(0x0188)
#define CAVM_NIX_AF_ERR_INT_ENA_W1S		(0x0190)
#define CAVM_NIX_AF_ERR_INT_ENA_W1C		(0x0198)
#define CAVM_NIX_AF_RAS				(0x01a0)
#define CAVM_NIX_AF_RAS_W1S			(0x01a8)
#define CAVM_NIX_AF_RAS_ENA_W1S			(0x01b0)
#define CAVM_NIX_AF_RAS_ENA_W1C			(0x01b8)
#define CAVM_NIX_AF_RVU_INT			(0x01c0)
#define CAVM_NIX_AF_RVU_INT_W1S			(0x01c8)
#define CAVM_NIX_AF_RVU_INT_ENA_W1S		(0x01d0)
#define CAVM_NIX_AF_RVU_INT_ENA_W1C		(0x01d8)
#define CAVM_NIX_AF_TCP_TIMER			(0x01e0)
#define CAVM_NIX_AF_RX_DEF_OL2			(0x0200)
#define CAVM_NIX_AF_RX_DEF_OIP4			(0x0210)
#define CAVM_NIX_AF_RX_DEF_IIP4			(0x0220)
#define CAVM_NIX_AF_RX_DEF_OIP6			(0x0230)
#define CAVM_NIX_AF_RX_DEF_IIP6			(0x0240)
#define CAVM_NIX_AF_RX_DEF_OTCP			(0x0250)
#define CAVM_NIX_AF_RX_DEF_ITCP			(0x0260)
#define CAVM_NIX_AF_RX_DEF_OUDP			(0x0270)
#define CAVM_NIX_AF_RX_DEF_IUDP			(0x0280)
#define CAVM_NIX_AF_RX_DEF_OSCTP		(0x0290)
#define CAVM_NIX_AF_RX_DEF_ISCTP		(0x02a0)
#define CAVM_NIX_AF_RX_DEF_IPSECX(x)		(0x02b0 + (x) * 8)

#define CAVM_NIX_AF_RX_IPSEC_GEN_CFG		(0x0300)
#define CAVM_NIX_AF_RX_CPTX_INST_QSEL(x)	(0x0320 + (x) * 8)

#define CAVM_NIX_AF_AQ_CFG			(0x0400)
#define CAVM_NIX_AF_AQ_BASE			(0x0410)
#define CAVM_NIX_AF_AQ_STATUS			(0x0420)
#define CAVM_NIX_AF_AQ_DOOR			(0x0430)
#define CAVM_NIX_AF_AQ_DONE_WAIT		(0x0440)
#define CAVM_NIX_AF_AQ_DONE			(0x0450)
#define CAVM_NIX_AF_AQ_DONE_ACK			(0x0460)
#define CAVM_NIX_AF_AQ_DONE_INT			(0x0480)
#define CAVM_NIX_AF_AQ_DONE_INT_W1S		(0x0488)
#define CAVM_NIX_AF_AQ_DONE_ENA_W1S		(0x0490)
#define CAVM_NIX_AF_AQ_DONE_ENA_W1C		(0x0498)

#define CAVM_NIX_AF_RX_LINKX_SLX_SPKT_CNT(a, b)	\
			(0x0500 + ((a) & 0xf) * 0x10000 + ((0xb) & 0x1) * 8)
#define CAVM_NIX_AF_RX_LINKX_CFG(x)		\
			(0x0540 + ((x) & 0xf) * 0x10000)
#define CAVM_NIX_AF_RX_SW_SYNC			(0x0550)
#define CAVM_NIX_AF_RX_LINKX_WRR_CFG(x)		\
			(0x0560 + ((x) & 0xf) * 0x10000)

#define CAVM_NIX_AF_SEB_ECO			(0x0600)
#define CAVM_NIX_AF_SEB_TEST_BP			(0x0610)
#define CAVM_NIX_AF_NORM_TX_FIFO_STATUS		(0x0620)
#define CAVM_NIX_AF_EXPR_TX_FIFO_STATUS		(0x0630)
#define CAVM_NIX_AF_SDP_TX_FIFO_STATUS		(0x0640)
#define CAVM_NIX_AF_TX_NPC_CAPTURE_CONFIG	(0x0660)
#define CAVM_NIX_AF_TX_NPC_CAPTURE_INFO		(0x0670)
#define CAVM_NIX_AF_TX_NPC_CAPTURE_RESPX(x)	\
			(0x0680 + ((x) & 0x7) * 8)

#define CAVM_NIX_AF_SMQX_CFG(x)			\
			(0x0700 + ((x) & 0x1ff) * 0x10000)

#define CAVM_NIX_AF_PSE_CHANNEL_LEVEL		(0x0800)
#define CAVM_NIX_AF_PSE_SHAPER_CFG		(0x0810)
#define CAVM_NIX_AF_TX_NORM_CREDIT		(0x0820)
#define CAVM_NIX_AF_TX_EXPR_CREDIT		(0x0830)

#define CAVM_NIX_AF_MARK_FORMATX_CTL(x)		\
			(0x0900 + ((x) & 0x7f) * 0x40000)

#define CAVM_NIX_AF_TX_LINKX_NORM_CREDIT(x)	\
			(0x0a00 + ((x) & 0xf) * 0x10000)
#define CAVM_NIX_AF_TX_LINKX_EXPR_CREDIT(x)	\
			(0x0a10 + ((x) & 0xf) * 0x10000)
#define CAVM_NIX_AF_TX_LINKX_SW_XOFF(x)		\
			(0x0a20 + ((x) & 0xf) * 0x10000)

#define CAVM_NIX_AF_TX_LINKX_HW_XOFF(x)		\
			(0x0a30 + ((x) & 0xf) * 0x10000)
#define CAVM_NIX_AF_SDP_LINK_CREDIT		(0x0a40)
#define CAVM_NIX_AF_SDP_SW_XOFFX(x)		\
			(0x0a60 + ((x) & 0x3) * 8)
#define CAVM_NIX_AF_SDP_HW_XOFFX(x)		\
			(0x0ac0 + ((x) & 0x3) * 8)

#define CAVM_NIX_AF_TL4X_BP_STATUS(x)		\
			(0x0b00 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_SDP_LINK_CFG(x)	\
			(0x0b10 + ((x) & 0x1ff) * 0x10000)

#define CAVM_NIX_AF_TL1X_SCHEDULE(x)		\
			(0x0c00 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_SHAPE(x)		\
			(0x0c10 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_CIR(x)			\
			(0x0c20 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_SHAPE_STATE(x)		\
			(0x0c50 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_SW_XOFF(x)		\
			(0x0c70 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_TOPOLOGY		(0x0c80)
#define CAVM_NIX_AF_TL1X_GREEN(x)		\
			(0x0c90 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_YELLOW(x)		\
			(0x0ca0 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_RED(x)			\
			(0x0cb0 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_MD_DEBUG0(x)		\
			(0x0cc0 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_MD_DEBUG1(x)		\
			(0x0cc8 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_MD_DEBUG2(x)		\
			(0x0cd0 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_MD_DEBUG3(x)		\
			(0x0cd8 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1_DEBUG_GREEN		(0x0d00)
#define CAVM_NIX_AF_TL1_DEBUG_NODE		(0x0d10)
#define CAVM_NIX_AF_TL1X_DROPPED_PACKETS(x)	\
			(0x0d20 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_DROPPED_BYTES(x)	\
			(0x0d30 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_RED_PACKETS(x)		\
			(0x0d40 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_RED_BYTES(x)		\
			(0x0d50 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_YELLOW_PACKETS(x)	\
			(0x0d60 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_YELLOW_BYTES(x)	\
			(0x0d70 + ((x) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_GREEN_PACKETS(x)	\
			(0x0d80 + ((X) & 0x1f) * 0x10000)
#define CAVM_NIX_AF_TL1X_GREEN_BYTES(x)		\
			(0x0d90 + ((x) & 0x1f) * 0x10000)

#define CAVM_NIX_AF_TL2X_SCHEDULE(x)		\
			(0x0e00 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_SHAPE(x)		\
			(0x0e10 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_CIR(x)			\
			(0x0e20 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_PIR(x)		\
			(0x0e30 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_SCHED_STATE(x)		\
			(0x0e40 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_SHAPE_STATE(x)		\
			(0x0e50 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_POINTERS(x)		\
			(0x0e60 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_SW_XOFF(x)		\
			(0x0e70 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_TOPOLOGY(x)		\
			(0x0e80 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_PARENT(x)		\
			(0x0e88 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_GREEN(x)		\
			(0x0e90 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_YELLOW(x)		\
			(0x0ea0 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL2X_RED(x)		\
			(0x0eb0 + ((x) & 0xff) * 0x10000)

#define CAVM_NIX_AF_TL3X_SCHEDULE(x)		\
			(0x1000 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_SHAPE(x)		\
			(0x1010 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_CIR(x)			\
			(0x1020 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_PIR(x)		\
			(0x1030 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_SCHED_STATE(x)		\
			(0x1040 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_SHAPE_STATE(x)		\
			(0x1050 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_POINTERS(x)		\
			(0x1060 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_SW_XOFF(x)		\
			(0x1070 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_TOPOLOGY(x)		\
			(0x1080 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_PARENT(x)		\
			(0x1088 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_GREEN(x)		\
			(0x1090 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_YELLOW(x)		\
			(0x10a0 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3X_RED(x)			\
			(0x10b0 + ((x) & 0xff) * 0x10000)

#define CAVM_NIX_AF_TL4X_SCHEDULE(x)		\
			(0x1200 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_SHAPE(x)		\
			(0x1210 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_CIR(x)			\
			(0x1220 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_PIR(x)			\
			(0x1230 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_SCHED_STATE(x)		\
			(0x1240 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_SHAPE_STATE(x)		\
			(0x1250 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_POINTERS(x)		\
			(0x1260 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_SW_XOFF(x)		\
			(0x1270 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_TOPOLOGY(x)		\
			(0x1280 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_PARENT(x)		\
			(0x1288 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_GREEN(x)		\
			(0x1290 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_YELLOW(x)		\
			(0x12a0 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_TL4X_RED(x)			\
			(0x12b0 + ((x) & 0x1ff) * 0x10000)

#define CAVM_NIX_AF_MDQX_SCHEDULE(x)		\
			(0x1400 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_SHAPE(x)		\
			(0x1410 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_CIR(x)			\
			(0x1420 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_PIR(x)			\
			(0x1430 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_SCHED_STATE(x)		\
			(0x1440 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_SHAPE_STATE(x)		\
			(0x1450 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_POINTERS(x)		\
			(0x1460 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_SW_XOFF(x)		\
			(0x1470 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_PARENT(x)		\
			(0x1280 + ((x) & 0x1ff) * 0x10000)
#define CAVM_NIX_AF_MDQX_DEBUG(x)		\
			(0x14c0 + ((x) & 0x1ff) * 0x10000)

#define CAVM_NIX_AF_TL3_TL2X_CFG(x)		\
			(0x1600 + ((x) & 0xff) * 0x10000)
#define CAVM_NIX_AF_TL3_TL2X_BP_STATUS(x)	\
			(0x1610 + ((x) & 0xff) * 0x10000)

#define CAVM_NIX_AF_TL3_TL2X_LINKX_CFG(a, b)	\
			(0x1700 + ((a) & 0xff) * 0x1000 + ((b) & 0xf) * 8)

#define CAVM_NIX_AF_RX_FLOW_KEY_ALGX_FIELDX(a, b)	\
			(0x1800 + ((a) & 0x1f) * 0x40000 + ((b) & 0x7) * 8)

#define CAVM_NIX_AF_TX_MCASTX(x)		\
			(0x1900 + ((x) & 0x7ff) * 0x8000)

#define CAVM_NIX_AF_TX_VTAG_DEFX_CTL(x)		\
			(0x1a00 + ((x) & 0x3ff) * 0x10000)
#define CAVM_NIX_AF_TX_VTAG_DEFX_DATA(x)	\
			(0x1a10 + ((x) & 0x3ff) * 0x10000)
#define CAVM_NIX_AF_RX_BPIDX_STATUS(x)		\
			(0x1a20 + ((x) & 0x1ff) * 0x20000)
#define CAVM_NIX_AF_RX_CHANX_CFG(x)		\
			(0x1a30 + ((x) & 0xfff) * 0x8000)
#define CAVM_NIX_AF_CINT_TIMERX(x)		\
			(0x1a40 + ((x) & 0xff) * 0x40000)

#define CAVM_NIX_AF_LSO_FORMATX_FIELDX(a, b)	\
			(0x1b00 + ((a) & 0x1f) * 0x1000 + ((b) & 0x7) * 8)

#define CAVM_NIX_AF_LFX_CFG(x)			\
			(0x4000 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_SQS_CFG(x)		\
			(0x4020 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_SQS_BASE(x)		\
			(0x4030 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RQS_CFG(x)		\
			(0x4040 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RQS_BASE(x)		\
			(0x4050 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_CQS_CFG(x)		\
			(0x4060 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_CQS_BASE(x)		\
			(0x4070 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_TX_CFG(x)		\
			(0x4080 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_TX_PARSE_CFG(x)		\
			(0x4090 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RX_CFG(x)		\
			(0x40a0 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RSS_CFG(x)		\
			(0x40c0 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RSS_BASE(x)		\
			(0x40d0 + ((x) & 0x7f) * 0x20000)

#define CAVM_NIX_AF_LFX_QINTS_CFG(x)		\
			(0x4100 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_QINTS_BASE(x)		\
			(0x4110 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_CINTS_CFG(x)		\
			(0x4120 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_CINTS_BASE(x)		\
			(0x4130 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RX_IPSEC_CFG0(x)		\
			(0x4140 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RX_IPSEC_CFG1(x)		\
			(0x4148 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RX_IPSEC_DYNO_CFG(x)	\
			(0x4150 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RX_IPSEC_DYNO_BASE(x)	\
			(0x4158 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_RX_IPSEC_SA_BASE(x)	\
			(0x4170 + ((x) & 0x7f) * 0x20000)
#define CAVM_NIX_AF_LFX_TX_STATUS(x)		\
			(0x4180 + ((x) & 0x7f) * 0x20000)

#define CAVM_NIX_AF_LFX_RX_VTAG_TYPEX(a, b)	\
			(0x4200 + ((a) & 0x7f) * 0x20000 + ((b) & 0x7) * 8)

#define CAVM_NIX_AF_LFX_LOCKX(a, b)		\
			(0x4300 + ((a) & 0x7f) * 0x20000 + ((b) & 0x7) * 8)

#define CAVM_NIX_AF_LFX_TX_STATX(a, b)		\
			(0x4400 + ((a) & 0x7f) * 0x20000 + ((b) & 0x7) * 8)

#define CAVM_NIX_AF_LFX_RX_STATX(a, b)		\
			(0x4500 + ((a) & 0x7f) * 0x20000 + ((b) & 0xf) * 8)

#define CAVM_NIX_AF_LFX_RSS_GRPX(a, b)		\
			(0x4600 + ((a) & 0x7f) * 0x20000 + ((b) & 0x7) * 8)

#define CAVM_NIX_AF_RX_NPC_MC_RCV		(0x4700)
#define CAVM_NIX_AF_RX_NPC_MC_DROP		(0x4710)
#define CAVM_NIX_AF_RX_NPC_MIRROR_RCV		(0x4720)
#define CAVM_NIX_AF_RX_NPC_MIRROR_DROP		(0x4730)

/* PF (BAR 2) registers and VF bar 0 */
#define CAVM_NIX_LF_RX_SECRETX(x)		(0x0000 + ((x) & 0x7) * 8)
#define CAVM_NIX_LF_GINT			(0x0200)
#define CAVM_NIX_LF_GINT_W1S			(0x0208)
#define CAVM_NIX_LF_GINT_ENA_W1C		(0x0210)
#define CAVM_NIX_LF_GINT_ENA_W1S		(0x0218)
#define CAVM_NIX_LF_ERR_INT			(0x0220)
#define CAVM_NIX_LF_ERR_INT_W1S			(0x0228)
#define CVMX_NIX_LF_ERR_INT_ENA_W1C			(0x0230)
#define CVMX_NIX_LF_ERR_INT_W1S			(0x0238)
#define CVMX_NIX_LF_RAS				(0x0240)
#define CVMX_NIX_LF_RAS_W1S			(0x0248)
#define CVMX_NIX_LF_RAS_ENA_W1C			(0x0250)
#define CVMX_NIX_LF_RAS_ENA_W1S			(0x0258)
#define CVMX_NIX_LF_SQ_OP_ERR_DBG		(0x0260)
#define CVMX_NIX_LF_MNQ_ERR_DBG			(0x0270)
#define CVMX_NIX_LF_SEND_ERR_DBG		(0x0280)

#define CAVM_NIX_LF_TX_STATX(x)			(0x0300 + ((x) & 0x7) * 8)
#define CAVM_NIX_LF_RX_STATX(x)			(0x0400 + ((x) & 0xf) * 8)
#define CAVM_NIX_LF_OP_SENDX(x)			(0x0800 + ((x) & 0xf) * 8)

#define CAVM_NIX_LF_RQ_OP_INT			(0x0900)
#define CAVM_NIX_LF_RQ_OP_OCTS			(0x0910)
#define CAVM_NIX_LF_RQ_OP_PKTS			(0x0920)
#define CAVM_NIX_LF_RQ_OP_RE_PKTS		(0x0950)

#define CAVM_NIX_LF_SQ_OP_INT			(0x0a00)
#define CAVM_NIX_LF_SQ_OP_OCTS			(0x0a10)
#define CAVM_NIX_LF_SQ_OP_PKTS			(0x0a20)
#define CAVM_NIX_LF_SQ_OP_STATUS		(0x0a30)

#define CAVM_NIX_LF_CQ_OP_INT			(0x0b00)
#define CAVM_NIX_LF_CQ_OP_DOOR			(0x0b30)
#define CAVM_NIX_LF_CQ_OP_STATUS		(0x0b40)

#define CAVM_NIX_LF_QINTX_CNT(x)		\
			(0x0c00 + ((x) & 0x3f) * 0x10000)
#define CAVM_NIX_LF_QINTX_INT(x)		\
			(0x0c10 + ((x) & 0x3f) * 0x10000)
#define CAVM_NIX_LF_QINTX_INT_W1S(x)		\
			(0x0c18 + ((x) & 0x3f) * 0x10000)
#define CAVM_NIX_LF_QINTX_ENA_W1S(x)		\
			(0x0c20 + ((x) & 0x3f) * 0x10000)
#define CAVM_NIX_LF_QINTX_ENA_W1C(x)		\
			(0x0c30 + ((x) & 0x3f) * 0x10000)

#define CAVM_NIX_LF_CINTX_CNT(x)		\
			(0x0d00 + ((x) & 0x3f) * 0x1000)
#define CAVM_NIX_LF_CINTX_WAIT(x)		\
			(0x0d10 + ((x) & 0x3f) * 0x1000)
#define CAVM_NIX_LF_CINTX_INT(x)		\
			(0x0d20 + ((x) & 0x3f) * 0x1000)
#define CAVM_NIX_LF_CINTX_INT_W1S(x)		\
			(0x0d30 + ((x) & 0x3f) * 0x1000)
#define CAVM_NIX_LF_CINTX_ENA_W1S(x)		\
			(0x0d40 + ((x) & 0x3f) * 0x1000)
#define CAVM_NIX_LF_CINTX_ENA_W1C(x)		\
			(0x0d50 + ((x) & 0x3f) * 0x1000)

/* Data structures */
/**
 * Structure nix_aq_inst_s
 *
 * NIX Admin Queue Instruction Structure
 * This structure specifies the AQ instruction.
 * Instructions and associated software structures are stored in memory as
 * little-endian unless NIX_AF_CFG[AF_BE] is set.
 *
 * Hardware reads of NIX_AQ_INST_S do not allocate into LLC.
 *
 * Hardware reads and writes of the context structure selected by [CTYPE], [LF]
 * and [CINDEX] use the NDC and LLC caching style configured for that context. For
 * example:
 * * When [CTYPE] = NIX_AQ_CTYPE_E::RQ: use NIX_AF_LF()_RQS_CFG[CACHING] and
 * NIX_AF_LF()_RQS_CFG[WAY_MASK].
 * * When [CTYPE] = NIX_AQ_CTYPE_E::MCE: use NIX_AF_RX_MCAST_CFG[CACHING] and
 * NIX_AF_RX_MCAST_CFG[WAY_MASK].
 */
union cavm_nix_aq_inst_s {
	u64 u[2];
	struct cavm_nix_aq_inst_s_s {
		u64 op:4;		     /**< [  3:  0] Instruction op code enumerated by NIX_AQ_INSTOP_E. */
		u64 ctype:4;	     /**< [  7:  4] Context type of instruction enumerated by NIX_AQ_CTYPE_E. */
		u64 lf:7;		     /**< [ 14:  8] Local function. Software must map the LF to a PF and function with
                                                                 NIX_PRIV_LF()_CFG[PF_FUNC] before issuing the AQ instruction.
                                                                 NIX_PRIV_LF()_CFG[ENA] is not required to be set when executing AQ
                                                                 instructions.

                                                                 Internal:
                                                                 Hardware uses PF(0)'s stream ID when accessing hardware context structures
                                                                 in LLC/DRAM, but NDC tracks the LF for context structures in its cache
                                                                 using the PF_FUNC's stream ID. */
		u64 reserved_15_23:9;
		u64 cindex:20;	     /**< [ 43: 24] Context index. Index of context of type [CTYPE] within [LF]. For example,
                                                                 if [CTYPE] = NIX_AQ_CTYPE_E::RQ, this is the RQ index within the [LF].

                                                                 If [CTYPE] = NIX_AQ_CTYPE_E::DYNO, index to array of
                                                                 1 \<\< NIX_AF_CONST3[DYNO_ARRAY_LOG2COUNTERS]
                                                                 IPSEC dynamic ordering counters, starting at counter index
                                                                 (1 \<\< NIX_AF_CONST3[DYNO_ARRAY_LOG2COUNTERS]) * [CINDEX].
                                                                 See NIX_AF_LF()_RX_IPSEC_DYNO_CFG[DYNO_ENA]. */
		u64 reserved_44_62:19;
		u64 doneint:1;	     /**< [ 63: 63] Done interrupt.
                                                                 0 = No interrupts related to this instruction.
                                                                 1 = When the instruction completes, NIX_AF_AQ_DONE[DONE] will be incremented,
                                                                 and based on the rules described there, an interrupt may occur. */
		u64 res_addr:64;	     /**< [127: 64] Result IOVA. Specifies where to write NIX_AQ_RES_S.

                                                                 Bits \<6:0\> must be zero; address must be 16-byte aligned. Bits \<63:53\> are
                                                                 ignored by hardware; software should use a sign-extended bit \<52\> for forward
                                                                 compatibility.

                                                                 Software must reserve one, two or three 128-byte cache lines at this
                                                                 address, as follows:
                                                                 * When [OP] = NIX_AQ_INSTOP_E::INIT or READ, software must reserve at least
                                                                 two cache lines.
                                                                 * When [OP] = NIX_AQ_INSTOP_E::WRITE, software must reserve at least three
                                                                 cache lines.
                                                                 * Otherwise, software must reserve at least one cache line.

                                                                 Hardware always stores full cache lines when writing NIX_AQ_RES_S and
                                                                 following context structures (e.g. NIX_RQ_CTX_S), if any.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<6:0\> are ignored by hardware, treated as always 0x0. */
	} s;
	/* struct cavm_nix_aq_inst_s_s cn; */
};

/**
 * Structure nix_aq_res_s
 *
 * NIX Admin Queue Result Structure
 * NIX writes this structure after it completes the NIX_AQ_INST_S instruction.
 * The result structure is exactly 16 bytes, and each instruction completion produces
 * exactly one result structure.
 *
 * Results and associated software structures are stored in memory as
 * little-endian unless NIX_AF_CFG[AF_BE] is set.
 *
 * When [OP] = NIX_AQ_INSTOP_E::INIT, WRITE or READ, this structure is
 * immediately followed by context read or write data. See NIX_AQ_INSTOP_E.
 *
 * Hardware writes of NIX_AQ_RES_S and context data always allocate into LLC.
 * Hardware reads of context data do not allocate into LLC.
 */
union cavm_nix_aq_res_s {
	u64 u[2];
	struct cavm_nix_aq_res_s_s {
		u64 op:4;		     /**< [  3:  0] Copy of NIX_AQ_INST_S[OP] for the completed instruction; enumerated by
                                                                 NIX_AQ_INSTOP_E. */
		u64 ctype:4;	     /**< [  7:  4] Copy of NIX_AQ_INST_S[CTYPE] for the completed instruction; enumerated by
                                                                 NIX_AQ_CTYPE_E. */
		u64 compcode:8;	     /**< [ 15:  8] Indicates completion/error status of the NIX coprocessor for the associated
                                                                 instruction, as enumerated by NIX_AQ_COMP_E. Core software may write the memory
                                                                 location containing [COMPCODE] to 0x0 before ringing the doorbell, and then poll
                                                                 for completion by checking for a nonzero value.

                                                                 Once the core observes a nonzero [COMPCODE] value in this case, NIX will have also
                                                                 completed LLC/DRAM reads and writes for the operation. */
		u64 doneint:1;	     /**< [ 16: 16] Done interrupt. This bit is copied from the corresponding instruction's
                                                                 NIX_AQ_INST_S[DONEINT]. */
		u64 reserved_17_63:47;
		u64 reserved_64_127:64;
	} s;
	/* struct cavm_nix_aq_res_s_s cn; */
};

/**
 * Structure nix_cint_hw_s
 *
 * NIX Completion Interrupt Context Hardware Structure
 * This structure contains context state maintained by hardware for each
 * completion interrupt (CINT) in NDC/LLC/DRAM. Software accesses this structure
 * with the NIX_LF_CINT()* registers.
 * Hardware maintains a table of NIX_AF_CONST2[CINTS] contiguous NIX_CINT_HW_S
 * structures per LF starting at AF IOVA NIX_AF_LF()_CINTS_BASE.
 * Always stored in byte invariant little-endian format (LE8).
 */
union cavm_nix_cint_hw_s {
	u64 u[2];
	struct cavm_nix_cint_hw_s_s {
		u64 ecount:32;	     /**< [ 31:  0] Entry count. See NIX_LF_CINT()_CNT[ECOUNT]. */
		u64 qcount:16;	     /**< [ 47: 32] Active queue count. See NIX_LF_CINT()_CNT[QCOUNT]. */
		u64 intr:1;	     /**< [ 48: 48] Interrupt status. See also NIX_LF_CINT()_INT and NIX_LF_CINT()_INT_W1S. */
		u64 ena:1;		     /**< [ 49: 49] Interrupt enable. See also NIX_LF_CINT()_ENA_W1S and
                                                                 NIX_LF_CINT()_ENA_W1C. */
		u64 timer_idx:8;	     /**< [ 57: 50] Timer index of NIX_AF_CINT_TIMER(). Select the TIMER for the CINT. */
		u64 reserved_58_63:6;
		u64 ecount_wait:32;     /**< [ 95: 64] Entry count hold-off. See NIX_LF_CINT()_WAIT[ECOUNT_WAIT]. */
		u64 qcount_wait:16;     /**< [111: 96] Active queue count hold-off. See NIX_LF_CINT()_WAIT[QCOUNT_WAIT]. */
		u64 time_wait:8;	     /**< [119:112] Time hold-off. See NIX_LF_CINT()_WAIT[TIME_WAIT]. */
		u64 reserved_120_127:8;
	} s;
	/* struct cavm_nix_cint_hw_s_s cn; */
};

/**
 * Structure nix_cq_ctx_s
 *
 * NIX Completion Queue Context Structure
 * This structure contains context state maintained by hardware for each CQ in
 * NDC/LLC/DRAM.
 * Software uses the same structure format to read and write an CQ context with
 * the NIX admin queue.
 */
union cavm_nix_cq_ctx_s {
	u64 u[4];
	struct cavm_nix_cq_ctx_s_s {
		u64 base:64;	     /**< [ 63:  0] Base LF IOVA of CQ ring in LLC/DRAM.

                                                                 Bits \<8:0\> must be zero; address must be 512-byte aligned.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended
                                                                 bit \<52\> for forward compatibility.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<6:0\> are ignored by hardware, treated as always 0x0. */
		u64 qsize:4;	     /**< [ 67: 64] Specifies CQ ring size in number of CQEs:
                                                                 0x0 = 16 CQEs.
                                                                 0x1 = 64 CQEs.
                                                                 0x2 = 256 CQEs.
                                                                 0x3 = 1K CQEs.
                                                                 0x4 = 4K CQEs.
                                                                 0x5 = 16K CQEs.
                                                                 0x6 = 64K CQEs.
                                                                 0x7 = 256K CQEs.
                                                                 0x8 = 1M CQEs.
                                                                 0x9-0xF = Reserved.

                                                                 The CQE size is selected by NIX_AF_LF()_CFG[XQE_SIZE].

                                                                 Note that the usable size of the ring is the specified size minus one
                                                                 ([HEAD]==[TAIL] always means empty). */
		u64 drop_ena:1;	     /**< [ 68: 68] Enable RQ packet DROP based on the [DROP] level. */
		u64 reserved_69_71:3;
		u64 drop:8;	     /**< [ 79: 72] If [DROP_ENA] is set for a received packet, the packet will be
                                                                 dropped if the current 8-bit shifted count is less than or equal to this
                                                                 value.
                                                                 See shifted_CNT in [AVG_CON]. */
		u64 ena:1;		     /**< [ 80: 80] CQ enable. */
		u64 reserved_81_83:3;
		u64 qint_idx:7;	     /**< [ 90: 84] Error queue interrupt index. Select the QINT within LF (index {a} of
                                                                 NIX_LF_QINT()*) which receives [CQ_ERR_INT] events.

                                                                 Internal:
                                                                 See QINT message generation note in NIX_RQ_CTX_S[QINT_IDX]. */
		u64 cq_err:1;	     /**< [ 91: 91] CQ error. Set along with the NIX_CQERRINT_E::WR_FULL bit in [CQ_ERR_INT]
                                                                 when the corresponding error is detected for a transmit CQE. The CQ is
                                                                 stopped and all new CQEs to be added to it are dropped. */
		u64 cint_idx:7;	     /**< [ 98: 92] Completion interrupt index. Select the CINT within LF (index {a} of
                                                                 NIX_LF_CINT()*) which receives completion events for
                                                                 this CQ. */
		u64 avg_con:9;	     /**< [107: 99] This value controls how much of the present average resource level is used
                                                                 to calculate the new resource level. The value is a number from zero to 256,
                                                                 which represents [AVG_CON]/256 of the average resource level that will be
                                                                 used in the calculation.

                                                                 NIX updates the average resource level as follows whenever the immediate resource
                                                                 count changes:

                                                                 \<pre\>
                                                                 // Eight-bit shifted count (1/256 units of queue size); higher count
                                                                 // indicates more free resources:
                                                                 if ([QSIZE] \>= 2) {
                                                                   shifted_CNT = 255 - ((([TAIL] - [HEAD]) \>\> (2 * ([QSIZE] - 2))) % 256);
                                                                 } else {
                                                                   shifted_CNT = 255 - ((([TAIL] - [HEAD]) \<\< (2 * (2 - [QSIZE]))) % 256);
                                                                 }
                                                                 adjusted_CON = [AVG_CON] \>\> ceil(log2(NIX_AF_AVG_DELAY[AVG_TIMER] - [UPDATE_TIME]));
                                                                 [AVG_LEVEL] = (adjusted_CON * [AVG_LEVEL] + (256 - adjusted_CON)
                                                                               * shifted_CNT) / 256;
                                                                 [UPDATE_TIME] = NIX_AF_AVG_DELAY[AVG_TIMER];
                                                                 \</pre\>

                                                                 Note setting this value to zero will disable averaging, and always use the most
                                                                 immediate levels. NIX_AF_AVG_DELAY[AVG_DLY] controls the periodicity of the level
                                                                 calculations. */
		u64 wrptr:20;	     /**< [127:108] Internal pointer for writing to the CQ ring. */
		u64 tail:20;	     /**< [147:128] Tail CQE pointer.
                                                                 Hardware advances [TAIL] when an entry written to the CQ is committed and
                                                                 visible to software.
                                                                 The tail LF IOVA is [BASE] + ([TAIL] * 512) when NIX_AF_LF()_CFG[XQE_SIZE] = NIX_XQESZ_E::W64,
                                                                 [BASE] + ([TAIL] * 128) when NIX_AF_LF()_CFG[XQE_SIZE] = NIX_XQESZ_E::W16. */
		u64 head:20;	     /**< [167:148] Head CQE pointer.
                                                                 Hardware advances [HEAD] when software writes to NIX_LF_CQ_OP_DOOR for
                                                                 this CQ.
                                                                 The head LF IOVA is [BASE] + ([HEAD] * 512) when NIX_AF_LF()_CFG[XQE_SIZE] = NIX_XQESZ_E::W64,
                                                                 [BASE] + ([HEAD] * 128) when NIX_AF_LF()_CFG[XQE_SIZE] = NIX_XQESZ_E::W16. */
		u64 avg_level:8;	     /**< [175:168] Current moving average of the eight-bit shifted count. The higher [AVG_LEVEL]
                                                                 is, the more free resources. The lower levels indicate buffer exhaustion.
                                                                 See [AVG_CON].

                                                                 NIX uses [AVG_LEVEL] in receive queue QOS calculations. See
                                                                 NIX_RQ_CTX_S[XQE_DROP]. */
		u64 update_time:16;     /**< [191:176] NIX_AF_AVG_DELAY[AVG_TIMER] value captured when [AVG_LEVEL] is updated. */
		u64 bp:8;		     /**< [199:192] Backpressure is asserted if [BP_ENA] bit is set and the current eight-bit
                                                                 shifted count is less than or equal to this value.
                                                                 See shifted_CNT in [AVG_CON]. */
		u64 bpid:9;	     /**< [208:200] Backpressure ID (index {a} of NIX_AF_RX_BPID()_STATUS) to which
                                                                 backpressure is asserted when [BP_ENA] bit is set. */
		u64 bp_ena:1;	     /**< [209:209] Enable CQ backpressure based on [BP] level. */
		u64 reserved_210_211:2;
		u64 substream:20;	     /**< [231:212] Substream ID used for writing CQEs to the CQ ring. */
		u64 caching:1;	     /**< [232:232] Selects the style of CQE write to the LLC.
                                                                 0 = Writes of CQE data will not allocate into the LLC.
                                                                 1 = Writes of CQE data are allocated into the LLC. */
		u64 reserved_233_239:7;
		u64 cq_err_int:8;	     /**< [247:240] Error interrupts. Bits enumerated by NIX_CQERRINT_E, which also defines when
                                                                 hardware sets each bit. Software can read, set or clear these bits with
                                                                 NIX_LF_CQ_OP_INT. */
		u64 cq_err_int_ena:8;   /**< [255:248] Error interrupt enables. Bits enumerated by NIX_CQERRINT_E. Software can read,
                                                                 set or clear these bits with NIX_LF_CQ_OP_INT. */
	} s;
	/* struct cavm_nix_cq_ctx_s_s cn; */
};

/**
 * Structure nix_cqe_hdr_s
 *
 * NIX Completion Queue Entry Header Structure
 * This 64-bit structure defines the first word of every CQE. It is immediately
 * followed by NIX_RX_PARSE_S in a receive CQE, and by NIX_SEND_COMP_S in a send
 * completion CQE.
 * Stored in memory as little-endian unless NIX_AF_LF()_CFG[BE] is set.
 */
union cavm_nix_cqe_hdr_s {
	u64 u;
	struct cavm_nix_cqe_hdr_s_s {
		u64 tag:32;	     /**< [ 31:  0] Tag computed for the RX packet. Valid for receive descriptor only.
                                                                 See pseudocode in NIX_RQ_CTX_S[LTAG]. */
		u64 q:20;		     /**< [ 51: 32] RQ or SQ within VF/PF. */
		u64 reserved_52_57:6;
		u64 node:2;	     /**< [ 59: 58] Node number on which the packet was received or transmitted.
                                                                 Internal:
                                                                 This is needed by software; do not remove on single-node parts. */
		u64 cqe_type:4;	     /**< [ 63: 60] Completion queue entry type. Enumerated by NIX_XQE_TYPE_E. */
	} s;
	/* struct cavm_nix_cqe_hdr_s_s cn; */
};

/**
 * Structure nix_inst_hdr_s
 *
 * NIX Instruction Header Structure
 * This structure defines the instruction header that precedes the packet header
 * supplied to NPC for packets to be transmitted by NIX.
 */
union cavm_nix_inst_hdr_s {
	u64 u;
	struct cavm_nix_inst_hdr_s_s {
		u64 pf_func:16;	     /**< [ 15:  0] PF and function transmitting the packet. Format specified by
                                                                 RVU_PF_FUNC_S. */
		u64 sq:20;		     /**< [ 35: 16] Send queue within [PF_FUNC]. */
		u64 reserved_36_63:28;
	} s;
	/* struct cavm_nix_inst_hdr_s_s cn; */
};

/**
 * Structure nix_iova_s
 *
 * NIX I/O Virtual Address Structure
 */
union cavm_nix_iova_s {
	u64 u;
	struct cavm_nix_iova_s_s {
		u64 addr:64;	     /**< [ 63:  0] I/O virtual address. Bits \<63:53\> are ignored by hardware; software should use a
                                                                 sign-extended bit \<52\> for forward compatibility. */
	} s;
	/* struct cavm_nix_iova_s_s cn; */
};

/**
 * Structure nix_mem_result_s
 *
 * NIX Memory Value Structure
 * When NIX_SEND_MEM_S[ALG]=NIX_SENDMEMALG_E::SETRSLT, the value written to memory is formed with
 * this structure.
 */
union cavm_nix_mem_result_s {
	u64 u;
	struct cavm_nix_mem_result_s_s {
		u64 v:1;		     /**< [  0:  0] Valid. Always set by hardware so software can distinguish from data that was (presumed to
                                                                 be) zeroed by software before the send operation. */
		u64 color:2;	     /**< [  2:  1] Final color of the packet. Enumerated by NIX_COLORRESULT_E. */
		u64 reserved_3_63:61;
	} s;
	/* struct cavm_nix_mem_result_s_s cn; */
};

/**
 * Structure nix_op_q_wdata_s
 *
 * NIX Statistics Operation Write Data Structure
 * This structure specifies the write data format of an atomic 64-bit load-and-add
 * of some NIX_LF_RQ_OP_*, NIX_LF_SQ_OP* and NIX_LF_CQ_OP* registers.
 */
union cavm_nix_op_q_wdata_s {
	u64 u;
	struct cavm_nix_op_q_wdata_s_s {
		u64 reserved_0_31:32;
		u64 q:20;		     /**< [ 51: 32] Queue within LF (RQ, SQ or CQ). */
		u64 reserved_52_63:12;
	} s;
	/* struct cavm_nix_op_q_wdata_s_s cn; */
};

/**
 * Structure nix_qint_hw_s
 *
 * NIX Queue Interrupt Context Hardware Structure
 * This structure contains context state maintained by hardware for each queue
 * interrupt (QINT) in NDC/LLC/DRAM. Software accesses this structure with the
 * NIX_LF_QINT()* registers.
 * Hardware maintains a table of NIX_AF_CONST2[QINTS] contiguous NIX_QINT_HW_S
 * structures per LF starting at IOVA NIX_AF_LF()_QINTS_BASE.
 * Always stored in byte invariant little-endian format (LE8).
 */
union cavm_nix_qint_hw_s {
	uint32_t u;
	struct cavm_nix_qint_hw_s_s {
		uint32_t count:22;	     /**< [ 21:  0] Interrupt count. See NIX_LF_QINT()_CNT[COUNT]. */
		uint32_t reserved_22_30:9;
		uint32_t ena:1;		     /**< [ 31: 31] Interrupt enable. See also NIX_LF_QINT()_ENA_W1S[INTR] and
                                                                 NIX_LF_QINT()_ENA_W1C[INTR]. */
	} s;
	/* struct cavm_nix_qint_hw_s_s cn; */
};

/**
 * Structure nix_rq_ctx_hw_s
 *
 * NIX Receive Queue Context Structure
 * This structure contains context state maintained by hardware for each RQ in
 * NDC/LLC/DRAM. Software uses the equivalent NIX_RQ_CTX_S structure format to
 * read and write an RQ context with the NIX admin queue.
 * Always stored in byte invariant little-endian format (LE8).
 */
union cavm_nix_rq_ctx_hw_s {
	u64 u[16];
	struct cavm_nix_rq_ctx_hw_s_s {
		u64 ena:1;		     /**< [  0:  0] See NIX_RQ_CTX_S[ENA]. */
		u64 sso_ena:1;	     /**< [  1:  1] See NIX_RQ_CTX_S[SSO_ENA]. */
		u64 ipsech_ena:1;	     /**< [  2:  2] See NIX_RQ_CTX_S[IPSECH_ENA]. */
		u64 ena_wqwd:1;	     /**< [  3:  3] See NIX_RQ_CTX_S[ENA_WQWD]. */
		u64 cq:20;		     /**< [ 23:  4] See NIX_RQ_CTX_S[CQ]. */
		u64 substream:20;	     /**< [ 43: 24] See NIX_RQ_CTX_S[SUBSTREAM]. */
		u64 wqe_aura:20;	     /**< [ 63: 44] See NIX_RQ_CTX_S[WQE_AURA]. */
		u64 spb_aura:20;	     /**< [ 83: 64] See NIX_RQ_CTX_S[SPB_AURA]. */
		u64 lpb_aura:20;	     /**< [103: 84] See NIX_RQ_CTX_S[LPB_AURA]. */
		u64 sso_grp:10;	     /**< [113:104] See NIX_RQ_CTX_S[SSO_GRP]. */
		u64 sso_tt:2;	     /**< [115:114] See NIX_RQ_CTX_S[SSO_TT]. */
		u64 pb_caching:2;	     /**< [117:116] See NIX_RQ_CTX_S[PB_CACHING]. */
		u64 wqe_caching:1;	     /**< [118:118] See NIX_RQ_CTX_S[WQE_CACHING]. */
		u64 xqe_drop_ena:1;     /**< [119:119] See NIX_RQ_CTX_S[XQE_DROP_ENA]. */
		u64 spb_drop_ena:1;     /**< [120:120] See NIX_RQ_CTX_S[SPB_DROP_ENA]. */
		u64 lpb_drop_ena:1;     /**< [121:121] See NIX_RQ_CTX_S[LPB_DROP_ENA]. */
		u64 wqe_skip:2;	     /**< [123:122] See NIX_RQ_CTX_S[WQE_SKIP]. */
		u64 reserved_124_127:4;
		u64 reserved_128_139:12;
		u64 spb_sizem1:6;	     /**< [145:140] See NIX_RQ_CTX_S[SPB_SIZEM1]. */
		u64 reserved_146_150:5;
		u64 spb_ena:1;	     /**< [151:151] See NIX_RQ_CTX_S[SPB_ENA]. */
		u64 lpb_sizem1:12;	     /**< [163:152] See NIX_RQ_CTX_S[LPB_SIZEM1]. */
		u64 first_skip:7;	     /**< [170:164] See NIX_RQ_CTX_S[FIRST_SKIP]. */
		u64 reserved_171:1;
		u64 later_skip:6;	     /**< [177:172] See NIX_RQ_CTX_S[LATER_SKIP]. */
		u64 xqe_imm_size:6;     /**< [183:178] See NIX_RQ_CTX_S[XQE_IMM_SIZE]. */
		u64 reserved_184_189:6;
		u64 xqe_imm_copy:1;     /**< [190:190] See NIX_RQ_CTX_S[XQE_IMM_COPY]. */
		u64 xqe_hdr_split:1;    /**< [191:191] See NIX_RQ_CTX_S[XQE_HDR_SPLIT]. */
		u64 xqe_drop:8;	     /**< [199:192] See NIX_RQ_CTX_S[XQE_DROP]. */
		u64 xqe_pass:8;	     /**< [207:200] See NIX_RQ_CTX_S[XQE_PASS]. */
		u64 wqe_pool_drop:8;    /**< [215:208] See NIX_RQ_CTX_S[WQE_POOL_DROP]. */
		u64 wqe_pool_pass:8;    /**< [223:216] See NIX_RQ_CTX_S[WQE_POOL_PASS]. */
		u64 spb_aura_drop:8;    /**< [231:224] See NIX_RQ_CTX_S[SPB_AURA_DROP]. */
		u64 spb_aura_pass:8;    /**< [239:232] See NIX_RQ_CTX_S[SPB_AURA_PASS]. */
		u64 spb_pool_drop:8;    /**< [247:240] See NIX_RQ_CTX_S[SPB_POOL_DROP]. */
		u64 spb_pool_pass:8;    /**< [255:248] See NIX_RQ_CTX_S[SPB_POOL_PASS]. */
		u64 lpb_aura_drop:8;    /**< [263:256] See NIX_RQ_CTX_S[LPB_AURA_DROP]. */
		u64 lpb_aura_pass:8;    /**< [271:264] See NIX_RQ_CTX_S[LPB_AURA_PASS]. */
		u64 lpb_pool_drop:8;    /**< [279:272] See NIX_RQ_CTX_S[LPB_POOL_DROP]. */
		u64 lpb_pool_pass:8;    /**< [287:280] See NIX_RQ_CTX_S[LPB_POOL_PASS]. */
		u64 reserved_288_319:32;
		u64 ltag:24;	     /**< [343:320] See NIX_RQ_CTX_S[LTAG]. */
		u64 good_utag:8;	     /**< [351:344] See NIX_RQ_CTX_S[GOOD_UTAG]. */
		u64 bad_utag:8;	     /**< [359:352] See NIX_RQ_CTX_S[BAD_UTAG]. */
		u64 flow_tagw:6;	     /**< [365:360] See NIX_RQ_CTX_S[FLOW_TAGW]. */
		u64 reserved_366_383:18;
		u64 octs:48;	     /**< [431:384] See NIX_RQ_CTX_S[OCTS]. */
		u64 reserved_432_447:16;
		u64 pkts:48;	     /**< [495:448] See NIX_RQ_CTX_S[PKTS]. */
		u64 reserved_496_511:16;
		u64 drop_octs:48;	     /**< [559:512] See NIX_RQ_CTX_S[DROP_OCTS]. */
		u64 reserved_560_575:16;
		u64 drop_pkts:48;	     /**< [623:576] See NIX_RQ_CTX_S[DROP_PKTS]. */
		u64 reserved_624_639:16;
		u64 re_pkts:48;	     /**< [687:640] See NIX_RQ_CTX_S[RE_PKTS]. */
		u64 reserved_688_702:15;
		u64 ena_copy:1;	     /**< [703:703] See NIX_RQ_CTX_S[ENA] */
		u64 reserved_704_739:36;
		u64 rq_int:8;	     /**< [747:740] See NIX_RQ_CTX_S[RQ_INT]. */
		u64 rq_int_ena:8;	     /**< [755:748] See NIX_RQ_CTX_S[RQ_INT_ENA]. */
		u64 qint_idx:7;	     /**< [762:756] See NIX_RQ_CTX_S[QINT_IDX]. */
		u64 reserved_763_767:5;
		u64 reserved_768_831:64;
		u64 reserved_832_895:64;
		u64 reserved_896_959:64;
		u64 reserved_960_1023:64;
	} s;
	/* struct cavm_nix_rq_ctx_hw_s_s cn; */
};

/**
 * Structure nix_rq_ctx_s
 *
 * NIX Receive Queue Context Structure
 * This structure specifies the format used by software to read and write an RQ context with
 * the NIX admin queue.
 */
union cavm_nix_rq_ctx_s {
	u64 u[16];
	struct cavm_nix_rq_ctx_s_s {
		u64 ena:1;		     /**< [  0:  0] RQ enable. */
		u64 sso_ena:1;	     /**< [  1:  1] WQE enable. Selects the receive descriptor type and destination generated for the LF.
                                                                 0 = The descriptor type is a CQE written to the CQ ring selected by NIX_RQ_CTX_S[CQ].
                                                                 1 = The descriptor type is a WQE sent to SSO. */
		u64 ipsech_ena:1;	     /**< [  2:  2] IPSEC hardware fast-path enable. When set along with [SSO_ENA], packets
                                                                 with NIX_RX_ACTION_S[OP] = NIX_RX_ACTIONOP_E::UCAST_IPSEC may use the IPSEC
                                                                 hardware fast-path subject to other packet checks. */
		u64 ena_wqwd:1;	     /**< [  3:  3] Enable WQE with data. Not used when [SSO_ENA] is clear.

                                                                 When [SSO_ENA] and [ENA_WQWD] are both set, [SPB_ENA] must be clear and the WQE is
                                                                 written at the beginning of the packet's first buffer allocated from [LPB_AURA], and
                                                                 the packet data starts at word offset [FIRST_SKIP] in the buffer.

                                                                 When [SSO_ENA] is set and [ENA_WQWD] is clear, the WQE is written to a
                                                                 dedicated buffer allocated from [WQE_AURA]. */
		u64 cq:20;		     /**< [ 23:  4] Completion Queue for this SQ. */
		u64 substream:20;	     /**< [ 43: 24] Substream ID of LF IOVA pointers allocated from [WQE_AURA,SPB_AURA,LPB_AURA]. */
		u64 wqe_aura:20;	     /**< [ 63: 44] WQE aura. Aura within NIX_AF_LF()_CFG[NPA_PF_FUNC] for allocating SSO
                                                                 work-queue entry buffers.
                                                                 Valid when [SSO_ENA] is set and [ENA_WQWD] is clear. */
		u64 spb_aura:20;	     /**< [ 83: 64] Small packet buffer aura. Aura within NIX_AF_LF()_CFG[NPA_PF_FUNC] for
                                                                 allocating data buffers for small packets.
                                                                 Packet data is written to WQE/CQE, SPB and/or LPBs as
                                                                 described by the following pseudocode:

                                                                 \<pre\>
                                                                 apad = nix_calc_alignment_pad(...); // see NIX_AF_LF()_RX_CFG[DIS_APAD]
                                                                 pkt_bytes_padded = NIX_RX_PARSE_S[PKT_LENM1] + 1 + apad;
                                                                 spb_bytes = 8*([SPB_SIZEM1] + 1 - [FIRST_SKIP]);
                                                                 imm_write = False;
                                                                 spb_write = False;
                                                                 lpb_write = False;

                                                                 if ([XQE_HDR_SPLIT]) {
                                                                    imm_bytes = min(NIX_RX_PARSE_S[EOH_PTR], 8*[XQE_IMM_SIZE]);
                                                                 } else {
                                                                    imm_bytes = 8*[XQE_IMM_SIZE];
                                                                 }

                                                                 if (imm_bytes == 0) {
                                                                    if ([SPB_ENA] && (pkt_bytes_padded \<= spb_bytes)) {
                                                                       spb_write = True;  // Write packet to an SPB
                                                                    } else {
                                                                       lpb_write = True;  // Write packet to one or more LPBs
                                                                    }
                                                                 }
                                                                 else { // imm_bytes \> 0
                                                                    // Write alignment pad and first imm_bytes of packet (or entire packet
                                                                    // if smaller) to WQE/CQE.
                                                                    imm_write = True;
                                                                    imm_bytes_padded = imm_bytes + apad;

                                                                    if ([XQE_IMM_COPY]) {
                                                                       // Include copy of first imm_bytes in SPB or LPB.
                                                                       if (pkt_bytes_padded \<= spb_bytes) {
                                                                          spb_write = True;  // Write packet to an SPB
                                                                       } else {
                                                                          lpb_write = True;  // Write packet to one or more LPBs
                                                                       }
                                                                    }
                                                                    else {
                                                                       if (pkt_bytes_padded \<= imm_bytes_padded) {
                                                                          ;  // No remaining packet data. Done.
                                                                       } else if ((pkt_bytes_padded - imm_bytes_padded) \<= spb_bytes) {
                                                                          // Write remaining packet data to an SPB
                                                                          spb_write = True;
                                                                       } else {
                                                                          // Write remaining packet data to one or more LPBs
                                                                          lpb_write = True;
                                                                       }
                                                                    }
                                                                 }
                                                                 \</pre\> */
		u64 lpb_aura:20;	     /**< [103: 84] Large packet buffer aura. See [SPB_AURA]. */
		u64 sso_grp:10;	     /**< [113:104] SSO group for the packet's SSO add work, and to store in NIX_WQE_HDR_S[GRP]. Valid
                                                                 when [SSO_ENA] is set.
                                                                 Bits \<9..8\> must be zero. */
		u64 sso_tt:2;	     /**< [115:114] SSO tag type for the packet's SSO add work and to store in NIX_WQE_HDR_S[TT].
                                                                 Enumerated by SSO_TT_E. Valid when [SSO_ENA] is set. */
		u64 pb_caching:2;	     /**< [117:116] Packet buffer caching. Selects the style of packet buffer write to LLC/DRAM packet.
                                                                 0x0 = Writes of SPB/LPB data will not allocate into the LLC.
                                                                 0x1 = All writes of SPB/LPB data are allocated into the LLC.
                                                                 0x2 = First aligned cache block is allocated into the LLC. All remaining cache
                                                                 blocks are not allocated.
                                                                 0x3 = First two aligned cache blocks are allocated into the LLC. All remaining
                                                                 cache blocks are not allocated. */
		u64 wqe_caching:1;	     /**< [118:118] WQE caching. Selects the style of work-queue entry write to LLC/DRAM.
                                                                 0 = Writes of WQE data will not allocate into LLC.
                                                                 1 = Writes of WQE data are allocated into LLC.

                                                                 Valid when [SSO_ENA] is set. */
		u64 xqe_drop_ena:1;     /**< [119:119] WQE/CQE drop enable. When [SSO_ENA] is set and [ENA_WQWD] is clear, request
                                                                 NPA to do DROP processing on [WQE_AURA]; see NPA_AURA_S[AURA_DROP] and
                                                                 NPA_AURA_S[POOL_DROP]. When [SSO_ENA] is clear, request CQ DROP processing;
                                                                 see NIX_CQ_CTX_S[DROP,DROP_ENA]. */
		u64 spb_drop_ena:1;     /**< [120:120] Request NPA to do DROP processing on [SPB_AURA] if an SPB is requested.
                                                                 See NPA_AURA_S[AURA_DROP] and NPA_AURA_S[POOL_DROP]. */
		u64 lpb_drop_ena:1;     /**< [121:121] Request NPA to do DROP processing on [LPB_AURA] if a first LPB is requested
                                                                 for a packet. See NPA_AURA_S[AURA_DROP] and NPA_AURA_S[POOL_DROP]. If
                                                                 multiple LPBs are requested for a packet, DROP processing is never
                                                                 requested for the second and subsequent LPBs. */
		u64 reserved_122_127:6;
		u64 reserved_128_139:12;
		u64 spb_sizem1:6;	     /**< [145:140] Small packet buffer size minus one. The number of eight-byte words (minus one)
                                                                 between the start of a buffer from [SPB_AURA] and the last word that NIX may
                                                                 write into that buffer. See [SPB_AURA].

                                                                 Internal:
                                                                 Limited to 6 bits (512 bytes) to enable early SPB/LBP decision and avoid
                                                                 store-and-forward of larger packets. */
		u64 wqe_skip:2;	     /**< [147:146] WQE start offset. The number of 128-byte cache lines to skip from the WQE
                                                                 buffer pointer (from [LPB_AURA] when [ENA_WQWD] is set and [WQE_AURA]
                                                                 otherwise) to the first WQE byte stored in the buffer. */
		u64 reserved_148_150:3;
		u64 spb_ena:1;	     /**< [151:151] Small packet buffer enable:

                                                                 0 = Do not use small packet buffers. All receive packets are stored in
                                                                 buffers from [LPB_AURA].

                                                                 1 = Use a single small packet buffer from [SPB_AURA] when a receive packet
                                                                 fits within that buffer.

                                                                 Must be clear when [ENA_WQWD] is set.
                                                                 See [SPB_AURA]. */
		u64 lpb_sizem1:12;	     /**< [163:152] Large packet buffer size minus one. The number of eight-byte words (minus
                                                                 one) between the start of a buffer from [LPB_AURA] and the last word that
                                                                 NIX may write into that buffer. Must be greater than or equal to
                                                                 [SPB_SIZEM1] when [SPB_ENA] is set.

                                                                 See [SPB_AURA]. */
		u64 first_skip:7;	     /**< [170:164] First buffer start offset. The number of eight-byte words from the
                                                                 [SPB_AURA] or first [LPB_AURA] buffer pointer to the first packet data byte
                                                                 stored in the buffer. Must not be greater than [LPB_SIZEM1], and when
                                                                 [SPB_ENA] is set not greater than [SPB_SIZEM1].

                                                                 When [SSO_ENA] and [ENA_WQWD] are both set, must satisfy the following
                                                                 to ensure that the WQE does not overlap with packet data:
                                                                 _ wqe_size = (NIX_AF_LF()_CFG[XQE_SIZE] == NIX_XQESZ_E::W64) ? 64 : 16.
                                                                 _ [FIRST_SKIP] \>= wqe_size + 16*[WQE_SKIP]. */
		u64 reserved_171:1;
		u64 later_skip:6;	     /**< [177:172] Later buffer start offset. The number of eight-byte words from the
                                                                 [LPB_AURA] buffer pointer (other than the packet's first buffer)
                                                                 the first byte stored in the buffer. Must not be greater than
                                                                 [LPB_SIZEM1]. */
		u64 xqe_imm_size:6;     /**< [183:178] WQE/CQE immediate size. Must not be greater than 32, and must be 0 when
                                                                 NIX_AF_LF()_CFG[XQE_SIZE] = NIX_XQESZ_E::W16.

                                                                 When nonzero, the maximum number of starting eight-byte words of immediate
                                                                 packet data written with NIX_RX_IMM_S in the receive descriptor (CQE or
                                                                 WQE), excluding any alignment padding before the immediate data (see
                                                                 NIX_AF_LF()_RX_CFG[DIS_APAD] and NIX_RX_IMM_S[APAD]).

                                                                 See also [XQE_HDR_SPLIT]. Remaining packet data (if any), or all packet
                                                                 data if [XQE_IMM_COPY] is set, is written to one or more buffers from
                                                                 [SPB_AURA] or [LPB_AURA].

                                                                 When zero, packet data is not written in the WQE/CQE; all packet data is
                                                                 written to buffers from [SPB_AURA] or [LPB_AURA].

                                                                 See pseudocode in [SPB_AURA]. */
		u64 reserved_184_189:6;
		u64 xqe_imm_copy:1;     /**< [190:190] WQE/CQE immediate data copy. When set, all packet data is written to one or
                                                                 more buffers from [SPB_AURA] or [LPB_AURA], and initial data bytes,
                                                                 including initial data bytes written to the WQE/CQE, if any. See also
                                                                 [XQE_IMM_SIZE] and [XQE_HDR_SPLIT]. */
		u64 xqe_hdr_split:1;    /**< [191:191] WQE/CQE header split.

                                                                 0 = The first 8*[XQE_IMM_SIZE] bytes (or all bytes if the packet is smaller) are
                                                                 written to the WQE/CQE irrespective of the parsed header size.

                                                                 1 = Only parsed header bytes (first NIX_RX_PARSE_S[EOH_PTR] bytes of packet) may
                                                                 be written to the WQE/CQE. The actual number of header bytes written to WQE/CQE
                                                                 is the smaller of NIX_RX_PARSE_S[EOH_PTR] or 8*[XQE_IMM_SIZE]. */
		u64 reserved_315_319:5;
		u64 qint_idx:7;	     /**< [314:308] Queue interrupt index. Select the QINT within LF (index {a} of
                                                                 NIX_LF_QINT()*) which receives [RQ_INT] events.

                                                                 Internal:
                                                                 QINT update message is generated on an interrupt update event or when [ENA]
                                                                 changes. Message op code (INCR/DECR/NOP) is based on current and next
                                                                 interrupt states:
                                                                 _ [ENA] && |([RQ_INT] & [RQ_INT_ENA]) */
		u64 rq_int_ena:8;	     /**< [307:300] RQ interrupt enables. Bits enumerated by NIX_RQINT_E. */
		u64 rq_int:8;	     /**< [299:292] RQ interrupts. Bits enumerated by NIX_RQINT_E. */
		u64 reserved_288_291:4;
		u64 lpb_pool_pass:8;    /**< [287:280] [LPB_AURA]'s average pool level pass threshold for RED.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
		u64 lpb_pool_drop:8;    /**< [279:272] [LPB_AURA]'s average pool level drop threshold for RED.

                                                                 Must be less than or equal to [LPB_POOL_PASS]. Software can set
                                                                 [LPB_POOL_DROP] = [LPB_POOL_PASS] = 0 to disable this level check in the RQ
                                                                 RED algorithm.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
		u64 lpb_aura_pass:8;    /**< [271:264] [LPB_AURA]'s average aura level pass threshold for RED.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
		u64 lpb_aura_drop:8;    /**< [263:256] [LPB_AURA]'s average aura level drop threshold for RED.

                                                                 Must be less than or equal to [LPB_AURA_PASS]. Software can set
                                                                 [LPB_AURA_DROP] = [LPB_AURA_PASS] = 0 to disable this level check in the RQ
                                                                 RED algorithm.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
#else				/* Word 4 - Little Endian */
		u64 lpb_aura_drop:8;    /**< [263:256] [LPB_AURA]'s average aura level drop threshold for RED.

                                                                 Must be less than or equal to [LPB_AURA_PASS]. Software can set
                                                                 [LPB_AURA_DROP] = [LPB_AURA_PASS] = 0 to disable this level check in the RQ
                                                                 RED algorithm.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
		u64 lpb_aura_pass:8;    /**< [271:264] [LPB_AURA]'s average aura level pass threshold for RED.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
		u64 lpb_pool_drop:8;    /**< [279:272] [LPB_AURA]'s average pool level drop threshold for RED.

                                                                 Must be less than or equal to [LPB_POOL_PASS]. Software can set
                                                                 [LPB_POOL_DROP] = [LPB_POOL_PASS] = 0 to disable this level check in the RQ
                                                                 RED algorithm.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
		u64 lpb_pool_pass:8;    /**< [287:280] [LPB_AURA]'s average pool level pass threshold for RED.

                                                                 See RQ RED algorithm pseudocode in [XQE_DROP]. */
		u64 reserved_288_291:4;
		u64 rq_int:8;	     /**< [299:292] RQ interrupts. Bits enumerated by NIX_RQINT_E. */
		u64 rq_int_ena:8;	     /**< [307:300] RQ interrupt enables. Bits enumerated by NIX_RQINT_E. */
		u64 qint_idx:7;	     /**< [314:308] Queue interrupt index. Select the QINT within LF (index {a} of
                                                                 NIX_LF_QINT()*) which receives [RQ_INT] events.

                                                                 Internal:
                                                                 QINT update message is generated on an interrupt update event or when [ENA]
                                                                 changes. Message op code (INCR/DECR/NOP) is based on current and next
                                                                 interrupt states:
                                                                 _ [ENA] && |([RQ_INT] & [RQ_INT_ENA]) */
		u64 reserved_315_319:5;
		u64 ltag:24;	     /**< [343:320] Lower WQE/CQE tag bits for a non-IPSEC receive packet, conditionally
                                                                 selected by [FLOW_TAGW].

                                                                 Pseudocode:
                                                                 \<pre\>
                                                                 rq_tag\<31:24\> = (NIX_RX_PARSE_S[ERRLEV] == 0 && NIX_RX_PARSE_S[ERRCODE] == 0)
                                                                      ? [GOOD_UTAG] : [BAD_UTAG];
                                                                 rq_tag\<23:0\> = [LTAG];
                                                                 flow_tag_mask\<31:0\> = (1 \<\< [FLOW_TAGW]) - 1;
                                                                 xqe_type = [SSO_ENA] ? NIX_WQE_HDR_S[WQE_TYPE] ? NIX_CQE_HDR_S[CQE_TYPE];

                                                                 if ((xqe_type == NIX_XQE_TYPE_E::RX)
                                                                     || (xqe_type == NIX_XQE_TYPE_E::RX_IPSECS))
                                                                 {
                                                                    // flow_tag\<31:0\> computation is defined in NIX_LF_RX_SECRET()
                                                                    tag\<31:0\> = (~flow_tag_mask & rq_tag) | (flow_tag_mask & flow_tag);
                                                                    if ([SSO_ENA]) NIX_WQE_HDR_S[TAG] = tag;
                                                                    else           NIX_CQE_HDR_S[TAG] = tag;
                                                                 }
                                                                 else { // NIX_XQE_TYPE_E::RX_IPSECH or IPSECD, only valid when [SSO_ENA]==1
                                                                    // SA_index computation is defined in NIX_AF_LF()_RX_IPSEC_CFG1[SA_IDX_W]
                                                                    NIX_WQE_HDR_S[TAG] = SA_index | (NIX_AF_LF()_RX_IPSEC_CFG1[TAG_CONST] \<\< 8);
                                                                 }
                                                                 \</pre\> */
		u64 good_utag:8;	     /**< [351:344] Upper WQE/CQE tag bits for a non-IPSEC packet received without error,
                                                                 conditionally selected by [FLOW_TAGW].
                                                                 See pseudocode in [LTAG]. */
		u64 bad_utag:8;	     /**< [359:352] Upper WQE/CQE tag bits for a non-IPSEC packet received with error,
                                                                 conditionally selected by [FLOW_TAGW].
                                                                 See pseudocode in [LTAG]. */
		u64 flow_tagw:6;	     /**< [365:360] Flow tag width. Number of lower bits of WQE/CQE tag taken from packet's
                                                                 flow_tag (see NIX_LF_RX_SECRET()). When greater than or equal to 32, the
                                                                 WQE/CQE tag equals flow_tag.
                                                                 See pseudocode in [LTAG]. */
		u64 reserved_366_383:18;
		u64 octs:48;	     /**< [431:384] Number of nondropped octets received (good and bad). Includes any
                                                                 timestamps, RX headers, Vtag bytes stripped by
                                                                 NIX_AF_LF()_RX_VTAG_TYPE()[STRIP], and frame minimum size pad bytes.
                                                                 Excludes FCS stripped by CGX. */
		u64 reserved_432_447:16;
		u64 pkts:48;	     /**< [495:448] Number of nondropped packets received (good and bad). */
		u64 reserved_496_511:16;
		u64 drop_octs:48;	     /**< [559:512] Number of octets dropped. See also [OCTS]. */
		u64 reserved_560_575:16;
		u64 drop_pkts:48;	     /**< [623:576] Number of packets dropped. */
		u64 reserved_624_639:16;
		u64 re_pkts:48;	     /**< [687:640] Number of nondropped packets with receive errors (NIX_RX_PARSE_S[ERRLEV] =
                                                                 NPC_ERRLEV_E::RE). */
		u64 reserved_688_703:16;
		u64 reserved_704_767:64;
		u64 reserved_768_831:64;
		u64 reserved_832_895:64;
		u64 reserved_896_959:64;
		u64 reserved_960_1023:64;
	} s;
	/* struct cavm_nix_rq_ctx_s_s cn; */
};

/**
 * Structure nix_rsse_hw_s
 *
 * INTERNAL: NIX Receive Side Scaling Entry Structure
 */
union cavm_nix_rsse_hw_s {
	uint32_t u;
	struct cavm_nix_rsse_hw_s_s {
		uint32_t rq:20;		     /**< [ 19:  0] See NIX_RSSE_S[RQ]. */
		uint32_t reserved_20_31:12;
	} s;
	/* struct cavm_nix_rsse_hw_s_s cn; */
};

/**
 * Structure nix_rsse_s
 *
 * NIX Receive Side Scaling Entry Structure
 * This structure specifies the format of each hardware entry in the NIX RSS
 * tables in NDC/LLC/DRAM. See NIX_AF_LF()_RSS_BASE and NIX_AF_LF()_RSS_GRP().
 * Software uses the same structure format to read and write an RSS table entry
 * with the NIX admin queue.
 */
union cavm_nix_rsse_s {
	uint32_t u;
	struct cavm_nix_rsse_s_s {
		uint32_t rq:20;		     /**< [ 19:  0] Receive queue index within LF to which the packet is directed. */
		uint32_t reserved_20_31:12;
	} s;
	/* struct cavm_nix_rsse_s_s cn; */
};

/**
 * Structure nix_rx_action_s
 *
 * NIX Receive Action Structure
 * This structure defines the format of NPC_RESULT_S[ACTION] for a receive packet.
 */
union cavm_nix_rx_action_s {
	u64 u;
	struct cavm_nix_rx_action_s_s {
		u64 op:4;		     /**< [  3:  0] Action op code enumerated by NIX_RX_ACTIONOP_E. */
		u64 pf_func:16;	     /**< [ 19:  4] PF and function to which the packet is directed if unicast.
                                                                 Format specified by RVU_PF_FUNC_S. Not meaningful when
                                                                 [OP] = NIX_RX_ACTIONOP_E::MCAST or NIX_RX_ACTIONOP_E::MIRROR. */
		u64 index:20;	     /**< [ 39: 20] Receive queue or table index in NIX. The index type
                                                                 is selected as follows:
                                                                 _ if [OP] = NIX_RX_ACTIONOP_E::UCAST or NIX_RX_ACTIONOP_E::UCAST_IPSEC, RQ
                                                                 index within [PF_FUNC].
                                                                 _ if [OP] = NIX_RX_ACTIONOP_E::RSS, RSS group index within [PF_FUNC].
                                                                 _ if [OP] = NIX_RX_ACTIONOP_E::MCAST, index of first NIX_RX_MCE_S of the
                                                                 multicast replication list in the NIX RX multicast/mirror table.
                                                                 _ if [OP] = NIX_RX_ACTIONOP_E::MIRROR, index of first NIX_RX_MCE_S of the
                                                                 mirror replication list in the NIX RX multicast/mirror table.
                                                                 _ otherwise, not used. */
		u64 match_id:16;	     /**< [ 55: 40] Software defined match identifier. */
		u64 flow_key_alg:5;     /**< [ 60: 56] Flow key algorithm. Index {a} (ALG) of NIX_AF_RX_FLOW_KEY_ALG()_FIELD(). */
		u64 reserved_61_63:3;
	} s;
	/* struct cavm_nix_rx_action_s_s cn; */
};

/**
 * Structure nix_rx_imm_s
 *
 * NIX Receive Immediate Subdescriptor Structure
 * The receive immediate subdescriptor indicates that bytes immediately following this
 * NIX_RX_IMM_S (after skipping [APAD] bytes) were saved from the received packet. The
 * next subdescriptor following this NIX_RX_IMM_S (when one exists) will follow the
 * immediate bytes, after rounding up the address to a multiple of 16 bytes.
 */
union cavm_nix_rx_imm_s {
	u64 u;
	struct cavm_nix_rx_imm_s_s {
		u64 size:16;	     /**< [ 15:  0] Size of immediate data (in bytes) that immediately follows this 64-bit
                                                                 structure. [SIZE] will be between 1 and 256 bytes. The next subdescriptor
                                                                 follows [APAD]+[SIZE] bytes later in the descriptor, rounded up to the next
                                                                 16-byte aligned address. */
		u64 apad:3;	     /**< [ 18: 16] Alignment pad. Number of bytes to skip following this 64-bit structure before
                                                                 the first byte of packet data. See pseudocode in NIX_AF_LF()_RX_CFG[DIS_APAD]. */
		u64 reserved_19_59:41;
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates immediate. Enumerated by NIX_SUBDC_E::IMM. */
	} s;
	/* struct cavm_nix_rx_imm_s_s cn; */
};

/**
 * Structure nix_rx_mce_hw_s
 *
 * INTERNAL: NIX Receive Multicast/Mirror Entry Structure
 */
union cavm_nix_rx_mce_hw_s {
	u64 u;
	struct cavm_nix_rx_mce_hw_s_s {
		u64 op:2;		     /**< [  1:  0] See NIX_RX_MCE_S[OP]. */
		u64 reserved_2:1;
		u64 eol:1;		     /**< [  3:  3] See NIX_RX_MCE_S[EOL]. */
		u64 index:20;	     /**< [ 23:  4] See NIX_RX_MCE_S[INDEX]. */
		u64 reserved_24_31:8;
		u64 pf_func:16;	     /**< [ 47: 32] See NIX_RX_MCE_S[PF_FUNC]. */
		u64 next:16;	     /**< [ 63: 48] See NIX_RX_MCE_S[NEXT]. */
	} s;
	/* struct cavm_nix_rx_mce_hw_s_s cn; */
};

/**
 * Structure nix_rx_mce_s
 *
 * NIX Receive Multicast/Mirror Entry Structure
 * This structure specifies the format of entries in the NIX receive
 * multicast/mirror table maintained by hardware in NDC/LLC/DRAM. See
 * NIX_AF_RX_MCAST_BASE and NIX_AF_RX_MCAST_CFG. Note the table may contain both
 * multicast and mirror replication lists.
 * Software uses the same structure format to read and write a multicast/mirror
 * table entry with the NIX admin queue.
 */
union cavm_nix_rx_mce_s {
	u64 u;
	struct cavm_nix_rx_mce_s_s {
		u64 op:2;		     /**< [  1:  0] Destination type within [PF_FUNC]; enumerated by NIX_RX_MCOP_E. */
		u64 reserved_2:1;
		u64 eol:1;		     /**< [  3:  3] End of multicast/mirror replication list. */
		u64 index:20;	     /**< [ 23:  4] Receive queue or RSS index within [PF_FUNC] to which the packet is directed. The
                                                                 index type is selected as follows:
                                                                 _ if [OP] = NIX_RX_MCOP_E::RQ, RQ index within [PF_FUNC].
                                                                 _ if [OP] = NIX_RX_MCOP_E::RSS, RSS group index within [PF_FUNC]. */
		u64 reserved_24_31:8;
		u64 pf_func:16;	     /**< [ 47: 32] PF and function to which the packet is directed. Format specified by RVU_PF_FUNC_S. */
		u64 next:16;	     /**< [ 63: 48] Index of next NIX_RX_MCE_S structure in the multicast/mirror replication
                                                                 list. Valid when [EOL] is clear. */
	} s;
	/* struct cavm_nix_rx_mce_s_s cn; */
};

/**
 * Structure nix_rx_parse_s
 *
 * NIX Receive Parse Structure
 * This structure contains the receive packet parse result. It immediately follows
 * NIX_CQE_HDR_S in a receive CQE, or NIX_WQE_HDR_S in a receive WQE.
 * Stored in memory as little-endian unless NIX_AF_LF()_CFG[BE] is set.
 *
 * Header layers are always 2-byte aligned, so all header pointers in this
 * structure ([EOH_PTR], [LAPTR] through [LHPTR], [VTAG*_PTR]) are even.
 */
union cavm_nix_rx_parse_s {
	u64 u[7];
	struct cavm_nix_rx_parse_s_s {
		u64 chan:12;	     /**< [ 11:  0] The logical channel that the packet arrived from, enumerated by NIX_CHAN_E. */
		u64 desc_sizem1:5;	     /**< [ 16: 12] Number of 128-bit words minus one in receive descriptor following NIX_RX_PARSE_S,
                                                                 i.e. size (minus one) of all NIX_RX_IMM_S, NIX_RX_SG_S and associated immediate
                                                                 data and IOVAs in the descriptor. */
		u64 imm_copy:1;	     /**< [ 17: 17] The immediate data following NIX_RX_IMM_S is a copy of data appearing the
                                                                 the segment buffers due to NIX_RQ_CTX_S[XQE_IMM_COPY] being set. */
		u64 express:1;	     /**< [ 18: 18] Express packet.
                                                                 0 = Normal (potentially preemptable) packet.
                                                                 1 = Express packet. */
		u64 wqwd:1;	     /**< [ 19: 19] WQE with data. Valid when NIX_RX_PARSE_S is included in a WQE, always clear in
                                                                 a CQE. Value from NIX_RQ_CTX_S[ENA_WQWD]. When set, indicates that the
                                                                 packet data starts in the same buffer as the WQE, i.e. the first NIX_IOVA_S
                                                                 of the first NIX_RX_SG_S in the receive descriptor points to an address
                                                                 within the WQE's buffer. */
		u64 errlev:4;	     /**< [ 23: 20] Normally zero, but when errors are detected contains the lowest protocol layer
                                                                 containing an error, and [ERRCODE] will indicate the precise error
                                                                 reason. Enumerated by NPC_ERRLEV_E. */
		u64 errcode:8;	     /**< [ 31: 24] When zero, indicates no error. When nonzero, contains opcode identifying
                                                                 the error reason, [ERRLEV] specifies the lowest protocol layer containing
                                                                 the eror, and software should ignore all parse information for layers
                                                                 higher than [ERRLEV], e.g. ignore [LF*], [LG*] and [LH*] when [ERRCODE] is
                                                                 nonzero and [ERRLEV]=NPC_ERRLEV_E::LE.

                                                                 Values defined as follows:
                                                                 * When [ERRLEV] = NPC_ERRLEV_E::RE, [ERRCODE] values are enumerated by
                                                                 NIX_RE_OPCODE_E.
                                                                 * When [ERRLEV] = NPC_ERRLEV_E::NIX, [ERRCODE] values are enumerated by
                                                                 NIX_RX_PERRCODE_E.
                                                                 * For all other [ERRLEV] values, [ERRCODE] values are software defined in
                                                                 NPC. */
		u64 latype:4;	     /**< [ 35: 32] Software defined layer A type from NPC_RESULT_S[LA[LTYPE]]. */
		u64 lbtype:4;	     /**< [ 39: 36] Software defined layer B type from NPC_RESULT_S[LB[LTYPE]]. */
		u64 lctype:4;	     /**< [ 43: 40] Software defined layer C type from NPC_RESULT_S[LC[LTYPE]]. */
		u64 ldtype:4;	     /**< [ 47: 44] Software defined layer D type from NPC_RESULT_S[LD[LTYPE]]. */
		u64 letype:4;	     /**< [ 51: 48] Software defined layer E type from NPC_RESULT_S[LE[LTYPE]]. */
		u64 lftype:4;	     /**< [ 55: 52] Software defined layer F type from NPC_RESULT_S[LF[LTYPE]]. */
		u64 lgtype:4;	     /**< [ 59: 56] Software defined layer G type from NPC_RESULT_S[LG[LTYPE]]. */
		u64 lhtype:4;	     /**< [ 63: 60] Software defined layer H type from NPC_RESULT_S[LH[LTYPE]]. */
		u64 pkt_lenm1:16;	     /**< [ 79: 64] Packet length in bytes minus one. Vtag bytes stripped by
                                                                 NIX_AF_LF()_RX_VTAG_TYPE()[STRIP] are not included. */
		u64 l2m:1;		     /**< [ 80: 80] Outer L2 multicast. See NPC_RESULT_S[L2M]. */
		u64 l2b:1;		     /**< [ 81: 81] Outer L2 broadcast. See NPC_RESULT_S[L2B]. */
		u64 l3m:1;		     /**< [ 82: 82] Outer IP multicast. See NPC_RESULT_S[L3M]. */
		u64 l3b:1;		     /**< [ 83: 83] Outer IP broadcast. See NPC_RESULT_S[L3B]. */
		u64 vtag0_valid:1;	     /**< [ 84: 84] Vtag 0 valid. Set when NPC_RESULT_S[NIX_RX_VTAG_ACTION_S[VTAG0_VALID]] and
                                                                 corresponding NIX_AF_LF()_RX_VTAG_TYPE()[CAPTURE] are set. */
		u64 vtag0_gone:1;	     /**< [ 85: 85] Vtag 0 gone. Valid when [VTAG0_VALID] is set. Indicates Vtag 0 was stripped from
                                                                 the packet data. Set when corresponding
                                                                 NIX_AF_LF()_RX_VTAG_TYPE()[CAPTURE,STRIP] are set. */
		u64 vtag1_valid:1;	     /**< [ 86: 86] Vtag 1 valid. See [VTAG0_VALID]. */
		u64 vtag1_gone:1;	     /**< [ 87: 87] Vtag 1 gone. See [VTAG0_GONE]. */
		u64 pkind:6;	     /**< [ 93: 88] Port kind supplied by CGX or LBK for received packet. */
		u64 reserved_94_95:2;
		u64 vtag0_tci:16;	     /**< [111: 96] Vtag 0 tag control information. First two bytes of Vtag's TCI field from the
                                                                 packet header.
                                                                 Valid when [VTAG0_VALID] is set. */
		u64 vtag1_tci:16;	     /**< [127:112] Vtag 1 tag control information. See [VTAG0_TCI]. */
		u64 laflags:8;	     /**< [135:128] Software defined layer A flags from NPC_RESULT_S[LA[FLAGS]]. */
		u64 lbflags:8;	     /**< [143:136] Software defined layer B flags from NPC_RESULT_S[LB[FLAGS]]. */
		u64 lcflags:8;	     /**< [151:144] Software defined layer C flags from NPC_RESULT_S[LC[FLAGS]]. */
		u64 ldflags:8;	     /**< [159:152] Software defined layer D flags from NPC_RESULT_S[LD[FLAGS]]. */
		u64 leflags:8;	     /**< [167:160] Software defined layer E flags from NPC_RESULT_S[LE[FLAGS]]. */
		u64 lfflags:8;	     /**< [175:168] Software defined layer F flags from NPC_RESULT_S[LF[FLAGS]]. */
		u64 lgflags:8;	     /**< [183:176] Software defined layer G flags from NPC_RESULT_S[LG[FLAGS]]. */
		u64 lhflags:8;	     /**< [191:184] Software defined layer H flags from NPC_RESULT_S[LH[FLAGS]]. */
		u64 eoh_ptr:8;	     /**< [199:192] End of header pointer. Byte offset from packet start to first byte after
                                                                 the last parsed header layer. */
		u64 wqe_aura:20;	     /**< [219:200] WQE aura. Valid when NIX_RX_PARSE_S is included in a WQE. Not valid when
                                                                 included in a CQE. Aura within NIX_AF_LF()_CFG[NPA_PF_FUNC] from which this
                                                                 WQE buffer was allocated. */
		u64 pb_aura:20;	     /**< [239:220] Packet buffer aura. Valid when the receive descriptor contains at least one
                                                                 NIX_RX_SG_S. Aura within NIX_AF_LF()_CFG[NPA_PF_FUNC] from which the
                                                                 packet buffer pointers in NIX_RX_SG_S are allocated. */
		u64 match_id:16;	     /**< [255:240] Software defined match identifier from NIX_RX_ACTION_S[MATCH_ID]. */
		u64 laptr:8;	     /**< [263:256] Layer A pointer. Byte offset from packet start to first byte of layer A. */
		u64 lbptr:8;	     /**< [271:264] Layer B pointer. Byte offset from packet start to first byte of layer B. */
		u64 lcptr:8;	     /**< [279:272] Layer C pointer. Byte offset from packet start to first byte of layer C. */
		u64 ldptr:8;	     /**< [287:280] Layer D pointer. Byte offset from packet start to first byte of layer D. */
		u64 leptr:8;	     /**< [295:288] Layer E pointer. Byte offset from packet start to first byte of layer E. */
		u64 lfptr:8;	     /**< [303:296] Layer F pointer. Byte offset from packet start to first byte of layer F. */
		u64 lgptr:8;	     /**< [311:304] Layer G pointer. Byte offset from packet start to first byte of layer G. */
		u64 lhptr:8;	     /**< [319:312] Layer H pointer. Byte offset from packet start to first byte of layer H. */
		u64 vtag0_ptr:8;	     /**< [327:320] Vtag 0 pointer. Byte offset from packet start to first byte of Vtag 0.
                                                                 Valid when [VTAG0_VALID] is set. */
		u64 vtag1_ptr:8;	     /**< [335:328] Vtag 1 pointer. See [VTAG0_PTR]. */
		u64 flow_key_alg:5;     /**< [340:336] Flow key algorithm used to generate NIX_CQE_HDR_S[TAG]. Index {a}
                                                                 (ALG) of NIX_AF_RX_FLOW_KEY_ALG()_FIELD(). */
		u64 reserved_341_383:43;
		u64 reserved_384_447:64;
	} s;
	/* struct cavm_nix_rx_parse_s_s cn; */
};

/**
 * Structure nix_rx_sg_s
 *
 * NIX Receive Scatter/Gather Subdescriptor Structure
 * The receive scatter/gather subdescriptor specifies one to three segments of packet data bytes.
 * There may be multiple NIX_RX_SG_Ss in each NIX receive descriptor.
 *
 * NIX_RX_SG_S is immediately followed by one NIX_IOVA_S word when [SEGS] = 1,
 * three NIX_IOVA_S words when [SEGS] \>= 2. Each NIX_IOVA_S word specifies the
 * LF IOVA of first packet data byte in the corresponding segment; first NIX_IOVA_S
 * word for segment 1, second word for segment 2, third word for segment 3. Note
 * the third word is present when [SEGS] \>= 2 but only valid when [SEGS] = 3.
 */
union cavm_nix_rx_sg_s {
	u64 u;
	struct cavm_nix_rx_sg_s_s {
		u64 seg1_size:16;	     /**< [ 15:  0] Size of segment 1 in bytes. */
		u64 seg2_size:16;	     /**< [ 31: 16] Size of segment 2 in bytes. Valid when [SEGS] \>= 2. */
		u64 seg3_size:16;	     /**< [ 47: 32] Size of segment 3 in bytes. Valid when [SEGS] = 3. */
		u64 segs:2;	     /**< [ 49: 48] Number of valid segments. Must be nonzero. */
		u64 reserved_50_59:10;
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates scatter/gather. Enumerated by NIX_SUBDC_E::SG. */
	} s;
	/* struct cavm_nix_rx_sg_s_s cn; */
};

/**
 * Structure nix_rx_vtag_action_s
 *
 * NIX Receive Vtag Action Structure
 * This structure defines the format of NPC_RESULT_S[VTAG_ACTION] for a receive packet.
 * It specifies up to two Vtags (e.g. C-VLAN/S-VLAN tags, 802.1BR E-TAG) for optional
 * capture and/or stripping.
 */
union cavm_nix_rx_vtag_action_s {
	u64 u;
	struct cavm_nix_rx_vtag_action_s_s {
		u64 vtag0_relptr:8;     /**< [  7:  0] Vtag 0 relative pointer. Byte offset from start of selected layer to first
                                                                 tag 0 byte. Must be even. For example, if [VTAG0_LID] = NPC_LID_E::LB, then
                                                                 the byte offset from packet start to the first tag 0 byte is
                                                                 NPC_RESULT_S[LB[LPTR]] + [VTAG0_RELPTR]. */
		u64 vtag0_lid:3;	     /**< [ 10:  8] Vtag 0 layer ID enumerated by NPC_LID_E. */
		u64 reserved_11:1;
		u64 vtag0_type:3;	     /**< [ 14: 12] Vtag 0 type. Index to NIX_AF_LF()_RX_VTAG_TYPE() entry for the receive
                                                                 packet's VF/PF. The selected entry specifies the tag size and optional tag
                                                                 strip/capture actions.

                                                                 The VF/PF is specified by NIX_RX_ACTION_S[PF_FUNC] when
                                                                 NIX_RX_ACTION_S[OP] != NIX_RX_ACTIONOP_E::MCAST or
                                                                 NIX_RX_ACTIONOP_E::MIRROR, and by the NIX RX multicast/mirror replication
                                                                 list entries otherwise. */
		u64 vtag0_valid:1;	     /**< [ 15: 15] Vtag 0 valid. Remaining [VTAG0_*] fields are valid when set. */
		u64 reserved_16_31:16;
		u64 vtag1_relptr:8;     /**< [ 39: 32] Vtag 1 relative pointer. See [VTAG0_RELPTR]. */
		u64 vtag1_lid:3;	     /**< [ 42: 40] Vtag 1 layer ID enumerated by NPC_LID_E. */
		u64 reserved_43:1;
		u64 vtag1_type:3;	     /**< [ 46: 44] Vtag 1 type. See [VTAG0_TYPE]. */
		u64 vtag1_valid:1;	     /**< [ 47: 47] Vtag 1 valid. Remaining [VTAG1_*] fields are valid when set. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nix_rx_vtag_action_s_s cn; */
};

/**
 * Structure nix_send_comp_s
 *
 * NIX Send Completion Structure
 * This structure immediately follows NIX_CQE_HDR_S in a send completion CQE.
 */
union cavm_nix_send_comp_s {
	u64 u;
	struct cavm_nix_send_comp_s_s {
		u64 status:8;	     /**< [  7:  0] Send completion status enumerated by NIX_SEND_STATUS_E. */
		u64 sqe_id:16;	     /**< [ 23:  8] SQE identifier from NIX_SEND_HDR_S[SQE_ID]. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nix_send_comp_s_s cn; */
};

/**
 * Structure nix_send_crc_s
 *
 * NIX Send CRC Subdescriptor Structure
 * The send CRC subdescriptor specifies a CRC calculation be performed during
 * transmission. There may be up to two NIX_SEND_CRC_Ss per send descriptor.
 *
 * NIX_SEND_CRC_S constraints:
 * * When present, NIX_SEND_CRC_S subdescriptors must precede all NIX_SEND_SG_S,
 * NIX_SEND_IMM_S and NIX_SEND_MEM_S subdescriptors in the send descriptor.
 * * NIX_SEND_CRC_S subdescriptors must follow the same order as their checksum
 * and insert regions in the packet, i.e. the checksum and insert regions of a
 * NIX_SEND_CRC_S must come after the checksum and insert regions of a preceding
 * NIX_SEND_CRC_S. There must be no overlap between any NIX_SEND_CRC_S checksum
 * and insert regions.
 * * If either NIX_SEND_HDR_S[OL4TYPE,IL4TYPE] = NIX_SENDL4TYPE_E::SCTP_CKSUM, the
 * SCTP checksum region and NIX_SEND_CRC_S insert region must not overlap, and
 * likewise the NIX_SEND_CRC_S checksum region and SCTP insert region must not
 * overlap.
 * * If either NIX_SEND_HDR_S[OL3TYPE,IL3TYPE] = NIX_SENDL3TYPE_E::IP4_CKSUM, the
 * IPv4 header checksum region and NIX_SEND_CRC_S insert region must not overlap.
 * * Any checksums inserted by NIX_SEND_HDR_S[OL3TYPE,OL4TYPE,IL3TYPE,IL4TYPE]
 * must be outside of the NIX_SEND_CRC_S checksum and insert regions.
 *
 * Hardware adjusts [START], [SIZE] and [INSERT] as needed to account for any VLAN
 * inserted by NIX_SEND_EXT_S[VLAN*] or Vtag inserted by NIX_TX_VTAG_ACTION_S.
 */
union cavm_nix_send_crc_s {
	u64 u[2];
	struct cavm_nix_send_crc_s_s {
		u64 size:16;	     /**< [ 15:  0] Length of checksum region, must not be zero. The region is contiguous in packet bytes
                                                                 [START] through [START]+[SIZE]-1. Note that these covered packet bytes need not be
                                                                 contiguous in LLC/DRAM -- they can straddle any number of NIX_SEND_SG_S subdescriptors. */
		u64 start:16;	     /**< [ 31: 16] Byte position relative to the first packet byte at which to start the
                                                                 checksum. Must be even when [ALG] = NIX_SENDCRCALG_E::ONES16. */
		u64 insert:16;	     /**< [ 47: 32] Byte position relative to the first packet byte at which to insert the first byte of the
                                                                 calculated CRC. NIX does not allocate bytes as it inserts the CRC result into the packet,
                                                                 it overwrites four pre-supplied packet bytes using NIX_SEND_SG_S or NIX_SEND_IMM_S.
                                                                 The insertion point may not be within the start/size region of this NIX_SEND_CRC_S or
                                                                 another NIX_SEND_CRC_S. */
		u64 reserved_48_57:10;
		u64 alg:2;		     /**< [ 59: 58] CRC algorithm enumerated by NIX_SENDCRCALG_E. */
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates send CRC. Enumerated by NIX_SUBDC_E::CRC. */
		u64 iv:32;		     /**< [ 95: 64] Initial value of the checksum. If [ALG] = ONES16, then only bits \<15:0\> in
                                                                 big-endian format are valid. */
		u64 reserved_96_127:32;
	} s;
	/* struct cavm_nix_send_crc_s_s cn; */
};

/**
 * Structure nix_send_ext_s
 *
 * NIX Send Extended Header Subdescriptor Structure
 * The send extended header specifies LSO, VLAN insertion, timestamp and/or
 * scheduling services on the packet. If present, it must immediately follow
 * NIX_SEND_HDR_S. All fields are assumed to be zero when this subdescriptor is not
 * present.
 */
union cavm_nix_send_ext_s {
	u64 u[2];
	struct cavm_nix_send_ext_s_s {
		u64 lso_mps:14;	     /**< [ 13:  0] When [LSO] set, maximum payload size in bytes per packet (e.g. maximum
                                                                 TCP segment size). Must be nonzero, else the send descriptor is treated as
                                                                 non-LSO.

                                                                 The maximum LSO packet size is [LSO_SB] + [LSO_MPS], plus optional VLAN
                                                                 bytes inserted by [VLAN*] and Vtag bytes inserted by
                                                                 NIX_TX_VTAG_ACTION_S. This must not exceed NIX_AF_SMQ()_CFG[MAXLEN].

                                                                 The number of LSO segments is (NIX_SEND_HDR_S[TOTAL]-[LSO_SB])/[LSO_MPS]
                                                                 rounded up to the nearest integer, and should be less than or equal to 256.
                                                                 Otherwise, NIX will terminate the LSO send operation after 256 segments. */
		u64 lso:1;		     /**< [ 14: 14] Large send offload. Ignored and treated as clear when
                                                                 NIX_AF_LSO_CFG[ENABLE] is clear. When set along with
                                                                 NIX_AF_LSO_CFG[ENABLE], the send descriptor is for one or more
                                                                 packets of a TCP flow, and the related [LSO_*] fields are valid. */
		u64 tstmp:1;	     /**< [ 15: 15] PTP timestamp. Ignored unless a NIX_SEND_MEM_S is present in the send
                                                                 descriptor with NIX_SEND_MEM_S[ALG] = NIX_SENDMEMALG_E::SETTSTMP. When set,
                                                                 hardware writes the packet's timestamp (MIO_PTP_CLOCK_HI) to LF IOVA
                                                                 NIX_SEND_MEM_S[ADDR] when the targeted CGX LMAC transmits the packet. See
                                                                 also NIX_SENDMEMALG_E::SETTSTMP.

                                                                 If NIX_SQ_CTX_S[CQ_ENA] and software wishes to receive a CQE on timestamp
                                                                 completion, it must set NIX_SEND_HDR_S[PNC] = 1 and NIX_SEND_MEM_S[WMEM] =
                                                                 1.

                                                                 If NIX_SQ_CTX_S[SSO_ENA] and software wishes to add work to SSO on
                                                                 timestamp completion, it must set NIX_SEND_MEM_S[WMEM] = 1 and include
                                                                 NIX_SEND_WORK_S in the descriptor. */
		u64 lso_sb:8;	     /**< [ 23: 16] Start bytes when [LSO] set. Location of the start byte of the TCP message
                                                                 payload, i.e. the size of the headers preceding the payload, excluding
                                                                 optional VLAN bytes inserted by [VLAN*] and Vtag bytes
                                                                 inserted by NIX_TX_VTAG_ACTION_S.

                                                                 Must be nonzero and less than NIX_SEND_HDR_S[TOTAL], else the send
                                                                 descriptor is treated as non-LSO. */
		u64 lso_format:5;	     /**< [ 28: 24] Large send offload format. Valid when [LSO] is set and selects index {a}
                                                                 (FORMAT) of NIX_AF_LSO_FORMAT()_FIELD(). */
		u64 reserved_29_31:3;
		u64 shp_chg:9;	     /**< [ 40: 32] Two's complement signed packet size adjustment. The packet size used for
                                                                 shaper {a} (PIR_ACCUM and CIR_ACCUM) and DWRR scheduler {a} (RR_COUNT)
                                                                 calculations at level {b} is:

                                                                 _  (NIX_AF_{b}{a}_SHAPE[LENGTH_DISABLE] ? 0 : (NIX_AF_{b}{a}_MD*[LENGTH] + [SHP_CHG]))
                                                                        + NIX_AF_{b}{a}_SHAPE[ADJUST]

                                                                 where {b} = TL1, TL2, TL3, TL4, or MDQ and {a} selects one of the shapers
                                                                 at the level selected by {b}.

                                                                 [SHP_CHG] values -255 .. 255 are allowed. [SHP_CHG] value 0x100 (i.e. -256)
                                                                 is reserved and must never be used.

                                                                 [SHP_CHG] becomes NIX_AF_{b}m_MD*[ADJUST].

                                                                 When [LSO] is set, hardware applies [SHP_CHG] to each LSO segment. */
		u64 shp_dis:1;	     /**< [ 41: 41] Disables the shaper update and internal coloring algorithms used as
                                                                 the packet traverses MDQ through TL2 shapers. [SHP_DIS]
                                                                 has no effect on the L1 rate limiters.

                                                                 When [SHP_DIS] is 0 enabled CIR and PIR counters are used and adjusted
                                                                 per mode as the packet traverses through an enabled shaper. The
                                                                 internal color of a packet can be any of NIX_COLORRESULT_E::GREEN,
                                                                 NIX_COLORRESULT_E::YELLOW, NIX_COLORRESULT_E::RED_SEND, or
                                                                 NIX_COLORRESULT_E::RED_DROP after a shaper, depending on the shaper state
                                                                 and configuration.

                                                                 When [SHP_DIS] is 1 there is no packet coloring. No shaper can change
                                                                 the packet from its initial GREEN color. Neither the CIR nor PIR
                                                                 counters are used nor adjusted in any shaper as this packet traverses.
                                                                 Similar behavior to when both NIX_AF_TL*()_CIR[ENABLE] and
                                                                 NIX_AF_TL*()_PIR[ENABLE] are clear in all traversed shapers.

                                                                 See NIX_AF_TL*()_MD_DEBUG*[PIR_DIS,CIR_DIS]. When [SHP_DIS] is set,
                                                                 NIX_AF_TL*()_MD_DEBUG*[PIR_DIS,CIR_DIS] are both set. When [SHP_DIS] is
                                                                 clear, NIX_AF_TL*()_MD_DEBUG*[PIR_DIS,CIR_DIS] are both cleared and
                                                                 NIX_AF_TL*()_SHAPE[YELLOW_DISABLE,RED_DISABLE] determines the packet
                                                                 coloring of the shaper.

                                                                 When [LSO] is set, hardware applies [SHP_DIS] to each LSO segment. */
		u64 shp_ra:2;	     /**< [ 43: 42] Red algorithm. Enumerated by NIX_REDALG_E. Specifies handling of a packet that
                                                                 traverses a RED MDQ through TL2 shaper. (A shaper is in RED state when
                                                                 NIX_AF_TL*()_SHAPE_STATE[COLOR]=0x2.) Has no effect when the packet traverses no
                                                                 shapers that are in the RED state. When [SHP_RA]!=STD, [SHP_RA] overrides the
                                                                 NIX_AF_TL*()_SHAPE[RED_ALGO] settings in all MDQ through TL2 shapers traversed
                                                                 by the packet. [SHP_RA] has no effect on the TL1 rate limiters. See
                                                                 NIX_AF_TL*()_MD_DEBUG*[RED_ALGO_OVERRIDE].

                                                                 When [LSO] is set in the descriptor, hardware applies [SHP_RA] to each LSO
                                                                 segment. */
		u64 markptr:8;	     /**< [ 51: 44] Mark pointer. When [MARK_EN] is set, byte offset from packet start to byte
                                                                 to use for packet shaper marking. [MARKFORM] indirectly determines how this
                                                                 offset is used, including whether and how an L2 or L3 header is marked. See
                                                                 also [MARK_EN]. */
		u64 markform:7;	     /**< [ 58: 52] Mark Format. When [MARK_EN] is set, the NIX_AF_MARK_FORMAT()_CTL register
                                                                 which specifies how NIX will mark NIX_COLORRESULT_E::YELLOW and
                                                                 NIX_COLORRESULT_E::RED_SEND packets. [MARKFORM] must be less than the size
                                                                 of the NIX_AF_MARK_FORMAT()_CTL array. See also [MARK_EN]. */
		u64 mark_en:1;	     /**< [ 59: 59] Enable for packet shaper marking. When one, NIX_COLORRESULT_E::YELLOW and
                                                                 NIX_COLORRESULT_E::RED_SEND packets will be marked as specified by
                                                                 [MARKFORM] and [MARKPTR].

                                                                 When [LSO] and [MARK_EN] are both set in the descriptor, NIX marks each LSO
                                                                 segment independently, using [MARKPTR] and [MARKFORM] for every LSO
                                                                 segment. */
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates send extended header. Enumerated by NIX_SUBDC_E::EXT. */
		u64 vlan0_ins_ptr:8;    /**< [ 71: 64] VLAN 0 insert pointer. Byte offset from packet start to first inserted VLAN byte when
                                                                 [VLAN0_INS_ENA] is set. Must be even. */
		u64 vlan0_ins_tci:16;   /**< [ 87: 72] VLAN 0 insert tag control information. See [VLAN0_INS_ENA]. */
		u64 vlan1_ins_ptr:8;    /**< [ 95: 88] VLAN 1 insert pointer. Byte offset from packet start to first inserted VLAN byte when
                                                                 [VLAN1_INS_ENA] is set. Must be even. */
		u64 vlan1_ins_tci:16;   /**< [111: 96] VLAN 1 insert tag control information. See [VLAN1_INS_ENA]. */
		u64 vlan0_ins_ena:1;    /**< [112:112] VLAN 0 insert enable. If set, NIX inserts a VLAN tag at byte offset
                                                                 [VLAN0_INS_PTR] from the start of packet. The inserted VLAN tag consists of:
                                                                 * 16-bit Ethertype given by NIX_AF_LF()_TX_CFG[VLAN0_INS_ETYPE], followed by
                                                                 * 16-bit tag control information given by [VLAN0_INS_TCI].

                                                                 Up to two VLAN tags may be inserted in a packet due to [VLAN0_INS_ENA] and
                                                                 [VLAN1_INS_ENA]. If two VLAN tags are inserted:
                                                                 * [VLAN0_INS_PTR] must be less than or equal to [VLAN1_INS_PTR].
                                                                 * Hardware inserts VLAN 0 first and adjusts [VLAN1_INS_PTR] accordingly.
                                                                 Thus, if the two pointers are equal in the descriptor, hardware inserts
                                                                 VLAN 1 immediately after VLAN 0 in the packet.

                                                                 A VLAN must not be inserted within an outer or inner L3/L4 header, but may be
                                                                 inserted within an outer L4 payload.

                                                                 The packet header is parsed by NPC after VLAN insertion. Note that the
                                                                 resulting NIX_TX_VTAG_ACTION_S[VTAG0_OP,VTAG1_OP] may replace or insert
                                                                 additional header bytes. Thus, Vtag may replace bytes that were inserted by
                                                                 [VLAN0_INS_*,VLAN1_INS_*]. */
		u64 vlan1_ins_ena:1;    /**< [113:113] VLAN 1 insert enable. See [VLAN0_INS_ENA]. */
		u64 reserved_114_127:14;
	} s;
	/* struct cavm_nix_send_ext_s_s cn; */
};

/**
 * Structure nix_send_hdr_s
 *
 * NIX Send Header Subdescriptor Structure
 * The send header is the first subdescriptor of every send descriptor.
 */
union cavm_nix_send_hdr_s {
	u64 u[2];
	struct cavm_nix_send_hdr_s_s {
		u64 total:18;	     /**< [ 17:  0] Total byte count to send, excluding optional VLAN bytes inserted by
                                                                 NIX_SEND_EXT_S[VLAN*] and Vtag bytes inserted by NIX_TX_VTAG_ACTION_S.

                                                                 For a non-LSO descriptor, total number of bytes, including any inserted
                                                                 VLAN and/or Vtag bytes, must not exceed NIX_AF_SMQ()_CFG[MAXLEN].

                                                                 For a LSO send descriptor (NIX_SEND_EXT_S[LSO] is present and set),
                                                                 specifies the total LSO payload size plus the size of the LSO header,
                                                                 excluding any inserted VLAN and/or Vtag bytes. In other words, the total
                                                                 LSO data payload size is [TOTAL] - NIX_SEND_EXT_S[LSO_SB].

                                                                 [TOTAL] does not include any of the outside FCS bytes that CGX may append
                                                                 to the packet(s). Hardware zero pads the packet when [TOTAL] is larger than
                                                                 the sum of all NIX_SEND_SG_S[SEG_SIZE*]s and NIX_SEND_IMM_S[SIZE]s in the
                                                                 descriptor. In addition, hardware zero pads the packet when
                                                                 NIX_AF_SMQ()_CFG[MINLEN] is larger than the sum of [TOTAL] and any inserted
                                                                 VLAN and Vtag bytes. */
		u64 reserved_18:1;
		u64 df:1;		     /**< [ 19: 19] Don't free. If set, by default NIX will not free the surrounding buffer of
                                                                 a packet segment from NIX_SEND_SG_S. If clear, by default NIX will free the
                                                                 buffer. See NIX_SEND_SG_S[I]. */
		u64 aura:20;	     /**< [ 39: 20] Aura number. NPA aura within NIX_AF_LF()_CFG[NPA_PF_FUNC] to which buffers are
                                                                 optionally freed for packet segments from NIX_SEND_SG_S. See [DF] and
                                                                 NIX_SEND_SG_S[I]. */
		u64 sizem1:3;	     /**< [ 42: 40] Number of 128-bit words in the SQE minus one. */
		u64 pnc:1;		     /**< [ 43: 43] Post normal completion. If set along with NIX_SQ_CTX_S[CQ_ENA], a CQE is
                                                                 created with NIX_CQE_HDR_S[CQE_TYPE] = NIX_XQE_TYPE_E::SEND when the send
                                                                 descriptor's operation completes. If NIX_SEND_EXT_S[LSO] is set, a CQE is
                                                                 created when the send operation completes for the last LSO segment.

                                                                 If clear, no CQE is added on send completion.

                                                                 This bit is ignored when NIX_SQ_CTX_S[CQ_ENA] is clear.

                                                                 NIX will not add a CQE for this send descriptor until after it has
                                                                 completed all LLC/DRAM fetches that service all prior NIX_SEND_SG_S
                                                                 subdescriptors, and it has fetched all subdescriptors in the send
                                                                 descriptor. If NIX_SEND_MEM_S[WMEM]=1, NIX also will not post the CQE until
                                                                 all NIX_SEND_MEM_S subdescriptors in the descriptor complete and commit. */
		u64 sq:20;		     /**< [ 63: 44] Send queue within LF. Valid in the first NIX_SEND_HDR_S of an LMT store to
                                                                 NIX_LF_OP_SEND(). If multiple SQEs are enqueued by the LMT store,
                                                                 ignored in all NIX_SEND_HDR_S other than the first one.

                                                                 Internal:
                                                                 Included in LMTST, removed by hardware. */
		u64 ol3ptr:8;	     /**< [ 71: 64] Outer Layer 3 pointer. Byte offset from packet start to first byte of outer
                                                                 L3 header, if present, before any VLAN or Vtag insertion. Must be even.
                                                                 Hardware adjusts the pointer as needed to account for any VLAN inserted by
                                                                 NIX_SEND_EXT_S[VLAN*] or Vtag inserted by NIX_TX_VTAG_ACTION_S at or before
                                                                 this byte offset.

                                                                 Must be valid when used by a selected checksum or LSO algorithm,
                                                                 specifically when any of the following is true:
                                                                 * [OL3TYPE] = NIX_SENDL3TYPE_E::IP4_CKSUM.
                                                                 * [OL4TYPE] = NIX_SENDL4TYPE_E::TCP_CKSUM or NIX_SENDL4TYPE_E::UDP_CKSUM
                                                                 (due to IPv4/IPv6 pseudo-header included in the TCP/UDP checksum).
                                                                 * NIX_SEND_EXT_S[LSO] is present and set in the descriptor and the LSO
                                                                 format selected by NIX_SEND_EXT_S[LSO_FORMAT] modifies at least one field
                                                                 in this layer
                                                                 (corresponding NIX_AF_LSO_FORMAT()_FIELD()[ALG] != NIX_LSOALG_E::NOP and
                                                                 NIX_AF_LSO_FORMAT()_FIELD()[LAYER] = NIX_TXLAYER_E::OL3). */
		u64 ol4ptr:8;	     /**< [ 79: 72] Outer Layer 4 pointer. Byte offset from packet start to first byte of
                                                                 outer L4 header, if present, before an VLAN or Vtag insertion. Must be even.
                                                                 Hardware adjusts the pointer as needed to account for any VLAN inserted by
                                                                 NIX_SEND_EXT_S[VLAN*] or Vtag inserted by NIX_TX_VTAG_ACTION_S at or before
                                                                 this byte offset.

                                                                 Must be valid when used by a selected checksum or LSO algorithm,
                                                                 specifically when any of the following is true:
                                                                 * [OL3TYPE] = NIX_SENDL3TYPE_E::IP4_CKSUM
                                                                 ([OL4PTR points to end of IPv4 header).
                                                                 * [OL4TYPE] = NIX_SENDL4TYPE_E::TCP_CKSUM or NIX_SENDL4TYPE_E::UDP_CKSUM
                                                                 * NIX_SEND_EXT_S[LSO] is present and set in the descriptor and the LSO
                                                                 format selected by NIX_SEND_EXT_S[LSO_FORMAT] modifies at least one field
                                                                 in this layer
                                                                 (corresponding NIX_AF_LSO_FORMAT()_FIELD()[ALG] != NIX_LSOALG_E::NOP and
                                                                 NIX_AF_LSO_FORMAT()_FIELD()[LAYER] = NIX_TXLAYER_E::OL4). */
		u64 il3ptr:8;	     /**< [ 87: 80] Inner Layer 3 pointer. Byte offset from packet start to first byte of inner
                                                                 L3 header, if present, before any VLAN or Vtag insertion. Must be even.
                                                                 Hardware adjusts the pointer as needed to account for any VLAN inserted by
                                                                 NIX_SEND_EXT_S[VLAN*] or Vtag inserted by NIX_TX_VTAG_ACTION_S at or before
                                                                 this byte offset.

                                                                 Must be valid when used by a selected checksum or LSO algorithm. See
                                                                 [OL3PTR] for equivalent outer L3 conditions. */
		u64 il4ptr:8;	     /**< [ 95: 88] Inner Layer 4 pointer. Byte offset from packet start to first byte of inner
                                                                 L4 header, if present, before any VLAN or Vtag insertion. Must be even.
                                                                 Hardware adjusts the pointer as needed to account for any VLAN inserted by
                                                                 NIX_SEND_EXT_S[VLAN*] or Vtag inserted by NIX_TX_VTAG_ACTION_S at or before
                                                                 this byte offset.

                                                                 Must be valid when used by a selected checksum or LSO algorithm. See
                                                                 [OL4PTR] for equivalent outer L4 conditions. */
		u64 ol3type:4;	     /**< [ 99: 96] Outer Layer 3 type enumerated by NIX_SENDL3TYPE_E. */
		u64 ol4type:4;	     /**< [103:100] Outer Layer 4 type enumerated by NIX_SENDL4TYPE_E. If checksum generation
                                                                 is specified, hardware includes all packet data following the outer layer 4
                                                                 header in the checksum calculation, excluding pad bytes added due to
                                                                 NIX_AF_SMQ()_CFG[MINLEN]. Hardware does not use IP or UDP length fields in
                                                                 the packet to determine the layer 4 checksum region.

                                                                 When [OL4TYPE] = NIX_SENDL4TYPE_E::SCTP_CKSUM, [IL3TYPE] and [IL4TYPE] must
                                                                 not specify checksum generation. */
		u64 il3type:4;	     /**< [107:104] Inner Layer 3 type enumerated by NIX_SENDL3TYPE_E. */
		u64 il4type:4;	     /**< [111:108] Inner Layer 4 type enumerated by NIX_SENDL4TYPE_E. If checksum generation
                                                                 is specified, hardware includes all packet data following the inner layer 4
                                                                 header in the checksum calculation, excluding pad bytes added due to
                                                                 NIX_AF_SMQ()_CFG[MINLEN]. Hardware does not use IP or UDP length fields in
                                                                 the packet to determine the layer 4 checksum region. */
		u64 sqe_id:16;	     /**< [127:112] Software defined SQE identifier copied to NIX_SEND_COMP_S[SQE_ID]. */
	} s;
	/* struct cavm_nix_send_hdr_s_s cn; */
};

/**
 * Structure nix_send_imm_s
 *
 * NIX Send Immediate Subdescriptor Structure
 * The send immediate subdescriptor requests that bytes immediately following this
 * NIX_SEND_IMM_S (after skipping [APAD] bytes) are to be included in the packet data.
 * The next subdescriptor following this NIX_SEND_IMM_S (when one exists) will
 * follow the immediate bytes, after rounding up the address to a multiple of 16 bytes.
 *
 * There may be multiple NIX_SEND_IMM_S in one NIX send descriptor. A
 * NIX_SEND_IMM_S is ignored in a NIX send descriptor if the sum of all prior
 * NIX_SEND_SG_S[SEG*_SIZE]s and NIX_SEND_IMM_S[SIZE]s meets or exceeds
 * NIX_SEND_HDR_S[TOTAL].
 *
 * When NIX_SEND_EXT_S[LSO] is set in the descriptor, all NIX_SEND_IMM_S
 * bytes must be included in the first NIX_SEND_EXT_S[LSO_SB] bytes of the
 * source packet.
 */
union cavm_nix_send_imm_s {
	u64 u;
	struct cavm_nix_send_imm_s_s {
		u64 size:16;	     /**< [ 15:  0] Size of immediate data (in bytes) that follows this 64-bit structure after
                                                                 skipping [APAD] bytes. The next subdescriptor follows [APAD]+[SIZE] bytes
                                                                 later in the descriptor, rounded up to the next 16-byte aligned address.
                                                                 [SIZE] must be greater than 0, and [APAD]+[SIZE] must be less than or equal
                                                                 to 264 bytes. */
		u64 apad:3;	     /**< [ 18: 16] Alignment pad. Number of bytes to skip following this 64-bit structure before
                                                                 the first byte to be included in the packet data. */
		u64 reserved_19_59:41;
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates immediate. Enumerated by NIX_SUBDC_E::IMM. */
	} s;
	/* struct cavm_nix_send_imm_s_s cn; */
};

/**
 * Structure nix_send_jump_s
 *
 * NIX Send Jump Subdescriptor Structure
 * The send jump subdescriptor selects a new address for fetching the remaining
 * subdescriptors of a send descriptor. This allows software to create a send
 * descriptor longer than SQE size selected by NIX_SQ_CTX_S[MAX_SQE_SIZE].
 *
 * There can be only one NIX_SEND_JUMP_S subdescriptor in a send descriptor. If
 * present, it must immediately follow NIX_SEND_HDR_S if NIX_SEND_EXT_S is not
 * present, else it must immediately follow NIX_SEND_EXT_S. In either case, it
 * must terminate the SQE enqueued by software.
 */
union cavm_nix_send_jump_s {
	u64 u[2];
	struct cavm_nix_send_jump_s_s {
		u64 sizem1:7;	     /**< [  6:  0] Number of 16-byte subdescriptor words (minus one) in the subdescriptor list that [ADDR]
                                                                 points to. */
		u64 reserved_7_13:7;
		u64 ld_type:2;	     /**< [ 15: 14] Specifies load transaction type to use for reading post-jump
                                                                 subdescriptors. Enumerated by NIX_SENDLDTYPE_E. */
		u64 aura:20;	     /**< [ 35: 16] Aura number. See [F]. */
		u64 reserved_36_58:23;
		u64 f:1;		     /**< [ 59: 59] Free.
                                                                 0 = Hardware will not free the buffer indicated by [ADDR].
                                                                 1 = Hardware will free the buffer indicated by [ADDR] to NPA after it has read all
                                                                 subdescriptors from it.

                                                                 NIX sends [ADDR] to NPA as part of the buffer free when [F] is set. Either an NPA
                                                                 naturally-aligned pool or opaque pool may be appropriate. Refer to the NPA chapter. */
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates send jump. Enumerated by NIX_SUBDC_E::JUMP. */
		u64 addr:64;	     /**< [127: 64] LF IOVA of the first byte of the next subdescriptor. See NIX_IOVA_S[ADDR]. Bits
                                                                 \<3:0\> are ignored; address must be 16-byte aligned.

                                                                 If NIX_AF_LF()_CFG[BE] is set for this LF (VF/PF), [ADDR] points to big-endian
                                                                 instructions, otherwise little-endian. */
	} s;
	/* struct cavm_nix_send_jump_s_s cn; */
};

/**
 * Structure nix_send_mem_s
 *
 * NIX Send Memory Subdescriptor Structure
 * The send memory subdescriptor atomically sets, increments or decrements a memory location.
 *
 * NIX_SEND_MEM_S subdescriptors must follow all NIX_SEND_SG_S and NIX_SEND_IMM_S
 * subdescriptors in the NIX send descriptor. NIX will not initiate the memory
 * update for this subdescriptor until after it has completed all LLC/DRAM fetches
 * that service all prior NIX_SEND_SG_S subdescriptors. The memory update is
 * executed once, even if the packet is replicated due to NIX_TX_ACTION_S[OP] =
 * NIX_TX_ACTIONOP_E::MCAST.
 *
 * Performance is best if a memory decrement by one is used rather than any other memory
 * set/increment/decrement. (Less internal bus bandwidth is used with memory decrements by one.)
 *
 * When NIX_SEND_EXT_S[LSO] is set in the descriptor, NIX executes the
 * memory update only while processing the last LSO segment, after
 * processing prior segments.
 */
union cavm_nix_send_mem_s {
	u64 u[2];
	struct cavm_nix_send_mem_s_s {
		u64 offset:16;	     /**< [ 15:  0] Adder offset. Constant value to add or subtract or set. If the count being
                                                                 modified is to represent the true packet size, then the offset may
                                                                 represent the pad and FCS appended to the packet.

                                                                 Internal:
                                                                 Note IOB hardware has a special encoding for atomic decrement,
                                                                 therefore a change of minus one is twice as IOB bandwidth efficient as adding/subtracting
                                                                 other values or setting. */
		u64 reserved_16_52:37;
		u64 wmem:1;	     /**< [ 53: 53] Wait for memory.
                                                                 0 = The memory operation may complete after the CQE is posted and/or add work is
                                                                 initiated, and potentially after software has begun servicing the
                                                                 work/completion.
                                                                 1 = NIX will wait for this NIX_SEND_MEM_S requested memory operation to
                                                                 complete and commit before adding a send completion CQE for the send
                                                                 descriptor if NIX_SEND_HDR_S[PNC] is set, and before initiating SSO add
                                                                 work for any NIX_SEND_WORK_S in the descriptor. This may have reduced
                                                                 performance over not waiting. */
		u64 dsz:2;		     /**< [ 55: 54] Memory data size. The size of the word in memory, enumerated by NIX_SENDMEMDSZ_E. */
		u64 alg:4;		     /**< [ 59: 56] Adder algorithm. How to modify the memory location, for example by setting or atomically
                                                                 incrementing. Enumerated by NIX_SENDMEMALG_E.

                                                                 Internal:
                                                                 NCB command type is selected as follows:
                                                                 \<pre\>
                                                                 switch ([ALG]) {
                                                                    case NIX_SENDMEMALG_E::SET :
                                                                    case NIX_SENDMEMALG_E::SETTSTMP :
                                                                    case NIX_SENDMEMALG_E::SETRSLT :
                                                                       cmd_type = RSTP;

                                                                    case NIX_SENDMEMALG_E::ADD :
                                                                    case NIX_SENDMEMALG_E::ADDLEN :
                                                                    case NIX_SENDMEMALG_E::SUBLEN :
                                                                    case NIX_SENDMEMALG_E::ADDMBUF :
                                                                    case NIX_SENDMEMALG_E::SUBMBUF :
                                                                       switch ([DSZ]) {
                                                                          case NIX_SENDMEMDSZ_E::B8 :
                                                                          case NIX_SENDMEMDSZ_E::B16 :
                                                                             unpredictable();
                                                                          case NIX_SENDMEMDSZ_E::B32 :
                                                                             cmd_type = SAA32;
                                                                          case NIX_SENDMEMDSZ_E::B64 :
                                                                             cmd_type = SAA64;
                                                                       }

                                                                    NIX_SENDMEMALG_E::SUB :
                                                                       switch ([DSZ]) {
                                                                          case NIX_SENDMEMDSZ_E::B8 :
                                                                          case NIX_SENDMEMDSZ_E::B16 :
                                                                             unpredictable();
                                                                          case NIX_SENDMEMDSZ_E::B32 :
                                                                             cmd_type = ([OFFSET] == 1) ? SAAM132 : SAA32;
                                                                          case NIX_SENDMEMDSZ_E::B64 :
                                                                             cmd_type = ([OFFSET] == 1) ? SAAM164 : SAA64;
                                                                       }

                                                                    default:
                                                                       unpredictable();
                                                                 }
                                                                 \</pre\> */
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates send memory. Enumerated by NIX_SUBDC_E::MEM. */
		u64 addr:64;	     /**< [127: 64] LF IOVA of the LLC/DRAM address to be modified.
                                                                 [ADDR] must be naturally aligned to the size specified in [DSZ].
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended
                                                                 bit \<52\> for forward compatibility.
                                                                 If NIX_AF_LF()_CFG[BE] is set for this LF (VF/PF), [ADDR] is a big-endian byte
                                                                 pointer. Otherwise, [ADDR] is a little-endian byte pointer. */
	} s;
	/* struct cavm_nix_send_mem_s_s cn; */
};

/**
 * Structure nix_send_sg_s
 *
 * NIX Send Scatter/Gather Subdescriptor Structure
 * The send scatter/gather subdescriptor requests one to three segments of packet
 * data bytes to be transmitted. There may be multiple NIX_SEND_SG_Ss in each NIX send descriptor.
 *
 * NIX_SEND_SG_S is immediately followed by one NIX_IOVA_S word when [SEGS] = 1,
 * three NIX_IOVA_S words when [SEGS] \>= 2. Each NIX_IOVA_S word specifies the
 * LF IOVA of first packet data byte in the corresponding segment; first NIX_IOVA_S
 * word for segment 1, second word for segment 2, third word for segment 3. Note
 * the third word is present when [SEGS] \>= 2 but only valid when [SEGS] = 3.
 *
 * If the sum of all prior NIX_SEND_SG_S[SEG*_SIZE]s and NIX_SEND_IMM_S[SIZE]s
 * meets or exceeds NIX_SEND_HDR_S[TOTAL], this subdescriptor will not contribute
 * any packet data but may free buffers to NPA (see [I1]).
 */
union cavm_nix_send_sg_s {
	u64 u;
	struct cavm_nix_send_sg_s_s {
		u64 seg1_size:16;	     /**< [ 15:  0] Size of segment 1 in bytes. */
		u64 seg2_size:16;	     /**< [ 31: 16] Size of segment 2 in bytes. Valid when [SEGS] \>= 2. */
		u64 seg3_size:16;	     /**< [ 47: 32] Size of segment 3 in bytes. Valid when [SEGS] = 3. */
		u64 segs:2;	     /**< [ 49: 48] Number of valid segments. Must be nonzero. */
		u64 reserved_50_54:5;
		u64 i1:1;		     /**< [ 55: 55] Invert segment 1 free. NIX frees buffer surrounding segment 1 when:

                                                                 _  (NIX_SEND_HDR_S[DF] == [I1]).

                                                                 NIX frees the buffer to NIX_SEND_HDR_S[AURA]. The buffer is freed even if
                                                                 the segment does not contribute any packet data (e.g. when [SEG1_SIZE] is
                                                                 zero or when NIX_SEND_HDR_S[TOTAL] has already been met or exceeded).

                                                                 NIX naturally aligns the segment's NIV_IOVA_S to 128 bytes before sending
                                                                 it to NPA as part of the buffer free. An NPA naturally-aligned pool is
                                                                 recommended, though opaque pool mode may also be possible. Refer to the NPA
                                                                 chapter. */
		u64 i2:1;		     /**< [ 56: 56] Invert segment 2 free. NIX frees buffer surrounding segment 2 when:

                                                                 _  ([SEGS] \>= 2) && (NIX_SEND_HDR_S[DF] == [I2]).

                                                                 See also [I1]. */
		u64 i3:1;		     /**< [ 57: 57] Invert segment 3 free. NIX frees buffer surrounding segment 3 when:

                                                                 _  ([SEGS] == 3) && (NIX_SEND_HDR_S[DF] == [I3]).

                                                                 See also [I1]. */
		u64 ld_type:2;	     /**< [ 59: 58] Specifies load transaction type to use for reading segment bytes. Enumerated by
                                                                 NIX_SENDLDTYPE_E. */
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates scatter/gather. Enumerated by NIX_SUBDC_E::SG. */
	} s;
	/* struct cavm_nix_send_sg_s_s cn; */
};

/**
 * Structure nix_send_work_s
 *
 * NIX Send Work Subdescriptor Structure
 * This subdescriptor adds work to the SSO. At most one NIX_SEND_WORK_S subdescriptor
 * can exist in the NIX send descriptor. If a NIX_SEND_WORK_S exists in the
 * descriptor, it must be the last subdescriptor. NIX will not initiate the work add
 * for this subdescriptor until after (1) it has completed all LLC/DRAM fetches that
 * service all prior NIX_SEND_SG_S subdescriptors, (2) it has
 * fetched all subdescriptors in the descriptor, and (3) all NIX_SEND_MEM_S[WMEM]=1
 * LLC/DRAM updates have completed.
 *
 * Provided the path of descriptors from the SQ through NIX to an output FIFO is
 * unmodified between the descriptors (as should normally be the case, but it is
 * possible for software to change the path), NIX also (1) will submit
 * the SSO add works from all descriptors in the SQ in order, and
 * (2) will not submit an SSO work add until after all prior descriptors
 * in the SQ have completed their NIX_SEND_SG_S
 * processing, and (3) will not submit an SSO work add until after
 * it has fetched all subdescriptors from prior descriptors in the SQ.
 *
 * When NIX_SEND_EXT_S[LSO] is set in the descriptor, NIX executes the
 * NIX_SEND_WORK_S work add only while processing the last LSO segment, after
 * processing prior segments.
 *
 * Hardware ignores NIX_SEND_WORK_S when NIX_SQ_CTX_S[SSO_ENA] is clear.
 */
union cavm_nix_send_work_s {
	u64 u[2];
	struct cavm_nix_send_work_s_s {
		u64 tag:32;	     /**< [ 31:  0] The SSO tag to use when NIX submits work to SSO. */
		u64 tt:2;		     /**< [ 33: 32] SSO tag type. The SSO tag type number to add work with. */
		u64 grp:10;	     /**< [ 43: 34] SSO group. The SSO group number to add work to. Note the upper two bits
                                                                 correspond to a node number. */
		u64 reserved_44_59:16;
		u64 subdc:4;	     /**< [ 63: 60] Subdescriptor code. Indicates send work. Enumerated by NIX_SUBDC_E::WORK. */
		u64 addr:64;	     /**< [127: 64] LF IOVA of the work-queue entry to be submitted to the SSO. See NIX_IOVA_S[ADDR].
                                                                 Bits \<2:0\> are ignored; address must be eight-byte aligned. */
	} s;
	/* struct cavm_nix_send_work_s_s cn; */
};

/**
 * Structure nix_sq_ctx_hw_s
 *
 * NIX SQ Context Hardware Structure
 * This structure contains context state maintained by hardware for each SQ in
 * NDC/LLC/DRAM.
 * Software uses the equivalent NIX_SQ_CTX_S structure format to read and write an
 * SQ context with the NIX admin queue.
 * Always stored in byte invariant little-endian format (LE8).
 */
union cavm_nix_sq_ctx_hw_s {
	u64 u[16];
	struct cavm_nix_sq_ctx_hw_s_s {
		u64 gbl_ena:1;	     /**< [  0:  0] See NIX_SQ_CTX_S[ENA]. */
		u64 gbl_substream:20;   /**< [ 20:  1] See NIX_SQ_CTX_S[SUBSTREAM]. */
		u64 gbl_max_sqe_size:2; /**< [ 22: 21] See NIX_SQ_CTX_S[MAX_SQE_SIZE]. */
		u64 gbl_sqe_way_mask:16;/**< [ 38: 23] See NIX_SQ_CTX_S[SQE_WAY_MASK]. */
		u64 gbl_sqb_aura:20;    /**< [ 58: 39] See NIX_SQ_CTX_S[SQB_AURA]. This is the last entry for the enqueue engine requests. */
		u64 gbl_rsvd1:5;	     /**< [ 63: 59] Reserved. */
		u64 gbl_cq_id:20;	     /**< [ 83: 64] See NIX_SQ_CTX_S[CQ]. */
		u64 gbl_cq_ena:1;	     /**< [ 84: 84] See NIX_SQ_CTX_S[CQ_ENA]. */
		u64 qint_idx:7;	     /**< [ 91: 85] See NIX_SQ_CTX_S[QINT_IDX]. */
		u64 sq_int:8;	     /**< [ 99: 92] See NIX_SQ_CTX_S[SQ_INT]. */
		u64 sq_int_ena:8;	     /**< [107:100] See NIX_SQ_CTX_S[SQ_INT_ENA]. */
		u64 xoff:1;	     /**< [108:108] See NIX_SQ_CTX_S[XOFF]. */
		u64 send_lso_segnum:8;  /**< [116:109] See NIX_SQ_CTX_S[SEND_LSO_SEGNUM] - NA. */
		u64 gbl_rsvd:11;	     /**< [127:117] Reserved. */
		u64 sqb_enqueue_count:16;
					     /**< [143:128] Used in combination with [SQB_DEQUEUE_COUNT] to respond back to Software
                                                                 for AQ reads corresponding to NIX_SQ_CTX_S[SQB_COUNT] =
                                                                 [SQB_ENQUEUE_COUNT] - [SQB_DEQUEUE_COUNT]. */
		u64 sqe_stype:2;	     /**< [145:144] See NIX_SQ_CTX_S[SQE_STYPE]. */
		u64 tail_offset:6;	     /**< [151:146] See NIX_SQ_CTX_S[TAIL_OFFSET]. */
		u64 lmt_dis:1;	     /**< [152:152] See NIX_SQ_CTX_S[LMT_DIS]. */
		u64 dnq_rsvd1:39;	     /**< [191:153] Reserved. */
		u64 tail_sqb:64;	     /**< [255:192] See NIX_SQ_CTX_S[TAIL_SQB]. */
		u64 next_sqb:64;	     /**< [319:256] See NIX_SQ_CTX_S[NEXT_SQB]. */
		u64 mnq_dis:1;	     /**< [320:320] See NIX_SQ_CTX_S[MNQ_DIS]. */
		u64 smq:10;	     /**< [330:321] See NIX_SQ_CTX_S[SMQ]. */
		u64 smq_pend:1;	     /**< [331:331] See NIX_SQ_CTX_S[SMQ_PEND]. */
		u64 smq_next_sq:20;     /**< [351:332] See NIX_SQ_CTX_S[SMQ_NEXT_SQ]. */
		u64 smq_next_sq_vld:1;  /**< [352:352] See NIX_SQ_CTX_S[SMQ_NEXT_SQ] is valid. */
		u64 scm1_rsvd2:31;	     /**< [383:353] Reserved. */
		u64 smenq_sqb:64;	     /**< [447:384] See NIX_SQ_CTX_S[SMENQ_SQB]. */
		u64 smenq_offset:6;     /**< [453:448] See NIX_SQ_CTX_S[SMENQ_OFFSET]. */
		u64 cq_limit:8;	     /**< [461:454] See NIX_SQ_CTX_S[CQ_LIMIT]. */
		u64 smq_rr_quantum:24;  /**< [485:462] See NIX_SQ_CTX_S[SMQ_RR_QUANTUM]. */
		u64 smq_rr_count:25;    /**< [510:486] See NIX_SQ_CTX_S[SMQ_RR_COUNT]. */
		u64 scm_dq_rsvd0:1;     /**< [511:511] Reserved. */
		u64 smq_lso_segnum:8;   /**< [519:512] See NIX_SQ_CTX_S[SMQ_LSO_SEGNUM]. */
		u64 vfi_lso_total:18;   /**< [537:520] Used for VF-Isolation. See NIX_SEND_HDR_S[TOTAL]. */
		u64 vfi_lso_sizem1:3;   /**< [540:538] Used for VF-Isolation. See NIX_SEND_HDR_S[SIZEM1]. */
		u64 vfi_lso_sb:8;	     /**< [548:541] Used for VF-Isolation. See NIX_SEND_EXT_S[LSO_SB]. */
		u64 vfi_lso_mps:14;     /**< [562:549] Used for VF-Isolation. See NIX_SEND_EXT_S[LSO_MPS]. */
		u64 vfi_lso_vlan0_ins_ena:1;
					     /**< [563:563] Used for VF-Isolation. See NIX_SEND_EXT_S[VLAN0_INS_ENA]. */
		u64 vfi_lso_vlan1_ins_ena:1;
					     /**< [564:564] Used for VF-Isolation. See NIX_SEND_EXT_S[VLAN1_INS_ENA]. */
		u64 vfi_lso_vld:1;	     /**< [565:565] Used for VF-Isolation. See NIX_SEND_EXT_S[LSO]. */
		u64 scm_dq_rsvd1:10;    /**< [575:566] Reserved. */
		u64 scm_lso_rem:18;     /**< [593:576] Reserved. */
		u64 vfi_smq_sqb:46;     /**< [639:594] Reserved. */
		u64 head_sqb:64;	     /**< [703:640] See NIX_SQ_CTX_S[HEAD_SQB]. */
		u64 head_offset:6;	     /**< [709:704] See NIX_SQ_CTX_S[HEAD_OFFSET]. */
		u64 sqb_dequeue_count:16;
					     /**< [725:710] See [SQB_ENQUEUE_COUNT]. */
		u64 default_chan:12;    /**< [737:726] See NIX_SQ_CTX_S[DEFAULT_CHAN]. */
		u64 sdp_mcast:1;	     /**< [738:738] See NIX_SQ_CTX_S[SDP_MCAST]. */
		u64 sso_ena:1;	     /**< [739:739] See NIX_SQ_CTX_S[SSO_ENA]. */
		u64 dse_rsvd1:28;	     /**< [767:740] Reserved. */
		u64 lso_crc_iv:32;	     /**< [799:768] See NIX_SQ_CTX_S[LSO_CRC_IV]. */
		u64 seb_rsvd1:32;	     /**< [831:800] Reserved. */
		u64 drop_pkts:48;	     /**< [879:832] See NIX_SQ_CTX_S[DROP_PKTS]. */
		u64 drop_octs_lsw:16;   /**< [895:880] See NIX_SQ_CTX_S[DROP_OCTS]. */
		u64 drop_octs_msw:32;   /**< [927:896] See NIX_SQ_CTX_S[DROP_OCTS]. */
		u64 pkts_lsw:32;	     /**< [959:928] See NIX_SQ_CTX_S[PKTS]. */
		u64 pkts_msw:16;	     /**< [975:960] See NIX_SQ_CTX_S[PKTS]. */
		u64 octs:48;	     /**< [1023:976] See NIX_SQ_CTX_S[OCTS]. */
	} s;
	/* struct cavm_nix_sq_ctx_hw_s_s cn; */
};

/**
 * Structure nix_sq_ctx_s
 *
 * NIX Send Queue Context Structure
 * This structure specifies the format used by software with the NIX admin queue
 * to read and write a send queue's NIX_SQ_CTX_HW_S structure maintained by
 * hardware in NDC/LLC/DRAM.
 *
 * The SQ statistics ([OCTS], [PKTS], [DROP_OCTS], [DROP_PKTS]) do not account for
 * packet replication due to NIX_TX_ACTION_S[OP] = NIX_TX_ACTIONOP_E::MCAST.
 */
union cavm_nix_sq_ctx_s {
	u64 u[16];
	struct cavm_nix_sq_ctx_s_s {
		u64 ena:1;		     /**< [  0:  0] SQ enable. */
		u64 cq_ena:1;	     /**< [  1:  1] Completion queue enable.
                                                                 0 = NIX_SEND_HDR_S[PNC] is ignored and a packet from this SQ will never generate
                                                                 a CQE.
                                                                 1 = A packet with NIX_SEND_HDR_S[PNC] will add a send completion CQE to [CQ]. */
		u64 max_sqe_size:2;     /**< [  3:  2] Selects maximum SQE size for this SQ. Enumerated by NIX_MAXSQESZ_E.
                                                                 Internal:
                                                                 Hardware allocates this size for each SQE stored in an SQB. */
		u64 substream:20;	     /**< [ 23:  4] Substream ID of IOVAs specified by NIX_SEND_SG_S, NIX_SEND_MEM_S, etc. */
		u64 sdp_mcast:1;	     /**< [ 24: 24] SDP multicast. Valid if the SQ sends packets to SDP (corresponding
                                                                 NIX_AF_TL4()_SDP_LINK_CFG[ENA] is set):
                                                                 0 = SQ sends SDP unicast packets.
                                                                 1 = SQ sends SDP multicast packets. */
		u64 lmt_dis:1;	     /**< [ 25: 25] LMT store disable. Hardware sets this bit along with
                                                                 [SQ_INT]\<NIX_SQINT_E::LMT_ERR\> when an LMT store to NIX_LF_OP_SEND()
                                                                 for this SQ has an error. See also NIX_LF_SQ_OP_ERR_DBG. When set,
                                                                 hardware drops LMT stores targeting this SQ. */
		u64 mnq_dis:1;	     /**< [ 26: 26] Meta-descriptor enqueue disable. Hardware sets this bit along with
                                                                 [SQ_INT]\<NIX_SQINT_E::MNQ_ERR\> when an error is detected while enqueuing a
                                                                 meta-descriptor to [SMQ] from this SQ. When set, hardware stops enqueuing to
                                                                 [SMQ] from this SQ. */
		u64 reserved_27_35:9;
		u64 cq:20;		     /**< [ 55: 36] Completion queue for this SQ. Valid when [CQ_ENA] is set. */
		u64 cq_limit:8;	     /**< [ 63: 56] Threshold level for suppressing packet send, in units of 1/256th of CQ
                                                                 level.  0xff represents an empty CQ ring, 0x0 represents a full ring.
                                                                 Packets will not be sent from the SQ if the available space in the
                                                                 associated CQ (see shifted_CNT in NIX_CQ_CTX_S[AVG_CON]) is less than the
                                                                 [CQ_LIMIT] value. */
		u64 smq:10;	     /**< [ 73: 64] Send meta-descriptor queue for this SQ. Must be less than 512. */
		u64 xoff:1;	     /**< [ 74: 74] Transmit off. When set, the SQ will not push meta descriptors to the
                                                                 associated SMQ. Software can read, set and clear this bit with
                                                                 NIX_LF_SQ_OP_INT[XOFF]. */
		u64 sso_ena:1;	     /**< [ 75: 75] SSO add work enable.
                                                                 0 = The SQ never adds work to SSO, and NIX_SEND_WORK_S is ignored when present
                                                                 in a send descriptor.
                                                                 1 = A packets with NIX_SEND_WORK_S will add work to SSO. */
		u64 smq_rr_quantum:24;  /**< [ 99: 76] Round-robin (DWRR) quantum for packets pushed from this SQ to the
                                                                 associated SMQ (24-bit unsigned integer). Specifies the amount of packet
                                                                 data bytes to push to SMQ in a round.

                                                                 The minimum value is the MTU; this is also the typical value for
                                                                 equal-weight arbitration. */
		u64 default_chan:12;    /**< [111:100] Default channel enumerated by NIX_CHAN_E.

                                                                 If the SQ transmits to CGX and/or LBK (corresponding
                                                                 NIX_AF_TL4()_SDP_LINK_CFG[ENA] is clear), this is the channel to which a
                                                                 packet is transmitted when NIX_TX_ACTION_S[OP] =
                                                                 NIX_TX_ACTIONOP_E::UCAST_DEFAULT in the NPC result.

                                                                 If the SQ transmits to SDP (corresponding NIX_AF_TL4()_SDP_LINK_CFG[ENA] is
                                                                 set), this is the SDP channel to which packets are transmitted when
                                                                 [SDP_MCAST] is clear, and the SDP multicast index when [SDP_MCAST] is set. */
		u64 sqb_count:16;	     /**< [127:112] Number of SQBs currently in use. Includes the SQBs at [HEAD_SQB] and
                                                                 [TAIL_SQB], and any linked SQBs in between. Excludes the SQB at [NEXT_SQB]. */
		u64 sqe_way_mask:16;    /**< [143:128] Way partitioning mask for allocating SQB data in NDC (1 means do not use).
                                                                 All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 reserved_144_171:28;
		u64 sq_int:8;	     /**< [179:172] SQ interrupts. Bits enumerated by NIX_SQINT_E, which also defines when
                                                                 hardware sets each bit. Software can read, set or clear these bits with
                                                                 NIX_LF_SQ_OP_INT. */
		u64 sq_int_ena:8;	     /**< [187:180] SQ interrupt enables. Bits enumerated by NIX_SQINT_E. Software can read,
                                                                 set or clear these bits with NIX_LF_SQ_OP_INT. */
		u64 sqe_stype:2;	     /**< [189:188] Selects the style of write and read for accessing SQB data in LLC/DRAM.
                                                                 Enumerated by NIX_STYPE_E.
                                                                 Must be NIX_STYPE_E::STP when NIX_SQ_CTX_S[MAX_SQE_SIZE] =
                                                                 NIX_MAXSQESZ_E::W8. */
		u64 smq_pend:1;	     /**< [190:190] When set, indicates that this SQ has pending SQEs to be parsed and pushed to the associated SMQ. */
		u64 reserved_191:1;
		u64 reserved_192_223:32;
		u64 send_lso_segnum:8;  /**< [231:224] Next LSO segment number to send. */
		u64 smq_lso_segnum:8;   /**< [239:232] Next LSO segment number to enqueue to PSE. */
		u64 qint_idx:7;	     /**< [246:240] Queue interrupt index. Select the QINT within LF (index {a} of
                                                                 NIX_LF_QINT()*) which receives [SQ_INT] events.

                                                                 Internal:
                                                                 See QINT message generation note in NIX_RQ_CTX_S[QINT_IDX]. */
		u64 reserved_247_255:9;
		u64 next_sqb:64;	     /**< [319:256] IOVA of next SQB. A NULL value when valid indicates allocation of next SQB
                                                                 from [SQB_AURA] failed. */
		u64 tail_sqb:64;	     /**< [383:320] IOVA of tail SQB. Valid when [SQB_COUNT] is nonzero. */
		u64 smenq_sqb:64;	     /**< [447:384] IOVA of SMQ enqueue SQB. Valid when [SQB_COUNT] is nonzero. */
		u64 head_sqb:64;	     /**< [511:448] IOVA of head SQB. Valid when [SQB_COUNT] is nonzero. */
		u64 tail_offset:6;	     /**< [517:512] Offset of next SQE to be enqueued in [TAIL_SQB]. */
		u64 smenq_offset:6;     /**< [523:518] Offset of next SQE to bve pushed to SMQ in [SMENQ_SQB]. */
		u64 head_offset:6;	     /**< [529:524] Offset of head SQE in [HEAD_SQB]. */
		u64 reserved_530_539:10;
		u64 sqb_aura:20;	     /**< [559:540] SQB aura. Aura within NIX_AF_LF()_CFG[NPA_PF_FUNC] used for SQE buffer
                                                                 allocations and frees for this SQ. The selected aura must correspond to a
                                                                 pool where the buffers (after any NPA_POOL_S[BUF_OFFSET]) are at least of
                                                                 size NIX_AF_SQ_CONST[SQB_SIZE] (4KB). */
		u64 reserved_560_575:16;
		u64 smq_rr_count:25;    /**< [600:576] Round-robin (DWRR) deficit counter for packets pushed from this SQ to the associated SMQ.
                                                                 A 25-bit two's complement signed integer count. */
		u64 reserved_601:1;
		u64 smq_next_sq:20;     /**< [621:602] Next SQ within the LF to process in SMQ parse link list. Valid when
                                                                 [SMQ_PEND] is set and the SQ is not at the tail of the SMQ's link list. */
		u64 reserved_622_639:18;
		u64 octs:48;	     /**< [687:640] Number of octets transmitted. Includes frame minimum size pad bytes due to
                                                                 NIX_AF_SMQ()_CFG[MINLEN], and excludes FCS bytes. Also includes any VLAN
                                                                 bytes inserted by NIX_SEND_EXT_S[VLAN*] and/or Vtag bytes inserted by
                                                                 NIX_TX_VTAG_ACTION_S. */
		u64 reserved_688_703:16;
		u64 pkts:48;	     /**< [751:704] Number of packets successfully transmitted. */
		u64 reserved_752_767:16;
		u64 lso_crc_iv:32;	     /**< [799:768] NIX_SEND_CRC_S intermediate value between LSO segments.
                                                                 Internal:
                                                                 This word is for SEB use and should not be written by SQM. */
		u64 reserved_800_831:32;
		u64 reserved_832_895:64;
		u64 drop_octs:48;	     /**< [943:896] Number of dropped octets. See also NIX_STAT_LF_TX_E::TX_DROP. */
		u64 reserved_944_959:16;
		u64 drop_pkts:48;	     /**< [1007:960] Number of dropped packets. See also NIX_STAT_LF_TX_E::TX_DROP. */
		u64 reserved_1008_1023:16;
	} s;
	struct cavm_nix_sq_ctx_s_cn {
		u64 ena:1;		     /**< [  0:  0] SQ enable. */
		u64 cq_ena:1;	     /**< [  1:  1] Completion queue enable.
                                                                 0 = NIX_SEND_HDR_S[PNC] is ignored and a packet from this SQ will never generate
                                                                 a CQE.
                                                                 1 = A packet with NIX_SEND_HDR_S[PNC] will add a send completion CQE to [CQ]. */
		u64 max_sqe_size:2;     /**< [  3:  2] Selects maximum SQE size for this SQ. Enumerated by NIX_MAXSQESZ_E.
                                                                 Internal:
                                                                 Hardware allocates this size for each SQE stored in an SQB. */
		u64 substream:20;	     /**< [ 23:  4] Substream ID of IOVAs specified by NIX_SEND_SG_S, NIX_SEND_MEM_S, etc. */
		u64 sdp_mcast:1;	     /**< [ 24: 24] SDP multicast. Valid if the SQ sends packets to SDP (corresponding
                                                                 NIX_AF_TL4()_SDP_LINK_CFG[ENA] is set):
                                                                 0 = SQ sends SDP unicast packets.
                                                                 1 = SQ sends SDP multicast packets. */
		u64 lmt_dis:1;	     /**< [ 25: 25] LMT store disable. Hardware sets this bit along with
                                                                 [SQ_INT]\<NIX_SQINT_E::LMT_ERR\> when an LMT store to NIX_LF_OP_SEND()
                                                                 for this SQ has an error. See also NIX_LF_SQ_OP_ERR_DBG. When set,
                                                                 hardware drops LMT stores targeting this SQ. */
		u64 mnq_dis:1;	     /**< [ 26: 26] Meta-descriptor enqueue disable. Hardware sets this bit along with
                                                                 [SQ_INT]\<NIX_SQINT_E::MNQ_ERR\> when an error is detected while enqueuing a
                                                                 meta-descriptor to [SMQ] from this SQ. When set, hardware stops enqueuing to
                                                                 [SMQ] from this SQ. */
		u64 reserved_27:1;
		u64 reserved_28_35:8;
		u64 cq:20;		     /**< [ 55: 36] Completion queue for this SQ. Valid when [CQ_ENA] is set. */
		u64 cq_limit:8;	     /**< [ 63: 56] Threshold level for suppressing packet send, in units of 1/256th of CQ
                                                                 level.  0xff represents an empty CQ ring, 0x0 represents a full ring.
                                                                 Packets will not be sent from the SQ if the available space in the
                                                                 associated CQ (see shifted_CNT in NIX_CQ_CTX_S[AVG_CON]) is less than the
                                                                 [CQ_LIMIT] value. */
		u64 smq:10;	     /**< [ 73: 64] Send meta-descriptor queue for this SQ. Must be less than 512. */
		u64 xoff:1;	     /**< [ 74: 74] Transmit off. When set, the SQ will not push meta descriptors to the
                                                                 associated SMQ. Software can read, set and clear this bit with
                                                                 NIX_LF_SQ_OP_INT[XOFF]. */
		u64 sso_ena:1;	     /**< [ 75: 75] SSO add work enable.
                                                                 0 = The SQ never adds work to SSO, and NIX_SEND_WORK_S is ignored when present
                                                                 in a send descriptor.
                                                                 1 = A packets with NIX_SEND_WORK_S will add work to SSO. */
		u64 smq_rr_quantum:24;  /**< [ 99: 76] Round-robin (DWRR) quantum for packets pushed from this SQ to the
                                                                 associated SMQ (24-bit unsigned integer). Specifies the amount of packet
                                                                 data bytes to push to SMQ in a round.

                                                                 The minimum value is the MTU; this is also the typical value for
                                                                 equal-weight arbitration. */
		u64 default_chan:12;    /**< [111:100] Default channel enumerated by NIX_CHAN_E.

                                                                 If the SQ transmits to CGX and/or LBK (corresponding
                                                                 NIX_AF_TL4()_SDP_LINK_CFG[ENA] is clear), this is the channel to which a
                                                                 packet is transmitted when NIX_TX_ACTION_S[OP] =
                                                                 NIX_TX_ACTIONOP_E::UCAST_DEFAULT in the NPC result.

                                                                 If the SQ transmits to SDP (corresponding NIX_AF_TL4()_SDP_LINK_CFG[ENA] is
                                                                 set), this is the SDP channel to which packets are transmitted when
                                                                 [SDP_MCAST] is clear, and the SDP multicast index when [SDP_MCAST] is set. */
		u64 sqb_count:16;	     /**< [127:112] Number of SQBs currently in use. Includes the SQBs at [HEAD_SQB] and
                                                                 [TAIL_SQB], and any linked SQBs in between. Excludes the SQB at [NEXT_SQB]. */
		u64 sqe_way_mask:16;    /**< [143:128] Way partitioning mask for allocating SQB data in NDC (1 means do not use).
                                                                 All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 reserved_144_171:28;
		u64 sq_int:8;	     /**< [179:172] SQ interrupts. Bits enumerated by NIX_SQINT_E, which also defines when
                                                                 hardware sets each bit. Software can read, set or clear these bits with
                                                                 NIX_LF_SQ_OP_INT. */
		u64 sq_int_ena:8;	     /**< [187:180] SQ interrupt enables. Bits enumerated by NIX_SQINT_E. Software can read,
                                                                 set or clear these bits with NIX_LF_SQ_OP_INT. */
		u64 sqe_stype:2;	     /**< [189:188] Selects the style of write and read for accessing SQB data in LLC/DRAM.
                                                                 Enumerated by NIX_STYPE_E.
                                                                 Must be NIX_STYPE_E::STP when NIX_SQ_CTX_S[MAX_SQE_SIZE] =
                                                                 NIX_MAXSQESZ_E::W8. */
		u64 smq_pend:1;	     /**< [190:190] When set, indicates that this SQ has pending SQEs to be parsed and pushed to the associated SMQ. */
		u64 reserved_191:1;
		u64 reserved_192_223:32;
		u64 send_lso_segnum:8;  /**< [231:224] Next LSO segment number to send. */
		u64 smq_lso_segnum:8;   /**< [239:232] Next LSO segment number to enqueue to PSE. */
		u64 qint_idx:7;	     /**< [246:240] Queue interrupt index. Select the QINT within LF (index {a} of
                                                                 NIX_LF_QINT()*) which receives [SQ_INT] events.

                                                                 Internal:
                                                                 See QINT message generation note in NIX_RQ_CTX_S[QINT_IDX]. */
		u64 reserved_247_255:9;
		u64 next_sqb:64;	     /**< [319:256] IOVA of next SQB. A NULL value when valid indicates allocation of next SQB
                                                                 from [SQB_AURA] failed. */
		u64 tail_sqb:64;	     /**< [383:320] IOVA of tail SQB. Valid when [SQB_COUNT] is nonzero. */
		u64 smenq_sqb:64;	     /**< [447:384] IOVA of SMQ enqueue SQB. Valid when [SQB_COUNT] is nonzero. */
		u64 head_sqb:64;	     /**< [511:448] IOVA of head SQB. Valid when [SQB_COUNT] is nonzero. */
		u64 tail_offset:6;	     /**< [517:512] Offset of next SQE to be enqueued in [TAIL_SQB]. */
		u64 smenq_offset:6;     /**< [523:518] Offset of next SQE to bve pushed to SMQ in [SMENQ_SQB]. */
		u64 head_offset:6;	     /**< [529:524] Offset of head SQE in [HEAD_SQB]. */
		u64 reserved_530_539:10;
		u64 sqb_aura:20;	     /**< [559:540] SQB aura. Aura within NIX_AF_LF()_CFG[NPA_PF_FUNC] used for SQE buffer
                                                                 allocations and frees for this SQ. The selected aura must correspond to a
                                                                 pool where the buffers (after any NPA_POOL_S[BUF_OFFSET]) are at least of
                                                                 size NIX_AF_SQ_CONST[SQB_SIZE] (4KB). */
		u64 reserved_560_575:16;
		u64 smq_rr_count:25;    /**< [600:576] Round-robin (DWRR) deficit counter for packets pushed from this SQ to the associated SMQ.
                                                                 A 25-bit two's complement signed integer count. */
		u64 reserved_601:1;
		u64 smq_next_sq:20;     /**< [621:602] Next SQ within the LF to process in SMQ parse link list. Valid when
                                                                 [SMQ_PEND] is set and the SQ is not at the tail of the SMQ's link list. */
		u64 reserved_622_639:18;
		u64 octs:48;	     /**< [687:640] Number of octets transmitted. Includes frame minimum size pad bytes due to
                                                                 NIX_AF_SMQ()_CFG[MINLEN], and excludes FCS bytes. Also includes any VLAN
                                                                 bytes inserted by NIX_SEND_EXT_S[VLAN*] and/or Vtag bytes inserted by
                                                                 NIX_TX_VTAG_ACTION_S. */
		u64 reserved_688_703:16;
		u64 pkts:48;	     /**< [751:704] Number of packets successfully transmitted. */
		u64 reserved_752_767:16;
		u64 lso_crc_iv:32;	     /**< [799:768] NIX_SEND_CRC_S intermediate value between LSO segments.
                                                                 Internal:
                                                                 This word is for SEB use and should not be written by SQM. */
		u64 reserved_800_831:32;
		u64 reserved_832_895:64;
		u64 drop_octs:48;	     /**< [943:896] Number of dropped octets. See also NIX_STAT_LF_TX_E::TX_DROP. */
		u64 reserved_944_959:16;
		u64 drop_pkts:48;	     /**< [1007:960] Number of dropped packets. See also NIX_STAT_LF_TX_E::TX_DROP. */
		u64 reserved_1008_1023:16;
	} cn;
};

/**
 * Structure nix_tx_action_s
 *
 * NIX Transmit Action Structure
 * This structure defines the format of NPC_RESULT_S[ACTION] for a transmit packet.
 */
union cavm_nix_tx_action_s {
	u64 u;
	struct cavm_nix_tx_action_s_s {
		u64 op:4;		     /**< [  3:  0] Action op code enumerated by NIX_TX_ACTIONOP_E. */
		u64 reserved_4_11:8;
		u64 index:20;	     /**< [ 31: 12] Transmit channel or table index in NIX. The index type is selected as
                                                                 follows:
                                                                 _ if [OP] = NIX_TX_ACTIONOP_E::UCAST_CHAN, transmit channel.
                                                                 _ if [OP] = NIX_TX_ACTIONOP_E::MCAST, pointer to start of multicast
                                                                 replication list in the NIX TX multicast table.
                                                                 _ otherwise, not used. */
		u64 match_id:16;	     /**< [ 47: 32] Software defined match identifier. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nix_tx_action_s_s cn; */
};

/**
 * Structure nix_tx_vtag_action_s
 *
 * NIX Transmit Vtag Action Structure
 * This structure defines the format of NPC_RESULT_S[VTAG_ACTION] for a transmit
 * packet. It specifies the optional insertion or replacement of up to two Vtags
 * (e.g. C-VLAN/S-VLAN tags, 802.1BR E-TAG).
 *
 * If two Vtags are specified:
 * * The Vtag 0 byte offset from packet start (see [VTAG0_RELPTR]) must be less
 * than or equal to the Vtag 1 byte offset.
 * * Hardware executes the Vtag 0 action first, Vtag 1 action second.
 * * If Vtag 0 is inserted, hardware adjusts the Vtag 1 byte offset accordingly.
 * Thus, if the two offsets are equal in the structure, hardware inserts Vtag 1
 * immediately after Vtag 0 in the packet.
 *
 * A Vtag must not be inserted or replaced within an outer or inner L3/L4 header,
 * but may be inserted or replaced within an outer L4 payload.
 */
union cavm_nix_tx_vtag_action_s {
	u64 u;
	struct cavm_nix_tx_vtag_action_s_s {
		u64 vtag0_relptr:8;     /**< [  7:  0] Vtag 0 relative pointer. Byte offset from start of selected layer to first
                                                                 tag 0 byte. Must be even.

                                                                 Note the layer pointers in NPC_RESULT_S are offset by 8 bytes to
                                                                 account for NIX_INST_HDR_S, which precedes the packet header supplied to
                                                                 NPC but is not included in the transmitted packet.
                                                                 For example, if [VTAG0_LID] = NPC_LID_E::LB, then the byte offset from
                                                                 packet start (excluding NIX_INST_HDR_S) to the first tag 0 byte is
                                                                 NPC_RESULT_S[LB[LPTR]] - 8 + [VTAG0_RELPTR]. */
		u64 vtag0_lid:3;	     /**< [ 10:  8] Vtag 0 layer ID enumerated by NPC_LID_E. */
		u64 reserved_11:1;
		u64 vtag0_op:2;	     /**< [ 13: 12] Vtag 0 operation enumerated by NIX_TX_VTAGOP_E. */
		u64 reserved_14_15:2;
		u64 vtag0_def:10;	     /**< [ 25: 16] Vtag 0 definition. Index to NIX_AF_TX_VTAG_DEF()_CTL/DATA entry that
                                                                 defines the tag size and data to insert or replace. */
		u64 reserved_26_31:6;
		u64 vtag1_relptr:8;     /**< [ 39: 32] Vtag 1 relative pointer. See [VTAG0_RELPTR]. */
		u64 vtag1_lid:3;	     /**< [ 42: 40] Vtag 1 layer ID enumerated by NPC_LID_E. */
		u64 reserved_43:1;
		u64 vtag1_op:2;	     /**< [ 45: 44] Vtag 1 operation enumerated by NIX_TX_VTAGOP_E. */
		u64 reserved_46_47:2;
		u64 vtag1_def:10;	     /**< [ 57: 48] Vtag 1 definition. Index to NIX_AF_TX_VTAG_DEF()_CTL/DATA entry that
                                                                 defines the tag size and data to insert or replace. */
		u64 reserved_58_63:6;
	} s;
	/* struct cavm_nix_tx_vtag_action_s_s cn; */
};

/**
 * Structure nix_wqe_hdr_s
 *
 * NIX Work Queue Entry Header Structure
 * This 64-bit structure defines the first word of every receive WQE generated by
 * NIX. It is immediately followed by NIX_RX_PARSE_S.
 * Stored in memory as little-endian unless NIX_AF_LF()_CFG[BE] is set.
 */
union cavm_nix_wqe_hdr_s {
	u64 u;
	struct cavm_nix_wqe_hdr_s_s {
		u64 tag:32;	     /**< [ 31:  0] The initial tag for the work-queue entry.
                                                                 See pseudocode in NIX_RQ_CTX_S[LTAG]. */
		u64 tt:2;		     /**< [ 33: 32] The initial tag type for the packet's SSO add work from
                                                                 NIX_RQ_CTX_S[SSO_TT]. Enumerated by SSO_TT_E. */
		u64 grp:10;	     /**< [ 43: 34] The SSO guest-group number used for the packet's add work from
                                                                 NIX_RQ_CTX_S[SSO_GRP]. [GRP]\<9:8\> is always zero. */
		u64 node:2;	     /**< [ 45: 44] Node number on which the packet was received or transmitted.
                                                                 Internal:
                                                                 This is needed by software; do not remove on single-node parts. */
		u64 q:14;		     /**< [ 59: 46] Lower 14 bits of RQ or SQ within VF/PF. */
		u64 wqe_type:4;	     /**< [ 63: 60] WQE type enumerated by NIX_XQE_TYPE_E. */
	} s;
	/* struct cavm_nix_wqe_hdr_s_s cn; */
};

/* Registers */

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_cfg
 *
 * NIX AF Admin Queue Configuration Register
 */
union cavm_nixx_af_aq_cfg {
	u64 u;
	struct cavm_nixx_af_aq_cfg_s {
		u64 qsize:4;		     /**< [  3:  0](R/W) Specifies AQ ring size in entries of 16 bytes:
                                                                 0x0 = 16 entries.
                                                                 0x1 = 64 entries.
                                                                 0x2 = 256 entries.
                                                                 0x3 = 1K entries.
                                                                 0x4 = 4K entries.
                                                                 0x5 = 16K entries.
                                                                 0x6 = 64K entries.
                                                                 0x7 = 256K entries.
                                                                 0x8 = 1M entries.
                                                                 0x9-0xF = Reserved.

                                                                 Note that the usable size of the ring is the specified size minus 1 (HEAD==TAIL always
                                                                 means empty). */
		u64 reserved_4_63:60;
	} s;
	/* struct cavm_nixx_af_aq_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_status
 *
 * NIX AF General Status Register
 */
union cavm_nixx_af_status {
	u64 u;
	struct cavm_nixx_af_status_s {
		u64 blk_busy:10;		     /**< [  9:  0](RO/H) If nonzero, block is not ready for configuration.
                                                                 Internal:
                                                                 Each bit corresponds to a subblock:
                                                                 \<9\> = Reserved.
                                                                 \<8\> = Reserved.
                                                                 \<7\> = TBD.
                                                                 \<6\> = TBD.
                                                                 \<5\> = TBD.
                                                                 \<4\> = TBD.
                                                                 \<3\> = TBD.
                                                                 \<2\> = PSE
                                                                 \<1\> = SEB
                                                                 \<0\> = SQM */
		u64 calibrate_done:1;	     /**< [ 10: 10](RO/H) Calibrate cycle is complete. */
		u64 reserved_11_15:5;
		u64 calibrate_status:5;	     /**< [ 20: 16](RO/H) X2P device calibration state.
                                                                 0 = Device inactive.
                                                                 1 = Device ready.

                                                                 Internal:
                                                                 a "Device Inactive" status means that the X2P agent did not respond to the calibration
                                                                 cycle.
                                                                 This is most likely caused because the X2P agents (CGX, LBK, etc) was in reset during the
                                                                 calibration cycle. */
		u64 reserved_21_63:43;
	} s;
	/* struct cavm_nixx_af_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_ndc_cfg
 *
 * NIX AF General Configuration Register
 */
union cavm_nixx_af_ndc_cfg {
	u64 u;
	struct cavm_nixx_af_ndc_cfg_s {
		u64 ndc_bypass_rx:1;	     /**< [  0:  0](R/W) Forces all NDC transations to Bypass the NDC cache attached to the NIXRX_ block. */
		u64 ndc_bypass_tx:1;	     /**< [  1:  1](R/W) Forces all NDC transations to Bypass the NDC cache attached to the NIXTX block. */
		u64 ndc_ign_pois:1;	     /**< [  2:  2](R/W) Ignore Poison responses from the NDC. */
		u64 reserved_3_63:61;
	} s;
	/* struct cavm_nixx_af_ndc_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_const
 *
 * NIX AF Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_const {
	u64 u;
	struct cavm_nixx_af_const_s {
		u64 cgx_lmac_channels:8;	     /**< [  7:  0](RO) Number of channels per CGX link/LMAC. */
		u64 cgx_lmacs:4;		     /**< [ 11:  8](RO) Number of LMACs (links) per CGX. */
		u64 num_cgx:4;		     /**< [ 15: 12](RO) Number of CGX interfaces enumerated in NIX_LINK_E. */
		u64 lbk_channels:8;	     /**< [ 23: 16](RO) Number of channels per LBK interface/link. */
		u64 lbk_to_nix0:6;		     /**< [ 29: 24](RO/H) Link number for transmitting loopback packets to NIX(0).
                                                                 Value is NIX_LINK_E::LBK(0) for NIX(0) to NIX(0) loopback. */
		u64 lbk_to_nix1:6;		     /**< [ 35: 30](RO/H) Reserved. */
		u64 lbk_from_nix0:6;	     /**< [ 41: 36](RO/H) Link number that receives loopback packets transmitted by NIX(0).
                                                                 Value is NIX_LINK_E::LBK(0) for NIX(0) to NIX(0) loopback. */
		u64 lbk_from_nix1:6;	     /**< [ 47: 42](RO/H) Reserved. */
		u64 links:8;		     /**< [ 55: 48](RO) Number of links enumerated by NIX_LINK_E, including the internal
                                                                 RX multicast/mirror replay interface, NIX_LINK_E::MC. */
		u64 reserved_56_63:8;
	} s;
	/* struct cavm_nixx_af_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_const1
 *
 * NIX AF Constants 1 Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_const1 {
	u64 u;
	struct cavm_nixx_af_const1_s {
		u64 sdp_channels:12;	     /**< [ 11:  0](RO) Number of channels per SDP interface/link. */
		u64 rx_bpids:12;		     /**< [ 23: 12](RO) Number of receive backpressure IDs. */
		u64 lf_tx_stats:8;		     /**< [ 31: 24](RO) Number of per-LF transmit statistics counters enumerated by NIX_STAT_LF_TX_E. */
		u64 lf_rx_stats:8;		     /**< [ 39: 32](RO) Number of per-LF receive statistics counters enumerated by NIX_STAT_LF_RX_E. */
		u64 lso_format_fields:8;	     /**< [ 47: 40](RO) Number of packets fields per LSO format, each selected by FIELD index of
                                                                 NIX_AF_LSO_FORMAT()_FIELD() registers. */
		u64 lso_formats:8;		     /**< [ 55: 48](RO) Number of LSO formats, each selected by FORMAT index of
                                                                 NIX_AF_LSO_FORMAT()_FIELD() registers. */
		u64 reserved_56_63:8;
	} s;
	/* struct cavm_nixx_af_const1_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_const2
 *
 * NIX AF Constants 2 Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_const2 {
	u64 u;
	struct cavm_nixx_af_const2_s {
		u64 lfs:12;		     /**< [ 11:  0](RO) Number of Local Functions. */
		u64 qints:12;		     /**< [ 23: 12](RO) Number of queue interrupts per LF. */
		u64 cints:12;		     /**< [ 35: 24](RO) Number of completion interrupts per LF. */
		u64 reserved_36_63:28;
	} s;
	/* struct cavm_nixx_af_const2_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_const3
 *
 * NIX AF Constants 2 Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_const3 {
	u64 u;
	struct cavm_nixx_af_const3_s {
		u64 sq_ctx_log2bytes:4;	     /**< [  3:  0](RO) SQ context size as log2(bytes). Size of each NIX_SQ_CTX_HW_S structure in a
                                                                 local function's SQ context table NDC/LLC/DRAM. See NIX_AF_LF()_SQS_BASE
                                                                 and NIX_AF_LF()_SQS_CFG. */
		u64 rq_ctx_log2bytes:4;	     /**< [  7:  4](RO) RQ context size as log2(bytes). Size of each NIX_RQ_CTX_S structure in a
                                                                 local function's RQ context table NDC/LLC/DRAM. See NIX_AF_LF()_RQS_BASE
                                                                 and NIX_AF_LF()_RQS_CFG. */
		u64 cq_ctx_log2bytes:4;	     /**< [ 11:  8](RO) CQ context size as log2(bytes). Size of each NIX_CQ_CTX_S structure in a
                                                                 local function's CQ context table NDC/LLC/DRAM. See NIX_AF_LF()_CQS_BASE
                                                                 and NIX_AF_LF()_CQS_CFG. */
		u64 rsse_log2bytes:4;	     /**< [ 15: 12](RO) RSS entry size as log2(bytes). Size of each NIX_RSSE_S structure in a
                                                                 local function's RSS table NDC/LLC/DRAM. See NIX_AF_LF()_RSS_BASE
                                                                 and NIX_AF_LF()_RSS_CFG. */
		u64 mce_log2bytes:4;	     /**< [ 19: 16](RO) Receive multicast/mirror entry size as log2(bytes). Size of each
                                                                 NIX_RX_MCE_S structure in the AF multicast/mirror table NDC/LLC/DRAM. See
                                                                 NIX_AF_RX_MCAST_BASE and NIX_AF_RX_MCAST_CFG. */
		u64 qint_log2bytes:4;	     /**< [ 23: 20](RO) Queue interrupt context size as log2(bytes). Size of each NIX_QINT_HW_S
                                                                 structure in a local function's queue interrupt context table NDC/LLC/DRAM.
                                                                 See NIX_AF_LF()_QINTS_BASE and NIX_AF_LF()_QINTS_CFG. */
		u64 cint_log2bytes:4;	     /**< [ 27: 24](RO) Completion interrupt context size as log2(bytes). Size of each
                                                                 NIX_CINT_HW_S structure in a local function's completion interrupt context
                                                                 table NDC/LLC/DRAM. See NIX_AF_LF()_CINTS_BASE and NIX_AF_LF()_CINTS_CFG. */
		u64 dyno_log2bytes:4;	     /**< [ 31: 28](RO) IPSEC dynamic ordering counter size as log2(bytes). Size of each entry in a
                                                                 local function's dynamic ordering (DYNO) counter table in NDC/LLC/DRAM. See
                                                                 NIX_AF_LF()_RX_IPSEC_DYNO_BASE and NIX_AF_LF()_RX_IPSEC_DYNO_CFG. */
		u64 dyno_array_log2counters:4;  /**< [ 35: 32](RO) IPSEC dynamic ordering counter array size as log2(counters). Size of
                                                                 counter array accessed by the NIX admin queue when NIX_AQ_INST_S[CTYPE] =
                                                                 NIX_AQ_CTYPE_E::DYNO. The size of the array in bytes is
                                                                 1 \<\< ([DYNO_ARRAY_LOG2COUNTERS] + [DYNO_LOG2BYTES]). */
		u64 reserved_36_63:28;
	} s;
	/* struct cavm_nixx_af_const3_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_sq_const
 *
 * NIX AF SQ Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_sq_const {
	u64 u;
	struct cavm_nixx_af_sq_const_s {
		u64 queues_per_lf:24;	     /**< [ 23:  0](RO) Maximum number of send queues per LF. */
		u64 smq_depth:10;		     /**< [ 33: 24](RO) Depth of each SMQ (number of meta descriptors). */
		u64 sqb_size:16;		     /**< [ 49: 34](RO) Number of bytes in each hardware-allocated SQE buffer (SQB) in NDC/LLC/DRAM. */
		u64 reserved_50_63:14;
	} s;
	/* struct cavm_nixx_af_sq_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_cq_const
 *
 * NIX AF CQ Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_cq_const {
	u64 u;
	struct cavm_nixx_af_cq_const_s {
		u64 queues_per_lf:24;	     /**< [ 23:  0](RO) Maximum number of completion queues per LF. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_cq_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rq_const
 *
 * NIX AF RQ Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_rq_const {
	u64 u;
	struct cavm_nixx_af_rq_const_s {
		u64 queues_per_lf:24;	     /**< [ 23:  0](RO) Maximum number of receive queues per LF. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_rq_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_const
 *
 * NIX AF PSE Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_pse_const {
	u64 u;
	struct cavm_nixx_af_pse_const_s {
		u64 levels:4;		     /**< [  3:  0](RO) Number of hierarchical transmit shaping levels. */
		u64 reserved_4_7:4;
		u64 mark_formats:8;	     /**< [ 15:  8](RO) Number of NIX_AF_MARK_FORMAT()_CTL registers. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_pse_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1_const
 *
 * NIX AF Transmit Level 1 Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_tl1_const {
	u64 u;
	struct cavm_nixx_af_tl1_const_s {
		u64 count:16;		     /**< [ 15:  0](RO) Number of transmit level 1 shaping queues. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_tl1_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2_const
 *
 * NIX AF Transmit Level 2 Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_tl2_const {
	u64 u;
	struct cavm_nixx_af_tl2_const_s {
		u64 count:16;		     /**< [ 15:  0](RO) Number of transmit level 2 shaping queues. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_tl2_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_const
 *
 * NIX AF Transmit Level 3 Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_tl3_const {
	u64 u;
	struct cavm_nixx_af_tl3_const_s {
		u64 count:16;		     /**< [ 15:  0](RO) Number of transmit level 3 shaping queues. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_tl3_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4_const
 *
 * NIX AF Transmit Level 4 Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_tl4_const {
	u64 u;
	struct cavm_nixx_af_tl4_const_s {
		u64 count:16;		     /**< [ 15:  0](RO) Number of transmit level 4 shaping queues. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_tl4_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq_const
 *
 * NIX AF Meta Descriptor Queue Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_mdq_const {
	u64 u;
	struct cavm_nixx_af_mdq_const_s {
		u64 count:16;		     /**< [ 15:  0](RO) Number of meta descriptor queues and SMQs. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_mdq_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mc_mirror_const
 *
 * NIX AF Multicast/Mirror Constants Register
 * This register contains constants for software discovery.
 */
union cavm_nixx_af_mc_mirror_const {
	u64 u;
	struct cavm_nixx_af_mc_mirror_const_s {
		u64 buf_size:16;		     /**< [ 15:  0](RO) Size in bytes of multicast/mirror buffers in NDC/LLC/DRAM. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_mc_mirror_const_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_active_cycles_pc
 *
 * NIX AF Active Cycles Register
 */
union cavm_nixx_af_active_cycles_pc {
	u64 u;
	struct cavm_nixx_af_active_cycles_pc_s {
		u64 act_cyc:64;		     /**< [ 63:  0](R/W/H) Counts every coprocessor-clock cycle that the conditional clocks are active. */
	} s;
	/* struct cavm_nixx_af_active_cycles_pc_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lso_cfg
 *
 * NIX AF Large Send Offload Configuration Register
 */
union cavm_nixx_af_lso_cfg {
	u64 u;
	struct cavm_nixx_af_lso_cfg_s {
		u64 tcp_lsf:16;		     /**< [ 15:  0](R/W) TCP last segment flags mask. Same as [TCP_FSF] but used for the last
                                                                 packet/segment of an LSO descriptor. */
		u64 tcp_msf:16;		     /**< [ 31: 16](R/W) TCP middle segment flags mask. Same as [TCP_FSF] but used for LSO
                                                                 packets/segments other than the first and last segments of an LSO
                                                                 descriptor. */
		u64 tcp_fsf:16;		     /**< [ 47: 32](R/W) TCP first segment flags mask. Used for the first packet/segment of an LSO
                                                                 descriptor when NIX_AF_LSO_FORMAT()_FIELD()[ALG] = NIX_LSOALG_E::TCP_FLAGS.
                                                                 The selected 16-bit field, typically consisting of the TCP data offset and
                                                                 TCP flags, is modified as follows:

                                                                 _ FIELD_new = (FIELD_original) AND [TCP_FSF].

                                                                 The upper four bits are typically 0xF in order to keep the original TCP data
                                                                 offset value.

                                                                 If the LSO descriptor only sends one packet/segment, the selected 16-bit
                                                                 field is modified as follows:
                                                                 _ FIELD_new = (FIELD_original) AND ([TCP_FSF] OR [TCP_LSF]). */
		u64 reserved_48_61:14;
		u64 crc_enable:1;		     /**< [ 62: 62](R/W) Enable NIX_SEND_CRC_S with LSO. When clear, NIX ignores NIX_SEND_CRC_S subdescriptors in a
                                                                 send descriptor with NIX_SEND_EXT_S[LSO]=1. */
		u64 enable:1;		     /**< [ 63: 63](R/W) Large send offload enable. When clear, NIX ignores NIX_SEND_EXT_S[LSO] and treats
                                                                 all send descriptors as non-LSO. */
	} s;
	/* struct cavm_nixx_af_lso_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_blk_rst
 *
 * NIX AF Block Reset Register
 */
union cavm_nixx_af_blk_rst {
	u64 u;
	struct cavm_nixx_af_blk_rst_s {
		u64 rst:1;			     /**< [  0:  0](WO/H) Write one to reset the block, except for privileged AF registers in PF BAR0
                                                                 (block_PRIV_*). Software must ensure that all block activity is quiesced before
                                                                 writing 1. */
		u64 reserved_1_62:62;
		u64 busy:1;		     /**< [ 63: 63](RO/H) When one, the block is busy completing reset. No access except the reading of
                                                                 this bit should occur to the block until this is clear. */
	} s;
	/* struct cavm_nixx_af_blk_rst_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_tstmp_cfg
 *
 * NIX AF Transmit Timestamp Configuration Register
 */
union cavm_nixx_af_tx_tstmp_cfg {
	u64 u;
	struct cavm_nixx_af_tx_tstmp_cfg_s {
		u64 tstmp_wd_period:4;	     /**< [  3:  0](R/W) Watchdog timeout count for send timestamp capture. See
                                                                 NIX_SEND_EXT_S[TSTMP] and NIX_SENDMEMALG_E::SETTSTMP.

                                                                 The timeout period is 4*(2^[TSTMP_WD_PERIOD]) timer ticks, where each tick
                                                                 is 128 cycles of the 100 MHz reference clock: 0 = 4 ticks, 1 = 8 ticks, ...
                                                                 15 = 131072 ticks. */
		u64 reserved_4_7:4;
		u64 express:16;		     /**< [ 23:  8](R/W) One bit per CGX LMAC, enumerated by NIX_LINK_E::CGX()_LMAC(). When a bit is
                                                                 set, only express packets to the LMAC are allowed to request PTP
                                                                 timestamps. When a bit is clear, only normal packets to the LMAC are
                                                                 allowed to request PTP timestamps. See NIX_SENDMEMALG_E::SETTSTMP. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_tx_tstmp_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_cfg
 *
 * NIX AF Receive Configuration Register
 */
union cavm_nixx_af_rx_cfg {
	u64 u;
	struct cavm_nixx_af_rx_cfg_s {
		u64 cbp_ena:1;		     /**< [  0:  0](R/W) Channel backpressure enable. Software should set this bit before any
                                                                 NIX_AF_RX_CHAN()_CFG[BP_ENA] is set, and must not clear it unless all
                                                                 NIX_AF_RX_CHAN()_CFG[BP_ENA] are clear for at least 10 microseconds.

                                                                 Internal:
                                                                 Enables sending of X2P backpressure. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_rx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_avg_delay
 *
 * NIX AF Queue Average Delay Register
 */
union cavm_nixx_af_avg_delay {
	u64 u;
	struct cavm_nixx_af_avg_delay_s {
		u64 avg_dly:19;		     /**< [ 18:  0](R/W) Average-queue-size delay. [AVG_DLY]+1 is the number of microseconds per
                                                                 timer tick for calculating the moving average for each CQ level. Note the
                                                                 minimum of one microsecond implies that at 100 M packets/sec, approximately
                                                                 100 packets may arrive between average calculations.

                                                                 Larger [AVG_DLY] causes the moving averages of all CQ levels to track
                                                                 changes in the actual free space more slowly. Larger NIX_CQ_CTX_S[AVG_CON])
                                                                 values causes a specific CQ to track more slowly, but only affects an
                                                                 individual level, rather than all. */
		u64 reserved_19_23:5;
		u64 avg_timer:16;		     /**< [ 39: 24](R/W/H) Running counter that is incremented every [AVG_DLY]+1 microseconds. */
		u64 reserved_40_63:24;
	} s;
	/* struct cavm_nixx_af_avg_delay_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_cint_delay
 *
 * NIX AF Completion Interrupt Delay Register
 */
union cavm_nixx_af_cint_delay {
	u64 u;
	struct cavm_nixx_af_cint_delay_s {
		u64 cint_dly:10;		     /**< [  9:  0](R/W) Completion interrupt timer delay. ([CINT_DLY]+1)*100 is the number
                                                                 of nanoseconds per timer tick for completion interrupts. */
		u64 reserved_10_15:6;
		u64 cint_timer:16;		     /**< [ 31: 16](R/W/H) Completion interrupt timer clock. Running counter that is
                                                                 incremented every ([CINT_DLY]+1)*100 nanoseconds. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_cint_delay_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_base
 *
 * NIX AF Receive Multicast/Mirror Table Base Address Register
 * This register specifies the base AF IOVA of the receive multicast/mirror
 * table in NDC/LLC/DRAM. The table consists of 1 << (NIX_AF_RX_MCAST_CFG[SIZE]+8)
 * contiguous NIX_RX_MCE_S structures. The size of each structure is
 * 1 << NIX_AF_CONST3[MCE_LOG2BYTES] bytes.
 *
 * The table contains multicast/mirror replication lists. Each list consists of
 * linked entries with NIX_RX_MCE_S[EOL] = 1 in the last entry. All lists
 * must reside within the table size specified by NIX_AF_RX_MCAST_CFG[SIZE]. A
 * mirror replication list will typically consist of two entries, but that is not
 * checked or enforced by hardware.
 *
 * A receive packet is multicast when the action returned by NPC has
 * NIX_RX_ACTION_S[OP] = NIX_RX_ACTIONOP_E::MCAST.
 * A receive packet is mirrored when the action returned by NPC has
 * NIX_RX_ACTION_S[OP] = NIX_RX_ACTIONOP_E::MIRROR.
 * In both cases, NIX_RX_ACTION_S[INDEX] specifies the index of the replication
 * list's first NIX_RX_MCE_S in the table, and a linked entry with
 * NIX_RX_MCE_S[EOL] = 1 indicates the end of list.
 *
 * If a mirrored flow is part of a multicast replication list, software should
 * include the two mirror entries in that list.
 *
 * Internal:
 * A multicast list may have multiple entries for the same LF (e.g. for future
 * RoCE/IB multicast).
 */
union cavm_nixx_af_rx_mcast_base {
	u64 u;
	struct cavm_nixx_af_rx_mcast_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_rx_mcast_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_cfg
 *
 * NIX AF Receive Multicast/Mirror Table Configuration Register
 * See NIX_AF_RX_MCAST_BASE.
 */
union cavm_nixx_af_rx_mcast_cfg {
	u64 u;
	struct cavm_nixx_af_rx_mcast_cfg_s {
		u64 size:4;		     /**< [  3:  0](R/W) Specifies table size in NIX_RX_MCE_S entries of eight bytes:
                                                                 0x0 = 256 entries.
                                                                 0x1 = 512 entries.
                                                                 0x2 = 1K entries.
                                                                 0x3 = 2K entries.
                                                                 0x4 = 4K entries.
                                                                 0x5 = 8K entries.
                                                                 0x6 = 16K entries.
                                                                 0x7 = 32K entries.
                                                                 0x8 = 64K entries.
                                                                 0x9-0xF = Reserved. */
		u64 max_list_lenm1:8;	     /**< [ 11:  4](R/W) Maximum list length minus 1. If a multicast or mirror replication list exceeds this
                                                                 length (e.g. due to a loop in the NIX_RX_MCE_S link list), hardware
                                                                 terminates the list and sets NIX_AF_ERR_INT[RX_MCE_LIST_ERR]. */
		u64 reserved_12_19:8;
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating NIX_RX_MCE_S structures in NDC (1
                                                                 means do not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style of read for accessing NIX_RX_MCE_S structures in
                                                                 LLC/DRAM:
                                                                 0 = NIX_RX_MCE_S reads will not allocate into the LLC.
                                                                 1 = NIX_RX_MCE_S reads are allocated into the LLC.

                                                                 NIX_RX_MCE_S writes that are not allocated in NDC will always allocate into
                                                                 LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_rx_mcast_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_buf_base
 *
 * NIX AF Receive Multicast Buffer Base Address Register
 * This register specifies the base AF IOVA of the receive multicast
 * buffers in NDC/LLC/DRAM. These buffers are used to temporarily store packets
 * whose action returned by NPC has NIX_RX_ACTION_S[OP] =
 * NIX_RX_ACTIONOP_E::MCAST. The number of buffers is configured by
 * NIX_AF_RX_MCAST_BUF_CFG[SIZE].
 *
 * If the number of free buffers is insufficient for a received multicast packet,
 * hardware tail drops the packet and sets NIX_AF_GEN_INT[RX_MCAST_DROP].
 *
 * Hardware prioritizes the processing of RX mirror packets over RX multicast
 * packets.
 */
union cavm_nixx_af_rx_mcast_buf_base {
	u64 u;
	struct cavm_nixx_af_rx_mcast_buf_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of buffer in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_rx_mcast_buf_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_buf_cfg
 *
 * NIX AF Receive Multicast Buffer Configuration Register
 * See NIX_AF_RX_MCAST_BUF_BASE.
 */
union cavm_nixx_af_rx_mcast_buf_cfg {
	u64 u;
	struct cavm_nixx_af_rx_mcast_buf_cfg_s {
		u64 size:4;		     /**< [  3:  0](R/W) Total number of buffers of size NIX_AF_MC_MIRROR_CONST[BUF_SIZE]:
                                                                 0x0 = 8 buffers.
                                                                 0x1 = 16 buffers.
                                                                 0x2 = 32 buffers.
                                                                 0x3 = 64 buffers.
                                                                 0x4 = 128 buffers.
                                                                 0x5 = 256 buffers.
                                                                 0x6 = 512 buffers.
                                                                 0x7 = 1024 buffers.
                                                                 0x8 = 2048 buffers.
                                                                 0x9-0xF = Reserved. */
		u64 way_mask:16;		     /**< [ 19:  4](R/W) Way partitioning mask for allocating buffer data in NDC (1 means do not
                                                                 use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 20: 20](R/W) Selects the style of write and read to the LLC.
                                                                 0 = Writes and reads of buffer data will not allocate into the LLC.
                                                                 1 = Writes and reads of buffer data are allocated into the LLC. */
		u64 reserved_21_23:3;
		u64 npc_replay_pkind:6;	     /**< [ 29: 24](R/W) Packet kind used on NPC request interface for all replayed copies of a
                                                                 multicasted packet. The NIX_RX_PARSE_S[PKIND] field for the replayed copies will
                                                                 be the original ingress PKIND. */
		u64 reserved_30_31:2;
		u64 free_buf_level:11;	     /**< [ 42: 32](RO/H) Free buffer level. Number of available free buffers (out of the total
                                                                 [SIZE]). */
		u64 reserved_43_62:20;
		u64 ena:1;			     /**< [ 63: 63](R/W) Multicast buffer enable.
                                                                 0 = All new incoming MC packets will get discarded. Software should
                                                                 wait until all MC packets in flight are played out before re-enabling [ENA].
                                                                 1 = The temporary memory defined in NIX_AF_RX_MCAST_BUF_CFG[SIZE] is
                                                                 divided into equal size buffers as defined by NIX_AF_MC_MIRROR_CONST[BUF_SIZE]. */
	} s;
	/* struct cavm_nixx_af_rx_mcast_buf_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mirror_buf_base
 *
 * NIX AF Receive Mirror Buffer Base Address Register
 * This register specifies the base AF IOVA of the receive mirror buffers
 * in NDC/LLC/DRAM. These buffers are used to temporarily store packets whose
 * action returned by NPC has NIX_RX_ACTION_S[OP] = NIX_RX_ACTIONOP_E::MIRROR. The
 * number of buffers is configured by NIX_AF_RX_MIRROR_BUF_CFG[SIZE].
 *
 * If the number of free buffers is insufficient for a received multicast packet,
 * hardware tail drops the packet and sets NIX_AF_GEN_INT[RX_MIRROR_DROP].
 *
 * Hardware prioritizes the processing of RX mirror packets over RX multicast
 * packets.
 */
union cavm_nixx_af_rx_mirror_buf_base {
	u64 u;
	struct cavm_nixx_af_rx_mirror_buf_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of buffer in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_rx_mirror_buf_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mirror_buf_cfg
 *
 * NIX AF Receive Mirror Buffer Configuration Register
 * See NIX_AF_RX_MIRROR_BUF_BASE.
 */
union cavm_nixx_af_rx_mirror_buf_cfg {
	u64 u;
	struct cavm_nixx_af_rx_mirror_buf_cfg_s {
		u64 size:4;		     /**< [  3:  0](R/W) Total number of buffers of size NIX_AF_MC_MIRROR_CONST[BUF_SIZE]:
                                                                 0x0 = 8 buffers.
                                                                 0x1 = 16 buffers.
                                                                 0x2 = 32 buffers.
                                                                 0x3 = 64 buffers.
                                                                 0x4 = 128 buffers.
                                                                 0x5 = 256 buffers.
                                                                 0x6 = 512 buffers.
                                                                 0x7 = 1024 buffers.
                                                                 0x8 = 2048 buffers.
                                                                 0x9-0xF = Reserved. */
		u64 way_mask:16;		     /**< [ 19:  4](R/W) Way partitioning mask for allocating buffer data in NDC (1 means do not
                                                                 use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 20: 20](R/W) Selects the style of write and read to the LLC.
                                                                 0 = Writes and reads of buffer data will not allocate into the LLC.
                                                                 1 = Writes and reads of buffer data are allocated into the LLC. */
		u64 reserved_21_23:3;
		u64 npc_replay_pkind:6;	     /**< [ 29: 24](R/W) Packet kind used on NPC request interface for all replayed copies of a
                                                                 multicasted packet. The NIX_RX_PARSE_S[PKIND] field for the replayed copies will
                                                                 be the original ingress PKIND. */
		u64 reserved_30_31:2;
		u64 free_buf_level:11;	     /**< [ 42: 32](RO/H) Free buffer level. Number of available free buffers (out of the total
                                                                 [SIZE]). */
		u64 reserved_43_62:20;
		u64 ena:1;			     /**< [ 63: 63](R/W) Multicast buffer enable.
                                                                 0 = All new incoming MC packets will get discarded. Software should
                                                                 wait until all MC packets in flight are played out before re-enabling [ENA].
                                                                 1 = The temporary memory defined in NIX_AF_RX_MCAST_BUF_CFG[SIZE] is
                                                                 divided into equal size buffers as defined by NIX_AF_MC_MIRROR_CONST[BUF_SIZE]. */
	} s;
	/* struct cavm_nixx_af_rx_mirror_buf_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf_rst
 *
 * NIX Admin Function LF Soft Reset Register
 */
union cavm_nixx_af_lf_rst {
	u64 u;
	struct cavm_nixx_af_lf_rst_s {
		u64 lf:8;			     /**< [  7:  0](R/W) Local function that is reset when [EXEC] is set. */
		u64 reserved_8_11:4;
		u64 exec:1;		     /**< [ 12: 12](R/W1S/H) Execute LF soft reset. When software writes a one to set this bit, hardware
                                                                 resets the local function selected by [LF]. Hardware clears this bit when
                                                                 done.

                                                                 Internal:
                                                                 This comment applies to all blocks that refer to this register:

                                                                 This should preferrably reset all registers/state associated with the LF, including
                                                                 any BLK_LF_* and BLK_AF_LF()_* registers. It would also be nice to reset any per-LF
                                                                 bits in other registers but its OK to have exceptions as long as the AF software has
                                                                 another way to reset them, e.g. by writing to the bits. Such additional steps
                                                                 expected from software should be documented in the HRM, e.g. in section 19.11.5
                                                                 "VF Function Level Reset". */
		u64 reserved_13_63:51;
	} s;
	/* struct cavm_nixx_af_lf_rst_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int
 *
 * NIX AF General Interrupt Register
 */
union cavm_nixx_af_gen_int {
	u64 u;
	struct cavm_nixx_af_gen_int_s {
		u64 rx_mcast_drop:1;	     /**< [  0:  0](R/W1C/H) Receive multicast packet dropped due to insufficient space in the RX
                                                                 multicast buffer specified by NIX_AF_RX_MCAST_BUF_BASE and
                                                                 NIX_AF_RX_MCAST_BUF_CFG. */
		u64 rx_mirror_drop:1;	     /**< [  1:  1](R/W1C/H) Receive mirror packet dropped due to insufficient space in the RX mirror
                                                                 buffer specified by NIX_AF_RX_MIRROR_BUF_BASE and NIX_AF_RX_MIRROR_BUF_CFG. */
		u64 reserved_2:1;
		u64 tl1_drain:1;		     /**< [  3:  3](R/W1C/H) Set when a NIX_AF_MDQ()_SW_XOFF[DRAIN,DRAIN_IRQ] or
                                                                 NIX_AF_TL*()_SW_XOFF[DRAIN,DRAIN_IRQ] command reaches the TL1 level. */
		u64 smq_flush_done:1;	     /**< [  4:  4](R/W1C/H) SMQ flush done. Set when an SMQ flush operation initiated by
                                                                 NIX_AF_SMQ()_CFG[FLUSH] is complete. */
		u64 reserved_5_63:59;
	} s;
	/* struct cavm_nixx_af_gen_int_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int_w1s
 *
 * NIX AF General Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_nixx_af_gen_int_w1s {
	u64 u;
	struct cavm_nixx_af_gen_int_w1s_s {
		u64 rx_mcast_drop:1;	     /**< [  0:  0](R/W1S/H) Reads or sets NIX_AF_GEN_INT[RX_MCAST_DROP]. */
		u64 rx_mirror_drop:1;	     /**< [  1:  1](R/W1S/H) Reads or sets NIX_AF_GEN_INT[RX_MIRROR_DROP]. */
		u64 reserved_2:1;
		u64 tl1_drain:1;		     /**< [  3:  3](R/W1S/H) Reads or sets NIX_AF_GEN_INT[TL1_DRAIN]. */
		u64 smq_flush_done:1;	     /**< [  4:  4](R/W1S/H) Reads or sets NIX_AF_GEN_INT[SMQ_FLUSH_DONE]. */
		u64 reserved_5_63:59;
	} s;
	/* struct cavm_nixx_af_gen_int_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int_ena_w1s
 *
 * NIX AF General Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_af_gen_int_ena_w1s {
	u64 u;
	struct cavm_nixx_af_gen_int_ena_w1s_s {
		u64 rx_mcast_drop:1;	     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_AF_GEN_INT[RX_MCAST_DROP]. */
		u64 rx_mirror_drop:1;	     /**< [  1:  1](R/W1S/H) Reads or sets enable for NIX_AF_GEN_INT[RX_MIRROR_DROP]. */
		u64 reserved_2:1;
		u64 tl1_drain:1;		     /**< [  3:  3](R/W1S/H) Reads or sets enable for NIX_AF_GEN_INT[TL1_DRAIN]. */
		u64 smq_flush_done:1;	     /**< [  4:  4](R/W1S/H) Reads or sets enable for NIX_AF_GEN_INT[SMQ_FLUSH_DONE]. */
		u64 reserved_5_63:59;
	} s;
	/* struct cavm_nixx_af_gen_int_ena_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int_ena_w1c
 *
 * NIX AF General Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_af_gen_int_ena_w1c {
	u64 u;
	struct cavm_nixx_af_gen_int_ena_w1c_s {
		u64 rx_mcast_drop:1;	     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_AF_GEN_INT[RX_MCAST_DROP]. */
		u64 rx_mirror_drop:1;	     /**< [  1:  1](R/W1C/H) Reads or clears enable for NIX_AF_GEN_INT[RX_MIRROR_DROP]. */
		u64 reserved_2:1;
		u64 tl1_drain:1;		     /**< [  3:  3](R/W1C/H) Reads or clears enable for NIX_AF_GEN_INT[TL1_DRAIN]. */
		u64 smq_flush_done:1;	     /**< [  4:  4](R/W1C/H) Reads or clears enable for NIX_AF_GEN_INT[SMQ_FLUSH_DONE]. */
		u64 reserved_5_63:59;
	} s;
	/* struct cavm_nixx_af_gen_int_ena_w1c_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int
 *
 * NIX Admin Function Error Interrupt Register
 */
union cavm_nixx_af_err_int {
	u64 u;
	struct cavm_nixx_af_err_int_s {
		u64 rx_mcast_data_fault:1;	     /**< [  0:  0](R/W1C/H) Memory fault on packet data or WQE write to a multicast buffer. */
		u64 rx_mirror_data_fault:1;     /**< [  1:  1](R/W1C/H) Memory fault on packet data or WQE write to a mirror buffer. */
		u64 rx_mcast_wqe_fault:1;	     /**< [  2:  2](R/W1C/H) Memory fault on WQE read from a mirror buffer. */
		u64 rx_mirror_wqe_fault:1;	     /**< [  3:  3](R/W1C/H) Memory fault on WQE read from a multicast buffer. */
		u64 rx_mce_fault:1;	     /**< [  4:  4](R/W1C/H) Memory fault on NIX_RX_MCE_S read. */
		u64 rx_mce_list_err:1;	     /**< [  5:  5](R/W1C/H) Receive multicast/mirror replication list error. One of the following (may
                                                                 not be exhaustive):
                                                                 * The length of a multicast or mirror replication list exceeds
                                                                 NIX_AF_RX_MCAST_CFG[MAX_LIST_LENM1]+1.
                                                                 * Invalid opcode in NIX_RX_MCE_S[OP].
                                                                 * NIX_RX_MCE_S[NEXT] pointer outside of table size specified by
                                                                 NIX_AF_RX_MCAST_CFG[SIZE]. */
		u64 reserved_6_11:6;
		u64 aq_door_err:1;		     /**< [ 12: 12](R/W1C/H) AQ doorbell error. See NIX_AF_AQ_DOOR[COUNT]. Hardware also sets
                                                                 NIX_AF_AQ_STATUS[AQ_ERR]. */
		u64 aq_res_fault:1;	     /**< [ 13: 13](R/W1C/H) Memory fault on NIX_AQ_RES_S write, or on read/write data following
                                                                 NIX_AQ_RES_S. Hardware also sets NIX_AF_AQ_STATUS[AQ_ERR]. */
		u64 aq_inst_fault:1;	     /**< [ 14: 14](R/W1C/H) Memory fault on NIX_AQ_INST_S read. Hardware also sets
                                                                 NIX_AF_AQ_STATUS[AQ_ERR]. */
		u64 reserved_15_63:49;
	} s;
	/* struct cavm_nixx_af_err_int_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int_w1s
 *
 * NIX Admin Function Error Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_nixx_af_err_int_w1s {
	u64 u;
	struct cavm_nixx_af_err_int_w1s_s {
		u64 rx_mcast_data_fault:1;	     /**< [  0:  0](R/W1S/H) Reads or sets NIX_AF_ERR_INT[RX_MCAST_DATA_FAULT]. */
		u64 rx_mirror_data_fault:1;     /**< [  1:  1](R/W1S/H) Reads or sets NIX_AF_ERR_INT[RX_MIRROR_DATA_FAULT]. */
		u64 rx_mcast_wqe_fault:1;	     /**< [  2:  2](R/W1S/H) Reads or sets NIX_AF_ERR_INT[RX_MCAST_WQE_FAULT]. */
		u64 rx_mirror_wqe_fault:1;	     /**< [  3:  3](R/W1S/H) Reads or sets NIX_AF_ERR_INT[RX_MIRROR_WQE_FAULT]. */
		u64 rx_mce_fault:1;	     /**< [  4:  4](R/W1S/H) Reads or sets NIX_AF_ERR_INT[RX_MCE_FAULT]. */
		u64 rx_mce_list_err:1;	     /**< [  5:  5](R/W1S/H) Reads or sets NIX_AF_ERR_INT[RX_MCE_LIST_ERR]. */
		u64 reserved_6_11:6;
		u64 aq_door_err:1;		     /**< [ 12: 12](R/W1S/H) Reads or sets NIX_AF_ERR_INT[AQ_DOOR_ERR]. */
		u64 aq_res_fault:1;	     /**< [ 13: 13](R/W1S/H) Reads or sets NIX_AF_ERR_INT[AQ_RES_FAULT]. */
		u64 aq_inst_fault:1;	     /**< [ 14: 14](R/W1S/H) Reads or sets NIX_AF_ERR_INT[AQ_INST_FAULT]. */
		u64 reserved_15_63:49;
	} s;
	/* struct cavm_nixx_af_err_int_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int_ena_w1s
 *
 * NIX Admin Function Error Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_af_err_int_ena_w1s {
	u64 u;
	struct cavm_nixx_af_err_int_ena_w1s_s {
		u64 rx_mcast_data_fault:1;	     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[RX_MCAST_DATA_FAULT]. */
		u64 rx_mirror_data_fault:1;     /**< [  1:  1](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[RX_MIRROR_DATA_FAULT]. */
		u64 rx_mcast_wqe_fault:1;	     /**< [  2:  2](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[RX_MCAST_WQE_FAULT]. */
		u64 rx_mirror_wqe_fault:1;	     /**< [  3:  3](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[RX_MIRROR_WQE_FAULT]. */
		u64 rx_mce_fault:1;	     /**< [  4:  4](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[RX_MCE_FAULT]. */
		u64 rx_mce_list_err:1;	     /**< [  5:  5](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[RX_MCE_LIST_ERR]. */
		u64 reserved_6_11:6;
		u64 aq_door_err:1;		     /**< [ 12: 12](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[AQ_DOOR_ERR]. */
		u64 aq_res_fault:1;	     /**< [ 13: 13](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[AQ_RES_FAULT]. */
		u64 aq_inst_fault:1;	     /**< [ 14: 14](R/W1S/H) Reads or sets enable for NIX_AF_ERR_INT[AQ_INST_FAULT]. */
		u64 reserved_15_63:49;
	} s;
	/* struct cavm_nixx_af_err_int_ena_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int_ena_w1c
 *
 * NIX Admin Function Error Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_af_err_int_ena_w1c {
	u64 u;
	struct cavm_nixx_af_err_int_ena_w1c_s {
		u64 rx_mcast_data_fault:1;	     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[RX_MCAST_DATA_FAULT]. */
		u64 rx_mirror_data_fault:1;     /**< [  1:  1](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[RX_MIRROR_DATA_FAULT]. */
		u64 rx_mcast_wqe_fault:1;	     /**< [  2:  2](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[RX_MCAST_WQE_FAULT]. */
		u64 rx_mirror_wqe_fault:1;	     /**< [  3:  3](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[RX_MIRROR_WQE_FAULT]. */
		u64 rx_mce_fault:1;	     /**< [  4:  4](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[RX_MCE_FAULT]. */
		u64 rx_mce_list_err:1;	     /**< [  5:  5](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[RX_MCE_LIST_ERR]. */
		u64 reserved_6_11:6;
		u64 aq_door_err:1;		     /**< [ 12: 12](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[AQ_DOOR_ERR]. */
		u64 aq_res_fault:1;	     /**< [ 13: 13](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[AQ_RES_FAULT]. */
		u64 aq_inst_fault:1;	     /**< [ 14: 14](R/W1C/H) Reads or clears enable for NIX_AF_ERR_INT[AQ_INST_FAULT]. */
		u64 reserved_15_63:49;
	} s;
	/* struct cavm_nixx_af_err_int_ena_w1c_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_ras
 *
 * NIX AF RAS Interrupt Register
 * This register is intended for delivery of RAS events to the SCP, so should be
 * ignored by OS drivers.
 */
union cavm_nixx_af_ras {
	u64 u;
	struct cavm_nixx_af_ras_s {
		u64 rx_mce_poison:1;	     /**< [  0:  0](R/W1C/H) Poisoned data returned on NIX_RX_MCE_S read. */
		u64 rx_mcast_wqe_poison:1;	     /**< [  1:  1](R/W1C/H) Poisoned data returned on WQE read from a multicast buffer. */
		u64 rx_mirror_wqe_poison:1;     /**< [  2:  2](R/W1C/H) Poisoned data returned on WQE read from a mirror buffer. */
		u64 rx_mcast_data_poison:1;     /**< [  3:  3](R/W1C/H) Poisoned data returned on packet data read from a multicast buffer. */
		u64 rx_mirror_data_poison:1;    /**< [  4:  4](R/W1C/H) Poisoned data returned on packet data read from a mirror buffer. */
		u64 reserved_5_31:27;
		u64 aq_ctx_poison:1;	     /**< [ 32: 32](R/W1C/H) Poisoned data returned on read of hardware context data selected by
                                                                 NIX_AQ_INST_S[LF,CTYPE,CINDEX]. If NIX_AF_NDC_CFG[NDC_IGN_POIS]=0, hardware
                                                                 also returns NIX_AQ_RES_S[COMPCODE] = NIX_AQ_COMP_E::CTX_POISON. */
		u64 aq_res_poison:1;	     /**< [ 33: 33](R/W1C/H) Poisoned read data returned following NIX_AQ_RES_S. If
                                                                 NIX_AF_NDC_CFG[NDC_IGN_POIS]=0, hardware also sets
                                                                 NIX_AF_AQ_STATUS[AQ_ERR]. */
		u64 aq_inst_poison:1;	     /**< [ 34: 34](R/W1C/H) Poisoned data returned on NIX_AQ_INST_S read. If
                                                                 NIX_AF_NDC_CFG[NDC_IGN_POIS]=0, hardware also sets
                                                                 NIX_AF_AQ_STATUS[AQ_ERR]. */
		u64 reserved_35_63:29;
	} s;
	/* struct cavm_nixx_af_ras_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_ras_w1s
 *
 * NIX AF RAS Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_nixx_af_ras_w1s {
	u64 u;
	struct cavm_nixx_af_ras_w1s_s {
		u64 rx_mce_poison:1;	     /**< [  0:  0](R/W1S/H) Reads or sets NIX_AF_RAS[RX_MCE_POISON]. */
		u64 rx_mcast_wqe_poison:1;	     /**< [  1:  1](R/W1S/H) Reads or sets NIX_AF_RAS[RX_MCAST_WQE_POISON]. */
		u64 rx_mirror_wqe_poison:1;     /**< [  2:  2](R/W1S/H) Reads or sets NIX_AF_RAS[RX_MIRROR_WQE_POISON]. */
		u64 rx_mcast_data_poison:1;     /**< [  3:  3](R/W1S/H) Reads or sets NIX_AF_RAS[RX_MCAST_DATA_POISON]. */
		u64 rx_mirror_data_poison:1;    /**< [  4:  4](R/W1S/H) Reads or sets NIX_AF_RAS[RX_MIRROR_DATA_POISON]. */
		u64 reserved_5_31:27;
		u64 aq_ctx_poison:1;	     /**< [ 32: 32](R/W1S/H) Reads or sets NIX_AF_RAS[AQ_CTX_POISON]. */
		u64 aq_res_poison:1;	     /**< [ 33: 33](R/W1S/H) Reads or sets NIX_AF_RAS[AQ_RES_POISON]. */
		u64 aq_inst_poison:1;	     /**< [ 34: 34](R/W1S/H) Reads or sets NIX_AF_RAS[AQ_INST_POISON]. */
		u64 reserved_35_63:29;
	} s;
	/* struct cavm_nixx_af_ras_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_ena_w1s
 *
 * NIX LF RAS Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_lf_ras_ena_w1s {
	u64 u;
	struct cavm_nixx_lf_ras_ena_w1s_s {
		u64 sqb_poison:1;		     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SQB_POISON]. */
		u64 sq_ctx_poison:1;	     /**< [  1:  1](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SQ_CTX_POISON]. */
		u64 rq_ctx_poison:1;	     /**< [  2:  2](R/W1S/H) Reads or sets enable for NIX_LF_RAS[RQ_CTX_POISON]. */
		u64 cq_ctx_poison:1;	     /**< [  3:  3](R/W1S/H) Reads or sets enable for NIX_LF_RAS[CQ_CTX_POISON]. */
		u64 reserved_4:1;
		u64 rsse_poison:1;		     /**< [  5:  5](R/W1S/H) Reads or sets enable for NIX_LF_RAS[RSSE_POISON]. */
		u64 ipsec_dyno_poison:1;	     /**< [  6:  6](R/W1S/H) Reads or sets enable for NIX_LF_RAS[IPSEC_DYNO_POISON]. */
		u64 send_jump_poison:1;	     /**< [  7:  7](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SEND_JUMP_POISON]. */
		u64 send_sg_poison:1;	     /**< [  8:  8](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SEND_SG_POISON]. */
		u64 qint_poison:1;		     /**< [  9:  9](R/W1S/H) Reads or sets enable for NIX_LF_RAS[QINT_POISON]. */
		u64 cint_poison:1;		     /**< [ 10: 10](R/W1S/H) Reads or sets enable for NIX_LF_RAS[CINT_POISON]. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_lf_ras_ena_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_ena_w1c
 *
 * NIX LF RAS Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_lf_ras_ena_w1c {
	u64 u;
	struct cavm_nixx_lf_ras_ena_w1c_s {
		u64 sqb_poison:1;		     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SQB_POISON]. */
		u64 sq_ctx_poison:1;	     /**< [  1:  1](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SQ_CTX_POISON]. */
		u64 rq_ctx_poison:1;	     /**< [  2:  2](R/W1C/H) Reads or clears enable for NIX_LF_RAS[RQ_CTX_POISON]. */
		u64 cq_ctx_poison:1;	     /**< [  3:  3](R/W1C/H) Reads or clears enable for NIX_LF_RAS[CQ_CTX_POISON]. */
		u64 reserved_4:1;
		u64 rsse_poison:1;		     /**< [  5:  5](R/W1C/H) Reads or clears enable for NIX_LF_RAS[RSSE_POISON]. */
		u64 ipsec_dyno_poison:1;	     /**< [  6:  6](R/W1C/H) Reads or clears enable for NIX_LF_RAS[IPSEC_DYNO_POISON]. */
		u64 send_jump_poison:1;	     /**< [  7:  7](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SEND_JUMP_POISON]. */
		u64 send_sg_poison:1;	     /**< [  8:  8](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SEND_SG_POISON]. */
		u64 qint_poison:1;		     /**< [  9:  9](R/W1C/H) Reads or clears enable for NIX_LF_RAS[QINT_POISON]. */
		u64 cint_poison:1;		     /**< [ 10: 10](R/W1C/H) Reads or clears enable for NIX_LF_RAS[CINT_POISON]. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_lf_ras_ena_w1c_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int
 *
 * NIX AF RVU Interrupt Register
 * This register contains RVU error interrupt summary bits.
 */
union cavm_nixx_af_rvu_int {
	u64 u;
	struct cavm_nixx_af_rvu_int_s {
		u64 unmapped_slot:1;	     /**< [  0:  0](R/W1C/H) Unmapped slot. Received an IO request to a VF/PF slot in BAR2 that is not
                                                                 reverse mapped to an LF. See NIX_PRIV_LF()_CFG and NIX_AF_RVU_LF_CFG_DEBUG. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_rvu_int_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int_w1s
 *
 * NIX AF RVU Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_nixx_af_rvu_int_w1s {
	u64 u;
	struct cavm_nixx_af_rvu_int_w1s_s {
		u64 unmapped_slot:1;	     /**< [  0:  0](R/W1S/H) Reads or sets NIX_AF_RVU_INT[UNMAPPED_SLOT]. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_rvu_int_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int_ena_w1s
 *
 * NIX AF RVU Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_af_rvu_int_ena_w1s {
	u64 u;
	struct cavm_nixx_af_rvu_int_ena_w1s_s {
		u64 unmapped_slot:1;	     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_AF_RVU_INT[UNMAPPED_SLOT]. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_rvu_int_ena_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int_ena_w1c
 *
 * NIX AF RVU Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_af_rvu_int_ena_w1c {
	u64 u;
	struct cavm_nixx_af_rvu_int_ena_w1c_s {
		u64 unmapped_slot:1;	     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_AF_RVU_INT[UNMAPPED_SLOT]. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_rvu_int_ena_w1c_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tcp_timer
 *
 * NIX TCP Timer Register
 */
union cavm_nixx_af_tcp_timer {
	u64 u;
	struct cavm_nixx_af_tcp_timer_s {
		u64 dur_counter:16;	     /**< [ 15:  0](RO/H) Periodic counter that wraps around every [DURATION]*100 nanoseconds.
                                                                 Enabled when [ENA] is set. */
		u64 lf_counter:8;		     /**< [ 23: 16](RO/H) LF Counter. Updated when [DUR_COUNTER] wraps around to select the next LF
                                                                 for which a timer event is generated. Cycles through values 0 through
                                                                 NIX_AF_CONST2[LFS]-1. */
		u64 reserved_24_31:8;
		u64 duration:16;		     /**< [ 47: 32](R/W) Duration of the TCP timer as a multiple of 100 nanoseconds. */
		u64 reserved_48_62:15;
		u64 ena:1;			     /**< [ 63: 63](R/W) TCP timer enable. When clear, [LF_COUNTER] and [DUR_COUNTER] are forced to
                                                                 0 and timer events will not be posted. */
	} s;
	/* struct cavm_nixx_af_tcp_timer_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_ol2
 *
 * NIX AF Receive Outer L2 Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an outer L2/Ethernet
 * header. Typically the same as NPC_PCK_DEF_OL2.
 */
union cavm_nixx_af_rx_def_ol2 {
	u64 u;
	struct cavm_nixx_af_rx_def_ol2_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_ol2_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_oip4
 *
 * NIX AF Receive Outer IPv4 Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an outer IPv4 L3 header.
 * Typically the same as NPC_PCK_DEF_OIP4.
 */
union cavm_nixx_af_rx_def_oip4 {
	u64 u;
	struct cavm_nixx_af_rx_def_oip4_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_oip4_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_iip4
 *
 * NIX AF Receive Inner IPv4 Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an inner IPv4 header.
 * Typically the same as NPC_PCK_DEF_IIP4.
 */
union cavm_nixx_af_rx_def_iip4 {
	u64 u;
	struct cavm_nixx_af_rx_def_iip4_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_iip4_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_oip6
 *
 * NIX AF Receive Outer IPv6 Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an outer IPv6 header.
 * Typically the same as NPC_PCK_DEF_OIP6.
 */
union cavm_nixx_af_rx_def_oip6 {
	u64 u;
	struct cavm_nixx_af_rx_def_oip6_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_oip6_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_iip6
 *
 * NIX AF Receive Inner IPv6 Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an inner IPv6 header.
 */
union cavm_nixx_af_rx_def_iip6 {
	u64 u;
	struct cavm_nixx_af_rx_def_iip6_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_iip6_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_otcp
 *
 * NIX AF Receive Outer TCP Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an outer TCP header.
 */
union cavm_nixx_af_rx_def_otcp {
	u64 u;
	struct cavm_nixx_af_rx_def_otcp_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_otcp_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_itcp
 *
 * NIX AF Receive Inner TCP Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an inner TCP header.
 */
union cavm_nixx_af_rx_def_itcp {
	u64 u;
	struct cavm_nixx_af_rx_def_itcp_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_itcp_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_oudp
 *
 * NIX AF Receive Outer UDP Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an outer UDP header.
 */
union cavm_nixx_af_rx_def_oudp {
	u64 u;
	struct cavm_nixx_af_rx_def_oudp_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_oudp_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_iudp
 *
 * NIX AF Receive Inner UDP Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an inner UDP header.
 */
union cavm_nixx_af_rx_def_iudp {
	u64 u;
	struct cavm_nixx_af_rx_def_iudp_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_iudp_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_osctp
 *
 * NIX AF Receive Outer SCTP Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an outer SCTP header.
 */
union cavm_nixx_af_rx_def_osctp {
	u64 u;
	struct cavm_nixx_af_rx_def_osctp_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_osctp_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_isctp
 *
 * NIX AF Receive Inner SCTP Header Definition Register
 * Defines layer information in NPC_RESULT_S to identify an inner SCTP header.
 */
union cavm_nixx_af_rx_def_isctp {
	u64 u;
	struct cavm_nixx_af_rx_def_isctp_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LH as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_af_rx_def_isctp_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_ipsec#
 *
 * NIX AF Receive IPSEC Header Definition Registers
 * These two registers define layer information in NPC_RESULT_S to identify an
 * IPSEC header for up to two IPSEC packet formats. The two formats are typically
 * IPSEC ESP (RFC 4303) and UDP-encapsulated IPSEC ESP (RFC 3948).
 */
union cavm_nixx_af_rx_def_ipsecx {
	u64 u;
	struct cavm_nixx_af_rx_def_ipsecx_s {
		u64 ltype_mask:4;		     /**< [  3:  0](R/W) Layer type mask. See [LTYPE_MATCH]. */
		u64 ltype_match:4;		     /**< [  7:  4](R/W) Layer type match value. Hardware detects a layer match when
                                                                 \<pre\>
                                                                 ([LTYPE_MATCH] & [LTYPE_MASK]) == (NPC_RESULT_S[LX[LTYPE]] & [LTYPE_MASK])
                                                                 \</pre\>

                                                                 where LX is one of LA, LB, ..., LG as selected by [LID]. */
		u64 lid:3;			     /**< [ 10:  8](R/W) Layer ID. Enumerated by NPC_LID_E. */
		u64 reserved_11:1;
		u64 spi_offset:4;		     /**< [ 15: 12](R/W) SPI offset. Starting byte offset of IPSEC SPI field relative to the start
                                                                 of the IPSEC header identifies by [LID], [LTYPE_MATCH] and [LTYPE_MASK]. */
		u64 spi_nz:1;		     /**< [ 16: 16](R/W) SPI non-zero. When set, match only is the IPSEC SPI field in the packet is non-zero. */
		u64 reserved_17_63:47;
	} s;
	/* struct cavm_nixx_af_rx_def_ipsecx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_ipsec_gen_cfg
 *
 * NIX AF Receive IPSEC General Configuration Register
 * This register specifie the values of certain fields in CPT instructions
 * (CPT_INST_S) generated by NIX for IPSEC hardware fast-path packets.
 */
union cavm_nixx_af_rx_ipsec_gen_cfg {
	u64 u;
	struct cavm_nixx_af_rx_ipsec_gen_cfg_s {
		u64 param2:16;		     /**< [ 15:  0](R/W) CPT_INST_S[PARAM2] value. */
		u64 param1:16;		     /**< [ 31: 16](R/W) CPT_INST_S[PARAM1] value. */
		u64 opcode:16;		     /**< [ 47: 32](R/W) CPT_INST_S[OPCODE] value. */
		u64 egrp:3;		     /**< [ 50: 48](R/W) CPT_INST_S[EGRP] value. */
		u64 reserved_51_63:13;
	} s;
	/* struct cavm_nixx_af_rx_ipsec_gen_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_cpt#_inst_qsel
 *
 * NIX AF Receive CPT Instruction Queue Select Register
 * Selects the CPT queue to which instructions (CPT_INST_S) are sent.
 * Internal:
 * NIX sends CPT_INST_S to the CPT_LF_NQ() physical address for [PF_FUNC] and [SLOT]:
 * \<pre\>
 * // CPT_LF_NQ() physical address:
 * chip_pa_defs::io_rvu2a_t cpt_addr;
 * cpt_addr = RVU_BAR_E::RVU_PF()_FUNC()_BAR2(pf, func);
 * cpt_addr.block = RVU_BLOCK_ADDR_E::CPT()({a}); // {a} = CPT index
 * cpt_addr.slot = [SLOT];
 * cpt_addr.offset = `CPT_LF_NQX__BASE;
 *
 * // NDC/NCBI command:
 * ncbi_cmd.paddr = 1; // Physical address
 * ncbi_cmd.addr = cpt_addr;
 * \</pre\>
 */
union cavm_nixx_af_rx_cptx_inst_qsel {
	u64 u;
	struct cavm_nixx_af_rx_cptx_inst_qsel_s {
		u64 slot:8;		     /**< [  7:  0](R/W) CPT queue's slot within [PF_FUNC]; */
		u64 pf_func:16;		     /**< [ 23:  8](R/W) RVU PF and function of the CPT queue. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_rx_cptx_inst_qsel_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_cfg
 *
 * NIX AF Admin Queue Configuration Register
 */
union cavm_nixx_af_aq_cfg {
	u64 u;
	struct cavm_nixx_af_aq_cfg_s {
		u64 qsize:4;		     /**< [  3:  0](R/W) Specifies AQ ring size in entries of 16 bytes:
                                                                 0x0 = 16 entries.
                                                                 0x1 = 64 entries.
                                                                 0x2 = 256 entries.
                                                                 0x3 = 1K entries.
                                                                 0x4 = 4K entries.
                                                                 0x5 = 16K entries.
                                                                 0x6 = 64K entries.
                                                                 0x7 = 256K entries.
                                                                 0x8 = 1M entries.
                                                                 0x9-0xF = Reserved.

                                                                 Note that the usable size of the ring is the specified size minus 1 (HEAD==TAIL always
                                                                 means empty). */
		u64 reserved_4_63:60;
	} s;
	/* struct cavm_nixx_af_aq_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_base
 *
 * NIX AF Admin Queue Base Address Register
 */
union cavm_nixx_af_aq_base {
	u64 u;
	struct cavm_nixx_af_aq_base_s {
		u64 reserved_0_6:7;
		u64 base_addr:46;		     /**< [ 52:  7](R/W) AF IOVA\<52:7\> of AQ ring in LLC/DRAM. IOVA bits \<6:0\> are always zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_aq_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_status
 *
 * NIX AF Admin Queue Status Register
 */
union cavm_nixx_af_aq_status {
	u64 u;
	struct cavm_nixx_af_aq_status_s {
		u64 reserved_0_3:4;
		u64 head_ptr:20;		     /**< [ 23:  4](R/W/H) Head pointer \<24:4\> of AQ ring relative to NIX_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the head pointer when it pops an entry
                                                                 from the AQ. */
		u64 reserved_24_35:12;
		u64 tail_ptr:20;		     /**< [ 55: 36](R/W/H) Tail pointer \<24:4\> of AQ ring relative to NIX_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the tail pointer when software writes to
                                                                 NIX_AF_AQ_DOOR. */
		u64 reserved_56_61:6;
		u64 aq_busy:1;		     /**< [ 62: 62](RO/H) This bit is set when an AQ command is currently being processed. */
		u64 aq_err:1;		     /**< [ 63: 63](R/W1C/H) AQ error. See NIX_AF_ERR_INT[AQ_INST_FAULT,AQ_RES_FAULT,AQ_DOOR_ERR] and
                                                                 NIX_AF_RAS[AQ_INST_POISON,AQ_RES_POISON].
                                                                 When set, hardware stops reading instructions from the AQ ring. Software
                                                                 clears the error by writing a one back. */
	} s;
	struct cavm_nixx_af_aq_status_cn {
		u64 reserved_0_3:4;
		u64 head_ptr:20;		     /**< [ 23:  4](R/W/H) Head pointer \<24:4\> of AQ ring relative to NIX_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the head pointer when it pops an entry
                                                                 from the AQ. */
		u64 reserved_24_31:8;
		u64 reserved_32_35:4;
		u64 tail_ptr:20;		     /**< [ 55: 36](R/W/H) Tail pointer \<24:4\> of AQ ring relative to NIX_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the tail pointer when software writes to
                                                                 NIX_AF_AQ_DOOR. */
		u64 reserved_56_61:6;
		u64 aq_busy:1;		     /**< [ 62: 62](RO/H) This bit is set when an AQ command is currently being processed. */
		u64 aq_err:1;		     /**< [ 63: 63](R/W1C/H) AQ error. See NIX_AF_ERR_INT[AQ_INST_FAULT,AQ_RES_FAULT,AQ_DOOR_ERR] and
                                                                 NIX_AF_RAS[AQ_INST_POISON,AQ_RES_POISON].
                                                                 When set, hardware stops reading instructions from the AQ ring. Software
                                                                 clears the error by writing a one back. */
	} cn;
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_door
 *
 * NIX AF Admin Queue Doorbell Register
 * Software writes to this register to enqueue entries to AQ.
 */
union cavm_nixx_af_aq_door {
	u64 u;
	struct cavm_nixx_af_aq_door_s {
		u64 count:16;		     /**< [ 15:  0](WO) Number of enqueued 16-byte entries. Hardware advances
                                                                 NIX_AF_AQ_STATUS[TAIL_PTR] by this value.

                                                                 A doorbell write that would overflow the AQ ring is suppressed and sets
                                                                 NIX_AF_AQ_STATUS[AQ_ERR] and NIX_AF_ERR_INT[AQ_DOOR_ERR]. */
		u64 reserved_16_63:48;
	} s;
	/* struct cavm_nixx_af_aq_door_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_wait
 *
 * NIX AF Admin Queue Done Interrupt Coalescing Wait Register
 * Specifies the queue interrupt coalescing settings.
 */
union cavm_nixx_af_aq_done_wait {
	u64 u;
	struct cavm_nixx_af_aq_done_wait_s {
		u64 num_wait:20;		     /**< [ 19:  0](R/W) Number of messages hold-off. When NIX_AF_AQ_DONE[DONE] \>= [NUM_WAIT] then
                                                                 interrupt coalescing ends; see NIX_AF_AQ_DONE[DONE]. If 0x0, same behavior as
                                                                 0x1. */
		u64 reserved_20_31:12;
		u64 time_wait:16;		     /**< [ 47: 32](R/W) Time hold-off in microseconds. When NIX_AF_AQ_DONE[DONE] = 0, or
                                                                 NIX_AF_AQ_DONE_ACK is written a timer is cleared. The timer increments
                                                                 every microsecond, and interrupt coalescing ends when timer reaches
                                                                 [TIME_WAIT]; see NIX_AF_AQ_DONE[DONE]. If 0x0, time coalescing is disabled. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_aq_done_wait_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done
 *
 * NIX AF Admin Queue Done Count Register
 */
union cavm_nixx_af_aq_done {
	u64 u;
	struct cavm_nixx_af_aq_done_s {
		u64 done:20;		     /**< [ 19:  0](R/W/H) Done count. When NIX_AQ_INST_S[DONEINT] set and that instruction completes,
                                                                 NIX_AF_AQ_DONE[DONE] is incremented. Write to this field are for diagnostic use only;
                                                                 instead software writes NIX_AF_AQ_DONE_ACK with the number of decrements for this field.

                                                                 Interrupts are sent as follows:

                                                                 * When NIX_AF_AQ_DONE[DONE] = 0, then no results are pending, the interrupt
                                                                 coalescing timer is held to zero, and an interrupt is not sent.

                                                                 * When NIX_AF_AQ_DONE[DONE] != 0, then the interrupt coalescing timer
                                                                 counts every microsecond. If the counter is \>= NIX_AF_AQ_DONE_WAIT[TIME_WAIT],
                                                                 or NIX_AF_AQ_DONE[DONE] \>= NIX_AF_AQ_DONE_WAIT[NUM_WAIT], i.e. enough time
                                                                 has passed or enough results have arrived, then the interrupt is sent.
                                                                 Otherwise, it is not sent due to coalescing.

                                                                 * When NIX_AF_AQ_DONE_ACK is written (or NIX_AF_AQ_DONE is written but this is
                                                                 not typical), the interrupt coalescing timer restarts. Note after decrementing
                                                                 this interrupt equation is recomputed, for example if NIX_AF_AQ_DONE[DONE] \>=
                                                                 NIX_AF_AQ_DONE_WAIT[NUM_WAIT] and because the timer is zero, the interrupt will
                                                                 be resent immediately. (This covers the race case between software
                                                                 acknowledging an interrupt and a result returning.).

                                                                 * When NIX_AF_AQ_DONE_ENA_W1S[DONE] = 0, interrupts are not sent, but the
                                                                 counting described above still occurs.

                                                                 AQ instructions complete in order.

                                                                 Software is responsible for making sure [DONE] does not overflow; for example by
                                                                 insuring there are not more than 2^20-1 instructions in flight that may request
                                                                 interrupts. */
		u64 reserved_20_63:44;
	} s;
	/* struct cavm_nixx_af_aq_done_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_ack
 *
 * NIX AF Admin Queue Done Count Ack Register
 * This register is written by software to acknowledge interrupts.
 */
union cavm_nixx_af_aq_done_ack {
	u64 u;
	struct cavm_nixx_af_aq_done_ack_s {
		u64 done_ack:20;		     /**< [ 19:  0](R/W/H) Number of decrements to NIX_AF_AQ_DONE[DONE]. Reads NIX_AF_AQ_DONE[DONE].

                                                                 Written by software to acknowledge interrupts. If NIX_AF_AQ_DONE[DONE] is still
                                                                 nonzero the interrupt will be resent if the conditions described in
                                                                 NIX_AF_AQ_DONE[DONE] are satisfied.

                                                                 Internal:
                                                                 If [DONE_ACK] write value is greater than NIX_AF_AQ_DONE[DONE], hardware
                                                                 resets NIX_AF_AQ_DONE[DONE] to zero. */
		u64 reserved_20_63:44;
	} s;
	/* struct cavm_nixx_af_aq_done_ack_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_int
 *
 * INTERNAL: NIX AF Admin Queue Done Interrupt Register
 */
union cavm_nixx_af_aq_done_int {
	u64 u;
	struct cavm_nixx_af_aq_done_int_s {
		u64 done:1;		     /**< [  0:  0](RO/H) Done interrupt. See NIX_AF_AQ_DONE[DONE]. Note this bit is read-only, to acknowledge
                                                                 interrupts use NIX_AF_AQ_DONE_ACK. To test interrupts, write nonzero to
                                                                 NIX_AF_AQ_DONE[DONE]. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_aq_done_int_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_int_w1s
 *
 * INTERNAL: NIX AF Admin Queue Done Interrupt Set Register
 */
union cavm_nixx_af_aq_done_int_w1s {
	u64 u;
	struct cavm_nixx_af_aq_done_int_w1s_s {
		u64 done:1;		     /**< [  0:  0](RO/H) Done interrupt. See NIX_AF_AQ_DONE[DONE]. Note this bit is read-only, to acknowledge
                                                                 interrupts use NIX_AF_AQ_DONE_ACK. To test interrupts, write nonzero to
                                                                 NIX_AF_AQ_DONE[DONE]. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_aq_done_int_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_ena_w1s
 *
 * NIX AF Admin Queue Done Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_af_aq_done_ena_w1s {
	u64 u;
	struct cavm_nixx_af_aq_done_ena_w1s_s {
		u64 done:1;		     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_AF_AQ_DONE_INT[DONE]. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_aq_done_ena_w1s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_ena_w1c
 *
 * NIX AF Admin Queue Done Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_af_aq_done_ena_w1c {
	u64 u;
	struct cavm_nixx_af_aq_done_ena_w1c_s {
		u64 done:1;		     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_AF_AQ_DONE_INT[DONE]. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_aq_done_ena_w1c_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_link#_sl#_spkt_cnt
 *
 * INTERNAL: NIX Receive Software Sync Link Packet Count Registers
 *
 * For diagnostic use only for debug of NIX_AF_RX_SW_SYNC[ENA] function. LINK
 * index is enumerated by NIX_LINK_E. SL index is zero for non-express packets,
 * one for express packets. For the internal NIX_LINK_E::MC, SL index is zero for
 * multicast replay, one for mirror replay.
 */
union cavm_nixx_af_rx_linkx_slx_spkt_cnt {
	u64 u;
	struct cavm_nixx_af_rx_linkx_slx_spkt_cnt_s {
		u64 in_cnt:20;		     /**< [ 19:  0](RO/H) Running count at input of machine. */
		u64 reserved_20_31:12;
		u64 out_cnt:20;		     /**< [ 51: 32](RO/H) Running count at output of machine. */
		u64 reserved_52_63:12;
	} s;
	/* struct cavm_nixx_af_rx_linkx_slx_spkt_cnt_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_link#_cfg
 *
 * NIX AF Receive Link Configuration Registers
 * Index enumerated by NIX_LINK_E.
 */
union cavm_nixx_af_rx_linkx_cfg {
	u64 u;
	struct cavm_nixx_af_rx_linkx_cfg_s {
		u64 minlen:16;		     /**< [ 15:  0](R/W) Byte count for min-sized frame check on packets received from this link.
                                                                 See NIX_RE_OPCODE_E::UNDERSIZE. Zero disables the check.
                                                                 See [MAXLEN] for packet bytes that may be included or excluded in the
                                                                 specified length. */
		u64 maxlen:16;		     /**< [ 31: 16](R/W) Byte count for max-sized frame check on packets received from this link.
                                                                 See NIX_RE_OPCODE_E::OVERSIZE. This length must include any any Vtags
                                                                 which may be stripped and optional timestamp inserted by CGX. FCS bytes
                                                                 stripped by CGX are not included. Must not exceed 9212 bytes (9216 minus 4
                                                                 byte FCS) for CGX and LBK links. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_rx_linkx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_sw_sync
 *
 * NIX AF Receive Software Sync Register
 */
union cavm_nixx_af_rx_sw_sync {
	u64 u;
	struct cavm_nixx_af_rx_sw_sync_s {
		u64 ena:1;			     /**< [  0:  0](R/W1S/H) Sync enable. Software sets this bit to kick off a sync cycle on the RX path.
                                                                 Hardware resets this bit to indicated the sync cycle is complete.
                                                                 This sync insures that all packets that were in flight are flushed out to memory.
                                                                 This can be used to assist in the tearing down of an active RQ. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_rx_sw_sync_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_link#_wrr_cfg
 *
 * NIX AF Receive Link Weighted Round Robin Configuration Registers
 * Index enumerated by NIX_LINK_E.
 */
union cavm_nixx_af_rx_linkx_wrr_cfg {
	u64 u;
	struct cavm_nixx_af_rx_linkx_wrr_cfg_s {
		u64 weight:8;		     /**< [  7:  0](R/W) Link's round robin weight for receiving packet data in 16-byte transfer
                                                                 units. Zero disables packed receive from the link.
                                                                 When the maximum aggregate data rate from all links exceeds the NIX
                                                                 data rate, software should program link weights proportional to the
                                                                 links speeds or based on desired link priorities.

                                                                 Internal:
                                                                 Link's weight in the X2P grant arbiter. */
		u64 reserved_8_63:56;
	} s;
	/* struct cavm_nixx_af_rx_linkx_wrr_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_eco
 *
 * INTERNAL: SEB AF ECO Register
 *
 * Internal:
 * TODO - When CSI ECO register is added, switch this one to inherit from it.
 */
union cavm_nixx_af_seb_eco {
	u64 u;
	struct cavm_nixx_af_seb_eco_s {
		u64 eco_rw:32;		     /**< [ 31:  0](R/W) Reserved for ECO usage. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_seb_eco_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_norm_tx_fifo_status
 *
 * NIX AF Normal Transmit FIFO Status Register
 * Status of FIFO which transmits normal (potentially preemptable) packets to CGX
 * and LBK.
 */
union cavm_nixx_af_norm_tx_fifo_status {
	u64 u;
	struct cavm_nixx_af_norm_tx_fifo_status_s {
		u64 count:12;		     /**< [ 11:  0](RO/H) Number of 128-bit entries in the TX FIFO. */
		u64 reserved_12_63:52;
	} s;
	/* struct cavm_nixx_af_norm_tx_fifo_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_expr_tx_fifo_status
 *
 * NIX AF Express Transmit FIFO Status Register
 * Status of FIFO which transmits express packets to CGX and LBK.
 */
union cavm_nixx_af_expr_tx_fifo_status {
	u64 u;
	struct cavm_nixx_af_expr_tx_fifo_status_s {
		u64 count:12;		     /**< [ 11:  0](RO/H) Number of 128-bit entries in the TX FIFO. */
		u64 reserved_12_63:52;
	} s;
	/* struct cavm_nixx_af_expr_tx_fifo_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_tx_fifo_status
 *
 * NIX AF SDP Transmit FIFO Status Register
 * Status of FIFO which transmits packets to SDP.
 */
union cavm_nixx_af_sdp_tx_fifo_status {
	u64 u;
	struct cavm_nixx_af_sdp_tx_fifo_status_s {
		u64 count:12;		     /**< [ 11:  0](RO/H) Number of 128-bit entries in the TX FIFO. */
		u64 reserved_12_63:52;
	} s;
	/* struct cavm_nixx_af_sdp_tx_fifo_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_npc_capture_config
 *
 * NIX AF Transmit NPC Response Capture Configuration Register
 * Configures the NPC response capture logic for transmit packets. When enabled,
 * allows NPC responses for selected packets to be captured in
 * NIX_AF_TX_NPC_CAPTURE_INFO and NIX_AF_TX_NPC_CAPTURE_RESP().
 */
union cavm_nixx_af_tx_npc_capture_config {
	u64 u;
	struct cavm_nixx_af_tx_npc_capture_config_s {
		u64 en:1;			     /**< [  0:  0](R/W) Enable for NPC response capturing. When one, NPC response capturing will be
                                                                 performed. When 0, NPC responses will not be captured. */
		u64 continuous:1;		     /**< [  1:  1](R/W) Enable for continuous mode of NPC response capturing. When one, the NPC response
                                                                 for any matching packet will be captured. When 0, only the first matching
                                                                 packet will be captured. */
		u64 lso_segnum_en:1;	     /**< [  2:  2](R/W) Enable for LSO segment number  matching, to trigger an NPC response capture.
                                                                 When one, an NPC response will be captured for a packet whose LSO segment number
                                                                 matches [LSO_SEGNUM]. */
		u64 sqe_id_en:1;		     /**< [  3:  3](R/W) Enable for SQE ID matching, to trigger an NPC response capture. When one, an NPC
                                                                 response will be captured for a packet whose SQE ID matches [SQE_ID]. */
		u64 sq_id_en:1;		     /**< [  4:  4](R/W) Enable for SQ matching, to trigger an NPC response capture. When one, an NPC
                                                                 response will be captured for a packet whose SQ matches [LF_ID]. */
		u64 lf_id_en:1;		     /**< [  5:  5](R/W) Enable for LF matching, to trigger an NPC response capture. When one, an NPC
                                                                 response will be captured for a packet whose LF matches [LF_ID]. */
		u64 reserved_6_11:6;
		u64 lso_segnum:8;		     /**< [ 19: 12](R/W) LSO segment number to be matched, when enabled by [LSO_SEGNUM_EN], to trigger
                                                                 an NPC response capture. */
		u64 sqe_id:16;		     /**< [ 35: 20](R/W) SQE to be matched, when enabled by [SQE_ID_EN], to trigger an NPC response capture. */
		u64 sq_id:20;		     /**< [ 55: 36](R/W) SQ to be matched, when enabled by [SQ_ID_EN], to trigger an NPC response capture. */
		u64 lf_id:8;		     /**< [ 63: 56](R/W) LF to be matched, when enabled by [LF_ID_EN], to trigger an NPC response capture. */
	} s;
	/* struct cavm_nixx_af_tx_npc_capture_config_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_npc_capture_info
 *
 * NIX AF Transmit NPC Response Capture Information Register
 * This register contains captured NPC response information for a transmit packet.
 * See NIX_AF_TX_NPC_CAPTURE_CONFIG.
 */
union cavm_nixx_af_tx_npc_capture_info {
	u64 u;
	struct cavm_nixx_af_tx_npc_capture_info_s {
		u64 vld:1;			     /**< [  0:  0](R/W/H) When set, indicates a valid NPC response has been captured in this register
                                                                 and in NIX_AF_TX_NPC_CAPTURE_RESP(). */
		u64 reserved_1_11:11;
		u64 lso_segnum:8;		     /**< [ 19: 12](R/W/H) LSO segment number corresponding to the captured packet. */
		u64 sqe_id:16;		     /**< [ 35: 20](R/W/H) SQE ID corresponding to the captured packet. */
		u64 sq_id:20;		     /**< [ 55: 36](R/W/H) SQ ID corresponding to the captured packet. */
		u64 lf_id:8;		     /**< [ 63: 56](R/W/H) LF ID corresponding to the captured packet. */
	} s;
	/* struct cavm_nixx_af_tx_npc_capture_info_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_smq#_cfg
 *
 * NIX AF SQM PSE Queue Configuration Registers
 */
union cavm_nixx_af_smqx_cfg {
	u64 u;
	struct cavm_nixx_af_smqx_cfg_s {
		u64 minlen:7;		     /**< [  6:  0](R/W) Minimum packet length in bytes, excluding FCS potentially appended outside
                                                                 NIX by CGX. Packets smaller than the minimum length from this SMQ,
                                                                 including optional VLAN bytes inserted by NIX_SEND_EXT_S[VLAN*] and Vtag
                                                                 bytes inserted by NIX_TX_VTAG_ACTION_S, are padded with zeros. Software
                                                                 should program this to match the minimum length for all links that this SMQ
                                                                 can transmit to.

                                                                 Must be greater than 16. The default value ensures the pre-FCS packet is at
                                                                 least 60 bytes. */
		u64 desc_shp_ctl_dis:1;	     /**< [  7:  7](R/W) Descriptor shaper control disable for packets transmitted by this SMQ.
                                                                 0 = NIX_SEND_EXT_S[SHP_RA,SHP_DIS,SHP_CHG] values are used when present in
                                                                 the descriptor.
                                                                 1 = NIX_SEND_EXT_S[SHP_RA,SHP_DIS,SHP_CHG] values in the send descriptor,
                                                                 are ignored and treated as 0. */
		u64 maxlen:16;		     /**< [ 23:  8](R/W) Maximum packet length in bytes, including optional VLAN bytes inserted by
                                                                 NIX_SEND_EXT_S[VLAN*] and Vtag bytes inserted by NIX_TX_VTAG_ACTION_S,
                                                                 but excluding FCS potentially appended outside NIX by CGX.

                                                                 Must not be less than [MINLEN].
                                                                 Must not exceed 9212 (9216 minus four byte FCS) if the SMQ transmits to
                                                                 CGX and LBK. May be set to a larger value (up to 65535 bytes) if the SMQ
                                                                 transmits to SDP (corresponding NIX_AF_TL4()_SDP_LINK_CFG[ENA] is set).

                                                                 Software should set a value that does not exceed the MTU of any link to
                                                                 which the SMQ can transmit. */
		u64 lf:7;			     /**< [ 30: 24](R/W) Local function with SQs that may feed this SMQ. Software must ensure NIX_SQ_CTX_S[SMQ]
                                                                 does not point to this SMQ for any SQ outside of this LF. */
		u64 reserved_31_35:5;
		u64 max_vtag_ins:3;	     /**< [ 38: 36](R/W) Maximum Vtag insertion size as a as a multiple of four bytes. Must be less
                                                                 than or equal to four (16 bytes), and must be large enough to account for the
                                                                 maximum number of bytes inserted by NIX_TX_VTAG_ACTION_S for any packet
                                                                 sent through this SMQ.

                                                                 Internal:
                                                                 SQM computes allowed maximum Vtag insertion bytes (ok_vtag_max) such that
                                                                 the computed packet size does not exceed [MAXLEN], including VLAN bytes
                                                                 inserted by NIX_SEND_EXT_S[VLAN*]. SEB enforces ok_vtag_max when inserting
                                                                 Vtag bytes based on NIX_TX_VTAG_ACTION_S. */
		u64 express:1;		     /**< [ 39: 39](R/W) Express.
                                                                 0 = The SMQ transmits normal packets.
                                                                 1 = The SMQ transmits express packets.

                                                                 Must have the same value as the corresponding
                                                                 NIX_AF_TL3_TL2()_CFG[EXPRESS]. Hardware prioritizes enqueue to express SMQs
                                                                 over normal SMQs. */
		u64 flush:1;		     /**< [ 40: 40](R/W1S/H) Software can write a one to set this bit and initiate an SMQ flush.
                                                                 When set, hardware flushes all meta-descriptors/packets from this SMQ
                                                                 through PSE and the send data path. Hardware clears this bit and sets
                                                                 NIX_AF_GEN_INT[SMQ_FLUSH_DONE] when the flush operation is complete,

                                                                 [ENQ_XOFF] must be set whenever this bit is set.

                                                                 Must not be set when a DRAIN command is active
                                                                 (NIX_AF_MDQ()_SW_XOFF[DRAIN,DRAIN_IRQ] or
                                                                 NIX_AF_TL*()_SW_XOFF[DRAIN,DRAIN_IRQ] command has been issued and resulting
                                                                 NIX_AF_GEN_INT[TL1_DRAIN] is not set).

                                                                 The SMQ flush operation may stall if the downstream TL3/TL2 queue is
                                                                 backpressured from CGX/LBK or the downstream TL4 queue is backpressured
                                                                 from SDP. If the backpressure does not go away, software may need to
                                                                 disable it at the destination link(s), e.g. by clearing
                                                                 CGX()_SMU()_RX_FRM_CTL[CTL_BCK] to disable physical backpressure from a
                                                                 10G+ CGX LMAC. */
		u64 enq_xoff:1;		     /**< [ 41: 41](R/W) Enqueue transmit off. When set, hardware will not enqueue meta-descriptorrs
                                                                 to the SMQ. */
		u64 reserved_42_63:22;
	} s;
	/* struct cavm_nixx_af_smqx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_tl2#_link#_cfg
 *
 * NIX AF Transmit Level 3/2 Link Configuration Registers
 * These registers specify the links and associated channels that a given TL3 or
 * TL2 queue (depending on NIX_AF_PSE_CHANNEL_LEVEL[BP_LEVEL]) can transmit on.
 * Each TL3/TL2 queue can be enabled to transmit on and be backpressured by one or
 * more links and associated channels.
 */
union cavm_nixx_af_tl3_tl2x_linkx_cfg {
	u64 u;
	struct cavm_nixx_af_tl3_tl2x_linkx_cfg_s {
		u64 relchan:8;		     /**< [  7:  0](R/W) Relative channel number within this link. See [ENA]. */
		u64 reserved_8_11:4;
		u64 ena:1;			     /**< [ 12: 12](R/W) Enable. When set, the TL3/TL2 queue can transmit on relative channel number
                                                                 [RELCHAN] of this link, and will respond to backpressure from link credits
                                                                 (NIX_AF_TX_LINK()_NORM_CREDIT, NIX_AF_TX_LINK()_EXPR_CREDIT). If
                                                                 [BP_ENA] is set, the queue also responds to channel backpressure. */
		u64 bp_ena:1;		     /**< [ 13: 13](R/W) Backpressure enable. When set, the TL3/TL2 queue responds to backpressure
                                                                 from (NIX_AF_TX_LINK()_HW_XOFF/_SW_XOFF[CHAN_XOFF]\<[RELCHAN]\>). */
		u64 reserved_14_63:50;
	} s;
	/* struct cavm_nixx_af_tl3_tl2x_linkx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_shaper_cfg
 *
 * NIX AF PSE Shaper Configuration Register
 */
union cavm_nixx_af_pse_shaper_cfg {
	u64 u;
	struct cavm_nixx_af_pse_shaper_cfg_s {
		u64 red_send_as_yellow:1;	     /**< [  0:  0](R/W) Red send as yellow. Configures the way packets colored
                                                                 NIX_COLORRESULT_E::RED_SEND are handled by the TL4 through TL2 shapers when
                                                                 operating in [COLOR_AWARE] mode. Normally packets colored
                                                                 NIX_COLORRESULT_E::RED_DROP do not decrement the PIR in TL4 through TL2
                                                                 shapers while packets colored YELLOW do. (Neither
                                                                 NIX_COLORRESULT_E::RED_DROP nor YELLOW packets decrement the CIR in TL4
                                                                 through TL2 shapers.)  Packets colored NIX_COLORRESULT_E::RED_SEND are
                                                                 treated as either NIX_COLORRESULT_E::RED_DROP or YELLOW in the TL4 through
                                                                 TL2 shapers as follows:
                                                                 0 = Treat NIX_COLORRESULT_E::RED_SEND as NIX_COLORRESULT_E::RED_DROP.
                                                                 1 = Treat NIX_COLORRESULT_E::RED_SEND as YELLOW.

                                                                 In the TL1 shapers, NIX_COLORRESULT_E::RED_DROP packets do not decrement
                                                                 the CIR, while YELLOW do. NIX_COLORRESULT_E::RED_SEND packets are always
                                                                 treated the same as YELLOW is in the TL1 shapers, irrespective of
                                                                 [RED_SEND_AS_YELLOW]. */
		u64 color_aware:1;		     /**< [  1:  1](R/W) Color aware. Selects whether or not the PSE shapers take into account
                                                                 the color of the incoming packet.
                                                                 0 = Color blind.
                                                                 1 = Color aware. */
		u64 reserved_2_63:62;
	} s;
	/* struct cavm_nixx_af_pse_shaper_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_norm_credit
 *
 * NIX AF Transmit Normal Credit Register
 * This register tracks internal credits for normal (potentially preemptable)
 * packets sent to CGX and LBK.
 */
union cavm_nixx_af_tx_norm_credit {
	u64 u;
	struct cavm_nixx_af_tx_norm_credit_s {
		u64 reserved_0_1:2;
		u64 cc_packet_cnt:10;	     /**< [ 11:  2](R/W) Credit packet count. This value, plus 1, represents the maximum outstanding
                                                                 aggregate packet count for packets associated with this register (normal or
                                                                 express packets to CGX and LBK).
                                                                 Note that this 10-bit field represents a two's complement signed value that decrements
                                                                 towards zero as credits are used. Packets are not allowed to flow when the
                                                                 count is less than zero. As such the most significant bit should be
                                                                 programmed as zero (positive count).

                                                                 The recommended value is 255. Must satisfy:
                                                                 _  (NIX_AF_TX_NORM_CREDIT[CC_PACKET_CNT] +
                                                                 NIX_AF_TX_EXPR_CREDIT[CC_PACKET_CNT]) \< 511

                                                                 Internal:
                                                                 Limited by the 512 non-SDP PSE packet IDs. */
		u64 cc_unit_cnt:20;	     /**< [ 31: 12](R/W) Credit unit count. This value, plus 1 MTU, represents the maximum
                                                                 outstanding aggregate credit units for all packets associated with this
                                                                 register (normal or express packets to CGX and LBK). A credit unit is 16
                                                                 bytes. Note that this 20-bit field represents a two's complement signed value that
                                                                 decrements towards zero as credits are used. Packets are not allowed to
                                                                 flow when the count is less than zero. As such, the most significant bit
                                                                 should normally be programmed as zero (positive count). This gives a
                                                                 maximum value for this field of 2^19 - 1.

                                                                 The recommended value is
                                                                 [CC_UNIT_CNT] = (10 * Max_aggregate_CGX_plus_LBK_Data_Rate),
                                                                 e.g. [CC_UNIT_CNT] = 1000 for 100 Gbps data rate.

                                                                 Internal:
                                                                 Recommended value is sized for specified data rate with 1200ns round trip
                                                                 latency. See LBK example in NIX_AF_TX_LINK()_NORM_CREDIT[CC_UNIT_CNT]. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_tx_norm_credit_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_expr_credit
 *
 * INTERNAL: NIX AF Transmit Express Credit Register
 *
 * This register tracks internal credits for express packets sent to CGX and LBK
 * that may potentially preempt normal packets.
 */
union cavm_nixx_af_tx_expr_credit {
	u64 u;
	struct cavm_nixx_af_tx_expr_credit_s {
		u64 reserved_0_1:2;
		u64 cc_packet_cnt:10;	     /**< [ 11:  2](R/W) Credit packet count. This value, plus 1, represents the maximum outstanding
                                                                 aggregate packet count for packets associated with this register (normal or
                                                                 express packets to CGX and LBK).
                                                                 Note that this 10-bit field represents a two's complement signed value that decrements
                                                                 towards zero as credits are used. Packets are not allowed to flow when the
                                                                 count is less than zero. As such the most significant bit should be
                                                                 programmed as zero (positive count).

                                                                 The recommended value is 255. Must satisfy:
                                                                 _  (NIX_AF_TX_NORM_CREDIT[CC_PACKET_CNT] +
                                                                 NIX_AF_TX_EXPR_CREDIT[CC_PACKET_CNT]) \< 511

                                                                 Internal:
                                                                 Limited by the 512 non-SDP PSE packet IDs. */
		u64 cc_unit_cnt:20;	     /**< [ 31: 12](R/W) Credit unit count. This value, plus 1 MTU, represents the maximum
                                                                 outstanding aggregate credit units for all packets associated with this
                                                                 register (normal or express packets to CGX and LBK). A credit unit is 16
                                                                 bytes. Note that this 20-bit field represents a two's complement signed value that
                                                                 decrements towards zero as credits are used. Packets are not allowed to
                                                                 flow when the count is less than zero. As such, the most significant bit
                                                                 should normally be programmed as zero (positive count). This gives a
                                                                 maximum value for this field of 2^19 - 1.

                                                                 The recommended value is
                                                                 [CC_UNIT_CNT] = (10 * Max_aggregate_CGX_plus_LBK_Data_Rate),
                                                                 e.g. [CC_UNIT_CNT] = 1000 for 100 Gbps data rate.

                                                                 Internal:
                                                                 Recommended value is sized for specified data rate with 1200ns round trip
                                                                 latency. See LBK example in NIX_AF_TX_LINK()_NORM_CREDIT[CC_UNIT_CNT]. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_tx_expr_credit_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mark_format#_ctl
 *
 * NIX AF Packet Marking Format Registers
 * Describes packet marking calculations for YELLOW and for
 * NIX_COLORRESULT_E::RED_SEND packets. NIX_SEND_EXT_S[MARKFORM] selects the CSR
 * used for the packet descriptor.
 *
 * All the packet marking offset calculations assume big-endian bits within a byte.
 *
 * For example, if NIX_SEND_EXT_S[MARKPTR] is 3 and [OFFSET] is 5 and the packet is YELLOW,
 * the NIX marking hardware would do this:
 *
 * _  byte[3]\<2:0\> |=   [Y_VAL]\<3:1\>
 * _  byte[3]\<2:0\> &= ~[Y_MASK]\<3:1\>
 * _  byte[4]\<7\>   |=   [Y_VAL]\<0\>
 * _  byte[4]\<7\>   &= ~[Y_MASK]\<0\>
 *
 * where byte[3] is the third byte in the packet, and byte[4] the fourth.
 *
 * For another example, if NIX_SEND_EXT_S[MARKPTR] is 3 and [OFFSET] is 0 and the
 * packet is NIX_COLORRESULT_E::RED_SEND,
 *
 * _   byte[3]\<7:4\> |=   [R_VAL]\<3:0\>
 * _   byte[3]\<7:4\> &= ~[R_MASK]\<3:0\>
 */
union cavm_nixx_af_mark_formatx_ctl {
	u64 u;
	struct cavm_nixx_af_mark_formatx_ctl_s {
		u64 r_val:4;		     /**< [  3:  0](R/W) Red mark value. Corresponding bits in packet's data are set when marking a
                                                                 NIX_COLORRESULT_E::RED_SEND packet. [R_MASK] & [R_VAL] must be zero. */
		u64 r_mask:4;		     /**< [  7:  4](R/W) Red mark mask. Corresponding bits in packet's data are cleared when marking
                                                                 a NIX_COLORRESULT_E::RED_SEND packet. [R_MASK] & [R_VAL] must be zero. */
		u64 y_val:4;		     /**< [ 11:  8](R/W) Yellow mark value. Corresponding bits in packet's data are set when marking a YELLOW
                                                                 packet. [Y_MASK] & [Y_VAL] must be zero. */
		u64 y_mask:4;		     /**< [ 15: 12](R/W) Yellow mark mask. Corresponding bits in packet's data are cleared when marking a YELLOW
                                                                 packet. [Y_MASK] & [Y_VAL] must be zero. */
		u64 offset:3;		     /**< [ 18: 16](R/W) Packet marking starts NIX_SEND_EXT_S[MARKPTR]*8 + [OFFSET] bits into the packet.
                                                                 All processing with [Y_MASK,Y_VAL,R_MASK,R_VAL] starts at this offset. */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_mark_formatx_ctl_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_norm_credit
 *
 * NIX AF Transmit Link Normal Credit Registers
 * These registers track credits per link for normal (potentially preemptable)
 * packets sent to CGX and LBK. Link index enumerated by NIX_LINK_E.
 */
union cavm_nixx_af_tx_linkx_norm_credit {
	u64 u;
	struct cavm_nixx_af_tx_linkx_norm_credit_s {
		u64 reserved_0:1;
		u64 cc_enable:1;		     /**< [  1:  1](R/W) Credit enable. Enables [CC_UNIT_CNT] and [CC_PACKET_CNT] link credit processing. */
		u64 cc_packet_cnt:10;	     /**< [ 11:  2](R/W/H) Link-credit packet count. This value, plus 1, represents the maximum outstanding
                                                                 packet count for this link. Note that this 10-bit field represents a two's
                                                                 complement signed value that decrements towards zero as credits are used.
                                                                 Packets are not allowed to flow
                                                                 when the count is less than zero. As such the most significant bit should normally be
                                                                 programmed as zero (positive count). This gives a maximum value for this field of 2^9 - 1. */
		u64 cc_unit_cnt:20;	     /**< [ 31: 12](R/W/H) Link-credit unit count. This value, plus 1 MTU, represents the maximum outstanding
                                                                 credit units for this link. A credit unit is 16 bytes. Note that this
                                                                 20-bit field represents a two's complement signed value that decrements
                                                                 towards zero as credits are used.
                                                                 Packets are not allowed to flow when the count is less than zero. As such, the most
                                                                 significant bit should normally be programmed as zero (positive count). This gives a
                                                                 maximum value for this field of 2^19 - 1.

                                                                 In order to prevent blocking between CGX LMACs, [CC_ENABLE] should be set to 1 and
                                                                 [CC_UNIT_CNT] should be less than

                                                                 _     ((LMAC TX buffer size in CGX) - (MTU excluding FCS))/16

                                                                 The LMAC TX buffer size is defined by CGX()_CMR_TX_LMACS[LMACS]. For example, if
                                                                 CGX()_CMR_TX_LMACS[LMACS]=0x4 (16 KB per LMAC) and the LMAC's MTU excluding FCS
                                                                 is 9212 bytes (9216 minus 4 byte FCS), then [CC_UNIT_CNT] should be \< (16384 - 9212)/16 =
                                                                 448.

                                                                 The recommended configuration for LBK is [CC_ENABLE] = 1 and
                                                                 [CC_UNIT_CNT] = (10 * Max_LBK_Data_Rate),
                                                                 e.g. [CC_UNIT_CNT] = 1000 for 100 Gbps max LBK data rate.

                                                                 Internal:
                                                                 LBK value is sized for specified data rate with 1200ns round trip latency,
                                                                 e.g. for 100 Gbps:

                                                                 _ Minimum LBK in-flight data = 100*1200/128b = 938 credit units.

                                                                 Note: maximum LBK in-fligh data = initial_value + MTU. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_tx_linkx_norm_credit_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_expr_credit
 *
 * NIX AF Transmit Link Express Credit Registers
 * These registers track credits per link for express packets that may potentially
 * preempt normal packets. Link index enumerated by NIX_LINK_E.
 */
union cavm_nixx_af_tx_linkx_expr_credit {
	u64 u;
	struct cavm_nixx_af_tx_linkx_expr_credit_s {
		u64 reserved_0:1;
		u64 cc_enable:1;		     /**< [  1:  1](R/W) Credit enable. Enables [CC_UNIT_CNT] and [CC_PACKET_CNT] link credit processing. */
		u64 cc_packet_cnt:10;	     /**< [ 11:  2](R/W/H) Link-credit packet count. This value, plus 1, represents the maximum outstanding
                                                                 packet count for this link. Note that this 10-bit field represents a two's
                                                                 complement signed value that decrements towards zero as credits are used.
                                                                 Packets are not allowed to flow
                                                                 when the count is less than zero. As such the most significant bit should normally be
                                                                 programmed as zero (positive count). This gives a maximum value for this field of 2^9 - 1. */
		u64 cc_unit_cnt:20;	     /**< [ 31: 12](R/W/H) Link-credit unit count. This value, plus 1 MTU, represents the maximum outstanding
                                                                 credit units for this link. A credit unit is 16 bytes. Note that this
                                                                 20-bit field represents a two's complement signed value that decrements
                                                                 towards zero as credits are used.
                                                                 Packets are not allowed to flow when the count is less than zero. As such, the most
                                                                 significant bit should normally be programmed as zero (positive count). This gives a
                                                                 maximum value for this field of 2^19 - 1.

                                                                 In order to prevent blocking between CGX LMACs, [CC_ENABLE] should be set to 1 and
                                                                 [CC_UNIT_CNT] should be less than

                                                                 _     ((LMAC TX buffer size in CGX) - (MTU excluding FCS))/16

                                                                 The LMAC TX buffer size is defined by CGX()_CMR_TX_LMACS[LMACS]. For example, if
                                                                 CGX()_CMR_TX_LMACS[LMACS]=0x4 (16 KB per LMAC) and the LMAC's MTU excluding FCS
                                                                 is 9212 bytes (9216 minus 4 byte FCS), then [CC_UNIT_CNT] should be \< (16384 - 9212)/16 =
                                                                 448.

                                                                 The recommended configuration for LBK is [CC_ENABLE] = 1 and
                                                                 [CC_UNIT_CNT] = (10 * Max_LBK_Data_Rate),
                                                                 e.g. [CC_UNIT_CNT] = 1000 for 100 Gbps max LBK data rate.

                                                                 Internal:
                                                                 LBK value is sized for specified data rate with 1200ns round trip latency,
                                                                 e.g. for 100 Gbps:

                                                                 _ Minimum LBK in-flight data = 100*1200/128b = 938 credit units.

                                                                 Note: maximum LBK in-fligh data = initial_value + MTU. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_tx_linkx_expr_credit_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_sw_xoff
 *
 * NIX AF Transmit Link Software Controlled XOFF Registers
 * Link index enumerated by NIX_LINK_E.
 */
union cavm_nixx_af_tx_linkx_sw_xoff {
	u64 u;
	struct cavm_nixx_af_tx_linkx_sw_xoff_s {
		u64 chan_xoff:64;		     /**< [ 63:  0](R/W) Channel software controlled XOFF. One bit per channel on this link. When a bit is set,
                                                                 packets will not be transmitted on the associated channel. */
	} s;
	/* struct cavm_nixx_af_tx_linkx_sw_xoff_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_hw_xoff
 *
 * NIX AF Transmit Link Hardware Controlled XOFF Registers
 * Link index enumerated by NIX_LINK_E.
 */
union cavm_nixx_af_tx_linkx_hw_xoff {
	u64 u;
	struct cavm_nixx_af_tx_linkx_hw_xoff_s {
		u64 chan_xoff:64;	     /**< [ 63:  0](RO/H) Channel hardware XOFF status. One bit per channel on this link. When a bit is set,
                                                                 indicates that the transmit channel is being backpressured (XOFF) by the link. */
	} s;
	/* struct cavm_nixx_af_tx_linkx_hw_xoff_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_link_credit
 *
 * NIX AF Transmit Link SDP Credit Register
 * This register tracks SDP link credits.
 */
union cavm_nixx_af_sdp_link_credit {
	u64 u;
	struct cavm_nixx_af_sdp_link_credit_s {
		u64 reserved_0:1;
		u64 cc_enable:1;		     /**< [  1:  1](R/W) Credit enable. Enables [CC_UNIT_CNT] and [CC_PACKET_CNT] link credit
                                                                 processing. Must be one when SDP is used. */
		u64 cc_packet_cnt:10;	     /**< [ 11:  2](R/W/H) See NIX_AF_TX_LINK()_NORM_CREDIT[CC_PACKET_CNT]. Must be less than to 512.
                                                                 Internal:
                                                                 Limited by the 512 PSE packet IDs for SDP. */
		u64 cc_unit_cnt:20;	     /**< [ 31: 12](R/W/H) See NIX_AF_TX_LINK()_NORM_CREDIT[CC_UNIT_CNT].

                                                                 The recommended configuration for SDP is
                                                                 [CC_UNIT_CNT] = (10 * Max_SDP_Data_Rate),
                                                                 e.g. [CC_UNIT_CNT] = 500 for 50 Gbps max SDP data rate. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_sdp_link_credit_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_sw_xoff#
 *
 * NIX AF SDP Transmit Link Software Controlled XOFF Registers
 */
union cavm_nixx_af_sdp_sw_xoffx {
	u64 u;
	struct cavm_nixx_af_sdp_sw_xoffx_s {
		u64 chan_xoff:64;		     /**< [ 63:  0](R/W) Channel software controlled XOFF.
                                                                 One bit per SDP channel (NIX_AF_SDP_HW_XOFF({n})[CHAN_XOFF]\<{m}\> for
                                                                 NIX_CHAN_E::SDP_CH(64*{n}+{m}).
                                                                 When a bit is set, packets will not be transmitted on the associated
                                                                 channel. */
	} s;
	/* struct cavm_nixx_af_sdp_sw_xoffx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_hw_xoff#
 *
 * NIX AF SDP Transmit Link Hardware Controlled XOFF Registers
 * .
 */
union cavm_nixx_af_sdp_hw_xoffx {
	u64 u;
	struct cavm_nixx_af_sdp_hw_xoffx_s {
		u64 chan_xoff:64;		     /**< [ 63:  0](RO/H) Channel hardware XOFF status.
                                                                 One bit per SDP channel (NIX_AF_SDP_HW_XOFF({n})[CHAN_XOFF]\<{m}\> for
                                                                 NIX_CHAN_E::SDP_CH(64*{n}+{m}).
                                                                 When a bit is set, indicates that the transmit channel is being
                                                                 backpressured (XOFF) by the link. */
	} s;
	/* struct cavm_nixx_af_sdp_hw_xoffx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_bp_status
 *
 * NIX AF Transmit Level 4 Backpressure Status Registers
 */
union cavm_nixx_af_tl4x_bp_status {
	u64 u;
	struct cavm_nixx_af_tl4x_bp_status_s {
		u64 hw_xoff:1;		     /**< [  0:  0](RO/H) Hardware XOFF status. Set if NIX_AF_TL4()_SDP_LINK_CFG[BP_ENA] and the SDP
                                                                 channel selected by NIX_AF_TL4()_SDP_LINK_CFG[RELCHAN] is backpressure with
                                                                 XOFF, or if the SDP link is backpressured by NIX_AF_SDP_LINK_CREDIT. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_tl4x_bp_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_sdp_link_cfg
 *
 * NIX AF Transmit Level 4 Link Configuration Registers
 * These registers specify the which TL4 queues transmit to and are optionally
 * backpressured by SDP.
 */
union cavm_nixx_af_tl4x_sdp_link_cfg {
	u64 u;
	struct cavm_nixx_af_tl4x_sdp_link_cfg_s {
		u64 relchan:8;		     /**< [  7:  0](R/W) Relative channel number. When [BP_ENA] is set, this is the
                                                                 NIX_CHAN_E::SDP_CH() index of the SDP channel that may backpressure the TL4
                                                                 queue. */
		u64 reserved_8_11:4;
		u64 ena:1;			     /**< [ 12: 12](R/W) Enable.
                                                                 0 = The TL4 queue will not transmit to SDP and may transmit to CGX and/or LBK.
                                                                 1 = The TL4 queue may only transmit to SDP and will respond to backpressure from
                                                                 NIX_AF_SDP_LINK_CREDIT. If [BP_ENA] is set, the queue also responds to channel
                                                                 backpressure. */
		u64 bp_ena:1;		     /**< [ 13: 13](R/W) Backpressure enable. When set, the TL4 queue responds to backpressure from
                                                                 (NIX_AF_SDP_HW_XOFF()/_SW_XOFF()[CHAN_XOFF]\<[RELCHAN]\>). */
		u64 reserved_14_63:50;
	} s;
	/* struct cavm_nixx_af_tl4x_sdp_link_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_schedule
 *
 * NIX AF Transmit Level 1 Scheduling Control Register
 */
union cavm_nixx_af_tl1x_schedule {
	u64 u;
	struct cavm_nixx_af_tl1x_schedule_s {
		u64 rr_quantum:24;		     /**< [ 23:  0](R/W) Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned
                                                                 integer).

                                                                 Typically [RR_QUANTUM] should be at or near the MTU or more (to limit or prevent
                                                                 negative accumulations of the deficit count).

                                                                 Transmit limiter 1 packet meta descriptor are active in the scheduler when the rate limiter
                                                                 has not been exceeded. The position of the child in the TL1 array is always the position in
                                                                 the circle (i.e. no linked list, used by packet queue arbiter). PSE moves the current head
                                                                 of the circle on quantum expiration or when the head cannot follow with an active packet.

                                                                 Packet queue arbiter takes a snap shot of TL1 active packet meta descriptor and performs
                                                                 round robin arbitration. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_tl1x_schedule_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_shape
 *
 * NIX AF Transmit Level 1 Shaping Control Register
 */
union cavm_nixx_af_tl1x_shape {
	u64 u;
	struct cavm_nixx_af_tl1x_shape_s {
		u64 adjust:9;		     /**< [  8:  0](R/W) NIX_AF_TL2()_SHAPE[ADJUST]. */
		u64 reserved_9_23:15;
		u64 length_disable:1;	     /**< [ 24: 24](R/W) NIX_AF_TL2()_SHAPE[LENGTH_DISABLE]. */
		u64 reserved_25_63:39;
	} s;
	struct cavm_nixx_af_tl1x_shape_cn {
		u64 adjust:9;		     /**< [  8:  0](R/W) NIX_AF_TL2()_SHAPE[ADJUST]. */
		u64 reserved_9_17:9;
		u64 reserved_18_23:6;
		u64 length_disable:1;	     /**< [ 24: 24](R/W) NIX_AF_TL2()_SHAPE[LENGTH_DISABLE]. */
		u64 reserved_25_63:39;
	} cn;
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_cir
 *
 * NIX AF Transmit Level 1 Committed Information Rate Register
 */
union cavm_nixx_af_tl1x_cir {
	u64 u;
	struct cavm_nixx_af_tl1x_cir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl1x_cir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_shape_state
 *
 * NIX AF Transmit Level 1 Shape State Register
 * This register must not be written during normal operation.
 */
union cavm_nixx_af_tl1x_shape_state {
	u64 u;
	struct cavm_nixx_af_tl1x_shape_state_s {
		u64 cir_accum:26;		     /**< [ 25:  0](R/W/H) Committed information rate accumulator. Debug access to the live CIR accumulator. */
		u64 reserved_26_51:26;
		u64 color:1;		     /**< [ 52: 52](R/W/H) Shaper connection status. Debug access to the live shaper state.
                                                                 0 = Connected - shaper is connected.
                                                                 1 = Pruned - shaper is disconnected. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_tl1x_shape_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_sw_xoff
 *
 * NIX AF Transmit Level 1 Software Controlled XOFF Registers
 */
union cavm_nixx_af_tl1x_sw_xoff {
	u64 u;
	struct cavm_nixx_af_tl1x_sw_xoff_s {
		u64 xoff:1;		     /**< [  0:  0](R/W) XOFF. Stops meta flow out of the MDQ/TL* shaping queue. When [XOFF] is set,
                                                                 the corresponding meta descriptor in the MDQ/TL* shaping queue cannot be
                                                                 transferred to the next level. */
		u64 drain:1;		     /**< [  1:  1](WO) Drain. This control activates a drain path through the PSE that starts at this queue
                                                                 and ends at the TL1 level. The drain path is prioritized over other paths through PSE
                                                                 and can be used in combination with [DRAIN_IRQ]. [DRAIN] need never be set for the
                                                                 TL1 level, but is useful at all other levels, including the TL4 and MDQ levels.
                                                                 NIX_AF_GEN_INT[TL1_DRAIN] should be clear prior to initiating a [DRAIN]=1 write to
                                                                 this CSR.

                                                                 After [DRAIN] is set for a shaping queue, it should not be set again, for
                                                                 this or any other shaping queue, until NIX_AF_GEN_INT[TL1_DRAIN] is set.

                                                                 [DRAIN] must not be set for any shaping queue when an SMQ FLUSH command is
                                                                 active (any NIX_AF_SMQ()_CFG[FLUSH] is set).

                                                                 DRAIN has no effect unless [XOFF] is also set. Only one drain command is
                                                                 allowed to be active at a time. */
		u64 reserved_2:1;
		u64 drain_irq:1;		     /**< [  3:  3](WO) Drain IRQ. Enables setting of NIX_AF_GEN_INT[TL1_DRAIN] when the drain
                                                                 operation has completed.
                                                                 [DRAIN_IRQ] should be set whenever [DRAIN] is, and must not be set when [DRAIN] isn't
                                                                 set. [DRAIN_IRQ] has no effect unless [DRAIN] and [XOFF] are also set. */
		u64 reserved_4_63:60;
	} s;
	/* struct cavm_nixx_af_tl1x_sw_xoff_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_topology
 *
 * NIX AF Transmit Level 1 Topology Registers
 */
union cavm_nixx_af_tl1x_topology {
	u64 u;
	struct cavm_nixx_af_tl1x_topology_s {
		u64 reserved_0:1;
		u64 rr_prio:4;		     /**< [  4:  1](R/W) Round-robin priority. The priority assigned to the round-robin scheduler. A
                                                                 higher-level queue is a child queue of this shaper when its
                                                                 NIX_AF_TL*()_PARENT/NIX_AF_MDQ()_PARENT
                                                                 selects this shaper, and it further is a round robin child queue when its
                                                                 NIX_AF_TL*()_SCHEDULE[PRIO] equals [RR_PRIO]. All round-robin queues
                                                                 attached to this shaper must have the same priority. But the number of
                                                                 round-robin child queues attached (at this priority) is limited only by the
                                                                 number of higher-level queues. When this shaper is not used, we recommend
                                                                 [RR_PRIO] be zero.

                                                                 When a shaper is used, [RR_PRIO] should be 0xF when there are no priorities
                                                                 with more than one child queue (i.e. when there are no round-robin child
                                                                 queues), and should otherwise be a legal priority (values 0-9). */
		u64 reserved_5_31:27;
		u64 prio_anchor:8;		     /**< [ 39: 32](R/W) Priority anchor. The base index positioning the static priority child
                                                                 queues of this shaper. A higher-level queue is a child queue of this shaper
                                                                 when its NIX_AF_TL*()_PARENT/NIX_AF_MDQ()_PARENT selects this shaper, and
                                                                 it further is a static priority child queue when its
                                                                 NIX_AF_TL*()_SCHEDULE[PRIO] does not equal [RR_PRIO]. A static priority
                                                                 child queue with priority PRIO must be located at n=[PRIO_ANCHOR]+PRIO,
                                                                 where PRIO=NIX_AF_TL*(n)_SCHEDULE[PRIO]. There can be at most one static
                                                                 priority child queue at each priority. When there are no static priority
                                                                 child queues attached at any priority, or if this shaper isn't used, the
                                                                 hardware does not use [PRIO_ANCHOR]. In this case, we recommend
                                                                 [PRIO_ANCHOR] be zero. Note that there are 10 available priorities, 0
                                                                 through 9, with priority 0 being the highest and priority 9 being the
                                                                 lowest. */
		u64 reserved_40_63:24;
	} s;
	/* struct cavm_nixx_af_tl1x_topology_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_green
 *
 * INTERNAL: NIX Transmit Level 1 Green State Debug Register
 */
union cavm_nixx_af_tl1x_green {
	u64 u;
	struct cavm_nixx_af_tl1x_green_s {
		u64 tail:8;		     /**< [  7:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_8_9:2;
		u64 head:8;		     /**< [ 17: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_18_19:2;
		u64 active_vec:20;		     /**< [ 39: 20](R/W/H) Active vector. A 20-bit vector, two bits per each of the 10 supported priorities.
                                                                 For the non-RR_PRIO priorities, the two bits encode whether the child is active
                                                                 GREEN, active YELLOW, active RED_SEND/RED_DROP, or inactive. At RR_PRIO, one
                                                                 bit is set if the GREEN DWRR child list is not empty, and the other is set if the
                                                                 YELLOW DWRR child list is not empty. For internal use only. */
		u64 rr_active:1;		     /**< [ 40: 40](R/W/H) Round-robin red active. Set when the RED_SEND/RED_DROP DWRR child list is not empty.
                                                                 For internal use only. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl1x_green_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_yellow
 *
 * INTERNAL: NIX Transmit Level 1 Yellow State Debug Register
 */
union cavm_nixx_af_tl1x_yellow {
	u64 u;
	struct cavm_nixx_af_tl1x_yellow_s {
		u64 tail:8;		     /**< [  7:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_8_9:2;
		u64 head:8;		     /**< [ 17: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_18_63:46;
	} s;
	/* struct cavm_nixx_af_tl1x_yellow_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_red
 *
 * INTERNAL: NIX Transmit Level 1 Red State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL1()_YELLOW.
 */
union cavm_nixx_af_tl1x_red {
	u64 u;
	struct cavm_nixx_af_tl1x_red_s {
		u64 tail:8;		     /**< [  7:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_8_9:2;
		u64 head:8;		     /**< [ 17: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_18_63:46;
	} s;
	/* struct cavm_nixx_af_tl1x_red_s cn; */
};

union cavm_nixx_af_tl1x_dropped_packets {
	u64 u;
	struct cavm_nixx_af_tl1x_dropped_packets_s {
		u64 count:40;		     /**< [ 39:  0](R/W/H) Count. The running count of packets. Note that this count wraps. */
		u64 reserved_40_63:24;
	} s;
	/* struct cavm_nixx_af_tl1x_dropped_packets_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_dropped_bytes
 *
 * NIX AF Transmit Level 1 Dropped Bytes Registers
 * This register has the same bit fields as NIX_AF_TL1()_GREEN_BYTES.
 */
union cavm_nixx_af_tl1x_dropped_bytes {
	u64 u;
	struct cavm_nixx_af_tl1x_dropped_bytes_s {
		u64 count:48;		     /**< [ 47:  0](R/W/H) Count. The running count of bytes. Note that this count wraps. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_tl1x_dropped_bytes_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_red_packets
 *
 * NIX AF Transmit Level 1 Red Sent Packets Registers
 * This register has the same bit fields as NIX_AF_TL1()_GREEN_PACKETS.
 */
union cavm_nixx_af_tl1x_red_packets {
	u64 u;
	struct cavm_nixx_af_tl1x_red_packets_s {
		u64 count:40;		     /**< [ 39:  0](R/W/H) Count. The running count of packets. Note that this count wraps. */
		u64 reserved_40_63:24;
	} s;
	/* struct cavm_nixx_af_tl1x_red_packets_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_red_bytes
 *
 * NIX AF Transmit Level 1 Red Sent Bytes Registers
 * This register has the same bit fields as NIX_AF_TL1()_GREEN_BYTES.
 */
union cavm_nixx_af_tl1x_red_bytes {
	u64 u;
	struct cavm_nixx_af_tl1x_red_bytes_s {
		u64 count:48;		     /**< [ 47:  0](R/W/H) Count. The running count of bytes. Note that this count wraps. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_tl1x_red_bytes_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_yellow_packets
 *
 * NIX AF Transmit Level 1 Yellow Sent Packets Registers
 * This register has the same bit fields as NIX_AF_TL1()_GREEN_PACKETS.
 */
union cavm_nixx_af_tl1x_yellow_packets {
	u64 u;
	struct cavm_nixx_af_tl1x_yellow_packets_s {
		u64 count:40;		     /**< [ 39:  0](R/W/H) Count. The running count of packets. Note that this count wraps. */
		u64 reserved_40_63:24;
	} s;
	/* struct cavm_nixx_af_tl1x_yellow_packets_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_yellow_bytes
 *
 * NIX AF Transmit Level 1 Yellow Sent Bytes Registers
 * This register has the same bit fields as NIX_AF_TL1()_GREEN_BYTES.
 */
union cavm_nixx_af_tl1x_yellow_bytes {
	u64 u;
	struct cavm_nixx_af_tl1x_yellow_bytes_s {
		u64 count:48;		     /**< [ 47:  0](R/W/H) Count. The running count of bytes. Note that this count wraps. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_tl1x_yellow_bytes_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_green_packets
 *
 * NIX AF Transmit Level 1 Green Sent Packets Registers
 */
union cavm_nixx_af_tl1x_green_packets {
	u64 u;
	struct cavm_nixx_af_tl1x_green_packets_s {
		u64 count:40;		     /**< [ 39:  0](R/W/H) Count. The running count of packets. Note that this count wraps. */
		u64 reserved_40_63:24;
	} s;
	/* struct cavm_nixx_af_tl1x_green_packets_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_green_bytes
 *
 * NIX AF Transmit Level 1 Green Sent Bytes Registers
 */
union cavm_nixx_af_tl1x_green_bytes {
	u64 u;
	struct cavm_nixx_af_tl1x_green_bytes_s {
		u64 count:48;		     /**< [ 47:  0](R/W/H) Count. The running count of bytes. Note that this count wraps. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_tl1x_green_bytes_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_schedule
 *
 * NIX AF Transmit Level 2 Scheduling Control Registers
 */
union cavm_nixx_af_tl2x_schedule {
	u64 u;
	struct cavm_nixx_af_tl2x_schedule_s {
		u64 rr_quantum:24;		     /**< [ 23:  0](R/W) Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned
                                                                 integer). The packet size used in all DWRR (RR_COUNT) calculations is:

                                                                 _  (NIX_nm_SHAPE[LENGTH_DISABLE] ? 0 : (NIX_nm_MD*[LENGTH] + NIX_nm_MD*[ADJUST]))
                                                                    + NIX_nm_SHAPE[ADJUST]

                                                                 where nm corresponds to this NIX_nm_SCHEDULE CSR.

                                                                 Typically [RR_QUANTUM] should be at or near the MTU or more (to limit or prevent
                                                                 negative accumulations of the deficit count). */
		u64 prio:4;		     /**< [ 27: 24](R/W) Priority. The priority used for this shaping queue in the (lower-level)
                                                                 parent's scheduling algorithm. When this shaping queue is not used, we
                                                                 recommend setting [PRIO] to zero. The legal [PRIO] values are zero to nine
                                                                 when the shaping queue is used. In addition to priority, [PRIO] determines
                                                                 whether the shaping queue is a static queue or not: If [PRIO] equals the
                                                                 parent's NIX_AF_TL*()_TOPOLOGY[RR_PRIO], then this is a round-robin child
                                                                 queue into the shaper at the next level. */
		u64 reserved_28_63:36;
	} s;
	/* struct cavm_nixx_af_tl2x_schedule_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_shape
 *
 * NIX AF Transmit Level 2 Shaping Control Registers
 */
union cavm_nixx_af_tl2x_shape {
	u64 u;
	struct cavm_nixx_af_tl2x_shape_s {
		u64 adjust:9;		     /**< [  8:  0](R/W) Shaping and scheduling calculation adjustment. This nine-bit two's
                                                                 complement signed value allows -255 .. 255 bytes to be added to the packet
                                                                 length for rate limiting and scheduling calculations. Constraints:
                                                                 * Must be positive when [LENGTH_DISABLE] is set
                                                                 * May be negative When [LENGTH_DISABLE] is clear, but must be greater than
                                                                 -[MINLEN].
                                                                 * Must not be 0x100. */
		u64 red_algo:2;		     /**< [ 10:  9](R/W) Shaper red state algorithm when not specified by the NIX SEND. Used by hardware
                                                                 only when the shaper is in RED state. (A shaper is in RED state when
                                                                 NIX_AF_TL*()_SHAPE_STATE[PIR_ACCUM] is negative.) When NIX_SEND_EXT_S[SHP_RA]!=STD (!=0) for a
                                                                 packet, this [RED_ALGO] is not used, and NIX_SEND_EXT_S[SHP_RA] instead defines
                                                                 the shaper red state algorithm used for the packet. The
                                                                 encoding for the [RED_ALGO]/NIX_SEND_EXT_S[SHP_RA] that is used:
                                                                 0x0 = STALL. See 0x2.
                                                                 0x1 = SEND. Send packets while the shaper is in RED state. When the shaper is
                                                                       in RED state, packets that traverse the shaper will be downgraded to
                                                                       NIX_COLORRESULT_E::RED_SEND.  (if not already
                                                                       NIX_COLORRESULT_E::RED_SEND or NIX_COLORRESULT_E::RED_DROP) unless
                                                                       [RED_DISABLE] is set or NIX_SEND_EXT_S[SHP_DIS] for the packet is
                                                                       set. See also NIX_REDALG_E::SEND.
                                                                 0x2 = STALL. Stall packets while the shaper is in RED state until the shaper is
                                                                       YELLOW or GREEN state. Packets that traverse the shaper are never
                                                                       downgraded to the RED state in this mode.
                                                                       See also NIX_REDALG_E::STALL.
                                                                 0x3 = DISCARD. Continually discard packets while the shaper is in RED state.
                                                                       When the shaper is in RED state, all packets that traverse the shaper
                                                                       will be downgraded to NIX_COLORRESULT_E::RED_DROP (if not already
                                                                       NIX_COLORRESULT_E::RED_DROP), unless [RED_DISABLE] is set or
                                                                       NIX_SEND_EXT_S[SHP_DIS] for the packet is set.
                                                                       NIX_COLORRESULT_E::RED_DROP packets traverse all subsequent
                                                                       schedulers/shapers (all the way through L1), but do so as quickly as
                                                                       possible without affecting any RR_COUNT, CIR_ACCUM, or PIR_ACCUM
                                                                       state, and are then discarded by NIX. See also NIX_REDALG_E::DISCARD. */
		u64 red_disable:1;		     /**< [ 11: 11](R/W) Disable red transitions. Disables green-to-red and yellow-to-red packet
                                                                 color marking transitions when set. Not used by hardware when
                                                                 [RED_ALGO]/NIX_SEND_EXT_S[SHP_RA]=0x2/STALLi nor when corresponding
                                                                 NIX_AF_TL*()_PIR[ENABLE]/NIX_AF_MDQ()_PIR[ENABLE] is clear. */
		u64 yellow_disable:1;	     /**< [ 12: 12](R/W) Disable yellow transitions. Disables green-to-yellow packet color marking
                                                                 transitions when set. Not used by hardware when corresponding
                                                                 NIX_AF_TL*()_CIR[ENABLE]/NIX_AF_MDQ()_CIR[ENABLE] is clear. */
		u64 reserved_13_23:11;
		u64 length_disable:1;	     /**< [ 24: 24](R/W) Length disable. Disables the use of packet lengths in DWRR scheduling
                                                                 and shaping calculations such that only the value of [ADJUST] is used. */
		u64 schedule_list:2;	     /**< [ 26: 25](R/W) Shaper scheduling list. Restricts shaper scheduling to specific lists.
                                                                   0x0 = Normal (selected for nearly all scheduling/shaping applications).
                                                                   0x1 = Green-only.
                                                                   0x2 = Yellow-only.
                                                                   0x3 = Red-only. */
		u64 reserved_27_63:37;
	} s;
	/* struct cavm_nixx_af_tl2x_shape_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_cir
 *
 * NIX AF Transmit Level 2 Committed Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_tl2x_cir {
	u64 u;
	struct cavm_nixx_af_tl2x_cir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl2x_cir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_pir
 *
 * NIX AF Transmit Level 2 Peak Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_tl2x_pir {
	u64 u;
	struct cavm_nixx_af_tl2x_pir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl2x_pir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_sched_state
 *
 * NIX AF Transmit Level 2 Scheduling Control State Registers
 */
union cavm_nixx_af_tl2x_sched_state {
	u64 u;
	struct cavm_nixx_af_tl2x_sched_state_s {
		u64 rr_count:25;		     /**< [ 24:  0](R/W/H) Round-robin (DWRR) deficit counter. A 25-bit two's complement signed
                                                                 integer count. For diagnostic use. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_tl2x_sched_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_shape_state
 *
 * NIX AF Transmit Level 2 Shape State Registers
 * This register must not be written during normal operation.
 */
union cavm_nixx_af_tl2x_shape_state {
	u64 u;
	struct cavm_nixx_af_tl2x_shape_state_s {
		u64 cir_accum:26;		     /**< [ 25:  0](R/W/H) Committed information rate accumulator. Debug access to the live CIR accumulator. */
		u64 pir_accum:26;		     /**< [ 51: 26](R/W/H) Peak information rate accumulator. Debug access to the live PIR accumulator. */
		u64 color:2;		     /**< [ 53: 52](R/W/H) Shaper connection status. Debug access to the live shaper state.
                                                                 0x0 = Green - shaper is connected into the green list.
                                                                 0x1 = Yellow - shaper is connected into the yellow list.
                                                                 0x2 = Red - shaper is connected into the red list.
                                                                 0x3 = Pruned - shaper is disconnected. */
		u64 reserved_54_63:10;
	} s;
	/* struct cavm_nixx_af_tl2x_shape_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_pointers
 *
 * INTERNAL: NIX Transmit Level 2 Linked List Pointers Debug Register
 */
union cavm_nixx_af_tl2x_pointers {
	u64 u;
	struct cavm_nixx_af_tl2x_pointers_s {
		u64 next:8;		     /**< [  7:  0](R/W/H) Next pointer. The linked-list next pointer. */
		u64 reserved_8_15:8;
		u64 prev:8;		     /**< [ 23: 16](R/W/H) Previous pointer. The linked-list previous pointer. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_tl2x_pointers_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_sw_xoff
 *
 * NIX AF Transmit Level 2 Software Controlled XOFF Registers
 * This register has the same bit fields as NIX_AF_TL1()_SW_XOFF.
 */
union cavm_nixx_af_tl2x_sw_xoff {
	u64 u;
	struct cavm_nixx_af_tl2x_sw_xoff_s {
		u64 xoff:1;		     /**< [  0:  0](R/W) XOFF. Stops meta flow out of the MDQ/TL* shaping queue. When [XOFF] is set,
                                                                 the corresponding meta descriptor in the MDQ/TL* shaping queue cannot be
                                                                 transferred to the next level. */
		u64 drain:1;		     /**< [  1:  1](WO) Drain. This control activates a drain path through the PSE that starts at this queue
                                                                 and ends at the TL1 level. The drain path is prioritized over other paths through PSE
                                                                 and can be used in combination with [DRAIN_IRQ]. [DRAIN] need never be set for the
                                                                 TL1 level, but is useful at all other levels, including the TL4 and MDQ levels.
                                                                 NIX_AF_GEN_INT[TL1_DRAIN] should be clear prior to initiating a [DRAIN]=1 write to
                                                                 this CSR.

                                                                 After [DRAIN] is set for a shaping queue, it should not be set again, for
                                                                 this or any other shaping queue, until NIX_AF_GEN_INT[TL1_DRAIN] is set.

                                                                 [DRAIN] must not be set for any shaping queue when an SMQ FLUSH command is
                                                                 active (any NIX_AF_SMQ()_CFG[FLUSH] is set).

                                                                 DRAIN has no effect unless [XOFF] is also set. Only one drain command is
                                                                 allowed to be active at a time. */
		u64 reserved_2:1;
		u64 drain_irq:1;		     /**< [  3:  3](WO) Drain IRQ. Enables setting of NIX_AF_GEN_INT[TL1_DRAIN] when the drain
                                                                 operation has completed.
                                                                 [DRAIN_IRQ] should be set whenever [DRAIN] is, and must not be set when [DRAIN] isn't
                                                                 set. [DRAIN_IRQ] has no effect unless [DRAIN] and [XOFF] are also set. */
		u64 reserved_4_63:60;
	} s;
	/* struct cavm_nixx_af_tl2x_sw_xoff_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_topology
 *
 * NIX AF Transmit Level 2 Topology Registers
 */
union cavm_nixx_af_tl2x_topology {
	u64 u;
	struct cavm_nixx_af_tl2x_topology_s {
		u64 reserved_0:1;
		u64 rr_prio:4;		     /**< [  4:  1](R/W) See NIX_AF_TL1()_TOPOLOGY[RR_PRIO]. */
		u64 reserved_5_31:27;
		u64 prio_anchor:8;		     /**< [ 39: 32](R/W) See NIX_AF_TL1()_TOPOLOGY[PRIO_ANCHOR]. */
		u64 reserved_40_63:24;
	} s;
	/* struct cavm_nixx_af_tl2x_topology_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_parent
 *
 * NIX AF Transmit Level 2 Parent Registers
 */
union cavm_nixx_af_tl2x_parent {
	u64 u;
	struct cavm_nixx_af_tl2x_parent_s {
		u64 reserved_0_15:16;
		u64 parent:5;		     /**< [ 20: 16](R/W) Parent queue index. The index of the shaping element at the next lower hierarchical level
                                                                 that accepts this shaping element's outputs. Refer to the NIX_AF_TL*()_TOPOLOGY
                                                                 [PRIO_ANCHOR,RR_PRIO] descriptions for constraints on which child queues can attach to
                                                                 which shapers at the next lower level. When this shaper is unused, we recommend that
                                                                 [PARENT] be zero. */
		u64 reserved_21_63:43;
	} s;
	/* struct cavm_nixx_af_tl2x_parent_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_green
 *
 * INTERNAL: NIX Transmit Level 2 Green State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL1()_GREEN.
 */
union cavm_nixx_af_tl2x_green {
	u64 u;
	struct cavm_nixx_af_tl2x_green_s {
		u64 tail:8;		     /**< [  7:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_8_9:2;
		u64 head:8;		     /**< [ 17: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_18_19:2;
		u64 active_vec:20;		     /**< [ 39: 20](R/W/H) Active vector. A 20-bit vector, two bits per each of the 10 supported priorities.
                                                                 For the non-RR_PRIO priorities, the two bits encode whether the child is active
                                                                 GREEN, active YELLOW, active RED_SEND/RED_DROP, or inactive. At RR_PRIO, one
                                                                 bit is set if the GREEN DWRR child list is not empty, and the other is set if the
                                                                 YELLOW DWRR child list is not empty. For internal use only. */
		u64 rr_active:1;		     /**< [ 40: 40](R/W/H) Round-robin red active. Set when the RED_SEND/RED_DROP DWRR child list is not empty.
                                                                 For internal use only. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl2x_green_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_yellow
 *
 * INTERNAL: NIX Transmit Level 2 Yellow State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL1()_YELLOW.
 */
union cavm_nixx_af_tl2x_yellow {
	u64 u;
	struct cavm_nixx_af_tl2x_yellow_s {
		u64 tail:8;		     /**< [  7:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_8_9:2;
		u64 head:8;		     /**< [ 17: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_18_63:46;
	} s;
	/* struct cavm_nixx_af_tl2x_yellow_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_red
 *
 * INTERNAL: NIX Transmit Level 2 Red State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL1()_RED.
 */
union cavm_nixx_af_tl2x_red {
	u64 u;
	struct cavm_nixx_af_tl2x_red_s {
#if __BYTE_ORDER == __BIG_ENDIAN	/* Word 0 - Big Endian */
		u64 reserved_18_63:46;
		u64 head:8;	     /**< [ 17: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_8_9:2;
		u64 tail:8;	     /**< [  7:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else				/* Word 0 - Little Endian */
		u64 tail:8;	     /**< [  7:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_8_9:2;
		u64 head:8;	     /**< [ 17: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_18_63:46;
#endif				/* Word 0 - End */
	} s;
	/* struct cavm_nixx_af_tl2x_red_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_schedule
 *
 * NIX AF Transmit Level 3 Scheduling Control Registers
 * This register has the same bit fields as NIX_AF_TL2()_SCHEDULE.
 */
union cavm_nixx_af_tl3x_schedule {
	u64 u;
	struct cavm_nixx_af_tl3x_schedule_s {
		u64 rr_quantum:24;		     /**< [ 23:  0](R/W) Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned
                                                                 integer). The packet size used in all DWRR (RR_COUNT) calculations is:

                                                                 _  (NIX_nm_SHAPE[LENGTH_DISABLE] ? 0 : (NIX_nm_MD*[LENGTH] + NIX_nm_MD*[ADJUST]))
                                                                    + NIX_nm_SHAPE[ADJUST]

                                                                 where nm corresponds to this NIX_nm_SCHEDULE CSR.

                                                                 Typically [RR_QUANTUM] should be at or near the MTU or more (to limit or prevent
                                                                 negative accumulations of the deficit count). */
		u64 prio:4;		     /**< [ 27: 24](R/W) Priority. The priority used for this shaping queue in the (lower-level)
                                                                 parent's scheduling algorithm. When this shaping queue is not used, we
                                                                 recommend setting [PRIO] to zero. The legal [PRIO] values are zero to nine
                                                                 when the shaping queue is used. In addition to priority, [PRIO] determines
                                                                 whether the shaping queue is a static queue or not: If [PRIO] equals the
                                                                 parent's NIX_AF_TL*()_TOPOLOGY[RR_PRIO], then this is a round-robin child
                                                                 queue into the shaper at the next level. */
		u64 reserved_28_63:36;
	} s;
	/* struct cavm_nixx_af_tl3x_schedule_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_shape
 *
 * NIX AF Transmit Level 3 Shaping Control Registers
 */
union cavm_nixx_af_tl3x_shape {
	u64 u;
	struct cavm_nixx_af_tl3x_shape_s {
		u64 adjust:9;		     /**< [  8:  0](R/W) See NIX_AF_TL2()_SHAPE[ADJUST]. */
		u64 red_algo:2;		     /**< [ 10:  9](R/W) See NIX_AF_TL2()_SHAPE[RED_ALGO]. */
		u64 red_disable:1;		     /**< [ 11: 11](R/W) See NIX_AF_TL2()_SHAPE[RED_DISABLE]. */
		u64 yellow_disable:1;	     /**< [ 12: 12](R/W) See NIX_AF_TL2()_SHAPE[YELLOW_DISABLE]. */
		u64 reserved_13_23:11;
		u64 length_disable:1;	     /**< [ 24: 24](R/W) Length disable. Disables the use of packet lengths in DWRR scheduling
                                                                 and shaping calculations such that only the value of [ADJUST] is used. */
		u64 schedule_list:2;	     /**< [ 26: 25](R/W) Shaper scheduling list. Restricts shaper scheduling to specific lists.
                                                                   0x0 = Normal (selected for nearly all scheduling/shaping applications).
                                                                   0x1 = Green-only.
                                                                   0x2 = Yellow-only.
                                                                   0x3 = Red-only. */
		u64 reserved_27_63:37;
	} s;
	/* struct cavm_nixx_af_tl3x_shape_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_cir
 *
 * NIX AF Transmit Level 3 Committed Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_tl3x_cir {
	u64 u;
	struct cavm_nixx_af_tl3x_cir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl3x_cir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_pir
 *
 * NIX AF Transmit Level 3 Peak Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_tl3x_pir {
	u64 u;
	struct cavm_nixx_af_tl3x_pir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl3x_pir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_sched_state
 *
 * NIX AF Transmit Level 3 Scheduling Control State Registers
 * This register has the same bit fields as NIX_AF_TL2()_SCHED_STATE.
 */
union cavm_nixx_af_tl3x_sched_state {
	u64 u;
	struct cavm_nixx_af_tl3x_sched_state_s {
		u64 rr_count:25;		     /**< [ 24:  0](R/W/H) Round-robin (DWRR) deficit counter. A 25-bit two's complement signed
                                                                 integer count. For diagnostic use. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_tl3x_sched_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_shape_state
 *
 * NIX AF Transmit Level 3 Shaping State Registers
 * This register has the same bit fields as NIX_AF_TL2()_SHAPE_STATE.
 * This register must not be written during normal operation.
 */
union cavm_nixx_af_tl3x_shape_state {
	u64 u;
	struct cavm_nixx_af_tl3x_shape_state_s {
		u64 cir_accum:26;		     /**< [ 25:  0](R/W/H) Committed information rate accumulator. Debug access to the live CIR accumulator. */
		u64 pir_accum:26;		     /**< [ 51: 26](R/W/H) Peak information rate accumulator. Debug access to the live PIR accumulator. */
		u64 color:2;		     /**< [ 53: 52](R/W/H) Shaper connection status. Debug access to the live shaper state.
                                                                 0x0 = Green - shaper is connected into the green list.
                                                                 0x1 = Yellow - shaper is connected into the yellow list.
                                                                 0x2 = Red - shaper is connected into the red list.
                                                                 0x3 = Pruned - shaper is disconnected. */
		u64 reserved_54_63:10;
	} s;
	/* struct cavm_nixx_af_tl3x_shape_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_pointers
 *
 * INTERNAL: NIX Transmit Level 3 Linked List Pointers Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL2()_POINTERS.
 */
union cavm_nixx_af_tl3x_pointers {
	u64 u;
	struct cavm_nixx_af_tl3x_pointers_s {
		u64 next:8;		     /**< [  7:  0](R/W/H) Next pointer. The linked-list next pointer. */
		u64 reserved_8_15:8;
		u64 prev:8;		     /**< [ 23: 16](R/W/H) Previous pointer. The linked-list previous pointer. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_tl3x_pointers_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_sw_xoff
 *
 * NIX AF Transmit Level 3 Software Controlled XOFF Registers
 * This register has the same bit fields as NIX_AF_TL1()_SW_XOFF
 */
union cavm_nixx_af_tl3x_sw_xoff {
	u64 u;
	struct cavm_nixx_af_tl3x_sw_xoff_s {
		u64 xoff:1;		     /**< [  0:  0](R/W) XOFF. Stops meta flow out of the MDQ/TL* shaping queue. When [XOFF] is set,
                                                                 the corresponding meta descriptor in the MDQ/TL* shaping queue cannot be
                                                                 transferred to the next level. */
		u64 drain:1;		     /**< [  1:  1](WO) Drain. This control activates a drain path through the PSE that starts at this queue
                                                                 and ends at the TL1 level. The drain path is prioritized over other paths through PSE
                                                                 and can be used in combination with [DRAIN_IRQ]. [DRAIN] need never be set for the
                                                                 TL1 level, but is useful at all other levels, including the TL4 and MDQ levels.
                                                                 NIX_AF_GEN_INT[TL1_DRAIN] should be clear prior to initiating a [DRAIN]=1 write to
                                                                 this CSR.

                                                                 After [DRAIN] is set for a shaping queue, it should not be set again, for
                                                                 this or any other shaping queue, until NIX_AF_GEN_INT[TL1_DRAIN] is set.

                                                                 [DRAIN] must not be set for any shaping queue when an SMQ FLUSH command is
                                                                 active (any NIX_AF_SMQ()_CFG[FLUSH] is set).

                                                                 DRAIN has no effect unless [XOFF] is also set. Only one drain command is
                                                                 allowed to be active at a time. */
		u64 reserved_2:1;
		u64 drain_irq:1;		     /**< [  3:  3](WO) Drain IRQ. Enables setting of NIX_AF_GEN_INT[TL1_DRAIN] when the drain
                                                                 operation has completed.
                                                                 [DRAIN_IRQ] should be set whenever [DRAIN] is, and must not be set when [DRAIN] isn't
                                                                 set. [DRAIN_IRQ] has no effect unless [DRAIN] and [XOFF] are also set. */
		u64 reserved_4_63:60;
	} s;
	/* struct cavm_nixx_af_tl3x_sw_xoff_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_topology
 *
 * NIX AF Transmit Level 3 Topology Registers
 */
union cavm_nixx_af_tl3x_topology {
	u64 u;
	struct cavm_nixx_af_tl3x_topology_s {
		u64 reserved_0:1;
		u64 rr_prio:4;		     /**< [  4:  1](R/W) See NIX_AF_TL1()_TOPOLOGY[RR_PRIO]. */
		u64 reserved_5_31:27;
		u64 prio_anchor:9;		     /**< [ 40: 32](R/W) See NIX_AF_TL1()_TOPOLOGY[PRIO_ANCHOR]. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl3x_topology_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_parent
 *
 * NIX AF Transmit Level 3 Parent Registers
 */
union cavm_nixx_af_tl3x_parent {
	u64 u;
	struct cavm_nixx_af_tl3x_parent_s {
		u64 reserved_0_15:16;
		u64 parent:8;		     /**< [ 23: 16](R/W) See NIX_AF_TL2()_PARENT[PARENT]. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_tl3x_parent_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_green
 *
 * INTERNAL: NIX Transmit Level 3 Green State Debug Register
 */
union cavm_nixx_af_tl3x_green {
	u64 u;
	struct cavm_nixx_af_tl3x_green_s {
		u64 tail:9;		     /**< [  8:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_9:1;
		u64 head:9;		     /**< [ 18: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_19:1;
		u64 active_vec:20;		     /**< [ 39: 20](R/W/H) Active vector. A 10-bit vector, ordered by priority, that indicate which inputs to this
                                                                 scheduling queue are active. For internal use only. */
		u64 rr_active:1;		     /**< [ 40: 40](R/W/H) Round-robin red active. Indicates that the round-robin input is mapped to RED. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl3x_green_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_yellow
 *
 * INTERNAL: NIX Transmit Level 3 Yellow State Debug Register
 */
union cavm_nixx_af_tl3x_yellow {
	u64 u;
	struct cavm_nixx_af_tl3x_yellow_s {
		u64 tail:9;		     /**< [  8:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_9:1;
		u64 head:9;		     /**< [ 18: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_tl3x_yellow_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_red
 *
 * INTERNAL: NIX Transmit Level 3 Red State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL3()_YELLOW.
 */
union cavm_nixx_af_tl3x_red {
	u64 u;
	struct cavm_nixx_af_tl3x_red_s {
		u64 tail:9;		     /**< [  8:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_9:1;
		u64 head:9;		     /**< [ 18: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_tl3x_red_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_schedule
 *
 * NIX AF Transmit Level 4 Scheduling Control Registers
 * This register has the same bit fields as NIX_AF_TL2()_SCHEDULE.
 */
union cavm_nixx_af_tl4x_schedule {
	u64 u;
	struct cavm_nixx_af_tl4x_schedule_s {
		u64 rr_quantum:24;		     /**< [ 23:  0](R/W) Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned
                                                                 integer). The packet size used in all DWRR (RR_COUNT) calculations is:

                                                                 _  (NIX_nm_SHAPE[LENGTH_DISABLE] ? 0 : (NIX_nm_MD*[LENGTH] + NIX_nm_MD*[ADJUST]))
                                                                    + NIX_nm_SHAPE[ADJUST]

                                                                 where nm corresponds to this NIX_nm_SCHEDULE CSR.

                                                                 Typically [RR_QUANTUM] should be at or near the MTU or more (to limit or prevent
                                                                 negative accumulations of the deficit count). */
		u64 prio:4;		     /**< [ 27: 24](R/W) Priority. The priority used for this shaping queue in the (lower-level)
                                                                 parent's scheduling algorithm. When this shaping queue is not used, we
                                                                 recommend setting [PRIO] to zero. The legal [PRIO] values are zero to nine
                                                                 when the shaping queue is used. In addition to priority, [PRIO] determines
                                                                 whether the shaping queue is a static queue or not: If [PRIO] equals the
                                                                 parent's NIX_AF_TL*()_TOPOLOGY[RR_PRIO], then this is a round-robin child
                                                                 queue into the shaper at the next level. */
		u64 reserved_28_63:36;
	} s;
	/* struct cavm_nixx_af_tl4x_schedule_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_shape
 *
 * NIX AF Transmit Level 4 Shaping Control Registers
 * This register has the same bit fields as NIX_AF_TL2()_SHAPE.
 */
union cavm_nixx_af_tl4x_shape {
	u64 u;
	struct cavm_nixx_af_tl4x_shape_s {
		u64 adjust:9;		     /**< [  8:  0](R/W) See NIX_AF_TL2()_SHAPE[ADJUST]. */
		u64 red_algo:2;		     /**< [ 10:  9](R/W) See NIX_AF_TL2()_SHAPE[RED_ALGO]. */
		u64 red_disable:1;		     /**< [ 11: 11](R/W) See NIX_AF_TL2()_SHAPE[RED_DISABLE]. */
		u64 yellow_disable:1;	     /**< [ 12: 12](R/W) See NIX_AF_TL2()_SHAPE[YELLOW_DISABLE]. */
		u64 reserved_13_23:11;
		u64 length_disable:1;	     /**< [ 24: 24](R/W) Length disable. Disables the use of packet lengths in DWRR scheduling
                                                                 and shaping calculations such that only the value of [ADJUST] is used. */
		u64 schedule_list:2;	     /**< [ 26: 25](R/W) Shaper scheduling list. Restricts shaper scheduling to specific lists.
                                                                   0x0 = Normal (selected for nearly all scheduling/shaping applications).
                                                                   0x1 = Green-only.
                                                                   0x2 = Yellow-only.
                                                                   0x3 = Red-only. */
		u64 reserved_27_63:37;
	} s;
	/* struct cavm_nixx_af_tl4x_shape_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_cir
 *
 * NIX AF Transmit Level 4 Committed Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_tl4x_cir {
	u64 u;
	struct cavm_nixx_af_tl4x_cir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl4x_cir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_pir
 *
 * NIX AF Transmit Level 4 Peak Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_tl4x_pir {
	u64 u;
	struct cavm_nixx_af_tl4x_pir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl4x_pir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_sched_state
 *
 * NIX AF Transmit Level 4 Scheduling Control State Registers
 * This register has the same bit fields as NIX_AF_TL2()_SCHED_STATE.
 */
union cavm_nixx_af_tl4x_sched_state {
	u64 u;
	struct cavm_nixx_af_tl4x_sched_state_s {
		u64 rr_count:25;		     /**< [ 24:  0](R/W/H) Round-robin (DWRR) deficit counter. A 25-bit two's complement signed
                                                                 integer count. For diagnostic use. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_tl4x_sched_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_shape_state
 *
 * NIX AF Transmit Level 4 Shaping State Registers
 * This register has the same bit fields as NIX_AF_TL2()_SHAPE_STATE.
 * This register must not be written during normal operation.
 */
union cavm_nixx_af_tl4x_shape_state {
	u64 u;
	struct cavm_nixx_af_tl4x_shape_state_s {
		u64 cir_accum:26;		     /**< [ 25:  0](R/W/H) Committed information rate accumulator. Debug access to the live CIR accumulator. */
		u64 pir_accum:26;		     /**< [ 51: 26](R/W/H) Peak information rate accumulator. Debug access to the live PIR accumulator. */
		u64 color:2;		     /**< [ 53: 52](R/W/H) Shaper connection status. Debug access to the live shaper state.
                                                                 0x0 = Green - shaper is connected into the green list.
                                                                 0x1 = Yellow - shaper is connected into the yellow list.
                                                                 0x2 = Red - shaper is connected into the red list.
                                                                 0x3 = Pruned - shaper is disconnected. */
		u64 reserved_54_63:10;
	} s;
	/* struct cavm_nixx_af_tl4x_shape_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_pointers
 *
 * INTERNAL: NIX Transmit Level 4 Linked List Pointers Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL2()_POINTERS.
 */
union cavm_nixx_af_tl4x_pointers {
	u64 u;
	struct cavm_nixx_af_tl4x_pointers_s {
		u64 next:9;		     /**< [  8:  0](R/W/H) See NIX_AF_TL2()_POINTERS[NEXT]. */
		u64 reserved_9_15:7;
		u64 prev:9;		     /**< [ 24: 16](R/W/H) See NIX_AF_TL2()_POINTERS[PREV]. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_tl4x_pointers_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_sw_xoff
 *
 * NIX AF Transmit Level 4 Software Controlled XOFF Registers
 * This register has the same bit fields as NIX_AF_TL1()_SW_XOFF
 */
union cavm_nixx_af_tl4x_sw_xoff {
	u64 u;
	struct cavm_nixx_af_tl4x_sw_xoff_s {
		u64 xoff:1;		     /**< [  0:  0](R/W) XOFF. Stops meta flow out of the MDQ/TL* shaping queue. When [XOFF] is set,
                                                                 the corresponding meta descriptor in the MDQ/TL* shaping queue cannot be
                                                                 transferred to the next level. */
		u64 drain:1;		     /**< [  1:  1](WO) Drain. This control activates a drain path through the PSE that starts at this queue
                                                                 and ends at the TL1 level. The drain path is prioritized over other paths through PSE
                                                                 and can be used in combination with [DRAIN_IRQ]. [DRAIN] need never be set for the
                                                                 TL1 level, but is useful at all other levels, including the TL4 and MDQ levels.
                                                                 NIX_AF_GEN_INT[TL1_DRAIN] should be clear prior to initiating a [DRAIN]=1 write to
                                                                 this CSR.

                                                                 After [DRAIN] is set for a shaping queue, it should not be set again, for
                                                                 this or any other shaping queue, until NIX_AF_GEN_INT[TL1_DRAIN] is set.

                                                                 [DRAIN] must not be set for any shaping queue when an SMQ FLUSH command is
                                                                 active (any NIX_AF_SMQ()_CFG[FLUSH] is set).

                                                                 DRAIN has no effect unless [XOFF] is also set. Only one drain command is
                                                                 allowed to be active at a time. */
		u64 reserved_2:1;
		u64 drain_irq:1;		     /**< [  3:  3](WO) Drain IRQ. Enables setting of NIX_AF_GEN_INT[TL1_DRAIN] when the drain
                                                                 operation has completed.
                                                                 [DRAIN_IRQ] should be set whenever [DRAIN] is, and must not be set when [DRAIN] isn't
                                                                 set. [DRAIN_IRQ] has no effect unless [DRAIN] and [XOFF] are also set. */
		u64 reserved_4_63:60;
	} s;
	/* struct cavm_nixx_af_tl4x_sw_xoff_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_topology
 *
 * NIX AF Transmit Level 4 Topology Registers
 */
union cavm_nixx_af_tl4x_topology {
	u64 u;
	struct cavm_nixx_af_tl4x_topology_s {
		u64 reserved_0:1;
		u64 rr_prio:4;		     /**< [  4:  1](R/W) See NIX_AF_TL1()_TOPOLOGY[RR_PRIO]. */
		u64 reserved_5_31:27;
		u64 prio_anchor:9;		     /**< [ 40: 32](R/W) See NIX_AF_TL1()_TOPOLOGY[PRIO_ANCHOR]. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl4x_topology_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_parent
 *
 * NIX AF Transmit Level 4 Parent Registers
 */
union cavm_nixx_af_tl4x_parent {
	u64 u;
	struct cavm_nixx_af_tl4x_parent_s {
		u64 reserved_0_15:16;
		u64 parent:8;		     /**< [ 23: 16](R/W) See NIX_AF_TL2()_PARENT[PARENT]. */
		u64 reserved_24_63:40;
	} s;
	/* struct cavm_nixx_af_tl4x_parent_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_green
 *
 * INTERNAL: NIX Transmit Level 4 Green State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL3()_GREEN.
 */
union cavm_nixx_af_tl4x_green {
	u64 u;
	struct cavm_nixx_af_tl4x_green_s {
		u64 tail:9;		     /**< [  8:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_9:1;
		u64 head:9;		     /**< [ 18: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_19:1;
		u64 active_vec:20;		     /**< [ 39: 20](R/W/H) Active vector. A 10-bit vector, ordered by priority, that indicate which inputs to this
                                                                 scheduling queue are active. For internal use only. */
		u64 rr_active:1;		     /**< [ 40: 40](R/W/H) Round-robin red active. Indicates that the round-robin input is mapped to RED. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_tl4x_green_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_yellow
 *
 * INTERNAL: NIX Transmit Level 4 Yellow State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL3()_YELLOW
 */
union cavm_nixx_af_tl4x_yellow {
	u64 u;
	struct cavm_nixx_af_tl4x_yellow_s {
		u64 tail:9;		     /**< [  8:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_9:1;
		u64 head:9;		     /**< [ 18: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_tl4x_yellow_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_red
 *
 * INTERNAL: NIX Transmit Level 4 Red State Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL3()_YELLOW.
 */
union cavm_nixx_af_tl4x_red {
	u64 u;
	struct cavm_nixx_af_tl4x_red_s {
		u64 tail:9;		     /**< [  8:  0](R/W/H) Tail pointer. The index of round-robin linked-list tail. For internal use only. */
		u64 reserved_9:1;
		u64 head:9;		     /**< [ 18: 10](R/W/H) Head pointer. The index of round-robin linked-list head. For internal use only. */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_tl4x_red_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_sched_state
 *
 * NIX AF Meta Descriptor Queue Scheduling Control State Registers
 * This register has the same bit fields as NIX_AF_TL2()_SCHED_STATE.
 */
union cavm_nixx_af_mdqx_sched_state {
	u64 u;
	struct cavm_nixx_af_mdqx_sched_state_s {
		u64 rr_count:25;		     /**< [ 24:  0](R/W/H) Round-robin (DWRR) deficit counter. A 25-bit two's complement signed
                                                                 integer count. For diagnostic use. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_mdqx_sched_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_shape
 *
 * NIX AF Meta Descriptor Queue Shaping Control Registers
 * This register has the same bit fields as NIX_AF_TL3()_SHAPE.
 */
union cavm_nixx_af_mdqx_shape {
	u64 u;
	struct cavm_nixx_af_mdqx_shape_s {
		u64 adjust:9;		     /**< [  8:  0](R/W) See NIX_AF_TL2()_SHAPE[ADJUST]. */
		u64 red_algo:2;		     /**< [ 10:  9](R/W) See NIX_AF_TL2()_SHAPE[RED_ALGO]. */
		u64 red_disable:1;		     /**< [ 11: 11](R/W) See NIX_AF_TL2()_SHAPE[RED_DISABLE]. */
		u64 yellow_disable:1;	     /**< [ 12: 12](R/W) See NIX_AF_TL2()_SHAPE[YELLOW_DISABLE]. */
		u64 reserved_13_23:11;
		u64 length_disable:1;	     /**< [ 24: 24](R/W) Length disable. Disables the use of packet lengths in DWRR scheduling
                                                                 and shaping calculations such that only the value of [ADJUST] is used. */
		u64 schedule_list:2;	     /**< [ 26: 25](R/W) Shaper scheduling list. Restricts shaper scheduling to specific lists.
                                                                   0x0 = Normal (selected for nearly all scheduling/shaping applications).
                                                                   0x1 = Green-only.
                                                                   0x2 = Yellow-only.
                                                                   0x3 = Red-only. */
		u64 reserved_27_63:37;
	} s;
	/* struct cavm_nixx_af_mdqx_shape_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_cir
 *
 * NIX AF Meta Descriptor Queue Committed Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_mdqx_cir {
	u64 u;
	struct cavm_nixx_af_mdqx_cir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_mdqx_cir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_pir
 *
 * NIX AF Meta Descriptor Queue Peak Information Rate Registers
 * This register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union cavm_nixx_af_mdqx_pir {
	u64 u;
	struct cavm_nixx_af_mdqx_pir_s {
		u64 enable:1;		     /**< [  0:  0](R/W) Enable. Enables CIR shaping. */
		u64 rate_mantissa:8;	     /**< [  8:  1](R/W) Rate mantissa. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_exponent:4;	     /**< [ 12:  9](R/W) Rate exponent. See [RATE_DIVIDER_EXPONENT]. */
		u64 rate_divider_exponent:4;    /**< [ 16: 13](R/W) Rate divider exponent. This base-2 exponent is used to divide the
                                                                 data rate by specifying the number of time-wheel turns required before the
                                                                 rate accumulator is increased.

                                                                 The supported range for [RATE_DIVIDER_EXPONENT] is 0 to 12. Programmed
                                                                 values greater than 12 are treated as 12.

                                                                 The data rate in Mbits/sec is computed as follows:
                                                                 \<pre\>
                                                                 rate_div_exp = min(12, [RATE_DIVIDER_EXPONENT]);
                                                                 data_rate = 2 Mbps * (1.[RATE_MANTISSA] \<\< [RATE_EXPONENT]) / (1 \<\< rate_div_exp);
                                                                 \</pre\>

                                                                 Internal:
                                                                 Hardware generates a rate divider tick every 400 rst__gbl_100mhz_sclk_edge
                                                                 pulses, thus 100/400 = 0.25 MBytes/sec = 2 Mbps. This corresponds to a
                                                                 minimum of 1200 SCLK cycles per tick with SCLK \>= 300 MHz. Each TL2/TL3/TL4/MDQ
                                                                 samples its rate divider every 860 SCLK cycles and each TL1 samples every
                                                                 240 SCLK cycles, so the sample rates are  fast enough to keep up with the
                                                                 divider tick.

                                                                 Max rate = 2 Mbps * ((1 + (255/256)) \<\< 15) = 130 Gbps. */
		u64 reserved_17_28:12;
		u64 burst_mantissa:8;	     /**< [ 36: 29](R/W) Burst mantissa. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 burst_exponent:4;	     /**< [ 40: 37](R/W) Burst exponent. The burst limit is 1.[BURST_MANTISSA] \<\< ([BURST_EXPONENT] + 1).
                                                                 With [BURST_EXPONENT]=0xF and [BURST_MANTISSA]=0xFF, the burst limit is the largest
                                                                 possible value, which is 130,816 (0x1FF00) bytes. */
		u64 reserved_41_63:23;
	} s;
	/* struct cavm_nixx_af_mdqx_pir_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_sched_state
 *
 * NIX AF Meta Descriptor Queue Scheduling Control State Registers
 * This register has the same bit fields as NIX_AF_TL2()_SCHED_STATE.
 */
union cavm_nixx_af_mdqx_sched_state {
	u64 u;
	struct cavm_nixx_af_mdqx_sched_state_s {
		u64 rr_count:25;		     /**< [ 24:  0](R/W/H) Round-robin (DWRR) deficit counter. A 25-bit two's complement signed
                                                                 integer count. For diagnostic use. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_mdqx_sched_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_shape_state
 *
 * NIX AF Meta Descriptor Queue Shaping State Registers
 * This register has the same bit fields as NIX_AF_TL2()_SHAPE_STATE.
 * This register must not be written during normal operation.
 */
union cavm_nixx_af_mdqx_shape_state {
	u64 u;
	struct cavm_nixx_af_mdqx_shape_state_s {
		u64 cir_accum:26;		     /**< [ 25:  0](R/W/H) Committed information rate accumulator. Debug access to the live CIR accumulator. */
		u64 pir_accum:26;		     /**< [ 51: 26](R/W/H) Peak information rate accumulator. Debug access to the live PIR accumulator. */
		u64 color:2;		     /**< [ 53: 52](R/W/H) Shaper connection status. Debug access to the live shaper state.
                                                                 0x0 = Green - shaper is connected into the green list.
                                                                 0x1 = Yellow - shaper is connected into the yellow list.
                                                                 0x2 = Red - shaper is connected into the red list.
                                                                 0x3 = Pruned - shaper is disconnected. */
		u64 reserved_54_63:10;
	} s;
	/* struct cavm_nixx_af_mdqx_shape_state_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_pointers
 *
 * INTERNAL: NIX AF Meta Descriptor 4 Linked List Pointers Debug Register
 *
 * This register has the same bit fields as NIX_AF_TL4()_POINTERS.
 */
union cavm_nixx_af_mdqx_pointers {
	u64 u;
	struct cavm_nixx_af_mdqx_pointers_s {
		u64 next:9;		     /**< [  8:  0](R/W/H) See NIX_AF_TL2()_POINTERS[NEXT]. */
		u64 reserved_9_15:7;
		u64 prev:9;		     /**< [ 24: 16](R/W/H) See NIX_AF_TL2()_POINTERS[PREV]. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_mdqx_pointers_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_sw_xoff
 *
 * NIX AF Meta Descriptor Controlled XOFF Registers
 * This register has the same bit fields as NIX_AF_TL1()_SW_XOFF
 */
union cavm_nixx_af_mdqx_sw_xoff {
	u64 u;
	struct cavm_nixx_af_mdqx_sw_xoff_s {
		u64 xoff:1;		     /**< [  0:  0](R/W) XOFF. Stops meta flow out of the MDQ/TL* shaping queue. When [XOFF] is set,
                                                                 the corresponding meta descriptor in the MDQ/TL* shaping queue cannot be
                                                                 transferred to the next level. */
		u64 drain:1;		     /**< [  1:  1](WO) Drain. This control activates a drain path through the PSE that starts at this queue
                                                                 and ends at the TL1 level. The drain path is prioritized over other paths through PSE
                                                                 and can be used in combination with [DRAIN_IRQ]. [DRAIN] need never be set for the
                                                                 TL1 level, but is useful at all other levels, including the TL4 and MDQ levels.
                                                                 NIX_AF_GEN_INT[TL1_DRAIN] should be clear prior to initiating a [DRAIN]=1 write to
                                                                 this CSR.

                                                                 After [DRAIN] is set for a shaping queue, it should not be set again, for
                                                                 this or any other shaping queue, until NIX_AF_GEN_INT[TL1_DRAIN] is set.

                                                                 [DRAIN] must not be set for any shaping queue when an SMQ FLUSH command is
                                                                 active (any NIX_AF_SMQ()_CFG[FLUSH] is set).

                                                                 DRAIN has no effect unless [XOFF] is also set. Only one drain command is
                                                                 allowed to be active at a time. */
		u64 reserved_2:1;
		u64 drain_irq:1;		     /**< [  3:  3](WO) Drain IRQ. Enables setting of NIX_AF_GEN_INT[TL1_DRAIN] when the drain
                                                                 operation has completed.
                                                                 [DRAIN_IRQ] should be set whenever [DRAIN] is, and must not be set when [DRAIN] isn't
                                                                 set. [DRAIN_IRQ] has no effect unless [DRAIN] and [XOFF] are also set. */
		u64 reserved_4_63:60;
	} s;
	/* struct cavm_nixx_af_mdqx_sw_xoff_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_parent
 *
 * NIX AF Meta Descriptor Queue Topology Registers
 */
union cavm_nixx_af_mdqx_parent {
	u64 u;
	struct cavm_nixx_af_mdqx_parent_s {
		u64 reserved_0_15:16;
		u64 parent:9;		     /**< [ 24: 16](R/W) See NIX_AF_TL2()_PARENT[PARENT]. */
		u64 reserved_25_63:39;
	} s;
	/* struct cavm_nixx_af_mdqx_parent_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_tl2#_cfg
 *
 * NIX AF Transmit Level 3/2 Configuration Registers
 */
union cavm_nixx_af_tl3_tl2x_cfg {
	u64 u;
	struct cavm_nixx_af_tl3_tl2x_cfg_s {
		u64 express:1;		     /**< [  0:  0](R/W) Express.
                                                                 0 = This level 3 or level 2 shaping queue transmits normal packets
                                                                 only and uses NIX_AF_TX_LINK()_NORM_CREDIT registers for link credits.
                                                                 1 = This level 3 or level 2 shaping queue transmits express packets
                                                                 only and uses NIX_AF_TX_LINK()_EXPR_CREDIT registers for link credits. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_tl3_tl2x_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_tl2#_bp_status
 *
 * NIX AF Transmit Level 3/2 Backpressure Status Registers
 */
union cavm_nixx_af_tl3_tl2x_bp_status {
	u64 u;
	struct cavm_nixx_af_tl3_tl2x_bp_status_s {
		u64 hw_xoff:1;		     /**< [  0:  0](RO/H) Hardware XOFF status. Set if any of the channels mapped to this level 3 or level 2 queue
                                                                 with NIX_AF_TL3_TL2()_LINK()_CFG is backpressured with XOFF, or if any associated link
                                                                 is backpressured due to lack of credits (see NIX_AF_TX_LINK()_NORM_CREDIT,
                                                                 NIX_AF_TX_LINK()_EXPR_CREDIT). */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_tl3_tl2x_bp_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_tl2#_link#_cfg
 *
 * NIX AF Transmit Level 3/2 Link Configuration Registers
 * These registers specify the links and associated channels that a given TL3 or
 * TL2 queue (depending on NIX_AF_PSE_CHANNEL_LEVEL[BP_LEVEL]) can transmit on.
 * Each TL3/TL2 queue can be enabled to transmit on and be backpressured by one or
 * more links and associated channels.
 */
union cavm_nixx_af_tl3_tl2x_linkx_cfg {
	u64 u;
	struct cavm_nixx_af_tl3_tl2x_linkx_cfg_s {
		u64 relchan:8;		     /**< [  7:  0](R/W) Relative channel number within this link. See [ENA]. */
		u64 reserved_8_11:4;
		u64 ena:1;			     /**< [ 12: 12](R/W) Enable. When set, the TL3/TL2 queue can transmit on relative channel number
                                                                 [RELCHAN] of this link, and will respond to backpressure from link credits
                                                                 (NIX_AF_TX_LINK()_NORM_CREDIT, NIX_AF_TX_LINK()_EXPR_CREDIT). If
                                                                 [BP_ENA] is set, the queue also responds to channel backpressure. */
		u64 bp_ena:1;		     /**< [ 13: 13](R/W) Backpressure enable. When set, the TL3/TL2 queue responds to backpressure
                                                                 from (NIX_AF_TX_LINK()_HW_XOFF/_SW_XOFF[CHAN_XOFF]\<[RELCHAN]\>). */
		u64 reserved_14_63:50;
	} s;
	/* struct cavm_nixx_af_tl3_tl2x_linkx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_flow_key_alg#_field#
 *
 * NIX AF Receive Flow Key Algorithm Field Registers
 * A flow key algorithm defines how the 40-byte FLOW_KEY is formed from the received
 * packet header. FLOW_KEY is formed using up to five header fields (this register's
 * last index) with up to 16 bytes per field.
 *
 * The algorithm (index {a} (ALG) of these registers) is selected by
 * NIX_RX_ACTION_S[FLOW_KEY_ALG] from the packet's NPC_RESULT_S[ACTION].
 *
 * Internal:
 * 40-byte FLOW_KEY is wide enough to support an IPv6 5-tuple that includes a
 * VXLAN/GENEVE/NVGRE tunnel ID, e.g:
 * _ Source IP: 16B.
 * _ Dest IP: 16B.
 * _ Source port: 2B.
 * _ Dest port: 2B.
 * _ Tunnel VNI/VSI: 3B.
 * _ Total: 39B.
 */
union cavm_nixx_af_rx_flow_key_algx_fieldx {
	u64 u;
	struct cavm_nixx_af_rx_flow_key_algx_fieldx_s {
		u64 key_offset:6;		     /**< [  5:  0](R/W) Key offset. Starting byte offset of field in FLOW_KEY\<319:0\>. Bytes in
                                                                 FLOW_KEY are enumerated in network byte order as follows:
                                                                 Byte 0: FLOW_KEY\<319:312\>.
                                                                 Byte 1: FLOW_KEY\<311:304\>.
                                                                 ...
                                                                 Byte 39: FLOW_KEY\<7:0\>.

                                                                 For example, if [KEY_OFFSET] = 5, [BYTESM1] = 3:
                                                                 _ First header byte is written to FLOW_KEY\<279:272\>.
                                                                 _ Second header byte is written to FLOW_KEY\<271:264\>.
                                                                 _ Third header byte is written to FLOW_KEY\<263:256\>. */
		u64 ln_mask:1;		     /**< [  6:  6](R/W) Last nibble mask. When set, the least significant 4 bits of the last
                                                                 extracted header byte are written to FLOW_KEY as zeros. */
		u64 fn_mask:1;		     /**< [  7:  7](R/W) First nibble mask. When set, the most significant 4 bits of the first
                                                                 extracted header byte are written to FLOW_KEY as zeros. */
		u64 hdr_offset:8;		     /**< [ 15:  8](R/W) Header offset. Starting byte offset of field relative to the start
                                                                 of the header layer. For example, when [LID] = NPC_LID_E::LC, the header
                                                                 layer's NPC_LAYER_INFO_S = NPC_RESULT_S[LC], and the field's first byte is
                                                                 at the following byte offset from packet start:
                                                                 _ NPC_LAYER_INFO_S[LPTR] + [HDR_OFFSET].

                                                                 Not used when [SEL_CHAN] is set. */
		u64 bytesm1:4;		     /**< [ 19: 16](R/W) Field size in bytes minus one. 0x0=1 byte; 0x1=2 bytes, ..., 0xF=16 bytes.

                                                                 Must be 0x1 when [SEL_CHAN] is set. */
		u64 lid:3;			     /**< [ 22: 20](R/W) Layer ID of packet header. Enumerated by NPC_LID_E. Not used when [SEL_CHAN] is set. */
		u64 reserved_23:1;
		u64 ena:1;			     /**< [ 24: 24](R/W) Field extract enable. */
		u64 sel_chan:1;		     /**< [ 25: 25](R/W) Select channel number. When set, {4'h0, NIX_RX_PARSE_S[CHAN]\<11:0\>} is
                                                                 selected as the field data instead of the packet header data specified by
                                                                 [LID] and [HDR_OFFSET].
                                                                 [BYTESM1] must be 0x1 (2 bytes) when this bit is set. */
		u64 reserved_26_63:38;
	} s;
	/* struct cavm_nixx_af_rx_flow_key_algx_fieldx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_mcast#
 *
 * NIX AF Transmit Multicast Registers
 * These registers access transmit multicast table entries used to specify multicast replication
 * lists. Each list consists of linked entries with [EOL] = 1 in the last entry.
 *
 * A transmit packet is multicast when the action returned by NPC has NIX_TX_ACTION_S[OP] =
 * NIX_TX_ACTIONOP_E::MCAST. NIX_TX_ACTION_S[INDEX] points to the start of the multicast
 * replication list, and [EOL] = 1 indicates the end of list.
 */
union cavm_nixx_af_tx_mcastx {
	u64 u;
	struct cavm_nixx_af_tx_mcastx_s {
		u64 channel:12;		     /**< [ 11:  0](R/W) Transmit channel ID enumerated by NIX_CHAN_E. */
		u64 eol:1;			     /**< [ 12: 12](R/W) End of multicast replication list. */
		u64 reserved_13_15:3;
		u64 next:16;		     /**< [ 31: 16](R/W) Pointer to next NIX_AF_TX_MCAST() register in the multicast replication
                                                                 list. Valid when [EOL] is clear. */
		u64 reserved_32_63:32;
	} s;
	/* struct cavm_nixx_af_tx_mcastx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_vtag_def#_ctl
 *
 * NIX AF Transmit Vtag Definition Control Registers
 * The transmit Vtag definition table specifies Vtag layers (e.g. VLAN, E-TAG) to
 * optionally insert or replace in the TX packet header. Indexed by
 * NIX_TX_VTAG_ACTION_S[VTAG*_DEF].
 */
union cavm_nixx_af_tx_vtag_defx_ctl {
	u64 u;
	struct cavm_nixx_af_tx_vtag_defx_ctl_s {
		u64 size:3;		     /**< [  2:  0](R/W) Vtag size enumerated by NIX_VTAGSIZE_E. */
		u64 reserved_3_63:61;
	} s;
	/* struct cavm_nixx_af_tx_vtag_defx_ctl_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_vtag_def#_data
 *
 * NIX AF Transmit Vtag Definition Data Registers
 * See NIX_AF_TX_VTAG_DEF()_CTL.
 */
union cavm_nixx_af_tx_vtag_defx_data {
	u64 u;
	struct cavm_nixx_af_tx_vtag_defx_data_s {
		u64 data:64;		     /**< [ 63:  0](R/W) Vtag data formatted as follows:
                                                                 * If NIX_AF_TX_VTAG_DEF()_CTL[SIZE] = NIX_VTAGSIZE_E::T4:
                                                                   \<63:48\> Ethertype, \<47:32\> TCI, \<31:0\> Reserved.
                                                                 * If NIX_AF_TX_VTAG_DEF()_CTL[SIZE] = NIX_VTAGSIZE_E::T8:
                                                                   \<63:48\> Ethertype, \<47:0\> TCI. */
	} s;
	/* struct cavm_nixx_af_tx_vtag_defx_data_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_bpid#_status
 *
 * NIX AF Receive Backpressure ID Status Registers
 */
union cavm_nixx_af_rx_bpidx_status {
	u64 u;
	struct cavm_nixx_af_rx_bpidx_status_s {
		u64 aura_cnt:32;		     /**< [ 31:  0](R/W/H) Backpressure aura count. Number of auras that are backpressuring (XOFF) this BPID.

                                                                 Writes to this field are for diagnostic use only. The write data is a two's
                                                                 complement signed value added to the aura count. */
		u64 cq_cnt:32;		     /**< [ 63: 32](R/W/H) Backpressure CQ count. Number of completion queues that are backpressuring (XOFF) this
                                                                 BPID.

                                                                 Writes to this field are for diagnostic use only. The write data is a two's
                                                                 complement signed value added to the CQ count. */
	} s;
	/* struct cavm_nixx_af_rx_bpidx_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_chan#_cfg
 *
 * NIX AF Receive Channel Configuration Registers
 */
union cavm_nixx_af_rx_chanx_cfg {
	u64 u;
	struct cavm_nixx_af_rx_chanx_cfg_s {
		u64 bpid:9;		     /**< [  8:  0](R/W/H) BPID used to receive backpressure when [BP_ENA] is set. */
		u64 reserved_9_15:7;
		u64 bp_ena:1;		     /**< [ 16: 16](R/W/H) Backpressure enable. When set, the channel receives backpressure from [BPID]. */
		u64 sw_xoff:1;		     /**< [ 17: 17](R/W/H) Software XOFF. When set, backpressure is forced on the RX channel. */
		u64 imp:1;			     /**< [ 18: 18](RO/H) Implemented. This register is sparse (only indexes with values in NIX_CHAN_E which are
                                                                 implemented).
                                                                 0 = Channel is not implemented.
                                                                 1 = Channel is implemented.

                                                                 Write to a non-implemented channel is ignored. Reading a non-implemented channel returns
                                                                 all zero data. */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_rx_chanx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_cint_timer#
 *
 * NIX AF Completion Interrupt Timer Registers
 */
union cavm_nixx_af_cint_timerx {
	u64 u;
	struct cavm_nixx_af_cint_timerx_s {
		u64 expir_time:16;		     /**< [ 15:  0](RO/H) CINT expiration time. Updated as follows when the CINT timer is activated
                                                                 (see [ACTIVE]):
                                                                 _ [EXPIR_TIME] = NIX_AF_CINT_DELAY[CINT_TIMER] + NIX_LF_CINT()_WAIT[TIME_WAIT]

                                                                 When [ACTIVE] is set and NIX_AF_CINT_DELAY[CINT_TIMER] crosses
                                                                 [EXPIR_TIME], hardware sets the associated
                                                                 NIX_LF_CINT()_INT[INTR] bit and deactivates this CINT timer. */
		u64 cint:7;		     /**< [ 22: 16](RO/H) Completion interrupt within [LF] associated with this timer. */
		u64 reserved_23:1;
		u64 lf:8;			     /**< [ 31: 24](RO/H) Local function associated with this timer. */
		u64 active:1;		     /**< [ 32: 32](RO/H) This CINT timer is active and the remaining fields in this register
                                                                 are valid when this bit is set. See also [EXPIR_TIME].

                                                                 A CINT timer is activated when a CINT's NIX_LF_CINT()_CNT[ECOUNT]
                                                                 increments from zero, and deactivated when the CINT's
                                                                 NIX_LF_CINT()_INT[INTR] is set or NIX_LF_CINT()_CNT[ECOUNT] goes to zero. A
                                                                 timer is also activated when software writes a one to clear
                                                                 NIX_LF_CINT()_INT[INTR] and NIX_LF_CINT()_CNT[ECOUNT] is nonzero.

                                                                 When all CINT timers are active and a new timer needs to be activated, the
                                                                 least recently activated timer is removed and the associated
                                                                 NIX_LF_CINT()_INT[INTR] is set. */
		u64 reserved_33_63:31;
	} s;
	/* struct cavm_nixx_af_cint_timerx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lso_format#_field#
 *
 * NIX AF Large Send Offload Format Field Registers
 * These registers specify LSO packet modification formats. Each format may modify
 * up to eight packet fields with the following constraints:
 * * If fewer than eight fields are modified, [ALG] must be NIX_LSOALG_E::NOP in the
 * unused field registers.
 * * Modified fields must be specified in contiguous field registers starting with
 * NIX_AF_LSO_FORMAT()_FIELD(0).
 * * Modified fields cannot overlap.
 * * Multiple fields with the same [LAYER] value must be specified in
 * ascending [OFFSET] order.
 * * Fields in different layers must be specified in ascending [LAYER] order.
 */
union cavm_nixx_af_lso_formatx_fieldx {
	u64 u;
	struct cavm_nixx_af_lso_formatx_fieldx_s {
		u64 offset:8;		     /**< [  7:  0](R/W) Starting byte offset of the field relative to the first byte of [LAYER] in
                                                                 the packet header. */
		u64 layer:2;		     /**< [  9:  8](R/W) Header layer that contains the field, enumerated by NIX_TXLAYER_E. */
		u64 reserved_10_11:2;
		u64 sizem1:2;		     /**< [ 13: 12](R/W) Field size in bytes minus one. */
		u64 reserved_14_15:2;
		u64 alg:3;			     /**< [ 18: 16](R/W) Field modification algorithm enumerated by NIX_LSOALG_E. The remaining
                                                                 fields in the register are valid when value is not NIX_LSOALG_E::NOP. */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_lso_formatx_fieldx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_priv_lf#_cfg
 *
 * NIX Privileged Local Function Configuration Registers
 * These registers allow each NIX local function (LF) to be provisioned to a VF/PF
 * for RVU. See also NIX_PRIV_PF_CFG_DEBUG.
 *
 * [SLOT] must be zero.
 *
 * Internal:
 * Hardware ignores [SLOT] and always assumes 0x0.
 */
union cavm_nixx_priv_lfx_cfg {
	u64 u;
	struct cavm_nixx_priv_lfx_cfg_s {
		u64 slot:8;		     /**< [  7:  0](R/W) Slot within the VF/PF selected by [PF_FUNC] to which the LF is
                                                                 provisioned. */
		u64 pf_func:16;		     /**< [ 23:  8](R/W) RVU VF/PF to which the LF is provisioned. Format defined by RVU_PF_FUNC_S.
                                                                 Interrupts from the LF are delivered to the selected PF/VF. */
		u64 reserved_24_62:39;
		u64 ena:1;			     /**< [ 63: 63](R/W) Enable. When set, the LF is enabled and provisioned to the VF/PF slot
                                                                 selected by [PF_FUNC] and [SLOT]. When clear, the LF is not provisioned.

                                                                 LF to slot mapping must be 1-to-1. Thus, each enabled LF must be provisioned
                                                                 to a unique {[PF_FUNC], [SLOT]} combination. */
	} s;
	/* struct cavm_nixx_priv_lfx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_sqs_cfg
 *
 * NIX AF Local Function Send Queues Configuration Register
 * This register configures send queues in the LF.
 */
union cavm_nixx_af_lfx_sqs_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_sqs_cfg_s {
		u64 max_queuesm1:20;	     /**< [ 19:  0](R/W) Maximum number of queues minus one. */
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating queue context structures in NDC (1
                                                                 means do not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style of write and read for accessing queue context structures
                                                                 in LLC/DRAM:
                                                                 0 = Writes and reads of context data will not allocate into the LLC.
                                                                 1 = Writes and reads of context data are allocated into the LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_lfx_sqs_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_sqs_base
 *
 * NIX AF Local Function Send Queues Base Address Register
 * This register specifies the base AF IOVA of the LF's SQ context table.
 * The table consists of NIX_AF_LF()_SQS_CFG[MAX_QUEUESM1]+1 contiguous
 * NIX_SQ_CTX_HW_S structures.
 */
union cavm_nixx_af_lfx_sqs_base {
	u64 u;
	struct cavm_nixx_af_lfx_sqs_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_sqs_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rqs_cfg
 *
 * NIX AF Local Function Receive Queues Configuration Register
 * This register configures receive queues in the LF.
 */
union cavm_nixx_af_lfx_rqs_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_rqs_cfg_s {
		u64 max_queuesm1:20;	     /**< [ 19:  0](R/W) Maximum number of queues minus one. */
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating queue context structures in NDC (1
                                                                 means do not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style of write and read for accessing queue context structures
                                                                 in LLC/DRAM:
                                                                 0 = Writes and reads of context data will not allocate into the LLC.
                                                                 1 = Writes and reads of context data are allocated into the LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_lfx_rqs_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rqs_base
 *
 * NIX AF Local Function Receive Queues Base Address Register
 * This register specifies the base AF IOVA of the LF's RQ context table.
 * The table consists of NIX_AF_LF()_RQS_CFG[MAX_QUEUESM1]+1 contiguous
 * NIX_RQ_CTX_S structures.
 */
union cavm_nixx_af_lfx_rqs_base {
	u64 u;
	struct cavm_nixx_af_lfx_rqs_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_rqs_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cqs_cfg
 *
 * NIX AF Local Function Completion Queues Configuration Register
 * This register configures completion queues in the LF.
 */
union cavm_nixx_af_lfx_cqs_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_cqs_cfg_s {
		u64 max_queuesm1:20;	     /**< [ 19:  0](R/W) Maximum number of queues minus one. */
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating queue context structures in NDC (1
                                                                 means do not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style of write and read for accessing queue context structures
                                                                 in LLC/DRAM:
                                                                 0 = Writes and reads of context data will not allocate into the LLC.
                                                                 1 = Writes and reads of context data are allocated into the LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_lfx_cqs_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cqs_base
 *
 * NIX AF Local Function Completion Queues Base Address Register
 * This register specifies the base AF IOVA of the LF's CQ context table.
 * The table consists of NIX_AF_LF()_CQS_CFG[MAX_QUEUESM1]+1 contiguous
 * NIX_CQ_CTX_S structures.
 */
union cavm_nixx_af_lfx_cqs_base {
	u64 u;
	struct cavm_nixx_af_lfx_cqs_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_cqs_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_cfg
 *
 * NIX AF Local Function Transmit Configuration Register
 */
union cavm_nixx_af_lfx_tx_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_tx_cfg_s {
		u64 vlan0_ins_etype:16;	     /**< [ 15:  0](R/W) VLAN 0 Insert Ethertype. Ethertype of VLAN tag that is inserted when
                                                                 NIX_SEND_EXT_S[VLAN0_INS_ENA]=1. */
		u64 vlan1_ins_etype:16;	     /**< [ 31: 16](R/W) VLAN 1 Insert Ethertype. Ethertype of VLAN tag that is inserted when
                                                                 NIX_SEND_EXT_S[VLAN1_INS_ENA]=1. */
		u64 send_tstmp_ena:1;	     /**< [ 32: 32](R/W) Send timestamp enable. When set, the LF is allowed to request PTP
                                                                 timestamps in send packets. See NIX_SEND_EXT_S[TSTMP] and
                                                                 NIX_SENDMEMALG_E::SETTSTMP. */
		u64 lock_viol_cqe_ena:1;	     /**< [ 33: 33](R/W) Enable reporting of lockdown violations due to either of the following
                                                                 conditions:
                                                                 * [LOCK_ENA] is set and a violation is detected on packet data locked by
                                                                 NIX_AF_LF()_LOCK().
                                                                 * NIX_TX_ACTION_S[OP] = NIX_TX_ACTIONOP_E::DROP_VIOL. */
		u64 lock_ena:1;		     /**< [ 34: 34](R/W) Lockdown enable. When set, NIX_AF_LF()_LOCK() can be used to lock
                                                                 down one or more bits in packets transmitted by the LF. */
		u64 reserved_35_63:29;
	} s;
	/* struct cavm_nixx_af_lfx_tx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_parse_cfg
 *
 * NIX AF Local Function Transmit Parse Configuration Register
 */
union cavm_nixx_af_lfx_tx_parse_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_tx_parse_cfg_s {
		u64 pkind:6;		     /**< [  5:  0](R/W/H) Port kind supplied to NPC to seed the parsing of packets to be transmitted by this LF. */
		u64 reserved_6_63:58;
	} s;
	/* struct cavm_nixx_af_lfx_tx_parse_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_cfg
 *
 * NIX AF Local Function Receive Configuration Register
 */
union cavm_nixx_af_lfx_rx_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_rx_cfg_s {
		u64 reserved_0_32:33;
		u64 lenerr_en:1;		     /**< [ 33: 33](R/W) Outer L2 length error check enable. See NIX_RE_OPCODE_E::OL2_LENMISM. */
		u64 ip6_udp_opt:1;		     /**< [ 34: 34](R/W) IPv6/UDP checksum is optional. IPv4 allows an optional UDP checksum by
                                                                 sending the all-0s patterns. IPv6 outlaws this and the spec says to always
                                                                 check UDP checksum.
                                                                 0 = Spec compliant, do not allow all-0s IPv6/UDP checksum.
                                                                 1 = Treat IPv6 as IPv4; the all-0s pattern will cause a UDP checksum pass. */
		u64 dis_apad:1;		     /**< [ 35: 35](R/W) Disable alignment pad. When set, alignment padding is not added before the packet's
                                                                 first byte.

                                                                 When clear, enables alignment padding before the packet's first byte is written to
                                                                 either of the following (or both when NIX_RQ_CTX_S[XQE_IMM_COPY] is set):
                                                                 * The packet's first buffer (first NIX_RX_SG_S segment in WQE/CQE).
                                                                   The alignment pad size (APAD) is in this case added to the first segment's
                                                                   NIX_IOVA_S, i.e.  NIX_IOVA_S\<2:0\> = APAD.
                                                                 * Immediate data following NIX_RX_IMM_S.
                                                                   The alignment pad size is in this case captured in NIX_RX_IMM_S[APAD].

                                                                 The padding is calculated by the following algorithm:
                                                                 \<pre\>
                                                                 int nix_calc_alignment_pad(
                                                                       // Layer valids and pointers based on NPC_RESULT_S[LA,..,LH] matching
                                                                       // NIX_AF_RX_DEF_OIP4/OIP6/IIP6.
                                                                       bool oip4_valid,
                                                                       bool oip6_valid,
                                                                       bool iip6_valid,
                                                                       int oip4_ptr,
                                                                       int oip6_ptr,
                                                                       int iip6_ptr )
                                                                 {
                                                                    int APAD;
                                                                    if ([DIS_APAD])
                                                                       APAD = 0;
                                                                    else if (oip6_valid) // Outer IP.ver == 6
                                                                       APAD = (8 - oip6_ptr) & 0x7;
                                                                    else if (oip4_valid && iip6_valid) // Inner IP.ver == 6
                                                                       APAD = (8 - iip6_ptr) & 0x7;
                                                                    else if (oip4_valid)
                                                                       APAD = (4 - oip4_ptr) & 0x7;
                                                                    else
                                                                       APAD = 0;
                                                                    return APAD;
                                                                 }
                                                                 \</pre\> */
		u64 csum_il4:1;		     /**< [ 36: 36](R/W) Enable checking of inner L4 TCP/UDP/SCTP checksum. See
                                                                 NIX_RX_PERRCODE_E::IL4_CHK. */
		u64 csum_ol4:1;		     /**< [ 37: 37](R/W) Enable checking of outer L4 TCP/UDP/SCTP checksum. See
                                                                 NIX_RX_PERRCODE_E::OL4_CHK. */
		u64 len_il4:1;		     /**< [ 38: 38](R/W) Inner L4 UDP length error check enable. See NIX_RX_PERRCODE_E::IL4_LEN. */
		u64 len_il3:1;		     /**< [ 39: 39](R/W) Inner L3 length error check enable. See NIX_RX_PERRCODE_E::IL3_LEN. */
		u64 len_ol4:1;		     /**< [ 40: 40](R/W) Outer L4 UDP length error check enable. See NIX_RX_PERRCODE_E::OL4_LEN. */
		u64 len_ol3:1;		     /**< [ 41: 41](R/W) Outer L3 length error check enable. See NIX_RX_PERRCODE_E::OL3_LEN. */
		u64 reserved_42_63:22;
	} s;
	/* struct cavm_nixx_af_lfx_rx_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rss_cfg
 *
 * NIX AF Local Function Receive Size Scaling Table Configuration Register
 * See NIX_AF_LF()_RSS_BASE and NIX_AF_LF()_RSS_GRP().
 */
union cavm_nixx_af_lfx_rss_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_rss_cfg_s {
		u64 size:4;		     /**< [  3:  0](R/W) Specifies table size in NIX_RSSE_S entries of four bytes when [ENA] is set:
                                                                 0x0 = 256 entries.
                                                                 0x1 = 512 entries.
                                                                 0x2 = 1K entries.
                                                                 0x3 = 2K entries.
                                                                 0x4-0xF = Reserved. */
		u64 ena:1;			     /**< [  4:  4](R/W) RSS is enabled for the LF. */
		u64 reserved_5_19:15;
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating NIX_RSSE_S structures in NDC (1 means
                                                                 do not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style of read for accessing NIX_RSSE_S structures in LLC/DRAM:
                                                                 0 = NIX_RSSE_S reads will not allocate into the LLC.
                                                                 1 = NIX_RSSE_S reads are allocated into the LLC.

                                                                 NIX_RX_MCE_S writes that are not allocated in NDC will always allocate
                                                                 into LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_lfx_rss_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rss_base
 *
 * NIX AF Local Function Receive Size Scaling Table Base Address Register
 * This register specifies the base AF IOVA of the RSS table per LF. The
 * table is present when NIX_AF_LF()_RSS_CFG[ENA] is set and consists of
 * 2^(NIX_AF_LF()_RSS_CFG[SIZE]+8) contiguous NIX_RSSE_S structures, where the
 * size of each structure is 1 \<\< NIX_AF_CONST3[RSSE_LOG2BYTES] bytes.
 * See NIX_AF_LF()_RSS_GRP().
 */
union cavm_nixx_af_lfx_rss_base {
	u64 u;
	struct cavm_nixx_af_lfx_rss_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_rss_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_qints_cfg
 *
 * NIX AF Local Function Queue Interrupts Configuration Registers
 * This register controls access to the LF's queue interrupt context table in
 * NDC/LLC/DRAM. The table consists of NIX_AF_CONST2[QINTS] contiguous NIX_QINT_HW_S
 * structures. The size of each structure is 1 \<\< NIX_AF_CONST3[QINT_LOG2BYTES]
 * bytes.
 */
union cavm_nixx_af_lfx_qints_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_qints_cfg_s {
		u64 reserved_0_19:20;
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating context structures in NDC (1 means do
                                                                 not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style read for accessing context structures in LLC/DRAM:
                                                                 0 = Context structure reads will not allocate into the LLC.
                                                                 1 = Context structure reads are allocated into the LLC.

                                                                 Context structure writes that are not allocated in NDC will always allocate
                                                                 into LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_lfx_qints_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_qints_base
 *
 * NIX AF Local Function Queue Interrupts Base Address Registers
 * This register specifies the base AF IOVA of LF's queue interrupt context
 * table in NDC/LLC/DRAM. The table consists of NIX_AF_CONST2[QINTS] contiguous
 * NIX_QINT_HW_S structures.
 */
union cavm_nixx_af_lfx_qints_base {
	u64 u;
	struct cavm_nixx_af_lfx_qints_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_qints_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cints_cfg
 *
 * NIX AF Local Function Completion Interrupts Configuration Registers
 * This register controls access to the LF's completion interrupt context table in
 * NDC/LLC/DRAM. The table consists of NIX_AF_CONST2[CINTS] contiguous NIX_CINT_HW_S
 * structures. The size of each structure is 1 \<\< NIX_AF_CONST3[CINT_LOG2BYTES]
 * bytes.
 */
union cavm_nixx_af_lfx_cints_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_cints_cfg_s {
		u64 reserved_0_19:20;
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating context structures in NDC (1 means do
                                                                 not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style read for accessing context structures in LLC/DRAM:
                                                                 0 = Context structure reads will not allocate into the LLC.
                                                                 1 = Context structure reads are allocated into the LLC.

                                                                 Context structure writes that are not allocated in NDC will always allocate
                                                                 into LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_lfx_cints_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cints_base
 *
 * NIX AF Local Function Completion Interrupts Base Address Registers
 * This register specifies the base AF IOVA of LF's completion interrupt
 * context table in NDC/LLC/DRAM. The table consists of NIX_AF_CONST2[CINTS]
 * contiguous NIX_CINT_HW_S structures.
 */
union cavm_nixx_af_lfx_cints_base {
	u64 u;
	struct cavm_nixx_af_lfx_cints_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_cints_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_cfg0
 *
 * NIX AF LF Receive IPSEC Configuration Registers
 */
union cavm_nixx_af_lfx_rx_ipsec_cfg0 {
	u64 u;
	struct cavm_nixx_af_lfx_rx_ipsec_cfg0_s {
		u64 lenm1_max:14;		     /**< [ 13:  0](R/W) Maximum length in bytes (minus 1) of a packet that may use the IPSEC
                                                                 hardware fast-path. */
		u64 sa_pow2_size:4;	     /**< [ 17: 14](R/W) Power of 2 size of IPSEC SA structure used by CPT:
                                                                 0x0-0x4 = Reserved.
                                                                 0x5 = 32 bytes.
                                                                 0x6 = 64 bytes.
                                                                 0x7 = 128 bytes.
                                                                 0x8 = 256 bytes.
                                                                 0x9 = 512 bytes.
                                                                 0xA-0xF = Reserved. */
		u64 reserved_18_39:22;
		u64 defcpt:1;		     /**< [ 40: 40](RAZ) Default CPT index. Always zero. */
		u64 hshcpt:1;		     /**< [ 41: 41](RAZ) Hash CPT index. Always zero. */
		u64 reserved_42_63:22;
	} s;
	/* struct cavm_nixx_af_lfx_rx_ipsec_cfg0_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_cfg1
 *
 * NIX AF LF Receive IPSEC Security Association Configuration Register
 */
union cavm_nixx_af_lfx_rx_ipsec_cfg1 {
	u64 u;
	struct cavm_nixx_af_lfx_rx_ipsec_cfg1_s {
		u64 sa_idx_max:32;		     /**< [ 31:  0](R/W) Maximum SA index recognized by hardware for the LF. See [SA_IDX_W]. */
		u64 tag_const:24;		     /**< [ 55: 32](R/W) Constant value ORed into NIX_WQE_HDR_S[TAG]\<31:8\> for IPSEC fast-path packets
                                                                 (NIX_WQE_HDR_S[WQE_TYPE] = NIX_XQE_TYPE_E::RX_IPSECH, NIX_XQE_TYPE_E::RX_IPSECD,
                                                                 or NIX_XQE_TYPE_E::RX_IPSECS). */
		u64 tt:2;			     /**< [ 57: 56](R/W) SSO tag type to load to NIX_WQE_HDR_S[TT] for IPSEC fast-path packets
                                                                 (NIX_WQE_HDR_S[WQE_TYPE] = NIX_XQE_TYPE_E::RX_IPSECH, NIX_XQE_TYPE_E::RX_IPSECD,
                                                                 or NIX_XQE_TYPE_E::RX_IPSECS). */
		u64 sa_idx_w:5;		     /**< [ 62: 58](R/W) Security association index width. Number of lower bits from the SPI field
                                                                 of an IPSEC packet that provide the packet's SA index. The SA index is
                                                                 computed as follows:
                                                                 \<pre\>
                                                                 SPI\<31:0\> = packet's 32-bit IPSEC SPI field; // see NIX_AF_RX_DEF_IPSEC()
                                                                 SA_index = SPI & ((1 \<\< [SA_IDX_W]) - 1);
                                                                 \</pre\>

                                                                 If the packet's SA index is greater than [SA_IDX_MAX], the packet uses the
                                                                 IPSEC software fast-path (NIX_WQE_HDR_S[WQE_TYPE]/NIX_CQE_HDR_S[CQE_TYPE] =
                                                                 NIX_XQE_TYPE_E::RX_IPSECS). */
		u64 reserved_63:1;
	} s;
	/* struct cavm_nixx_af_lfx_rx_ipsec_cfg1_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_dyno_cfg
 *
 * NIX AF LF Receive IPSEC Dynamic Ordering Base Address Registers
 */
union cavm_nixx_af_lfx_rx_ipsec_dyno_cfg {
	u64 u;
	struct cavm_nixx_af_lfx_rx_ipsec_dyno_cfg_s {
		u64 dyno_idx_w:4;		     /**< [  3:  0](R/W) Dynamic ordering counter index width. When [DYNO_ENA]==1, specifies the
                                                                 number of lower bits of an IPSEC packet's SA index used to select the DYNO
                                                                 counter for the packet. Must not be greater than
                                                                 NIX_AF_LF()_RX_IPSEC_CFG1[SA_IDX_W]. See [DYNO_ENA]. */
		u64 dyno_ena:1;		     /**< [  4:  4](R/W) Dynamic ordering enable. When set, enables dynamic ordering (DYNO) counters
                                                                 used to enforce ordering between IPSEC hardware fast-path packets
                                                                 (NIX_WQE_HDR_S[WQE_TYPE] = NIX_XQE_TYPE_E::RX_IPSECH) and dynamically
                                                                 prevented packets (NIX_WQE_HDR_S[WQE_TYPE] = NIX_XQE_TYPE_E::RX_IPSECD)
                                                                 within a given flow.

                                                                 When set, hardware maintains 1 \<\< [DYNO_IDX_W] DYNO counters in
                                                                 NDC/LLC/DRAM starting at address NIX_AF_LF()_RX_IPSEC_DYNO_BASE, where the
                                                                 size of each counter is 1 \<\< NIX_AF_CONST3[DYNO_LOG2BYTES] bytes.

                                                                 The DYNO counter index for an IPSEC hardware fast-path or a dynamically
                                                                 prevented packet is obtained from the lower [DYNO_IDX_W] bits of the
                                                                 packet's IPSEC SA index. See NIX_AF_LF()_RX_IPSEC_CFG1[SA_IDX_W].

                                                                 Hardware increments the selected DYNO counter by one for each dynamically
                                                                 prevented packet. Software decrements the counter by writing to
                                                                 NIX_LF_OP_IPSEC_DYNO_CNT. Hardware suppresses the IPSEC hardware fast-path
                                                                 when the counter is non-zero. */
		u64 reserved_5_19:15;
		u64 way_mask:16;		     /**< [ 35: 20](R/W) Way partitioning mask for allocating NIX_RSSE_S structures in NDC (1 means
                                                                 do not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
		u64 caching:1;		     /**< [ 36: 36](R/W) Selects the style of read for accessing dynamic ordering counters in
                                                                 LLC/DRAM:
                                                                 0 = DYNO counter reads will not allocate into the LLC.
                                                                 1 = DYNO counter reads are allocated into the LLC.

                                                                 DYNO counter writes that are not allocated in NDC will always allocate into
                                                                 LLC. */
		u64 reserved_37_63:27;
	} s;
	/* struct cavm_nixx_af_lfx_rx_ipsec_dyno_cfg_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_dyno_base
 *
 * NIX AF LF Receive IPSEC Dynamic Ordering Base Address Registers
 * This register specifies the base AF IOVA of LF's dynamic ordering
 * (DYNO) counter table in NDC/LLC/DRAM. The table consists of
 * 1 \<\< (NIX_AF_LF()_RX_IPSEC_DYNO_CFG[DYNO_IDX_W]) counters. The size of each
 * counter is 1 \<\< NIX_AF_CONST3[DYNO_LOG2BYTES] bytes.
 * See NIX_AF_LF()_RX_IPSEC_DYNO_CFG[DYNO_ENA].
 */
union cavm_nixx_af_lfx_rx_ipsec_dyno_base {
	u64 u;
	struct cavm_nixx_af_lfx_rx_ipsec_dyno_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_rx_ipsec_dyno_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_sa_base
 *
 * NIX AF LF Receive IPSEC Security Association Base Address Register
 * This register specifies the base IOVA of CPT's IPSEC SA table in LLC/DRAM.
 */
union cavm_nixx_af_lfx_rx_ipsec_sa_base {
	u64 u;
	struct cavm_nixx_af_lfx_rx_ipsec_sa_base_s {
		u64 reserved_0_6:7;
		u64 addr:46;		     /**< [ 52:  7](R/W) Base AF IOVA of table in NDC/LLC/DRAM. IOVA bits \<6:0\> are always
                                                                 zero. */
		u64 reserved_53_63:11;
	} s;
	/* struct cavm_nixx_af_lfx_rx_ipsec_sa_base_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_status
 *
 * NIX AF LF Transmit Status Register
 */
union cavm_nixx_af_lfx_tx_status {
	u64 u;
	struct cavm_nixx_af_lfx_tx_status_s {
		u64 sq_ctx_err:1;		     /**< [  0:  0](R/W1C/H) SQ context error. An error was detected on a NIX_SQ_CTX_S read. When set:
                                                                 * Hardware drops LMT stores to NIX_LF_OP_SEND().
                                                                 * Hardware stops enqueueing MDs to SMQs assigned to the LF
                                                                 (NIX_AF_SMQ()_CFG[LF]).

                                                                 Software can clear this bit by writing a one. */
		u64 reserved_1_63:63;
	} s;
	/* struct cavm_nixx_af_lfx_tx_status_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_vtag_type#
 *
 * NIX AF Local Function Receive Vtag Type Registers
 * These registers specify optional Vtag (e.g. VLAN, E-TAG) actions for received
 * packets. Indexed by NIX_RX_VTAG_ACTION_S[VTAG*_TYPE].
 */
union cavm_nixx_af_lfx_rx_vtag_typex {
	u64 u;
	struct cavm_nixx_af_lfx_rx_vtag_typex_s {
		u64 size:3;		     /**< [  2:  0](R/W) Vtag size enumerated by NIX_VTAGSIZE_E. */
		u64 reserved_3:1;
		u64 strip:1;		     /**< [  4:  4](R/W) When set, the Vtag is stripped from the received packet header. Note that the
                                                                 Vtag is silently stripped if [STRIP] is set and [CAPTURE] is clear. */
		u64 capture:1;		     /**< [  5:  5](R/W) When set, Vtag's information is captured in NIX_RX_PARSE_S[VTAG*]. */
		u64 reserved_6_63:58;
	} s;
	/* struct cavm_nixx_af_lfx_rx_vtag_typex_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_lock#
 *
 * NIX AF Local Function Lockdown Registers
 * Internal:
 * The NIX lockdown depth of 32 bytes is shallow compared to 96 bytes for NIC and meant for outer
 * MAC and/or VLAN (optionally preceded by a small number of skip bytes). NPC's MCAM can be used
 * for deeper protocol-aware lockdown.
 */
union cavm_nixx_af_lfx_lockx {
	u64 u;
	struct cavm_nixx_af_lfx_lockx_s {
		u64 data:32;		     /**< [ 31:  0](R/W) Lockdown data. If corresponding [BIT_ENA] is set and
                                                                 NIX_AF_LF()_TX_CFG[LOCK_ENA] is set, outbound packet data must match the
                                                                 [DATA] bit or the packet will be dropped. Bytes are numbered in little
                                                                 endian form, with byte 0 the first byte onto the wire:
                                                                 _ If LOCK(0)[BIT_ENA]\<7:0\> set, LOCK(0)[DATA]\<7:0\> = packet byte 0.
                                                                 _ If LOCK(0)[BIT_ENA]\<15:8\> set, LOCK(0)[DATA]\<15:8\> = packet byte 1.
                                                                 _ If LOCK(0)[BIT_ENA]\<23:16\> set, LOCK(0)[DATA]\<23:16\> = packet byte 2.
                                                                 _ ...
                                                                 _ If LOCK(1)[BIT_ENA]\<7:0\> set, LOCK(1)[DATA]\<7:0\> = packet byte 4.
                                                                 _ ...
                                                                 _ If LOCK(7)[BIT_ENA]\<31:24\> set, LOCK(7)[DATA]\<31:24\> = packet byte 31.

                                                                 Lockdown data is checked after all packet modifications by hardware other
                                                                 than checksum/CRC updates. These modifications include potential VLAN
                                                                 insertion, packet shaper marking, LSO modifications and Vtag
                                                                 insertion/replacement.

                                                                 In addition, if any checksum/CRC bits updated by NIX_SEND_HDR_S[CKL*]
                                                                 and/or NIX_SEND_CRC_S are locked down, a lockdown violation is detected and
                                                                 the packet is dropped. */
		u64 bit_ena:32;		     /**< [ 63: 32](R/W) Lockdown bit enable. Each set bit indicates that the transmitted packet's corresponding
                                                                 bit number will be compared against [DATA]. */
	} s;
	/* struct cavm_nixx_af_lfx_lockx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_stat#
 *
 * NIX AF Local Function Transmit Statistics Registers
 * The last dimension indicates which statistic, and is enumerated by NIX_STAT_LF_TX_E.
 */
union cavm_nixx_af_lfx_tx_statx {
	u64 u;
	struct cavm_nixx_af_lfx_tx_statx_s {
		u64 stat:48;		     /**< [ 47:  0](R/W/H) Statistic value. See also NIX_LF_TX_STAT() for a read-only alias of this
                                                                 field. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_lfx_tx_statx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_stat#
 *
 * NIX AF Local Function Receive Statistics Registers
 * The last dimension indicates which statistic, and is enumerated by NIX_STAT_LF_RX_E.
 */
union cavm_nixx_af_lfx_rx_statx {
	u64 u;
	struct cavm_nixx_af_lfx_rx_statx_s {
		u64 stat:48;		     /**< [ 47:  0](R/W/H) Statistic value. See also NIX_LF_RX_STAT() for a read-only alias of this
                                                                 field. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_lfx_rx_statx_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rss_grp#
 *
 * NIX AF Local Function Receive Side Scaling Group Registers
 * A receive packet targets a LF's RSS group when its NIX_RX_ACTION_S[OP] =
 * NIX_RX_ACTIONOP_E::RSS, or its target multicast list has an entry with
 * NIX_RX_MCE_S[OP] = NIX_RX_MCOP_E::RSS. The RSS group index (this register's last
 * index) is NIX_RX_ACTION_S[INDEX] or NIX_RX_MCE_S[INDEX].
 *
 * The RSS computation is as follows:
 * * The packet's flow_tag (see NIX_LF_RX_SECRET()) and RSS group are used to
 * select a NIX_RSSE_S entry in the LF's RSS table (see [SIZEM1]).
 * * NIX_RSSE_S selects the packet's destination RQ.
 */
union cavm_nixx_af_lfx_rss_grpx {
	u64 u;
	struct cavm_nixx_af_lfx_rss_grpx_s {
		u64 offset:11;		     /**< [ 10:  0](R/W) Offset (number of four-byte NIX_RSSE_S structures) into RSS table from
                                                                 NIX_AF_LF()_RSS_BASE. See [SIZEM1]. */
		u64 reserved_11_15:5;
		u64 sizem1:3;		     /**< [ 18: 16](R/W) Number of RSS adder bits minus one to add in RSS calculation.
                                                                 0x0 = rss_adder\<0\> included in RSS.
                                                                 0x1 = rss_adder\<1:0\> included in RSS.
                                                                 0x2 = rss_adder\<2:0\> included in RSS.
                                                                 0x3 = rss_adder\<3:0\> included in RSS.
                                                                 0x4 = rss_adder\<4:0\> included in RSS.
                                                                 0x5 = rss_adder\<5:0\> included in RSS.
                                                                 0x6 = rss_adder\<6:0\> included in RSS.
                                                                 0x7 = rss_adder\<7:0\> included in RSS.

                                                                 where:

                                                                 _ rss_adder\<7:0\> = flow_tag\<7:0\> ^ flow_tag\<15:8\> ^ flow_tag\<23:16\> ^ flow_tag\<31:24\>

                                                                 The AF IOVA of the packet's final NIX_RSSE_S structure is computed as follows:
                                                                 \<pre\>
                                                                 rsse_offset = ([OFFSET] + rss_adder[\<[SIZEM1]:0\>]) % (1 \<\< (NIX_AF_LF()_RSS_CFG[SIZE] + 8));
                                                                 rsse_iova = NIX_AF_LF()_RSS_BASE + 4*rsse_offset;
                                                                 \</pre\> */
		u64 reserved_19_63:45;
	} s;
	/* struct cavm_nixx_af_lfx_rss_grpx_s cn; */
};

***Register(RVU_PF_BAR0) nix
#_af_rx_npc_mc_rcv
    * *NIX AF Multicast Receive Statistics Register * The counter increments for every
recieved MC packet marked by the NPC. * /union cavm_nixx_af_rx_npc_mc_rcv {
	u64 u;
	struct cavm_nixx_af_rx_npc_mc_rcv_s {
		u64 stat:48;		     /**< [ 47:  0](R/W/H) Statistic value. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_rx_npc_mc_rcv_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_npc_mc_drop
 *
 * NIX AF Multicast Drop Statistics Register
 * The counter increments for every dropped MC packet marked by the NPC.
 */
union cavm_nixx_af_rx_npc_mc_drop {
	u64 u;
	struct cavm_nixx_af_rx_npc_mc_drop_s {
		u64 stat:48;		     /**< [ 47:  0](R/W/H) Statistic value. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_rx_npc_mc_drop_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_npc_mirror_rcv
 *
 * NIX AF Mirror Receive Statistics Register
 * The counter increments for every recieved MIRROR packet marked by the NPC.
 */
union cavm_nixx_af_rx_npc_mirror_rcv {
	u64 u;
	struct cavm_nixx_af_rx_npc_mirror_rcv_s {
		u64 stat:48;		     /**< [ 47:  0](R/W/H) Statistic value. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_rx_npc_mirror_rcv_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_npc_mirror_drop
 *
 * NIX AF Mirror Drop Statistics Register
 * The counter increments for every dropped MIRROR packet marked by the NPC.
 */
union cavm_nixx_af_rx_npc_mirror_drop {
	u64 u;
	struct cavm_nixx_af_rx_npc_mirror_drop_s {
		u64 stat:48;		     /**< [ 47:  0](R/W/H) Statistic value. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_af_rx_npc_mirror_drop_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rx_secret#
 *
 * NIX LF Receive Secret Key Registers
 */
union cavm_nixx_lf_rx_secretx {
	u64 u;
	struct cavm_nixx_lf_rx_secretx_s {
		u64 key:64;		     /**< [ 63:  0](R/W) 64 bits of the 352-bit VF secret key for flow key hashing. Software must set
                                                                 this to a random secret value shared with the driver.

                                                                 The FLOW_KEY\<319:0\> is generated for the packet as described in
                                                                 NIX_AF_RX_FLOW_KEY_ALG()_FIELD(). The following hash function computes flow_tag\<31:0\>
                                                                 from FLOW_KEY:
                                                                 \<pre\>
                                                                   uint32_t nix_rx_flow_tag_hash(bit_array\<319:0\> FLOW_KEY) {
                                                                      uint32_t flow_tag = 0;
                                                                      bit_array\<351:0\> secret;
                                                                      secret\<351:288\> = NIX_LF_RX_SECRET(0);
                                                                      secret\<287:224\> = NIX_LF_RX_SECRET(1);
                                                                      secret\<223:160\> = NIX_LF_RX_SECRET(2);
                                                                      secret\<159:96\> = NIX_LF_RX_SECRET(3);
                                                                      secret\<95:32\> = NIX_LF_RX_SECRET(4);
                                                                      secret\<31:0\> = NIX_LF_RX_SECRET(5)[KEY]\<63:32\>;
                                                                      // Note NIX_LF_RX_SECRET(5)[KEY]\<31:0\> bits are unused.
                                                                      secret_bit = 351;
                                                                      foreach bit_value (FLOW_KEY) { // MSB processed first
                                                                         assert(secret_bit\>=31);
                                                                         if (bit_value) {
                                                                            flow_tag ^= secret\<secret_bit:(secret_bit-31)\>;
                                                                         }
                                                                         secret_bit--;
                                                                      }
                                                                      return flow_tag;
                                                                   }
                                                                 \</pre\>

                                                                 If bidirectional flows are desired to land on the same queue, a secret key that
                                                                 provides a symmetric hash can be selected. The hash is symmetric for two fields of
                                                                 width W starting at MSB positions P1 and P2 in FLOW_KEY\<319:0\> if the secret key
                                                                 satisfies:
                                                                 _ secret\<(P1+32):(P1-W+1)\> = secret\<(P2+32):(P2-W+1)\>

                                                                 For example, the hash will be symmetric for source and destination IPv4 addresses at
                                                                 FLOW_KEY\<87:56\> and FLOW_KEY\<55:24\> when secret\<119:56\> = secret\<87:24\>.

                                                                 For L3/L4 flows (e.g. TCP/UDP/SCTP over IPv4 or IPv6) with 16-bit aligned L3/L4
                                                                 source/destination fields in FLOW_KEY, a secret key with a repeating 16-bit pattern
                                                                 will provide a symmetric hash. */
	} s;
	/* struct cavm_nixx_lf_rx_secretx_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint
 *
 * NIX LF General Interrupt Register
 */
union cavm_nixx_lf_gint {
	u64 u;
	struct cavm_nixx_lf_gint_s {
		u64 drop:1;		     /**< [  0:  0](R/W1C/H) Packet dropped interrupt. Set when any packet has been dropped. This is intended for
                                                                 diagnostic use; typical production software will want this interrupt disabled. */
		u64 tcp_timer:1;		     /**< [  1:  1](R/W1C/H) TCP timer interrupt. Enabled when NIX_AF_TCP_TIMER[ENA] and
                                                                 NIX_LF_CFG[TCP_TIMER_INT_ENA] are both set. Set every
                                                                 NIX_AF_TCP_TIMER[DURATION]*256*128 coprocessor cycles when enabled. */
		u64 reserved_2_63:62;
	} s;
	/* struct cavm_nixx_lf_gint_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint_w1s
 *
 * NIX LF General Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_nixx_lf_gint_w1s {
	u64 u;
	struct cavm_nixx_lf_gint_w1s_s {
		u64 drop:1;		     /**< [  0:  0](R/W1S/H) Reads or sets NIX_LF_GINT[DROP]. */
		u64 tcp_timer:1;		     /**< [  1:  1](R/W1S/H) Reads or sets NIX_LF_GINT[TCP_TIMER]. */
		u64 reserved_2_63:62;
	} s;
	/* struct cavm_nixx_lf_gint_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint_ena_w1c
 *
 * NIX LF General Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_lf_gint_ena_w1c {
	u64 u;
	struct cavm_nixx_lf_gint_ena_w1c_s {
		u64 drop:1;		     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_LF_GINT[DROP]. */
		u64 tcp_timer:1;		     /**< [  1:  1](R/W1C/H) Reads or clears enable for NIX_LF_GINT[TCP_TIMER]. */
		u64 reserved_2_63:62;
	} s;
	/* struct cavm_nixx_lf_gint_ena_w1c_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint_ena_w1s
 *
 * NIX LF General Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_lf_gint_ena_w1s {
	u64 u;
	struct cavm_nixx_lf_gint_ena_w1s_s {
		u64 drop:1;		     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_LF_GINT[DROP]. */
		u64 tcp_timer:1;		     /**< [  1:  1](R/W1S/H) Reads or sets enable for NIX_LF_GINT[TCP_TIMER]. */
		u64 reserved_2_63:62;
	} s;
	/* struct cavm_nixx_lf_gint_ena_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int
 *
 * NIX LF Error Interrupt Register
 */
union cavm_nixx_lf_err_int {
	u64 u;
	struct cavm_nixx_lf_err_int_s {
		u64 sqb_fault:1;		     /**< [  0:  0](R/W1C/H) Memory fault on SQB read or write. */
		u64 sq_ctx_fault:1;	     /**< [  1:  1](R/W1C/H) Memory fault on NIX_SQ_CTX_HW_S read or write. */
		u64 rq_ctx_fault:1;	     /**< [  2:  2](R/W1C/H) Memory fault on NIX_RQ_CTX_S read on packet receive or NIX_LF_RQ_OP_* access. */
		u64 cq_ctx_fault:1;	     /**< [  3:  3](R/W1C/H) Memory fault on NIX_CQ_CTX_S read or write. */
		u64 reserved_4:1;
		u64 rsse_fault:1;		     /**< [  5:  5](R/W1C/H) Memory fault on NIX_RSSE_S read. */
		u64 ipsec_dyno_fault:1;	     /**< [  6:  6](R/W1C/H) Memory fault on IPSEC dynamic ordering counter read. See
                                                                 NIX_AF_LF()_RX_IPSEC_CFG0. */
		u64 sq_disabled:1;		     /**< [  7:  7](R/W1C/H) LMT store or NIX_LF_SQ_OP_* access to a disabled SQ. NIX_SQ_CTX_S[ENA]
                                                                 was clear for the selected SQ. */
		u64 sq_oor:1;		     /**< [  8:  8](R/W1C/H) LMT store or NIX_LF_SQ_OP_* access to out-of-range SQ. The SQ index was
                                                                 greater than NIX_AF_LF()_SQS_CFG[MAX_QUEUESM1]. */
		u64 send_jump_fault:1;	     /**< [  9:  9](R/W1C/H) Memory fault on send descriptor read at or beyond NIX_SEND_JUMP_S[ADDR]. */
		u64 send_sg_fault:1;	     /**< [ 10: 10](R/W1C/H) Memory fault on packet data read for NIX_SEND_SG_S. */
		u64 rq_disabled:1;		     /**< [ 11: 11](R/W1C/H) Packet receive or NIX_LF_RQ_OP_* access to a disabled RQ;
                                                                 NIX_RQ_CTX_S[ENA] was clear for selected RQ. See [RQ_OOR] for RQ
                                                                 selection. */
		u64 rq_oor:1;		     /**< [ 12: 12](R/W1C/H) Packet receive or NIX_LF_RQ_OP_* access to out-of-range RQ. The
                                                                 packet's RQ index was greater than NIX_AF_LF()_RQS_CFG[MAX_QUEUESM1]. The RQ
                                                                 index for a received packet is obtained as follows:
                                                                 * When NIX_RX_ACTION_S[OP] = NIX_RX_ACTIONOP_E::UCAST or
                                                                 NIX_RX_ACTIONOP_E::UCAST_IPSEC, RQ = NIX_RX_ACTION_S[INDEX].
                                                                 * For RSS, NIX_RSSE_S[RQ].
                                                                 * For multicast or mirror packet with NIX_RX_MCE_S[OP] = NIX_RX_MCOP_E::RQ,
                                                                 RQ = NIX_RX_MCE_S[INDEX]. */
		u64 rx_wqe_fault:1;	     /**< [ 13: 13](R/W1C/H) Memory fault on receive packet WQE write to LLC/DRAM. */
		u64 rss_err:1;		     /**< [ 14: 14](R/W1C/H) RSSE Table entry was disabled or the rsse_offset was larger than the programmed size. */
		u64 reserved_15_19:5;
		u64 dyno_err:1;		     /**< [ 20: 20](R/W1C/H) NIX_LF_OP_IPSEC_DYNO_CNT[COUNT] underflow or overflow. */
		u64 reserved_21_23:3;
		u64 cq_disabled:1;		     /**< [ 24: 24](R/W1C/H) CQ disabled. NIX_CQ_CTX_S[ENA] was clear for the CQ of a send/receive CQE
                                                                 write or NIX_LF_CQ_OP_* access. */
		u64 cq_oor:1;		     /**< [ 25: 25](R/W1C/H) CQ out of range. The CQ index of a send/receive CQE write or
                                                                 NIX_LF_CQ_OP_* access was greater than
                                                                 NIX_AF_LF()_CQS_CFG[MAX_QUEUESM1]. */
		u64 reserved_26_27:2;
		u64 qint_fault:1;		     /**< [ 28: 28](R/W1C/H) Memory fault on NIX_QINT_HW_S read or write. */
		u64 cint_fault:1;		     /**< [ 29: 29](R/W1C/H) Memory fault on NIX_CINT_HW_S read or write. */
		u64 reserved_30_63:34;
	} s;
	/* struct cavm_nixx_lf_err_int_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int_w1s
 *
 * NIX LF Error Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_nixx_lf_err_int_w1s {
	u64 u;
	struct cavm_nixx_lf_err_int_w1s_s {
		u64 sqb_fault:1;		     /**< [  0:  0](R/W1S/H) Reads or sets NIX_LF_ERR_INT[SQB_FAULT]. */
		u64 sq_ctx_fault:1;	     /**< [  1:  1](R/W1S/H) Reads or sets NIX_LF_ERR_INT[SQ_CTX_FAULT]. */
		u64 rq_ctx_fault:1;	     /**< [  2:  2](R/W1S/H) Reads or sets NIX_LF_ERR_INT[RQ_CTX_FAULT]. */
		u64 cq_ctx_fault:1;	     /**< [  3:  3](R/W1S/H) Reads or sets NIX_LF_ERR_INT[CQ_CTX_FAULT]. */
		u64 reserved_4:1;
		u64 rsse_fault:1;		     /**< [  5:  5](R/W1S/H) Reads or sets NIX_LF_ERR_INT[RSSE_FAULT]. */
		u64 ipsec_dyno_fault:1;	     /**< [  6:  6](R/W1S/H) Reads or sets NIX_LF_ERR_INT[IPSEC_DYNO_FAULT]. */
		u64 sq_disabled:1;		     /**< [  7:  7](R/W1S/H) Reads or sets NIX_LF_ERR_INT[SQ_DISABLED]. */
		u64 sq_oor:1;		     /**< [  8:  8](R/W1S/H) Reads or sets NIX_LF_ERR_INT[SQ_OOR]. */
		u64 send_jump_fault:1;	     /**< [  9:  9](R/W1S/H) Reads or sets NIX_LF_ERR_INT[SEND_JUMP_FAULT]. */
		u64 send_sg_fault:1;	     /**< [ 10: 10](R/W1S/H) Reads or sets NIX_LF_ERR_INT[SEND_SG_FAULT]. */
		u64 rq_disabled:1;		     /**< [ 11: 11](R/W1S/H) Reads or sets NIX_LF_ERR_INT[RQ_DISABLED]. */
		u64 rq_oor:1;		     /**< [ 12: 12](R/W1S/H) Reads or sets NIX_LF_ERR_INT[RQ_OOR]. */
		u64 rx_wqe_fault:1;	     /**< [ 13: 13](R/W1S/H) Reads or sets NIX_LF_ERR_INT[RX_WQE_FAULT]. */
		u64 rss_err:1;		     /**< [ 14: 14](R/W1S/H) Reads or sets NIX_LF_ERR_INT[RSS_ERR]. */
		u64 reserved_15_19:5;
		u64 dyno_err:1;		     /**< [ 20: 20](R/W1S/H) Reads or sets NIX_LF_ERR_INT[DYNO_ERR]. */
		u64 reserved_21_23:3;
		u64 cq_disabled:1;		     /**< [ 24: 24](R/W1S/H) Reads or sets NIX_LF_ERR_INT[CQ_DISABLED]. */
		u64 cq_oor:1;		     /**< [ 25: 25](R/W1S/H) Reads or sets NIX_LF_ERR_INT[CQ_OOR]. */
		u64 reserved_26_27:2;
		u64 qint_fault:1;		     /**< [ 28: 28](R/W1S/H) Reads or sets NIX_LF_ERR_INT[QINT_FAULT]. */
		u64 cint_fault:1;		     /**< [ 29: 29](R/W1S/H) Reads or sets NIX_LF_ERR_INT[CINT_FAULT]. */
		u64 reserved_30_63:34;
	} s;
	/* struct cavm_nixx_lf_err_int_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int_ena_w1c
 *
 * NIX LF Error Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_lf_err_int_ena_w1c {
	u64 u;
	struct cavm_nixx_lf_err_int_ena_w1c_s {
		u64 sqb_fault:1;		     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[SQB_FAULT]. */
		u64 sq_ctx_fault:1;	     /**< [  1:  1](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[SQ_CTX_FAULT]. */
		u64 rq_ctx_fault:1;	     /**< [  2:  2](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[RQ_CTX_FAULT]. */
		u64 cq_ctx_fault:1;	     /**< [  3:  3](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[CQ_CTX_FAULT]. */
		u64 reserved_4:1;
		u64 rsse_fault:1;		     /**< [  5:  5](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[RSSE_FAULT]. */
		u64 ipsec_dyno_fault:1;	     /**< [  6:  6](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[IPSEC_DYNO_FAULT]. */
		u64 sq_disabled:1;		     /**< [  7:  7](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[SQ_DISABLED]. */
		u64 sq_oor:1;		     /**< [  8:  8](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[SQ_OOR]. */
		u64 send_jump_fault:1;	     /**< [  9:  9](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[SEND_JUMP_FAULT]. */
		u64 send_sg_fault:1;	     /**< [ 10: 10](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[SEND_SG_FAULT]. */
		u64 rq_disabled:1;		     /**< [ 11: 11](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[RQ_DISABLED]. */
		u64 rq_oor:1;		     /**< [ 12: 12](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[RQ_OOR]. */
		u64 rx_wqe_fault:1;	     /**< [ 13: 13](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[RX_WQE_FAULT]. */
		u64 rss_err:1;		     /**< [ 14: 14](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[RSS_ERR]. */
		u64 reserved_15_19:5;
		u64 dyno_err:1;		     /**< [ 20: 20](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[DYNO_ERR]. */
		u64 reserved_21_23:3;
		u64 cq_disabled:1;		     /**< [ 24: 24](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[CQ_DISABLED]. */
		u64 cq_oor:1;		     /**< [ 25: 25](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[CQ_OOR]. */
		u64 reserved_26_27:2;
		u64 qint_fault:1;		     /**< [ 28: 28](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[QINT_FAULT]. */
		u64 cint_fault:1;		     /**< [ 29: 29](R/W1C/H) Reads or clears enable for NIX_LF_ERR_INT[CINT_FAULT]. */
		u64 reserved_30_63:34;
	} s;
	/* struct cavm_nixx_lf_err_int_ena_w1c_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int_ena_w1s
 *
 * NIX LF Error Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_lf_err_int_ena_w1s {
	u64 u;
	struct cavm_nixx_lf_err_int_ena_w1s_s {
		u64 sqb_fault:1;		     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[SQB_FAULT]. */
		u64 sq_ctx_fault:1;	     /**< [  1:  1](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[SQ_CTX_FAULT]. */
		u64 rq_ctx_fault:1;	     /**< [  2:  2](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[RQ_CTX_FAULT]. */
		u64 cq_ctx_fault:1;	     /**< [  3:  3](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[CQ_CTX_FAULT]. */
		u64 reserved_4:1;
		u64 rsse_fault:1;		     /**< [  5:  5](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[RSSE_FAULT]. */
		u64 ipsec_dyno_fault:1;	     /**< [  6:  6](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[IPSEC_DYNO_FAULT]. */
		u64 sq_disabled:1;		     /**< [  7:  7](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[SQ_DISABLED]. */
		u64 sq_oor:1;		     /**< [  8:  8](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[SQ_OOR]. */
		u64 send_jump_fault:1;	     /**< [  9:  9](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[SEND_JUMP_FAULT]. */
		u64 send_sg_fault:1;	     /**< [ 10: 10](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[SEND_SG_FAULT]. */
		u64 rq_disabled:1;		     /**< [ 11: 11](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[RQ_DISABLED]. */
		u64 rq_oor:1;		     /**< [ 12: 12](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[RQ_OOR]. */
		u64 rx_wqe_fault:1;	     /**< [ 13: 13](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[RX_WQE_FAULT]. */
		u64 rss_err:1;		     /**< [ 14: 14](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[RSS_ERR]. */
		u64 reserved_15_19:5;
		u64 dyno_err:1;		     /**< [ 20: 20](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[DYNO_ERR]. */
		u64 reserved_21_23:3;
		u64 cq_disabled:1;		     /**< [ 24: 24](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[CQ_DISABLED]. */
		u64 cq_oor:1;		     /**< [ 25: 25](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[CQ_OOR]. */
		u64 reserved_26_27:2;
		u64 qint_fault:1;		     /**< [ 28: 28](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[QINT_FAULT]. */
		u64 cint_fault:1;		     /**< [ 29: 29](R/W1S/H) Reads or sets enable for NIX_LF_ERR_INT[CINT_FAULT]. */
		u64 reserved_30_63:34;
	} s;
	/* struct cavm_nixx_lf_err_int_ena_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras
 *
 * NIX LF RAS Interrupt Register
 */
union cavm_nixx_lf_ras {
	u64 u;
	struct cavm_nixx_lf_ras_s {
		u64 sqb_poison:1;		     /**< [  0:  0](R/W1C/H) Poisoned data returned on SQB read. */
		u64 sq_ctx_poison:1;	     /**< [  1:  1](R/W1C/H) Poisoned data returned on NIX_SQ_CTX_HW_S read. */
		u64 rq_ctx_poison:1;	     /**< [  2:  2](R/W1C/H) Poisoned data returned on NIX_RQ_CTX_S read. */
		u64 cq_ctx_poison:1;	     /**< [  3:  3](R/W1C/H) Poisoned data returned on NIX_CQ_CTX_S read. */
		u64 reserved_4:1;
		u64 rsse_poison:1;		     /**< [  5:  5](R/W1C/H) Poisoned data returned on NIX_RSSE_S read. */
		u64 ipsec_dyno_poison:1;	     /**< [  6:  6](R/W1C/H) Poisoned data returned on IPSEC dynamic ordering counter read. See
                                                                 NIX_AF_LF()_RX_IPSEC_CFG0. */
		u64 send_jump_poison:1;	     /**< [  7:  7](R/W1C/H) Poisoned data returned on send descriptor read at or beyond
                                                                 NIX_SEND_JUMP_S[ADDR]. */
		u64 send_sg_poison:1;	     /**< [  8:  8](R/W1C/H) Poisoned data returned on packet data read for NIX_SEND_SG_S. */
		u64 qint_poison:1;		     /**< [  9:  9](R/W1C/H) Poisoned data returned on NIX_QINT_HW_S read. */
		u64 cint_poison:1;		     /**< [ 10: 10](R/W1C/H) Poisoned data returned on NIX_CINT_HW_S read. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_lf_ras_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_w1s
 *
 * NIX LF RAS Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_nixx_lf_ras_w1s {
	u64 u;
	struct cavm_nixx_lf_ras_w1s_s {
		u64 sqb_poison:1;		     /**< [  0:  0](R/W1S/H) Reads or sets NIX_LF_RAS[SQB_POISON]. */
		u64 sq_ctx_poison:1;	     /**< [  1:  1](R/W1S/H) Reads or sets NIX_LF_RAS[SQ_CTX_POISON]. */
		u64 rq_ctx_poison:1;	     /**< [  2:  2](R/W1S/H) Reads or sets NIX_LF_RAS[RQ_CTX_POISON]. */
		u64 cq_ctx_poison:1;	     /**< [  3:  3](R/W1S/H) Reads or sets NIX_LF_RAS[CQ_CTX_POISON]. */
		u64 reserved_4:1;
		u64 rsse_poison:1;		     /**< [  5:  5](R/W1S/H) Reads or sets NIX_LF_RAS[RSSE_POISON]. */
		u64 ipsec_dyno_poison:1;	     /**< [  6:  6](R/W1S/H) Reads or sets NIX_LF_RAS[IPSEC_DYNO_POISON]. */
		u64 send_jump_poison:1;	     /**< [  7:  7](R/W1S/H) Reads or sets NIX_LF_RAS[SEND_JUMP_POISON]. */
		u64 send_sg_poison:1;	     /**< [  8:  8](R/W1S/H) Reads or sets NIX_LF_RAS[SEND_SG_POISON]. */
		u64 qint_poison:1;		     /**< [  9:  9](R/W1S/H) Reads or sets NIX_LF_RAS[QINT_POISON]. */
		u64 cint_poison:1;		     /**< [ 10: 10](R/W1S/H) Reads or sets NIX_LF_RAS[CINT_POISON]. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_lf_ras_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_ena_w1c
 *
 * NIX LF RAS Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_nixx_lf_ras_ena_w1c {
	u64 u;
	struct cavm_nixx_lf_ras_ena_w1c_s {
		u64 sqb_poison:1;		     /**< [  0:  0](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SQB_POISON]. */
		u64 sq_ctx_poison:1;	     /**< [  1:  1](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SQ_CTX_POISON]. */
		u64 rq_ctx_poison:1;	     /**< [  2:  2](R/W1C/H) Reads or clears enable for NIX_LF_RAS[RQ_CTX_POISON]. */
		u64 cq_ctx_poison:1;	     /**< [  3:  3](R/W1C/H) Reads or clears enable for NIX_LF_RAS[CQ_CTX_POISON]. */
		u64 reserved_4:1;
		u64 rsse_poison:1;		     /**< [  5:  5](R/W1C/H) Reads or clears enable for NIX_LF_RAS[RSSE_POISON]. */
		u64 ipsec_dyno_poison:1;	     /**< [  6:  6](R/W1C/H) Reads or clears enable for NIX_LF_RAS[IPSEC_DYNO_POISON]. */
		u64 send_jump_poison:1;	     /**< [  7:  7](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SEND_JUMP_POISON]. */
		u64 send_sg_poison:1;	     /**< [  8:  8](R/W1C/H) Reads or clears enable for NIX_LF_RAS[SEND_SG_POISON]. */
		u64 qint_poison:1;		     /**< [  9:  9](R/W1C/H) Reads or clears enable for NIX_LF_RAS[QINT_POISON]. */
		u64 cint_poison:1;		     /**< [ 10: 10](R/W1C/H) Reads or clears enable for NIX_LF_RAS[CINT_POISON]. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_lf_ras_ena_w1c_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_ena_w1s
 *
 * NIX LF RAS Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_nixx_lf_ras_ena_w1s {
	u64 u;
	struct cavm_nixx_lf_ras_ena_w1s_s {
		u64 sqb_poison:1;		     /**< [  0:  0](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SQB_POISON]. */
		u64 sq_ctx_poison:1;	     /**< [  1:  1](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SQ_CTX_POISON]. */
		u64 rq_ctx_poison:1;	     /**< [  2:  2](R/W1S/H) Reads or sets enable for NIX_LF_RAS[RQ_CTX_POISON]. */
		u64 cq_ctx_poison:1;	     /**< [  3:  3](R/W1S/H) Reads or sets enable for NIX_LF_RAS[CQ_CTX_POISON]. */
		u64 reserved_4:1;
		u64 rsse_poison:1;		     /**< [  5:  5](R/W1S/H) Reads or sets enable for NIX_LF_RAS[RSSE_POISON]. */
		u64 ipsec_dyno_poison:1;	     /**< [  6:  6](R/W1S/H) Reads or sets enable for NIX_LF_RAS[IPSEC_DYNO_POISON]. */
		u64 send_jump_poison:1;	     /**< [  7:  7](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SEND_JUMP_POISON]. */
		u64 send_sg_poison:1;	     /**< [  8:  8](R/W1S/H) Reads or sets enable for NIX_LF_RAS[SEND_SG_POISON]. */
		u64 qint_poison:1;		     /**< [  9:  9](R/W1S/H) Reads or sets enable for NIX_LF_RAS[QINT_POISON]. */
		u64 cint_poison:1;		     /**< [ 10: 10](R/W1S/H) Reads or sets enable for NIX_LF_RAS[CINT_POISON]. */
		u64 reserved_11_63:53;
	} s;
	/* struct cavm_nixx_lf_ras_ena_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_err_dbg
 *
 * NIX LF SQ Operation Error Debug Register
 * This register captures debug info for an error detected on LMT store to
 * NIX_LF_OP_SEND() or when a NIX_LF_SQ_OP_* register is accessed.
 * Hardware sets [VALID] when the debug info is captured, and subsequent errors
 * are not captured until software clears [VALID] by writing a one to it.
 */
union cavm_nixx_lf_sq_op_err_dbg {
	u64 u;
	struct cavm_nixx_lf_sq_op_err_dbg_s {
		u64 errcode:8;		     /**< [  7:  0](RO/H) Error code enumerated by NIX_SQOPERR_E. */
		u64 sq:20;			     /**< [ 27:  8](RO/H) SQ within LF. */
		u64 sqe_id:16;		     /**< [ 43: 28](RO/H) SQE identifier from NIX_SEND_HDR_S[SQE_ID]. */
		u64 valid:1;		     /**< [ 44: 44](R/W1C/H) Debug info valid. When set, remaining fields in this register are valid. */
		u64 reserved_45_63:19;
	} s;
	/* struct cavm_nixx_lf_sq_op_err_dbg_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_mnq_err_dbg
 *
 * NIX LF Meta-descriptor Enqueue Error Debug Register
 * This register captures debug info for an error detected during send
 * meta-descriptor enqueue from an SQ to an SMQ.
 * Hardware sets [VALID] when the debug info is captured, and subsequent errors
 * are not captured until software clears [VALID] by writing a one to it.
 */
union cavm_nixx_lf_mnq_err_dbg {
	u64 u;
	struct cavm_nixx_lf_mnq_err_dbg_s {
		u64 errcode:8;		     /**< [  7:  0](RO/H) Error code enumerated by NIX_MNQERR_E. */
		u64 sq:20;			     /**< [ 27:  8](RO/H) SQ within LF. */
		u64 sqe_id:16;		     /**< [ 43: 28](RO/H) SQE identifier from NIX_SEND_HDR_S[SQE_ID]. */
		u64 valid:1;		     /**< [ 44: 44](R/W1C/H) Debug info valid. When set, remaining fields in this register are valid. */
		u64 reserved_45_63:19;
	} s;
	/* struct cavm_nixx_lf_mnq_err_dbg_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_send_err_dbg
 *
 * NIX LF Send Error Debug Register
 * This register captures debug info an error detected on packet send after a
 * meta-descriptor is granted by PSE.
 * Hardware sets [VALID] when the debug info is captured, and subsequent errors
 * are not captured until software clears [VALID] by writing a one to it.
 */
union cavm_nixx_lf_send_err_dbg {
	u64 u;
	struct cavm_nixx_lf_send_err_dbg_s {
		u64 errcode:8;		     /**< [  7:  0](RO/H) Error code enumerated by NIX_SEND_STATUS_E. */
		u64 sq:20;			     /**< [ 27:  8](RO/H) SQ within LF. */
		u64 sqe_id:16;		     /**< [ 43: 28](RO/H) SQE identifier from NIX_SEND_HDR_S[SQE_ID]. */
		u64 valid:1;		     /**< [ 44: 44](R/W1C/H) Debug info valid. When set, remaining fields in this register are valid. */
		u64 reserved_45_63:19;
	} s;
	/* struct cavm_nixx_lf_send_err_dbg_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_tx_stat#
 *
 * NIX LF Transmit Statistics Registers
 * The last dimension indicates which statistic, and is enumerated by NIX_STAT_LF_TX_E.
 */
union cavm_nixx_lf_tx_statx {
	u64 u;
	struct cavm_nixx_lf_tx_statx_s {
		u64 stat:48;		     /**< [ 47:  0](RO/H) Statistic value. See also NIX_AF_LF()_TX_STAT() for a writable alias of this field. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_lf_tx_statx_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rx_stat#
 *
 * NIX LF Receive Statistics Registers
 * The last dimension indicates which statistic, and is enumerated by NIX_STAT_LF_RX_E.
 */
union cavm_nixx_lf_rx_statx {
	u64 u;
	struct cavm_nixx_lf_rx_statx_s {
		u64 stat:48;		     /**< [ 47:  0](RO/H) Statistic value. See also NIX_AF_LF()_RX_STAT() for a writable alias of this field. */
		u64 reserved_48_63:16;
	} s;
	/* struct cavm_nixx_lf_rx_statx_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_op_send#
 *
 * NIX LF Send Operation Registers
 * An LMTST (or large store from CPT) to this address enqueues one or more SQEs to
 * a send queue. NIX_SEND_HDR_S[SQ] in the first SQE selects the send queue.The
 * maximum size of each SQE is specified by NIX_SQ_CTX_S[MAX_SQE_SIZE].
 *
 * A read to this address is RAZ.
 *
 * An RSL access to this address will fault.
 *
 * The endianness of the instruction write data is controlled by NIX_AF_LF()_CFG[BE].
 *
 * When a NIX_SEND_JUMP_S is not present in the SQE, the SQE consists of the
 * entire send descriptor.
 *
 * When a NIX_SEND_JUMP_S is present in the SQE, the SQE must contain exactly the
 * portion of the send descriptor up to and including the NIX_SEND_JUMP_S, and the
 * remainder of the send descriptor must be at LF IOVA NIX_SEND_JUMP_S[ADDR] in
 * LLC/DRAM.
 *
 * Software must ensure that all LLC/DRAM locations that will be referenced by NIX while
 * processing this descriptor, including all packet data and post-jump subdescriptors
 * contain the latest updates before issuing the LMTST. A DMB instruction may be required prior
 * to the LMTST to ensure this. A DMB following the LMTST may be useful if SQ descriptor ordering
 * matters and more than one CPU core is simultaneously enqueueing to the same SQ. For more
 * information on ordering, refer to the HRM "Core Memory Reference Ordering" section in the CPU
 * cores chapter.
 */
union cavm_nixx_lf_op_sendx {
	u64 u;
	struct cavm_nixx_lf_op_sendx_s {
		u64 data:64;		     /**< [ 63:  0](WO) Data that forms the send descriptor; see register description. */
	} s;
	/* struct cavm_nixx_lf_op_sendx_s cn; */
};

union cavm_nixx_lf_rq_op_int {
	u64 u;
	struct cavm_nixx_lf_rq_op_int_s {
		u64 rq_int:8;		     /**< [  7:  0](R/W/H) Returns NIX_RQ_CTX_S[RQ_INT] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NIX_RQ_CTX_S[RQ_INT] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
		u64 rq_int_ena:8;		     /**< [ 15:  8](R/W/H) Returns NIX_RQ_CTX_S[RQ_INT_ENA] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NIX_RQ_CTX_S[RQ_INT_ENA] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
		u64 reserved_16_41:26;
		u64 op_err:1;		     /**< [ 42: 42](RO/H) Operation error. Remaining read data fields are not valid. One of the
                                                                 following (may not be exhaustive):
                                                                 * Memory fault on NIX_RQ_CTX_S read; also sets NIX_LF_ERR_INT[RQ_CTX_FAULT].
                                                                 * Poisoned data returned on NIX_RQ_CTX_S read; also sets
                                                                 NIX_LF_RAS[RQ_CTX_POISON].
                                                                 * Access to out-of-range RQ ([RQ] \> NIX_AF_LF()_RQS_CFG[MAX_QUEUESM1]);
                                                                 also sets NIX_LF_ERR_INT[RQ_OOR].
                                                                 * Disabled RQ (NIX_RQ_CTX_S[ENA] = 0); also sets
                                                                 NIX_LF_ERR_INT[RQ_DISABLED]. */
		u64 setop:1;		     /**< [ 43: 43](WO) Set Operation. Valid on a write. Indicates write-one-to-set when set,
                                                                 write-one-to-clear when clear. */
		u64 rq:20;			     /**< [ 63: 44](WO) RQ within LF. This field is present on a write, or in the write data
                                                                 of an atomic load-and-add. */
	} s;
	/* struct cavm_nixx_lf_rq_op_int_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_octs
 *
 * NIX LF Receive Queue Octets Operation Register
 * A 64-bit atomic load-and-add to this register reads NIX_RQ_CTX_S[OCTS]. The atomic
 * write data has format NIX_OP_Q_WDATA_S and selects the RQ within LF.
 *
 * All other accesses to this register (e.g. reads and writes) are RAZ/WI.
 *
 * An RSL access to this register will fault.
 */
union cavm_nixx_lf_rq_op_octs {
	u64 u;
	struct cavm_nixx_lf_rq_op_octs_s {
		u64 cnt:48;		     /**< [ 47:  0](RO) Count. */
		u64 reserved_48_62:15;
		u64 op_err:1;		     /**< [ 63: 63](RO/H) Operation error. See NIX_LF_RQ_OP_INT[OP_ERR]. */
	} s;
	/* struct cavm_nixx_lf_rq_op_octs_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_pkts
 *
 * NIX LF Receive Queue Packets Operation Register
 * A 64-bit atomic load-and-add to this register reads NIX_RQ_CTX_S[PKTS]. The atomic
 * write data has format NIX_OP_Q_WDATA_S and selects the RQ within LF.
 *
 * All other accesses to this register (e.g. reads and writes) are RAZ/WI.
 *
 * An RSL access to this register will fault.
 */
union cavm_nixx_lf_rq_op_pkts {
	u64 u;
	struct cavm_nixx_lf_rq_op_pkts_s {
		u64 cnt:48;		     /**< [ 47:  0](RO) Count. */
		u64 reserved_48_62:15;
		u64 op_err:1;		     /**< [ 63: 63](RO/H) Operation error. See NIX_LF_RQ_OP_INT[OP_ERR]. */
	} s;
	/* struct cavm_nixx_lf_rq_op_pkts_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_re_pkts
 *
 * NIX LF Receive Queue Errored Packets Operation Register
 * A 64-bit atomic load-and-add to this register reads NIX_RQ_CTX_S[RE_PKTS].
 * The atomic write data has format NIX_OP_Q_WDATA_S and selects the RQ within LF.
 *
 * All other accesses to this register (e.g. reads and writes) are RAZ/WI.
 *
 * An RSL access to this register will fault.
 */
union cavm_nixx_lf_rq_op_re_pkts {
	u64 u;
	struct cavm_nixx_lf_rq_op_re_pkts_s {
		u64 cnt:48;		     /**< [ 47:  0](RO) Count. */
		u64 reserved_48_62:15;
		u64 op_err:1;		     /**< [ 63: 63](RO/H) Operation error. See NIX_LF_RQ_OP_INT[OP_ERR]. */
	} s;
	/* struct cavm_nixx_lf_rq_op_re_pkts_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cq_op_int
 *
 * NIX LF Completion Queue Interrupt Operation Register
 * A 64-bit atomic load-and-add to this register reads CQ interrupts and
 * interrupt enables.
 * A write optionally sets or clears interrupts and interrupt enables.
 * A read is RAZ.
 *
 * An RSL access to this register will fault.
 */
union cavm_nixx_lf_cq_op_int {
	u64 u;
	struct cavm_nixx_lf_cq_op_int_s {
		u64 cq_err_int:8;		     /**< [  7:  0](R/W/H) Returns NIX_CQ_CTX_S[CQ_ERR_INT] on an atomic load-and-add. On a
                                                                 write, write-one-to-set NIX_CQ_CTX_S[CQ_ERR_INT] if [SETOP] is set,
                                                                 write-one-to-clear otherwise. */
		u64 cq_err_int_ena:8;	     /**< [ 15:  8](R/W/H) Returns NIX_CQ_CTX_S[CQ_ERR_INT_ENA] on an atomic load-and-add. On a
                                                                 write, write-one-to-set NIX_CQ_CTX_S[CQ_ERR_INT_ENA] if [SETOP] is
                                                                 set, write-one-to-clear otherwise. */
		u64 reserved_16_41:26;
		u64 op_err:1;		     /**< [ 42: 42](RO/H) Operation error. Remaining read data fields are not valid. One of the
                                                                 following (may not be exhaustive):
                                                                 * Memory fault on NIX_CQ_CTX_S read; also sets NIX_LF_ERR_INT[CQ_CTX_FAULT].
                                                                 * Poisoned data returned on NIX_CQ_CTX_S read; also sets
                                                                 NIX_LF_RAS[CQ_CTX_POISON].
                                                                 * Access to out-of-range CQ ([CQ] \> NIX_AF_LF()_CQS_CFG[MAX_QUEUESM1]);
                                                                 also sets NIX_LF_ERR_INT[CQ_OOR].
                                                                 * Disabled CQ (NIX_CQ_CTX_S[ENA] = 0); also sets
                                                                 NIX_LF_ERR_INT[CQ_DISABLED]. */
		u64 setop:1;		     /**< [ 43: 43](WO) Set operation. Valid on a write. Indicates write-one-to-set when set,
                                                                 write-one-to-clear when clear. */
		u64 cq:20;			     /**< [ 63: 44](WO) CQ within LF. This field is present on a write, or in the write data
                                                                 of an atomic load-and-add. */
	} s;
	/* struct cavm_nixx_lf_cq_op_int_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cq_op_door
 *
 * NIX LF CQ Doorbell Operation Register
 * A write to this register dequeues CQEs from a CQ ring within the LF.
 * A read is RAZ.
 *
 * An RSL access to this register will fault.
 */
union cavm_nixx_lf_cq_op_door {
	u64 u;
	struct cavm_nixx_lf_cq_op_door_s {
		u64 count:16;		     /**< [ 15:  0](WO) Number of dequeued CQEs. Hardware advances NIX_CQ_CTX_S[HEAD] by this value
                                                                 if the CQ is enabled. The size of each dequeued CQE is selected by
                                                                 NIX_AF_LF()_CFG[XQE_SIZE].

                                                                 A doorbell write that would underflow the ring is suppressed and sets
                                                                 NIX_CQ_CTX_S[CQ_ERR_INT]\<NIX_CQERRINT_E::DOOR_ERR\>. */
		u64 reserved_16_31:16;
		u64 cq:20;			     /**< [ 51: 32](WO) CQ within the VF. */
		u64 reserved_52_63:12;
	} s;
	/* struct cavm_nixx_lf_cq_op_door_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cq_op_status
 *
 * NIX LF Completion Queue Status Operation Register
 * A 64-bit atomic load-and-add to this register reads NIX_CQ_CTX_S[HEAD,TAIL].
 * The atomic write data has format NIX_OP_Q_WDATA_S and selects the CQ within LF.
 *
 * All other accesses to this register (e.g. reads and writes) are RAZ/WI.
 *
 * An RSL access to this register will fault.
 */
union cavm_nixx_lf_cq_op_status {
	u64 u;
	struct cavm_nixx_lf_cq_op_status_s {
		u64 tail:20;		     /**< [ 19:  0](RO/H) See NIX_CQ_CTX_S[TAIL]. */
		u64 head:20;		     /**< [ 39: 20](RO/H) See NIX_CQ_CTX_S[HEAD]. The number of entries in the CQ is:
                                                                 _ ([TAIL] - [HEAD]) % (1 \<\< (2 * (NIX_CQ_CTX_S[QSIZE] + 2)) */
		u64 state:6;		     /**< [ 45: 40](RO/H) See NIX_CQ_CTX_S[STATE]. */
		u64 cq_err:1;		     /**< [ 46: 46](RO/H) See NIX_CQ_CTX_S[CQ_ERR]. */
		u64 reserved_47_62:16;
		u64 op_err:1;		     /**< [ 63: 63](RO/H) Operation error. See NIX_LF_CQ_OP_INT[OP_ERR]. */
	} s;
	/* struct cavm_nixx_lf_cq_op_status_s cn; */
};

#endif /* __NIX_REG_H__ */
