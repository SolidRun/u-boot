/*
 * Copyright (C) 2017 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 */

#ifndef __RVU_COMMON_H__
#define __RVU_COMMON_H__

#define ALIGNED		__aligned(CONFIG_SYS_CACHELINE_SIZE)

/* PCI device IDs */
#define	PCI_DEVID_OCTEONTX2_CGX			0xA059
#define	PCI_DEVID_OCTEONTX2_RVU_AF		0xA065
#define	PCI_DEVID_OCTEONTX2_RVU_PF		0xA063
#define	PCI_DEVID_OCTEONTX2_RVU_VF		0xA064

/* PCI BAR nos */
#define	PCI_AF_REG_BAR_NUM			0
#define	PCI_CFG_REG_BAR_NUM			2
#define	PCI_MBOX_BAR_NUM			4

#define RVU_PFVF_PF_SHIFT	10
#define RVU_PFVF_PF_MASK	0x3F
#define RVU_PFVF_FUNC_SHIFT	0
#define RVU_PFVF_FUNC_MASK	0x3FF

#define MAX_NIX			1

#define NAME_SIZE				32

#define OTX2_ALIGN	CONFIG_SYS_CACHELINE_SIZE  /* Align to cacheline */

#define Q_SIZE_16		0ULL /* 16 entries */
#define Q_SIZE_64		1ULL /* 64 entries */
#define Q_SIZE_256		2ULL
#define Q_SIZE_1K		3ULL
#define Q_SIZE_4K		4ULL
#define Q_SIZE_16K		5ULL
#define Q_SIZE_64K		6ULL
#define Q_SIZE_256K		7ULL
#define Q_SIZE_1M		8ULL /* Million entries */
#define Q_SIZE_MIN		Q_SIZE_16
#define Q_SIZE_MAX		Q_SIZE_1M

#define Q_COUNT(x)		(16ULL << (2 * x))
#define Q_SIZE(x, n)		((ilog2(x) - (n)) / 2)

#define NIX_INTF_TYPE_CGX		0
#define NIX_INTF_TYPE_LBK		1
#define NIX_MAX_HW_MTU			9212
#define NIX_MIN_HW_MTU			64

#define MAX_LMAC_PKIND			12

/* Admin queue info */

/* Since we intend to add only one instruction at a time,
 * keep queue size to it's minimum.
 */
#define AQ_SIZE			Q_SIZE_16
/* HW head & tail pointer mask */
#define AQ_PTR_MASK		0xFFFFF

#define RQ_LEN		QCOUNT(Q_SIZE_1K)
#define SQ_LEN		QCOUNT(Q_SIZE_16)

/** RVU Block Type Enumeration */
enum rvu_block_type_e {
	BLKTYPE_RVUM = 0x0,
	BLKTYPE_MSIX = 0x1,
	BLKTYPE_LMT  = 0x2,
	BLKTYPE_NIX  = 0x3,
	BLKTYPE_NPA  = 0x4,
	BLKTYPE_NPC  = 0x5,
	BLKTYPE_SSO  = 0x6,
	BLKTYPE_SSOW = 0x7,
	BLKTYPE_TIM  = 0x8,
	BLKTYPE_CPT  = 0x9,
	BLKTYPE_NDC  = 0xa,
	BLKTYPE_MAX  = 0xa,
};


/** Resource bitmap */
struct rsrc_bmap {
	unsigned long *bmap;
	u16  max;
};

struct rvu_block {
	struct rsrc_bmap	rsrc;
	u16			*fn_map;	/** LF to pcifunc mapping */
	bool			multislot;
	u8			blkid;
	u8			addr;
	u8			lfshift;
	u64			lookup_reg;
	u64			pf_lfcnt_reg;
	u64			vf_lfcnt_reg;
	u64			lfcfg_reg;
	u64			msixcfg_reg;
	u64			lfreset_reg;
	unsigned char		name[32];
};

struct rvu_hwinfo {
	struct rvu_block block[BLKTYPE_MAX];
	u8	total_pfs;	/** MAX RVU PFs HW supports */
	u8	total_vfs;	/** Max RVU VFs HW supports */
	u16	max_vfs_per_pf;	/** Max VFs that can be attached to PF */
	u8	ndc;		/** Number of cache units available */
	u8	cgx;
	u8	lmac_per_cgx;
	u8	cgx_links;
	u8	lbk_links;
	u8	sdp_links;
	u8	npc_kpus;
	u16	sso_hwgrps;
	u16	sso_xaq_num_works;
	u16	sso_xaq_buf_size;

	u8	sso_hws;
};

struct qmem {
	void		*base;
	dma_addr_t	iova;
	size_t		alloc_sz;
	u32		qsize;
	u8		entry_sz;
};

struct admin_queue {
	struct qmem inst;
	struct qmem res;
};

/**
 * Store 128 bit value
 *
 * @param[out]	dest	pointer to destination address
 * @param	val0	first 64 bits to write
 * @param	val1	second 64 bits to write
 */
static inline void cavm_st128(void *dest, u64 val0, u64 val1)
{
	__asm__ __volatile__(
		"stp %x[x0], %x[x1], [%[pm]]"
		:
		: [x0]"r"(val0), [x1]"r"(val1), [pm]"r"(dest)
		: "memory");
}

/**
 * Load 128 bit value
 *
 * @param[in]	source		pointer to 128 bits of data to load
 * @param[out]	val0		first 64 bits of data
 * @param[out]	val1		second 64 bits of data
 */
static inline void cavm_ld128(const u64 *src, u64 *val0, u64 *val1)
{
	__asm__ __volatile__ (
		"ldp %x[x0], %x[x1], [%[pm]]"
		:
		: [x0]"r"(*val0), [x1]"r"(*val1), [pm]"r"(src));
}

void qmem_free(struct qmem *q);
int qmem_alloc(struct qmem *q, u32 qsize, size_t entry_sz);

/**
 * Allocates an admin queue for instructions and results
 *
 * @param	aq	admin queue to allocate for
 * @param	qsize	Number of entries in the queue
 * @param	inst_size	Size of each instruction
 * @param	res_size	Size of each result
 *
 * @return	-ENOMEM on error, 0 on success
 */
int cavm_rvu_aq_alloc(struct admin_queue *aq, unsigned qsize,
		      size_t inst_size, size_t res_size);

/**
 * Frees an admin queue
 *
 * @param	aq	Admin queue to free
 */
void cavm_rvu_aq_free(struct admin_queue *aq);

static inline uint32_t rvu_get_pf(u16 pcifunc)
{
	return (pcifunc >> RVU_PFVF_PF_SHIFT) & RVU_PFVF_PF_MASK;
}

int rvu_alloc_bitmap(struct rsrc_bmap *rsrc);

#endif /* __RVU_COMMON_H__ */
