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
#include <fdt_support.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>
#include <env.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

#define PFC_BASE 0x11030000

#define ETH_CH0 (PFC_BASE + 0x300c)
#define I2C_CH1 (PFC_BASE + 0x1870)
#define ETH_PVDD_3300 0x00
#define ETH_PVDD_1800 0x01
#define ETH_PVDD_2500 0x02
#define ETH_MII_RGMII (PFC_BASE + 0x3018)

/* CPG */
#define CPG_BASE 0x11010000
#define CPG_CLKON_BASE (CPG_BASE + 0x500)
#define CPG_RESET_BASE (CPG_BASE + 0x800)
#define CPG_RESET_ETH (CPG_RESET_BASE + 0x7C)
#define CPG_RESET_I2C (CPG_RESET_BASE + 0x80)
#define CPG_PL2_SDHI_DSEL (CPG_BASE + 0x218)
#define CPG_CLK_STATUS (CPG_BASE + 0x280)
#define CPG_RST_USB (CPG_BASE + 0x878)
#define CPG_CLKON_USB (CPG_BASE + 0x578)

/* PFC */
#define PFC_P37 (PFC_BASE + 0x037)
#define PFC_PM37 (PFC_BASE + 0x16E)
#define PFC_PMC37 (PFC_BASE + 0x237)

#define PFC_P26 (PFC_BASE + 0x026)	 /* PORT REGISTER26 */
#define PFC_PM26 (PFC_BASE + 0x14C)	 /* PORT MODE REGISTER26 */
#define PFC_PMC26 (PFC_BASE + 0x226) /* PORT MODE CONTROL REGISTER26 */
#define PFC_PIN26 (PFC_BASE + 0x826)
#define PFC_PUPD26 (PFC_BASE + 0x1d30) /* PULL UP/PULL DOWN SWITCHING REGISTER26 */
#define FILONOFF26 (PFC_BASE + 0x2130) /* Digital Noise Filter Switching Register */
#define FILNUM26 (PFC_BASE + 0x2530)   /* DIGITAL NOISE FILTER NUMBER REGISTER26 */

#define PFC_PWPR (PFC_BASE + 0x3014)

#define PFC_P14 (PFC_BASE + 0x0014)
#define PFC_PM14 (PFC_BASE + 0x0128)
#define PFC_PMC14 (PFC_BASE + 0x214)
#define PFC_PFC14 (PFC_BASE + 0x450)
#define PFC_P15 (PFC_BASE + 0x0015)
#define PFC_PM15 (PFC_BASE + 0x012A)
#define PFC_PMC15 (PFC_BASE + 0x215)
#define PFC_PFC15 (PFC_BASE + 0x454)
#define PFC_PMC3A (PFC_BASE + 0x23a)
#define PFC_PFC3A (PFC_BASE + 0x4e8)

#define USBPHY_BASE (0x11c40000)
#define USB0_BASE (0x11c50000)
#define USB1_BASE (0x11c70000)
#define USBF_BASE (0x11c60000)
#define USBPHY_RESET (USBPHY_BASE + 0x000u)
#define COMMCTRL 0x800
#define HcRhDescriptorA 0x048
#define LPSTS 0x102

/* WDT */
#define WDT_BASE 0x12800800
#define WDTCNT 0x00
#define WDTSET 0x04
#define WDTTIM 0x08
#define WDTINT 0x0C
#define PECR 0x10
#define PEEN 0x14
#define WDTCNT_WDTEN BIT(0)
#define WDTINT_INTDISP BIT(0)

/**
 * The Hummingboard requires Open-Drain VBUS signals.
 * Comment the line below to enable Push-Pull signals instead.
 * TODO: remove this macro and change signal type based on TLV info.
 */
#define USB_VBUS_OD

/* eMMC/SD Auto detecte*/
#define EMMC_SD_AUTO 1
/* force select eMMC*/
#define SEL_EMMC 0
/* force select uSD*/
#define SEL_SD 0

static int check_sd_emmc_select(void)
{
	int value = 0;
	/* Read SD0_DEV_SEL_SW value - P22_1 */
	/* eMMC/uSD Device Select - SD0_DEV_SEL_SW (High: uSD ; Low: eMMC) */

	generic_clear_bit(1, PFC_PMC26); /* P22_1 Port GPIO mode */
	generic_set_bit(2, PFC_PM26);	 /* P22_1 GPIO input mode */

	value = ((u32)(((*(volatile u32 *)(PFC_PIN26)) & (1 << 1))) != 0); /* Port 22[1] read input value */
	printf("SD0_DEV_SEL_SW = %d \n", value);
	if (value == 1 || SEL_SD == 1)
		return 1;

	return 0;
}

static void select_sd_emmc(int select_sd)
{
	if (select_sd == 0 || SEL_EMMC == 1)
	{
		printf("%s: select emmc.\n", __func__);
		/* Enable eMMC */
		/* Set SD0 VDD = 1.8v -> PFC-eMMC - LDO_SEL1 (High: 3.3v ; Low: 1.8v) */
		generic_clear_bit(1, PFC_PMC26); /* P39_0 Port GPIO mode */
		generic_set_bit(3, PFC_PM26);	 /* P39_0 GPIO output mode */
		generic_clear_bit(1, PFC_P26);	 /* P39_0 GPIO out LOW */

		/* Select eMMC */
		generic_clear_bit(0, PFC_PMC37); /* P22_1 Port GPIO mode */
		generic_set_bit(1, PFC_PM37);	 /* P22_1 GPIO output mode */
		generic_clear_bit(0, PFC_P37);	 /* P22_1 GPIO out LOW */
	}
	else if (select_sd != 0 || SEL_SD == 1)
	{
		printf("%s: select uSD.\n", __func__);
		/* Enable uSD */
		generic_clear_bit(1, PFC_PMC26); /* P39_0 Port GPIO mode */
		generic_set_bit(3, PFC_PM26);	 /* P39_0 GPIO output mode */
		generic_set_bit(1, PFC_P26);	 /* P39_0 GPIO out HIGH */

		/* Select uSD */
		generic_clear_bit(0, PFC_PMC37); /* P22_1 Port GPIO mode */
		generic_set_bit(1, PFC_PM37);	 /* P22_1 GPIO output mode */
		generic_set_bit(0, PFC_P37);	 /* P22_1 GPIO out HIGH */
	}
}

static void set_bootsource_env(int select_sd)
{
	int ret;
	if (select_sd)
		ret = env_set("boot_source", "sd");
	else
		ret = env_set("boot_source", "emmc");
	if (ret)
		pr_err("Failed to set boot_source env, err: %d \n", ret);
}

static void board_sd_emmc_init(void)
{
	/* Select eMMC/uSD based on SD0_DEV_SEL_SW (P22_1) GPIO value {High: uSD ; Low: eMMC}*/
	int value = check_sd_emmc_select();
	select_sd_emmc(value);
	set_bootsource_env(value);
}

void s_init(void)
{
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0)&0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for ETH0 */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII)&0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x30001;
	/* I2C CLK */
	*(volatile u32 *)(CPG_RESET_I2C) = 0xF000F;
	/* I2C pin non GPIO enable */
	*(volatile u32 *)(I2C_CH1) = 0x01010101;
	/* SD CLK */
	*(volatile u32 *)(CPG_PL2_SDHI_DSEL) = 0x00110011;
	while (*(volatile u32 *)(CPG_CLK_STATUS) != 0)
		;
}

static void board_usb_init(void)
{
	/*Enable USB*/
	(*(volatile u32 *)CPG_RST_USB) = 0x000f000f;
	(*(volatile u32 *)CPG_CLKON_USB) = 0x000f000f;

	// /* Setup  */
	// /* Disable GPIO Write Protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 7); /* PWPR.BOWI = 0 */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 6);	 /* PWPR.PFCWE = 1 */

#ifdef USB_VBUS_OD
	/* Humming board has pulled up signals, enabled by default */
	/* set P4_0 as GPIO Input */
	(*(volatile u8 *)PFC_PM14) = 0;
	/* set P5_0 as GPIO Input */
	(*(volatile u8 *)PFC_PM15) = 0;
#elif
	/* set P4_0 as GPIO Output High VBUSEN */
	(*(volatile u8 *)PFC_PM14) |= (0x1u << 1);
	(*(volatile u8 *)PFC_P14) |= (0x1u << 0);
	// /* set P5_0 as GPIO Output High */
	(*(volatile u8 *)PFC_PM15) |= (0x1u << 1);
	(*(volatile u8 *)PFC_P15) |= (0x1u << 0);

#endif

	// /* Enable write protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 6); /* PWPR.PFCWE = 0 */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 7);	 /* PWPR.BOWI = 1 */

	/*Enable 2 USB ports*/
	(*(volatile u32 *)USBPHY_RESET) = 0x00001000u;
	/*USB0 is HOST*/
	(*(volatile u32 *)(USB0_BASE + COMMCTRL)) = 0;
	/*USB1 is HOST*/
	(*(volatile u32 *)(USB1_BASE + COMMCTRL)) = 0;
	/* Set USBPHY normal operation (Function only) */
	(*(volatile u16 *)(USBF_BASE + LPSTS)) |= (0x1u << 14); /* USBPHY.SUSPM = 1 (func only) */
	/* Overcurrent is not supported */
	(*(volatile u32 *)(USB0_BASE + HcRhDescriptorA)) |= (0x1u << 12); /* NOCP = 1 */
	(*(volatile u32 *)(USB1_BASE + HcRhDescriptorA)) |= (0x1u << 12); /* NOCP = 1 */
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_SYSTEM_SETUP)
/*
 * Configure the correct sdhi0 node (eMMC/SD) in device-tree:
 *  Set up board-specific details in device tree before boot
 */

static bool preboot_check_sd_emmc(void)
{
	int sd_select = 0;
	char *boot_source_env = from_env("boot_source");
	if (!boot_source_env)
	{
		sd_select = check_sd_emmc_select();
	}
	else if (strcmp(boot_source_env, "sd") == 0)
	{
		sd_select = 1;
	}

	return (sd_select || SEL_SD) && !SEL_EMMC;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
#if EMMC_SD_AUTO
	int ret, node_sdhi0, node = 0;
	bool enable_sdhc = preboot_check_sd_emmc();

	if (enable_sdhc)
	{
		printf("patching DTS | Select uSD...\n");
		/* dts changes (
		set | gpio-sd0-dev-sel-emmc-hog | replace output-low with output-high
		set | gpio-sd0-vdd-18v-hog | replace output-low with output-high
		------------------------------------------------
		# sdhi0 ->
		set | pinctrl-0 = <&sdhi0_pins>;
		set | pinctrl-1 = <&sdhi0_pins_uhs>;
		set | vqmmc-supply = <&vccq_sdhi0>;
		set | bus-width = <4>;
		add | max-frequency = <50000000>;
		remove | mmc-hs200-1_8v;
		remove | non-removable;
		remove | fixed-emmc-driver-type = <1>;
		*/

		/* select uSD and set SD0_VDD=3.3V */
		node = fdt_path_offset(blob, "/soc/pinctrl@11030000/gpio-sd0-dev-sel-emmc-hog");

		ret = fdt_delprop(blob, node, "output-high");
		if (ret < 0 && enable_sdhc)
			pr_err("%s: failed to disable gpio-sd0-dev-sel-emmc-hog in dtb!\n", __func__);

		ret = fdt_setprop_empty(blob, node, "output-low");
		if (ret < 0 && enable_sdhc)
			pr_err("%s: failed to set output-low -> gpio-sd0-dev-sel-emmc-hog in dtb!\n", __func__);

		node = fdt_path_offset(blob, "/soc/pinctrl@11030000/gpio-sd0-vdd-18v-hog");
		ret = fdt_delprop(blob, node, "output-high");
		if (ret < 0 && enable_sdhc)
			pr_err("%s: failed to delete output-high gpio-sd0-vdd-18v-hog in dtb!\n", __func__);

		ret = fdt_setprop_empty(blob, node, "output-low");
		if (ret < 0 && enable_sdhc)
			pr_err("%s: failed to set output-low -> gpio-sd0-vdd-18v-hog in dtb!\n", __func__);

		/* update sdhi0 settings (SD/eMMC) mmc@11c00000 */
		node_sdhi0 = fdt_path_offset(blob, "/soc/mmc@11c00000");

		ret = fdt_setprop_u32(blob, node_sdhi0, "bus-width", 4);
		if (ret < 0 && enable_sdhc)
			pr_err("%s : failed to set bus-width at node mmc@11c00000 in dtb!\n", __func__);

		ret = fdt_setprop_u32(blob, node_sdhi0, "max-frequency", 50000000);
		if (ret < 0 && enable_sdhc)
			pr_err("%s : failed to set max-frequency at node mmc@11c00000 in dtb!\n", __func__);

		ret = fdt_delprop(blob, node_sdhi0, "mmc-hs200-1_8v");
		if (ret < 0 && enable_sdhc)
			pr_err("%s : ffailed to set mmc-hs200-1_8v at node mmc@11c00000 in dtb!\n", __func__);

		ret = fdt_delprop(blob, node_sdhi0, "non-removable");
		if (ret < 0 && enable_sdhc)
			pr_err("%s : failed to set non-removable at node mmc@11c00000 in dtb!\n", __func__);

		ret = fdt_delprop(blob, node_sdhi0, "fixed-emmc-driver-type");
		if (ret < 0 && enable_sdhc)
			pr_err("%s : failed to set fixed-emmc-driver-type at node mmc@11c00000 in dtb!\n", __func__);
	}
	else
	{
		printf("patching DTS | keep default settings \n");
	}
#endif

	return 0;
}

void ft_board_setup_ex(void *blob, struct bd_info *bd)
{
}

#endif

int board_early_init_f(void)
{

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}

int board_late_init(void)
{
	board_usb_init();
#if EMMC_SD_AUTO
	board_sd_emmc_init();
#endif

	return 0;
}

static void wdt_write(u32 val, unsigned int reg)
{
	writel(val, WDT_BASE + reg);
}

static int reset_wdt_start(void)
{
	/* Clear Lapsed Time Register and clear Interrupt */
	wdt_write(WDTINT_INTDISP, WDTINT);
	/* 2 consecutive overflow cycle needed to trigger reset */
	wdt_write(0, WDTSET);
	/* Initialize watchdog counter register */
	wdt_write(0, WDTTIM);
	/* Enable watchdog timer*/
	wdt_write(WDTCNT_WDTEN, WDTCNT);

	return 0;
}

void reset_cpu(void)
{
	reset_wdt_start();
}
