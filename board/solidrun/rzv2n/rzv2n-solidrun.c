/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 SolidRun Ltd.
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
#include <asm/arch/renesas.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>
#include <linux/delay.h>
#include <env.h>
#include "../rzg-common/rzg-common.h"

DECLARE_GLOBAL_DATA_PTR;

/* Forward declarations */
static void board_pmic_i2c_init(void);
#if defined(CONFIG_RZG_SOLIDRUN_COMMON) && !defined(CONFIG_SOLIDRUN_DISABLE_TLV)
static void carrier_select_fdt(int carrier);
#endif

/* RZ/V2N System reset registers */
#define RST_BASE		0x10430000
#define RST_RSTCTRL		(RST_BASE + 0x00)

void s_init(void)
{
	*(volatile u32 *)PWPR |= (PWPR_REGWE_A | PWPR_REGWE_B);
	/* SD1  */
	*(volatile u8 *)PMC_2A   &= ~(0x03 << 2);/* PA3,PA2 port */
	*(volatile u8 *)P_2A      = (*(volatile u32 *)P_2A  & ~(0x03<<2)) | (0x01 <<3); /* PA3=1,PA2=0		*/
	*(volatile u16 *)PM_2A    = (*(volatile u32 *)PM_2A & ~(0x0f<<4)) | (0x0a <<4); /* PA3,PA2 output	*/

	/* I2C2: P2_0=SDA(func4), P2_1=SCL(func4) */
	*(volatile u32 *)PFC_22  = (*(volatile u32 *)PFC_22 & ~0xFF) | (0x04 << 4) | (0x04 << 0);
	*(volatile u8 *)PMC_22   |= 0x03;		/* P2_0,P2_1 multiplexed function */

	/* I2C8: P0_6=SDA(func1), P0_7=SCL(func1) */
	*(volatile u32 *)PFC_20  = (*(volatile u32 *)PFC_20 & 0x00FFFFFF) | (0x01 << 28) | (0x01 << 24);
	*(volatile u8 *)PMC_20   |= (0x03) << 6;	/* P07,P06 multiplexed function	*/

	*(volatile u32 *)CPG_CLKON_9 = 0x00080008;
	*(volatile u32 *)CPG_RST_10  = 0x00010001;

	/* Enale OE for IO blocks of xSPI0 */
	*(volatile u32 *)(PFC_OEN) &= ~GENMASK(5,2);

	/* Use PLL clock for clk_tx_i only for RGMII mode */
	/* Wite OEN reg. OEN0 bit "0" for output direction */
	*(volatile u32 *)(PFC_OEN) &= ~(PFC_OEN_OEN1 | PFC_OEN_OEN0);
	while((*(volatile u32 *)(PFC_OEN) & (PFC_OEN_OEN1 | PFC_OEN_OEN0)) != 0x0)
		;

	*(volatile u32 *)PWPR &= ~(PWPR_REGWE_A | PWPR_REGWE_B);
	/* Set Bypass and Powerdown mode for Audio OSC */
	*(volatile u32 *)(PFC_OSCBYPS) = 0x001C0406;

	*(volatile u32 *)(ICU_IPTSR_REG) = 0;

	/* Reset ETH0 */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x00010000;
	while ((*(volatile u32 *)(CPG_RESETMON_ETH) & 0x00000002) == 0x0)
		;

	/* Release reset ETH0 */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x00010001;
	while ((*(volatile u32 *)(CPG_RESETMON_ETH) & 0x00000002) != 0x0)
		;

	/* Disable SMUX2_GBE0_RXCLK and SMUX2_GBE1_RXCLK */
	*(volatile u32 *) (CPG_SSEL0) = 0x10000000;
	*(volatile u32 *) (CPG_SSEL1) = 0x00100000;

	/* Enable SMUX2_GBE0_RXCLK and SMUX2_GBE1_RXCLK */
	*(volatile u32 *) (CPG_SSEL0) = 0x10001000;
	*(volatile u32 *) (CPG_SSEL1) = 0x00100010;

	/* Enable aclk_csr, aclk, tx, rx, tx_180, rx_180 for ETH0 */
	*(volatile u32 *)(CPG_CLKON_ETH0) = 0x3F003F00;
	while((*(volatile u32 *)(CPG_CLKMON_ETH0) & 0x3F000000) != 0x3F000000)
		;

	/* Enable ADC */
	*(volatile u32 *)(SYS_ADC_CFG) = 0;
}

static void _usbphy_init(void)
{
	/* Overwrite SLEEPM/SUSPENDM signals by USB2PHY Control */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_UTMICTRL2)) = 0x00000303;

	/* Assert USB2PHY reset */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_RESET)) = 0x00000206;

	/* Delay 10us */
	udelay(10);

	/* De-Assert USB2PHY reset */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_RESET)) = 0x00000200;

	/* Release overwrites of SLEEPM/SUSMENDM signals, and RESET signal */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_UTMICTRL2)) = 0x00000003;
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_RESET)) = 0;

	/* Activate VBUS Valid comparator */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_OTGR)) = 0x00000909;
}

static void _reset_usb2(void)
{
	/* Reset for USBTEST */
	(*(volatile u32 *)CPG_RST_USB) |= 0x80008000;
	while((*(volatile u32 *)(CPG_RSTMON5_USB) & 0x00000001) != 0x0);

	/* Reset for USB2 host */
	(*(volatile u32 *)CPG_RST_USB) |= 0x30003000;
	while((*(volatile u32 *)(CPG_RSTMON4_USB) & 0x60000000) != 0x0);
}

static void board_usb_init(void)
{
	/* Reset USB*/
	_reset_usb2();

	/* Enable clock for USB */
	(*(volatile u32 *)CPG_CLKON_USB) = 0x00F800F8;
	while((*(volatile u32 *)(CPG_CLKMON_USB) & 0x00F80000) != 0x00F80000);

	/* Setup  */
	/* Disable GPIO Write Protect */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 6);

#if CONFIG_TARGET_RZV2N_DEV
	/* Set P6_0 as Func.15 for VBUSEN */
	(*(volatile u32 *)PFC_PMC26) |= (0x1u << 0);
	(*(volatile u32 *)PFC_PFC26) &= ~(0xF << 0);
	/* Function mode 15 */
	(*(volatile u32 *)PFC_PFC26) |= (0xF << 0);

	/* Set P6_1 as Func.15 for OVRCUR */
	(*(volatile u32 *)PFC_PMC26) |= (0x1u << 1);
	(*(volatile u32 *)PFC_PFC26) &= ~(0xF << 4);
	/* Function mode 15 */
	(*(volatile u32 *)PFC_PFC26) |= (0xF << 4);
#endif /* CONFIG_TARGET_RZV2N_DEV */

#if CONFIG_TARGET_RZV2N_EVK
        /* Set P9_5 as Func.14 for VBUSEN */
        /* Control mode (multiplexed function) */
        (*(volatile u32 *)PFC_PMC29) |= (0x1u << 5);
        (*(volatile u32 *)PFC_PFC29) &= ~(0xF << 20);
        /* Function mode 14 */
        (*(volatile u32 *)PFC_PFC29) |= (0x0E << 20);

        /* Set P9_6 as Func.14 for OVRCUR */
        /* Control mode (multiplexed function) */
        (*(volatile u32 *)PFC_PMC29) |= (0x1u << 6);
        (*(volatile u32 *)PFC_PFC29) &= ~(0xF << 24);
        /* Function mode 14 */
        (*(volatile u32 *)PFC_PFC29) |= (0x0E << 24);

#endif /* CONFIG_TARGET_RZV2N_EVK */

	/* Enable Write protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 6);

	/* Initialize phy */
	_usbphy_init();

	/*USB0 is HOST*/
	(*(volatile u32 *)(USB20_BASE + COMMCTRL)) = 0;

	/* Set USBPHY normal operation (Function only) */
	(*(volatile u16 *)(USBF_BASE + LPSTS)) |= (0x1u << 14);

	/* Overcurrent is not supported */
	(*(volatile u32 *)(USB20_BASE + HcRhDescriptorA)) |= (0x1u << 12);
}

static void board_pmic_i2c_check_val(struct udevice *dev, u8 reg_addr, u8 reg_val)
{
	u8 read_val;

	/* Read back the value from register */
	if (dm_i2c_read(dev, reg_addr, &read_val, 1)) {
		printf("Error: Failed to read back value from register 0x%x\n", reg_addr);
		return;
	}

	/* Check if the value was written correctly */
	if (read_val != reg_val)
		printf("Error: Written value was not correctly at register 0x%x\n", reg_addr);
}
void board_preboot_os(void)
{
	/* RZV2N has separate MMC0/MMC1 interfaces, no SDIO mux overlay needed */
}

int board_early_init_f(void)
{
	return 0;
}

/*
 * RZ/V2N SoM has separate MMC0 (eMMC) and MMC1 (uSD) interfaces, with no
 * SDIO/MMC GPIO mux. Override the rzg-common default which looks for a
 * /config node + sdio_mux_gpios (RZG2L/G2LC/G2UL/V2L only).
 */
int board_select_sd_emmc(int select_sd)
{
	return 0;
}

static void board_sd1_power_cycle(void)
{
	/* Power-cycle SD1 card to ensure clean state after warm reset.
	 * PA3 controls SD1 power enable, PA2 is SD1 reset.
	 * PWPR write-protect was already unlocked/relocked in s_init(),
	 * but the GPIO output register (P) can be written without PWPR. */
	*(volatile u32 *)PWPR |= (PWPR_REGWE_A | PWPR_REGWE_B);

	/* Deassert SD1 power: PA3=0 */
	*(volatile u8 *)P_2A = (*(volatile u8 *)P_2A & ~(0x03 << 2)); /* PA3=0,PA2=0 */

	*(volatile u32 *)PWPR &= ~(PWPR_REGWE_A | PWPR_REGWE_B);

	/* Wait 10ms for power rail to discharge */
	mdelay(10);

	*(volatile u32 *)PWPR |= (PWPR_REGWE_A | PWPR_REGWE_B);

	/* Re-enable SD1 power: PA3=1, PA2=0 */
	*(volatile u8 *)P_2A = (*(volatile u8 *)P_2A & ~(0x03 << 2)) | (0x01 << 3); /* PA3=1,PA2=0 */

	*(volatile u32 *)PWPR &= ~(PWPR_REGWE_A | PWPR_REGWE_B);

	/* Wait for power to stabilize before SDHI driver probes */
	mdelay(10);
}

int board_early_init_r(void)
{
	board_sd1_power_cycle();
	rzg_sd_emmc_init();
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	board_usb_init();
	/* Initialize PMIC I2C devices */
	board_pmic_i2c_init();

	return 0;
}

int board_late_init(void)
{
	/*
	 * PMIC at I2C8@0x12 may not be ready at this point on some
	 * board revisions, causing the I2C probe to hang. The critical
	 * PMIC setup is handled at 0x6f in board_pmic_i2c_init().
	 */

#ifdef CONFIG_RZG_SOLIDRUN_COMMON
#ifndef CONFIG_SOLIDRUN_DISABLE_TLV
	int carrier = rzg_get_carrier();
	if (carrier < 0)
	{
		pr_warn("Can't detect carrier board (ret=%d), using default FDT\n", carrier);
	} else {
		carrier_select_fdt(carrier);
	}
#endif
	rzg_set_bootsource_env();
#endif

	/*
	 * On a freshly-flashed board the SPI environment region is unprogrammed
	 * (or has a stale CRC), so U-Boot prints "bad CRC, using default
	 * environment" and falls back to defaults — which means the random MAC
	 * and any other runtime tweaks above don't persist across reboots.
	 *
	 * Detect that case and write the defaults to flash once. Subsequent
	 * boots then find a valid CRC and skip this branch.
	 */
	if (gd->env_valid != ENV_VALID) {
		printf("Env invalid: writing defaults to storage...\n");
		if (env_save())
			pr_err("env_save() failed\n");
	}

	return 0;
}

static void board_pmic_i2c_init(void)
{
	struct udevice *bus, *dev;
	int ret;
	u8 reg_addr, reg_val;

	/* Get the I2C bus */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 8, &bus);
	if (ret)
		goto pmic_failed;

	/* Initialize I2C device at address 0x6f */
	ret = dm_i2c_probe(bus, 0x6f, 0, &dev);
	if (ret)
		goto pmic_failed;

	/* Write 0x00 to register 0x24 of device 0x6f */
	reg_addr = 0x24;
	reg_val = 0x00;
	ret = dm_i2c_write(dev, reg_addr, &reg_val, 1);
	if (ret)
		goto pmic_failed;

	board_pmic_i2c_check_val(dev, reg_addr, reg_val);

	return;

pmic_failed:
	printf("Can not initialize PMIC settings via I2C8\n");
	return;
}

#if defined(CONFIG_RZG_SOLIDRUN_COMMON) && !defined(CONFIG_SOLIDRUN_DISABLE_TLV)
static void carrier_select_fdt(int carrier)
{
	pr_info("Selecting fdt file for board %d...\n", carrier);
	switch (carrier)
	{
	case CARRIER_HB_MATE:
	case CARRIER_HB_RIPPLE:
	case CARRIER_HB_PULSE:
		env_set("fdtfile", "renesas/r9a09g056n48-hummingboard-puls.dtb");
		break;
	case CARRIER_HB_PRO:
		env_set("fdtfile", "renesas/r9a09g056n48-hummingboard-pro.dtb");
		break;
	case CARRIER_HB_IIOT:
		env_set("fdtfile", "renesas/r9a09g056n48-hummingboard-iiot.dtb");
		break;
	case CARRIER_SOLIDSENSE_AIOT:
		env_set("fdtfile", "renesas/r9a09g056n48-solidsense-aiot.dtb");
		break;
	default:
		pr_warn("Leaving default fdtfile \n");
		break;
	}
}
#endif

void reset_cpu(void)
{
}