/** @file
#
#  Copyright (c) 2014, Cavium Inc. All rights reserved.<BR>
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

#include <common.h>
#include <asm/io.h>
#include <net.h>

#include <malloc.h>
#include <cavium/atf.h>

#include <libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

extern struct fdt_header *working_fdt;

struct mac_range {
	unsigned char	enetaddr[6];
	uint32_t		size;
} __attribute__((packed));

int do_vnic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret, len, i;
	unsigned int node;

	struct mac_range range;
	struct mac_range *rangep;

	char nodename[25];
	int nodeoffset;

	if ((argc == 5) && !strcmp(argv[1], "addmac")) {
		rangep = &range;
		node = simple_strtoul(argv[2], NULL, 10);

		if (node > atf_node_count()) {
			printf("Invalid node number\n");
			return 0;
		}

		eth_parse_enetaddr(argv[3], rangep->enetaddr);

		if (!is_valid_ethaddr(rangep->enetaddr)) {
			printf("Invalid MAC address\n");
			return 0;
		}

		rangep->size = simple_strtoul(argv[4], NULL, 10);
		rangep->size = cpu_to_fdt32(rangep->size);

		snprintf(nodename, sizeof(nodename), "/ethernet-macs/node%d", node);

		nodeoffset = fdt_path_offset(working_fdt, nodename);

		if (nodeoffset < 0) {
			printf("WARNING: could not find %s: %s.\n", nodename,
						fdt_strerror(nodeoffset));
		}

		ret = fdt_appendprop(working_fdt, nodeoffset, "mac",
									rangep, sizeof(struct mac_range));

		if (ret < 0) {
			printf("WARNING: could not add the ethernet address %pM: %s.\n",
						rangep->enetaddr, fdt_strerror(ret));
			return 0;
		}
	} else if ((argc == 3) && !strcmp(argv[1], "listmacs")) {
		node = simple_strtoul(argv[2], NULL, 10);

		if (node > atf_node_count()) {
			printf("Invalid node number\n");
			return 0;
		}

		snprintf(nodename, sizeof(nodename), "/ethernet-macs/node%d", node);

		nodeoffset = fdt_path_offset(working_fdt, nodename);

		if (nodeoffset < 0) {
			printf("WARNING: could not find %s: %s.\n", nodename,
						fdt_strerror(nodeoffset));
		}

		rangep = (void *)fdt_getprop(working_fdt, nodeoffset, "mac", &len);

		if (len < 0) {
			printf("WARNING: could not get the property: %s.\n", fdt_strerror(len));
			return 0;
		}

		debug("len: %d, sizeof(struct mac_range): %ld\n",
				len, sizeof(struct mac_range));

		if (len % sizeof(struct mac_range)) {
			printf("Invalid property length %d\n", len);
			return 0;
		}

		printf("MAC\t\tSize\n");

		for (i = 0; i < len / sizeof(struct mac_range); i++) {
			printf("%pM\t%u\n", rangep[i].enetaddr, fdt32_to_cpu(rangep[i].size));
		}

	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	vnic,   10,   1,     do_vnic,
	"VNIC related commands",
	"\t addmac node mac size - add a MAC address range\n"
	"\t listmacs node        - list MAC address ranges\n"
);
