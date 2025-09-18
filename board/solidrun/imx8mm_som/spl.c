// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019, 2021 NXP
 * Copyright 2019, 2025 SolidRun ltd.
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>
#include <asm/sections.h>

#include <power/pmic.h>
#include <power/bd71837.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <linux/delay.h>
#include <fsl_sec.h>
#include <asm/arch/ddr.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
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
	default:
		return BOOT_DEVICE_NONE;
	}
}

#define LPDDR4_MR_05			0x05
#define LPDDR4_MR_05_MFID_SAMSUNG	0x01
#define LPDDR4_MR_05_MFID_MICRON	0xFF
#define LPDDR4_MR_06			0x06
#define LPDDR4_MR_07			0x07
#define LPDDR4_MR_08			0x08
#define LPDDR4_MR_08_DENSITY_MASK	0b00111100
#define LPDDR4_MR_08_DENSITY_02G	0b00000000
#define LPDDR4_MR_08_DENSITY_03G	0b00000100
#define LPDDR4_MR_08_DENSITY_04G	0b00001000
#define LPDDR4_MR_08_DENSITY_06G	0b00001100
#define LPDDR4_MR_08_DENSITY_08G	0b00010000
#define LPDDR4_MR_08_DENSITY_12G	0b00010100
#define LPDDR4_MR_08_DENSITY_16G	0b00011000
#define LPDDR4_MR_08_DENSITY_UNK1	0b00011100
#define LPDDR4_MR_08_DENSITY_UNK2	0b00100000

#define SAMSUNG_MR_07_REVISION_MASK	0b11111110

extern struct dram_timing_info dram_timing_MT53D512M32D2;
extern struct dram_timing_info dram_timing_nxpevk;
extern struct dram_timing_info dram_timing_nxpevk_reduced;
extern struct dram_timing_info dram_timing_K4F8E3S4HD_MGCL;
extern struct dram_timing_info dram_timing_K4F6E3S4HM_SGLC;

void spl_dram_init(void)
{
	uint8_t bconf1, bconf2, bconf3, bconf4;
	uint8_t density;
	phys_size_t *size = (void *)0x7e0000;

	/*
	 * Initialise DDR with a limited compatible configuration
	 */
	ddr_init(&dram_timing_nxpevk_reduced);

	/*
	 * Identify DDR
	 */
	bconf1 = lpddr4_mr_read(0xF, LPDDR4_MR_05);
	bconf2 = lpddr4_mr_read(0xF, LPDDR4_MR_06);
	bconf3 = lpddr4_mr_read(0xF, LPDDR4_MR_07);
	bconf4 = lpddr4_mr_read(0xF, LPDDR4_MR_08);
	density = bconf4 & LPDDR4_MR_08_DENSITY_MASK;

	/*
	 * Reinitialise DDR with correct parameters
	 */
	switch (bconf1) {
		case LPDDR4_MR_05_MFID_SAMSUNG:
		{
			uint8_t revision1 = bconf2;
			uint8_t revision2 = bconf3 & SAMSUNG_MR_07_REVISION_MASK;
			printf("Samsung DDR Revision 0x%x 0x%x density 0x%x\n", revision1, revision2, density);

			if (density == LPDDR4_MR_08_DENSITY_08G) {
				/*
				 * probably Samsung K4F8E3S4HD-MGCL
				 * - 1GB total
				 * - 1 chipselect
				 * - 2 channels
				 *
				 * Found bconf2 = 0b00001010, bconf3 = 0b0000000X
				 */
				ddr_init(&dram_timing_K4F8E3S4HD_MGCL);
				*size = 0x40000000;
			} else if (density == LPDDR4_MR_08_DENSITY_UNK2) {
				/*
				 * probably Samsung K4F6E3S4HM-SGCL
				 * - 2GB total
				 * - 1 chipselect
				 * - 2 channels
				 *
				 * Found bconf2 = 0b00001010, bconf3 = 0b0010000X
				 */
				ddr_init(&dram_timing_K4F6E3S4HM_SGLC);
				*size = 0x80000000;
			} else {
				printf("Warning: can't identify Samsung DDR; attempting evk calibration as last resort.\n");
				ddr_init(&dram_timing_nxpevk);
				*size = 0x80000000;
			}
			break;
		}
		case LPDDR4_MR_05_MFID_MICRON:
		{
			uint8_t revision1 = bconf2;
			uint8_t revision2 = bconf3;
			printf("Micron DDR Revision 0x%x 0x%x density 0x%x\n", revision1, revision2, density);

			if (density == LPDDR4_MR_08_DENSITY_UNK2) {
				/*
				 * probably Micron MT53D512M32D2
				 * - 2GB total
				 * - 1 chipselect
				 * - 2 channels
				 *
				 * Found bconf2 = 0b00000000, bconf3 = 0b00000001
				 */
				ddr_init(&dram_timing_MT53D512M32D2);
				*size = 0x80000000;
			} else {
				printf("Warning: can't identify Micron DDR; attempting evk calibration as last resort.\n");
				ddr_init(&dram_timing_nxpevk);
				*size = 0x80000000;
			}
			break;
		}
		default:
			printf("Warning: can't identify ddr vendor 0x%x; attempting evk calibration as last resort.\n", bconf1);
			ddr_init(&dram_timing_nxpevk);
			*size = 0x80000000;
			break;
	}
}

#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = IMX8MM_PAD_I2C1_SCL_I2C1_SCL | PC,
		.gpio_mode = IMX8MM_PAD_I2C1_SCL_GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MM_PAD_I2C1_SDA_I2C1_SDA | PC,
		.gpio_mode = IMX8MM_PAD_I2C1_SDA_GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE | \
			 PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_DSE1)

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IMX8MM_PAD_NAND_WE_B_USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_WP_B_USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA04_USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA05_USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA06_USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA07_USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_RE_B_USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CE2_B_USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CE3_B_USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CLE_USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IMX8MM_PAD_SD2_CLK_USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD2_CMD_USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD2_DATA0_USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD2_DATA1_USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD2_DATA2_USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD2_DATA3_USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD2_RESET_B_GPIO2_IO19 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MM_PAD_SD2_CD_B_USDHC2_CD_B  | MUX_PAD_CTRL(0),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC2_BASE_ADDR, 0, 4},
	{USDHC3_BASE_ADDR, 0, 8},
};

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			init_clk_usdhc(1);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			imx_iomux_v3_setup_multiple_pads(
				usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
			break;
		case 1:
			init_clk_usdhc(2);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			imx_iomux_v3_setup_multiple_pads(
				usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(POWER_LEGACY)
#define I2C_PMIC	0
int power_init_board(void)
{
	struct pmic *p;
	int ret;

	ret = power_bd71837_init(I2C_PMIC);
	if (ret)
		printf("power init failed");

	p = pmic_get("BD71837");
	pmic_probe(p);


	/* decrease RESET key long push time from the default 10s to 10ms */
	pmic_reg_write(p, BD718XX_PWRONCONFIG1, 0x0);

	/* unlock the PMIC regs */
	pmic_reg_write(p, BD718XX_REGLOCK, 0x1);

	/* increase VDD_SOC to typical value 0.85v before first DRAM access */
	pmic_reg_write(p, BD718XX_BUCK1_VOLT_RUN, 0x0f);

	/* increase VDD_DRAM to 0.9v for 3Ghz DDR */
	/* TODO: EVK sets 0.975V */
	pmic_reg_write(p, BD718XX_1ST_NODVS_BUCK_VOLT, 0x2);

#ifndef CONFIG_IMX8M_LPDDR4
	/* increase NVCC_DRAM_1V2 to 1.2v for DDR4 */
	pmic_reg_write(p, BD718XX_4TH_NODVS_BUCK_VOLT, 0x28);
#endif

	/* lock the PMIC regs */
	pmic_reg_write(p, BD718XX_REGLOCK, 0x11);

	return 0;
}
#endif

void spl_board_init(void)
{
	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		if (sec_init())
			printf("\nsec_init failed!\n");
	}
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
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* Adjust pmic voltage to 1.0V for 800M */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
