/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 SolidRun Ltd.
 */

#include <common.h>
#include <env.h>
#include <asm/gpio.h>
#include <blk.h>
#include <command.h>
#include <dm/uclass.h>
#include <tlv_eeprom.h>
#include <linux/err.h>
#include <fdt_support.h>
#include <mapmem.h>
#include <mmc.h>
#include "rzg-common.h"

#define SD_EMMC_SEL_ENV "sdio_select"
static int sdio_sd_mmc_state = SDIO_SELECT_EMMC;

static int get_tlv_udevice_by_alias(struct udevice **dev, const char *alias)
{
	int node, ret;
	const char *path;
	path = fdt_get_alias(gd->fdt_blob, alias);
	if (!path)
	{
		pr_err("Cannot find the path for label %s.\n", alias);
		return -ENODEV;
	}
	/* Get the node offset using the path */
	node = fdt_path_offset(gd->fdt_blob, path);
	if (node < 0)
	{
		pr_err("Cannot find the node for path %s.\n", path);
		return -ENODEV;
	}

	/* Get the udevice using the node offset */
	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, node, dev);
	if (ret)
	{
		pr_err("Failed to find eeprom device, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

int get_sku_from_tlv_dev(struct udevice *dev, char *sku)
{
	int ret = 0;
	char eeprom[2048];
	struct tlvinfo_priv *tlv;
	struct tlvinfo_tlv *entry;

	tlv = tlv_eeprom_read(dev, 0, eeprom, ARRAY_SIZE(eeprom));
	if (IS_ERR(tlv))
	{
		pr_err("Can't parse the tlv: %ld\n", IS_ERR(tlv));
		return IS_ERR(tlv);
	}
	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_PART_NUMBER);
	if (IS_ERR(entry))
	{
		pr_err("Bad entry, ret: %ld\n", IS_ERR(entry));
		return IS_ERR(entry);
	}
	ret = tlv_entry_get_string(entry, sku, CARRIER_SKU_MAX_SIZE);
	if (ret)
	{
		pr_err("Can't get tlv_entry_get_string, ret: %d\n", ret);
		return ret;
	}
	return 0;
}

int rzg_get_carrier(void)
{
	int board, ret = 0;
	struct udevice *dev;
	char sku[CARRIER_SKU_MAX_SIZE];

	ret = get_tlv_udevice_by_alias(&dev, "eeprom_carrier");
	if (ret)
		return ret;

	ret = get_sku_from_tlv_dev(dev, sku);
	if (ret)
		return ret;

	switch (sku[5])
	{
	case 'M': // Mate
		board = CARRIER_HB_MATE;
		break;
	case 'R': // Ripple
		board = CARRIER_HB_RIPPLE;
		break;
	case 'U': // Pulse or Extended or Pro
		board = CARRIER_HB_PULSE;
		if (sku[6] == 'E' || sku[6] == 'P')
			board = CARRIER_HB_PRO;
		break;
	case 'I': // IIoT
		board = CARRIER_HB_IIOT;
		break;
	case 'E': // EU-205
		board = CARRIER_HB_EU205;
		break;
	default:
		board = CARRIER_UNRECOGNIZED;
		pr_warn("Did not recognise board variant in sku \"%s\"\n", sku);
	}

	return board;
}

__weak int board_check_initial_boot_source(void)
{
	uint32_t reg_md_boot = 0;
	/*
	 * return 1 for uSD, 0 for eMMC
	 * MD_BOOT[2:0]:
	 * 0-uSD
	 * 1-eMMC(1.8V)
	 * 2-eMMC(3.3V)
	 * 3-SPI(1.8V)
	 * 4-SPI(3.3V)
	 * 5-SCIF Downloading
	 *
	 * Note: eMMC/uSD Device Select - SD0_DEV_SEL_SW (LOW: eMMC ; HIGH: uSD)
	 */
	/* Extract MD_BOOT[2:0] (bits 0-2) */
	reg_md_boot = (*(volatile u32 *)SYS_LSI_MODE) & 0x7;
	debug("_MD_BOOT[2:0]=0x%x\n", reg_md_boot);
	sdio_sd_mmc_state = (reg_md_boot == 0) ? SDIO_SELECT_SD : SDIO_SELECT_EMMC;
	return sdio_sd_mmc_state;
}

__weak int board_check_sd_emmc(void)
{
	return sdio_sd_mmc_state;
}

__weak int board_select_sd_emmc(int select_sd)
{
	struct gpio_desc gpio[2];
	ofnode node;
	int i, count, ret;

	if (select_sd < 0 || select_sd > 1)
		return -EINVAL;

	node = ofnode_path("/config");
	if (!ofnode_valid(node))
	{
		pr_err("%s: no /config node in device tree\n", __func__);
		return -ENOENT;
	}

	count = gpio_request_list_by_name_nodev(node, "sdio_mux_gpios",
											gpio, ARRAY_SIZE(gpio),
											GPIOD_IS_OUT);
	if (count < 0)
	{
		pr_err("%s: failed to request sd mux gpios: %d\n", __func__, count);
		return count;
	}
	for (i = 0; i < count; i++)
	{
		ret = dm_gpio_set_value(&(gpio[i]), select_sd);
		if (ret)
		{
			pr_err("%s: Failed to set gpio %d: %d\n", __func__, i, ret);
			return ret;
		}
		ret = dm_gpio_free(NULL, &(gpio[i]));
		if (ret)
		{
			pr_err("%s: Failed to free gpio %d: %d\n", __func__, i, ret);
			return ret;
		}
		sdio_sd_mmc_state = select_sd;
	}
	pr_info("Select %s.\n", (select_sd == SDIO_SELECT_EMMC) ? "MMC" : "uSD");
	return 0;
}

void rzg_set_bootsource_env(void)
{
	int ret = 0;
	int select_sd = board_check_sd_emmc();
	char *sd_select_env = env_get(SD_EMMC_SEL_ENV);

	if (CONFIG_IS_ENABLED(SOLIDRUN_FORCE_SD_BOOT))
		ret = env_set(SD_EMMC_SEL_ENV, "sd");
	else if (CONFIG_IS_ENABLED(SOLIDRUN_FORCE_MMC_BOOT))
		ret = env_set(SD_EMMC_SEL_ENV, "mmc");
	else if (sd_select_env)
	{
		pr_info("Keeping saved sd/mmc env: %s\n", sd_select_env);
	}
	else
	{
		if (select_sd)
			ret = env_set(SD_EMMC_SEL_ENV, "sd");
		else
			ret = env_set(SD_EMMC_SEL_ENV, "mmc");
	}
	if (ret)
		pr_err("Failed to set boot_source env, err: %d \n", ret);
}

void rzg_sd_emmc_init(void)
{
	int value = board_check_initial_boot_source();
	board_select_sd_emmc(value);
}

__weak int board_init_usb_vbus(int gpio_flags_extra)
{
	struct gpio_desc gpio[2];
	ofnode node;
	int i, count, ret = 0;

	node = ofnode_path("/config");
	if (!ofnode_valid(node))
	{
		pr_err("%s: no /config node in device tree\n", __func__);
		return -ENOENT;
	}

	count = gpio_request_list_by_name_nodev(node, "usb_vbus_gpios",
											gpio, ARRAY_SIZE(gpio),
											GPIOD_IS_OUT | gpio_flags_extra);
	if (count < 0)
	{
		pr_err("%s: failed to request vbus gpios: %d\n", __func__, count);
		return count;
	}
	for (i = 0; i < count; i++)
	{
		ret = dm_gpio_set_value(&(gpio[i]), 1);
		if (ret)
		{
			pr_err("%s: Failed to set gpio %d: %d\n", __func__, i, ret);
			return ret;
		}
		ret = dm_gpio_free(NULL, &(gpio[i]));
		if (ret)
		{
			pr_err("%s: Failed to free gpio %d: %d\n", __func__, i, ret);
			return ret;
		}
	}
	return 0;
}

int rzg_board_usb_init(int gpio_flags_extra)
{
	int ret = board_init_usb_vbus(gpio_flags_extra);
	if (ret)
	{
		pr_err("Failed to enable USB VBUS, %d\n", ret);
		return ret;
	}
	/*Enable USB*/
	(*(volatile u32 *)CPG_RST_USB) = 0x000f000f;
	(*(volatile u32 *)CPG_CLKON_USB) = 0x000f000f;

	/* Setup  */
	/* Disable GPIO Write Protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 7); /* PWPR.BOWI = 0 */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 6);	 /* PWPR.PFCWE = 1 */

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
	return 0;
}

void rzg_carrier_usb_init(int carrier)
{
	switch (carrier)
	{
	case CARRIER_HB_MATE:
	case CARRIER_HB_RIPPLE:
	case CARRIER_HB_PULSE:
	case CARRIER_HB_PRO:
	case CARRIER_HB_IIOT:
		rzg_board_usb_init(GPIOD_OPEN_DRAIN);
		break;
	default:
		rzg_board_usb_init(0);
		break;
	}
}

uint mmc_get_env_part(struct mmc *mmc)
{
	int value = board_check_sd_emmc();
	if (value == SDIO_SELECT_SD)
		return 0;
	else
		return CONFIG_SYS_MMC_ENV_PART;
}

static int preboot_check_sd_emmc(void)
{
	int sd_select = 0;
	char *sd_select_env = from_env(SD_EMMC_SEL_ENV);
	if (!sd_select_env)
		sd_select = board_check_sd_emmc();
	else if (strcmp(sd_select_env, "sd") == 0)
		sd_select = SDIO_SELECT_SD;
	else if (strcmp(sd_select_env, "mmc") == 0)
		sd_select = SDIO_SELECT_EMMC;
	else
	{
		printf("Unknown " SD_EMMC_SEL_ENV " value, using default\n");
		sd_select = SDIO_SELECT_EMMC;
	}
	return (sd_select);
}

void board_preboot_os(void)
{
	int ret = 0;
	const char *cmd_load = "run load_sdio_overlay";
	const char *cmd_apply = "run apply_sdio_overlay";

	pr_info("Loading and extracting overlay... \n");
	ret = run_command(cmd_load, 0);
	if(ret != 0)
	{
		pr_err("Failed to run command \"%s\": ret %d\n", cmd_load, ret);
		return;
	}

	pr_info("Applying overlay...\n");
	ret = run_command(cmd_apply, 0);
	if(ret != 0)
	{
		pr_err("Failed to run command \"%s\": ret %d\n", cmd_apply, ret);
		return;
	}
	int enable_sdhc = preboot_check_sd_emmc();
	board_select_sd_emmc(enable_sdhc);
}

int tlv_get_mac_eeprom_udevice(struct udevice **dev)
{
	return get_tlv_udevice_by_alias(dev, "eeprom_som");
}
