/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 SolidRun Ltd.
 */

#include <common.h>
#include <init.h>
#include <env.h>
#include "../rzg-common/rzg-common.h"
#include "../rzg-common/rzg2l-regs.h"

DECLARE_GLOBAL_DATA_PTR;

void s_init(void)
{
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0) & 0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH_CH1) = (*(volatile u32 *)(ETH_CH1) & 0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x30003;
	/* I2C CLK */
	*(volatile u32 *)(CPG_RESET_I2C) = 0xF000F;
	/* I2C pin non GPIO enable */
	*(volatile u32 *)(I2C_CH1) = 0x01010101;
}

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
	rzg_sd_emmc_init();
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;
	return 0;
}

static void carrier_select_fdt(int carrier)
{
	pr_info("Selecting fdt file for board %d...\n", carrier);
	switch (carrier)
	{
	case CARRIER_HB_MATE:
	case CARRIER_HB_RIPPLE:
	case CARRIER_HB_PULSE:
		env_set("fdtfile", "renesas/rzg2l-hummingboard-ripple.dtb");
		break;
	case CARRIER_HB_PRO:
		env_set("fdtfile", "renesas/rzg2l-hummingboard-pro.dtb");
		break;
	case CARRIER_HB_IIOT:
		env_set("fdtfile", "renesas/rzg2l-hummingboard-iiot.dtb");
		break;
	case CARRIER_HB_EU205:
		env_set("fdtfile", "renesas/rzg2l-hummingboard-eu205.dtb");
		break;
	default:
		pr_warn("Leaving default fdtfile \n");
		break;
	}
}

int board_late_init(void)
{
#ifndef CONFIG_SOLIDRUN_DISABLE_TLV
	int carrier = rzg_get_carrier();
	if (carrier < 0)
	{
		pr_err("Can't recognize the carrier board \n");
	}
	rzg_carrier_usb_init(carrier);
	carrier_select_fdt(carrier);
#else
	rzg_carrier_usb_init(CARRIER_UNRECOGNIZED);
#endif
	rzg_set_bootsource_env();
	return 0;
}
