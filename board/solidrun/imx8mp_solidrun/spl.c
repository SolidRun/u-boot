/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <power/pmic.h>

#include <power/pca9450.h>
#include <asm/arch/clock.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <asm/arch/ddr.h>

#define ONE_GB 0x40000000ULL
DECLARE_GLOBAL_DATA_PTR;

extern struct dram_timing_info dram_timing_3gb_micron;
extern struct dram_timing_info dram_timing_1gb_samsung;
extern struct dram_timing_info dram_timing_2gb_samsung;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
#ifdef CONFIG_SPL_BOOTROM_SUPPORT
	return BOOT_DEVICE_BOOTROM;
#else
	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD3_BOOT:
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	case QSPI_BOOT:
		return BOOT_DEVICE_NOR;
	case NAND_BOOT:
		return BOOT_DEVICE_NAND;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}
#endif
}

int check_mirror_ddr(unsigned int addr_1, unsigned int addr_2)
{

	/* return 1 if mirror detected between addr_1 & addre_2, else return 0*/
	int retrain_tmp;
	unsigned int save1, save2, mirror;
	volatile unsigned int *ptr;

	retrain_tmp = 0;

	ptr = (volatile unsigned int *)CONFIG_SYS_SDRAM_BASE;
	save1 = ptr[addr_1];
	save2 = ptr[addr_2];
	ptr[addr_2] = save1 << 2;
	ptr[addr_1] = ~save1;
	mirror = ptr[addr_2];
	if (mirror == ~save1) {
	       printf ("Mirror detected\n");
	       retrain_tmp = 1;
	}
	ptr[addr_1] = save1;
	ptr[addr_2] = save2;

	// Check if mirror have detected
	if (retrain_tmp == 1)
	       return 1;

	return 0;
}

void spl_dram_init(void)
{
	int ret, retrain_1gb, retrain_2gb;

	printf ("Training for 3GByte Mimcron\n");
	ret = ddr_init(&dram_timing_3gb_micron);
	if (ret == 0) {
		// Check Mirror for 1GB
		retrain_1gb = check_mirror_ddr(0, ONE_GB/4);
		if (retrain_1gb == 1)
		{
			printf ("Re-training for 1GByte Samsung (3->1)\n");
			ret = ddr_init(&dram_timing_1gb_samsung);
			return;
		}
		// Check Mirror for 2GB
		retrain_2gb = check_mirror_ddr(0, 2*ONE_GB/4);
		if (retrain_2gb == 1)
		{
			printf ("Re-training for 2GByte Samsung (3->2)\n");
			ret = ddr_init(&dram_timing_2gb_samsung);
			return;
		}
	} else {
		printf ("Re-training for 2GByte Samsung\n");
		ret = ddr_init(&dram_timing_2gb_samsung);
		if (ret == 0) {
			// Check Mirror for 1GB
			retrain_1gb = check_mirror_ddr(0, ONE_GB/4);
			if (retrain_1gb == 1) {
				printf ("Re-training for 1GByte Samsung (2->1)\n");
				ret = ddr_init(&dram_timing_1gb_samsung);
				return;
			}
		} else {
			printf ("Re-training for 1GByte Samsung(1)\n");
			ddr_init(&dram_timing_1gb_samsung);
			return;
		}
	}
}

#if CONFIG_IS_ENABLED(DM_PMIC_PCA9450)
int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pca9450@25", &dev);
	if (ret == -ENODEV) {
		puts("No pca9450@25\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/*
	 * increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85v for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD freq for SOC */
	/* To avoid timing risk from SOC to ARM,increase VDD_ARM to OD voltage 0.95v */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1C);

	/* set WDOG_B_CFG to cold reset */
	pmic_reg_write(dev, PCA9450_RESET_CTRL, 0xA1);

	return 0;
}
#endif

void spl_board_init(void)
{
	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(caam_jr), &dev);
		if (ret)
			printf("Failed to initialize caam_jr: %d\n", ret);
	}

	/* Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does not allow to change it.
	 * Should set the clock after PMIC setting done.
	 * Default is 400Mhz (system_pll1_800m with div = 2) set by ROM for ND VDD_SOC
	 */
#if defined(CONFIG_IMX8M_LPDDR4) && !defined(CONFIG_IMX8M_VDD_SOC_850MV)
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
#endif

	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
