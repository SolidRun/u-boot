#include <common.h>
#include <init.h>
#include <asm/io.h>
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

// VBUS: P4_0 and P42_0
static void board_usb_init(int pin_type)
{
	/*Enable USB*/
	(*(volatile u32 *)CPG_RST_USB) = 0x000f000f;
	(*(volatile u32 *)CPG_CLKON_USB) = 0x000f000f;

	// /* Setup  */
	// /* Disable GPIO Write Protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 7); /* PWPR.BOWI = 0 */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 6);	 /* PWPR.PFCWE = 1 */

	if (pin_type == VBUS_OUT_OD || CONFIG_IS_ENABLED(SOLIDRUN_VBUS_OUT_OD))
	{
		pr_info("Using VBUS open-drain \n");
		/* Humming board has pulled up signals, enabled by default */
		/* set P4_0 as GPIO Input */
		(*(volatile u8 *)PFC_PM14) = 0;
		/* set P42_0 as GPIO Input */
		(*(volatile u8 *)PFC_PM3A) = 0;
	}
	else if (pin_type == VBUS_OUT_PP || CONFIG_IS_ENABLED(SOLIDRUN_VBUS_OUT_PP))
	{
		pr_info("Using VBUS push-pull\n");
		/* set P4_0 as GPIO Output High VBUSEN */
		(*(volatile u8 *)PFC_PM14) |= (0x1u << 1);
		(*(volatile u8 *)PFC_P14) |= (0x1u << 0);
		// /* set P42_0 as GPIO Output High */
		(*(volatile u8 *)PFC_PM3A) |= (0x1u << 1);
		(*(volatile u8 *)PFC_P3A) |= (0x1u << 0);
	}

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

int board_check_sd_emmc(void)
{
	int value = 0;
	/* Read SD0_DEV_SEL_SW value - P22_1 */
	/* eMMC/uSD Device Select - SD0_DEV_SEL_SW (LOW: uSD ; HIGH: eMMC) */

	generic_clear_bit(1, PFC_PMC26); /* P22_1 Port GPIO mode */
	generic_set_bit(2, PFC_PM26);	 /* P22_1 GPIO input mode */

	value = ((u32)(((*(volatile u32 *)(PFC_PIN26)) & (1 << 1))) != 0); /* Port 22[1] read input value */
	if (value == 0 || CONFIG_IS_ENABLED(SOLIDRUN_FORCE_SD_BOOT)) // Note: sd is LOW in g2l.
		return 1;

	return 0;
}

void board_select_sd_emmc(int select_sd)
{
	if (select_sd == 0 || CONFIG_IS_ENABLED(SOLIDRUN_FORCE_EMMC_BOOT))
	{
		printf("%s: select emmc.\n", __func__);
		/* Enable eMMC */
		/* Set SD0 VDD = 1.8v -> PFC-eMMC - LDO_SEL1 (High: 3.3v ; Low: 1.8v) */
		generic_clear_bit(1, PFC_PMC26); /* P22_1 Port GPIO mode */
		generic_set_bit(3, PFC_PM26);	 /* P22_1 GPIO output mode */
		generic_set_bit(1, PFC_P26);	 /* P22_1 GPIO out HIGH */

		/* Select eMMC */
		generic_clear_bit(0, PFC_PMC37); /* P39_0 Port GPIO mode */
		generic_set_bit(1, PFC_PM37);	 /* P39_0 GPIO output mode */
		generic_clear_bit(0, PFC_P37);	 /* P39_0 GPIO out LOW */
	}
	else if (select_sd != 0 || CONFIG_IS_ENABLED(SOLIDRUN_FORCE_SD_BOOT))
	{
		printf("%s: select uSD.\n", __func__);
		/* Enable uSD */
		generic_clear_bit(1, PFC_PMC26); /* P22_1 Port GPIO mode */
		generic_set_bit(3, PFC_PM26);	 /* P22_1 GPIO output mode */
		generic_clear_bit(1, PFC_P26);	 /* P22_1 GPIO out LOW */

		/* Select uSD */
		generic_clear_bit(0, PFC_PMC37); /* P39_0 Port GPIO mode */
		generic_set_bit(1, PFC_PM37);	 /* P39_0 GPIO output mode */
		generic_set_bit(0, PFC_P37);	 /* P39_0 GPIO out HIGH */
	}
}

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

static void carrier_usb_init(int carrier)
{
	switch (carrier)
	{
	case CARRIER_HB_MATE:
	case CARRIER_HB_RIPPLE:
	case CARRIER_HB_PULSE:
	case CARRIER_HB_EXTENDED:
		board_usb_init(VBUS_OUT_OD);
		break;
	default:
		board_usb_init(VBUS_OUT_PP);
		break;
	}
}

static void carrier_select_fdt(int carrier)
{
	pr_info("Selecting fdt file for board %d...\n", carrier);
	switch (carrier)
	{
	case CARRIER_HB_MATE:
	case CARRIER_HB_RIPPLE:
	case CARRIER_HB_PULSE:
		env_set("fdtfile", "rzv2l-hummingboard-ripple.dtb");
		break;
	case CARRIER_HB_EXTENDED:
		env_set("fdtfile", "rzv2l-hummingboard-extended.dtb");
		break;
	default:
		pr_warn("Leaving default fdtfile \n");
		break;
	}
}

int board_late_init(void)
{
#ifndef CONFIG_SOLIDRUN_DISABLE_TLV
	int carrier = get_carrier();
	if (carrier < 0)
	{
		pr_err("Can't recognize the carrier board \n");
	}
	carrier_usb_init(carrier);
	carrier_select_fdt(carrier);
#else
	board_usb_init(0);
#endif

	rzg_sd_emmc_init();

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_SYSTEM_SETUP)

int ft_system_setup(void *blob, struct bd_info *bd)
{
	return rzg_preboot_sd_emmc_setup(blob, bd);
}

void ft_board_setup_ex(void *blob, struct bd_info *bd) {}
#endif

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
