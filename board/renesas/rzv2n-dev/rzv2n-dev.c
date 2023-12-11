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

#define PFC_BASE			0x10410000

#define	P_2A				(PFC_BASE + 0x002A)
#define	PM_2A				(PFC_BASE + 0x0154)
#define	PMC_2A				(PFC_BASE + 0x022A)

void s_init(void)
{
#if CONFIG_TARGET_RZV2N_DEV
	/* PA5,PA4 port	*/
	*(volatile u8 *)PMC_2A   &= ~(0x03 << 4);
	/* PA5=1,PA4=0 */
	*(volatile u8 *)P_2A      = (*(volatile u32 *)P_2A  & ~(0x03 << 4)) | (0x01 << 5);
	/* PA5,PA4 output */
	*(volatile u16 *)PM_2A    = (*(volatile u32 *)PM_2A & ~(0x0f << 8)) | (0x0c << 8);
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
