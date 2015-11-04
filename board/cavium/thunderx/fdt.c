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

#ifdef CONFIG_THUNDER_BGX
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

static void ft_setup_bgx(char *fdt, unsigned int node, unsigned int bgx_id)
{
	union bgxx_cmrx_config cmrx_config;
	union bgxx_spux_br_pmd_control spux_br_pmd_control;

	char fdt_key[20];

	int offset, next_offset;
	char qlm[32];
	char *mode;
	int qlm_key_len;
	int rc;

	/* Read LMAC0 type to figure out QLM mode
	 * This is configured by low level firmware
	 */

	cmrx_config.u = CSR_READ_PA(node, BGXX_CMRX_CONFIG(bgx_id, 0));
	spux_br_pmd_control.u  = CSR_READ_PA(node, BGXX_SPUX_BR_PMD_CONTROL(bgx_id, 0));

	switch(cmrx_config.s.lmac_type) {
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
	mode = qlm;
	strsep(&mode, ",");
	qlm_key_len = strlen(qlm);

	if (!fdt || fdt_check_header(fdt) != 0) {
		printf("%s: Invalid device tree\n", __func__);
		return;
	}

	/* Prune out the unwanted parts based on the QLM mode.  */
	for (offset = fdt_next_node(fdt, 0, NULL);
		 offset >= 0; offset = next_offset) {
		int len;
		const char *val;

		next_offset = fdt_next_node(fdt, offset, NULL);
		val = fdt_getprop(fdt, offset, "qlm-mode", &len);
		if (!val)
			continue;

		if (strncmp(val, qlm, qlm_key_len) != 0)
			continue; /* Not this QLM. */

		if (!fdt_stringlist_contains(val, len, fdt_key)) {
			debug("Key \"%s\" does not match \"%s\"\n",
					val, fdt_key);
			/* This QLM, but wrong mode.  Delete it. */
			debug("fdt trimming matching key %s\n", fdt_key);
			next_offset = fdt_parent_offset(fdt, offset);
			rc = fdt_nop_node(fdt, offset);
			if (rc) {
				printf("Error %d noping node in device tree\n",
						rc);
			}
		}
	}
}

#define VNIC_PER_NODE 2

static void ft_setup_macs(void *fdt, int node)
{
	int nodeoffset, addr, len, ret, i;
	char nodename[25];

	struct mac_range mac, *macp;

	mac.size = 1;

	snprintf(nodename, sizeof(nodename), "/ethernet-macs/node%d", node);

	nodeoffset = fdt_path_offset(fdt, nodename);

	if (nodeoffset < 0) {
		printf("WARNING: could not find %s: %s.\n", nodename,
					fdt_strerror(nodeoffset));
	}

	for (addr = (node + 0) * VNIC_PER_NODE;
		 addr < (node + 1) * VNIC_PER_NODE; addr++) {

		if (!eth_getenv_enetaddr_by_index("eth", addr, mac.enetaddr))
			continue;

		macp = (void *)fdt_getprop(fdt, nodeoffset, "mac", &len);

		if (len < 0) {
			printf("WARNING: could not find %s/mac: %s.\n",
						nodename, fdt_strerror(len));
			return;
		}

		for (i = 0; i < len / sizeof(mac); i++) {
			if (!memcmp(macp[i].enetaddr, mac.enetaddr, 6))
				continue;
		}

		ret = fdt_appendprop(fdt, nodeoffset, "mac",
						&mac, sizeof(struct mac_range));

		if (ret < 0) {
			printf("WARNING: could not add the ethernet address %pM: %s.\n",
							mac.enetaddr, fdt_strerror(ret));
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

int ft_board_setup(void *blob, bd_t *bd)
{
	unsigned int node;

	ft_setup_coremask(blob);

#ifdef CONFIG_THUNDER_BGX
	for (node = 0; node < atf_node_count(); node++) {
		ft_setup_bgx(blob, node, 0);
		ft_setup_bgx(blob, node, 1);
		ft_setup_macs(blob, node);
	}
#endif
	ft_setup_uaa_clk(blob);

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
