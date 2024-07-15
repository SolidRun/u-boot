// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * https://spdx.org/licenses
 *
 * Copyright (C) 2024 Marvell
 *
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch/board.h>
#include <asm/global_data.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

#define CN20K_MEM_MAP_USED 11

/* +1 is end of list which needs to be empty */
#define CN20K_MEM_MAP_MAX (CN20K_MEM_MAP_USED + CONFIG_NR_DRAM_BANKS + 3)

static struct mm_region cn20k_mem_map[CN20K_MEM_MAP_MAX] = {
	{
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
		.virt = 0xC00000000000UL,
		.phys = 0xC00000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0xC40000000000UL,
		.phys = 0xC40000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0xC80000000000UL,
		.phys = 0xC80000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0xCc0000000000UL,
		.phys = 0xCc0000000000UL,
		.size = 0x40000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0x400000000000UL,
		.phys = 0x400000000000UL,
		.size = 0x10000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0x500000000000UL,
		.phys = 0x500000000000UL,
		.size = 0x10000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}, {
		.virt = 0x600000000000UL,
		.phys = 0x600000000000UL,
		.size = 0x10000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	}
};

struct mm_region *mem_map = cn20k_mem_map;

#define SHFW_REGION	0x3000000UL
void mem_map_fill(u64 rvu_addr, u64 rvu_size)
{
	int banks = CN20K_MEM_MAP_USED;
	u32 dram_start = CONFIG_TEXT_BASE;

	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		cn20k_mem_map[banks].virt = dram_start;
		cn20k_mem_map[banks].phys = dram_start;
		cn20k_mem_map[banks].size = gd->ram_size;
		cn20k_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					    PTE_BLOCK_INNER_SHARE;
		banks = banks + 1;
	}
	cn20k_mem_map[banks].virt = dram_start - SHFW_REGION;
	cn20k_mem_map[banks].phys = dram_start - SHFW_REGION;
	cn20k_mem_map[banks].size = SHFW_REGION;
	cn20k_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				    PTE_BLOCK_INNER_SHARE;
	banks++;
	cn20k_mem_map[banks].virt = rvu_addr;
	cn20k_mem_map[banks].phys = rvu_addr;
	cn20k_mem_map[banks].size = rvu_size;
	cn20k_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				    PTE_BLOCK_INNER_SHARE;
}

u64 get_page_table_size(void)
{
	return 0xC0000;
}

void reset_cpu(void)
{
	psci_system_reset();
}
