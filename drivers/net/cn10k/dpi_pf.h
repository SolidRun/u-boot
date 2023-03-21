/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2022 Marvell.
 */

#ifndef __DPI_PF_H__
#define	__DPI_PF_H__

#include "dpi_inst.h"

#define DPI_DMA_CMD_BUF_SIZE			64

#define DPI_DMA_IBUFF_CSIZE_CSIZE(x)            ((x) & 0x1fff)
#define DPI_DMA_IBUFF_CSIZE_GET_CSIZE(x)        ((x) & 0x1fff)

#define DPI_DMA_IBUFF_CSIZE_NPA_FREE            (1 << 16)

#define DPI_DMA_IDS_DMA_NPA_PF_FUNC(x)          ((u64)((x) & 0xffff) << 16)
#define DPI_DMA_IDS_GET_DMA_NPA_PF_FUNC(x)      (((x) >> 16) & 0xffff)

#define DPI_DMA_IDS2_INST_AURA(x)               ((u64)((x) & 0xfffff))
#define DPI_DMA_IDS2_GET_INST_AURA(x)           ((x) & 0xfffff)

#define DPI_ENG_BUF_BLKS(x)                     ((x) & 0x1fULL)
#define DPI_ENG_BUF_GET_BLKS(x)                 ((x) & 0x1fULL)

#define DPI_ENG_BUF_BASE(x)                     (((x) & 0x3fULL) << 16)
#define DPI_ENG_BUF_GET_BASE(x)                 (((x) >> 16) & 0x3fULL)

#define DPI_DMA_ENG_EN_QEN(x)                   ((x) & 0xffULL)
#define DPI_DMA_ENG_EN_GET_QEN(x)               ((x) & 0xffULL)

#define DPI_DMA_CONTROL_DMA_ENB(x)              (((x) & 0x3fULL) << 48)
#define DPI_DMA_CONTROL_GET_DMA_ENB(x)          (((x) >> 48) & 0x3fULL)

#define DPI_DMA_CONTROL_O_ES(x)                 (((x) & 0x3ULL) << 15)
#define DPI_DMA_CONTROL_GET_O_ES(x)             (((x) >> 15) & 0x3ULL)

#define DPI_DMA_CONTROL_O_MODE                  (0x1ULL << 14)
#define DPI_DMA_CONTROL_O_NS                    (0x1ULL << 17)
#define DPI_DMA_CONTROL_O_RO                    (0x1ULL << 18)
#define DPI_DMA_CONTROL_O_ADD1                  (0x1ULL << 19)
#define DPI_DMA_CONTROL_LDWB                    (0x1ULL << 32)
#define DPI_DMA_CONTROL_NCB_TAG_DIS             (0x1ULL << 34)
#define DPI_DMA_CONTROL_ZBWCSEN                 (0x1ULL << 39)
#define DPI_DMA_CONTROL_WQECSDIS                (0x1ULL << 47)
#define DPI_DMA_CONTROL_UIO_DIS                 (0x1ULL << 55)
#define DPI_DMA_CONTROL_PKT_EN                  (0x1ULL << 56)
#define DPI_DMA_CONTROL_FFP_DIS                 (0x1ULL << 59)

#define DPI_CTL_EN                              0x1ULL

#define DPI_MAX_ENGINES                         6
#define DPI_EBUS_MAX_PORTS                      2
#define DPI_EBUS_PORTX_CFG_MRRS(x)              (((x) & 0x7) << 0)
#define DPI_EBUS_PORTX_CFG_MPS(x)               (((x) & 0x7) << 4)

/***************** Registers ******************/
#define DPI_DMAX_IBUFF_CSIZE                 0x0ULL
#define DPI_DMAX_REQBANK0                    0x8ULL
#define DPI_DMAX_REQBANK1                    0x10ULL
#define DPI_DMAX_IDS                         0x18ULL
#define DPI_DMAX_IDS2                        0x20ULL
#define DPI_DMAX_IFLIGHT                     0x28ULL
#define DPI_DMAX_QRST                        0x30ULL
#define DPI_DMAX_ERR_RSP_STATUS              0x38ULL
#define DPI_CTL	                             0x10010ULL
#define DPI_DMA_CONTROL                      0x10018ULL
#define DPI_DMA_ENGX_EN(x)                   (0x10040ULL | (x) << 3)
#define DPI_DMA_ENGX_BUF(x)                  (0x100c0ULL | (x) << 3)
#define DPI_EBUS_PORTX_CFG(x)                (0x10100ULL | (x) << 3)
#define DPI_WCTL_FIF_THR                     (0x17008ULL)

/* VF Registers: */
#define DPI_VDMA_EN             0x0ULL
#define DPI_VDMA_REQQ_CTL       0x8ULL
#define DPI_VDMA_DBELL          0x10ULL
#define DPI_VDMA_SADDR          0x18ULL
#define DPI_VDMA_COUNTS         0x20ULL
#define DPI_VDMA_NADDR          0x28ULL
#define DPI_VDMA_IWBUSY         0x30ULL
#define DPI_VDMA_CNT            0x38ULL

#define BAR4_DRAM_OFFSET	0x10000000 //256MB

#define PEM_BASE		0x8e0000000000
#define PEM_DIAG_STS_OFFSET	0x10
#define PEM_DIS_PORT_OFFSET	0x50
#define PEM_CFG_OFFSET		0xd8
#define PEM_BAR4_INDEX_OFFSET(x) (0x700 + (x << 3))

#define GLOBAL_REG_OFFSET	0
#define HOST_VERSION_REG	(GLOBAL_REG_OFFSET + 8)

#define HOST_DOWN                 0
#define HOST_READY                1
#define HOST_RUNNING              2
#define HOST_GOING_DOWN           3
#define HOST_FATAL                4

#define HOST_RW_OFFSET            128
#define HOST_STATUS_REG           (HOST_RW_OFFSET + 0)
#define HOST_INTR_REG             (HOST_RW_OFFSET + 8)
#define HOST_MBOX_ACK_REG         (HOST_RW_OFFSET + 16)
#define HOST_MBOX_OFFSET          (HOST_RW_OFFSET + 24)

#define TARGET_DOWN               0
#define TARGET_READY              1
#define TARGET_RUNNING            2
#define TARGET_GOING_DOWN         3
#define TARGET_FATAL              4

#define TARGET_VERSION                    0x0100

#define TARGET_VERSION_REG                0x60
#define TARGET_RW_OFFSET                  256
#define TARGET_STATUS_REG                 (TARGET_RW_OFFSET + 0)
#define TARGET_INTR_REG                   (TARGET_RW_OFFSET + 8)
#define TARGET_MBOX_ACK_REG               (TARGET_RW_OFFSET + 16)
#define TARGET_MBOX_OFFSET                (TARGET_RW_OFFSET + 24)

#define TX_DESCQ_OFFSET		1024
#define RX_DESCQ_OFFSET		65536

#define RECV_BUF_SIZE		12288
#define INST_CHUNK_SIZE		1024

#define MBOX_SIZE_WORDS 8
#define MBOX_HOST_STATUS_CHANGE 1
#define MBOX_TARGET_STATUS_CHANGE 2

#define COMP_TIMEOUT_MS 1000
#define MBOX_TIMEOUT_MS 100
#define MBOX_WAIT_MS 10

/* Mailbox */
struct mbox_hdr {
	u64 opcode  :8;
	u64 id      :8;
	u64 req_ack :1;
	u64 sizew   :3; /* size in words excluding hdr */
	u64 rsvd    :44;
} _packed;

union mbox_msg {
	u64 words[MBOX_SIZE_WORDS];
	struct {
		struct mbox_hdr hdr;
		u64 data[7];
	} s;
} __packed;

/* HW DESC */
struct hw_desc_ptr {
	union {
		u64 u;
		struct {
			u64 facility_rsvd: 46;
			u64 ptr_type:2; /* direct or indirect */
			u64 ptr_len:16; /* length of this buf */
		} s_generic;
		struct {
			u64 rsvd:29;
			u64 is_frag:1; /* is this part of a packet */
			u64 total_len:16; /* total length of the packet */
			u64 ptr_type:2; /* direct or indirect */
			u64 ptr_len:16; /* length of this buf */
		} s_mgmt_net;
	} hdr;
	u64 ptr; /* hardware address */
} __packed;

struct hw_descq {
	u32 prod_idx;
	u32 cons_idx;
	u32 num_entries;
	u32 buf_size;
	u64 shadow_cons_idx_addr;
	u64 shadow_prod_idx_addr;
	struct hw_desc_ptr desc_arr[];
} __packed;

#define HW_DESC_ARR_ENTRY_OFFSET(i) \
	(sizeof(struct hw_descq) + (i * sizeof(struct hw_desc_ptr)))

static inline u32 circq_add(u32 index, u32 add, u32 mask)
{
	return (index + add) & mask;
}

static inline u32 circq_inc(u32 index, u32 mask)
{
	return circq_add(index, 1, mask);
}

static inline u32 circq_depth(u32 pi, u32 ci, u32 mask)
{
	return (pi - ci) & mask;
}

static inline u32 circq_space(u32 pi, u32 ci, u32 mask)
{
	return mask - circq_depth(pi, ci, mask);
}

struct dpi_instr {
	u64 *compaddr;
	u64 hostaddr;
	u64 localaddr;
	u32 dma_len;
	u16 xfer_dir;
	u16 rsvd;
};

struct dpi_instr_q {
	u16 chunk_size_m1;
	u16 index;
	u32 rsvd;
	u64 instr_buf;
};

struct sw_descq {
	struct hw_descq *hw_descq;
	u32 local_cons_idx;
	u32 cons_idx;
	u32 refill_prod_idx;
	void *priv;
	u32 __iomem *shadow_cons_idx_ioremap_addr;
	u32 mask;
	u32 num_entries;
	u64 *dma_list;
	u64 comp_data;
};

struct dpi_pf {
	struct udevice *dev;
	struct udevice *npa_dev;
	void __iomem *pf_base;
	void __iomem *vf_base;
	u32 tx_mbox_id;
	u32 rx_mbox_id;
	struct sw_descq txqs;
	struct sw_descq rxqs;
	struct dpi_instr_q instrq;
	u8 hw_addr[ETH_ALEN];
};

#endif /* __DPI_PF_H__ */
