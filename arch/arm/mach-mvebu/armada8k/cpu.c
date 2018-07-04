// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>
#include <mach/fw_info.h>

/* Armada 7k/8k */
#define MVEBU_RFU_BASE			(MVEBU_REGISTER(0x6f0000))
#define RFU_GLOBAL_SW_RST		(MVEBU_RFU_BASE + 0x84)
#define RFU_SW_RESET_OFFSET		0

#define SAR0_REG			(MVEBU_REGISTER(0x2400200))
#define BOOT_MODE_MASK			0x3f
#define BOOT_MODE_OFFSET		4

#define NF_CLOCK_SEL_MASK		0x1
#define SOC_MUX_NAND_EN_MASK		0x1
#define CLOCK_1Mhz			1000000

static struct mm_region mvebu_mem_map[] = {
	/* Armada 80x0 memory regions include the CP1 (slave) units */
	{
		/* RAM 0-64MB */
		.phys = 0x0UL,
		.virt = 0x0UL,
		.size = ATF_REGION_START,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	/* ATF and TEE region 0x4000000-0x5400000 not mapped */
	{
		/* RAM 66MB-2GB */
		.phys = ATF_REGION_END,
		.virt = ATF_REGION_END,
		.size = SZ_2G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		/* MMIO regions */
		.phys = MMIO_REGS_PHY_BASE,
		.virt = MMIO_REGS_PHY_BASE,
		.size = SZ_1G,

		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = mvebu_mem_map;

void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}

void reset_cpu(ulong ignored)
{
	u32 reg;

	reg = readl(RFU_GLOBAL_SW_RST);
	reg &= ~(1 << RFU_SW_RESET_OFFSET);
	writel(reg, RFU_GLOBAL_SW_RST);
}

#if CONFIG_IS_ENABLED(NAND_PXA3XX)
/* Return NAND clock in Hz */
u32 mvebu_get_nand_clock(void __iomem *nand_flash_clk_ctrl_reg)
{
	u32 reg;

	if (!nand_flash_clk_ctrl_reg)
		return 0;

	reg = readl(nand_flash_clk_ctrl_reg);
	if (reg & NF_CLOCK_SEL_MASK)
		return 400 * CLOCK_1Mhz;
	else
		return 250 * CLOCK_1Mhz;
}

/* Select NAND in the device bus multiplexer */
void mvebu_nand_select(void __iomem *soc_dev_multiplex_reg)
{
	if (!soc_dev_multiplex_reg)
		return;

	setbits_le32(soc_dev_multiplex_reg, SOC_MUX_NAND_EN_MASK);
}
#endif

int mmc_get_env_dev(void)
{
	u32 reg;
	unsigned int boot_mode;

	reg = readl(SAR0_REG);
	boot_mode = (reg >> BOOT_MODE_OFFSET) & BOOT_MODE_MASK;

	switch (boot_mode) {
	case 0x28:
	case 0x2a:
		return 0;
	case 0x29:
	case 0x2b:
		return 1;
	}

	return CONFIG_SYS_MMC_ENV_DEV;
}
