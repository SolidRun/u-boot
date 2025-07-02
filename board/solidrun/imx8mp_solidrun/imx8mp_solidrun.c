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
#include <asm/arch/ddr.h>
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
#include <asm/mach-imx/video.h>
#include <linux/delay.h>
#include <env.h>
#include <tlv_eeprom.h>
#include "../common/tlv_data.h"

#define ONE_GB 0x40000000ULL

static struct tlv_data hb_tlv_data[2];
int hb_tlv_ret[2];
static bool tlv_read_once;

static struct board_id {
	const char *carrier_name;
	char carrier_rev[3];
	const char *som_name;
	char som_rev[3];
	const char *product_name;
	char product_rev[3];
} board_id = {0};

static void hb_read_tlv_data(void)
{
	if (tlv_read_once)
		return;
	tlv_read_once = true;

	hb_tlv_ret[0] = read_tlv_data(0, &hb_tlv_data[0]);
	hb_tlv_ret[1] = read_tlv_data(1, &hb_tlv_data[1]);
}

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

int check_mirror_ddr_tmp(unsigned int addr_1, unsigned int addr_2)
{
	/* return 1 if mirror detected between addr_1 and addre_2, else return 0*/
	int retrain_tmp;
	unsigned int save1, save2, mirror;
	volatile unsigned int *ptr;

	retrain_tmp = 0;
	ptr = (volatile unsigned int *)CFG_SYS_SDRAM_BASE;
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

__weak unsigned int lpddr4_mr_read(unsigned int mr_rank, unsigned int mr_addr)
{
	unsigned int tmp;

	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x1);

	do {
		tmp = reg32_read(DDRC_MRSTAT(0));
	} while (tmp & 0x1);

	reg32_write(DDRC_MRCTRL0(0), (mr_rank << 4) | 0x1);
	reg32_write(DDRC_MRCTRL1(0), (mr_addr << 8));
	reg32setbit(DDRC_MRCTRL0(0), 31);
	do {
		tmp = reg32_read(DRC_PERF_MON_MRR0_DAT(0));
	} while ((tmp & 0x8) == 0);
	tmp = reg32_read(DRC_PERF_MON_MRR1_DAT(0));
	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x4);

	while (tmp) { //try to find a significant byte in the word
		if (tmp & 0xff) {
			tmp &= 0xff;
			break;
		}
		tmp >>= 8;
	}

	return tmp;
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


int board_phys_sdram2_size(phys_size_t *size)
{
	phys_size_t output = 0;
	unsigned int mr5, mr8;
	int ret;

	ret = board_phys_sdram_size(size);
	if (ret)
		return ret;

	/* 4G configuration are Samsung/Micron.
	 * If SDRAM1 size is 3G, there are 3 options:
	 *
	 * (*) A 3G Micron chip.
	 * (*) 4G Micron/Samsung
	 * (*) 8G Micron
	 *
	 */

	if (*size != 3*ONE_GB)
		goto exit;

	/* Read LPDDr MR5 register, if MAN. ID is Samsung, this is a 4G Samsung DDR */
	mr5 = lpddr4_mr_read(0xF, 0x5);
	if (mr5 == LPDDR4_SAMSUNG_MANID) {
		output = ONE_GB;
		goto exit;
	}

	/* At this point, this is either:
	 * - 3G Micron
	 * - 4G Micron
	 * - 8G Micron
	 * Can be determined based on MR8.
	 * If MR8 = 0x10, then the density is 16Gb per die (8Gb per channel),
	 * Since the Micron 4G is dual die, this means 32Gb => 4GB
	 * If MR8 = 0x18, then density is 16Gb per die, single channel,
	 * since Micron 8G is quad die, this means 64Gb => 8GB
	 */
	mr8 = lpddr4_mr_read(0xF, 0x8);
	if (mr8 == 0x10)
		output = ONE_GB;
	else if (mr8 == 0x18)
		output = 5*ONE_GB;

exit:
	*size = output;
	return 0;
}

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	init_uart_clk(1);

	return 0;
}

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

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
        dwc3_uboot_handle_interrupt(dev);
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

int board_init(void)
{
#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
	init_usb_clk();
#endif

	return 0;
}

/*
 * Identify board from TLV EEPROM - store result in board_id:
 * - product_name: name of kit
 * - product_rev: revision of kit
 */
static void board_id_kit_from_tlv_info(struct tlv_data *data) {
	const char *product_name = "";

	/* sample kit SKU: SRMP8QDW00D01GE008X01CE */
	if (!data->tlv_kit_number[0]) {
		return;
	} else if (strlen(data->tlv_kit_number) != 23) {
		pr_err("%s: kit sku \"%s\" has wrong length (expecting %d)\n", __func__, data->tlv_kit_number, 23);
	} else if (memcmp(&data->tlv_kit_number[2], "MP8", 3) != 0) {
		pr_err("%s: kit sku \"%s\" is not i.MX8M Plus (expecting XX%s...)\n", __func__, data->tlv_kit_number, "MP8");
	} else {
		// board type at index 18
		switch (data->tlv_kit_number[18]) {
		case 'M': // Mate
			product_name = "hummingboard-mate";
			break;
		case 'U': // Pulse
			product_name = "hummingboard-pulse";
			break;
		case 'R': // Ripple
			product_name = "hummingboard-ripple";
			break;
		case 'P': // Pro
		case 'T': // Extended (treated as Pro)
			product_name = "hummingboard-pro";
			break;
		case 'I': // IIOT
			product_name = "hummingboard-iiot";
			break;
		case 'X': // CuBox
			product_name = "cubox-m";
			break;
		default:
			pr_err("%s: did not recognize kit variant '%c' in sku \"%s\"!\n", __func__, data->tlv_kit_number[18], data->tlv_kit_number);
		}

		if (board_id.product_name && (strcmp(board_id.product_name, product_name) != 0)) {
			pr_err("%s: kit mismatch between som and carrier, have \"%s\" and \"%s\", using the latter!\n", __func__, board_id.product_name, product_name);
		}
		board_id.product_name = product_name;

		// kit revision at index 19-20
		board_id.product_rev[0] = data->tlv_kit_number[19];
		board_id.product_rev[1] = data->tlv_kit_number[20];
		board_id.product_rev[2] = 0;
	}
}

/*
 * Identify board from TLV EEPROM - store result in board_id:
 * - carrier_name: name of carrier
 * - carrier_rev: revision of carrier
 * - som_name: name of SoC
 * - som_rev: revision of SoM
 */
static void board_id_from_tlv_info(void) {
	hb_read_tlv_data();

	/* SoM EEPROM */
	if (!hb_tlv_ret[0]) {
		/* sample SKU: SRMP8QDWB1D01GE008V11C0 */
		if (hb_tlv_data[0].tlv_part_number[0] && strlen(hb_tlv_data[0].tlv_part_number) < 21) {
			pr_err("%s: som sku \"%s\" has wrong length (expecting >= %d)\n", __func__, hb_tlv_data[0].tlv_part_number, 21);
		} else if (memcmp(&hb_tlv_data[0].tlv_part_number[2], "MP8", 3) != 0) {
			pr_err("%s: som sku \"%s\" is not i.MX8M Plus (expecting XX%s...)\n", __func__, hb_tlv_data[0].tlv_part_number, "MP8");
		} else {
			board_id.som_name = "imx8mp";

			// SoM revision at index 19-20
			if (strlen(hb_tlv_data[0].tlv_part_number) >= 20) {
				board_id.som_rev[0] = hb_tlv_data[0].tlv_part_number[19];
				board_id.som_rev[1] = hb_tlv_data[0].tlv_part_number[20];
				board_id.som_rev[2] = 0;
			}
		}

		/* evalue kit if programmed to SoM EEPROM */
		board_id_kit_from_tlv_info(&hb_tlv_data[0]);
	}

	/* fall-back when identification failed */
	if(!board_id.som_name) {
		// could be anything ...
		printf("%s: could not identify som, defaulting to i.MX8M Plus Revision 1.1!\n", __func__);
		board_id.som_name = "imx8mp";
		strcpy(board_id.som_rev, "11");
	}

	/* Carrier EEPROM */
	if (!hb_tlv_ret[1]) {
		int rev_offset = 12;
		/* Example SKUs:
		 * - SRHBCUE000CVxx  HB-Pulse
		 * - SRHBCUEXT0CVxx  HB-Extended
		 * - SRHBCUPRO0IVxx  HB-Pro
		 * - SRHBCME000CVxx  HB-Mate
		 * - SRHBCRE000CVxx  HB-Ripple
		 * - SRHBIIOTIVxx    HB-IIOT
		 */
		if ((memcmp(&hb_tlv_data[1].tlv_part_number[2], "HBCUEXT", 7) == 0) ||
			 (memcmp(&hb_tlv_data[1].tlv_part_number[2], "HBCUPRO", 7) == 0)) {
			board_id.carrier_name = "hummingboard-pro";
		} else if (memcmp(&hb_tlv_data[1].tlv_part_number[2], "HBCUE", 5) == 0) {
			board_id.carrier_name = "hummingboard-pulse";
		} else if (memcmp(&hb_tlv_data[1].tlv_part_number[2], "HBCME", 5) == 0) {
			board_id.carrier_name = "hummingboard-mate";
		} else if (memcmp(&hb_tlv_data[1].tlv_part_number[2], "HBCRE", 5) == 0) {
			board_id.carrier_name = "hummingboard-ripple";
		} else if (memcmp(&hb_tlv_data[1].tlv_part_number[2], "HBIIOT", 6) == 0) {
			board_id.carrier_name = "hummingboard-iiot";
			rev_offset = 10;
		} else {
			pr_err("%s: did not recognize carrier sku \"%s\"!\n", __func__, hb_tlv_data[1].tlv_part_number);
			rev_offset = 0;
		}

		if (rev_offset) {
			board_id.carrier_rev[0] = hb_tlv_data[1].tlv_part_number[rev_offset];
			board_id.carrier_rev[1] = hb_tlv_data[1].tlv_part_number[rev_offset+1];
			board_id.carrier_rev[2] = 0;
		}

		/* evalue kit if programmed to Carrier EEPROM (takes precedence over previous call) */
		board_id_kit_from_tlv_info(&hb_tlv_data[1]);
	}

	/* fall-back when identification failed */
	if(!board_id.carrier_name) {
		// could be HummingBoard or CuBox ...
		if(board_id.product_name && strcmp(board_id.product_name, "cubox-m") == 0) {
			// we have a kit and it's a CuBox
			printf("%s: SoM is part of a CuBox-M Kit, infering that carrier is CuBox-M!\n", __func__);
			board_id.carrier_name = board_id.product_name;
		}
		else if (hb_tlv_ret[1] == -ENODEV) {
			// no eeprom, likely a Cubox
			printf("%s: could not identify board, defaulting to CuBox-M!\n", __func__);
			board_id.carrier_name = "cubox-m";
		}
		else {
			// if EEPROM exists, it must be HummingBoard
			printf("%s: could not identify board, defaulting to HummingBoard Pulse Revision 2.5!\n", __func__);
			board_id.carrier_name = "hummingboard-pulse";
			strcpy(board_id.carrier_rev, "25");
		}
	}
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	// identify device
	board_id_from_tlv_info();

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	// expose identity to environment
	env_set("carrier_name", board_id.carrier_name);
	env_set("carrier_rev", board_id.carrier_rev);
	env_set("som_name", board_id.som_name);
	env_set("som_rev", board_id.som_rev);
	env_set("product_name", board_id.product_name);
	env_set("product_rev", board_id.product_rev);
#endif

	return 0;
}

// calculate n-th mac from base
static void mac_add_n(unsigned char *base, u16 n) {
	if (n == 0)
		return;

	/*
	 * There is no 48 or 64-bit capable big-endian / host order
	 * conversion function available, increment byte for byte ...
	 */
	base[5]++;
	if (base[5] == 0) {
		base[4]++;
		if (base[4] == 0) {
			base[3]++;
			if (base[3] == 0) {
				base[2]++;
				if (base[2] == 0) {
					base[1]++;
					if (base[1] == 0)
						base[0]++;
				}
			}
		}
	}

	return mac_add_n(base, n-1);
}

/*
 * select board mac address for given interface
 */
int board_get_mac(int dev_id, unsigned char *mac) {
	int i;

	// tlv eeproms
	hb_read_tlv_data();
	i = dev_id;
	for(int j = 0; j < ARRAY_SIZE(hb_tlv_data); j++) {
		if(!is_valid_ethaddr((const u8 *)hb_tlv_data[j].tlv_mac_base))
			continue;

		// count if enough macs are provided
		if (i >= hb_tlv_data[j].tlv_mac_count) {
			i -= hb_tlv_data[j].tlv_mac_count;
			continue;
		}

		// compute i-th mac
		memcpy(mac, &hb_tlv_data[j].tlv_mac_base, 6);
		mac_add_n(mac, i);

		if (is_valid_ethaddr(mac)) {
			printf("%s: interface %i: using mac from tlv eeprom: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, dev_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			return 0;
		} else {
			pr_debug("%s: computed mac %02X:%02X:%02X:%02X:%02X:%02X is invalid\n", __func__, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			break;
		}
	}

	return -ENOENT;
}

int board_fit_config_name_match(const char *name) {
	char match[7+32] = "imx8mp-cubox-m";

	board_id_from_tlv_info();

	if (board_id.product_name)
		snprintf(match, sizeof(match), "%s-%s", "imx8mp", board_id.product_name);
	else if (board_id.carrier_name)
		snprintf(match, sizeof(match), "%s-%s", "imx8mp", board_id.carrier_name);
	else
		printf("%s: could not identify board, defaulting to CuBox-M!\n", __func__);

	/*
	 * match prefix only, e.g. imx8mp-hummingboard-iiot should also match
	 * imx8mp-hummingboard-iiot-main. Take care in OF_LIST and imx-mkimage
	 * to place generic name last as fall-back.
	 */
	if (!strncmp(name, match, strlen(name)))
		return 0;

	// always match old common name when there was single dtb only */
	if (!strcmp(name, "imx8mp-solidrun"))
		return 0;

	return -EINVAL;
}

#if defined(CONFIG_OF_BOARD_SETUP) || defined(CONFIG_OF_BOARD_FIXUP)
/* sample hdmi encoder power-down signal initial state to derive i2c address and line polarity */
static int find_hdmi_encoder(uint8_t *addr_encoder, uint8_t *addr_encoder_cec, uint8_t *addr_encoder_edid, uint8_t *addr_encoder_pkt) {
	struct udevice *bus = NULL;
	struct udevice *dev = NULL;
	uint16_t chipid;
	struct encoder_addr {
		uint8_t main;
		uint8_t cec;
		uint8_t edid;
		uint8_t pkt;
		bool pol;
	} const addr[2] = { { 0x3d, 0x3c, 0x3f, 0x38, true }, { 0x39, 0x38, 0x3b, 0x34, false }, };
	int ret;

	ret = uclass_get_device_by_name(UCLASS_I2C, "i2c@30a40000", &bus);
	if (ret)
		return ret;

	for (int i = 0; i < ARRAY_SIZE(addr); i++) {
		ret = dm_i2c_probe(bus, addr[i].main, 0, &dev);
		if (ret) {
			pr_err("%s: failed to probe i2c device 0x%x\n", __func__, addr[i].main);
			continue;
		}

		/* chip-id at 0xf5-0xf6 */
		ret = dm_i2c_read(dev, 0xf5, (uint8_t *)&chipid, 2);
		if (ret) {
			pr_err("%s: failed to read i2c device 0x%x register 0x%x: %d\n", __func__, addr[i].main, 0xf5, ret);
			continue;
		}

		switch (chipid) {
		case 0x8989:
			*addr_encoder = addr[i].main;
			*addr_encoder_cec = addr[i].cec;
			*addr_encoder_edid = addr[i].edid;
			*addr_encoder_pkt = addr[i].pkt;
			*addr_encoder_pkt = addr[i].pkt;
			return 0;
		default:
			pr_err("%s: read unknown chipid 0x%x from device at 0x%x\n", __func__, chipid, addr[i].main);
			continue;
		}
	}

	return -ENODEV;
}

/*
 * Patch device-tree for HDMI encoder (i2c address depending on optional R185)
 */
static int board_fix_hdmi(void *fdt, const char *stage) {
	uint8_t addr_encoder, addr_encoder_cec, addr_encoder_edid, addr_encoder_pkt;
	int node_encoder;
	const char *node_encoder_path[] = {
		"/soc@0/bus@30800000/i2c@30a40000/hdmi@3d"
	};
	int ret;

	printf("board_fix_hdmi\n");

	/* find hdmi encoder node */
	for (uint8_t i = 0; i < ARRAY_SIZE(node_encoder_path); i++) {
		node_encoder = fdt_path_offset(fdt, node_encoder_path[i]);
		if(node_encoder >= 0)
			break;
	}

	/* dtb without encoder does not need fixup or detection */
	if(node_encoder < 0)
		return 0;

	printf("board_fix_hdmi: detecting encoder ...\n");

	/* detect encoder */
	ret = find_hdmi_encoder(&addr_encoder, &addr_encoder_cec, &addr_encoder_edid, &addr_encoder_pkt);
	if (ret) {
		pr_err("%s: couldn't detect hdmi encoder, not patching %s dtb: %d!\n", __func__, stage, ret);
		return 0;
	}
	printf("%s: Found HDMI encoder at 0x%x!\n", __func__, addr_encoder);

	/* patch fdt node with probed address */
	ret = fdt_setprop_u32(fdt, node_encoder, "reg", addr_encoder);
	fdt_setprop_u32(fdt, node_encoder, "adi,addr-cec", addr_encoder_cec);
	fdt_setprop_u32(fdt, node_encoder, "adi,addr-edid", addr_encoder_edid);
	fdt_setprop_u32(fdt, node_encoder, "adi,addr-pkt", addr_encoder_pkt);

	if(ret < 0)
		pr_err("%s: failed to patch hdmi encoder address in %s dtb!\n", __func__, stage);

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
/* Patch device-tree for OS */
int ft_board_setup(void *blob, struct bd_info *bd)
{
	board_fix_hdmi(blob, "os");

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
#endif /* defined(CONFIG_OF_BOARD_SETUP) */
/* TODO: implement board_fix_fdt for u-boot own dtb */
#endif /* defined(CONFIG_OF_BOARD_SETUP) || defined(CONFIG_OF_BOARD_FIXUP) */

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
