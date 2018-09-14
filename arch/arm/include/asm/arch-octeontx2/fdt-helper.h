/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#ifndef __FDT_HELPER_H__
#define __FDT_HELPER_H__

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
				     const char * const *strlist);

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
					      const char * const *strlist);

#endif /* __FDT_HELPER_H__ */
