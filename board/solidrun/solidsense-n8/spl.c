// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019, 2021 NXP
 * Copyright 2025 Josua Mayer <josua@solid-run.com>
 */

#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <hang.h>
#include <init.h>
#include <power/pmic.h>
#include <power/bd71837.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case SD2_BOOT:
		return BOOT_DEVICE_MMC1;
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		if (IS_ENABLED(CONFIG_SPL_BOOTROM_SUPPORT))
			return BOOT_DEVICE_BOOTROM;

		return BOOT_DEVICE_NONE;
	}
}

void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@4b", &dev);
	if (ret == -ENODEV) {
		puts("No pmic@4b\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* decrease RESET key long push time from the default 10s to 10ms */
	pmic_reg_write(dev, BD718XX_PWRONCONFIG1, 0x0);

	/* unlock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x1);

	/* Set VDD_ARM to typical value 0.85v for 1.2Ghz */
	pmic_reg_write(dev, BD718XX_BUCK2_VOLT_RUN, 0xf);

	if (IS_ENABLED(CONFIG_IMX8MN_LOW_DRIVE_MODE)) {
		/* Set VDD_SOC/VDD_DRAM to typical value 0.8v for low drive mode */
		pmic_reg_write(dev, BD718XX_BUCK1_VOLT_RUN, 0xa);
	} else {
		/* Set VDD_SOC/VDD_DRAM to typical value 0.85v for nominal mode */
		pmic_reg_write(dev, BD718XX_BUCK1_VOLT_RUN, 0xf);
	}

	/* Set VDD_SOC 0.75v for low-v suspend */
	pmic_reg_write(dev, BD718XX_BUCK1_VOLT_SUSP, 0x5);

	/* increase NVCC_DRAM_1V2 to 1.2v for DDR4 */
	pmic_reg_write(dev, BD718XX_4TH_NODVS_BUCK_VOLT, 0x28);

	/* lock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x11);

	return 0;
}

void spl_board_init(void)
{
	arch_misc_init();

	puts("Normal Boot\n");
}

int board_fit_config_name_match(const char *name)
{
	/* only have one board, match any dtb */
	return 0;
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

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

	preloader_console_init();

	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
