// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright 2019, 2025 SolidRun ltd.
 */
#include <common.h>
#include <efi_loader.h>
#include <env.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <i2c.h>
#include <asm/io.h>
#include <usb.h>
#include <tlv_eeprom.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MM_PAD_UART2_RXD_UART2_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MM_PAD_UART2_TXD_UART2_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const hdmi_pads[] = {
	IMX8MM_PAD_SAI5_RXD1_GPIO3_IO22  | MUX_PAD_CTRL(0),
};

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX8MM-EVK-RAW",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 2=flash-bin raw 0x42 0x2000 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	imx_iomux_v3_setup_multiple_pads(hdmi_pads, ARRAY_SIZE(hdmi_pads));

	init_uart_clk(1);

	return 0;
}

#if IS_ENABLED(CONFIG_FEC_MXC)
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

#ifndef CONFIG_DM_ETH
	/* enable rgmii rxc skew and phy mode select to RGMII copper */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);
#endif

	return 0;
}
#endif

int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		/* TODO: from TLV */
		env_set("board_name", "EVK");
		env_set("board_rev", "iMX8MM");
	}

	return 0;
}

#ifdef CONFIG_ANDROID_SUPPORT
bool is_power_key_pressed(void) {
	return (bool)(!!(readl(SNVS_HPSR) & (0x1 << 6)));
}
#endif

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0; /* TODO */
}
#endif /* CONFIG_ANDROID_RECOVERY */
#endif /* CONFIG_FSL_FASTBOOT */

#ifdef CONFIG_ANDROID_BOOT_IMAGE
__weak int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}
#endif

int board_phys_sdram_size(phys_size_t *size)
{
	/* spl stored size for later use, restore here */
	phys_size_t *_size = (void *)0x7e0000;

	*size = *_size;

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP) || defined(CONFIG_OF_BOARD_FIXUP)
/* sample hdmi encoder power-down signal initial state to derive i2c address and line polarity */
static int find_hdmi_encoder(uint8_t *addr_encoder, uint8_t *addr_encoder_cec, uint8_t *addr_encoder_edid, uint8_t *addr_encoder_pkt, bool *pd_polarity) {
	struct gpio_desc desc;
	int ret;

	ret = dm_gpio_lookup_name("GPIO3_22", &desc);
	if (ret)
		return ret;

	ret = dm_gpio_request(&desc, "hdmi-pd");
	if (ret)
		return ret;
	dm_gpio_set_dir_flags(&desc, GPIOD_IS_IN);
	*pd_polarity = dm_gpio_get_value(&desc);

	if (*pd_polarity) {
		*addr_encoder = 0x3d;
		*addr_encoder_cec = 0x3c;
		*addr_encoder_edid = 0x3f;
		*addr_encoder_pkt = 0x38;
	} else {
		*addr_encoder = 0x39;
		*addr_encoder_cec = 0x38;
		*addr_encoder_edid = 0x3b;
		*addr_encoder_pkt = 0x34;
	}

	return 0;
}

/*
 * Patch device-tree for HDMI encoder (i2c address depending on optional R185)
 */
static int board_fix_hdmi(void *fdt, const char *stage) {
	uint8_t addr_encoder, addr_encoder_cec, addr_encoder_edid, addr_encoder_pkt;
	bool pd_polarity;
	int node_encoder;
	const char *node_encoder_path[] = {
		"/i2c@30a40000/adv7535@39",
		"/i2c@30a40000/adv7535@3d",
		"/soc@0/bus@30800000/i2c@30a40000/hdmi@3d"
	};
	int ret;
	u32 pd_gpios_prop[3];

	/* detect encoder */
	ret = find_hdmi_encoder(&addr_encoder, &addr_encoder_cec, &addr_encoder_edid, &addr_encoder_pkt, &pd_polarity);
	if (ret) {
		pr_err("%s: couldn't detect hdmi encoder, not patching %s dtb: %d!\n", __func__, stage, ret);
		return 0;
	}
	printf("%s: Found HDMI encoder at 0x%x!\n", __func__, addr_encoder);

	/* patch known fdt nodes with probed address */
	for (uint8_t i = 0; i < ARRAY_SIZE(node_encoder_path); i++) {
		node_encoder = fdt_path_offset(fdt, node_encoder_path[i]);
		if(node_encoder < 0)
			continue;

		ret = fdt_setprop_u32(fdt, node_encoder, "reg", addr_encoder);
		fdt_setprop_u32(fdt, node_encoder, "adi,addr-cec", addr_encoder_cec);
		fdt_setprop_u32(fdt, node_encoder, "adi,addr-edid", addr_encoder_edid);
		fdt_setprop_u32(fdt, node_encoder, "adi,addr-pkt", addr_encoder_pkt);

		ret = fdtdec_get_int_array(fdt, node_encoder, "pd-gpios", pd_gpios_prop, ARRAY_SIZE(pd_gpios_prop));
		if (!ret) {
			/* <phandle> <num> <flags>: set flag active-high or active-low */
			pd_gpios_prop[0] = cpu_to_fdt32(pd_gpios_prop[0]);
			pd_gpios_prop[1] = cpu_to_fdt32(pd_gpios_prop[1]);
			pd_gpios_prop[2] = cpu_to_fdt32(pd_polarity);
			ret = fdt_setprop(fdt, node_encoder, "pd-gpios", pd_gpios_prop, sizeof(pd_gpios_prop));
		}
	}

	if(ret < 0)
		pr_err("%s: failed to patch hdmi encoder address in %s dtb!\n", __func__, stage);

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
/* Patch device-tree for OS */
int ft_board_setup(void *fdt, struct bd_info *bd) {
	board_fix_hdmi(fdt, "os");

	return 0;
}
#endif
#ifdef CONFIG_OF_BOARD_FIXUP
/* Patch device-tree for U-Boot */
int board_fix_fdt(void *fdt) {
	board_fix_hdmi(fdt, "uboot");

	return 0;
}
#endif
#endif
