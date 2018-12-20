/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region octeontx2_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0x800000000000UL,
		.phys = 0x800000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0x840000000000UL,
		.phys = 0x840000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0x880000000000UL,
		.phys = 0x880000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0x8c0000000000UL,
		.phys = 0x8c0000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		/* List terminator */
		0,
	}
};
struct mm_region *mem_map = octeontx2_mem_map;

u64 get_page_table_size(void)
{
	return 0x6c000;
}

void reset_cpu(ulong addr)
{

}
