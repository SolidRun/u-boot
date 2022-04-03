// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <efi_loader.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/delay.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <spl.h>
#include <asm/mach-imx/dma.h>
#include <power/pmic.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <imx_sip.h>
#include <linux/arm-smccc.h>
#include <mmc.h>
#include <malloc.h>
#include <fsl_esdhc.h>
#include <power/bd71837.h>
#include <asm/mach-imx/video.h>
#include <linux/delay.h>
#include <env.h>
#include <tlv_eeprom.h>

#define ONE_GB 0x40000000ULL

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static iomux_v3_cfg_t const uart_pads[] = {
	MX8MP_PAD_UART2_RXD__UART2_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_UART2_TXD__UART2_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

#ifdef CONFIG_NAND_MXS

static void setup_gpmi_nand(void)
{
	init_nand_clk();
}
#endif

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX8MP-SR-RAW",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 2=flash-bin raw 0 0x2000 mmcpart 1",
	.images = fw_images,
};

u8 num_image_type_guids = ARRAY_SIZE(fw_images);
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	init_uart_clk(1);

	return 0;
}

int check_mirror_ddr_tmp(unsigned int addr_1, unsigned int addr_2)
{
	/* return 1 if mirror detected between addr_1 and addre_2, else return 0*/
	int retrain_tmp;
	unsigned int save1, save2, mirror;
	volatile unsigned int *ptr;

	retrain_tmp = 0;
	ptr = (volatile unsigned int *)CONFIG_SYS_SDRAM_BASE;
	save1 = ptr[addr_1];
	save2 = ptr[addr_2];
	ptr[addr_2] = save1 << 1;
	ptr[addr_1] = ~save1;
	mirror = ptr[addr_2];
	if (mirror == ~save1) {
		retrain_tmp = 1;
	}
	ptr[addr_1] = save1;
	ptr[addr_2] = save2;

	// Check if mirror have detected
	if (retrain_tmp == 1)
		return 1;

	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	// Check Mirror for 1GB
	if (check_mirror_ddr_tmp(0, ONE_GB/4)) {
		*size = ONE_GB;
		return 0;
	}
	// Check Mirror for 2GB
	if (check_mirror_ddr_tmp(0, 2*ONE_GB/4)) {
		*size = 2*ONE_GB;
		return 0;
	}

	// Default size 3GByte
	*size = 3*ONE_GB;
	return 0;
}


#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_IMX8M_DRAM_INLINE_ECC
	int rc;
	phys_addr_t ecc0_start = 0xb0000000;
	phys_addr_t ecc1_start = 0x130000000;
	phys_addr_t ecc2_start = 0x1b0000000;
	size_t ecc_size = 0x10000000;

	rc = add_res_mem_dt_node(blob, "ecc", ecc0_start, ecc_size);
	if (rc < 0) {
		printf("Could not create ecc0 reserved-memory node.\n");
		return rc;
	}

	rc = add_res_mem_dt_node(blob, "ecc", ecc1_start, ecc_size);
	if (rc < 0) {
		printf("Could not create ecc1 reserved-memory node.\n");
		return rc;
	}

	rc = add_res_mem_dt_node(blob, "ecc", ecc2_start, ecc_size);
	if (rc < 0) {
		printf("Could not create ecc2 reserved-memory node.\n");
		return rc;
	}
#endif

	return 0;
}
#endif

#ifdef CONFIG_USB_DWC3

#define USB_PHY_CTRL0			0xF0040
#define USB_PHY_CTRL0_REF_SSP_EN	BIT(2)

#define USB_PHY_CTRL1			0xF0044
#define USB_PHY_CTRL1_RESET		BIT(0)
#define USB_PHY_CTRL1_COMMONONN		BIT(1)
#define USB_PHY_CTRL1_ATERESET		BIT(3)
#define USB_PHY_CTRL1_VDATSRCENB0	BIT(19)
#define USB_PHY_CTRL1_VDATDETENB0	BIT(20)

#define USB_PHY_CTRL2			0xF0048
#define USB_PHY_CTRL2_TXENABLEN0	BIT(8)

#define USB_PHY_CTRL6			0xF0058

#define HSIO_GPR_BASE			       (0x32F10000U)
#define HSIO_GPR_REG_0			      (HSIO_GPR_BASE)
#define HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN_SHIFT    (1)
#define HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN	  (0x1U << HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN_SHIFT)


static struct dwc3_device dwc3_device_data = {
#ifdef CONFIG_SPL_BUILD
	.maximum_speed = USB_SPEED_HIGH,
#else
	.maximum_speed = USB_SPEED_SUPER,
#endif
	.base = USB1_BASE_ADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.power_down_scale = 2,
};

int usb_gadget_handle_interrupts(int index)
{
	dwc3_uboot_handle_interrupt(index);
	return 0;
}

static void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3)
{
	u32 RegData;

	/* enable usb clock via hsio gpr */
	RegData = readl(HSIO_GPR_REG_0);
	RegData |= HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN;
	writel(RegData, HSIO_GPR_REG_0);

	/* USB3.0 PHY signal fsel for 100M ref */
	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData = (RegData & 0xfffff81f) | (0x2a<<5);
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL6);
	RegData &=~0x1;
	writel(RegData, dwc3->base + USB_PHY_CTRL6);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	RegData |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData |= USB_PHY_CTRL0_REF_SSP_EN;
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL2);
	RegData |= USB_PHY_CTRL2_TXENABLEN0;
	writel(RegData, dwc3->base + USB_PHY_CTRL2);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET);
	writel(RegData, dwc3->base + USB_PHY_CTRL1);
}
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
#define USB2_PWR_EN IMX_GPIO_NR(1, 14)
int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;
	imx8m_usb_power(index, true);

	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_nxp_usb_phy_init(&dwc3_device_data);
		return dwc3_uboot_init(&dwc3_device_data);
	} else if (index == 0 && init == USB_INIT_HOST) {
		return ret;
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int ret = 0;
	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_uboot_exit(index);
	}

	imx8m_usb_power(index, false);

	return ret;
}

#endif

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));
}

static int setup_eqos(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* set INTF as RGMII, enable RGMII TXC clock */
	clrsetbits_le32(&gpr->gpr[1],
			IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK, BIT(16));
	setbits_le32(&gpr->gpr[1], BIT(19) | BIT(21));

	return set_clk_eqos(ENET_125MHZ);
}

#if CONFIG_IS_ENABLED(NET)
int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

#define DISPMIX				13
#define MIPI				15

int board_init(void)
{
	struct arm_smccc_res res;

	if (CONFIG_IS_ENABLED(FEC_MXC)) {
		setup_fec();
	}

	if (CONFIG_IS_ENABLED(DWC_ETH_QOS)) {
		setup_eqos();
	}

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
	init_usb_clk();
#endif

	/* enable the dispmix & mipi phy power domain */
	arm_smccc_smc(IMX_SIP_GPC, IMX_SIP_GPC_PM_DOMAIN,
		      DISPMIX, true, 0, 0, 0, 0, &res);
	arm_smccc_smc(IMX_SIP_GPC, IMX_SIP_GPC_PM_DOMAIN,
		      MIPI, true, 0, 0, 0, 0, &res);

	return 0;
}

static bool find_i2c_dev(u8 i2c_bus, u8 address) {
	struct udevice *bus;
	struct udevice *i2c_dev = NULL;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, i2c_bus, &bus);
	if (ret) {
		pr_err("%s: failed to get i2c bus %u: %i\n", __func__, i2c_bus, ret);
		return false;
	}

	ret = dm_i2c_probe(bus, address, 0, &i2c_dev);
	if (ret) {
		return false;
	}

	return true;
}

/*
 * Read TLV formatted data from eeprom.
 * Only read as much data as indicated by the TLV header.
 */
// TODO: this should be a library function?!
static bool get_tlvinfo_from_eeprom(int index, u8 *buffer, size_t length) {
	struct tlvinfo_header *eeprom_hdr = (struct tlvinfo_header *) buffer;
	struct tlvinfo_tlv *eeprom_tlv = (struct tlvinfo_tlv *) &buffer[sizeof(struct tlvinfo_header)];

	if(length < TLV_INFO_HEADER_SIZE) {
		pr_err("%s: buffer too small for tlv header!\n", __func__);
		return false;
	}
	if(read_tlv_eeprom((void *)eeprom_hdr, 0, TLV_INFO_HEADER_SIZE, index) != 0) {
		pr_err("%s: failed to read from eeprom!\n", __func__);
		return false;
	}
	if(!is_valid_tlvinfo_header(eeprom_hdr)) {
		pr_warn("%s: invalid tlv header!\n", __func__);
		return false;
	}
	if(length - TLV_INFO_HEADER_SIZE < be16_to_cpu(eeprom_hdr->totallen)) {
		pr_err("%s: buffer too small for tlv data!\n", __func__);
		return false;
	}
	if(read_tlv_eeprom((void *)eeprom_tlv, sizeof(struct tlvinfo_header), be16_to_cpu(eeprom_hdr->totallen), index) != 0) {
		pr_err("%s: failed to read from eeprom!\n", __func__);
		return false;
	}

	return true;
}

static struct board_id {
	char carrier_name[32];
	char carrier_rev[3];
	char som_name[8];
	char som_rev[3];
} board_id = {0};

/*
 * Identify board from TLV EEPROM - store result in board_id:
 * - carrier_name: name of carrier
 * - carrier_rev: revision of carrier
 * - som_name: name of SoC
 * - som_rev: revision of SoM
 */
static void board_id_from_tlv_eeprom(void) {
	static u8 eeprom[TLV_INFO_MAX_LEN];
	char sku[257];
	char *tmp;

	for(int i = 0; i < 2;i++) {
		// read eeprom
		if(!get_tlvinfo_from_eeprom(i, eeprom, sizeof(eeprom))) {
			pr_info("%s: failed to read eeprom %d\n", __func__, i);
			continue;
		}

		// read sku
		if(!tlvinfo_read_tlv(eeprom, TLV_CODE_PART_NUMBER, (u8 *)sku, sizeof(sku))) {
			pr_warn("%s: could not find sku in eeprom\n", __func__);
			continue;
		}
		pr_debug("%s: read sku %s\n", __func__, sku);

		// parse sku - processor or carrier indicated at index 2-6
		if(memcmp(&sku[2], "HBC", 3) == 0) {
			// HummingBoard
			switch(sku[5]) {
			    case 'M': // Mate
				tmp = "mate";
				break;
			    case 'R': // Ripple
				tmp = "ripple";
				break;
			    case 'U': // Pulse
				tmp = "pulse";
				break;
			    default:
				pr_err("%s: did not recognise board variant '%c' in sku \"%s\"!\n", __func__, sku[5], sku);
				tmp = 0;
			}

			if(tmp) {
				if(snprintf(board_id.carrier_name, sizeof(board_id.carrier_name), "hummingboard-%s", tmp) >= sizeof(board_id.carrier_name)) {
					pr_err("%s: buffer too small, carrier_name skipped!\n", __func__);
					board_id.carrier_name[0] = 0;
				}
			}

			// board revision at index 12-13
			if(sku[12] && sku[13]) {
				board_id.carrier_rev[0] = sku[12];
				board_id.carrier_rev[1] = sku[13];
				board_id.carrier_rev[2] = 0;
			} else {
			    pr_err("%s: did not find board revision in sku \"%s\"!\n", __func__, sku);
			}
		} else if(memcmp(&sku[2], "MP8", 3) == 0) {
			// i.MX8MP SoM
			strcpy(board_id.som_name, "imx8mp");

			// variant
			switch(sku[5]) {
			    case 'D':
				break;
			    case 'Q':
				break;
			    default:
				pr_err("%s: did not recognise cpu variant '%c' in sku \"%s\"!\n", __func__, sku[5], sku);
			}

			// SoM revision at index 19-20
			if(sku[19] && sku[20]) {
				board_id.som_rev[0] = sku[19];
				board_id.som_rev[1] = sku[20];
				board_id.som_rev[2] = 0;
			} else {
			    pr_err("%s: did not find som revision in sku \"%s\"!\n", __func__, sku);
			}
		} else {
			pr_err("%s: did not recognise SKU %s!\n", __func__, sku);
		}
	}
}

int board_late_init(void)
{
	char fdtfile[48] = {0};
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	// identify device
	board_id_from_tlv_eeprom();

	// fall-back when identification fails
	if(!board_id.carrier_name[0]) {
		// could be HummingBoard or CuBox ...
		if(find_i2c_dev(2, 0x57)) {
			// if EEPROM exists, it must be HummingBoard
			printf("%s: could not identify board, defaulting to HummingBoard Pulse Revision 2.5!\n", __func__);
			strcpy(board_id.carrier_name, "hummingboard-pulse");
			strcpy(board_id.carrier_rev, "25");
		} else {
			// likely a CuBox
			printf("%s: could not identify board, defaulting to CuBox-M!\n", __func__);
			strcpy(board_id.carrier_name, "cubox-m");
		}
	}
	if(!board_id.som_name[0]) {
		// could be anything ...
		printf("%s: could not identify som, defaulting to i.MX8M Plus Revision 1.1!\n", __func__);
		strcpy(board_id.som_name, "imx8mp");
		strcpy(board_id.som_rev, "11");
	}

	// auto-select device-tree
	if (!env_get("fdtfile")) {
		if(snprintf(fdtfile, sizeof(fdtfile), "freescale/%s-%s.dtb", board_id.som_name, board_id.carrier_name) >= sizeof(fdtfile)) {
			pr_err("%s: buffer too small, fdtfile truncated!\n", __func__);
		}
		env_set("fdtfile", fdtfile);
	} else {
		printf("%s: fdtfile set in environment, keeping as is.\n", __func__);
	}

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	// expose identity to environment
	if(board_id.carrier_name[0])
		env_set("carrier_name", board_id.carrier_name);
	if(board_id.carrier_rev[0])
		env_set("carrier_rev", board_id.carrier_rev);
	if(board_id.som_name[0])
		env_set("som_name", board_id.som_name);
	if(board_id.som_rev[0])
		env_set("som_rev", board_id.som_rev);
#endif

	return 0;
}

/*
 * select board mac address for given interface
 */
int board_get_mac(int dev_id, unsigned char *mac) {
	char macenv[16] = {0};
	mac[0] = 0;

	/*
	 * Note: Environment ethaddr (eth1addr, eth2addr, ...) has first priority,
	 * therefore it should be read and returned here.
	 * However the fec driver will write the result from this function to the environment,
	 * causing a feedback loop.
	 */

	// fuses
	imx_get_mac_from_fuse(dev_id, mac);
	if(is_valid_ethaddr(mac)) {
		printf("%s: interface %i: using mac from fuses: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, dev_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return 0;
	}

	return -ENOENT;
}

#ifdef CONFIG_ANDROID_SUPPORT
bool is_power_key_pressed(void) {
	return (bool)(!!(readl(SNVS_HPSR) & (0x1 << 6)));
}
#endif

#ifdef CONFIG_SPL_MMC_SUPPORT
#define UBOOT_RAW_SECTOR_OFFSET 0x40
unsigned long spl_mmc_get_uboot_raw_sector(struct mmc *mmc)
{
	u32 boot_dev = spl_boot_device();
	switch (boot_dev) {
		case BOOT_DEVICE_MMC2:
			return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR - UBOOT_RAW_SECTOR_OFFSET;
		default:
			return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
	}
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
