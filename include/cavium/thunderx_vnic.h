/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#ifndef VNIC_H_
#define VNIC_H_

struct nicpf;
struct nicvf;

struct nicpf *nic_initialize(unsigned int node);
int nicvf_initialize(struct nicpf *, int vf_num, unsigned int node);
int bgx_initialize(unsigned int bgx_idx,
		   unsigned int smi_idx, unsigned int node);

void bgx_get_count(int node, int *bgx_count);
int bgx_get_lmac_count(int node, int bgx_idx);



#endif /* VNIC_H_ */
