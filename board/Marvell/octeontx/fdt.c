/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <environment.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <asm/arch/atf.h>
#include <asm/arch/octeontx.h>
#include <asm/arch/octeontx_vnic.h>

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long fdt_base_addr;

#ifdef CONFIG_NET_OCTEONTX
static int octeontx_get_mdio_bus(const void *fdt, int phy_offset)
{
	int node, bus = -1;
	const u64 *reg;
	u64 addr;

	if (phy_offset < 0)
		return -1;
	/* obtain mdio node and get the reg prop */
	node = fdt_parent_offset(fdt, phy_offset);
	if (node < 0)
		return -1;

	reg = fdt_getprop(fdt, node, "reg", NULL);
	addr = fdt64_to_cpu(*reg);
	bus = (addr & (1 << 7)) ? 1 : 0;
	return bus;
}

static int octeontx_get_phy_addr(const void *fdt, int phy_offset)
{
	const u32 *reg;
	int addr = -1;

	if (phy_offset < 0)
		return -1;
	reg = fdt_getprop(fdt, phy_offset, "reg", NULL);
	addr = fdt32_to_cpu(*reg);
	return addr;
}
#endif

void octeontx_parse_phy_info(void)
{
#ifdef CONFIG_NET_OCTEONTX
	const void *fdt = gd->fdt_blob;
	int offset = 0, node, bgx_id = 0, lmacid = 0;
	const u32 *val;
	char bgxname[24];
	int len, rgx_id = 0, eth_id = 0;
	int phandle, phy_offset;
	int subnode, i;

	int bgxnode;
	bgxnode = fdt_path_offset(gd->fdt_blob, "/cavium,bdk");
	if (bgxnode < 0) {
		printf("%s: /cavium,bdk is missing from device tree: %s\n",
			__func__, fdt_strerror(bgxnode));
	}

	offset = fdt_node_offset_by_compatible(fdt, -1, "pci-bridge");
	if (offset > 1) {
		for (bgx_id = 0; bgx_id < CONFIG_MAX_BGX_PER_NODE; bgx_id++) {
			int phy_addr[MAX_LMAC_PER_BGX] = { [0 ... MAX_LMAC_PER_BGX - 1] = -1};
			bool autoneg_dis[MAX_LMAC_PER_BGX] = { [0 ... MAX_LMAC_PER_BGX - 1] = 0};
			int mdio_bus[MAX_LMAC_PER_BGX] = { [0 ... MAX_LMAC_PER_BGX - 1] = -1};
			bool lmac_reg[MAX_LMAC_PER_BGX] = { [0 ... MAX_LMAC_PER_BGX - 1] = 0};
			bool lmac_enable[MAX_LMAC_PER_BGX] = { [0 ... MAX_LMAC_PER_BGX - 1] = 0};
			snprintf(bgxname, sizeof(bgxname),
				 "bgx%d", bgx_id);
			node = fdt_subnode_offset(fdt, offset, bgxname);
			if (node < 0) {
				/* check if it is rgx node */
				snprintf(bgxname, sizeof(bgxname),
					 "rgx%d", rgx_id);
				node = fdt_subnode_offset(fdt, offset, bgxname);
				if (node < 0) {
					debug("bgx%d/rgx0 node not found\n",
					       bgx_id);
					return;
				}
			}
			debug("bgx%d node found\n", bgx_id);

			/* loop through each of the bgx/rgx nodes
			to find PHY nodes */
			fdt_for_each_subnode(subnode, fdt, node) {
				/* Check for reg property */
				val = fdt_getprop(fdt, subnode, "reg",
						  &len);

				if (val) {
					debug("lmacid = %d\n", lmacid);
					lmac_reg[lmacid] = 1;
				}
				/* check for phy-handle property */
				val = fdt_getprop(fdt, subnode, "phy-handle",
						  &len);
				if (val) {
					phandle = fdt32_to_cpu(*val);
					if (!phandle) {
						debug("phandle not valid %d\n",
						      lmacid);
					} else {
						phy_offset =
						fdt_node_offset_by_phandle
							(fdt, phandle);
						phy_addr[lmacid] =
						octeontx_get_phy_addr
							(fdt, phy_offset);

						mdio_bus[lmacid] =
						octeontx_get_mdio_bus
							(fdt, phy_offset);
					}
				} else
					debug("phy-handle property not found %d\n",
					      lmacid);

				/* check for autonegotiation property */
				val = fdt_getprop(fdt, subnode,
						  "cavium,disable-autonegotiation",
						  &len);
				if (val)
					autoneg_dis[lmacid] = 1;

				eth_id++;
				lmacid++;
			}

			for (i = 0; i < MAX_LMAC_PER_BGX; i++) {
				const char *str;
				snprintf(bgxname, sizeof(bgxname), "BGX-ENABLE.N0.BGX%d.P%d", bgx_id, i);
				if (bgxnode >= 0) {
					str = fdt_getprop(gd->fdt_blob, bgxnode, bgxname, &len);
					if (str)
						lmac_enable[i] = simple_strtol(str, NULL, 10);
				}
			}

			lmacid = 0;
			bgx_set_board_info(bgx_id, mdio_bus, phy_addr,
					   autoneg_dis, lmac_reg, lmac_enable);
		}
	}
#endif
}

int arch_fixup_memory_node(void *blob)
{
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	/* remove "cavium, bdk" node from DT */
	int ret = 0, offset;

	ret = fdt_check_header(blob);
	if (ret < 0) {
		printf("ERROR: %s\n", fdt_strerror(ret));
		return ret;
	}

	if (blob != NULL) {
		offset = fdt_path_offset(blob, "/cavium,bdk");
		if(offset < 0) {
			printf("ERROR: FDT BDK node not found\n");
			return offset;
		}

		/* delete node */
		ret = fdt_del_node(blob, offset);
		if (ret < 0) {
			printf("WARNING : could not remove cavium, bdk node\n");
			return ret;
		}

		debug("%s deleted 'cavium,bdk' node\n", __FUNCTION__);
	}

	return 0;
}

/**
 * Return the FDT base address that was passed by ATF
 *
 * @return	FDT base address received from ATF in x1 register
 */
void *board_fdt_blob_setup(void)
{
	return (void *)fdt_base_addr;
}
