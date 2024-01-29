// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
/*
 * Copyright (C) 2023 Renesas Electronics Corp.
 */

#include <common.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

/* PFC */
#define PFC_BASE			0x10410000

#define PWPR				(PFC_BASE + 0x3C04)
#define PWPR_REGWE_A			BIT(6)
#define	PWPR_REGWE_B			BIT(5)

#define	P_2A				(PFC_BASE + 0x002A)
#define	PM_2A				(PFC_BASE + 0x0154)
#define	PMC_2A				(PFC_BASE + 0x022A)
#define PFC_OEN				(PFC_BASE + 0x3C40)

#define	PMC_20				(PFC_BASE + 0x0220)
#define	PFC_20				(PFC_BASE + 0x0480)

/* CPG */
#define CPG_BASE			0x10420000
#define CPG_CLKON_ETH0			(CPG_BASE + 0x062C)
#define CPG_CLKMON_ETH0			(CPG_BASE + 0x0814)
#define CPG_RESET_ETH			(CPG_BASE + 0x092C)
#define CPG_RESETMON_ETH		(CPG_BASE + 0x0A14)

#define	CPG_CLKON_9			(CPG_BASE + 0x0624)
#define	CPG_RST_9			(CPG_BASE + 0x0924)
#define	CPG_RST_10			(CPG_BASE + 0x0928)

void s_init(void)
{
#if CONFIG_TARGET_RZV2N_DEV
	*(volatile u32 *)PWPR |= (PWPR_REGWE_A | PWPR_REGWE_B);

	/* PA5,PA4 port	*/
	*(volatile u8 *)PMC_2A   &= ~(0x03 << 4);
	/* PA5=1,PA4=0 */
	*(volatile u8 *)P_2A      = (*(volatile u32 *)P_2A  & ~(0x03 << 4)) | (0x01 << 5);
	/* PA5,PA4 output */
	*(volatile u16 *)PM_2A    = (*(volatile u32 *)PM_2A & ~(0x0f << 8)) | (0x0c << 8);

	/* I2C8 */
	*(volatile u32 *)PFC_20  = (*(volatile u32 *)PFC_20 & 0x00FFFFFF) | (0x01 << 28) | (0x01 << 24);
	*(volatile u8 *)PMC_20   |= (0x03) << 6;	/* P07,P06 multiplexed function	*/

	*(volatile u32 *)PWPR &= ~(PWPR_REGWE_A | PWPR_REGWE_B);

	*(volatile u32 *)CPG_CLKON_9 = 0x00080008;
	*(volatile u32 *)CPG_RST_10  = 0x00010001;

	/* Use PLL clock for clk_tx_i only for RGMII mode */
	/* Wite OEN reg. OEN0 bit "0" for output direction */
	*(volatile u32 *)(PFC_OEN) &= ~(0x00000001);
	while ((*(volatile u32 *)(PFC_OEN) & 0x00000001) != 0x0)
		;

	/* Enable aclk_csr, aclk, tx, rx, tx_180, rx_180 for ETH0 */
	*(volatile u32 *)(CPG_CLKON_ETH0) = 0x3F003F00;
	while ((*(volatile u32 *)(CPG_CLKMON_ETH0) & 0x3F000000) != 0x3F000000)
		;

	/* Reset ETH0 */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x00010000;
	while ((*(volatile u32 *)(CPG_RESETMON_ETH) & 0x00000002) == 0x0)
		;

	*(volatile u32 *)(CPG_RESET_ETH) = 0x00010001;
	while ((*(volatile u32 *)(CPG_RESETMON_ETH) & 0x00000002) != 0x0)
		;
#endif
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}

void reset_cpu(void)
{
}
