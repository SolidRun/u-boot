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

#define PFC_BASE 0x11030000

#define ETH_CH0 (PFC_BASE + 0x300c)
#define ETH_CH1 (PFC_BASE + 0x3010)
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
#define CPG_RST_USB (CPG_BASE + 0x878)
#define CPG_CLKON_USB (CPG_BASE + 0x578)

/* PFC */
#define PFC_P37 (PFC_BASE + 0x037)
#define PFC_PM37 (PFC_BASE + 0x16E)
#define PFC_PMC37 (PFC_BASE + 0x237)
#define PFC_PWPR (PFC_BASE + 0x3014)

#define PFC_P14 (PFC_BASE + 0x0014)
#define PFC_PM14 (PFC_BASE + 0x0128)
#define PFC_PMC14 (PFC_BASE + 0x214)
#define PFC_PFC14 (PFC_BASE + 0x450)

#define PFC_P3A (PFC_BASE + 0x003A)
#define PFC_PM3A (PFC_BASE + 0x0174)
#define PFC_PMC3A (PFC_BASE + 0x23A)
#define PFC_PFC3A (PFC_BASE + 0x4E8)

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

void s_init(void)
{
	/* SD1 power control: P39_1 = 0; P39_2 = 1; */
	*(volatile u32 *)(PFC_PMC37) &= 0xFFFFFFF9;									   /* Port func mode 0b00 */
	*(volatile u32 *)(PFC_PM37) = (*(volatile u32 *)(PFC_PM37)&0xFFFFFFC3) | 0x28; /* Port output mode 0b1010 */
#if CONFIG_TARGET_RZG2L_SOLIDRUN
	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37)&0xFFFFFFF9) | 0x6; /* Port 39[2:1] output value 0b11*/
#else

	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37)&0xFFFFFFF9) | 0x4; /* Port 39[2:1] output value 0b10*/
#endif

	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0)&0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH_CH1) = (*(volatile u32 *)(ETH_CH1)&0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII)&0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x30003;
	/* I2C CLK */
	*(volatile u32 *)(CPG_RESET_I2C) = 0xF000F;
	/* I2C pin non GPIO enable */
	*(volatile u32 *)(I2C_CH1) = 0x01010101;
}

// VBUS: P4_0 and P42_0
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
	/* set P42_0 as GPIO Input */
	(*(volatile u8 *)PFC_PM3A) = 0;
#elif
	/* set P4_0 as GPIO Output High VBUSEN */
	(*(volatile u8 *)PFC_PM14) |= (0x1u << 1);
	(*(volatile u8 *)PFC_P14) |= (0x1u << 0);
	// /* set P42_0 as GPIO Output High */
	(*(volatile u8 *)PFC_PM3A) |= (0x1u << 1);
	(*(volatile u8 *)PFC_P3A) |= (0x1u << 0);

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
	return 0;
}

static void wdt_write(u32 val, unsigned int reg)
{
	writel(val, WDT_BASE + reg);
}

static int reset_wdt_start()
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
