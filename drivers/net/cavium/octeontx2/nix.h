/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __NIX_H__
#define	__NIX_H__

#include "rvu_common.h"
#include "cavm-csrs-nix.h"

/** Maximum number of LMACs supported */
#define MAX_LMAC				12

#define PCI_DEVICE_ID_OCTEONTX2_RVU		0xa063
#define PCI_DEVICE_ID_OCTEONTX2_RVU_SSO_TIM_PF	0xa0f9
#define PCI_DEVICE_ID_OCTEONTX2_RVU_SSO_TIM_VF	0xa0fa
#define PCI_DEVICE_ID_OCTEONTX2_RVU_NPA_PF	0xa0fb
#define PCI_DEVICE_ID_OCTEONTX2_RVU_NPA_VF	0xa0fc
#define PCI_DEVICE_ID_OCTEONTX2_RVU_CPT_PF	0xa0fd
#define PCI_DEVICE_ID_OCTEONTX2_RVU_CPT_VF	0xa0fe

#define NIX_PCI_NPC_FN
#define NIX_PCI_FN

/* NIX RX action operation*/
#define NIX_RX_ACTIONOP_DROP		(0x0ull)
#define NIX_RX_ACTIONOP_UCAST		(0x1ull)
#define NIX_RX_ACTIONOP_UCAST_IPSEC	(0x2ull)
#define NIX_RX_ACTIONOP_MCAST		(0x3ull)
#define NIX_RX_ACTIONOP_RSS		(0x4ull)

/* NIX TX action operation*/
#define NIX_TX_ACTIONOP_DROP		(0x0ull)
#define NIX_TX_ACTIONOP_UCAST_DEFAULT	(0x1ull)
#define NIX_TX_ACTIONOP_UCAST_CHAN	(0x2ull)
#define NIX_TX_ACTIONOP_MCAST		(0x3ull)
#define NIX_TX_ACTIONOP_DROP_VIOL	(0x5ull)

#define NIX_INTF_RX			0
#define NIX_INTF_TX			1

#define NIX_INTF_TYPE_CGX		0
#define NIX_INTF_TYPE_LBK		1
#define NIX_MAX_HW_MTU			9212
#define NIX_MIN_HW_MTU			64

#define NPA_POOL_COUNT			2
#define NPA_AURA_COUNT(x)		(1ULL << ((x) + 6))
#define NPA_POOL_RX			0ULL
#define NPA_POOL_TX			1ULL
#define RQ_QLEN				1024
#define SQ_QLEN				128

#define NIX_CQ_RX			0ULL
#define NIX_CQ_TX			1ULL
#define NIX_CQ_COUNT			2ULL
#define NIX_CQE_SIZE_W16		(16 * sizeof(u64))
#define NIX_CQE_SIZE_W64		(64 * sizeof(u64))

/** Size of aura hardware context */
#define NPA_AURA_HW_CTX_SIZE		48
/** Size of pool hardware context */
#define NPA_POOL_HW_CTX_SIZE		64

#define NPA_DEFAULT_PF_FUNC		0xffff

#define NIX_CHAN_CGX_LMAC_CHX(a, b, c)	(0x800 + 0x100 * (a) + 0x10 * (b) + (c))
#define NIX_LINK_CGX_LMAC(a, b)		(0 + 4 * (a) + (b))
#define NIX_LINK_LBK(a)			(12 + (a))
#define NIX_CHAN_LBK_CHX(a, b)		(0 + 0x100 * (a) + (b))
#define MAX_LMAC_PKIND			12

enum npa_aura_size {
	NPA_AURA_SZ_0,
	NPA_AURA_SZ_128,
	NPA_AURA_SZ_256,
	NPA_AURA_SZ_512,
	NPA_AURA_SZ_1K,
	NPA_AURA_SZ_2K,
	NPA_AURA_SZ_4K,
	NPA_AURA_SZ_8K,
	NPA_AURA_SZ_16K,
	NPA_AURA_SZ_32K,
	NPA_AURA_SZ_64K,
	NPA_AURA_SZ_128K,
	NPA_AURA_SZ_256K,
	NPA_AURA_SZ_512K,
	NPA_AURA_SZ_1M,
	NPA_AURA_SZ_MAX,
};
#define NPA_AURA_SIZE_DEFAULT		NPA_AURA_SZ_128

/* NIX Transmit schedulers */
enum nix_scheduler {
	NIX_TXSCH_LVL_SMQ = 0x0,
	NIX_TXSCH_LVL_MDQ = 0x0,
	NIX_TXSCH_LVL_TL4 = 0x1,
	NIX_TXSCH_LVL_TL3 = 0x2,
	NIX_TXSCH_LVL_TL2 = 0x3,
	NIX_TXSCH_LVL_TL1 = 0x4,
	NIX_TXSCH_LVL_CNT = 0x5,
};

struct cgx;
struct rvu_pf;

struct nix_stats {
	u64	num_packets;
	u64	num_bytes;
};

struct nix_af_handle;

struct nix_txsch {
	struct rsrc_bmap rsrc;
	u8	lvl;
	u16	*pfvf_map;
};

struct nix_handle;
struct cgx;
struct lmac;

struct npa_af_handle {
	void __iomem		*npa_base;
	struct admin_queue	aq;
	u32			aura;
};

struct npa_handle {
	struct npa_af_handle	*npa_af;
	void __iomem		*npa_base;
	void __iomem		*npc_base;
	void __iomem		*lmt_base;
	/** Hardware aura context */
	void			*aura_ctx;
	/** Hardware pool context */
	void			*pool_ctx[NPA_POOL_COUNT];
	void			*pool_stack[NPA_POOL_COUNT];
	void			**rx_buffers;
	void			**tx_buffers;
	u32			rx_pool_stack_pages;
	u32			tx_pool_stack_pages;
	u32			pool_stack_pointers;
	u32			q_len[NPA_POOL_COUNT];
	u32			buf_size[NPA_POOL_COUNT];
	u32			stack_pages[NPA_POOL_COUNT];
};

struct nix_af_handle {
	struct udevice			*dev;
	struct list_head		nix_af_list;
	struct nix_handle		*lmacs[MAX_LMAC];
	struct npa_af_handle		*npa_af;
	void __iomem			*nix_af_base;
	void __iomem			*npc_af_base;
	struct admin_queue		aq;
	u8				num_lmacs;
	s8				index;
	u8				xqe_size;
};

struct nix_tx_descr {
	union cavm_nix_send_hdr_s	hdr;
	union cavm_nix_send_sg_s	segments;
	dma_addr_t			dev_addr;
	void				*host_addr;
};

struct nix_rx_descr {
	union cavm_nix_cqe_hdr_s hdr;
	union cavm_nix_rx_parse_s rx_parse;
	union cavm_nix_rx_sg_s rx_sg;
};

struct nix_handle {
	struct udevice			*dev;
	struct eth_device		*netdev;
	struct list_head		nix_list;
	struct nix_af_handle		*nix_af;
	struct npa_handle		*npa;
	struct lmac			*lmac;
	union cavm_nix_cint_hw_s	*cint_base;
	union cavm_nix_cq_ctx_s		*cq_ctx_base;
	union cavm_nix_qint_hw_s	*qint_base;
	union cavm_nix_rq_ctx_s		*rq_ctx_base;
	union cavm_nix_rsse_s		*rss_base;
	union cavm_nix_sq_ctx_s		*sq_ctx_base;
	void				*cqe_base;
	struct qmem			sq;
	struct qmem			cq[NIX_CQ_COUNT];
	struct qmem			rq;
	struct qmem			rss;
	struct qmem			cq_ints;
	struct qmem			qints;
	char				name[16];
	void __iomem			*nix_base;	/** PF reg base */
	void __iomem			*npc_base;
	void __iomem			*lmt_base;
	struct nix_tx_descr		send_descriptors[SQ_QLEN];
	struct nix_tx_descr		*free_send_descriptors[SQ_QLEN];
	u32				current_free_send_descriptor;
	struct nix_stats		tx_stats;
	struct nix_stats		rx_stats;
	u32				aura;
	int				pknd;
	u16				pki_channel;
	u16				pki_dstat;
	u16				pko_queue;
	u16				nic_id;
	int				lf;
	int				pf;
	int				rq_idx;
	int				sq_idx;
	int				cq_idx;
};

struct nix_lf_alloc_req {
	u32	rq_cnt;		/** Number of receive queues */
	u32	sq_cnt;		/** Number of send squeues */
	u32	cq_cnt;		/** Number of completion queues */
	u16	rss_sz;
	u8	rss_grps;
	u8	xqe_sz;
	u16	npa_func;
};

struct nix_lf_alloc_rsp {
	u16	sqb_size;
	u16	chan_base;
	u8	chan_cnt;
#if 0
	u8	lso_tsov4_idx;
	u8	lso_tsov6_idx;
#endif
	u8	mac_addr[6];
};

static inline u64 nix_af_reg_read(struct nix_af_handle *nix_af, u64 offset)
{
	return readq(nix_af->nix_af_base + offset);
}

static inline void nix_af_reg_write(struct nix_af_handle *nix_af, u64 offset,
				    u64 val)
{
	writeq(val, nix_af->nix_af_base + offset);
}

static inline u64 nix_pf_reg_read(struct nix_handle *nix, u64 offset)
{
	return readq(nix->nix_base + offset);
}

static inline void nix_pf_reg_write(struct nix_handle *nix, u64 offset,
				    u64 val)
{
	writeq(val, nix->nix_base + offset);
}

static inline u64 npa_af_reg_read(struct npa_af_handle *npa_af, u64 offset)
{
	return readq(npa_af->npa_base + offset);
}

static inline void npa_af_reg_write(struct npa_af_handle *npa_af, u64 offset,
				    u64 val)
{
	writeq(val, npa_af->npa_base + offset);
}

static inline u64 npc_af_reg_read(struct nix_af_handle *nix_af, u64 offset)
{
	return readq(nix_af->npc_af_base + offset);
}

static inline void npc_af_reg_write(struct nix_af_handle *nix_af, u64 offset,
				    u64 val)
{
	writeq(val, nix_af->npc_af_base + offset);
}

struct nix_af_handle *nix_af_initialize(int instance, struct udevice *dev,
					void *bar0_ptr, void *bar2_ptr,
					void *npa_bar0_ptr);
int npa_lf_admin_setup(struct nix_af_handle *nix_af, int lf,
		       u32 aura_size,
		       const union cavm_npa_aura_s *aura_ctx,
		       dma_addr_t auras_dev_addr,
		       const union cavm_npa_pool_s *pool_ctx,
		       u32 pool_cnt);

int npa_lf_admin_shutdown(struct nix_af_handle *nix_af, int lf, u32 pool_count);

int npc_lf_admin_setup(struct nix_af_handle *nix_af, struct cgx *cgx,
		       u64 link_num);

int nix_lf_admin_setup(struct nix_af_handle *nix_af, int lf, int pf,
		       union cavm_nix_cq_ctx_s *cq_descriptors,
		       dma_addr_t cq_dev_addr,
		       u32 cq_count,
		       union cavm_nix_rq_ctx_s *rq_descriptors,
		       dma_addr_t rq_dev_addr,
		       u32 rq_count,
		       union cavm_nix_sq_ctx_s *sq_descriptors,
		       dma_addr_t sq_dev_addr,
		       u32 sq_count);
int nix_lf_admin_shutdown(struct nix_af_handle *nix_af, int lf,
			  u32 cq_count, u32 rq_count, u32 sq_count);
struct nix_handle *nix_get_pdata(int nic_id);
int nix_get_nix_cnt(void);
int nix_get_pf_num(const struct nix_handle *nix);
int nix_linear_link_number(const struct nix_handle *nix);
struct nix_af_handle *nix_get_af(u64 nix_pf_base);
struct nix_handle *cavm_nix_lf_alloc(struct nix_af_handle *nix_af,
				     struct udevice *dev,
				     u16 pcifunc,
				     u16 nix_lf,
				     void __iomem *nix_base,
				     void __iomem *npc_base,
				     void __iomem *lmt_base,
				     int cgx_id, int lmac_id,
				     struct nix_lf_alloc_req *req,
				     struct nix_lf_alloc_rsp *rsp);

#endif /* __NIX_H__ */
