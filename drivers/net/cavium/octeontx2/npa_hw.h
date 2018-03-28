/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This file defines the NPA registers for the Cavium OcteonTX2.
 */

#ifndef __NPA_HW_H__
#define __NPA_HW_H__

/**
 * Enumeration npa_af_int_vec_e
 *
 * NPA Admin Function Interrupt Vector Enumeration
 * Enumerates the NPA AF MSI-X interrupt vectors.
 */
#define CAVM_NPA_AF_INT_VEC_E_AF_ERR (3)
#define CAVM_NPA_AF_INT_VEC_E_AQ_DONE (2)
#define CAVM_NPA_AF_INT_VEC_E_GEN (1)
#define CAVM_NPA_AF_INT_VEC_E_POISON (4)
#define CAVM_NPA_AF_INT_VEC_E_RVU (0)

/**
 * Enumeration npa_aq_comp_e
 *
 * NPA Admin Queue Completion Enumeration
 * Enumerates the values of NPA_AQ_RES_S[COMPCODE].
 */
#define CAVM_NPA_AQ_COMP_E_CTX_FAULT (4)
#define CAVM_NPA_AQ_COMP_E_CTX_POISON (3)
#define CAVM_NPA_AQ_COMP_E_GOOD (1)
#define CAVM_NPA_AQ_COMP_E_LOCKERR (5)
#define CAVM_NPA_AQ_COMP_E_NOTDONE (0)
#define CAVM_NPA_AQ_COMP_E_SWERR (2)

/**
 * Enumeration npa_aq_ctype_e
 *
 * NPA Admin Queue Context Type Enumeration
 * Enumerates NPA_AQ_INST_S[CTYPE] values.
 */
#define CAVM_NPA_AQ_CTYPE_E_AURA (0)
#define CAVM_NPA_AQ_CTYPE_E_POOL (1)

/**
 * Enumeration npa_aq_instop_e
 *
 * NPA Admin Queue Opcode Enumeration
 * Enumerates NPA_AQ_INST_S[OP] values.
 */
#define CAVM_NPA_AQ_INSTOP_E_INIT (1)
#define CAVM_NPA_AQ_INSTOP_E_LOCK (4)
#define CAVM_NPA_AQ_INSTOP_E_NOP (0)
#define CAVM_NPA_AQ_INSTOP_E_READ (3)
#define CAVM_NPA_AQ_INSTOP_E_UNLOCK (5)
#define CAVM_NPA_AQ_INSTOP_E_WRITE (2)

/**
 * Enumeration npa_aura_err_int_e
 *
 * NPA Aura Error Interrupt Enumeration
 * Enumerates the bit index of NPA_AURA_S[ERR_INT], and NPA_AURA_S[ERR_INT_ENA].
 */
#define CAVM_NPA_AURA_ERR_INT_E_AURA_ADD_OVER (1)
#define CAVM_NPA_AURA_ERR_INT_E_AURA_ADD_UNDER (2)
#define CAVM_NPA_AURA_ERR_INT_E_AURA_FREE_UNDER (0)
#define CAVM_NPA_AURA_ERR_INT_E_POOL_DIS (3)
#define CAVM_NPA_AURA_ERR_INT_E_RX(a) (0 + (a))

/**
 * Enumeration npa_bpintf_e
 *
 * NPA Backpressure Interface Enumeration
 * Enumerates index of NPA_AURA_S[BP_ENA].
 */
#define CAVM_NPA_BPINTF_E_NIXX_RX(a) (0 + (a))

/**
 * Enumeration npa_inpq_e
 *
 * NPA Input Queue Enumeration
 * Enumerates ALLOC/FREE input queues from coprocessors.
 */
#define CAVM_NPA_INPQ_E_AURA_OP (0xe)
#define CAVM_NPA_INPQ_E_DPI (6)
#define CAVM_NPA_INPQ_E_INTERNAL_RSV (0xf)
#define CAVM_NPA_INPQ_E_NIXX_RX(a) (0 + 2 * (a))
#define CAVM_NPA_INPQ_E_NIXX_TX(a) (1 + 2 * (a))
#define CAVM_NPA_INPQ_E_RX(a) (0 + (a))
#define CAVM_NPA_INPQ_E_SSO (4)
#define CAVM_NPA_INPQ_E_TIM (5)

/**
 * Enumeration npa_lf_int_vec_e
 *
 * NPA Local Function Interrupt Vector Enumeration
 * Enumerates the NPA MSI-X interrupt vectors per LF.
 */
#define CAVM_NPA_LF_INT_VEC_E_ERR_INT (0x40)
#define CAVM_NPA_LF_INT_VEC_E_POISON (0x41)
#define CAVM_NPA_LF_INT_VEC_E_QINTX(a) (0 + (a))

/**
 * Enumeration npa_ndc0_port_e
 *
 * NPA NDC0 Port Enumeration
 * Enumerates NPA NDC0 (NDC_IDX_E::NPA_U(0)) ports and the PORT index of
 * NDC_AF_PORT()_RT()_RW()_REQ_PC and NDC_AF_PORT()_RT()_RW()_LAT_PC.
 */
#define CAVM_NPA_NDC0_PORT_E_AURA0 (0)
#define CAVM_NPA_NDC0_PORT_E_AURA1 (1)
#define CAVM_NPA_NDC0_PORT_E_POOL0 (2)
#define CAVM_NPA_NDC0_PORT_E_POOL1 (3)
#define CAVM_NPA_NDC0_PORT_E_STACK0 (4)
#define CAVM_NPA_NDC0_PORT_E_STACK1 (5)

/**
 * Enumeration npa_pool_err_int_e
 *
 * NPA Pool Error Interrupt Enumeration
 * Enumerates the bit index of NPA_POOL_S[ERR_INT] and NPA_POOL_S[ERR_INT_ENA].
 */
#define CAVM_NPA_POOL_ERR_INT_E_OVFLS (0)
#define CAVM_NPA_POOL_ERR_INT_E_PERR (2)
#define CAVM_NPA_POOL_ERR_INT_E_RX(a) (0 + (a))
#define CAVM_NPA_POOL_ERR_INT_E_RANGE (1)

#define NPA_AF_BLK_RST                  (0x0ull)
#define NPA_AF_CONST                    (0x10ull)
#define NPA_AF_CONST1                   (0x18ull)
#define NPA_AF_LF_RST                   (0x20ull)
#define NPA_AF_GEN_CFG                  (0x30ull)
#define NPA_AF_NDC_CFG                  (0x40ull)
#define NPA_AF_NDC_SYNC                 (0x50ull)
#define NPA_AF_INP_CTL                  (0xd0ull)
#define NPA_AF_ACTIVE_CYCLES_PC         (0xf0ull)
#define NPA_AF_AVG_DELAY                (0x100ull)
#define NPA_AF_GEN_INT                  (0x140ull)
#define NPA_AF_GEN_INT_W1S              (0x148ull)
#define NPA_AF_GEN_INT_ENA_W1S          (0x150ull)
#define NPA_AF_GEN_INT_ENA_W1C          (0x158ull)
#define NPA_AF_RVU_INT                  (0x160ull)
#define NPA_AF_RVU_INT_W1S              (0x168ull)
#define NPA_AF_RVU_INT_ENA_W1S          (0x170ull)
#define NPA_AF_RVU_INT_ENA_W1C          (0x178ull)
#define NPA_AF_ERR_INT                  (0x180ull)
#define NPA_AF_ERR_INT_W1S              (0x188ull)
#define NPA_AF_ERR_INT_ENA_W1S          (0x190ull)
#define NPA_AF_ERR_INT_ENA_W1C          (0x198ull)
#define NPA_AF_RAS                      (0x1a0ull)
#define NPA_AF_RAS_W1S                  (0x1a8ull)
#define NPA_AF_RAS_ENA_W1S              (0x1b0ull)
#define NPA_AF_RAS_ENA_W1C              (0x1b8ull)
#define NPA_AF_AQ_CFG                   (0x600ull)
#define NPA_AF_AQ_BASE                  (0x610ull)
#define NPA_AF_AQ_STATUS                (0x620ull)
#define NPA_AF_AQ_DOOR                  (0x630ull)
#define NPA_AF_AQ_DONE_WAIT             (0x640ull)
#define NPA_AF_AQ_DONE                  (0x650ull)
#define NPA_AF_AQ_DONE_ACK              (0x660ull)
#define NPA_AF_AQ_DONE_TIMER            (0x670ull)
#define NPA_AF_AQ_DONE_INT              (0x680ull)
#define NPA_AF_AQ_DONE_ENA_W1S          (0x690ull)
#define NPA_AF_AQ_DONE_ENA_W1C          (0x698ull)
#define NPA_AF_LFX_AURAS_CFG(a)         (0x4000ull | (uint64_t)(a) << 18)
#define NPA_AF_LFX_LOC_AURAS_BASE(a)    (0x4010ull | (uint64_t)(a) << 18)
#define NPA_AF_LFX_QINTS_CFG(a)         (0x4100ull | (uint64_t)(a) << 18)
#define NPA_AF_LFX_QINTS_BASE(a)        (0x4110ull | (uint64_t)(a) << 18)
#define NPA_PRIV_AF_INT_CFG             (0x10000ull)
#define NPA_PRIV_LFX_CFG(a)             (0x10010ull | (uint64_t)(a) << 8)
#define NPA_PRIV_LFX_INT_CFG(a)         (0x10020ull | (uint64_t)(a) << 8)
#define NPA_AF_RVU_LF_CFG_DEBUG         (0x10030ull)
#define NPA_AF_DTX_FILTER_CTL           (0x10040ull)

#define NPA_LF_AURA_OP_ALLOCX(a)        (0x10ull | (uint64_t)(a) << 3)
#define NPA_LF_AURA_OP_FREE0            (0x20ull)
#define NPA_LF_AURA_OP_FREE1            (0x28ull)
#define NPA_LF_AURA_OP_CNT              (0x30ull)
#define NPA_LF_AURA_OP_LIMIT            (0x50ull)
#define NPA_LF_AURA_OP_INT              (0x60ull)
#define NPA_LF_AURA_OP_THRESH           (0x70ull)
#define NPA_LF_POOL_OP_PC               (0x100ull)
#define NPA_LF_POOL_OP_AVAILABLE        (0x110ull)
#define NPA_LF_POOL_OP_PTR_START0       (0x120ull)
#define NPA_LF_POOL_OP_PTR_START1       (0x128ull)
#define NPA_LF_POOL_OP_PTR_END0         (0x130ull)
#define NPA_LF_POOL_OP_PTR_END1         (0x138ull)
#define NPA_LF_POOL_OP_INT              (0x160ull)
#define NPA_LF_POOL_OP_THRESH           (0x170ull)
#define NPA_LF_ERR_INT                  (0x200ull)
#define NPA_LF_ERR_INT_W1S              (0x208ull)
#define NPA_LF_ERR_INT_ENA_W1C          (0x210ull)
#define NPA_LF_ERR_INT_ENA_W1S          (0x218ull)
#define NPA_LF_RAS                      (0x220ull)
#define NPA_LF_RAS_W1S                  (0x228ull)
#define NPA_LF_RAS_ENA_W1C              (0x230ull)
#define NPA_LF_RAS_ENA_W1S              (0x238ull)
#define NPA_LF_QINTX_CNT(a)             (0x300ull | (uint64_t)(a) << 12)
#define NPA_LF_QINTX_INT(a)             (0x310ull | (uint64_t)(a) << 12)
#define NPA_LF_QINTX_ENA_W1S(a)         (0x320ull | (uint64_t)(a) << 12)
#define NPA_LF_QINTX_ENA_W1C(a)         (0x330ull | (uint64_t)(a) << 12)


/* Enum offsets */

#define NPA_AQ_COMP_NOTDONE                 (0x0ull)
#define NPA_AQ_COMP_GOOD                    (0x1ull)
#define NPA_AQ_COMP_SWERR                   (0x2ull)
#define NPA_AQ_COMP_CTX_POISON              (0x3ull)
#define NPA_AQ_COMP_CTX_FAULT               (0x4ull)
#define NPA_AQ_COMP_LOCKERR                 (0x5ull)

#define NPA_AF_INT_VEC_RVU                  (0x0ull)
#define NPA_AF_INT_VEC_GEN                  (0x1ull)
#define NPA_AF_INT_VEC_AQ_DONE              (0x2ull)
#define NPA_AF_INT_VEC_AF_ERR               (0x3ull)
#define NPA_AF_INT_VEC_POISON               (0x4ull)

#define NPA_AQ_INSTOP_NOP                   (0x0ull)
#define NPA_AQ_INSTOP_INIT                  (0x1ull)
#define NPA_AQ_INSTOP_WRITE                 (0x2ull)
#define NPA_AQ_INSTOP_READ                  (0x3ull)
#define NPA_AQ_INSTOP_LOCK                  (0x4ull)
#define NPA_AQ_INSTOP_UNLOCK                (0x5ull)

#define NPA_AQ_CTYPE_AURA                   (0x0ull)
#define NPA_AQ_CTYPE_POOL                   (0x1ull)

#define NPA_BPINTF_NIX0_RX                  (0x0ull)
#define NPA_BPINTF_NIX1_RX                  (0x1ull)

#define NPA_AURA_ERR_INT_AURA_FREE_UNDER    (0x0ull)
#define NPA_AURA_ERR_INT_AURA_ADD_OVER      (0x1ull)
#define NPA_AURA_ERR_INT_AURA_ADD_UNDER     (0x2ull)
#define NPA_AURA_ERR_INT_POOL_DIS           (0x3ull)
#define NPA_AURA_ERR_INT_R4                 (0x4ull)
#define NPA_AURA_ERR_INT_R5                 (0x5ull)
#define NPA_AURA_ERR_INT_R6                 (0x6ull)
#define NPA_AURA_ERR_INT_R7                 (0x7ull)

#define NPA_LF_INT_VEC_ERR_INT              (0x40ull)
#define NPA_LF_INT_VEC_POISON               (0x41ull)
#define NPA_LF_INT_VEC_QINT_END             (0x3full)
#define NPA_LF_INT_VEC_QINT_START           (0x0ull)

#define NPA_INPQ_SSO                        (0x4ull)
#define NPA_INPQ_TIM                        (0x5ull)
#define NPA_INPQ_DPI                        (0x6ull)
#define NPA_INPQ_AURA_OP                    (0xeull)
#define NPA_INPQ_INTERNAL_RSV               (0xfull)
#define NPA_INPQ_NIX0_RX                    (0x0ull)
#define NPA_INPQ_NIX1_RX                    (0x2ull)
#define NPA_INPQ_NIX0_TX                    (0x1ull)
#define NPA_INPQ_NIX1_TX                    (0x3ull)
#define NPA_INPQ_R_END                      (0xdull)
#define NPA_INPQ_R_START                    (0x7ull)

#define NPA_POOL_ERR_INT_OVFLS              (0x0ull)
#define NPA_POOL_ERR_INT_RANGE              (0x1ull)
#define NPA_POOL_ERR_INT_PERR               (0x2ull)
#define NPA_POOL_ERR_INT_R3                 (0x3ull)
#define NPA_POOL_ERR_INT_R4                 (0x4ull)
#define NPA_POOL_ERR_INT_R5                 (0x5ull)
#define NPA_POOL_ERR_INT_R6                 (0x6ull)
#define NPA_POOL_ERR_INT_R7                 (0x7ull)

#define NPA_NDC0_PORT_AURA0                 (0x0ull)
#define NPA_NDC0_PORT_AURA1                 (0x1ull)
#define NPA_NDC0_PORT_POOL0                 (0x2ull)
#define NPA_NDC0_PORT_POOL1                 (0x3ull)
#define NPA_NDC0_PORT_STACK0                (0x4ull)

/**
 * Structure npa_aq_inst_s
 *
 * NPA Admin Queue Instruction Structure
 * This structure specifies the AQ instruction.
 * Instructions and associated software structures are stored in memory as
 * little-endian unless NPA_AF_GEN_CFG[AF_BE] is set.
 *
 * Hardware reads of NPA_AQ_INST_S do not allocate into LLC.
 *
 * Hardware reads and writes of the context structure selected by [CTYPE], [LF]
 * and [CINDEX] use the NDC and LLC caching style configured for that context,
 * i.e.:
 * * NPA_AURA_HW_S reads and writes use NPA_AF_LF()_AURAS_CFG[CACHING] and
 * NPA_AF_LF()_AURAS_CFG[WAY_MASK].
 * * NPA_POOL_HW_S reads and writes use NPA_AURA_HW_S[POOL_CACHING] and
 * NPA_AURA_HW_S[POOL_WAY_MASK].
 */
union cavm_npa_aq_inst_s
{
    uint64_t u[2];
    struct cavm_npa_aq_inst_s_s
    {
        uint64_t op                    : 4;  /**< [  3:  0] Instruction op code enumerated by NPA_AQ_INSTOP_E. */
        uint64_t ctype                 : 4;  /**< [  7:  4] Context type of instruction enumerated by NPA_AQ_CTYPE_E. */
        uint64_t lf                    : 9;  /**< [ 16:  8] Local function. Software must map the LF to a PF and function with
                                                                 NPA_PRIV_LF()_CFG[PF_FUNC] before issuing the AQ instruction.
                                                                 NPA_PRIV_LF()_CFG[ENA] is not required to be set when executing AQ
                                                                 instructions.

                                                                 Internal:
                                                                 Hardware uses PF(0)'s stream ID when accessing hardware context structures
                                                                 in LLC/DRAM, but NDC tracks the LF for context structures in its cache
                                                                 using the PF_FUNC's stream ID. */
        uint64_t reserved_17_23        : 7;
        uint64_t cindex                : 20; /**< [ 43: 24] Context index. Aura index of the instruction within [LF]. */
        uint64_t reserved_44_62        : 19;
        uint64_t doneint               : 1;  /**< [ 63: 63] Done interrupt.
                                                                 0 = No interrupts related to this instruction.
                                                                 1 = When the instruction completes, NPA_AF_AQ_DONE[DONE] will be incremented,
                                                                 and based on the rules described there, an interrupt may occur. */
        uint64_t res_addr              : 64; /**< [127: 64] Result AF IOVA. Specifies where to write NPA_AQ_RES_S.

                                                                 Bits \<6:0\> must be zero; address must be 128-byte aligned. Bits \<63:53\> are
                                                                 ignored by hardware; software should use a sign-extended bit \<52\> for
                                                                 forward compatibility.

                                                                 Software must reserve one, two or three 128-byte cache lines at this
                                                                 address, as follows:
                                                                 * When [OP] = NPA_AQ_INSTOP_E::INIT or READ, software must reserve at least
                                                                 two cache lines.
                                                                 * When [OP] = NPA_AQ_INSTOP_E::WRITE, software must reserve at least three
                                                                 cache lines.
                                                                 * Otherwise, software must reserve at least one cache line.

                                                                 Hardware always stores full cache lines when writing NPA_AQ_RES_S and
                                                                 following NPA_AURA_S/NPA_POOL_S structures, if any.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<6:0\> are ignored by hardware, treated as always 0x0. */
    } s;
};

/**
 * Structure npa_aq_res_s
 *
 * NPA Admin Queue Result Structure
 * NPA writes this structure after it completes the NPA_AQ_INST_S instruction.
 * The result structure is exactly 16 bytes, and each instruction completion produces
 * exactly one result structure.
 *
 * Results and associated software structures are stored in memory as
 * little-endian unless NPA_AF_GEN_CFG[AF_BE] is set.
 *
 * When [OP] = NPA_AQ_INSTOP_E::INIT, WRITE or READ, this structure is
 * immediately followed by context read or write data. See NPA_AQ_INSTOP_E.
 *
 * Hardware writes of NPA_AQ_RES_S and context data always allocate into LLC.
 * Hardware reads of context data do not allocate into LLC.
 */
union cavm_npa_aq_res_s
{
    uint64_t u[2];
    struct cavm_npa_aq_res_s_s
    {
        uint64_t op                    : 4;  /**< [  3:  0] Copy of NPA_AQ_INST_S[OP] for the completed instruction; enumerated
                                                                 by NPA_AQ_INSTOP_E. */
        uint64_t ctype                 : 4;  /**< [  7:  4] Copy of NPA_AQ_INST_S[CTYPE] for the completed instruction; enumerated by
                                                                 NPA_AQ_CTYPE_E. */
        uint64_t compcode              : 8;  /**< [ 15:  8] Indicates completion/error status of the NPA coprocessor for the associated
                                                                 instruction, as enumerated by NPA_AQ_COMP_E. Core software may write the memory
                                                                 location containing [COMPCODE] to 0x0 before ringing the doorbell, and then poll
                                                                 for completion by checking for a nonzero value.

                                                                 Once the core observes a nonzero [COMPCODE] value in this case, NPA will have also
                                                                 completed LLC/DRAM reads and writes for the operation. */
        uint64_t doneint               : 1;  /**< [ 16: 16] Done interrupt. This bit is copied from the corresponding instruction's
                                                                 NPA_AQ_INST_S[DONEINT]. */
        uint64_t reserved_17_63        : 47;
        uint64_t reserved_64_127       : 64;
    } s;
};

/**
 * Structure npa_aura_op_wdata_s
 *
 * NPA Aura Operation Write Data Structure
 * This structure specifies the write data format of a 64-bit atomic load-and-add
 * to NPA_LF_AURA_OP_ALLOC() and NPA_LF_POOL_OP_PC, and a 128-bit atomic CASP
 * operation to NPA_LF_AURA_OP_ALLOC().
 */
union cavm_npa_aura_op_wdata_s
{
    uint64_t u;
    struct cavm_npa_aura_op_wdata_s_s
    {
        uint64_t aura                  : 20; /**< [ 19:  0] Aura of instruction. */
        uint64_t reserved_20_62        : 43;
        uint64_t drop                  : 1;  /**< [ 63: 63] Perform DROP processing on allocation. See NPA_AURA_S[POOL_DROP],
                                                                 NPA_AURA_S[AURA_DROP].
                                                                 Not used when accessing NPA_LF_POOL_OP_PC. */
    } s;
};

/**
 * Structure npa_aura_s
 *
 * NPA Aura Context Structure
 * This structure specifies the format used by software with the NPA admin queue
 * to read and write an aura's NPA_AURA_HW_S structure maintained by hardware in
 * LLC/DRAM.
 */
union cavm_npa_aura_s
{
    uint64_t u[8];
    struct cavm_npa_aura_s_s
    {
        uint64_t pool_addr             : 64; /**< [ 63:  0] AF IOVA of the associated pool's NPA_POOL_HW_S structure in NDC/LLC/DRAM. The
                                                                 size of the structure is 1 \<\< NPA_AF_CONST1[POOL_LOG2BYTES] bytes.

                                                                 Bits \<5:0\> must be zero; address must be 64-byte aligned. Bits \<63:53\> are
                                                                 ignored by hardware; software should use a sign-extended bit \<52\> for forward
                                                                 compatibility.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<5:0\> are ignored by hardware, treated as always 0x0. */
        uint64_t ena                   : 1;  /**< [ 64: 64] Enable. If clear any allocations will fail and returns will be dropped. */
        uint64_t reserved_65_66        : 2;
        uint64_t pool_caching          : 1;  /**< [ 67: 67] Selects the style read for accessing NPA_POOL_HW_S in LLC/DRAM:
                                                                 0x0 = NPA_POOL_HW_S reads will not allocate into the LLC.
                                                                 0x1 = NPA_POOL_HW_S reads are allocated into the LLC.

                                                                 NPA_POOL_HW_S writes that are not allocated in NDC will always allocate
                                                                 into LLC. */
        uint64_t pool_way_mask         : 16; /**< [ 83: 68] Way partitioning mask for allocating associated NPA_POOL_HW_S in NDC (1 means do not use).
                                                                 Internal:
                                                                 Bypass NDC when all ones. */
        uint64_t avg_con               : 9;  /**< [ 92: 84] This value controls how much of the present average resource level is used
                                                                 to calculate the new resource level. The value is a number from 0 to 256,
                                                                 which represents [AVG_CON]/256 of the average resource level that will be
                                                                 used in the calculation.

                                                                 NPA updates the average resource level as follows whenever the immediate resource
                                                                 count changes:

                                                                 \<pre\>
                                                                 // norm_CNT = 255 - (8-bit shifted and saturated aura count); see [SHIFT].
                                                                 adjusted_CON = [AVG_CON] \>\> log2(NPA_AF_AVG_DELAY[AVG_TIMER] - [UPDATE_TIME]);
                                                                 [AVG_LEVEL] = (adjusted_CON * [AVG_LEVEL] + (256 - adjusted_CON) * norm_CNT) / 256;
                                                                 [UPDATE_TIME] = NPA_AF_AVG_DELAY[AVG_TIMER];
                                                                 \</pre\>

                                                                 Note setting this value to zero will disable averaging, and always use the most
                                                                 immediate levels. NPA_AF_AVG_DELAY[AVG_DLY] controls the periodicity of the level
                                                                 calculations. */
        uint64_t reserved_93           : 1;
        uint64_t pool_drop_ena         : 1;  /**< [ 94: 94] Enable aura-unique pool DROP based on the [POOL_DROP] level. */
        uint64_t aura_drop_ena         : 1;  /**< [ 95: 95] Enable aura DROP based on the [AURA_DROP] level. */
        uint64_t bp_ena                : 2;  /**< [ 97: 96] Enable aura backpressure to NIX-RX based on [BP] level. One bit per
                                                                 NIX-RX; index enumerated by NPA_BPINTF_E. */
        uint64_t reserved_98_103       : 6;
        uint64_t aura_drop             : 8;  /**< [111:104] If [AURA_DROP_ENA] is set and DROP processing is requested, the packet will
                                                                 be dropped if the current 8-bit shifted and saturated aura count is equal
                                                                 to or greater than this value. */
        uint64_t shift                 : 6;  /**< [117:112] Right shift to aura [COUNT] to create a narrower depth for aura QOS and backpressure
                                                                 calculations. NPA saturates the aura [COUNT] to 8-bits for the aura, and compares this
                                                                 8-bit shifted and saturated count directly to [AURA_DROP] and [BP]. */
        uint64_t reserved_118_119      : 2;
        uint64_t avg_level             : 8;  /**< [127:120] Current moving average of the 8-bit shifted and saturated aura count. The
                                                                 higher [AVG_LEVEL] is, the more free resources. The lower levels indicate
                                                                 buffer exhaustion. See [SHIFT] and [AVG_CON].

                                                                 NPA_INPQ_E::NIX()_RX uses [AVG_LEVEL] in receive queue QOS calculations. */
        uint64_t count                 : 36; /**< [163:128] Number of pointers allocated to the aura. Increments on ALLOC and decrements on FREE. */
        uint64_t reserved_164_167      : 4;
        uint64_t nix0_bpid             : 9;  /**< [176:168] NIX(0) RX BPID (BPID index of NIX_AF_RX_BPID()_STATUS) to which backpressure
                                                                 is asserted when the corresponding [BP_ENA] bit is set. */
        uint64_t reserved_177_179      : 3;
        uint64_t nix1_bpid             : 9;  /**< [188:180] Reserved.
                                                                 Internal:
                                                                 NIX(1) RX BPID (BPID index of NIX_AF_RX_BPID()_STATUS) to which backpressure
                                                                 is asserted when the corresponding [BP_ENA] bit is set. */
        uint64_t reserved_189_191      : 3;
        uint64_t limit                 : 36; /**< [227:192] When the aura [COUNT] is equal to or greater than this value, any allocations
                                                                 using this aura will fail. This allows a hard resource division between multiple
                                                                 auras sharing a common pool. */
        uint64_t reserved_228_231      : 4;
        uint64_t bp                    : 8;  /**< [239:232] Backpressure to [NIX0_BPID]/[NIX1_BPID] NPA_BPINTF_E::NIX()_RX is asserted if
                                                                 the corresponding [BP_ENA] bit is set and the current 8-bit shifted and
                                                                 saturated aura [COUNT] is greater than or equal to this value. */
        uint64_t reserved_240_243      : 4;
        uint64_t fc_ena                : 1;  /**< [244:244] Enable flow control. When enabled NPA will periodically store the [COUNT] value
                                                                 as an unsigned 64-bit to the LF IOVA specified by [FC_ADDR] for flow control
                                                                 purposes. The frequency of the stores is controlled via [FC_HYST_BITS].

                                                                 When set, software should also enable the associated pool's flow control
                                                                 with NPA_POOL_S[FC_ENA] and monitor the pool level stored in LLC/DRAM. */
        uint64_t fc_up_crossing        : 1;  /**< [245:245] Flow control up-crossing flag. Set on an aura count up-crossing, and cleared on
                                                                 a down-crossing. see [FC_HYST_BITS]. NPA maintains this value and software may
                                                                 ignore it. */
        uint64_t fc_stype              : 2;  /**< [247:246] Type of store to write [COUNT] in LLC/DRAM:
                                                                 0x0 = Store full cache line, allocate cache (STF).
                                                                 0x1 = Store full cache line, no allocate (STT).
                                                                 0x2 = Store partial cache line, allocate cache (STP).
                                                                 0x3 = Reserved. */
        uint64_t fc_hyst_bits          : 4;  /**< [251:248] Flow control hysteresis bits. Use hysteresis to reduce the number of stores that
                                                                 NPA does to update [COUNT] in LLC/DRAM. Hysteresis is accomplished by monitoring
                                                                 a range of least significant bits of [COUNT] and triggering stores as follows:

                                                                 * When [FC_HYST_BITS] == 0, no count bits are monitored.

                                                                 * Otherwise, count bits \<[FC_HYST_BITS]-1:0\> are monitored.

                                                                 For purposes of describing the hysteresis algorithm, a down-crossing is defined
                                                                 as the case where a count decrement causes the specified LSB range to transition
                                                                 from all zeros to all ones, and an up-crossing as the case where a count
                                                                 increment causes the specified LSB range to transition from all ones to all
                                                                 zeros. A zero-crossing is defined as a down-crossing or up-crossing.

                                                                 The hysteresis algorithm triggers stores upon down-crossings that follow a
                                                                 down-crossing and upon up-crossings that follow an up-crossing. Conversely
                                                                 down-crossings that follow an up-crossing and up-crossings that follow a
                                                                 down-crossing do not trigger stores. This prevents the repeated updates that
                                                                 would otherwise occur if the count oscillated around some zero-crossing. When
                                                                 [FC_HYST_BITS] = 0, there is no hysteresis and all count updates trigger stores. */
        uint64_t reserved_252_255      : 4;
        uint64_t fc_addr               : 64; /**< [319:256] Flow control address. LF IOVA in LLC/DRAM to write the count. See also
                                                                 [FC_ENA] and [FC_STYPE]. Must be on a dedicated 128-byte cache line when
                                                                 [FC_STYPE] indicates full cache line.

                                                                 Bits \<2:0\> must be zero; address must be 8-byte aligned.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit \<52\> for
                                                                 forward compatibility.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<2:0\> are ignored by hardware, treated as always 0x0. */
        uint64_t pool_drop             : 8;  /**< [327:320] If [POOL_DROP_ENA] is set and DROP processing is requested, the packet will
                                                                 be dropped if the current 8-bit shifted and saturated free pointer count in
                                                                 the aura's pool is less than or equal this value. See
                                                                 NPA_POOL_S[SHIFT] and NPA_POOL_S[STACK_PAGES]. */
        uint64_t update_time           : 16; /**< [343:328] NPA_AF_AVG_DELAY[AVG_TIMER] value captured when [AVG_LEVEL] is updated.
                                                                 NPA maintains this value and software may ignore it. */
        uint64_t err_int               : 8;  /**< [351:344] Error interrupts. Bits enumerated by NPA_AURA_ERR_INT_E, which also defines
                                                                 when hardware sets each bit. Software can read, set or clear these bits
                                                                 with NPA_LF_AURA_OP_INT. */
        uint64_t err_int_ena           : 8;  /**< [359:352] Error interrupt enables. Bits enumerated by NPA_AURA_ERR_INT_E. Software
                                                                 can read, set or clear these bits with NPA_LF_AURA_OP_INT. */
        uint64_t thresh_int            : 1;  /**< [360:360] Threshold interrupt. When [THRESH_UP] is set, hardware sets this bit when
                                                                 [COUNT] goes to or above [THRESH]. When [THRESH_UP] is clear, hardware sets
                                                                 this bit when [COUNT] drops below [THRESH].

                                                                 Software can read, set or clear this bit with NPA_LF_AURA_OP_INT.

                                                                 Internal:
                                                                 "When [THRESH_UP]==0:
                                                                 * Interrupt set to QINT when ([THRESH_INT] &
                                                                 [THRESH_INT_ENA]) goes from 0 to 1.
                                                                 * Interrupt clear to QINT when ([THRESH_INT] &
                                                                 [THRESH_INT_ENA]) goes from 1 to 0.
                                                                 * Interrupt resend to QINT when [THRESH_INT_ENA] = 1, software clears
                                                                 [THRESH_INT], and [COUNT] \< [THRESH].
                                                                 Similar when [THRESH_UP]==1, except:
                                                                 * Interrupt resend to QINT when [THRESH_INT_ENA] = 1, software clears
                                                                 [THRESH_INT], and [COUNT] \>= [THRESH].
                                                                 " */
        uint64_t thresh_int_ena        : 1;  /**< [361:361] Threshold interrupt enable. Software can read, set or clear this bit with
                                                                 NPA_LF_AURA_OP_INT. */
        uint64_t thresh_up             : 1;  /**< [362:362] Threshold up direction. When set, [THRESH_INT] is set when [COUNT] rises to
                                                                 or above [THRESH]. When clear, [THRESH_INT] is set when [COUNT] drops below
                                                                 [THRESH]. Software can read or write this bit with
                                                                 NPA_LF_AURA_OP_THRESH. */
        uint64_t reserved_363          : 1;
        uint64_t thresh_qint_idx       : 7;  /**< [370:364] Threshold queue interrupt index. Select the QINT within VF/PF (QINT index
                                                                 of NPA_LF_QINT()_INT) which receives [THRESH_INT] events. */
        uint64_t reserved_371          : 1;
        uint64_t err_qint_idx          : 7;  /**< [378:372] Error queue interrupt index. Select the QINT within VF/PF (QINT index of
                                                                 NPA_LF_QINT()_INT) which receives [ERR_INT] events. */
        uint64_t reserved_379_383      : 5;
        uint64_t thresh                : 36; /**< [419:384] Interrupt threshold count. See [THRESH_INT]. Software can read or write
                                                                 this field with NPA_LF_AURA_OP_THRESH. */
        uint64_t reserved_420_447      : 28;
        uint64_t reserved_448_511      : 64;
    } s;
};

/**
 * Structure npa_pool_s
 *
 * NPA Pool Context Structure
 * This structure specifies the format used by software with the NPA admin queue
 * to read and write a pool's NPA_POOL_HW_S structure maintained by hardware in
 * LLC/DRAM.
 */
union cavm_npa_pool_s
{
    uint64_t u[16];
    struct cavm_npa_pool_s_s
    {
        uint64_t stack_base            : 64; /**< [ 63:  0] Pool stack base LF IOVA in NDC/LLC/DRAM. This is the lowest address used by the stack.

                                                                 Bits \<6:0\> must be zero; address must be 128-byte aligned.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit \<52\> for
                                                                 forward compatibility.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<6:0\> are ignored by hardware, treated as always 0x0. */
        uint64_t ena                   : 1;  /**< [ 64: 64] Enable. Must be set after writing pool configuration, if clear any allocations will fail
                                                                 and returns will be dropped. If any pool configuration is changed while this bit is set
                                                                 (or until traffic is quiesced after clearing), the NPA may operate incorrectly. */
        uint64_t nat_align             : 1;  /**< [ 65: 65] Returning buffers should be rounded to the nearest natural alignment specified with
                                                                 [BUF_SIZE]. */
        uint64_t reserved_66_67        : 2;
        uint64_t stack_caching         : 1;  /**< [ 68: 68] Selects the style read for accessing NPA_STACK_PAGE_S in LLC/DRAM:
                                                                 0x0 = NPA_STACK_PAGE_S reads will not allocate into the LLC.
                                                                 0x1 = NPA_STACK_PAGE_S reads are allocated into the LLC.

                                                                 NPA_STACK_PAGE_S writes that are not allocated in NDC will always allocate
                                                                 into LLC. */
        uint64_t reserved_69_71        : 3;
        uint64_t stack_way_mask        : 16; /**< [ 87: 72] Way partitioning mask for allocating stack pages in NDC (1 means do not use).
                                                                 Internal:
                                                                 Bypass NDC when all ones. */
        uint64_t buf_offset            : 12; /**< [ 99: 88] Number of 128-byte cache lines to offset the stored pointer. This field is sign extended
                                                                 so that two's complement numbers may be used to do subtractions.

                                                                 If [NAT_ALIGN] is clear, the pointer stored in the pool is normally the freed pointer
                                                                 adjusted by [BUF_OFFSET]. [BUF_OFFSET] will normally be zero or negative to adjust the
                                                                 pointer back to the beginning of the buffer.)

                                                                 If [NAT_ALIGN] is set, the pointer stored in the pool is normally [BUF_OFFSET] from the
                                                                 beginning of the buffer. [BUF_OFFSET] will normally be zero or positive to adjust the
                                                                 pointer into the buffer. */
        uint64_t reserved_100_103      : 4;
        uint64_t buf_size              : 11; /**< [114:104] Buffer size in 128-byte cache lines. Must be zero if [NAT_ALIGN] is clear. Buffer sizes
                                                                 are supported that are any multiple of 128 bytes in the range of 128 bytes to 128 KB. */
        uint64_t reserved_115_127      : 13;
        uint64_t stack_max_pages       : 32; /**< [159:128] Maximum number of pages in the stack starting at [STACK_BASE], with
                                                                 NPA_AF_CONST[STACK_PAGE_PTRS] free pointers per page. Stack is full if
                                                                 [STACK_PAGES] equals this value.

                                                                 Internal:
                                                                 Provides more than 2^35 pointers, 2^42 phisical memory with one cache line
                                                                 per pointer. */
        uint64_t stack_pages           : 32; /**< [191:160] Number of nonpartial pages in the stack, with NPA_AF_CONST[STACK_PAGE_PTRS]
                                                                 pointers per page. Must be initialized to zero when stack is created.
                                                                 The stack page format is defined by NPA_STACK_PAGE_S.
                                                                 The number of free pointers in this pool is:
                                                                 _ COUNT = NPA_AF_CONST[STACK_PAGE_PTRS]*[STACK_PAGES] + [STACK_OFFSET]. */
        uint64_t op_pc                 : 48; /**< [239:192] Performance counter. Number of allocations or returns performed to this pool, including
                                                                 those that fail, e.g. due to DROP, stack empty/full, etc. */
        uint64_t reserved_240_255      : 16;
        uint64_t stack_offset          : 4;  /**< [259:256] Number of valid pointers in partially populated NPA_STACK_PAGE_S following
                                                                 the number of full pages specified by [STACK_PAGES]. Must be initialized to
                                                                 0 when stack is created. */
        uint64_t reserved_260_263      : 4;
        uint64_t shift                 : 6;  /**< [269:264] Right shift to COUNT (see [STACK_PAGES]) used to create a narrower depth for pool QOS
                                                                 calculations. NPA saturates the shifted COUNT to 8-bits, and compares this 8-bit shifted
                                                                 and saturated count directly to NPA_AURA_S[POOL_DROP]. */
        uint64_t reserved_270_271      : 2;
        uint64_t avg_level             : 8;  /**< [279:272] Current moving average of the 8-bit shifted and saturated COUNT (see
                                                                 [STACK_PAGES]). The higher [AVG_LEVEL] is, the more free resources. The
                                                                 lowest levels indicate buffer exhaustion. See [SHIFT] and [AVG_CON].

                                                                 NPA_INPQ_E::NIX()_RX uses [AVG_LEVEL] in receive queue QOS calculations.
                                                                 See NIX_RQ_CTX_S[XQE_DROP]. */
        uint64_t avg_con               : 9;  /**< [288:280] This value controls how much of the present average resource level is used
                                                                 to calculate the new resource level. The value is a number from 0 to 256,
                                                                 which represents [AVG_CON]/256 of the average resource level that will be
                                                                 used in the calculation.

                                                                 NPA updates the average resource level as follows whenever the immediate resource
                                                                 count changes:

                                                                 \<pre\>
                                                                 // norm_CNT = 8-bit shifted and saturated pool count; see [SHIFT].
                                                                 adjusted_CON = [AVG_CON] \>\> log2(NPA_AF_AVG_DELAY[AVG_TIMER] - [UPDATE_TIME]);
                                                                 [AVG_LEVEL] = (adjusted_CON * [AVG_LEVEL] + (256 - adjusted_CON) * norm_CNT) / 256;
                                                                 [UPDATE_TIME] = NPA_AF_AVG_DELAY[AVG_TIMER];
                                                                 \</pre\>

                                                                 Note setting this value to zero will disable averaging, and always use the most
                                                                 immediate levels. NPA_AF_AVG_DELAY[AVG_DLY] controls the periodicity of the level
                                                                 calculations. */
        uint64_t fc_ena                : 1;  /**< [289:289] Enable flow control. When enabled NPA will periodically store the free
                                                                 pointer COUNT value (see [STACK_PAGES]) as an unsigned 64-bit to the IOVA
                                                                 specified by [FC_ADDR] for flow control purposes. The frequency of the
                                                                 stores is controlled via [FC_HYST_BITS]. */
        uint64_t fc_stype              : 2;  /**< [291:290] Type of store to write COUNT in LLC/DRAM:
                                                                 0x0 = Store full cache line, allocate cache (STF).
                                                                 0x1 = Store full cache line, no allocate (STT).
                                                                 0x2 = Store partial cache line, allocate cache (STP).
                                                                 0x3 = Reserved. */
        uint64_t fc_hyst_bits          : 4;  /**< [295:292] Flow control hysteresis bits. Use hysteresis to reduce the number of stores that
                                                                 NPA does to update COUNT in LLC/DRAM. Hysteresis is accomplished by monitoring a
                                                                 range of least significant bits of COUNT and triggering stores as follows:

                                                                 * When [FC_HYST_BITS] == 0, no count bits are monitored.

                                                                 * Otherwise, count bits \<[FC_HYST_BITS]-1:0\> are monitored.

                                                                 For purposes of describing the hysteresis algorithm, a down-crossing is defined
                                                                 as the case where a count decrement causes the specified LSB range to transition
                                                                 from all zeros to all ones, and an up-crossing as the case where a count
                                                                 increment causes the specified LSB range to transition from all ones to all
                                                                 zeros. A zero-crossing is defined as a down-crossing or up-crossing.

                                                                 The hysteresis algorithm triggers stores upon down-crossings that follow a
                                                                 down-crossing and upon up-crossings that follow an up-crossing. Conversely
                                                                 down-crossings that follow an up-crossing and up-crossings that follow a
                                                                 down-crossing do not trigger stores. This prevents the repeated updates that
                                                                 would otherwise occur if the count oscillated around some zero-crossing. When
                                                                 [FC_HYST_BITS] = 0, there is no hysteresis and all count updates trigger stores. */
        uint64_t fc_up_crossing        : 1;  /**< [296:296] Flow control up-crossing flag. Set on an aura count up-crossing, and cleared on
                                                                 a down-crossing. see [FC_HYST_BITS]. NPA maintains this value and software may
                                                                 ignore it. */
        uint64_t reserved_297_299      : 3;
        uint64_t update_time           : 16; /**< [315:300] NPA_AF_AVG_DELAY[AVG_TIMER] value captured when [AVG_LEVEL] is updated.
                                                                 NPA maintains this value and software may ignore it. */
        uint64_t reserved_316_319      : 4;
        uint64_t fc_addr               : 64; /**< [383:320] Flow control address. LF IOVA in LLC/DRAM to write the count. See also
                                                                 [FC_ENA] and [FC_STYPE]. Must be on a dedicated 128-byte cache line when
                                                                 [FC_STYPE] indicates full cache line.

                                                                 Bits \<2:0\> must be zero; address must be 8-byte aligned.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit \<52\> for
                                                                 forward compatibility.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<2:0\> are ignored by hardware, treated as always 0x0. */
        uint64_t ptr_start             : 64; /**< [447:384] Pointer start LF IOVA. Pointers freed to this pool after alignment must be
                                                                 greater that or equal to this value. Typically nonzero so that a NULL
                                                                 pointer free causes an exception.

                                                                 Bits \<6:0\> must be zero; address must be 128-byte aligned. Bits \<63:53\> are
                                                                 ignored by hardware; software should use a sign-extended bit \<52\> for forward
                                                                 compatibility.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<6:0\> are ignored by hardware, treated as always 0x0. */
        uint64_t ptr_end               : 64; /**< [511:448] Pointer end LF IOVA. Pointers freed to this pool after alignment must be equal to or
                                                                 less than this address.

                                                                 Bits \<6:0\> must be zero; address must be 128-byte aligned. Bits \<63:53\> are
                                                                 ignored by hardware; software should use a sign-extended bit \<52\> for forward
                                                                 compatibility.

                                                                 Internal:
                                                                 Bits \<63:53\>, \<6:0\> are ignored by hardware, treated as always 0x0. */
        uint64_t reserved_512_535      : 24;
        uint64_t err_int               : 8;  /**< [543:536] Error interrupts. Bits enumerated by NPA_POOL_ERR_INT_E, which also defines
                                                                 when hardware sets each bit. Software can read, set or clear these bits
                                                                 with NPA_LF_POOL_OP_INT. */
        uint64_t err_int_ena           : 8;  /**< [551:544] Error interrupt enables. Bits enumerated by NPA_POOL_ERR_INT_E. Software
                                                                 can read, set or clear these bits with NPA_LF_POOL_OP_INT. */
        uint64_t thresh_int            : 1;  /**< [552:552] Threshold interrupt. When [THRESH_UP] is set, hardware sets this bit when
                                                                 the free pointer COUNT (see [STACK_PAGES]) goes to or above [THRESH]. When
                                                                 [THRESH_UP] is clear, hardware sets this bit when COUNT drops below
                                                                 [THRESH].

                                                                 Software can read, set or clear this bit with NPA_LF_POOL_OP_INT.

                                                                 Internal:
                                                                 See NPA_AURA_S[THRESH_INT]. */
        uint64_t thresh_int_ena        : 1;  /**< [553:553] Threshold interrupt enable. Software can read, set or clear this bit with
                                                                 NPA_LF_POOL_OP_INT. */
        uint64_t thresh_up             : 1;  /**< [554:554] Threshold up direction. When set, [THRESH_INT] is set when COUNT (see
                                                                 [STACK_PAGES]) rises to or above [THRESH]. When clear, [THRESH_INT] is set
                                                                 when COUNT drops below [THRESH]. Software can read, set or clear this bit
                                                                 with NPA_LF_POOL_OP_THRESH. */
        uint64_t reserved_555          : 1;
        uint64_t thresh_qint_idx       : 7;  /**< [562:556] Threshold queue interrupt index. Select the QINT within VF/PF (QINT index
                                                                 of NPA_LF_QINT()_INT) which receives [THRESH_INT] events. */
        uint64_t reserved_563          : 1;
        uint64_t err_qint_idx          : 7;  /**< [570:564] Error queue interrupt index. Select the QINT within VF/PF (QINT index of
                                                                 NPA_LF_QINT()_INT) which receives [ERR_INT] events. */
        uint64_t reserved_571_575      : 5;
        uint64_t thresh                : 36; /**< [611:576] Interrupt threshold count. See [THRESH_INT]. Software can read or write
                                                                 this field with NPA_LF_POOL_OP_THRESH. */
        uint64_t reserved_612_639      : 28;
        uint64_t reserved_640_703      : 64;
        uint64_t reserved_704_767      : 64;
        uint64_t reserved_768_831      : 64;
        uint64_t reserved_832_895      : 64;
        uint64_t reserved_896_959      : 64;
        uint64_t reserved_960_1023     : 64;
    } s;
};

/**
 * Structure npa_qint_hw_s
 *
 * NPA Queue Interrupt Context Hardware Structure
 * This structure contains context state maintained by hardware for each queue
 * interrupt (QINT) in in NDC/LLC/DRAM. Software accesses this structure with the
 * NPA_LF_QINT()_* registers.
 * Hardware maintains a table of NPA_AF_CONST[QINTS] contiguous NPA_QINT_HW_S
 * structures per LF starting at IOVA NPA_AF_LF()_QINTS_BASE.
 * Always stored in byte invariant little-endian format (LE8).
 */
union cavm_npa_qint_hw_s
{
    uint64_t u;
    struct cavm_npa_qint_hw_s_s
    {
        uint64_t count                 : 22; /**< [ 21:  0] Interrupt count. See NPA_LF_QINT()_CNT[COUNT]. */
        uint64_t reserved_22_30        : 9;
        uint64_t ena                   : 1;  /**< [ 31: 31] Interrupt enable. See also NPA_LF_QINT()_ENA_W1S[INTR] and
                                                                 NPA_LF_QINT()_ENA_W1C[INTR]. */
        uint64_t reserved_32_63        : 32;
    } s;
    struct cavm_npa_qint_hw_s_cn
    {
        uint64_t count                 : 22; /**< [ 21:  0] Interrupt count. See NPA_LF_QINT()_CNT[COUNT]. */
        uint64_t reserved_22_30        : 9;
        uint64_t ena                   : 1;  /**< [ 31: 31] Interrupt enable. See also NPA_LF_QINT()_ENA_W1S[INTR] and
                                                                 NPA_LF_QINT()_ENA_W1C[INTR]. */
    } cn;
};

/**
 * Register (RVU_PF_BAR0) npa_af_active_cycles_pc
 *
 * NPA AF Active Cycles Register
 */
union cavm_npa_af_active_cycles_pc
{
    uint64_t u;
    struct cavm_npa_af_active_cycles_pc_s
    {
        uint64_t act_cyc               : 64; /**< [ 63:  0](R/W/H) Counts every coprocessor-clock cycle that the conditional clocks are active. */
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_base
 *
 * NPA AF Admin Queue Base Address Register
 */
union cavm_npa_af_aq_base
{
    uint64_t u;
    struct cavm_npa_af_aq_base_s
    {
        uint64_t reserved_0_6          : 7;
        uint64_t base_addr             : 46; /**< [ 52:  7](R/W) AF IOVA\<52:7\> of AQ ring in LLC/DRAM. IOVA bits \<6:0\> are always zero. */
        uint64_t reserved_53_63        : 11;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_cfg
 *
 * NPA AF Admin Queue Configuration Register
 */
union cavm_npa_af_aq_cfg
{
    uint64_t u;
    struct cavm_npa_af_aq_cfg_s
    {
        uint64_t qsize                 : 4;  /**< [  3:  0](R/W) Specifies AQ ring size in entries of 16 bytes:
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
        uint64_t reserved_4_63         : 60;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done
 *
 * NPA AF AQ Done Count Register
 */
union cavm_npa_af_aq_done
{
    uint64_t u;
    struct cavm_npa_af_aq_done_s
    {
        uint64_t done                  : 20; /**< [ 19:  0](R/W/H) Done count. When NPA_AQ_INST_S[DONEINT] set and that instruction completes,
                                                                 NPA_AF_AQ_DONE[DONE] is incremented. Write to this field are for diagnostic
                                                                 use only; instead software writes NPA_AF_AQ_DONE_ACK with the number of
                                                                 decrements for this field.

                                                                 Interrupts are sent as follows:

                                                                 * When NPA_AF_AQ_DONE[DONE] = 0, then no results are pending, the interrupt
                                                                 coalescing (NPA_AF_AQ_DONE_TIMER[COUNT]) timer is held to zero, and an
                                                                 interrupt is not sent.

                                                                 * When NPA_AF_AQ_DONE[DONE] != 0, then NPA_AF_AQ_DONE_TIMER[COUNT]
                                                                 counts every microsecond. If the counter is \>= NPA_AF_AQ_DONE_WAIT[TIME_WAIT],
                                                                 or NPA_AF_AQ_DONE[DONE] \>= NPA_AF_AQ_DONE_WAIT[NUM_WAIT], i.e. enough time
                                                                 has passed or enough results have arrived, then the interrupt is sent.
                                                                 Otherwise, it is not sent due to coalescing.

                                                                 * When NPA_AF_AQ_DONE_ACK is written (or NPA_AF_AQ_DONE is written but this
                                                                 is not typical), the NPA_AF_AQ_DONE_TIMER[COUNT] restarts. Note after
                                                                 decrementing this interrupt equation is recomputed, for example if
                                                                 NPA_AF_AQ_DONE[DONE] \>= NPA_AF_AQ_DONE_WAIT[NUM_WAIT] and because the timer
                                                                 is zero, the interrupt will be resent immediately. (This covers the race
                                                                 case between software acknowledging an interrupt and a result returning.)

                                                                 * When NPA_AF_AQ_DONE_ENA_W1S[DONE] = 0, interrupts are not sent, but the
                                                                 counting described above still occurs.

                                                                 AQ instructions complete in order.

                                                                 Software is responsible for making sure [DONE] does not overflow; for example by
                                                                 insuring there are not more than 2^20-1 instructions in flight that may request
                                                                 interrupts. */
        uint64_t reserved_20_63        : 44;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_ack
 *
 * NPA AF AQ Done Count Ack Register
 * This register is written by software to acknowledge interrupts.
 */
union cavm_npa_af_aq_done_ack
{
    uint64_t u;
    struct cavm_npa_af_aq_done_ack_s
    {
        uint64_t done_ack              : 20; /**< [ 19:  0](R/W/H) Number of decrements to NPA_AF_AQ_DONE[DONE]. Reads NPA_AF_AQ_DONE[DONE].

                                                                 Written by software to acknowledge interrupts. If NPA_AF_AQ_DONE[DONE] is
                                                                 still nonzero the interrupt will be re-sent if the conditions described in
                                                                 NPA_AF_AQ_DONE[DONE] are satisfied.

                                                                 Internal:
                                                                 If [DONE_ACK] write value is greater than NPA_AF_AQ_DONE[DONE], hardware
                                                                 resets NPA_AF_AQ_DONE[DONE] to zero. */
        uint64_t reserved_20_63        : 44;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_ena_w1c
 *
 * NPA AF AQ Done Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_npa_af_aq_done_ena_w1c
{
    uint64_t u;
    struct cavm_npa_af_aq_done_ena_w1c_s
    {
        uint64_t done                  : 1;  /**< [  0:  0](R/W1C/H) Reads or clears enable for NPA_AF_AQ_DONE_INT[DONE]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_ena_w1s
 *
 * NPA AF AQ Done Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_npa_af_aq_done_ena_w1s
{
    uint64_t u;
    struct cavm_npa_af_aq_done_ena_w1s_s
    {
        uint64_t done                  : 1;  /**< [  0:  0](R/W1S/H) Reads or sets enable for NPA_AF_AQ_DONE_INT[DONE]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_int
 *
 * NPA AF AQ Done Interrupt Register
 */
union cavm_npa_af_aq_done_int
{
    uint64_t u;
    struct cavm_npa_af_aq_done_int_s
    {
        uint64_t done                  : 1;  /**< [  0:  0](RO/H) Done interrupt. See NPA_AF_AQ_DONE[DONE]. Note this bit is read-only, to
                                                                 acknowledge interrupts use NPA_AF_AQ_DONE_ACK. To test interrupts, write
                                                                 nonzero to NPA_AF_AQ_DONE[DONE]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_int_w1s
 *
 * INTERNAL: NPA AF AQ Done Interrupt Set Register
 */
union cavm_npa_af_aq_done_int_w1s
{
    uint64_t u;
    struct cavm_npa_af_aq_done_int_w1s_s
    {
        uint64_t done                  : 1;  /**< [  0:  0](RO/H) Done interrupt. See NPA_AF_AQ_DONE[DONE]. Note this bit is read-only, to
                                                                 acknowledge interrupts use NPA_AF_AQ_DONE_ACK. To test interrupts, write
                                                                 nonzero to NPA_AF_AQ_DONE[DONE]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_timer
 *
 * NPA AF Admin Queue Done Interrupt Timer Register
 * Used to debug the queue interrupt coalescing timer.
 */
union cavm_npa_af_aq_done_timer
{
    uint64_t u;
    struct cavm_npa_af_aq_done_timer_s
    {
        uint64_t count                 : 16; /**< [ 15:  0](R/W/H) Timer count. Hardware counter that increments every microsecond when
                                                                 interrupt coalescing is active; coalescing ends when the counter reaches
                                                                 NPA_AF_AQ_DONE_WAIT[TIME_WAIT]. Writes to this field are for diagnostic use
                                                                 only. See also NPA_AF_AQ_DONE[DONE]. */
        uint64_t reserved_16_63        : 48;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_done_wait
 *
 * NPA AF AQ Done Interrupt Coalescing Wait Register
 * Specifies the queue interrupt coalescing settings.
 */
union cavm_npa_af_aq_done_wait
{
    uint64_t u;
    struct cavm_npa_af_aq_done_wait_s
    {
        uint64_t num_wait              : 20; /**< [ 19:  0](R/W) Number of messages hold-off. When NPA_AF_AQ_DONE[DONE] \>= [NUM_WAIT] then
                                                                 interrupt coalescing ends; see NPA_AF_AQ_DONE[DONE]. If 0x0, same behavior
                                                                 as 0x1. */
        uint64_t reserved_20_31        : 12;
        uint64_t time_wait             : 16; /**< [ 47: 32](R/W) Time hold-off in microseconds. When NPA_AF_AQ_DONE[DONE] = 0, or
                                                                 NPA_AF_AQ_DONE_ACK is written, the interrupt coalescing timer
                                                                 (NPA_AF_AQ_DONE_TIMER[COUNT]) is cleared. The timer increments every
                                                                 microsecond, and interrupt coalescing ends when timer reaches [TIME_WAIT];
                                                                 see NPA_AF_AQ_DONE[DONE]. If 0x0, time coalescing is disabled. */
        uint64_t reserved_48_63        : 16;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_door
 *
 * NPA AF Admin Queue Doorbell Register
 * Software writes to this register to enqueue one or more entries to AQ.
 */
union cavm_npa_af_aq_door
{
    uint64_t u;
    struct cavm_npa_af_aq_door_s
    {
        uint64_t count                 : 16; /**< [ 15:  0](WO) Number of enqueued 16-byte entries. Hardware advances
                                                                 NPA_AF_AQ_STATUS[TAIL_PTR] by this value.

                                                                 A doorbell write that would overflow the AQ ring is suppressed and sets
                                                                 NPA_AF_AQ_STATUS[AQ_ERR] and NPA_AF_ERR_INT[AQ_DOOR_ERR]. */
        uint64_t reserved_16_63        : 48;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_aq_status
 *
 * NPA AF Admin Queue Status Register
 */
union cavm_npa_af_aq_status
{
    uint64_t u;
    struct cavm_npa_af_aq_status_s
    {
        uint64_t reserved_0_3          : 4;
        uint64_t head_ptr              : 20; /**< [ 23:  4](R/W/H) Head pointer \<24:4\> of AQ ring relative to NPA_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the head pointer when it pops an entry
                                                                 from the AQ. */
        uint64_t reserved_24_35        : 12;
        uint64_t tail_ptr              : 20; /**< [ 55: 36](R/W/H) Tail pointer \<24:4\> of AQ ring relative to NPA_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the tail pointer when software writes to
                                                                 NPA_AF_AQ_DOOR. */
        uint64_t reserved_56_61        : 6;
        uint64_t aq_busy               : 1;  /**< [ 62: 62](RO/H) This bit is set when an AQ command is currently being processed. */
        uint64_t aq_err                : 1;  /**< [ 63: 63](R/W1C/H) AQ error. See NPA_AF_ERR_INT[AQ_INST_FAULT,AQ_RES_FAULT,AQ_DOOR_ERR] and
                                                                 NPA_AF_RAS[AQ_INST_POISON,AQ_RES_POISON].
                                                                 When set, hardware stops reading instructions from the AQ ring. Software
                                                                 clears the error by writing a one back. */
    } s;
    struct cavm_npa_af_aq_status_cn
    {
        uint64_t reserved_0_3          : 4;
        uint64_t head_ptr              : 20; /**< [ 23:  4](R/W/H) Head pointer \<24:4\> of AQ ring relative to NPA_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the head pointer when it pops an entry
                                                                 from the AQ. */
        uint64_t reserved_24_31        : 8;
        uint64_t reserved_32_35        : 4;
        uint64_t tail_ptr              : 20; /**< [ 55: 36](R/W/H) Tail pointer \<24:4\> of AQ ring relative to NPA_AF_AQ_BASE. Address bits \<3:0\>
                                                                 are always 0x0. Hardware advances the tail pointer when software writes to
                                                                 NPA_AF_AQ_DOOR. */
        uint64_t reserved_56_61        : 6;
        uint64_t aq_busy               : 1;  /**< [ 62: 62](RO/H) This bit is set when an AQ command is currently being processed. */
        uint64_t aq_err                : 1;  /**< [ 63: 63](R/W1C/H) AQ error. See NPA_AF_ERR_INT[AQ_INST_FAULT,AQ_RES_FAULT,AQ_DOOR_ERR] and
                                                                 NPA_AF_RAS[AQ_INST_POISON,AQ_RES_POISON].
                                                                 When set, hardware stops reading instructions from the AQ ring. Software
                                                                 clears the error by writing a one back. */
    } cn;
};

/**
 * Register (RVU_PF_BAR0) npa_af_avg_delay
 *
 * NPA AF Queue Average Delay Register
 */
union cavm_npa_af_avg_delay
{
    uint64_t u;
    struct cavm_npa_af_avg_delay_s
    {
        uint64_t avg_dly               : 19; /**< [ 18:  0](R/W) Average-queue-size delay. [AVG_DLY]+1 is the number of microseconds per timer
                                                                 tick for calculating the moving average for each aura and pool level. Note the
                                                                 minimum of one microsecond implies that at 100 M packets/sec, approximately 100
                                                                 packets may arrive between average calculations.

                                                                 Larger [AVG_DLY] causes the moving averages of all aura and pool levels to track changes
                                                                 in the actual free space more slowly. Larger NPA_AURA_S[AVG_CON]) values causes
                                                                 a specific aura or pool to track more slowly, but only affects an individual level,
                                                                 rather than all. */
        uint64_t reserved_19_23        : 5;
        uint64_t avg_timer             : 16; /**< [ 39: 24](R/W/H) Running counter that is incremented every [AVG_DLY]+1 microseconds when
                                                                 [AVG_TIMER_DIS] is clear. */
        uint64_t reserved_40_62        : 23;
        uint64_t avg_timer_dis         : 1;  /**< [ 63: 63](R/W) When set, [AVG_TIMER] updates are disabled. */
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_bar2_alias#
 *
 * INTERNAL: NPA Admin Function  BAR2 Alias Registers
 *
 * These registers alias to the NPA BAR2 registers for the PF and function
 * selected by NPA_AF_BAR2_SEL[PF_FUNC].
 *
 * Internal:
 * Not implemented. Placeholder for bug33464.
 */
union cavm_npa_af_bar2_aliasx
{
    uint64_t u;
    struct cavm_npa_af_bar2_aliasx_s
    {
        uint64_t data                  : 64; /**< [ 63:  0](R/W/H) Aliased register data. */
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_bar2_sel
 *
 * INTERNAL: NPA Admin Function BAR2 Select Register
 *
 * This register configures BAR2 accesses from the NPA_AF_BAR2_ALIAS() registers in BAR0.
 * Internal:
 * Not implemented. Placeholder for bug33464.
 */
union cavm_npa_af_bar2_sel
{
    uint64_t u;
    struct cavm_npa_af_bar2_sel_s
    {
        uint64_t alias_pf_func         : 16; /**< [ 15:  0](R/W) PF and function whose BAR2 registers may be accessed from the AF BAR2 alias
                                                                 registers. Format specified by RVU_PF_FUNC_S. */
        uint64_t alias_ena             : 1;  /**< [ 16: 16](R/W) Enable BAR2 register accesses from the AF BAR2 alias registers in BAR0. */
        uint64_t reserved_17_63        : 47;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_blk_rst
 *
 * NPA AF Block Reset Register
 */
union cavm_npa_af_blk_rst
{
    uint64_t u;
    struct cavm_npa_af_blk_rst_s
    {
        uint64_t rst                   : 1;  /**< [  0:  0](WO/H) Write one to reset the block, except for privileged AF registers in PF BAR0
                                                                 (block_PRIV_*). Software must ensure that all block activity is quiesced before
                                                                 writing 1. */
        uint64_t reserved_1_62         : 62;
        uint64_t busy                  : 1;  /**< [ 63: 63](RO/H) When one, the block is busy completing reset. No access except the reading of
                                                                 this bit should occur to the block until this is clear. */
    } s;
};


/**
 * Register (RVU_PF_BAR0) npa_af_bp_test
 *
 * INTERNAL: NPA AF Backpressure Test Register
 */
union cavm_npa_af_bp_test
{
    uint64_t u;
    struct cavm_npa_af_bp_test_s
    {
        uint64_t lfsr_freq             : 12; /**< [ 11:  0](R/W) Test LFSR update frequency in coprocessor-clocks minus one. */
        uint64_t reserved_12_15        : 4;
        uint64_t bp_cfg                : 32; /**< [ 47: 16](R/W) Backpressure weight. For diagnostic use only.
                                                                 Internal:
                                                                 There are 2 backpressure configuration bits per enable, with the two bits
                                                                 defined as 0x0=100% of the time, 0x1=25% of the time, 0x2=50% of the time,
                                                                 0x3=75% of the time.
                                                                   \<47:46\> = Config 15.
                                                                   \<45:44\> = Config 14.
                                                                   \<43:42\> = Config 13.
                                                                   \<41:40\> = Config 12.
                                                                   \<39:38\> = Config 11.
                                                                   \<37:36\> = Config 10.
                                                                   \<35:34\> = Config 9.
                                                                   \<33:32\> = Config 8.
                                                                   \<31:30\> = Config 7.
                                                                   \<29:28\> = Config 6.
                                                                   \<27:26\> = Config 5.
                                                                   \<25:24\> = Config 4.
                                                                   \<23:22\> = Config 3.
                                                                   \<21:20\> = Config 2.
                                                                   \<19:18\> = Config 1.
                                                                   \<17:16\> = Config 0. */
        uint64_t enable                : 16; /**< [ 63: 48](R/W) Enable test mode. For diagnostic use only.
                                                                 Internal:
                                                                 FIXME: the following is copied from FPA; to be updated for NPA.

                                                                 Once a bit is set, random backpressure is generated
                                                                 at the corresponding point to allow for more frequent backpressure.
                                                                 \<63\> = Reserved.
                                                                 \<62\> = Reserved.
                                                                 \<61\> = Reserved.
                                                                 \<60\> = Apply backpressure to adp to csr traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<24:23\>.
                                                                 \<59\> = Apply backpressure to adp to l2s traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<23:22\>.
                                                                 \<58\> = Apply backpressure to adp to l2l traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<21:20\>.
                                                                 \<57\> = Apply backpressure to red read to csr traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<19:18\>.
                                                                 \<56\> = Apply backpressure to l2arb to csr traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<17:16\>.
                                                                 \<55\> = Apply backpressure to csr to adp traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<15:14\>.
                                                                 \<54\> = Apply backpressure to pcc to gib traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<13:12\>.
                                                                 \<53\> = Apply backpressure to adp to pfc traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<11:10\>.
                                                                 \<52\> = generate pmc to pfc backpressure. Backpressure weight controlled
                                                                 by [BP_CFG]\<9:8\>.
                                                                 \<51\> = Apply backpressure to pmc to ncbi traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<7:6\>.
                                                                 \<50\> = Apply backpressure to csr to ncbi traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<5:4\>.
                                                                 \<49\> = Apply backpressure to l2s to ncbi traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<3:2\>.
                                                                 \<48\> = Apply backpressure to l2l to ncbi traffic. Backpressure weight controlled
                                                                 by [BP_CFG]\<1:0\>. */
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_const
 *
 * NPA AF Constants Register
 * This register contains constants for software discovery.
 */
union cavm_npa_af_const
{
    uint64_t u;
    struct cavm_npa_af_const_s
    {
        uint64_t stack_page_bytes      : 8;  /**< [  7:  0](RO) NPA_STACK_PAGE_S size in bytes. */
        uint64_t stack_page_ptrs       : 8;  /**< [ 15:  8](RO) Number of pointers stored in a stack page. */
        uint64_t lfs                   : 12; /**< [ 27: 16](RO) Number of NPA Local Functions. */
        uint64_t qints                 : 12; /**< [ 39: 28](RO) Number of queue interrupts per VF/PF. */
        uint64_t num_ndc               : 3;  /**< [ 42: 40](RO) Number of NDCs enumerated by NDC_IDX_E::NPA_U(). */
        uint64_t reserved_43_63        : 21;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_const1
 *
 * NPA AF Constants Register 1
 * This register contains constants for software discovery.
 */
union cavm_npa_af_const1
{
    uint64_t u;
    struct cavm_npa_af_const1_s
    {
        uint64_t aura_log2bytes        : 4;  /**< [  3:  0](RO) Aura context size as log2(bytes). Size of each NPA_AURA_HW_S structure in a
                                                                 local function's aura context table in NDC/LLC/DRAM. See
                                                                 NPA_AF_LF()_LOC_AURAS_BASE and NPA_AF_LF()_AURAS_CFG. */
        uint64_t pool_log2bytes        : 4;  /**< [  7:  4](RO) Pool context size as log2(bytes). Size of each NPA_POOL_HW_S structure at
                                                                 IOVA NPA_AURA_S[POOL_ADDR]. */
        uint64_t qint_log2bytes        : 4;  /**< [ 11:  8](RO) Queue interrupt context size as log2(bytes). Size of each NPA_QINT_HW_S
                                                                 structure in a local function's queue interrupt context table NDC/LLC/DRAM.
                                                                 See NPA_AF_LF()_QINTS_BASE and NPA_AF_LF()_QINTS_CFG. */
        uint64_t reserved_12_63        : 52;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_dtx_filter_ctl
 *
 * NPA AF DTX LF Filter Control Register
 */
union cavm_npa_af_dtx_filter_ctl
{
    uint64_t u;
    struct cavm_npa_af_dtx_filter_ctl_s
    {
        uint64_t ena                   : 1;  /**< [  0:  0](R/W) Enable DTX pointer debug filtering based on LF.
                                                                 When combined with appropriate DTX selects, permits observation of full aura values for that LF. */
        uint64_t reserved_1_3          : 3;
        uint64_t lf                    : 7;  /**< [ 10:  4](R/W) LF value used for DTX pointer debug filtering when [ENA] is one. */
        uint64_t reserved_11_63        : 53;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_eco
 *
 * INTERNAL: NPA AF ECO Register
 */
union cavm_npa_af_eco
{
    uint64_t u;
    struct cavm_npa_af_eco_s
    {
        uint64_t eco_rw                : 32; /**< [ 31:  0](R/W) Reserved for ECO usage. */
        uint64_t reserved_32_63        : 32;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_err_int
 *
 * NPA Admin Function Error Interrupt Register
 */
union cavm_npa_af_err_int
{
    uint64_t u;
    struct cavm_npa_af_err_int_s
    {
        uint64_t reserved_0_11         : 12;
        uint64_t aq_door_err           : 1;  /**< [ 12: 12](R/W1C/H) AQ doorbell error. See NPA_AF_AQ_DOOR[COUNT]. Hardware also sets
                                                                 NPA_AF_AQ_STATUS[AQ_ERR]. */
        uint64_t aq_res_fault          : 1;  /**< [ 13: 13](R/W1C/H) Memory fault on NPA_AQ_RES_S write, or on read/write data following
                                                                 NPA_AQ_RES_S. Hardware also sets NPA_AF_AQ_STATUS[AQ_ERR]. */
        uint64_t aq_inst_fault         : 1;  /**< [ 14: 14](R/W1C/H) Memory fault on NPA_AQ_INST_S read. Hardware also sets
                                                                 NPA_AF_AQ_STATUS[AQ_ERR]. */
        uint64_t reserved_15_63        : 49;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_err_int_ena_w1c
 *
 * NPA Admin Function Error Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_npa_af_err_int_ena_w1c
{
    uint64_t u;
    struct cavm_npa_af_err_int_ena_w1c_s
    {
        uint64_t reserved_0_11         : 12;
        uint64_t aq_door_err           : 1;  /**< [ 12: 12](R/W1C/H) Reads or clears enable for NPA_AF_ERR_INT[AQ_DOOR_ERR]. */
        uint64_t aq_res_fault          : 1;  /**< [ 13: 13](R/W1C/H) Reads or clears enable for NPA_AF_ERR_INT[AQ_RES_FAULT]. */
        uint64_t aq_inst_fault         : 1;  /**< [ 14: 14](R/W1C/H) Reads or clears enable for NPA_AF_ERR_INT[AQ_INST_FAULT]. */
        uint64_t reserved_15_63        : 49;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_err_int_ena_w1s
 *
 * NPA Admin Function Error Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_npa_af_err_int_ena_w1s
{
    uint64_t u;
    struct cavm_npa_af_err_int_ena_w1s_s
    {
        uint64_t reserved_0_11         : 12;
        uint64_t aq_door_err           : 1;  /**< [ 12: 12](R/W1S/H) Reads or sets enable for NPA_AF_ERR_INT[AQ_DOOR_ERR]. */
        uint64_t aq_res_fault          : 1;  /**< [ 13: 13](R/W1S/H) Reads or sets enable for NPA_AF_ERR_INT[AQ_RES_FAULT]. */
        uint64_t aq_inst_fault         : 1;  /**< [ 14: 14](R/W1S/H) Reads or sets enable for NPA_AF_ERR_INT[AQ_INST_FAULT]. */
        uint64_t reserved_15_63        : 49;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_err_int_w1s
 *
 * NPA Admin Function Error Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_npa_af_err_int_w1s
{
    uint64_t u;
    struct cavm_npa_af_err_int_w1s_s
    {
        uint64_t reserved_0_11         : 12;
        uint64_t aq_door_err           : 1;  /**< [ 12: 12](R/W1S/H) Reads or sets NPA_AF_ERR_INT[AQ_DOOR_ERR]. */
        uint64_t aq_res_fault          : 1;  /**< [ 13: 13](R/W1S/H) Reads or sets NPA_AF_ERR_INT[AQ_RES_FAULT]. */
        uint64_t aq_inst_fault         : 1;  /**< [ 14: 14](R/W1S/H) Reads or sets NPA_AF_ERR_INT[AQ_INST_FAULT]. */
        uint64_t reserved_15_63        : 49;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_gen_cfg
 *
 * NPA AF General Configuration Register
 * This register provides NPA control and status information.
 */
union cavm_npa_af_gen_cfg
{
    uint64_t u;
    struct cavm_npa_af_gen_cfg_s
    {
        uint64_t reserved_0            : 1;
        uint64_t af_be                 : 1;  /**< [  1:  1](R/W) Admin function big-endian select. Specifies endianness of all admin queue
                                                                 instructions, results and associated structures stored in LLC/DRAM:

                                                                 0 = Little-endian. All AF software data structures are in byte invariant
                                                                 little-endian format (LE8) with the following ordering within each 64-bit
                                                                 word: \<7:0\> at byte address 0, \<15:8\> at address 1, ..., \<63:56\> at address
                                                                 0x7.

                                                                 1 = Big-endian. All AF software data structures are in byte invariant
                                                                 big-endian format (BE8) with the following ordering within each 64-bit
                                                                 word: \<63:56\> at byte address 0, \<55:48\> at address 1, ..., \<7:0\> at
                                                                 address 0x7.

                                                                 The affected data structures are:
                                                                 * NPA_AQ_INST_S.
                                                                 * NPA_AQ_RES_S.
                                                                 * Software context READ/WRITE/INIT data following NPA_AQ_RES_S. */
        uint64_t reserved_2            : 1;
        uint64_t force_cond_clk_en     : 1;  /**< [  3:  3](R/W) Force clock enables within block. For diagnostic use only. */
        uint64_t force_intf_clk_en     : 1;  /**< [  4:  4](R/W) Force clock enables on interface busses between blocks. For diagnostic use only. */
        uint64_t reserved_5_9          : 5;
        uint64_t ocla_bp               : 1;  /**< [ 10: 10](R/W) OCLA backpressure enable. When OCLA FIFOs are near full, allow OCLA to backpressure
                                                                 alloc/frees. See also [RATEM1]. */
        uint64_t reserved_11           : 1;
        uint64_t ratem1                : 4;  /**< [ 15: 12](R/W) Issue rate minus one. Limit peak alloc/frees to once per [RATEM1]+1
                                                                 clock cycles to insure all alloc/frees are visible to OCLA. */
        uint64_t reserved_16_63        : 48;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int
 *
 * NPA AF General Interrupt Register
 * This register contains general error interrupt summary bits.
 */
union cavm_npa_af_gen_int
{
    uint64_t u;
    struct cavm_npa_af_gen_int_s
    {
        uint64_t free_dis              : 16; /**< [ 15:  0](R/W1C/H) Free input disabled. A FREE request was dropped because the
                                                                 corresponding NPA_AF_INP_CTL[FREE_DIS] bit was set.
                                                                 Each bit corresponds to a hardware free input queue from a coprocessor. Bit
                                                                 indices are enumerated by NPA_INPQ_E. */
        uint64_t alloc_dis             : 16; /**< [ 31: 16](R/W1C/H) Allocation input disabled. An ALLOC request was dropped because the
                                                                 corresponding NPA_AF_INP_CTL[ALLOC_DIS] bit was set.
                                                                 Each bit corresponds to a hardware allocation input queue from a
                                                                 coprocessor. Bit indices are enumerated by NPA_INPQ_E. */
        uint64_t unmapped_pf_func      : 1;  /**< [ 32: 32](R/W1C/H) Unmapped PF FUNC. Coprocessor ALLOC failed or FREE dropped due to that
                                                                 coprocessor requesting with a PF FUNC that is not mapped to an NPA LF. */
        uint64_t reserved_33_63        : 31;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int_ena_w1c
 *
 * NPA AF General Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_npa_af_gen_int_ena_w1c
{
    uint64_t u;
    struct cavm_npa_af_gen_int_ena_w1c_s
    {
        uint64_t free_dis              : 16; /**< [ 15:  0](R/W1C/H) Reads or clears enable for NPA_AF_GEN_INT[FREE_DIS]. */
        uint64_t alloc_dis             : 16; /**< [ 31: 16](R/W1C/H) Reads or clears enable for NPA_AF_GEN_INT[ALLOC_DIS]. */
        uint64_t unmapped_pf_func      : 1;  /**< [ 32: 32](R/W1C/H) Reads or clears enable for NPA_AF_GEN_INT[UNMAPPED_PF_FUNC]. */
        uint64_t reserved_33_63        : 31;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int_ena_w1s
 *
 * NPA AF General Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_npa_af_gen_int_ena_w1s
{
    uint64_t u;
    struct cavm_npa_af_gen_int_ena_w1s_s
    {
        uint64_t free_dis              : 16; /**< [ 15:  0](R/W1S/H) Reads or sets enable for NPA_AF_GEN_INT[FREE_DIS]. */
        uint64_t alloc_dis             : 16; /**< [ 31: 16](R/W1S/H) Reads or sets enable for NPA_AF_GEN_INT[ALLOC_DIS]. */
        uint64_t unmapped_pf_func      : 1;  /**< [ 32: 32](R/W1S/H) Reads or sets enable for NPA_AF_GEN_INT[UNMAPPED_PF_FUNC]. */
        uint64_t reserved_33_63        : 31;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_gen_int_w1s
 *
 * NPA AF General Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_npa_af_gen_int_w1s
{
    uint64_t u;
    struct cavm_npa_af_gen_int_w1s_s
    {
        uint64_t free_dis              : 16; /**< [ 15:  0](R/W1S/H) Reads or sets NPA_AF_GEN_INT[FREE_DIS]. */
        uint64_t alloc_dis             : 16; /**< [ 31: 16](R/W1S/H) Reads or sets NPA_AF_GEN_INT[ALLOC_DIS]. */
        uint64_t unmapped_pf_func      : 1;  /**< [ 32: 32](R/W1S/H) Reads or sets NPA_AF_GEN_INT[UNMAPPED_PF_FUNC]. */
        uint64_t reserved_33_63        : 31;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_inp_ctl
 *
 * NPA AF Input Control Register
 */
union cavm_npa_af_inp_ctl
{
    uint64_t u;
    struct cavm_npa_af_inp_ctl_s
    {
        uint64_t free_dis              : 16; /**< [ 15:  0](R/W) Free input disable.
                                                                 Each bit corresponds to a hardware allocation input queue from a coprocessor. Bit indices
                                                                 are enumerated by NPA_INPQ_E.
                                                                 If set, a FREE request to the queue is dropped and NPA_AF_GEN_INT[FREE_DIS] is set.

                                                                 Internal:
                                                                 Once the grant is sent, the request is marked and it is dropped when the request
                                                                 data is received. */
        uint64_t alloc_dis             : 16; /**< [ 31: 16](R/W) Allocation input disable.
                                                                 Each bit corresponds to a hardware allocation input queue from a coprocessor. Bit indices
                                                                 are enumerated by NPA_INPQ_E.
                                                                 If set, an ALLOC request to the queue is dropped and NPA_AF_GEN_INT[ALLOC_DIS] is set. */
        uint64_t reserved_32_63        : 32;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_auras_cfg
 *
 * NPA AF Local Function Auras Configuration Registers
 */
union cavm_npa_af_lfx_auras_cfg
{
    uint64_t u;
    struct cavm_npa_af_lfx_auras_cfg_s
    {
        uint64_t way_mask              : 16; /**< [ 15:  0](R/W) Way partitioning mask for allocating associated NPA_AURA_HW_S in NDC (1
                                                                 means do not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
        uint64_t loc_aura_size         : 4;  /**< [ 19: 16](R/W) Local aura size. Specifies number of auras managed by this NPA as follows:
                                                                 0x0 = 0 auras.
                                                                 0x1 = 128 auras.
                                                                 0x2 = 256 auras.
                                                                 0x3 = 512 auras.
                                                                 0x4 = 1K auras.
                                                                 0x5 = 2K auras.
                                                                 0x6 = 4K auras.
                                                                 0x7 = 8K auras.
                                                                 0x8 = 16K auras.
                                                                 0x9 = 32K auras.
                                                                 0xA = 64K auras.
                                                                 0xB = 128K auras
                                                                 0xC = 256K auras.
                                                                 0xD = 512K auras.
                                                                 0xE = 1M auras.
                                                                 0xF = Reserved.

                                                                 See [LOC_AURA_OFFSET]. */
        uint64_t loc_aura_offset       : 14; /**< [ 33: 20](R/W) Local aura offset. Minimum aura number managed by this NPA divided by 64
                                                                 when [LOC_AURA_SIZE] is nonzero.

                                                                 When [LOC_AURA_SIZE] is zero, there are no local auras for this LF.
                                                                 Otherwise, the range of local aura numbers managed by this NPA is loc_start
                                                                 through loc_limit-1, inclusive, where:
                                                                 _ loc_start = [LOC_AURA_OFFSET]*64
                                                                 _ loc_limit = [LOC_AURA_OFFSET]*64 + (1 \<\< ([LOC_AURA_SIZE] + 6))

                                                                 See also NPA_AF_LF()_LOC_AURAS_BASE.

                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Likewise, when [RMT_AURA_SIZE] is zero, there are no remote auras for this
                                                                 LF. Otherwise, the range of remote aura numbers managed by the NPA on
                                                                 the remote node is rmt_start through rmt_limit-1, inclusive, where:
                                                                 _ rmt_start = [RMT_AURA_OFFSET]*64
                                                                 _ rmt_limit = [RMT_AURA_OFFSET]*64 + (1 \<\< ([RMT_AURA_SIZE] + 6))

                                                                 The local and remote aura ranges must not overlap. */
        uint64_t caching               : 1;  /**< [ 34: 34](R/W) Selects the style read for accessing NPA_AURA_HW_S in LLC/DRAM:
                                                                 0x0 = NPA_AURA_HW_S reads will not allocate into the LLC.
                                                                 0x1 = NPA_AURA_HW_S reads are allocated into the LLC.

                                                                 NPA_AURA_HW_S writes that are not allocated in NDC will always allocate
                                                                 into LLC. */
        uint64_t reserved_35           : 1;
        uint64_t rmt_aura_size         : 4;  /**< [ 39: 36](R/W) Reserved. Must be zero.
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Specifies number of auras managed by the NPA on the remote node. See
                                                                 [LOC_AURA_SIZE] and [LOC_AURA_OFFSET]. */
        uint64_t rmt_aura_offset       : 14; /**< [ 53: 40](R/W) Reserved.
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Minimum aura number managed by the NPA on the remote node divided by 64.
                                                                 See [LOC_AURA_OFFSET]. */
        uint64_t rmt_lf                : 7;  /**< [ 60: 54](R/W) Reserved.
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Remote local function. NPA LF on the remote node that manages the aura
                                                                 range specified by [RMT_AURA_OFFSET] and [RMT_AURA_SIZE]. See
                                                                 [LOC_AURA_OFFSET]. */
        uint64_t reserved_61_63        : 3;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_loc_auras_base
 *
 * NPA AF Local Function Auras Base Registers
 */
union cavm_npa_af_lfx_loc_auras_base
{
    uint64_t u;
    struct cavm_npa_af_lfx_loc_auras_base_s
    {
        uint64_t reserved_0_6          : 7;
        uint64_t addr                  : 46; /**< [ 52:  7](R/W) AF IOVA\<52:7\> of local aura context table in NDC/LLC/DRAM.
                                                                 The table consists of 1 \<\< (NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE] + 6)
                                                                 contiguous NPA_AURA_HW_S structures, where the size of each structure is
                                                                 1 \<\< NPA_AF_CONST1[AURA_LOG2BYTES] bytes.
                                                                 Aura number NPA_AF_LF()_AURAS_CFG[LOC_AURA_OFFSET]*64 stored at index 0 of
                                                                 the table. */
        uint64_t reserved_53_63        : 11;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_qints_base
 *
 * NPA AF Local Function Queue Interrupts Base Registers
 */
union cavm_npa_af_lfx_qints_base
{
    uint64_t u;
    struct cavm_npa_af_lfx_qints_base_s
    {
        uint64_t reserved_0_6          : 7;
        uint64_t addr                  : 46; /**< [ 52:  7](R/W) AF IOVA\<52:7\> of local queue interrupt context table in LLC/DRAM.
                                                                 The table consists of NPA_AF_CONST[QINTS] contiguous NPA_QINT_HW_S structures.
                                                                 The size of each structure is 1 \<\< NPA_AF_CONST1[QINT_LOG2BYTES] bytes. */
        uint64_t reserved_53_63        : 11;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_lf#_qints_cfg
 *
 * NPA AF Local Function Queue Interrupts Configuration Registers
 * This register controls access to the LF's queue interrupt context table in
 * LLC/DRAM. The table consists of NPA_AF_CONST[QINTS] contiguous NPA_QINT_HW_S
 * structures. The size of each structure is 1 \<\< NPA_AF_CONST1[QINT_LOG2BYTES]
 * bytes.
 */
union cavm_npa_af_lfx_qints_cfg
{
    uint64_t u;
    struct cavm_npa_af_lfx_qints_cfg_s
    {
        uint64_t reserved_0_19         : 20;
        uint64_t way_mask              : 16; /**< [ 35: 20](R/W) Way partitioning mask for allocating context structures in NDC (1 means do
                                                                 not use). All ones disables allocation in NDC.

                                                                 Internal:
                                                                 Bypass NDC when all ones. */
        uint64_t caching               : 2;  /**< [ 37: 36](R/W) Selects the style read for accessing NPA_QINT_HW_S in LLC/DRAM:
                                                                 0x0 = NPA_QINT_HW_S reads will not allocate into the LLC.
                                                                 0x1 = NPA_QINT_HW_S reads are allocated into the LLC.
                                                                 0x2 = Reserved.
                                                                 0x3 = Reserved.

                                                                 NPA_QINT_HW_S writes that are not allocated in NDC will always allocate
                                                                 into LLC. */
        uint64_t reserved_38_63        : 26;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_lf_rst
 *
 * NPA Admin Function LF Reset Register
 */
union cavm_npa_af_lf_rst
{
    uint64_t u;
    struct cavm_npa_af_lf_rst_s
    {
        uint64_t lf                    : 8;  /**< [  7:  0](R/W) Local function that is reset when [EXEC] is set. */
        uint64_t reserved_8_11         : 4;
        uint64_t exec                  : 1;  /**< [ 12: 12](R/W1S/H) Execute LF software-initiated reset. When software writes a one to set this bit, hardware
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
        uint64_t reserved_13_63        : 51;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_ndc_cfg
 *
 * NDC AF General Configuration Register
 * This register provides NDC control.
 */
union cavm_npa_af_ndc_cfg
{
    uint64_t u;
    struct cavm_npa_af_ndc_cfg_s
    {
        uint64_t ndc_bypass            : 1;  /**< [  0:  0](R/W) Forces all NDC transations to bypass the NDC cache. */
        uint64_t ndc_ign_pois          : 1;  /**< [  1:  1](R/W) Ignore poison responses from NDC. */
        uint64_t byp_aura              : 1;  /**< [  2:  2](R/W) Force all NPA_AURA_HW_S transactions to bypass NDC. */
        uint64_t byp_pool              : 1;  /**< [  3:  3](R/W) Force all NPA_POOL_HW_S transactions to bypass NDC. */
        uint64_t byp_stack             : 1;  /**< [  4:  4](R/W) Force all NPA_STACK_PAGE_S transactions to bypass NDC. */
        uint64_t byp_qint              : 1;  /**< [  5:  5](R/W) Force all NPA_QINT_HW_S transactions to bypass NDC. */
        uint64_t reserved_6_63         : 58;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_ndc_sync
 *
 * NPA AF NDC Sync Register
 * Used to synchronize the NPA NDC.
 */
union cavm_npa_af_ndc_sync
{
    uint64_t u;
    struct cavm_npa_af_ndc_sync_s
    {
        uint64_t lf                    : 8;  /**< [  7:  0](R/W) Local function whose data is synced when [EXEC] is set. */
        uint64_t reserved_8_11         : 4;
        uint64_t exec                  : 1;  /**< [ 12: 12](R/W1S/H) Execute sync. When software writes a one to set this bit, NDC writes back
                                                                 to LLC/DRAM all dirty lines associated with the local function selected by
                                                                 [LF]. Hardware clears this bit when done. */
        uint64_t reserved_13_63        : 51;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_ras
 *
 * NPA AF RAS Interrupt Register
 * This register is intended for delivery of RAS events to the SCP, so should be
 * ignored by OS drivers.
 */
union cavm_npa_af_ras
{
    uint64_t u;
    struct cavm_npa_af_ras_s
    {
        uint64_t reserved_0_31         : 32;
        uint64_t aq_ctx_poison         : 1;  /**< [ 32: 32](R/W1C/H) Poisoned data returned on read of hardware context data selected by
                                                                 NPA_AQ_INST_S[LF,CTYPE,AURA]. If NPA_AF_NDC_CFG[NDC_IGN_POIS]=0, hardware
                                                                 also returns NPA_AQ_RES_S[COMPCODE] = NPA_AQ_COMP_E::CTX_POISON. */
        uint64_t aq_res_poison         : 1;  /**< [ 33: 33](R/W1C/H) Poisoned read data returned following NPA_AQ_RES_S. If
                                                                 NPA_AF_NDC_CFG[NDC_IGN_POIS]=0, hardware also sets
                                                                 NPA_AF_AQ_STATUS[AQ_ERR]. */
        uint64_t aq_inst_poison        : 1;  /**< [ 34: 34](R/W1C/H) Poisoned data returned on NPA_AQ_INST_S read. If
                                                                 NPA_AF_NDC_CFG[NDC_IGN_POIS]=0, hardware also sets
                                                                 NPA_AF_AQ_STATUS[AQ_ERR]. */
        uint64_t reserved_35_63        : 29;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_ras_ena_w1c
 *
 * NPA AF RAS Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_npa_af_ras_ena_w1c
{
    uint64_t u;
    struct cavm_npa_af_ras_ena_w1c_s
    {
        uint64_t reserved_0_31         : 32;
        uint64_t aq_ctx_poison         : 1;  /**< [ 32: 32](R/W1C/H) Reads or clears enable for NPA_AF_RAS[AQ_CTX_POISON]. */
        uint64_t aq_res_poison         : 1;  /**< [ 33: 33](R/W1C/H) Reads or clears enable for NPA_AF_RAS[AQ_RES_POISON]. */
        uint64_t aq_inst_poison        : 1;  /**< [ 34: 34](R/W1C/H) Reads or clears enable for NPA_AF_RAS[AQ_INST_POISON]. */
        uint64_t reserved_35_63        : 29;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_ras_ena_w1s
 *
 * NPA AF RAS Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_npa_af_ras_ena_w1s
{
    uint64_t u;
    struct cavm_npa_af_ras_ena_w1s_s
    {
        uint64_t reserved_0_31         : 32;
        uint64_t aq_ctx_poison         : 1;  /**< [ 32: 32](R/W1S/H) Reads or sets enable for NPA_AF_RAS[AQ_CTX_POISON]. */
        uint64_t aq_res_poison         : 1;  /**< [ 33: 33](R/W1S/H) Reads or sets enable for NPA_AF_RAS[AQ_RES_POISON]. */
        uint64_t aq_inst_poison        : 1;  /**< [ 34: 34](R/W1S/H) Reads or sets enable for NPA_AF_RAS[AQ_INST_POISON]. */
        uint64_t reserved_35_63        : 29;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_ras_w1s
 *
 * NPA AF RAS Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_npa_af_ras_w1s
{
    uint64_t u;
    struct cavm_npa_af_ras_w1s_s
    {
        uint64_t reserved_0_31         : 32;
        uint64_t aq_ctx_poison         : 1;  /**< [ 32: 32](R/W1S/H) Reads or sets NPA_AF_RAS[AQ_CTX_POISON]. */
        uint64_t aq_res_poison         : 1;  /**< [ 33: 33](R/W1S/H) Reads or sets NPA_AF_RAS[AQ_RES_POISON]. */
        uint64_t aq_inst_poison        : 1;  /**< [ 34: 34](R/W1S/H) Reads or sets NPA_AF_RAS[AQ_INST_POISON]. */
        uint64_t reserved_35_63        : 29;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int
 *
 * NPA AF RVU Interrupt Register
 * This register contains RVU error interrupt summary bits.
 */
union cavm_npa_af_rvu_int
{
    uint64_t u;
    struct cavm_npa_af_rvu_int_s
    {
        uint64_t unmapped_slot         : 1;  /**< [  0:  0](R/W1C/H) Unmapped slot. Received an IO request to a VF/PF slot in BAR2 that is not
                                                                 reverse mapped to an LF. See NPA_PRIV_LF()_CFG and NPA_AF_RVU_LF_CFG_DEBUG. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int_ena_w1c
 *
 * NPA AF RVU Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_npa_af_rvu_int_ena_w1c
{
    uint64_t u;
    struct cavm_npa_af_rvu_int_ena_w1c_s
    {
        uint64_t unmapped_slot         : 1;  /**< [  0:  0](R/W1C/H) Reads or clears enable for NPA_AF_RVU_INT[UNMAPPED_SLOT]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int_ena_w1s
 *
 * NPA AF RVU Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_npa_af_rvu_int_ena_w1s
{
    uint64_t u;
    struct cavm_npa_af_rvu_int_ena_w1s_s
    {
        uint64_t unmapped_slot         : 1;  /**< [  0:  0](R/W1S/H) Reads or sets enable for NPA_AF_RVU_INT[UNMAPPED_SLOT]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_int_w1s
 *
 * NPA AF RVU Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_npa_af_rvu_int_w1s
{
    uint64_t u;
    struct cavm_npa_af_rvu_int_w1s_s
    {
        uint64_t unmapped_slot         : 1;  /**< [  0:  0](R/W1S/H) Reads or sets NPA_AF_RVU_INT[UNMAPPED_SLOT]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_af_rvu_lf_cfg_debug
 *
 * NPA Privileged LF Configuration Debug Register
 * This debug register allows software to lookup the reverse mapping from VF/PF
 * slot to LF. The forward mapping is programmed with NPA_PRIV_LF()_CFG.
 */
union cavm_npa_af_rvu_lf_cfg_debug
{
    uint64_t u;
    struct cavm_npa_af_rvu_lf_cfg_debug_s
    {
        uint64_t lf                    : 12; /**< [ 11:  0](RO/H) When [LF_VALID] is set, local function provisioned to the VF/PF slot. */
        uint64_t lf_valid              : 1;  /**< [ 12: 12](RO/H) When set, indicates local function [LF] is provisioned to the VF/PF slot
                                                                 indexed by this register. When clear, a local function is not provisioned
                                                                 to the VF/PF slot. */
        uint64_t exec                  : 1;  /**< [ 13: 13](R/W1S/H) Execute lookup. Writing a one to this bits initiates the reverse lookup
                                                                 from {[PF_FUNC], [SLOT]}. Hardware writes the lookup result to {[LF_VALID],
                                                                 [LF]} and clears this bit when done. */
        uint64_t reserved_14_15        : 2;
        uint64_t slot                  : 8;  /**< [ 23: 16](R/W) Slot within the VF/PF selected by [PF_FUNC] for reverse lookup. Must be
                                                                 zero for NIX and NPA. */
        uint64_t pf_func               : 16; /**< [ 39: 24](R/W) RVU VF/PF for reverse lookup. Format defined by RVU_PF_FUNC_S. */
        uint64_t reserved_40_63        : 24;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_alloc#
 *
 * NPA Aura Allocate Operation Registers
 * These registers are used to allocate one or two pointers from a given aura's pool.
 * A 64-bit atomic load-and-add to NPA_LF_AURA_OP_ALLOC(0) allocates a single pointer.
 * A 128-bit atomic CASP operation to NPA_LF_AURA_OP_ALLOC(0..1) allocates two pointers.
 * The atomic write data format is NPA_AURA_OP_WDATA_S.
 * For CASP, the first SWAP word in the write data contains NPA_AURA_OP_WDATA_S
 * and the remaining write data words are ignored.
 *
 * All other accesses to this register (e.g. reads and writes) are RAZ/WI.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_aura_op_allocx
{
    uint64_t u;
    struct cavm_npa_lf_aura_op_allocx_s
    {
        uint64_t addr                  : 64; /**< [ 63:  0](RO/H) LF IOVA newly allocated by hardware returned as atomic read data. Bits \<6:0\>
                                                                 are always zero. Bits \<63:53\> are a sign extension of bit \<52\>.

                                                                 If 0x0, the selected pool is empty, aura limit has been hit, or drop
                                                                 (NPA_AURA_OP_WDATA_S[DROP]) was applied. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_cnt
 *
 * NPA LF Aura Count Register
 * A 64-bit atomic load-and-add to this register returns a given aura's
 * count. A write sets or adds the aura's count. A read is RAZ.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_aura_op_cnt
{
    uint64_t u;
    struct cavm_npa_lf_aura_op_cnt_s
    {
        uint64_t count                 : 36; /**< [ 35:  0](R/W/H) Returns the current value of NPA_AURA_HW_S[COUNT] on a load-and-add. Value
                                                                 to write or add to NPA_AURA_HW_S[COUNT] on a write; see [CNT_ADD]. */
        uint64_t reserved_36_41        : 6;
        uint64_t op_err                : 1;  /**< [ 42: 42](RO/H) Operation error. Remaining read data fields are not valid. One of the
                                                                 following (may not be exhaustive):
                                                                 * Memory fault on NPA_AURA_HW_S read; also sets NPA_LF_ERR_INT[AURA_FAULT].
                                                                 * Poisoned data returned on NPA_AURA_HW_S read; also sets
                                                                 NPA_LF_RAS[AURA_POISON].
                                                                 * [AURA] is outside of the range specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET].
                                                                 * Aura is disabled (NPA_AURA_HW_S[ENA] = 0); also sets
                                                                 NPA_LF_ERR_INT[AURA_DIS]. */
        uint64_t cnt_add               : 1;  /**< [ 43: 43](WO) Count add. This field is present on a write. When clear, [COUNT] is
                                                                 unsigned and written to NPA_AURA_HW_S[COUNT]. When set, [COUNT] is a signed
                                                                 value added to NPA_AURA_HW_S[COUNT]. */
        uint64_t aura                  : 20; /**< [ 63: 44](WO) Aura within VF. This field is present on a write, or in the write data of
                                                                 an atomic load-and-add. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_free0
 *
 * NPA LF Aura Free Operation Register 0
 * A 128-bit write to the NPA_LF_AURA_OP_FREE0 and NPA_LF_AURA_OP_FREE1
 * registers frees a pointer into a given aura's pool.
 * All other accesses to these registers (e.g. reads and 64-bit writes) are
 * RAZ/WI.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_aura_op_free0
{
    uint64_t u;
    struct cavm_npa_lf_aura_op_free0_s
    {
        uint64_t addr                  : 64; /**< [ 63:  0](WO) LF IOVA to return to aura. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_free1
 *
 * NPA LF Aura Free Operation Register 1
 * See NPA_LF_AURA_OP_FREE0.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_aura_op_free1
{
    uint64_t u;
    struct cavm_npa_lf_aura_op_free1_s
    {
        uint64_t aura                  : 20; /**< [ 19:  0](WO) Aura within VF. */
        uint64_t reserved_20_62        : 43;
        uint64_t fabs                  : 1;  /**< [ 63: 63](WO) Free absolute. If set, the pointer is absolute and is pushed to the pool exactly
                                                                 as provided. If clear, the freed pointer is adjusted based on
                                                                 NPA_POOL_S[NAT_ALIGN], NPA_POOL_S[BUF_SIZE]. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_int
 *
 * NPA LF Aura Interrupt Operation Register
 * A 64-bit atomic load-and-add to this register reads
 * NPA_AURA_HW_S[ERR_INT,ERR_INT_ENA,THRESH_INT,THRESH_INT_ENA]. A write
 * optionally sets or clears these fields. A read is RAZ.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_aura_op_int
{
    uint64_t u;
    struct cavm_npa_lf_aura_op_int_s
    {
        uint64_t err_int               : 8;  /**< [  7:  0](R/W/H) Returns NPA_AURA_HW_S[ERR_INT] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[ERR_INT] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t err_int_ena           : 8;  /**< [ 15:  8](R/W/H) Returns NPA_AURA_HW_S[ERR_INT_ENA] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[ERR_INT_ENA] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t thresh_int            : 1;  /**< [ 16: 16](R/W/H) Returns NPA_AURA_HW_S[THRESH_INT] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[THRESH_INT] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t thresh_int_ena        : 1;  /**< [ 17: 17](R/W/H) Returns NPA_AURA_HW_S[THRESH_INT_ENA] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[THRESH_INT_ENA] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t reserved_18_41        : 24;
        uint64_t op_err                : 1;  /**< [ 42: 42](RO/H) Operation error. See NPA_LF_AURA_OP_CNT[OP_ERR]. */
        uint64_t setop                 : 1;  /**< [ 43: 43](WO) Set operation. Valid on a write. Indicates write-one-to-set when set,
                                                                 write-one-to-clear otherwise. */
        uint64_t aura                  : 20; /**< [ 63: 44](WO) Aura within VF. This field is present on a write, or in the write data of
                                                                 an atomic load-and-add. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_limit
 *
 * NPA LF Aura Allocation Limit Register
 * A 64-bit atomic load-and-add to this register returns a given aura's
 * limit. A write sets the aura's limit. A read is RAZ.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_aura_op_limit
{
    uint64_t u;
    struct cavm_npa_lf_aura_op_limit_s
    {
        uint64_t limit                 : 36; /**< [ 35:  0](R/W/H) Value written to or read from NPA_AURA_HW_S[LIMIT]. */
        uint64_t reserved_36_41        : 6;
        uint64_t op_err                : 1;  /**< [ 42: 42](RO/H) Operation error. See NPA_LF_AURA_OP_CNT[OP_ERR]. */
        uint64_t reserved_43           : 1;
        uint64_t aura                  : 20; /**< [ 63: 44](WO) Aura within VF. This field is present on a write, or in the write data of
                                                                 an atomic load-and-add. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_aura_op_thresh
 *
 * NPA LF Aura Threshold Operation Register
 * A 64-bit atomic load-and-add to this register reads
 * NPA_AURA_HW_S[THRESH_UP,THRESH]. A write to the register writes
 * NPA_AURA_HW_S[THRESH_UP,THRESH] and recomputes NPA_AURA_HW_S[THRESH_INT].
 * A read is RAZ.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_aura_op_thresh
{
    uint64_t u;
    struct cavm_npa_lf_aura_op_thresh_s
    {
        uint64_t thresh                : 36; /**< [ 35:  0](R/W/H) Returns NPA_AURA_HW_S[THRESH] on an atomic load-and-add. Value written to
                                                                 NPA_AURA_HW_S[THRESH] on a write. */
        uint64_t reserved_36_41        : 6;
        uint64_t op_err                : 1;  /**< [ 42: 42](RO/H) Operation error. See NPA_LF_AURA_OP_CNT[OP_ERR]. */
        uint64_t thresh_up             : 1;  /**< [ 43: 43](R/W/H) Returns NPA_AURA_HW_S[THRESH_UP] on an atomic load-and-add. Value written
                                                                 to NPA_AURA_HW_S[THRESH_UP] on a write. */
        uint64_t aura                  : 20; /**< [ 63: 44](WO) Aura within VF. This field is present on a write, or in the write data of
                                                                 an atomic load-and-add. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int
 *
 * NPA LF Error Interrupt Register
 */
union cavm_npa_lf_err_int
{
    uint64_t u;
    struct cavm_npa_lf_err_int_s
   {
        uint64_t aura_dis              : 1;  /**< [  0:  0](R/W1C/H) Aura disabled. Coprocessor allocate/return or
                                                                 NPA_LF_AURA_OP_* /NPA_LF_POOL_OP_* access dropped due to disabled aura
                                                                 (NPA_AURA_HW_S[ENA] is clear). */
        uint64_t aura_oor              : 1;  /**< [  1:  1](R/W1C/H) Aura out of range. Coprocessor ALLOC or FREE or
                                                                 NPA_LF_AURA_OP_* /NPA_LF_POOL_OP_* access dropped because the aura number
                                                                 was outside of the range specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET].

                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Aura out of range. Coprocessor ALLOC or FREE or
                                                                 NPA_LF_AURA_OP_* /NPA_LF_POOL_OP_* access dropped because the aura number
                                                                 was outside of the local and remote ranges specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET,RMT_AURA_SIZE,RMT_AURA_OFFSET]. */
        uint64_t reserved_2            : 1;
        uint64_t rmt_req_oor           : 1;  /**< [  3:  3](R/W1C/H) Reserved.
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Remote request out of range. ALLOC or FREE from the remote node was dropped
                                                                 because the aura number was outside of the local range specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET]. */
        uint64_t reserved_4_11         : 8;
        uint64_t aura_fault            : 1;  /**< [ 12: 12](R/W1C/H) Memory fault on NPA_AURA_HW_S read or write, or on write to LF IOVA
                                                                 specified by NPA_AURA_S[FC_ADDR]. */
        uint64_t pool_fault            : 1;  /**< [ 13: 13](R/W1C/H) Memory fault on NPA_POOL_HW_S read or write, or on write to LF IOVA
                                                                 specified by NPA_POOL_S[FC_ADDR]. */
        uint64_t stack_fault           : 1;  /**< [ 14: 14](R/W1C/H) Memory fault on NPA_STACK_PAGE_S read or write. */
        uint64_t qint_fault            : 1;  /**< [ 15: 15](R/W1C/H) Memory fault on NPA_QINT_HW_S read or write. */
        uint64_t reserved_16_63        : 48;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int_ena_w1c
 *
 * NPA LF Error Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_npa_lf_err_int_ena_w1c
{
    uint64_t u;
    struct cavm_npa_lf_err_int_ena_w1c_s
    {
        uint64_t aura_dis              : 1;  /**< [  0:  0](R/W1C/H) Reads or clears enable for NPA_LF_ERR_INT[AURA_DIS]. */
        uint64_t aura_oor              : 1;  /**< [  1:  1](R/W1C/H) Reads or clears enable for NPA_LF_ERR_INT[AURA_OOR].
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Aura out of range. Coprocessor ALLOC or FREE or
                                                                 NPA_LF_AURA_OP_* /NPA_LF_POOL_OP_* access dropped because the aura number
                                                                 was outside of the local and remote ranges specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET,RMT_AURA_SIZE,RMT_AURA_OFFSET]. */
        uint64_t reserved_2            : 1;
        uint64_t rmt_req_oor           : 1;  /**< [  3:  3](R/W1C/H) Reads or clears enable for NPA_LF_ERR_INT[RMT_REQ_OOR].
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Remote request out of range. ALLOC or FREE from the remote node was dropped
                                                                 because the aura number was outside of the local range specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET]. */
        uint64_t reserved_4_11         : 8;
        uint64_t aura_fault            : 1;  /**< [ 12: 12](R/W1C/H) Reads or clears enable for NPA_LF_ERR_INT[AURA_FAULT]. */
        uint64_t pool_fault            : 1;  /**< [ 13: 13](R/W1C/H) Reads or clears enable for NPA_LF_ERR_INT[POOL_FAULT]. */
        uint64_t stack_fault           : 1;  /**< [ 14: 14](R/W1C/H) Reads or clears enable for NPA_LF_ERR_INT[STACK_FAULT]. */
        uint64_t qint_fault            : 1;  /**< [ 15: 15](R/W1C/H) Reads or clears enable for NPA_LF_ERR_INT[QINT_FAULT]. */
        uint64_t reserved_16_63        : 48;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int_ena_w1s
 *
 * NPA LF Error Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_npa_lf_err_int_ena_w1s
{
    uint64_t u;
    struct cavm_npa_lf_err_int_ena_w1s_s
    {
        uint64_t aura_dis              : 1;  /**< [  0:  0](R/W1S/H) Reads or sets enable for NPA_LF_ERR_INT[AURA_DIS]. */
        uint64_t aura_oor              : 1;  /**< [  1:  1](R/W1S/H) Reads or sets enable for NPA_LF_ERR_INT[AURA_OOR].
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Aura out of range. Coprocessor ALLOC or FREE or
                                                                 NPA_LF_AURA_OP_* /NPA_LF_POOL_OP_* access dropped because the aura number
                                                                 was outside of the local and remote ranges specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET,RMT_AURA_SIZE,RMT_AURA_OFFSET]. */
        uint64_t reserved_2            : 1;
        uint64_t rmt_req_oor           : 1;  /**< [  3:  3](R/W1S/H) Reads or sets enable for NPA_LF_ERR_INT[RMT_REQ_OOR].
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Remote request out of range. ALLOC or FREE from the remote node was dropped
                                                                 because the aura number was outside of the local range specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET]. */
        uint64_t reserved_4_11         : 8;
        uint64_t aura_fault            : 1;  /**< [ 12: 12](R/W1S/H) Reads or sets enable for NPA_LF_ERR_INT[AURA_FAULT]. */
        uint64_t pool_fault            : 1;  /**< [ 13: 13](R/W1S/H) Reads or sets enable for NPA_LF_ERR_INT[POOL_FAULT]. */
        uint64_t stack_fault           : 1;  /**< [ 14: 14](R/W1S/H) Reads or sets enable for NPA_LF_ERR_INT[STACK_FAULT]. */
        uint64_t qint_fault            : 1;  /**< [ 15: 15](R/W1S/H) Reads or sets enable for NPA_LF_ERR_INT[QINT_FAULT]. */
        uint64_t reserved_16_63        : 48;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_err_int_w1s
 *
 * NPA LF Error Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_npa_lf_err_int_w1s
{
    uint64_t u;
    struct cavm_npa_lf_err_int_w1s_s
    {
        uint64_t aura_dis              : 1;  /**< [  0:  0](R/W1S/H) Reads or sets NPA_LF_ERR_INT[AURA_DIS]. */
        uint64_t aura_oor              : 1;  /**< [  1:  1](R/W1S/H) Reads or sets NPA_LF_ERR_INT[AURA_OOR].
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Aura out of range. Coprocessor ALLOC or FREE or
                                                                 NPA_LF_AURA_OP_* /NPA_LF_POOL_OP_* access dropped because the aura number
                                                                 was outside of the local and remote ranges specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET,RMT_AURA_SIZE,RMT_AURA_OFFSET]. */
        uint64_t reserved_2            : 1;
        uint64_t rmt_req_oor           : 1;  /**< [  3:  3](R/W1S/H) Reads or sets NPA_LF_ERR_INT[RMT_REQ_OOR].
                                                                 Internal:
                                                                 For dual-node support in future products:
                                                                 Remote request out of range. ALLOC or FREE from the remote node was dropped
                                                                 because the aura number was outside of the local range specified by
                                                                 NPA_AF_LF()_AURAS_CFG[LOC_AURA_SIZE,LOC_AURA_OFFSET]. */
        uint64_t reserved_4_11         : 8;
        uint64_t aura_fault            : 1;  /**< [ 12: 12](R/W1S/H) Reads or sets NPA_LF_ERR_INT[AURA_FAULT]. */
        uint64_t pool_fault            : 1;  /**< [ 13: 13](R/W1S/H) Reads or sets NPA_LF_ERR_INT[POOL_FAULT]. */
        uint64_t stack_fault           : 1;  /**< [ 14: 14](R/W1S/H) Reads or sets NPA_LF_ERR_INT[STACK_FAULT]. */
        uint64_t qint_fault            : 1;  /**< [ 15: 15](R/W1S/H) Reads or sets NPA_LF_ERR_INT[QINT_FAULT]. */
        uint64_t reserved_16_63        : 48;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_available
 *
 * NPA LF Pool Available Count Operation Register
 * A 64-bit atomic load-and-add to this register returns a given pool's free
 * pointer count. Reads and writes are RAZ/WI.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_available
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_available_s
    {
        uint64_t count                 : 36; /**< [ 35:  0](RO/H) Free pointer count. See NPA_POOL_S[STACK_PAGES]. */
        uint64_t reserved_36_41        : 6;
        uint64_t op_err                : 1;  /**< [ 42: 42](RO/H) Operation error. Includes all conditions listed in
                                                                 NPA_LF_AURA_OP_CNT[OP_ERR] in addition to the following:
                                                                 * Memory fault on NPA_POOL_HW_S read; also sets NPA_LF_ERR_INT[POOL_FAULT].
                                                                 * Poisoned data returned on NPA_POOL_HW_S read; also sets
                                                                 NPA_LF_RAS[POOL_POISON].
                                                                 * Pool is disabled (NPA_POOL_HW_S[ENA] = 0); also sets
                                                                 NPA_AURA_HW_S[ERR_INT]\<NPA_AURA_ERR_INT_E::POOL_DIS\>. */
        uint64_t reserved_43           : 1;
        uint64_t aura                  : 20; /**< [ 63: 44](WO) Aura within VF that points to this pool. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_int
 *
 * NPA LF Pool Interrupt Operation Register
 * A 64-bit atomic load-and-add to this register reads
 * NPA_POOL_S[ERR_INT,ERR_INT_ENA,THRESH_INT,THRESH_INT_ENA]. A write optionally
 * sets or clears these fields. A read is RAZ.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_int
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_int_s
    {
        uint64_t err_int               : 8;  /**< [  7:  0](R/W/H) Returns NPA_AURA_HW_S[ERR_INT] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[ERR_INT] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t err_int_ena           : 8;  /**< [ 15:  8](R/W/H) Returns NPA_AURA_HW_S[ERR_INT_ENA] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[ERR_INT_ENA] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t thresh_int            : 1;  /**< [ 16: 16](R/W/H) Returns NPA_AURA_HW_S[THRESH_INT] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[THRESH_INT] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t thresh_int_ena        : 1;  /**< [ 17: 17](R/W/H) Returns NPA_AURA_HW_S[THRESH_INT_ENA] on an atomic load-and-add. On a write,
                                                                 write-one-to-set NPA_AURA_HW_S[THRESH_INT_ENA] if [SETOP] is set, write-one-to-clear
                                                                 otherwise. */
        uint64_t reserved_18_41        : 24;
        uint64_t op_err                : 1;  /**< [ 42: 42](RO/H) Operation error. See NPA_LF_AURA_OP_CNT[OP_ERR]. */
        uint64_t setop                 : 1;  /**< [ 43: 43](WO) Set operation. Valid on a write. Indicates write-one-to-set when set,
                                                                 write-one-to-clear otherwise. */
        uint64_t aura                  : 20; /**< [ 63: 44](WO) Aura within VF. This field is present on a write, or in the write data of
                                                                 an atomic load-and-add. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_pc
 *
 * NPA LF Pool Performance Count Register
 * A 64-bit atomic load-and-add to this register reads NPA_POOL_S[OP_PC] from a
 * given aura's pool. The aura is slected by the atomic write data, whose format
 * is NPA_AURA_OP_WDATA_S. Reads and writes are RAZ/WI.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_pc
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_pc_s
    {
        uint64_t op_pc                 : 48; /**< [ 47:  0](RO/H) Pool performance counter. See NPA_POOL_S[OP_PC]. */
        uint64_t op_err                : 1;  /**< [ 48: 48](RO/H) Operation error. See NPA_LF_POOL_OP_AVAILABLE[OP_ERR]. */
        uint64_t reserved_49_63        : 15;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_end0
 *
 * NPA LF Pool Pointer End Operation Register 0
 * A 128-bit write to the NPA_LF_POOL_OP_PTR_END0 and NPA_LF_POOL_OP_PTR_END1
 * registers writes to a given pool's pointer end value.
 * All other accesses to these registers (e.g. reads and 64-bit writes) are
 * RAZ/WI.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_ptr_end0
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_ptr_end0_s
    {
        uint64_t ptr_end               : 64; /**< [ 63:  0](WO) Value written to NPA_POOL_S[PTR_END]. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_end1
 *
 * NPA LF Pool Pointer End Operation Register 1
 * See NPA_LF_POOL_OP_PTR_END0.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_ptr_end1
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_ptr_end1_s
    {
        uint64_t aura                  : 20; /**< [ 19:  0](WO) Aura within VF that points to this pool. */
        uint64_t reserved_20_63        : 44;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_start0
 *
 * NPA LF Pool Pointer Start Operation Register 0
 * A 128-bit write to the NPA_LF_POOL_OP_PTR_START0 and NPA_LF_POOL_OP_PTR_START1
 * registers writes to a given pool's pointer start value.
 * All other accesses to these registers (e.g. reads and 64-bit writes) are
 * RAZ/WI.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_ptr_start0
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_ptr_start0_s
    {
        uint64_t ptr_start             : 64; /**< [ 63:  0](WO) Value written to NPA_POOL_S[PTR_START]. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_ptr_start1
 *
 * NPA LF Pool Pointer Start Operation Register 1
 * See NPA_LF_POOL_OP_PTR_START0.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_ptr_start1
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_ptr_start1_s
    {
        uint64_t aura                  : 20; /**< [ 19:  0](WO) Aura within VF that points to this pool. */
        uint64_t reserved_20_63        : 44;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_pool_op_thresh
 *
 * NPA LF Pool Threshold Operation Register
 * A 64-bit atomic load-and-add to this register reads
 * NPA_POOL_S[THRESH_UP,THRESH]. A write to the register writes
 * NPA_POOL_S[THRESH_UP,THRESH]. A read is RAZ.
 *
 * RSL accesses to this register are RAZ/WI.
 */
union cavm_npa_lf_pool_op_thresh
{
    uint64_t u;
    struct cavm_npa_lf_pool_op_thresh_s
    {
        uint64_t thresh                : 36; /**< [ 35:  0](R/W/H) Returns NPA_AURA_HW_S[THRESH] on an atomic load-and-add. Value written to
                                                                 NPA_AURA_HW_S[THRESH] on a write. */
        uint64_t reserved_36_41        : 6;
        uint64_t op_err                : 1;  /**< [ 42: 42](RO/H) Operation error. See NPA_LF_AURA_OP_CNT[OP_ERR]. */
        uint64_t thresh_up             : 1;  /**< [ 43: 43](R/W/H) Returns NPA_AURA_HW_S[THRESH_UP] on an atomic load-and-add. Value written
                                                                 to NPA_AURA_HW_S[THRESH_UP] on a write. */
        uint64_t aura                  : 20; /**< [ 63: 44](WO) Aura within VF. This field is present on a write, or in the write data of
                                                                 an atomic load-and-add. */
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_cnt
 *
 * NPA LF Queue Interrupt Count Registers
 */
union cavm_npa_lf_qintx_cnt
{
    uint64_t u;
    struct cavm_npa_lf_qintx_cnt_s
    {
        uint64_t count                 : 22; /**< [ 21:  0](R/W/H) Interrupt count. Value in NPA_QINT_HW_S[COUNT]. Number of pending
                                                                 interrupts to this QINT from NPA_AURA_S and NPA_POOL_S contexts in the VF.
                                                                 Hardware increments the counter when an interrupt is set and decrements it
                                                                 when an interrupt is cleared. NPA_LF_QINT()_INT[INTR] interrupt is set when
                                                                 the count is nonzero.

                                                                 Writes to this field are for diagnostic use only. The write data is a two's
                                                                 complement signed value added to the count. */
        uint64_t reserved_22_63        : 42;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_ena_w1c
 *
 * NPA LF Queue Interrupt Enable Clear Registers
 * This register clears interrupt enable bits.
 */
union cavm_npa_lf_qintx_ena_w1c
{
    uint64_t u;
    struct cavm_npa_lf_qintx_ena_w1c_s
    {
        uint64_t intr                  : 1;  /**< [  0:  0](R/W1C/H) Reads or clears enable for NPA_LF_QINT(0..63)_INT[INTR]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_ena_w1s
 *
 * NPA LF Queue Interrupt Enable Set Registers
 * This register sets interrupt enable bits.
 */
union cavm_npa_lf_qintx_ena_w1s
{
    uint64_t u;
    struct cavm_npa_lf_qintx_ena_w1s_s
    {
        uint64_t intr                  : 1;  /**< [  0:  0](R/W1S/H) Reads or sets enable for NPA_LF_QINT(0..63)_INT[INTR]. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_int
 *
 * NPA LF Queue Interrupt Registers
 */
union cavm_npa_lf_qintx_int
{
    uint64_t u;
    struct cavm_npa_lf_qintx_int_s
    {
        uint64_t intr                  : 1;  /**< [  0:  0](RO/H) Interrupt pending. Set when NPA_LF_QINT()_CNT[COUNT] is nonzero. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_qint#_int_w1s
 *
 * INTERNAL: NPA LF Queue Interrupt Set Registers
 */
union cavm_npa_lf_qintx_int_w1s
{
    uint64_t u;
    struct cavm_npa_lf_qintx_int_w1s_s
    {
        uint64_t intr                  : 1;  /**< [  0:  0](RO/H) Interrupt pending. Set when NPA_LF_QINT()_CNT[COUNT] is nonzero. */
        uint64_t reserved_1_63         : 63;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras
 *
 * NPA LF RAS Interrupt Register
 */
union cavm_npa_lf_ras
{
    uint64_t u;
    struct cavm_npa_lf_ras_s
    {
        uint64_t aura_poison           : 1;  /**< [  0:  0](R/W1C/H) Poisoned data returned on NPA_AURA_HW_S read. */
        uint64_t pool_poison           : 1;  /**< [  1:  1](R/W1C/H) Poisoned data returned on NPA_POOL_HW_S read. */
        uint64_t stack_poison          : 1;  /**< [  2:  2](R/W1C/H) Poisoned data returned on NPA_STACK_PAGE_S read. */
        uint64_t qint_poison           : 1;  /**< [  3:  3](R/W1C/H) Poisoned data returned on NPA_QINT_HW_S read. */
        uint64_t reserved_4_63         : 60;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras_ena_w1c
 *
 * NPA LF RAS Interrupt Enable Clear Register
 * This register clears interrupt enable bits.
 */
union cavm_npa_lf_ras_ena_w1c
{
    uint64_t u;
    struct cavm_npa_lf_ras_ena_w1c_s
    {
        uint64_t aura_poison           : 1;  /**< [  0:  0](R/W1C/H) Reads or clears enable for NPA_LF_RAS[AURA_POISON]. */
        uint64_t pool_poison           : 1;  /**< [  1:  1](R/W1C/H) Reads or clears enable for NPA_LF_RAS[POOL_POISON]. */
        uint64_t stack_poison          : 1;  /**< [  2:  2](R/W1C/H) Reads or clears enable for NPA_LF_RAS[STACK_POISON]. */
        uint64_t qint_poison           : 1;  /**< [  3:  3](R/W1C/H) Reads or clears enable for NPA_LF_RAS[QINT_POISON]. */
        uint64_t reserved_4_63         : 60;
    } s;
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras_ena_w1s
 *
 * NPA LF RAS Interrupt Enable Set Register
 * This register sets interrupt enable bits.
 */
union cavm_npa_lf_ras_ena_w1s
{
    uint64_t u;
    struct cavm_npa_lf_ras_ena_w1s_s
    {
        uint64_t aura_poison           : 1;  /**< [  0:  0](R/W1S/H) Reads or sets enable for NPA_LF_RAS[AURA_POISON]. */
        uint64_t pool_poison           : 1;  /**< [  1:  1](R/W1S/H) Reads or sets enable for NPA_LF_RAS[POOL_POISON]. */
        uint64_t stack_poison          : 1;  /**< [  2:  2](R/W1S/H) Reads or sets enable for NPA_LF_RAS[STACK_POISON]. */
        uint64_t qint_poison           : 1;  /**< [  3:  3](R/W1S/H) Reads or sets enable for NPA_LF_RAS[QINT_POISON]. */
        uint64_t reserved_4_63         : 60;
    } s;
    /* struct cavm_npa_lf_ras_ena_w1s_s cn; */
};

/**
 * Register (RVU_PFVF_BAR2) npa_lf_ras_w1s
 *
 * NPA LF RAS Interrupt Set Register
 * This register sets interrupt bits.
 */
union cavm_npa_lf_ras_w1s
{
    uint64_t u;
    struct cavm_npa_lf_ras_w1s_s
    {
        uint64_t aura_poison           : 1;  /**< [  0:  0](R/W1S/H) Reads or sets NPA_LF_RAS[AURA_POISON]. */
        uint64_t pool_poison           : 1;  /**< [  1:  1](R/W1S/H) Reads or sets NPA_LF_RAS[POOL_POISON]. */
        uint64_t stack_poison          : 1;  /**< [  2:  2](R/W1S/H) Reads or sets NPA_LF_RAS[STACK_POISON]. */
        uint64_t qint_poison           : 1;  /**< [  3:  3](R/W1S/H) Reads or sets NPA_LF_RAS[QINT_POISON]. */
        uint64_t reserved_4_63         : 60;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_priv_af_int_cfg
 *
 * NPA Privileged AF Interrupt Configuration Register
 */
union cavm_npa_priv_af_int_cfg
{
    uint64_t u;
    struct cavm_npa_priv_af_int_cfg_s
    {
        uint64_t msix_offset           : 11; /**< [ 10:  0](R/W) MSI-X offset. Offset of AF interrupt vectors enumerated by
                                                                 NPA_AF_INT_VEC_E in RVU PF(0)'s MSI-X table. This offset is added to each
                                                                 enumerated value to obtain the corresponding MSI-X vector index. The
                                                                 highest enumerated value plus [MSIX_OFFSET] must be less than or equal to
                                                                 RVU_PRIV_PF(0)_MSIX_CFG[PF_MSIXT_SIZEM1]. */
        uint64_t reserved_11           : 1;
        uint64_t msix_size             : 8;  /**< [ 19: 12](RO) Number of interrupt vectors enumerated by NPA_AF_INT_VEC_E. */
        uint64_t reserved_20_63        : 44;
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_priv_lf#_cfg
 *
 * NPA Privileged Local Function Configuration Registers
 * These registers allow each NPA local function (LF) to be provisioned to a VF/PF
 * slot for RVU. See also NPA_AF_RVU_LF_CFG_DEBUG.
 *
 * Software should read this register after write to ensure that the LF is mapped to
 * [PF_FUNC] before issuing transactions to the mapped PF and function.
 *
 * [SLOT] must be zero.
 *
 * Internal:
 * Hardware ignores [SLOT] and always assumes 0x0.
 */
union cavm_npa_priv_lfx_cfg
{
    uint64_t u;
    struct cavm_npa_priv_lfx_cfg_s
    {
        uint64_t slot                  : 8;  /**< [  7:  0](R/W) Slot within the VF/PF selected by [PF_FUNC] to which the LF is
                                                                 provisioned. */
        uint64_t pf_func               : 16; /**< [ 23:  8](R/W) RVU VF/PF to which the LF is provisioned. Format defined by RVU_PF_FUNC_S.
                                                                 Interrupts from the LF are delivered to the selected PF/VF. */
        uint64_t reserved_24_62        : 39;
        uint64_t ena                   : 1;  /**< [ 63: 63](R/W) Enable. When set, the LF is enabled and provisioned to the VF/PF slot
                                                                 selected by [PF_FUNC] and [SLOT]. When clear, the LF is not provisioned.

                                                                 LF to slot mapping must be 1-to-1. Thus, each enabled LF must be provisioned
                                                                 to a unique {[PF_FUNC], [SLOT]} combination. */
    } s;
};

/**
 * Register (RVU_PF_BAR0) npa_priv_lf#_int_cfg
 *
 * NPA Privileged LF Interrupt Configuration Registers
 */
union cavm_npa_priv_lfx_int_cfg
{
    uint64_t u;
    struct cavm_npa_priv_lfx_int_cfg_s
    {
        uint64_t msix_offset           : 11; /**< [ 10:  0](R/W) MSI-X offset. Offset of LF interrupt vectors enumerated by the block's
                                                                 NPA_LF_INT_VEC_E in the MSI-X table of the corresponding RVU VF/PF (see
                                                                 NPA_PRIV_LF()_CFG[PF_FUNC]). This offset is added to each enumerated value
                                                                 to obtain the corresponding MSI-X vector index. The highest enumerated
                                                                 value plus [MSIX_OFFSET] must be less than or equal to
                                                                 RVU_PRIV_PF()_MSIX_CFG[PF_MSIXT_SIZEM1,VF_MSIXT_SIZEM1]. */
        uint64_t reserved_11           : 1;
        uint64_t msix_size             : 8;  /**< [ 19: 12](RO) Number of interrupt vectors enumerated by NPA_LF_INT_VEC_E. */
        uint64_t reserved_20_63        : 44;
    } s;
    /* struct cavm_npa_priv_lfx_int_cfg_s cn; */

#endif /* __NPA_HW_H__ */
