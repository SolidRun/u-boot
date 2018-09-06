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
#include <linux/libfdt.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <asm/arch/atf.h>
#include <asm/arch/octeontx2.h>


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

void octeontx2_parse_board_info(void)
{
	const char *str;
	int node;
	int ret = 0, len = 16;
	u64 midr;

	debug("%s: ENTER\n", __func__);

	midr = cavm_get_model();
	g_cavm_bdt.prod_id = (midr >> 4) & 0xff;

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
