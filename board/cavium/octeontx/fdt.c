/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <environment.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <asm/arch/atf.h>
#include <asm/arch/octeontx.h>

#ifdef CONFIG_OCTEONTX_VNIC
# include <asm/arch/octeontx_vnic.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

struct cavm_bdt g_cavm_bdt;
struct cavm_bdt *p_cavm_bdt;

extern unsigned long fdt_base_addr;

static inline uint64_t cavm_get_model(void) __attribute__ ((pure, always_inline));
static inline uint64_t cavm_get_model(void)
{
#ifdef CAVM_BUILD_HOST
    extern uint32_t thunder_remote_get_model(void) __attribute__ ((pure));
    return thunder_remote_get_model();
#else
    uint64_t result;
    asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (result));
    return result;
#endif
}

void octeontx_parse_board_info(void)
{
	const char *str;
	int node;
	int ret = 0, len = 16;
	u64 midr, val;

	debug("%s: ENTER\n", __func__);

	midr = cavm_get_model();
	g_cavm_bdt.prod_id = (midr >> 4) & 0xff;

	val = readq(CAVM_MIO_FUS_DAT2);
	g_cavm_bdt.alt_pkg = (val >> 22) & 0x3;
	if ((g_cavm_bdt.prod_id == CN81XX) &&
		(g_cavm_bdt.alt_pkg || ((val >> 30) & 0x1)))
		g_cavm_bdt.alt_pkg = 2;

	if (!gd->fdt_blob) {
		printf("ERROR: %s: no valid device tree found\n", __func__);
		return;
	}

	debug("%s: fdt blob at %p\n", __func__, gd->fdt_blob);
	ret = fdt_check_header(gd->fdt_blob);
	if (ret < 0) {
		printf("fdt: %s\n", fdt_strerror(ret));
		return;
	}

	node = fdt_path_offset(gd->fdt_blob, "/cavium,bdk");
	if (node < 0) {
		printf("%s: /cavium,bdk is missing from device tree: %s\n",
		       __func__, fdt_strerror(node));
		return;
	}

	debug("fdt:size %d\n", fdt_totalsize(gd->fdt_blob));
	str = fdt_getprop(gd->fdt_blob, node, "BOARD-MODEL", &len);
	debug("fdt: BOARD-MODEL str %s len %d\n", str, len);
	if (str) {
		strncpy(g_cavm_bdt.type, str, sizeof(g_cavm_bdt.type));
		debug("fdt: BOARD-MODEL bdt.type %s \n", g_cavm_bdt.type);
	} else {
		printf("Error: cannot retrieve board type from fdt\n");
	}
	p_cavm_bdt = &g_cavm_bdt;
}

#ifdef CONFIG_OCTEONTX_BGX
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
#ifdef CONFIG_OCTEONTX_BGX
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

int ethaddr_delete(const char *name)
{
	int flag = H_INTERACTIVE | H_FORCE;

	return hdelete_r(name, &env_htab, flag);
}

int ethaddr_exists(const char *name)
{
	ENTRY e, *ep;

	e.key = name;
	e.data = NULL;
	hsearch_r(e, FIND, &ep, &env_htab, 0);

	return (ep == NULL) ? 1 : 0;
}

void octeontx_parse_mac_addr(void)
{
#ifdef CONFIG_OCTEONTX_BGX
	const char *mac_addr;
	const void *fdt = gd->fdt_blob;
	int subnode, mismatch = 0;
	int len, rgx_id = 0, eth_id = 0;
	int offset = 0, node, bgx_id = 0, lmacid = 0;
	unsigned char eth_addr[ARP_HLEN], env_addr[ARP_HLEN];
	char eaddr[32], bgxname[16], envname[16];
	struct eth_device *ethdev;

	debug("%s: ENTER\n", __func__);
	offset = fdt_node_offset_by_compatible(fdt, -1, "pci-bridge");
	if (offset < 0)
		return;

	for (bgx_id = 0; bgx_id < CONFIG_MAX_BGX_PER_NODE; bgx_id++) {
		snprintf(bgxname, sizeof(bgxname), "bgx%d", bgx_id);
		node = fdt_subnode_offset(fdt, offset, bgxname);
		if (node < 0) {
			/* check if it is rgx node */
			snprintf(bgxname, sizeof(bgxname),
				 "rgx%d", rgx_id);
			node = fdt_subnode_offset(fdt, offset, bgxname);
			if (node < 0) {
				debug("bgx%d/rgx0 node not found\n", bgx_id);
				continue;
			}
		}
		debug("bgx%d node found\n", bgx_id);

		/* loop through each of the bgx/rgx nodes to find PHY nodes */
		fdt_for_each_subnode(subnode, fdt, node) {
			sprintf(envname, eth_id ? "eth%daddr" : "ethaddr",
				eth_id);
			/* check for local-mac-address */
			mac_addr = fdt_getprop(fdt, subnode,
					       "local-mac-address", &len);
			if (mac_addr) {
				debug("%s mac %pM\n", __func__, mac_addr);
				snprintf(eaddr, sizeof(eaddr), "%pM",
					 mac_addr);
				debug("%s eaddr %pM\n", __func__, eaddr);
				memcpy(eth_addr, mac_addr, ARP_HLEN);
				debug("%s eth %pM\n", __func__, eth_addr);
				if (env_get(envname) == NULL)
					mismatch = 1;
				else {
					eth_env_get_enetaddr(envname, env_addr);
					debug("\n env %pM\n", env_addr);
					if (memcmp(env_addr, eth_addr,
						   ARP_HLEN))
						mismatch = 1;
				}
				env_set_force(envname, eaddr);
				ethdev = eth_get_dev_by_index(eth_id);
				if (ethdev &&
					memcmp(ethdev->enetaddr, eth_addr,
						ARP_HLEN))
					memcpy(ethdev->enetaddr, eth_addr,
						ARP_HLEN);
			} else {
				printf("Warning: local-mac-address "
				       "not found for bgx%d lmac%d\n",
				       bgx_id, lmacid);
				if (!ethaddr_exists(envname)) {
					memset(eth_addr, 0, ARP_HLEN);
					eth_addr[5] = eth_id;
					eth_env_get_enetaddr(envname, env_addr);
					if (memcmp(env_addr, eth_addr,
						ARP_HLEN)) {
						mismatch = 1;
						snprintf(eaddr, sizeof(eaddr),
							 "%pM", eth_addr);
						env_set_force(envname, eaddr);
						ethdev = eth_get_dev_by_index(
								eth_id);
						if (ethdev &&
						    memcmp(ethdev->enetaddr,
							eth_addr, ARP_HLEN))
							memcpy(
							ethdev->enetaddr,
							eth_addr, ARP_HLEN);
					}
				}
			}
			eth_id++;
			lmacid++;
		}
		lmacid = 0;
	}

	debug("\n %s eth_id %d ",__func__, eth_id);

	/*
	 * After parsing bgx nodes, remove extra ethaddr env variables
	 * maybe configuration or mac addr count might have changed
	 * use eth_id as base
	 */
	for ( ;eth_id < 20; eth_id++) {
		sprintf(envname, eth_id ? "eth%daddr" : "ethaddr", eth_id);
		if ((!ethaddr_exists(envname)) && (!ethaddr_delete(envname)))
			printf("\n delete env %s failed", envname);
	}
	if (mismatch) {
		printf("Interface configuration or mac addr count"
			" changed, saving env...\n");
		env_save();
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
