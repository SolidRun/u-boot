/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2022 Marvell.
 */

#ifndef __DPI_INST_H_
#define __DPI_INST_H_

#define DPI_HDR_XTYPE_E_EXTERNAL (3)
#define DPI_HDR_XTYPE_E_EXTERNAL_ONLY (3)
#define DPI_HDR_XTYPE_E_INBOUND (1)
#define DPI_HDR_XTYPE_E_INTERNAL (2)
#define DPI_HDR_XTYPE_E_INTERNAL_ONLY (2)
#define DPI_HDR_XTYPE_E_OUTBOUND (0)

typedef union dpi_dma_ptr_s {
	u64 u[2];
	struct dpi_dma_s {
		u64 length:16;
		u64 reserved:44;
		u64 bed:1; /* Big-Endian */
		u64 alloc_l2:1;
		u64 full_write:1;
		u64 invert:1;
		u64 ptr;
	} s;
} dpi_dma_ptr_t;

union dpi_dma_instr_hdr_s {
	u64 u[4];
	struct dpi_dma_instr_hdr_s_s {
#ifdef __BIG_ENDIAN_BITFIELD /* Word 0 - Big Endian */
		u64 reserved_58_63        : 6;
		u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.
                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E:EXTERNAL_ONLY instructions, the last pointers block
                                                                 contains MAC pointers.

                                                                 With DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the last pointers block
                                                                 contains L2/DRAM pointers.

                                                                 The last pointers block is [NFST] * 2 64-bit words. Note that the sum of the number of
                                                                 64-bit words in the last pointers block and first pointers block must never exceed 60. */
		u64 reserved_52_53        : 2;
		u64 nfst                  : 4;  /**< [ 51: 48] The number of DPI_DMA_LOCAL_PTR_Ss in the first pointers block. Valid values are 1 thru 15.

                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the first pointers block contains
                                                                 L2/DRAM pointers.

                                                                 With DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, the first pointers
                                                                 block contains MAC pointers

                                                                 The first pointers block is [NFST] * 2 64-bit words. Note that the sum of the number
                                                                 of 64-bit words in the last pointers block and first pointers block must never exceed 60. */
        u64 reserved_44_47        : 4;
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request, SSO_PF_MAP() must map
                                                                 [GRP] and DPI()_DMA()_IDS[GMID] as valid.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
#else /* Word 0 - Little Endian */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request, SSO_PF_MAP() must map
                                                                 [GRP] and DPI()_DMA()_IDS[GMID] as valid.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 reserved_44_47        : 4;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of DPI_DMA_LOCAL_PTR_Ss in the first pointers block. Valid values are 1 thru 15.

                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the first pointers block contains
                                                                 L2/DRAM pointers.

                                                                 With DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, the first pointers
                                                                 block contains MAC pointers

                                                                 The first pointers block is [NFST] * 2 64-bit words. Note that the sum of the number
                                                                 of 64-bit words in the last pointers block and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E:EXTERNAL_ONLY instructions, the last pointers block
                                                                 contains MAC pointers.

                                                                 With DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the last pointers block
                                                                 contains L2/DRAM pointers.

                                                                 The last pointers block is [NFST] * 2 64-bit words. Note that the sum of the number of
                                                                 64-bit words in the last pointers block and first pointers block must never exceed 60. */
        u64 reserved_58_63        : 6;
#endif /* Word 0 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 1 - Big Endian */
        u64 reserved_126_127      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions.

                                                                 [LPORT] must be zero for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL_ONLY DPI DMA instruction.

                                                                 [FPORT] must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_114_119      : 6;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (L2/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>L2/DRAM), DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 (L2/DRAM-\>L2/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL_ONLY (MAC-\>MAC). */
        u64 reserved_109_111      : 3;
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SLI()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SLI()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY instruction.

                                                                 _ If [CSEL] = 0, DPI updates SLI(0)_EPF()_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SLI(0)_EPF()_DMA_CNT(1)[CNT].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 Note that these updates may indirectly cause corresponding
                                                                 SLI()_EPF()_DMA_RINT[DCNT,DTIME] to become set (depending on the
                                                                 SLI()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SLI()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SLI()_EPF()_DMA_CNT[CNT]. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 _ If [CSEL] = 0, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<0\>].
                                                                 _ If [CSEL] = 1, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<1\>].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_LOCAL_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_LOCAL_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_LOCAL_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_LOCAL_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_LOCAL_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_LOCAL_PTR_S[PTR] to [AURA] in FPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_LOCAL_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 reserved_102_103      : 2;
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 reserved_64_99        : 36;
#else /* Word 1 - Little Endian */
        u64 reserved_64_99        : 36;
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 reserved_102_103      : 2;
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_LOCAL_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_LOCAL_PTR_S[PTR] to [AURA] in FPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_LOCAL_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_LOCAL_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_LOCAL_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_LOCAL_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_LOCAL_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 _ If [CSEL] = 0, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<0\>].
                                                                 _ If [CSEL] = 1, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<1\>].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY instruction.

                                                                 _ If [CSEL] = 0, DPI updates SLI(0)_EPF()_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SLI(0)_EPF()_DMA_CNT(1)[CNT].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 Note that these updates may indirectly cause corresponding
                                                                 SLI()_EPF()_DMA_RINT[DCNT,DTIME] to become set (depending on the
                                                                 SLI()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SLI()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SLI()_EPF()_DMA_CNT[CNT]. */
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SLI()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SLI()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 reserved_109_111      : 3;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (L2/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>L2/DRAM), DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 (L2/DRAM-\>L2/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL_ONLY (MAC-\>MAC). */
        u64 reserved_114_119      : 6;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL_ONLY DPI DMA instruction.

                                                                 [FPORT] must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions.

                                                                 [LPORT] must be zero for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_126_127      : 2;
#endif /* Word 1 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 2 - Big Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:49\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<48\> for forward compatibility. */
#else /* Word 2 - Little Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:49\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<48\> for forward compatibility. */
#endif /* Word 2 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 3 - Big Endian */
        u64 reserved_192_255      : 64;
#else /* Word 3 - Little Endian */
        u64 reserved_192_255      : 64;
#endif /* Word 3 - End */
    } s;
    struct dpi_dma_instr_hdr_s_cn8
    {
#ifdef __BIG_ENDIAN_BITFIELD /* Word 0 - Big Endian */
        u64 reserved_60_63        : 4;
        u64 reserved_58_59        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E:EXTERNAL_ONLY instructions, the last pointers block
                                                                 contains MAC pointers.

                                                                 With DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the last pointers block
                                                                 contains L2/DRAM pointers.

                                                                 The last pointers block is [NFST] * 2 64-bit words. Note that the sum of the number of
                                                                 64-bit words in the last pointers block and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of DPI_DMA_LOCAL_PTR_Ss in the first pointers block. Valid values are 1 thru 15.

                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the first pointers block contains
                                                                 L2/DRAM pointers.

                                                                 With DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, the first pointers
                                                                 block contains MAC pointers

                                                                 The first pointers block is [NFST] * 2 64-bit words. Note that the sum of the number
                                                                 of 64-bit words in the last pointers block and first pointers block must never exceed 60. */
        u64 reserved_46_47        : 2;
        u64 reserved_44_45        : 2;
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request, SSO_PF_MAP() must map
                                                                 [GRP] and DPI()_DMA()_IDS[GMID] as valid.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
#else /* Word 0 - Little Endian */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request, SSO_PF_MAP() must map
                                                                 [GRP] and DPI()_DMA()_IDS[GMID] as valid.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 reserved_44_45        : 2;
        u64 reserved_46_47        : 2;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of DPI_DMA_LOCAL_PTR_Ss in the first pointers block. Valid values are 1 thru 15.

                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the first pointers block contains
                                                                 L2/DRAM pointers.

                                                                 With DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, the first pointers
                                                                 block contains MAC pointers

                                                                 The first pointers block is [NFST] * 2 64-bit words. Note that the sum of the number
                                                                 of 64-bit words in the last pointers block and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 With DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::OUTBOUND, and
                                                                 DPI_HDR_XTYPE_E:EXTERNAL_ONLY instructions, the last pointers block
                                                                 contains MAC pointers.

                                                                 With DPI_HDR_XTYPE_E::INTERNAL_ONLY instructions, the last pointers block
                                                                 contains L2/DRAM pointers.

                                                                 The last pointers block is [NFST] * 2 64-bit words. Note that the sum of the number of
                                                                 64-bit words in the last pointers block and first pointers block must never exceed 60. */
        u64 reserved_58_59        : 2;
        u64 reserved_60_63        : 4;
#endif /* Word 0 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 1 - Big Endian */
        u64 reserved_126_127      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions.

                                                                 [LPORT] must be zero for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL_ONLY DPI DMA instruction.

                                                                 [FPORT] must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_116_119      : 4;
        u64 reserved_114_115      : 2;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (L2/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>L2/DRAM), DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 (L2/DRAM-\>L2/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL_ONLY (MAC-\>MAC). */
        u64 reserved_109_111      : 3;
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SLI()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SLI()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY instruction.

                                                                 _ If [CSEL] = 0, DPI updates SLI(0)_EPF()_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SLI(0)_EPF()_DMA_CNT(1)[CNT].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 Note that these updates may indirectly cause corresponding
                                                                 SLI()_EPF()_DMA_RINT[DCNT,DTIME] to become set (depending on the
                                                                 SLI()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SLI()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SLI()_EPF()_DMA_CNT[CNT]. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 _ If [CSEL] = 0, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<0\>].
                                                                 _ If [CSEL] = 1, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<1\>].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_LOCAL_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_LOCAL_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_LOCAL_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_LOCAL_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_LOCAL_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_LOCAL_PTR_S[PTR] to [AURA] in FPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_LOCAL_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 reserved_102_103      : 2;
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 reserved_98_99        : 2;
        u64 pvfe                  : 1;  /**< [ 97: 97] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [DEALLOCV] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE]
                                                                 must not be set when [DEALLOCE] is set. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL_ONLY or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 dealloce              : 1;  /**< [ 96: 96] Aura count subtract enable. When [DEALLOCE] is set, DPI subtracts
                                                                 [DEALLOCV] from the FPA aura count selected by [AURA]. When [DEALLOCE]
                                                                 is clear, DPI does not subtract [DEALLOCV] from any aura count.
                                                                 [DEALLOCE] can only be set when [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND.
                                                                 [DEALLOCE] must not be set when [PVFE] is set. [DEALLOCV] must not be 0
                                                                 when [DEALLOCE] is set.

                                                                 The [DEALLOCV] aura count subtract is in addition to other
                                                                 aura count activity. When FPA_AURA([AURA])_CFG[PTR_DIS]==0, FPA also
                                                                 decrements the aura count by one for each L2/DRAM pointer free. */
        u64 deallocv              : 16; /**< [ 95: 80] The DEALLOCV/FUNC field.

                                                                 When [DEALLOCE] is set, [DEALLOCV] is the value to decrement the aura count on
                                                                 the instruction's final pointer return.

                                                                 When [PVFE] is set, DPI_DMA_FUNC_SEL_S describes the [DEALLOCV] format.
                                                                 [DEALLOCV] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. */
        u64 reserved_76_79        : 4;
        u64 aura                  : 12; /**< [ 75: 64] FPA guest-aura.  The FPA guest-aura DPI uses for all FPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the FPA to not discard the request, FPA_PF_MAP() must map
                                                                 [AURA] and DPI()_DMA()_IDS[GMID] as valid.

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_LOCAL_PTR_S[I] determine whether DPI frees a
                                                                 L2/DRAM address to [AURA] in FPA, and [DEALLOCE] determines
                                                                 whether DPI subtracts [DEALLOCV] from [AURA]'s FPA aura counter. */
#else /* Word 1 - Little Endian */
        u64 aura                  : 12; /**< [ 75: 64] FPA guest-aura.  The FPA guest-aura DPI uses for all FPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the FPA to not discard the request, FPA_PF_MAP() must map
                                                                 [AURA] and DPI()_DMA()_IDS[GMID] as valid.

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_LOCAL_PTR_S[I] determine whether DPI frees a
                                                                 L2/DRAM address to [AURA] in FPA, and [DEALLOCE] determines
                                                                 whether DPI subtracts [DEALLOCV] from [AURA]'s FPA aura counter. */
        u64 reserved_76_79        : 4;
        u64 deallocv              : 16; /**< [ 95: 80] The DEALLOCV/FUNC field.

                                                                 When [DEALLOCE] is set, [DEALLOCV] is the value to decrement the aura count on
                                                                 the instruction's final pointer return.

                                                                 When [PVFE] is set, DPI_DMA_FUNC_SEL_S describes the [DEALLOCV] format.
                                                                 [DEALLOCV] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. */
        u64 dealloce              : 1;  /**< [ 96: 96] Aura count subtract enable. When [DEALLOCE] is set, DPI subtracts
                                                                 [DEALLOCV] from the FPA aura count selected by [AURA]. When [DEALLOCE]
                                                                 is clear, DPI does not subtract [DEALLOCV] from any aura count.
                                                                 [DEALLOCE] can only be set when [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND.
                                                                 [DEALLOCE] must not be set when [PVFE] is set. [DEALLOCV] must not be 0
                                                                 when [DEALLOCE] is set.

                                                                 The [DEALLOCV] aura count subtract is in addition to other
                                                                 aura count activity. When FPA_AURA([AURA])_CFG[PTR_DIS]==0, FPA also
                                                                 decrements the aura count by one for each L2/DRAM pointer free. */
        u64 pvfe                  : 1;  /**< [ 97: 97] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [DEALLOCV] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE]
                                                                 must not be set when [DEALLOCE] is set. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL_ONLY or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 reserved_98_99        : 2;
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 reserved_102_103      : 2;
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_LOCAL_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_LOCAL_PTR_S[PTR] to [AURA] in FPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_LOCAL_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_LOCAL_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_LOCAL_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_LOCAL_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_LOCAL_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_LOCAL_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL_ONLY, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL_ONLY) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 _ If [CSEL] = 0, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<0\>].
                                                                 _ If [CSEL] = 1, DPI sets SLI()_EPF()_DMA_RINT[DMAFI\<1\>].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY instruction.

                                                                 _ If [CSEL] = 0, DPI updates SLI(0)_EPF()_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SLI(0)_EPF()_DMA_CNT(1)[CNT].

                                                                 [LPORT] and [DEALLOCV] determine the EPF used. ([DEALLOCV] is
                                                                 only relevant when [PVFE]=1. Only the DPI_DMA_FUNC_SEL_S[PF]
                                                                 of [DEALLOCV] is relevant here.)

                                                                 Note that these updates may indirectly cause corresponding
                                                                 SLI()_EPF()_DMA_RINT[DCNT,DTIME] to become set (depending on the
                                                                 SLI()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SLI()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SLI()_EPF()_DMA_CNT[CNT]. */
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SLI()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SLI()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL_ONLY
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 reserved_109_111      : 3;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (L2/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>L2/DRAM), DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 (L2/DRAM-\>L2/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL_ONLY (MAC-\>MAC). */
        u64 reserved_114_115      : 2;
        u64 reserved_116_119      : 4;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL_ONLY DPI DMA instruction.

                                                                 [FPORT] must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL_ONLY instructions.

                                                                 [LPORT] must be zero for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                                 instructions. */
        u64 reserved_126_127      : 2;
#endif /* Word 1 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 2 - Big Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:49\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<48\> for forward compatibility. */
#else /* Word 2 - Little Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:49\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<48\> for forward compatibility. */
#endif /* Word 2 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 3 - Big Endian */
        u64 reserved_192_255      : 64;
#else /* Word 3 - Little Endian */
        u64 reserved_192_255      : 64;
#endif /* Word 3 - End */
    } cn8;
    struct dpi_dma_instr_hdr_s_cn9
    {
#ifdef __BIG_ENDIAN_BITFIELD /* Word 0 - Big Endian */
        u64 reserved_58_63        : 6;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_44_47        : 4;
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
#else /* Word 0 - Little Endian */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 reserved_44_47        : 4;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_58_63        : 6;
#endif /* Word 0 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 1 - Big Endian */
        u64 reserved_126_127      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_114_119      : 6;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        u64 reserved_109_111      : 3;
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 pvfe                  : 1;  /**< [103:103] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 reserved_102          : 1;
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 func                  : 16; /**< [ 99: 84] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        u64 aura                  : 20; /**< [ 83: 64] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
#else /* Word 1 - Little Endian */
        u64 aura                  : 20; /**< [ 83: 64] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
        u64 func                  : 16; /**< [ 99: 84] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 reserved_102          : 1;
        u64 pvfe                  : 1;  /**< [103:103] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 reserved_109_111      : 3;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        u64 reserved_114_119      : 6;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_126_127      : 2;
#endif /* Word 1 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 2 - Big Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#else /* Word 2 - Little Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#endif /* Word 2 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 3 - Big Endian */
        u64 reserved_192_255      : 64;
#else /* Word 3 - Little Endian */
        u64 reserved_192_255      : 64;
#endif /* Word 3 - End */
    } cn9;
    struct dpi_dma_instr_hdr_s_cn96xx
    {
#ifdef __BIG_ENDIAN_BITFIELD /* Word 0 - Big Endian */
        u64 reserved_60_63        : 4;
        u64 reserved_58_59        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_46_47        : 2;
        u64 reserved_44_45        : 2;
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
#else /* Word 0 - Little Endian */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 reserved_44_45        : 2;
        u64 reserved_46_47        : 2;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_58_59        : 2;
        u64 reserved_60_63        : 4;
#endif /* Word 0 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 1 - Big Endian */
        u64 reserved_126_127      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_116_119      : 4;
        u64 reserved_114_115      : 2;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        u64 reserved_109_111      : 3;
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 pvfe                  : 1;  /**< [103:103] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 reserved_102          : 1;
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 func                  : 16; /**< [ 99: 84] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        u64 aura                  : 20; /**< [ 83: 64] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
#else /* Word 1 - Little Endian */
        u64 aura                  : 20; /**< [ 83: 64] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
        u64 func                  : 16; /**< [ 99: 84] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 reserved_102          : 1;
        u64 pvfe                  : 1;  /**< [103:103] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP(0)_EPF(0..15)_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 reserved_109_111      : 3;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        u64 reserved_114_115      : 2;
        u64 reserved_116_119      : 4;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_126_127      : 2;
#endif /* Word 1 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 2 - Big Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#else /* Word 2 - Little Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#endif /* Word 2 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 3 - Big Endian */
        u64 reserved_192_255      : 64;
#else /* Word 3 - Little Endian */
        u64 reserved_192_255      : 64;
#endif /* Word 3 - End */
    } cn96xx;
    struct dpi_dma_instr_hdr_s_cn98xx
    {
#ifdef __BIG_ENDIAN_BITFIELD /* Word 0 - Big Endian */
        u64 reserved_60_63        : 4;
        u64 reserved_58_59        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_46_47        : 2;
        u64 reserved_44_45        : 2;
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
#else /* Word 0 - Little Endian */
        u64 tag                   : 32; /**< [ 31:  0] SSO Tag */
        u64 tt                    : 2;  /**< [ 33: 32] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 grp                   : 10; /**< [ 43: 34] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        u64 reserved_44_45        : 2;
        u64 reserved_46_47        : 2;
        u64 nfst                  : 4;  /**< [ 51: 48] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_52_53        : 2;
        u64 nlst                  : 4;  /**< [ 57: 54] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        u64 reserved_58_59        : 2;
        u64 reserved_60_63        : 4;
#endif /* Word 0 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 1 - Big Endian */
        u64 reserved_126_127      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_116_119      : 4;
        u64 reserved_114_115      : 2;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        u64 reserved_109_111      : 3;
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP(0..1)_EPF(0..15)_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP(0..1)_EPF(0..15)_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 pvfe                  : 1;  /**< [103:103] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 reserved_102          : 1;
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 func                  : 16; /**< [ 99: 84] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        u64 aura                  : 20; /**< [ 83: 64] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
#else /* Word 1 - Little Endian */
        u64 aura                  : 20; /**< [ 83: 64] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
        u64 func                  : 16; /**< [ 99: 84] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        u64 pt                    : 2;  /**< [101:100] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        u64 reserved_102          : 1;
        u64 pvfe                  : 1;  /**< [103:103] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        u64 fl                    : 1;  /**< [104:104] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 ii                    : 1;  /**< [105:105] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        u64 fi                    : 1;  /**< [106:106] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        u64 ca                    : 1;  /**< [107:107] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP(0..1)_EPF(0..15)_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP(0..1)_EPF(0..15)_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        u64 csel                  : 1;  /**< [108:108] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        u64 reserved_109_111      : 3;
        u64 xtype                 : 2;  /**< [113:112] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        u64 reserved_114_115      : 2;
        u64 reserved_116_119      : 4;
        u64 fport                 : 2;  /**< [121:120] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_122_123      : 2;
        u64 lport                 : 2;  /**< [125:124] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        u64 reserved_126_127      : 2;
#endif /* Word 1 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 2 - Big Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#else /* Word 2 - Little Endian */
        u64 ptr                   : 64; /**< [191:128] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#endif /* Word 2 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 3 - Big Endian */
        u64 reserved_192_255      : 64;
#else /* Word 3 - Little Endian */
        u64 reserved_192_255      : 64;
#endif /* Word 3 - End */
    } cn98xx;
    struct dpi_dma_instr_hdr_s_cn10k
    {
#ifdef __BIG_ENDIAN_BITFIELD /* Word 0 - Big Endian */
        uint64_t reserved_62_63        : 2;
        uint64_t lport                 : 2;  /**< [ 61: 60] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        uint64_t reserved_58_59        : 2;
        uint64_t fport                 : 2;  /**< [ 57: 56] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        uint64_t pt                    : 2;  /**< [ 55: 54] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        uint64_t reserved_52_53        : 2;
        uint64_t xtype                 : 2;  /**< [ 51: 50] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        uint64_t aura                  : 20; /**< [ 49: 30] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
        uint64_t func                  : 16; /**< [ 29: 14] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        uint64_t reserved_13           : 1;
        uint64_t pvfe                  : 1;  /**< [ 12: 12] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        uint64_t reserved_10_11        : 2;
        uint64_t nlst                  : 4;  /**< [  9:  6] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        uint64_t reserved_4_5          : 2;
        uint64_t nfst                  : 4;  /**< [  3:  0] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
#else /* Word 0 - Little Endian */
        uint64_t nfst                  : 4;  /**< [  3:  0] The number of pointers in the first pointers block. Valid values are 1 through 15.

                                                                 The first pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST]
                                                                 * 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        uint64_t reserved_4_5          : 2;
        uint64_t nlst                  : 4;  /**< [  9:  6] The number of pointers in the last pointers block. Valid values are 1 through 15.

                                                                 The last pointers block includes [NFST] DPI_DMA_PTR_S local pointers and is [NFST] *
                                                                 2 64-bit words.

                                                                 Note that the sum of the number of 64-bit words in the last pointers block
                                                                 and first pointers block must never exceed 60. */
        uint64_t reserved_10_11        : 2;
        uint64_t pvfe                  : 1;  /**< [ 12: 12] Function enable. When [PVFE] is set, DPI directs all MAC reads/writes
                                                                 to the function (physical or virtual) that [FUNC] selects within
                                                                 MAC/port [LPORT]. When [PVFE] is clear, DPI directs all MAC reads/writes
                                                                 to physical function 0 within the MACs/ports [LPORT] or [FPORT]. [PVFE] must not be set when
                                                                 [XTYPE] is DPI_HDR_XTYPE_E::INTERNAL or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL. [PVFE] can only be set when MAC/PORT
                                                                 [LPORT] is a PCIe MAC that either has multiple physical functions or
                                                                 has SR-IOV enabled. */
        uint64_t reserved_13           : 1;
        uint64_t func                  : 16; /**< [ 29: 14] [FUNC] selects the function within the MAC selected by [LPORT]
                                                                 when [PVFE] is set. Defined by DPI_DMA_FUNC_SEL_S. */
        uint64_t aura                  : 20; /**< [ 49: 30] NPA guest-aura.  The NPA guest-aura DPI uses for all NPA transactions for the
                                                                 DPI DMA instruction. [AURA] can only be used when
                                                                 [XTYPE]=DPI_HDR_XTYPE_E::OUTBOUND, and must be zero otherwise.

                                                                 For the NPA to not discard the request, NPA must map [AURA] to be
                                                                 valid inside the PF/function DPI()_DMA()_IDS[SSO_PF_FUNC].

                                                                 During an DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instruction, [FL], [II],
                                                                 and DPI_DMA_PTR_S[I] determine whether DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA. */
        uint64_t xtype                 : 2;  /**< [ 51: 50] Transfer type of the instruction. Enumerated by DPI_HDR_XTYPE_E. Each DPI DMA
                                                                 instruction can be DPI_HDR_XTYPE_E::OUTBOUND (LLC/DRAM-\>MAC),
                                                                 DPI_HDR_XTYPE_E::INBOUND (MAC-\>LLC/DRAM), DPI_HDR_XTYPE_E::INTERNAL
                                                                 (LLC/DRAM-\>LLC/DRAM), or DPI_HDR_XTYPE_E::EXTERNAL (MAC-\>MAC). */
        uint64_t reserved_52_53        : 2;
        uint64_t pt                    : 2;  /**< [ 55: 54] Pointer type. Enumerated by DPI_HDR_PT_E. Indicates how [PTR] is used
                                                                 upon completion of the DPI DMA instruction: byte write, SSO
                                                                 work add, or counter add.

                                                                 If no completion indication is desired for the DPI DMA instruction,
                                                                 software should set [PT]=DPI_HDR_PT_E::ZBW_CA and [PTR]=0. */
        uint64_t fport                 : 2;  /**< [ 57: 56] Port for the first pointers block.

                                                                 DPI sends MAC memory space reads for the MAC addresses in the first
                                                                 pointers block to the MAC selected by [FPORT] while processing
                                                                 a DPI_HDR_XTYPE_E::EXTERNAL DPI DMA instruction.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for
                                                                 DPI_HDR_XTYPE_E::OUTBOUND and DPI_HDR_XTYPE_E::INBOUND instructions.

                                                                 [FPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [FPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::OUTBOUND,
                                                                 DPI_HDR_XTYPE_E::INBOUND, and DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        uint64_t reserved_58_59        : 2;
        uint64_t lport                 : 2;  /**< [ 61: 60] Port for the last pointers block.

                                                                 DPI sends MAC memory space reads and writes for the MAC addresses
                                                                 in the last pointers block to the MAC selected by [LPORT]
                                                                 while processing DPI_HDR_XTYPE_E::OUTBOUND, DPI_HDR_XTYPE_E::INBOUND,
                                                                 and DPI_HDR_XTYPE_E::EXTERNAL instructions.

                                                                 [LPORT]\<0\> normally determines the NCB DPI uses for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions.

                                                                 [LPORT]\<1\> must be zero for DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions. */
        uint64_t reserved_62_63        : 2;
#endif /* Word 0 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 1 - Big Endian */
        uint64_t ptr                   : 64; /**< [127: 64] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#else /* Word 1 - Little Endian */
        uint64_t ptr                   : 64; /**< [127: 64] Completion pointer. Usage determined by [PT] value. The DPI_HDR_PT_E
                                                                 enumeration describes the supported [PT] values and the [PTR] usage
                                                                 and requirements in each case.
                                                                 Bits \<63:53\> are ignored by hardware; software should use a sign-extended bit
                                                                 \<52\> for forward compatibility. */
#endif /* Word 1 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 2 - Big Endian */
        uint64_t reserved_179_191      : 13;
        uint64_t csel                  : 1;  /**< [178:178] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        uint64_t ca                    : 1;  /**< [177:177] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP()_EPF()_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP()_EPF()_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        uint64_t fi                    : 1;  /**< [176:176] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        uint64_t ii                    : 1;  /**< [175:175] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        uint64_t fl                    : 1;  /**< [174:174] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        uint64_t reserved_172_173      : 2;
        uint64_t grp                   : 10; /**< [171:162] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        uint64_t tt                    : 2;  /**< [161:160] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        uint64_t tag                   : 32; /**< [159:128] SSO Tag. */
#else /* Word 2 - Little Endian */
        uint64_t tag                   : 32; /**< [159:128] SSO Tag. */
        uint64_t tt                    : 2;  /**< [161:160] SSO tag type. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.

                                                                 [TT] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        uint64_t grp                   : 10; /**< [171:162] SSO guest-group. Sent to SSO upon instruction completion when [PT] = DPI_HDR_PT_E::WQP.
                                                                 For the SSO to not discard the add-work request.

                                                                 [GRP] must be zero when [PT] != DPI_HDR_PT_E::WQP. */
        uint64_t reserved_172_173      : 2;
        uint64_t fl                    : 1;  /**< [174:174] Determines whether DPI frees a DPI_DMA_PTR_S[PTR] during
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction processing
                                                                 (along with [II] and DPI_DMA_PTR_S[I]).

                                                                 During DPI_HDR_XTYPE_E::OUTBOUND instruction processing, DPI frees a
                                                                 DPI_DMA_PTR_S[PTR] to [AURA] in NPA when:

                                                                 _ [FL] XOR (![II] AND DPI_DMA_PTR_S[I])

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [FL] must be clear,
                                                                 and DPI never frees local buffers. */
        uint64_t ii                    : 1;  /**< [175:175] Ignore I. Determines if DPI_DMA_PTR_S[I]'s influence whether
                                                                 DPI frees a DPI_DMA_PTR_S[PTR] during DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction processing.

                                                                 If [II] is set, [FL] solely determines whether DPI frees, and
                                                                 all DPI_DMA_PTR_S[PTR]'s in the DPI_HDR_XTYPE_E::OUTBOUND
                                                                 instruction are either freed or not.

                                                                 If [II] is clear, ([FL] XOR DPI_DMA_PTR_S[I]) determines
                                                                 whether DPI frees a given DPI_DMA_PTR_S[PTR] in the
                                                                 DPI_HDR_XTYPE_E::OUTBOUND instruction, and each .

                                                                 For DPI_HDR_XTYPE_E::INBOUND, DPI_HDR_XTYPE_E::INTERNAL, or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL instructions, [II] must be clear,
                                                                 and DPI never frees local buffers. */
        uint64_t fi                    : 1;  /**< [176:176] Force interrupt to a remote host.

                                                                 When [FI] is set for a (DPI_HDR_XTYPE_E::OUTBOUND or
                                                                 DPI_HDR_XTYPE_E::EXTERNAL) DPI DMA instruction, DPI sets an
                                                                 interrupt bit after completing instruction. If [CSEL] = 0, DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<0\> for all MACs, else DPI sets
                                                                 SDP()_EPF()_DMA_RINT[DMAFI]\<1\> for all MACs. This may
                                                                 cause an interrupt to be sent to a remote host.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL
                                                                 instructions, [II] must be clear, and DPI never sets DMAFI interrupt bits. */
        uint64_t ca                    : 1;  /**< [177:177] Add to a counter that can interrupt a remote host.

                                                                 When [CA] = 1, DPI updates a selected counter after it completes the DMA
                                                                 DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL instruction.

                                                                 _ If [CSEL] = 0, DPI updates SDP()_EPF()_DMA_CNT(0)[CNT].
                                                                 _ If [CSEL] = 1, DPI updates SDP()_EPF()_DMA_CNT(1)[CNT].

                                                                 Note that these updates may indirectly cause
                                                                 SDP()_EPF()_DMA_RINT[DCNT,DTIME] to become set for all MACs
                                                                 (depending on the SDP()_EPF()_DMA_INT_LEVEL() settings), so may cause interrupts to
                                                                 be sent to a remote MAC host.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 1, DPI updates the counter by 1.

                                                                 If DPI()_DMA_CONTROL[O_ADD1] = 0, DPI updates the counter by the total bytes in
                                                                 the transfer.

                                                                 When [CA] = 0, DPI does not update any SDP()_EPF()_DMA_CNT()[CNT].

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CA] must never be set, and DPI never adds to any
                                                                 SDP()_EPF()_DMA_CNT()[CNT]. */
        uint64_t csel                  : 1;  /**< [178:178] Counter and interrupt select. See [CA] and [FI]. [CSEL] selects which of two counters
                                                                 (SDP()_EPF()_DMA_CNT()[CNT]) and/or two interrupt bits (SDP()_EPF()_DMA_RINT[DMAFI])
                                                                 DPI can modify during DPI_HDR_XTYPE_E::OUTBOUND or DPI_HDR_XTYPE_E::EXTERNAL
                                                                 instruction execution.

                                                                 For DPI_HDR_XTYPE_E::INBOUND or DPI_HDR_XTYPE_E::INTERNAL DPI DMA
                                                                 instructions, [CSEL] must be zero. */
        uint64_t reserved_179_191      : 13;
#endif /* Word 2 - End */
#ifdef __BIG_ENDIAN_BITFIELD /* Word 0 - Big Endian */
        uint64_t reserved_192_255      : 64;
#else /* Word 3 - Little Endian */
        uint64_t reserved_192_255      : 64;
#endif /* Word 3 - End */

    } cn10k;
};

#endif /* __DPI_INST_H_ */
