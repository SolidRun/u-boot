/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

// #define PRINT_DDR_TABLES
// #define DEBUG

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
#include <asm/sections.h>

#include "lpddr4_timing.h"

#define ONE_GB 0x40000000ULL
DECLARE_GLOBAL_DATA_PTR;

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

#ifdef PRINT_DDR_TABLES
static struct dram_timing_info *const dram_timing_patch(struct dram_timing_info *const timings)
{
	if (timings == &dram_timing_patch_2gb_samsung) {
		timing_patch_apply(&dram_timing_1gb_samsung_micron, timings);
		return &dram_timing_1gb_samsung_micron;
	}
	if (timings == &dram_timing_patch_8gb_micron) {
		timing_patch_apply(&dram_timing_4gb_samsung_micron, timings);
		return &dram_timing_4gb_samsung_micron;
	}
	return timings;
}

static struct dram_configs {
	const char *const label;
	struct dram_timing_info *const timings;
	unsigned int mr5, mr6, mr7, mr8;
	bool is_valid;
} confs[] = {
	{ .label = "Samsung 8G       ", .timings = &dram_timing_patch_8gb_micron },
	{ .label = "Samsung/Micron 4G", .timings = &dram_timing_4gb_samsung_micron },
	{ .label = "Micron 3G        ", .timings = &dram_timing_3gb_micron },
	{ .label = "Samsung 2G       ", .timings = &dram_timing_patch_2gb_samsung },
	{ .label = "Samsung/Micron 1G", .timings = &dram_timing_1gb_samsung_micron },
};

static void spl_print_ddr_tables(void)
{
	int ret, i;

	/* Collect data */
	for (i = 0; i < ARRAY_SIZE(confs); i++) {
		ret = ddr_init(dram_timing_patch(confs[i].timings));
		dram_timing_patch(confs[i].timings);
		if (ret) {
			confs[i].is_valid = false;
		} else {
			confs[i].is_valid = true;
			confs[i].mr5 = lpddr4_mr_read(0xFF, 0x5);
			confs[i].mr6 = lpddr4_mr_read(0xFF, 0x6);
			confs[i].mr7 = lpddr4_mr_read(0xFF, 0x7);
			confs[i].mr8 = lpddr4_mr_read(0xFF, 0x8);
		}
	}

	/* Now print table */

	printf("\n\n\n");
	printf("****************************************\n");
	printf("************** DDR Tables **************\n");
	printf("****************************************\n");
	printf("(Please print the tables multiple times\nto determine if the values are stable).\n");
	printf("\n");
	printf("Size\tMR5\tMR6\tMR7\tMR8\n");

	for (i = 0; i < ARRAY_SIZE(confs); i++) {
		if (!confs[i].is_valid)
			printf("%-17s\t********** Failed **********\n", confs[i].label);
		else
			printf("%-17s\t0x%x\t0x%x\t0x%x\t0x%x\n", confs[i].label, confs[i].mr5, confs[i].mr6, confs[i].mr7, confs[i].mr8);
	}
	printf("\n");
	printf("****************************************\n");
	printf("************ DDR Tables End ************\n");
	printf("****************************************\n");
	printf("\n\n\n");
}
#endif //PRINT_DDR_TABLES

/* MUST be called after DDR training with 4G parameters! */
static bool spl_dram_is_3G(void)
{
	volatile uint32_t *base;
	uint32_t backup;
	bool ret = false;

	/* SDRAM 2 starts after 3G.
	 * We write 512M after the 3G offset.
	 * If the value is not written, this is a 3G configuration.
	 */
	base = (volatile uint32_t *)(PHYS_SDRAM_2 + SZ_512M);

	/* Backup value */
	backup = *base;

	/* Write something */
	*base = ~backup;
	/* Read back */
	if (*base != ~backup)
		ret = true;

	*base = backup;
	return ret;
}

/* MUST be called after DDR training with 2G parameters! */
static bool spl_dram_is_1G(void)
{
	volatile uint32_t *base1, *base2;
	volatile uint32_t tmp;
	uint32_t backup1, backup2;
	bool ret = false;

	/* The idea is to write a value in offset 1G and see if it is
	 * written in offset 0 as well
	 */
	base1 = (volatile uint32_t *)CFG_SYS_SDRAM_BASE;
	base2 = (volatile uint32_t *)((uint64_t)CFG_SYS_SDRAM_BASE + SZ_1G);

	backup1 = *base1;
	backup2 = *base2;

	*base2 = 0xAAAAAAAA;
	*base1 = 0x55555555;

	tmp = *base2;
	if (tmp == 0x55555555)
		ret = true;

	*base1 = backup1;
	*base2 = backup2;

	return ret;
}

static bool spl_generic_ddr_init(void)
{
	int ret;
	bool output = true;

	/* Try 8GB Micron. */
	timing_patch_apply(&dram_timing_4gb_samsung_micron, &dram_timing_patch_8gb_micron);
	ret = ddr_init(&dram_timing_4gb_samsung_micron);
	timing_patch_apply(&dram_timing_4gb_samsung_micron, &dram_timing_patch_8gb_micron);
	if (!ret) {
		printf("DDR 8G Micron identified!\n");
		goto exit;
	}

	/* Try 4G Samsung.
	 * Will work with: 3G Micron as well.
	 */
	ret = ddr_init(&dram_timing_4gb_samsung_micron);
	if (!ret) {
		if (!spl_dram_is_3G()) {
			printf("DDR 4G Samsung/Micron identified!\n");
			goto exit;
		}
	}

	/* Try 3G Micron.
	 * Will work with: 4G Samsung/Micron, but since we already tested it,
	 * it's not relevant
	 */
	ret = ddr_init(&dram_timing_3gb_micron);
	if (!ret) {
		printf("DDR 3G Micron identified!\n");
		goto exit;
	}

	/* Try 2G Samsung.
	 * Will work with: 1G Samsung as well.
	 */
	timing_patch_apply(&dram_timing_1gb_samsung_micron, &dram_timing_patch_2gb_samsung);
	ret = ddr_init(&dram_timing_1gb_samsung_micron);
	timing_patch_apply(&dram_timing_1gb_samsung_micron, &dram_timing_patch_2gb_samsung);
	if (!ret) {
		if (!spl_dram_is_1G()) {
			printf("DDR 2G Samsung identified!\n");
			goto exit;
		}
	}

	/* Try 1G Samsung/Micron.
	 * If fails, we give up...
	 */
	ret = ddr_init(&dram_timing_1gb_samsung_micron);
	if (ret) {
		printf("Failed to initialize the DDR with the generic approach, giving up...\n");
		output = false;
	} else {
		printf("DDR 1G Samsung/Micron identified!\n");
	}

exit:
	return output;
}

/* Function used to identify the DDR.
 * Function returns the timing parameters to use for DDR training, or NULL if failed
 * to identify the DDR.
 *
 * In order to read the DDR values, the function will train the DDR with
 * default parameters.
 * Those parameters may be the same parameters needed to train the DDR.
 * In this case, this function will set @needs_training to false, indicating that
 * there is no need to train the DDR again.
 * Caller can ignore this argument without harm (argument can be NULL).
 */
static struct dram_timing_info *spl_identify_ddr(bool *needs_training)
{
	int ret;
	unsigned int mr5, mr6, mr7, mr8;
	bool tmp;

	/*                    Values for 8G Micron training
	 *
	 *              MR5     MR6     MR7     MR8
	 * Micron 8G    255		7		0		24
	 *
	 *                    Values for 3G Micron training
	 *
	 *		MR5	|	M6	|	MR7	|	MR8
	 * Samsung 1G   ************** TRAINING FAILURE ********************
	 * Micron 1G    ************** TRAINING FAILURE ********************
	 * Samsung 2G   ************** TRAINING FAILURE ********************
	 * Micron 3G    255		4		1		12
	 * Samsung 4G   1		6		16		16
	 * Micron 4G    255		7		0		16
	 * Micron 8G    255		7		0		24
	 *
	 *   		      Values for 1G Samsung/Micron training
	 *
	 *		MR5	|	M6	|	MR7	|	MR8
	 * Samsung 1G   1               6               0               8
	 * Micron 1G    255		3		0		8
	 * Samsung 2G   1		6		16		16
	 * Micron 3G    255		4		1		12
	 * Samsung 4G   ****************** UNSTABLE ************************
	 * Micron 4G    ****************** UNSTABLE ************************
	 * Micron 8G    255		0		0		0
	 *
	 * Algorithm:
	 * DDR training with 3G Micron, if succeeds, check if this is a 3G Micron,
	 * 4G Samsung/Micron, 8G Micron, or unknown.
	 * If fails, DDR training with 1G Samsung/Micron, check if this is a 2G Samsung,
	 * 1G Samsung, 1G Micron or unknown.
	 */

	/* Init the @needs_training argument */
	if (!needs_training)
		needs_training = &tmp;
	*needs_training = true;

	/* Training with 3G Micron */
	if (!ddr_init(&dram_timing_3gb_micron)) {
		/* Training with 3G Micron succedded */
		mr5 = lpddr4_mr_read(0xF, 0x5);
		mr6 = lpddr4_mr_read(0xF, 0x6);
		mr7 = lpddr4_mr_read(0xF, 0x7);
		mr8 = lpddr4_mr_read(0xF, 0x8);

		debug("MR5=0x%x, MR6=0x%x, MR7=0x%x, MR8=0x%x\n", mr5, mr6, mr7, mr8);

		if (mr5 == 0xFF && mr6 == 0x4 && mr7 == 0x1 && mr8 == 0xC) {
			printf("DDR 3G Micron identified!\n");
			*needs_training = false;
			return &dram_timing_3gb_micron;
		} else if (mr5 == 0x1 && mr6 == 0x6 && mr7 == 0x10 && mr8 == 0x10) {
			printf("DDR 4G Samsung identified!\n");
			return &dram_timing_4gb_samsung_micron;
		} else if (mr5 == 0xFF && mr6 == 0x7 && mr7 == 0x0 && mr8 == 0x10) {
			printf("DDR 4G Micron identified!\n");
			return &dram_timing_4gb_samsung_micron;
		} else if (mr5 == 0xFF && mr6 == 0x7 && mr7 == 0x0 && mr8 == 0x18) {
			printf("DDR 8G Micron identified!\n");
			timing_patch_apply(&dram_timing_4gb_samsung_micron, &dram_timing_patch_8gb_micron);
			return &dram_timing_4gb_samsung_micron;
		} else {
			goto err;
		}
	} else {
		/* Training with 3G Micron failed
		 * DDR training with 1G Samsung/Micron
		 */
		ret = ddr_init(&dram_timing_1gb_samsung_micron);
		if (ret)
			goto err;

		mr5 = lpddr4_mr_read(0xF, 0x5);
		mr6 = lpddr4_mr_read(0xF, 0x6);
		mr7 = lpddr4_mr_read(0xF, 0x7);
		mr8 = lpddr4_mr_read(0xF, 0x8);

		debug("MR5=0x%x, MR6=0x%x, MR7=0x%x, MR8=0x%x\n", mr5, mr6, mr7, mr8);

		if (mr5 == 0x1 && mr6 == 0x6 && mr7 == 0x0 && mr8 == 0x8) {
			printf("DDR 1G Samsung identified!\n");
			*needs_training = false;
			return &dram_timing_1gb_samsung_micron;
		} else if (mr5 == 0xFF && mr6 == 0x3 && mr7 == 0x0 && mr8 == 0x8) {
			printf("DDR 1G Micron identified!\n");
			*needs_training = false;
			return &dram_timing_1gb_samsung_micron;
		} else if (mr5 == 0x1 && mr6 == 0x6 && mr7 == 0x10 && mr8 == 0x10) {
			printf("DDR 2G Samsung identified!\n");
			timing_patch_apply(&dram_timing_1gb_samsung_micron, &dram_timing_patch_2gb_samsung);
			return &dram_timing_1gb_samsung_micron;
		} else {
			goto err;
		}
	}

err:
	printf("Could not identify DDR!\n");
	return NULL;
}

static void spl_dram_init(void)
{
	struct dram_timing_info *dram_info;
	int ret = -1;
	bool need_training;

#ifdef PRINT_DDR_TABLES
	spl_print_ddr_tables();
#endif
	dram_info = spl_identify_ddr(&need_training);
	if (dram_info) {
		/* DDR was identified, do we need to train the DDR? */
		if (need_training)
			ret = ddr_init(dram_info);
		else
			ret = 0;
	}

	/* If we failed to identify the DDR, or the parameters returned from
	 * spl_identify_ddr caused in DDR training failure - fall back to a
	 * generic way to train the DDR.
	 */
	if (ret == -1) {
		if (!spl_generic_ddr_init())
			hang(); //Could not init the DDR - nothing we can do..
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
	arch_misc_init();

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
