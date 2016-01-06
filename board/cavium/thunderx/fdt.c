/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <linux/compiler.h>

#include <libfdt.h>
#include <fdt_support.h>
#include <cavium/atf.h>

#if defined(CONFIG_OF_LIBFDT)
#include <cavm-csr.h>

#ifdef CONFIG_THUNDERX_BGX
/**
 * To remove unwanted nodes from fdt .
 *
 *  @param fdt_key - key to preserve.
 *  fdt_key of formate < bgx, qlm-type >.
 *  All non-matching keys are removed
 *
 * */
enum lmac_type {
	BGX_MODE_SGMII = 0, /* 1 lane, 1.250 Gbaud */
	BGX_MODE_XAUI = 1,  /* 4 lanes, 3.125 Gbaud */
	BGX_MODE_DXAUI = 1, /* 4 lanes, 6.250 Gbaud */
	BGX_MODE_RXAUI = 2, /* 2 lanes, 6.250 Gbaud */
	BGX_MODE_XFI = 3,   /* 1 lane, 10.3125 Gbaud */
	BGX_MODE_XLAUI = 4, /* 4 lanes, 10.3125 Gbaud */
	BGX_MODE_10G_KR = 3,/* 1 lane, 10.3125 Gbaud */
	BGX_MODE_40G_KR = 4,/* 4 lanes, 10.3125 Gbaud */
};

struct mac_range {
	unsigned char	enetaddr[6];
	uint32_t		size;
} __attribute__((packed));


int arch_fixup_memory_node(void *blob)
{
	int err, nodeoffset;
	u64 tmp[2], start, size;
	char nodename[64];
	int node;

	err = fdt_check_header(blob);
	if (err < 0) {
		printf("%s: %s\n", __FUNCTION__, fdt_strerror(err));
		return err;
	}

	for (node = 0; node < atf_node_count(); node++) {
		/* update, or add and update /memory node */
		snprintf(nodename, sizeof(nodename), "/memory@%.8lx",
			 (phys_addr_t)node << 40);
		nodeoffset = fdt_path_offset(blob, nodename);

		if (nodeoffset < 0) {
			nodeoffset = fdt_add_subnode(blob, 0, nodename);
			if (nodeoffset < 0) {
				printf("WARNING: could not create %s: %s.\n",
						nodename,
						fdt_strerror(nodeoffset));
				return nodeoffset;
			}
		}
		err = fdt_setprop(blob, nodeoffset, "device_type", "memory",
				  sizeof("memory"));
		if (err < 0) {
			printf("WARNING: could not set %s %s.\n",
					"device_type",
					fdt_strerror(err));
			return err;
		}

		start = (phys_addr_t)node << 40;
		size = atf_dram_size(node);

		if (node == 0) {
			start += MEM_BASE;
			size -= MEM_BASE;
		}

		tmp[0] = cpu_to_be64(start);
		tmp[1] = cpu_to_be64(size);

		err = fdt_setprop(blob, nodeoffset, "reg", tmp, sizeof(tmp));
		if (err < 0) {
			printf("WARNING: could not set %s %s.\n",
					"reg", fdt_strerror(err));
			return err;
		}
	}

	return 0;
}

static void ft_del_phy(void *fdt, int offset)
{
	int len, phy_offset;
	const fdt32_t *php;
	uint32_t phandle;

	php = fdt_getprop(fdt, offset, "phy-handle", &len);

	if (php && len == sizeof(*php)) {
		phandle = fdt32_to_cpu(*php);
		fdt_nop_property(fdt, offset, "phy-handle");
		phy_offset = fdt_node_offset_by_phandle(fdt, phandle);
		if (phy_offset > 0)
			fdt_nop_node(fdt, phy_offset);
	}
}

static void ft_setup_bgx(char *fdt, unsigned int node, unsigned int bgx_id)
{
	union bgxx_cmrx_config cmrx_config;
	union bgxx_spux_br_pmd_control spux_br_pmd_control;
	char fdt_key[20];
	int parent, offset, next_offset = 0, rc = 0, err;
	int qlm_key_len, qlm_mode;
	char bgx_node[12], qlm[32];
	const char *fdt_node_name;

	if (!fdt) {
		printf("%s: Invalid device tree\n", __func__);
		return;
	}

	err = fdt_check_header(fdt);
	if (err < 0) {
		printf("%s: %s\n", __FUNCTION__, fdt_strerror(err));
		return;
	}

	/* Read LMAC0 type to figure out QLM mode
	 * This is configured by low level firmware
	 */

	cmrx_config.u = CSR_READ_PA(node, BGXX_CMRX_CONFIG(bgx_id, 0));
	spux_br_pmd_control.u  = CSR_READ_PA(node, BGXX_SPUX_BR_PMD_CONTROL(bgx_id, 0));

	qlm_mode = cmrx_config.s.lmac_type;

	switch(qlm_mode) {
	case BGX_MODE_SGMII:
		snprintf(fdt_key, sizeof(fdt_key), "%d,sgmii", bgx_id);
		break;
	case BGX_MODE_XAUI:
		snprintf(fdt_key, sizeof(fdt_key), "%d,xaui", bgx_id);
		break;
	case BGX_MODE_RXAUI:
		snprintf(fdt_key, sizeof(fdt_key), "%d,rxaui", bgx_id);
		break;
	case BGX_MODE_XFI:
		if (!!spux_br_pmd_control.s.train_en) {
			snprintf(fdt_key, sizeof(fdt_key), "%d,xfi", bgx_id);
		} else {
			snprintf(fdt_key, sizeof(fdt_key), "%d,xfi-10g-kr", bgx_id);
		}
		break;
	case BGX_MODE_XLAUI:
		if (!spux_br_pmd_control.s.train_en) {
			snprintf(fdt_key, sizeof(fdt_key), "%d,xlaui", bgx_id);
		} else {
			snprintf(fdt_key, sizeof(fdt_key), "%d,xlaui-40g-kr", bgx_id);
		}
		break;
	}

	strncpy(qlm, fdt_key, sizeof(qlm));
	qlm_key_len = strlen(qlm);

	sprintf(bgx_node, "bgx%d", bgx_id);
	for (parent = 0; parent >= 0; parent = fdt_next_node(fdt, parent, NULL)) {
                fdt_node_name = fdt_get_name(fdt, parent, &err);
                if (err < 0) {
			printf("WARNING : Couldn't find BGX node\n");
			return;
                }
                if (strcmp(bgx_node, fdt_node_name) == 0) {
                        break;
                }       
        }


        offset= fdt_first_subnode(fdt,parent);
	while (offset >= 0) {
		int len;
		const char *val;

		if (!rc)
			next_offset = fdt_next_subnode(fdt, offset);

		val = fdt_getprop(fdt, offset, "qlm-mode", &len);

		if (!val)
			break;

		if (!strncmp(val, qlm, qlm_key_len)) {
			offset = next_offset;
			rc = 0;
		} else {
			ft_del_phy(fdt,offset);
			/* fdt_del_node puts the offset to next
			 * node, so no need to get next node
			 * while deleting.
			 */
			fdt_del_node(fdt, offset);
			rc = 1;
		}
	}
}

#define VNIC_PER_NODE 4

static void ft_setup_macs(void *fdt, unsigned int node, unsigned int bgxnum)
{
	int bgxnode, addr, phynode, len;
	char bgxname[25];
	const char *fdt_node_name;
	uint8_t mac[VNIC_PER_NODE][6];
	int mac_set[VNIC_PER_NODE];

	/* Look up the bgx<n> value in the dt. */
	snprintf(bgxname, sizeof(bgxname), "bgx%d", bgxnum);
        
	for (bgxnode = 0; bgxnode >= 0; bgxnode = 
			fdt_next_node(fdt, bgxnode, NULL)) {
		fdt_node_name = fdt_get_name(fdt, bgxnode, &len);
		if (len < 0) {
			printf("WARNING : couldn't find node with offset %d\n",
				bgxnode);
			return;
		}
		if (strcmp(bgxname, fdt_node_name) == 0)
			break;
	}
	
	if (bgxnode < 0) {
		printf("WARNING: could not find %s: %s.\n", bgxname,
					fdt_strerror(bgxnode));
		return;
	}

	/* Fetch all the ethernet addresses for this node. */
	for (addr = 0; addr < VNIC_PER_NODE; addr++) {
                int taddr = addr + (bgxnum * VNIC_PER_NODE);

                mac_set[addr] = 1;
                if (!eth_getenv_enetaddr_by_index("eth", taddr, mac[addr])) {
                        mac_set[addr] = 0;
                        printf("WARNING: failed to get eth addr %d.\n", taddr);
                        continue;
                }
        }
	
	/* Now for each phy in the node, set it's ethernet address. */
	fdt_for_each_subnode(fdt, phynode, bgxnode) {
		int phynum, len, ret;
		const char *phyname;
		char *end, strnum[2];
		
		phyname = fdt_get_name(fdt, phynode, &len);
		if (!phyname) {
			printf("WARNING: No phy name for node in %s.\n",
				bgxname);
			continue;
		}

		if (len < 2) {
			printf("WARNING: phy name too short for %s:%s.\n",
				bgxname, phyname);
			continue;
		}
		
		if (!fdt_getprop(fdt, phynode, "phy-handle", NULL))
			continue;
		
		if (fdt_getprop(fdt, phynode, "mac-address", NULL))
			/* Address already set. */
			continue;

		/* phyname=typename+qlm+typenum. consider only
  		the type number from phyname for phynum */
		
		strnum[0] = phyname[len - 1];
		strnum[1] = '\0';

		phynum = simple_strtol(strnum, &end, 10);
		if (*end  != '\0') {
			printf("WARNING: "
				"phy for %s doesn't end in a 2 digit "
				"number: %s.\n", bgxname, phyname);
				continue;
		}
		
		if (phynum >= VNIC_PER_NODE) {
			printf("WARNING: "
				"Too many phys for %s, maximum %d "
				"supported: %s.\n", bgxname, VNIC_PER_NODE,
				phyname);
			continue;
		}
	
		
		if (!mac_set[phynum])
			continue;
		
		ret = fdt_appendprop(fdt, phynode, "mac-address",
				mac[phynum], 6);
		if (ret < 0) {
			printf("WARNING: "
				"could not add mac address to %s:%s: %s\n",
				bgxname, phyname, fdt_strerror(ret));
			return;
		} 
	}
}

#endif

static void ft_setup_coremask(void *fdt)
{
	int err;

	int nodeoffset = fdt_path_offset(fdt, "/cpus");
	char *coremask = getenv("coremask");

	if (coremask == NULL)
		return;

	if (nodeoffset < 0) {
		printf("WARNING: could not find /cpus: %s.\n",
		       fdt_strerror(nodeoffset));
		return;
	}

	err = fdt_setprop(fdt, nodeoffset, "coremask",
			  coremask, sizeof(coremask));

	if (err < 0) {
		printf("WARNING: could not set %s: %s.\n", "coremask",
		       fdt_strerror(err));
		return;
	}
}
#if defined(CONFIG_OF_BOARD_SETUP)

static void ft_setup_uaa_clk(char *fdt)
{
	unsigned int ibrd, fbrd;
	unsigned long baud = CONFIG_BAUDRATE;
	uint32_t refclk;
	int nodeoffset, err;


	ibrd = CSR_READ_PA(0, UAAX_IBRD(0));
	ibrd = ibrd & 0xffff;

	fbrd = CSR_READ_PA(0, UAAX_FBRD(0));
	fbrd = fbrd & 0x3f;

	refclk = (baud * (64 * ibrd + fbrd)) / 4;

	nodeoffset = fdt_path_offset(fdt, "/soc/refclkuaa");

	err = fdt_setprop_u32(fdt, nodeoffset, "clock-frequency", refclk);

	if(err < 0) {
		printf("WARNING: could not set %s: %s.\n", "clock-frequency",
				fdt_strerror(err));
		return;
	}
}

extern int rc_is_ready(unsigned int rc);

static void ft_setup_pems(char *fdt)
{
	unsigned int pem;
	char name[32];
	const char *nodename;
	int node, len, res;

	for (pem = 0; pem < CONFIG_THUNDERX_RCS; pem++) {
		snprintf(name, sizeof(name), "/soc/pem%u", pem);
		node = fdt_path_offset(fdt, name);

		if (node < 0) {
			debug("WARNING: could not find %s: %s.\n", name,
			       fdt_strerror(node));
			continue;
		}

		nodename = fdt_get_name(fdt, node, &len);

		strncpy(name, nodename, sizeof(name));

		res = rc_is_ready(pem);

		debug("%s: %d: name: %s, ready: %d\n",
		      __FUNCTION__, __LINE__, name, res);

		if (!rc_is_ready(pem)) {
			debug("%s: %d: removing: %s\n",
			      __FUNCTION__, __LINE__, name);
			res = fdt_del_node(fdt, node);

			if (res < 0) {
				printf("WARNING: could not delete %s: %s.\n",
				       name, fdt_strerror(res));
				continue;
			}
		}
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_THUNDERX_BGX
	int node = 0;
#endif
	ft_setup_coremask(blob);

#ifdef CONFIG_THUNDERX_BGX
	for (node = 0; node < atf_node_count(); node++) {
		ft_setup_bgx(blob, node, 0);
		ft_setup_bgx(blob, node, 1);
		ft_setup_macs(blob, node, 0);
		ft_setup_macs(blob, node, 1);
	}
#endif
	ft_setup_uaa_clk(blob);

	ft_setup_pems(blob);

	return 0;
}

#endif

#define NODENAME_BUFLEN 32

int ft_getcore(void *blob, char *id)
{
	int nodeoffset;
	char nodename[NODENAME_BUFLEN];

	snprintf(nodename, sizeof(nodename), "/cpus/cpu@%s", id);

	nodeoffset = fdt_path_offset(blob, nodename);

	if (nodeoffset < 0) {
		printf("WARNING: could not find %s: %s.\n", nodename,
		       fdt_strerror(nodeoffset));
	}

	return nodeoffset;
}

void ft_coreenable(bd_t *bd, char *id, int enable)
{
	int err;
	void *blob = working_fdt;

	int nodeoffset = ft_getcore(blob, id);

	if (nodeoffset < 0)
		return;

	err = fdt_setprop_u32(blob, nodeoffset, "enabled", enable);

	if (err < 0) {
		printf("WARNING: could not set %s: %s.\n", "enabled",
		       fdt_strerror(err));
		return;
	}
}

int ft_corestatus(bd_t *bd, char *id)
{
	const u32 *enabled;
	void *blob = working_fdt;

	int nodeoffset = ft_getcore(blob, id);

	if (nodeoffset < 0)
		return -1;

	enabled = fdt_getprop(blob, nodeoffset, "enabled", NULL);

	if (enabled == NULL)
		return 1;
	else
		return *enabled;
}

static int parse_argv(const char *s)
{
	if (strncmp(s, "en", 2) == 0)
		return 1;
	else if (strncmp(s, "di", 2) == 0)
		return 0;

	return -1;
}

int do_cpucore(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int enable;

	switch (argc) {
	case 3:			/* enable / disable	*/
		enable = parse_argv(argv[2]);

		if (enable < 0)
			return CMD_RET_USAGE;
		else
			ft_coreenable(NULL, argv[1], enable);

		break;
	case 2:			/* get status */
		printf("CPU Core %s is %s\n", argv[1],
		       ft_corestatus(NULL, argv[1]) ? "ENABLED" : "DISABLED");
		return 0;
	default:
		return CMD_RET_USAGE;
	}
	return 0;
}

U_BOOT_CMD(
	cpucore,   3,   1,     do_cpucore,
	"enable or disable a CPU core",
	"id [enable, disable]\n"
	"    - enable or disable a CPU core"
);

#endif
