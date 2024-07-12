// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI setup code
 *
 *  Copyright (c) 2016-2018 Alexander Graf et al.
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <log.h>
#include <dm.h>
#include <mmc.h>
#include <asm/arch/update.h>

#define OBJ_LIST_NOT_INITIALIZED 1

efi_status_t __efi_runtime_data efi_obj_list_initialized = OBJ_LIST_NOT_INITIALIZED;

#ifdef CONFIG_TARGET_CN20K_A
efi_status_t efi_get_boot_device_mode(const char *dev_name, u64 *mode)
{
	int dev_bus;
	struct mmc *mmc_dev;
	u64 cs;

	*mode = 0;
	if (!strcmp(dev_name, "EMMC_CS0")) {
		dev_bus = 0;
	} else {
		dev_bus = -1;
		return EFI_SUCCESS;
	}

	mmc_dev = find_mmc_device(dev_bus);
	if (!mmc_dev) {
		printf("No mmc device for bus %d\n", dev_bus);
		return EFI_SUCCESS;
	}
	if (!mmc_getcd(mmc_dev))
		mmc_dev->has_init = 0;
	mmc_dev->user_speed_mode = MMC_MODES_END;
	if (mmc_init(mmc_dev))
		return EFI_DEVICE_ERROR;

	cs = UPDATE_MMC_CS_RCA(mmc_dev->rca) | UPDATE_MMC_CS_FLAG;
	if (!mmc_dev->high_capacity)
		cs |= UPDATE_MMC_CS_FLAG_BYTE_ACCESS;
	if (IS_SD(mmc_dev))
		cs |= UPDATE_MMC_CS_FLAG_SD;
	if (mmc_dev->ocr & BIT(7))
		cs |= UPDATE_MMC_CS_FLAG_1_8V;
	if (mmc_dev->ocr & 0xff8000)
		cs |= UPDATE_MMC_CS_FLAG_3_3V;
	pr_debug("%s: rca: 0x%x, ocr: 0x%x, signal voltage: 0x%x\n",
		 __func__, mmc_dev->rca, mmc_dev->ocr, mmc_dev->signal_voltage);
	*mode = cs;

	return EFI_SUCCESS;
}
#endif

efi_status_t efi_get_boot_device_name(const char **name)
{
	ofnode node;

	*name = NULL;

	if (IS_ENABLED(CONFIG_ARCH_CN20K))
		node = ofnode_path("/marvell,ebf");
	else
		node = ofnode_path("/cavium,bdk");
	if (ofnode_valid(node))
		if (IS_ENABLED(CONFIG_ARCH_CN10K) || IS_ENABLED(CONFIG_ARCH_CN20K))
			*name = ofnode_read_string(node, "BOOT-DEVICE");
		else
			*name = ofnode_read_string(node, "BOOT-DEVICE.N0");
	else
		printf("Error: cannot retrieve boot device from fdt\n");

	return EFI_SUCCESS;
}

/*
 * Allow unaligned memory access.
 *
 * This routine is overridden by architectures providing this feature.
 */
void __weak allow_unaligned(void)
{
}

/**
 * efi_init_platform_lang() - define supported languages
 *
 * Set the PlatformLangCodes and PlatformLang variables.
 *
 * Return:	status code
 */
static efi_status_t efi_init_platform_lang(void)
{
	efi_status_t ret;
	efi_uintn_t data_size = 0;
	char *lang = CONFIG_EFI_PLATFORM_LANG_CODES;
	char *pos;

	/*
	 * Variable PlatformLangCodes defines the language codes that the
	 * machine can support.
	 */
	ret = efi_set_variable_int(u"PlatformLangCodes",
				   &efi_global_variable_guid,
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS |
				   EFI_VARIABLE_READ_ONLY,
				   sizeof(CONFIG_EFI_PLATFORM_LANG_CODES),
				   CONFIG_EFI_PLATFORM_LANG_CODES, false);
	if (ret != EFI_SUCCESS)
		goto out;

	/*
	 * Variable PlatformLang defines the language that the machine has been
	 * configured for.
	 */
	ret = efi_get_variable_int(u"PlatformLang",
				   &efi_global_variable_guid,
				   NULL, &data_size, &pos, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		/* The variable is already set. Do not change it. */
		ret = EFI_SUCCESS;
		goto out;
	}

	/*
	 * The list of supported languages is semicolon separated. Use the first
	 * language to initialize PlatformLang.
	 */
	pos = strchr(lang, ';');
	if (pos)
		*pos = 0;

	ret = efi_set_variable_int(u"PlatformLang",
				   &efi_global_variable_guid,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   1 + strlen(lang), lang, false);
out:
	if (ret != EFI_SUCCESS)
		printf("EFI: cannot initialize platform language settings\n");
	return ret;
}

#ifdef CONFIG_EFI_SECURE_BOOT
/**
 * efi_init_secure_boot - initialize secure boot state
 *
 * Return:	status code
 */
static efi_status_t efi_init_secure_boot(void)
{
	efi_guid_t signature_types[] = {
		EFI_CERT_SHA256_GUID,
		EFI_CERT_X509_GUID,
	};
	efi_status_t ret;

	ret = efi_set_variable_int(u"SignatureSupport",
				   &efi_global_variable_guid,
				   EFI_VARIABLE_READ_ONLY |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   sizeof(signature_types),
				   &signature_types, false);
	if (ret != EFI_SUCCESS)
		printf("EFI: cannot initialize SignatureSupport variable\n");

	return ret;
}
#else
static efi_status_t efi_init_secure_boot(void)
{
	return EFI_SUCCESS;
}
#endif /* CONFIG_EFI_SECURE_BOOT */

/**
 * efi_init_capsule - initialize capsule update state
 *
 * Return:	status code
 */
static efi_status_t efi_init_capsule(void)
{
	efi_status_t ret = EFI_SUCCESS;

	if (IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_UPDATE)) {
		ret = efi_set_variable_int(u"CapsuleMax",
					   &efi_guid_capsule_report,
					   EFI_VARIABLE_READ_ONLY |
					   EFI_VARIABLE_BOOTSERVICE_ACCESS |
					   EFI_VARIABLE_RUNTIME_ACCESS,
					   22, u"CapsuleFFFF", false);
		if (ret != EFI_SUCCESS)
			printf("EFI: cannot initialize CapsuleMax variable\n");
	}

	return ret;
}

/**
 * efi_init_os_indications() - indicate supported features for OS requests
 *
 * Set the OsIndicationsSupported variable.
 *
 * Return:	status code
 */
static efi_status_t efi_init_os_indications(void)
{
	u64 os_indications_supported = 0;

	if (IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT))
		os_indications_supported |=
			EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED;

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK))
		os_indications_supported |=
			EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED;

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT))
		os_indications_supported |=
			EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED;

	return efi_set_variable_int(u"OsIndicationsSupported",
				    &efi_global_variable_guid,
				    EFI_VARIABLE_BOOTSERVICE_ACCESS |
				    EFI_VARIABLE_RUNTIME_ACCESS |
				    EFI_VARIABLE_READ_ONLY,
				    sizeof(os_indications_supported),
				    &os_indications_supported, false);
}

/**
 * efi_init_early() - handle initialization at early stage
 *
 * expected to be called in board_init_r().
 *
 * Return:	status code
 */
int efi_init_early(void)
{
	efi_status_t ret;

	/* Allow unaligned memory access */
	allow_unaligned();

	/* Initialize root node */
	ret = efi_root_node_register();
	if (ret != EFI_SUCCESS)
		goto out;

	ret = efi_console_register();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize EFI driver uclass */
	ret = efi_driver_init();
	if (ret != EFI_SUCCESS)
		goto out;

	return 0;
out:
	/* never re-init UEFI subsystem */
	efi_obj_list_initialized = ret;

	return -1;
}

/**
 * efi_init_obj_list() - Initialize and populate EFI object list
 *
 * Return:	status code
 */
efi_status_t efi_init_obj_list(void)
{
	efi_status_t ret = EFI_SUCCESS;
	const char *str;

	efi_get_boot_device_name(&str);

	/* Initialize once only */
	if (efi_obj_list_initialized != OBJ_LIST_NOT_INITIALIZED)
		return efi_obj_list_initialized;

	/* Set up console modes */
	efi_setup_console_size();

	/*
	 * Probe block devices to find the ESP.
	 * efi_disks_register() must be called before efi_init_variables().
	 */
	ret = efi_disks_register();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize variable services */
	ret = efi_init_variables();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Define supported languages */
	ret = efi_init_platform_lang();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Indicate supported features */
	ret = efi_init_os_indications();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize system table */
	ret = efi_initialize_system_table();
	if (ret != EFI_SUCCESS)
		goto out;

	if (IS_ENABLED(CONFIG_EFI_ECPT)) {
		ret = efi_ecpt_register();
		if (ret != EFI_SUCCESS)
			goto out;
	}

	if (IS_ENABLED(CONFIG_EFI_ESRT)) {
		ret = efi_esrt_register();
		if (ret != EFI_SUCCESS)
			goto out;
	}

	if (IS_ENABLED(CONFIG_EFI_TCG2_PROTOCOL)) {
		if ((fdt_node_offset_by_compatible(gd->fdt_blob, -1, "tcg,tpm_tis-spi") >= 0) ||
		    (fdt_node_offset_by_compatible(gd->fdt_blob, -1, "tcg,tpm-tis-i2c") >= 0)) {
			ret = efi_tcg2_register();
			if (ret != EFI_SUCCESS)
				goto out;

			ret = efi_tcg2_do_initial_measurement();
			if (ret == EFI_SECURITY_VIOLATION)
				goto out;
		}
	}

	/* Install EFI_RNG_PROTOCOL */
	if (IS_ENABLED(CONFIG_EFI_RNG_PROTOCOL)) {
		ret = efi_rng_register();
		if (ret != EFI_SUCCESS)
			goto out;
	}

	if (IS_ENABLED(CONFIG_EFI_RISCV_BOOT_PROTOCOL)) {
		ret = efi_riscv_register();
		if (ret != EFI_SUCCESS)
			goto out;
	}

	/* Secure boot */
	ret = efi_init_secure_boot();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Indicate supported runtime services */
	ret = efi_init_runtime_supported();
	if (ret != EFI_SUCCESS)
		goto out;

	if (IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)) {
		ret = efi_load_capsule_drivers();
		if (ret != EFI_SUCCESS)
			goto out;
	}

	if (IS_ENABLED(CONFIG_VIDEO)) {
		ret = efi_gop_register();
		if (ret != EFI_SUCCESS)
			goto out;
	}
#ifdef CONFIG_NETDEVICES
	ret = efi_net_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_GENERATE_ACPI_TABLE
	ret = efi_acpi_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_GENERATE_SMBIOS_TABLE
	ret = efi_smbios_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
	ret = efi_watchdog_register();
	if (ret != EFI_SUCCESS)
		goto out;

	ret = efi_init_capsule();
	if (ret != EFI_SUCCESS)
		goto out;

#ifdef CONFIG_EFI_SPI_NOR_FLASH_PROTOCOL
	ret = efi_spinor_protocol_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_EFI_PCI_IO_PROTOCOL
	ret = efi_pci_io_protocol_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_EFI_SEC_SPI_NOR_FLASH
	ret = efi_sec_spinor_protocol_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_EFI_SWCFG_PROTOCOL
	ret = efi_switch_config_protocol_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_EFI_GPIO_PROTOCOL
	ret = efi_gpio_protocol_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_EFI_I2C_PROTOCOL
	ret = efi_i2c_protocol_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif

	/* Initialize EFI runtime services */
	ret = efi_reset_system_init();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Execute capsules after reboot */
	if (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK) &&
	    !IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY))
		ret = efi_launch_capsules();

	if (str) {
		ret = EFI_CALL(efi_set_variable(L"BootDevice",
						&efi_global_variable_guid,
						EFI_VARIABLE_BOOTSERVICE_ACCESS |
						EFI_VARIABLE_RUNTIME_ACCESS,
						strlen(str),
						str));
		if (ret != EFI_SUCCESS)
			printf("Error: cannot set BootDevice EFI variable\n");
	}

out:
	efi_obj_list_initialized = ret;
	return ret;
}
