/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#ifndef VNIC_H_
#define VNIC_H_

struct nicpf;
struct nicvf;

int nicvf_initialize(struct udevice *dev, int vf_num);
int thunderx_bgx_initialize(unsigned int bgx_idx, unsigned int node);
void bgx_get_count(int node, int *bgx_count);
int bgx_get_lmac_count(int node, int bgx_idx);
void bgx_set_board_info(int bgx_id, int *mdio_bus,
			int *phy_addr, bool *autoneg_dis, bool *lmac_reg, bool *lmac_enable);

#endif /* VNIC_H_ */
