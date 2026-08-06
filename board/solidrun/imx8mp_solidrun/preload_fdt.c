// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Josua Mayer <josua@solid-run.com>
 */

#include <binman.h>
#include <dm/ofnode.h>
#include <env.h>
#include <linux/libfdt.h>

#define OS_DTBS_STAGE_ADDR	RAMDISK_ADDR_R	/* imx-mkimage loads fit image here */

/* preload builtin dtbs for operating system into fdt_addr_r */
int board_preload_os_fdt(void)
{
	void *fit = (void *)OS_DTBS_STAGE_ADDR;
	int node, section_node;
	const char *target_compatible;
	ulong fdt_addr_r;
	void *dest_ptr;
	int ret;

	/* check for valid fit image */
	if (fdt_check_header(fit) != 0) {
		printf("%s: could not find builtin 'os-dtbs' fit image\n", __func__);
		return -ENOENT;
	}

	/* get '/images' section */
	section_node = fdt_path_offset(fit, "/images");
	if (section_node < 0) {
		printf("%s: could not find builtin 'os-dtbs' fit image '/images' section\n", __func__);
		return -ENOENT;
	}

	/* get active control fdt board level compatible string */
	target_compatible = fdt_getprop(gd->fdt_blob, 0, "compatible", NULL);
	if (!target_compatible) {
		printf("%s: control fdt missing compatible strings\n", __func__);
		return -EINVAL;
	}

	/* get fdt load address */
	fdt_addr_r = env_get_ulong("fdt_addr_r", 16, FDT_ADDR_R);
	dest_ptr = (void *)fdt_addr_r;

	/* can each subnode */
	fdt_for_each_subnode(node, fit, section_node) {
		const char *node_name = fdt_get_name(fit, node, NULL);
		const void *dtb_data;
		size_t dtb_size;

		/* get pointer to,and size of data blob */
		ret = fit_image_get_data(fit, node, &dtb_data, &dtb_size);
		if (ret) {
			printf("%s: could not get 'os-dtbs/images/%s': %d\n", __func__, node_name, ret);
			continue;
		}
		if (fdt_check_header(dtb_data) != 0) {
			printf("%s: 'os-dtbs/images/%s' fdt header invalid\n", __func__, node_name);
			continue;
		}

		/* compare control fdt board level compatible string with compatible string in array */
		// TODO: Consider matching against fdtfile / fit description field
		if (fdt_node_check_compatible(dtb_data, 0, target_compatible) == 0) {
			debug("%s: Matched '%s' (%s, %zu bytes) -> 0x%lx\n", __func__, node_name, target_compatible, dtb_size, fdt_addr_r);

			/* copy to fdt load address */
			memcpy(dest_ptr, dtb_data, dtb_size);

			/* set firmware-provided fdt address environment */
			env_set_hex("fdt_addr", fdt_addr_r);

			/* update fdt_addr_r in case it was previously unset */
			env_set_hex("fdt_addr_r", fdt_addr_r);

			printf("%s: preloaded os dtb for '%s'.\n", __func__, target_compatible);
			return 0;
		}
	}

	printf("%s: no dtb in 'os-dtbs/images/' matched compatible '%s'.\n", __func__, target_compatible);
	return -ENOENT;
}
