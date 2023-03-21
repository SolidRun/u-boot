/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2022 Marvell.
 */

#ifndef __NPA_PF_H__
#define	__NPA_PF_H__

#include "nix.h"
#include "lmt.h"

#define NPA_PF_LF_ID 32
#define NPA_POOL_INST NPA_POOL_RX
#define INST_QLEN SQB_QLEN

struct npa_pf {
	void __iomem	*pf_base;
	struct udevice	*dev;
	struct udevice	*afdev;
	struct npa	*npa;
	int pf_id;
	int lf_id;
};

void *npa_memalloc(int num_elements, size_t elem_size, const char *msg);
u64 npa_pf_aura_op_alloc(struct npa_pf *npa_pf);
#endif /* __NPA_PF_H__ */
