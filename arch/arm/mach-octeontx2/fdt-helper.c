/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

/* This file contains flat device-tree helper functions.
 * At some later point these functions should be moved into U-Boot common code.
 */

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <asm/arch/fdt-helper.h>

/**
 * Given a FDT node, check if it is compatible with a list of devices
 *
 * @param[in]	fdt		Flat device tree pointer
 * @param	node_offset	Node offset in device tree
 * @param[in]	strlist		Array of FDT devices to check, end must be NULL
 *
 * @return	0 if at least one device is compatible, 1 if not compatible.
 */
int cavium_fdt_node_check_compatible(const void *fdt, int node_offset,
				     const char * const *strlist)
{
	while (*strlist && **strlist) {
		debug("%s: Checking %s\n", __func__, *strlist);
		if (!fdt_node_check_compatible(fdt, node_offset, *strlist)) {
			debug("%s: match found\n", __func__);
			return 0;
		}
		strlist++;
	}
	debug("%s: No match found\n", __func__);
	return 1;
}

/**
 * Given a FDT node, return the next compatible node.
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	start_offset	Starting node offset or -1 to find the first
 * @param	strlist		Array of FDT device compatibility strings, must
 *				end with NULL or empty string.
 *
 * @return	next matching node or -1 if no more matches.
 */
int cavium_fdt_node_offset_by_compatible_list(const void *fdt_addr,
					      int startoffset,
					      const char * const *strlist)
{
	int offset;
	const char * const *searchlist;

	for (offset = fdt_next_node(fdt_addr, startoffset, NULL);
	     offset >= 0;
	offset = fdt_next_node(fdt_addr, offset, NULL)) {
		searchlist = strlist;
		while (*searchlist && **searchlist) {
			if (!fdt_node_check_compatible(fdt_addr, offset,
				*searchlist))
				return offset;
			searchlist++;
		}
	}
	return -1;
}

