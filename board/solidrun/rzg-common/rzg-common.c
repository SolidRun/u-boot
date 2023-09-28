// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <env.h>
#include <dm/uclass.h>
#include <tlv_eeprom.h>
#include <linux/err.h>
#include "rzg-common.h"

#define SD_EMMC_SEL_ENV "sdio_select"

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
	struct tlvinfo_priv *tlv, *entry;

	tlv = tlv_eeprom_read(dev, 0, eeprom, ARRAY_SIZE(eeprom));
	if (IS_ERR(tlv))
	{
		pr_err("Can't parse the tlv: %d\n", tlv);
		return tlv;
	}
	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_PART_NUMBER);
	if (IS_ERR(entry))
	{
		pr_err("Bad entry, ret: %d\n", entry);
		return entry;
	}
	ret = tlv_entry_get_string(entry, sku, CARRIER_SKU_MAX_SIZE);
	if (ret)
	{
		pr_err("Can't get tlv_entry_get_string, ret: %d\n", ret);
		return ret;
	}
	return 0;
}

int get_carrier(void)
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
	case 'U': // Pulse or Extended
		board = CARRIER_HB_PULSE;
		if (sku[6] == 'E')
			board = CARRIER_HB_EXTENDED;
		break;
	default:
		board = CARRIER_UNRECOGNIZED;
		pr_warn("Did not recognise board variant in sku \"%s\"\n", sku);
	}

	return board;
}

// Should return 1 on SD and 0 on eMMC
__weak int board_check_sd_emmc(void)
{
	printf("Warning! %s not implemented!", __func__);
	return 0;
}

__weak void board_select_sd_emmc(int select_sd)
{
	printf("Warning! %s not implemented!", __func__);
}

static void set_bootsource_env(int select_sd)
{
	int ret;
	if (select_sd)
		ret = env_set(SD_EMMC_SEL_ENV, "sd");
	else
		ret = env_set(SD_EMMC_SEL_ENV, "emmc");
	if (ret)
		pr_err("Failed to set boot_source env, err: %d \n", ret);
}

void rzg_sd_emmc_init(void)
{
	/* Select eMMC/uSD based on SD0_DEV_SEL_SW (P22_1) GPIO value {High: uSD ; Low: eMMC}*/
	int value = board_check_sd_emmc();
	board_select_sd_emmc(value);
	set_bootsource_env(value);
}

uint mmc_get_env_part(struct mmc *mmc)
{
	int value = board_check_sd_emmc();
	if (value == 1)
		return 0;
	else
		return CONFIG_SYS_MMC_ENV_PART;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_SYSTEM_SETUP)
/*
 * Configure the correct sdhi0 node (eMMC/SD) in device-tree:
 *  Set up board-specific details in device tree before boot
 */

static bool preboot_check_sd_emmc(void)
{
	int sd_select = 0;
	char *sd_select_env = from_env(SD_EMMC_SEL_ENV); // here is the fail!!
	if (!sd_select_env)
	{
		sd_select = board_check_sd_emmc();
	}
	else if (strcmp(sd_select_env, "sd") == 0)
	{
		sd_select = 1;
	}

	return (sd_select || CONFIG_IS_ENABLED(SOLIDRUN_FORCE_SD_BOOT)) && !CONFIG_IS_ENABLED(SOLIDRUN_FORCE_MMC_BOOT);
}

int rzg_preboot_sd_emmc_setup(void *blob, struct bd_info *bd)
{
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
		set | bus-width = <4>;
		add | max-frequency = <50000000>;
		remove | mmc-hs200-1_8v;
		remove | non-removable;
		remove | fixed-emmc-driver-type = <1>;
		*/

		/* select uSD and set SD0_VDD=3.3V */
		node = fdt_path_offset(blob, "/soc/pinctrl@11030000/gpio-sd0-dev-sel-hog");

		ret = fdt_delprop(blob, node, "output-high");
		if (ret < 0 && enable_sdhc)
			pr_err("%s: failed to disable gpio-sd0-dev-sel-hog in dtb!\n", __func__);

		ret = fdt_setprop_empty(blob, node, "output-low");
		if (ret < 0 && enable_sdhc)
			pr_err("%s: failed to set output-low -> gpio-sd0-dev-sel-hog in dtb!\n", __func__);

		node = fdt_path_offset(blob, "/soc/pinctrl@11030000/gpio-sd0-vdd-18v-hog");
		ret = fdt_delprop(blob, node, "output-high");
		if (ret < 0 && enable_sdhc)
			pr_err("%s: failed to delete output-high gpio-sd0-vdd-18v-hog in dtb!\n", __func__);

		ret = fdt_setprop((blob), (node), ("output-low"), ((void *)0), 0);
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

	return 0;
}
#endif

