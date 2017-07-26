/*
 * Copyright (C) 2017 NXP Semiconductors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DRIVER_NVME_H__
#define __DRIVER_NVME_H__

#include <pci.h>
#include <asm/io.h>
#include "nvme_uapi.h"
#include <linux/list.h>
#include <nvme.h>
extern struct nvme_info *nvme_info;
struct nvme_bar {
	__u64			cap;	/* Controller Capabilities */
	__u32			vs;	/* Version */
	__u32			intms;	/* Interrupt Mask Set */
	__u32			intmc;	/* Interrupt Mask Clear */
	__u32			cc;	/* Controller Configuration */
	__u32			rsvd1;	/* Reserved */
	__u32			csts;	/* Controller Status */
	__u32			rsvd2;	/* Reserved */
	__u32			aqa;	/* Admin Queue Attributes */
	__u64			asq;	/* Admin SQ Base Address */
	__u64			acq;	/* Admin CQ Base Address */
};

#define NVME_CAP_MQES(cap)	((cap) & 0xffff)
#define NVME_CAP_TIMEOUT(cap)	(((cap) >> 24) & 0xff)
#define NVME_CAP_STRIDE(cap)	(((cap) >> 32) & 0xf)
#define NVME_CAP_MPSMIN(cap)	(((cap) >> 48) & 0xf)
#define NVME_CAP_MPSMAX(cap)	(((cap) >> 52) & 0xf)

enum {
	NVME_CC_ENABLE		= 1 << 0,
	NVME_CC_CSS_NVM		= 0 << 4,
	NVME_CC_MPS_SHIFT	= 7,
	NVME_CC_ARB_RR		= 0 << 11,
	NVME_CC_ARB_WRRU	= 1 << 11,
	NVME_CC_ARB_VS		= 7 << 11,
	NVME_CC_SHN_NONE	= 0 << 14,
	NVME_CC_SHN_NORMAL	= 1 << 14,
	NVME_CC_SHN_ABRUPT	= 2 << 14,
	NVME_CC_SHN_MASK	= 3 << 14,
	NVME_CC_IOSQES		= 6 << 16,
	NVME_CC_IOCQES		= 4 << 20,
	NVME_CSTS_RDY		= 1 << 0,
	NVME_CSTS_CFS		= 1 << 1,
	NVME_CSTS_SHST_NORMAL	= 0 << 2,
	NVME_CSTS_SHST_OCCUR	= 1 << 2,
	NVME_CSTS_SHST_CMPLT	= 2 << 2,
	NVME_CSTS_SHST_MASK	= 3 << 2,
};

#define NVME_IO_TIMEOUT  30

/*
 * Represents an NVM Express device.  Each nvme_dev is a PCI function.
 */
struct nvme_dev {
	struct list_head node;
	struct nvme_queue **queues;
	u32 __iomem *dbs;
	unsigned int cardnum;
	struct udevice *pdev;
	pci_dev_t pci_dev;
	int instance;
	uint8_t *hw_addr;
	unsigned queue_count;
	unsigned online_queues;
	unsigned max_qid;
	int q_depth;
	u32 db_stride;
	u32 ctrl_config;
	struct nvme_bar __iomem *bar;
	struct list_head namespaces;
	const char *name;
	char serial[20];
	char model[40];
	char firmware_rev[8];
	u32 max_transfer_shift;
	u32 stripe_size;
	u32 page_size;
	u16 oncs;
	u16 abort_limit;
	u8 event_limit;
	u8 vwc;
	u64 *prp_pool;
	u32 prp_entry_num;
	u32 nn;
	u32 blk_dev_start;
};

struct nvme_info {
	int ns_num;	/*the number of nvme namespaces*/
	int ndev_num;	/*the number of nvme devices*/
	struct list_head dev_list;
};

/*
 * The nvme_iod describes the data in an I/O, including the list of PRP
 * entries.  You can't see it in this data structure because C doesn't let
 * me express that.  Use nvme_alloc_iod to ensure there's enough space
 * allocated to store the PRP list.
 */
struct nvme_iod {
	unsigned long private;	/* For the use of the submitter of the I/O */
	int npages;		/* In the PRP list. 0 means small pool in use */
	int offset;		/* Of PRP list */
	int nents;		/* Used in scatterlist */
	int length;		/* Of data, in bytes */
	dma_addr_t first_dma;
};

/*
 * An NVM Express namespace is equivalent to a SCSI LUN
 * Each namespace is operated as an independent "device".
 */
struct nvme_ns {
	struct list_head list;
	struct nvme_dev *dev;
	unsigned ns_id;
	int devnum;
	int lba_shift;
	u16 ms;
	u8 flbas;
	u8 pi_type;
	u64 mode_select_num_blocks;
	u32 mode_select_block_len;
};
#endif
