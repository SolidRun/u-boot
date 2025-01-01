// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 * Copyright 2025 Josua Mayer <josua@solid-run.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx8mn_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <env.h>
#include <mmc.h>
#include <netdev.h>
#include <phy.h>

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static const iomux_v3_cfg_t uart_pads[] = {
	IMX8MN_PAD_UART2_RXD__UART2_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MN_PAD_UART2_TXD__UART2_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static const iomux_v3_cfg_t wdog_pads[] = {
	IMX8MN_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	/* configure GPIO1_IO02 pad connected to pmic for WDOG_B function */
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	/* configure watchdog to toggle WDOG_B signal on expire */
	set_wdog_reset(wdog);

	/* configure console uart pads */
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	/* start uart clock to activate console */
	init_uart_clk(1);

	return 0;
}

static int find_ethernet_phy(void)
{
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int phy_addr = -ENOENT;

	if (IS_ENABLED(CONFIG_FEC_MXC)) {
		bus = fec_get_miibus(ENET1_BASE_ADDR, -1);
		if (!bus)
			return -ENOENT;

		// scan address 0, 4
		phydev = phy_find_by_mask(bus, 0b00010001);
		if (!phydev)
			return -ENOENT;
		pr_debug("%s: detected ethernet phy at address %d\n", __func__, phydev->addr);
		phy_addr = phydev->addr;
	}

	return phy_addr;
}

/*
 * Configure the correct ethernet PHYs nodes in device-tree:
 * - AR8035 at addresses 4: SolidSense N8 Compact v1.0+
 * - ADIN1300 at address 0: SolidSense N8 Compact v1.2+
 */
int ft_board_setup(void *fdt, struct bd_info *bd)
{
	int node_fec, node_phy0, node_phy4;
	int ret, phy;
	bool enable_phy0 = false, enable_phy4 = false;
	u32 phy_handle;

	// detect phy
	phy = find_ethernet_phy();
	switch (phy) {
	case 0:
		enable_phy0 = true;
		break;
	case 4:
		enable_phy4 = true;
		break;
	default:
		pr_err("%s: couldn't detect ethernet phy, not patching dtb!\n", __func__);
		return 0;
	}

	// update all phy nodes status
	node_phy0 = fdt_path_offset(fdt, "/soc@0/bus@30800000/ethernet@30be0000/mdio/ethernet-phy@0");
	ret = fdt_setprop_string(fdt, node_phy0, "status", enable_phy0 ? "okay" : "disabled");
	if (ret < 0 && enable_phy0)
		pr_err("%s: failed to enable ethernet phy at address 0 in dtb!\n", __func__);
	node_phy4 = fdt_path_offset(fdt, "/soc@0/bus@30800000/ethernet@30be0000/mdio/ethernet-phy@4");
	ret = fdt_setprop_string(fdt, node_phy4, "status", enable_phy4 ? "okay" : "disabled");
	if (ret < 0 && enable_phy4)
		pr_err("%s: failed to enable ethernet phy at address 4 in dtb!\n", __func__);

	// update phy-handle
	phy_handle = fdt_get_phandle(fdt, enable_phy0 ? node_phy0 : node_phy4);
	node_fec = fdt_path_offset(fdt, "/soc@0/bus@30800000/ethernet@30be0000");
	ret = fdt_setprop_u32(fdt, node_fec, "phy-handle", phy_handle);
	if (ret < 0)
		pr_err("%s: failed to update phy-handle in dtb!\n", __func__);

	return 0;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG))
		env_set("board_name", "SolidSense N8 Compact");

	return 0;
}

#if CONFIG_IS_ENABLED(ANDROID_BOOT_IMAGE)
int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}
#endif /* CONFIG_IS_ENABLED(ANDROID_BOOT_IMAGE) */
