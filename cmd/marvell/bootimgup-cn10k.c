// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <div64.h>
#include <linux/bitops.h>
#include <asm/arch/smc.h>
#include <stdio.h>
#include <malloc.h>
#include <asm/arch/update.h>

/** Simple macro to set or clear flags */
#define PARSE_FLAG(parm, flag, set)				\
	{							\
		u32 mflag = (flag);				\
		if (!strcmp(argv[0], parm)) {			\
			if (set)				\
				desc.update_flags |= mflag;	\
			else					\
				desc.update_flags &= ~mflag;	\
			argc--;					\
			argv++;					\
			continue;				\
		}						\
	}

static void print_version_data(const char *str,
			       const char *name,
			       const struct tim_opaque_data_version_info *vinfo)
{
	pr_debug("%s %s: %d.%d.%d.%d %04d-%02d-%02d %02d:%02d 0x%04x 0x%08x\n    %32s\n",
	       str, name,
	       vinfo->major_version, vinfo->minor_version,
	       vinfo->revision_number, vinfo->revision_type,
	       vinfo->year, vinfo->month, vinfo->day,
	       vinfo->hour, vinfo->minute,
	       vinfo->flags, vinfo->customer_version,
	       vinfo->version_string);
}

/**
 * Send update requests to ATF
 */
static int do_bootimgup(struct cmd_tbl *cmdtp, int flag, int argc,

			char * const argv[])
{
	unsigned long value;
	unsigned num_updated = 0;
	int i;
	int ret;
	struct smc_update_descriptor desc;
	const char *env_addr, *env_size;
	bool spi = false, mmc = false;
	bool reset = false;
	char *update_log_ptr = NULL;
	uint64_t total_written = 0;
	const char *data_name;

	memset(&desc, 0, sizeof(desc));
	desc.magic = UPDATE_MAGIC;
	desc.version = UPDATE_VERSION;
	argv++;
	argc--;

	if (CONFIG_IS_ENABLED(CMD_BOOTIMGUP_CUST_SIG)) {
		/*
		 * The customer signature information can be either at the end
		 * or stored in environment variables (or both).
		 */
		env_addr = env_get("sigaddr");
		env_size = env_get("sigsize");

		if (env_addr && env_size) {
			ret = strict_strtoul(env_addr, 16, &value);
			if (ret) {
				printf("Invalid value for environment variable %s\n",
				       "sigaddr");
				return CMD_RET_FAILURE;
			}
			desc.user_addr = value;
			ret = strict_strtoul(env_size, 16, &value);
			if (ret) {
				printf("Invalid value for environment variable %s\n",
				       "siglen");
				return CMD_RET_FAILURE;
			}
			desc.user_size = value;
			pr_debug("Signature address: 0x%llx, size: 0x%llx\n",
				 desc.user_addr, desc.user_size);
		}
	}

	env_addr = env_get("fileaddr");
	env_size = env_get("filesize");

	if (env_addr) {
		ret = strict_strtoul(env_addr, 16, &value);
		if (ret) {
			printf("Invalid fileaddr value %s\n", env_addr);
			return CMD_RET_FAILURE;
		}
		desc.image_addr = value;
		pr_debug("fileaddr: 0x%llx\n", desc.image_addr);
	}
	if (env_size) {
		ret = strict_strtoul(env_size, 16, &value);
		if (ret) {
			printf("Invalid filesize value %s\n", env_size);
			return CMD_RET_FAILURE;
		}
		desc.image_size = value;
		pr_debug("filesize: 0x%llx\n", desc.image_size);
	}

	/* Default to erasing EBF config data */
	desc.update_flags |= UPDATE_FLAG_ERASE_CONFIG;
	/* Walk through all arguments */
	while (argc > 0) {
		PARSE_FLAG("-f", UPDATE_FLAG_FORCE_WRITE, true);
		PARSE_FLAG("-e", UPDATE_FLAG_ERASE_CONFIG, false);
		PARSE_FLAG("-v", UPDATE_FLAG_IGNORE_VERSION, true);
		if (CONFIG_IS_ENABLED(CMD_BOOTIMGUP_BACKUP))
			PARSE_FLAG("-b", UPDATE_FLAG_BACKUP, true);
		PARSE_FLAG("-p", UPDATE_FLAG_ERASE_PART, true);
		PARSE_FLAG("-l", UPDATE_FLAG_LOG_PROGRESS, true);

		if (!strcmp(argv[0], "-r")) {
			reset = true;
			argc--;
			argv++;
			continue;
		}
		/* Parse customer signature */
		if (CONFIG_IS_ENABLED(CMD_BOOTIMGUP_CUST_SIG) &&
		    !strcmp(argv[0], "-sig")) {
			if (argc < 3) {
				printf("Missing address and/or size for signature data\n");
				return CMD_RET_USAGE;
			}
			ret = strict_strtoul(argv[1], 16, &value);
			if (ret) {
				printf("Invalid signature address %s\n",
				       argv[1]);
				return CMD_RET_USAGE;
			}
			desc.user_addr = value;
			ret = strict_strtoul(argv[2], 16, &value);
			if (ret) {
				printf("Invalid signature size %s\n", argv[2]);
				return CMD_RET_USAGE;
			}
			desc.user_size = value;
			argc -= 3;
			argv += 3;
			pr_debug("Signature address: 0x%llx, size: 0x%llx\n",
				 desc.user_addr, desc.user_size);
			continue;
		}
		if (!strcmp(argv[0], "mmc")) {
			argv++;
			argc--;
			desc.update_flags |= UPDATE_FLAG_EMMC;
			pr_debug("Parsing mmc parameters\n");
			/*
			 * mmc can have the parameters:
			 *   [device ID/bus]
			 * or
			 *   [-p] [device ID/bus]
			 * or
			 *   [device ID/bus] [image address] [image size]
			 * or
			 *   [-p] [device ID/bus] [image address] [image size]
			 */
			if (argc > 0 && !strcmp(argv[0], "-p")) {
				desc.update_flags |= UPDATE_FLAG_ERASE_PART;
				pr_debug("Overwrite partition data\n");
				argc--;
				argv++;
			}
			if (argc < 1) {
				printf("mmc missing device ID\n");
				return CMD_RET_USAGE;
			}
			ret = strict_strtoul(argv[0], 0, &value);
			if (ret) {
				printf("Error parsing mmc device/bus number\n");
				return CMD_RET_USAGE;
			}
			desc.bus = value;
			pr_debug("MMC device: %lu\n", value);
			argv++;
			argc--;
			mmc = true;
			if (argc >= 2) {
				ret = strict_strtoul(argv[0], 16, &value);
				if (ret)
					continue;

				desc.image_addr = value;
				pr_debug("mmc image address: 0x%lx\n", value);

				ret = strict_strtoul(argv[1], 16, &value);
				if (ret) {
					printf("Error parsing mmc image size\n");
					return CMD_RET_USAGE;
				}
				pr_debug("mmc image size: 0x%lx\n", value);
				desc.image_size = value;
				argc -= 2;
				argv += 2;
				continue;
			}
		}
		if (!argc)
			break;
		if (!strcmp(argv[0], "spi")) {
			char *end;

			spi = true;
			argv++;
			argc--;
			desc.bus = 0;
			if (argc > 0) {
				desc.bus = simple_strtoul(argv[0], &end, 0);
				if (end && *end == ':')
					desc.cs = simple_strtoul(end + 1,
								 NULL, 0);
				else
					desc.cs = 0;
				pr_debug("SPI bus: 0x%x, cs: 0x%x\n",
					 desc.bus, desc.cs);
				argv++;
				argc--;
			}
			if (argc >= 2) {
				ret = strict_strtoul(argv[0], 16, &value);
				if (ret)
					continue;

				desc.image_addr = value;
				ret = strict_strtoul(argv[1], 16, &value);
				if (ret) {
					printf("Error parsing image size\n");
					return CMD_RET_USAGE;
				}

				desc.image_size = value;
				pr_debug("SPI image address: 0x%llx, size: 0x%llx\n",
					 desc.image_addr, desc.image_size);
				argc -= 2;
				argv += 2;
			}
			continue;
		}
		printf("Unexpected argument %s\n", argv[0]);
		return CMD_RET_USAGE;
	}

	if (mmc && spi) {
		printf("Only specify mmc or spi, not both\n");
		return CMD_RET_USAGE;
	}
	if (!spi && !mmc) {
		printf("Error: either SPI or eMMC must be specified.\n");
		return CMD_RET_USAGE;
	}
	if (!mmc && (desc.update_flags & UPDATE_FLAG_ERASE_PART)) {
		printf("Error: -p may only be used with eMMC devices\n");
		return CMD_RET_USAGE;
	}
	if (desc.image_size == 0 || desc.image_addr == 0) {
		printf("Error: Image address and/or size cannot be zero\n");
		return CMD_RET_USAGE;
	}

	if (desc.update_flags & UPDATE_FLAG_LOG_PROGRESS) {
		update_log_ptr = memalign(0x10000, UPDATE_LOG_SIZE);
		if (update_log_ptr) {
			desc.output_console = (uintptr_t)update_log_ptr;
			desc.output_console_size = UPDATE_LOG_SIZE;
		}
	}
	pr_debug("Using update version %d.%d\n",
	       (UPDATE_VERSION >> 8) & 0xff, UPDATE_VERSION & 0xff);
	ret = smc_spi_update(&desc);

	if (update_log_ptr) {
		pr_debug("Update log:\n");
		puts(update_log_ptr);
		pr_debug("\n\n");
		free(update_log_ptr);
	}

	if (ret) {
		printf("ERROR %d\n", ret);
		return CMD_RET_FAILURE;
	}
	pr_debug("Bootloader update %s: %llu bytes\n", mmc ? "MMC" : "SPI",
	       desc.image_size);
	if (!ret) {
		bool skipped = true;

		for (i = 0; i < SMC_MAX_OBJECTS; i++) {
			struct smc_update_obj_info *obj = &desc.object_retinfo[i];
			if (obj->tim_name[0] == '\0')
				break;
			if (obj->object_name[0] != '\0')
				data_name = (const char *)obj->object_name;
			else
				data_name = NULL;
			print_version_data((const char *)obj->tim_name,
					   "old version",
					   (const struct tim_opaque_data_version_info *)
					   obj->old_version_data);
			print_version_data((const char *)obj->tim_name,
					   "new version",
					  (const struct tim_opaque_data_version_info *)
					  obj->new_version_data);
			if (data_name != NULL)
				pr_debug("Loaded file: %s\n", data_name);
			if (obj->retcode == OBJ_UPDATE_SKIP_VERSION_MATCH ||
			    obj->retcode == OBJ_UPDATE_SKIP_DATA_MATCH) {
				skipped = true;
				pr_debug("Installation skipped due to matching version\n");
			} else if (obj->retcode == OBJ_UPDATE_OK) {
				skipped = false;
				num_updated++;
				pr_debug("%s/%s was installed\n",
				       obj->tim_name,
				       data_name != NULL ?
					      data_name : "(none)");
			} else {
				printf("Error %d installing %s/%s\n",
				       obj->retcode, obj->tim_name,
				       data_name != NULL ? data_name : "(none)");
			}
			if (!skipped) {
				total_written += obj->bytes_written;
				pr_debug("TIM address: 0x%llx, size: 0x%llx\n",
				       obj->tim_address, obj->tim_size);
				if (data_name != NULL)
					pr_debug("File address: 0x%llx, size: 0x%llx, bytes written: 0x%llx\n",
						obj->data_address,
						obj->data_size,
						obj->bytes_written);
				else
					pr_debug("Bytes written: 0x%llx\n",
					       obj->bytes_written);
			}
			pr_debug("\n");
		}
	}

	printf("Total bytes written: %llu, %u files updated\n",
	       total_written, num_updated);
	env_set_hex("update_bytes_written", total_written);
	env_set_ulong("update_skipped", total_written == 0 ? 1 : 0);

	if (reset && total_written > 0) {
		printf("Resetting after update\n\n\n");
		do_reset(NULL, 0, 0, NULL);
	}
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
#if defined(CONFIG_CMD_BOOTIMGUP_CUST_SIG) && defined(CONFIG_CMD_BOOTIMGUP_BACKUP)
	bootimgup, 12, 0, do_bootimgup, "Updates Boot Image",
	" <[-v]> <[-b]> <[-e]> <[-p]> <[-l]> <mmc [devid] | spi [bus:cs]> [image_address] [image_size] -sig [signature address] [signature size]\n"
#elif defined(CONFIG_CMD_BOOTIMGUP_CUST_SIG) && !defined(CONFIG_CMD_BOOTIMGUP_BACKUP)
	bootimgup, 11, 0, do_bootimgup, "Updates Boot Image",
	" <[-v]> <[-e]> <[-p]> <[-l]> <mmc [devid] | spi [bus:cs]> [image_address] [image_size] -sig [signature address] [signature size]\n"
#elif !defined(CONFIG_CMD_BOOTIMGUP_CUST_SIG) && defined(CONFIG_CMD_BOOTIMGUP_BACKUP)
	bootimgup, 9, 0, do_bootimgup, "Updates Boot Image",
	" <[-v]> <[-b]> <[-e]> <[-p]> <[-l]> <mmc | spi> <[devid] | [bus:cs]> [image_address] [image_size]\n"
#else
	bootimgup, 8, 0, do_bootimgup, "Updates Boot Image",
	" <[-v]> <[-e]> <[-p]> <[-l]> <[-r]> <mmc | spi> <[devid] | [bus:cs]> [image_address] [image_size]\n"
#endif
#ifdef CONFIG_CMD_BOOTIMGUP_BACKUP
	" -b - updates the backup image location\n"
#endif
	" -e - do not erase EBF configuration\n"
	" -v - perform hash check and bypass version check\n"
	" -f - force writes over matching data\n"
	" -p - (MMC only) overwrite the partition table\n"
	" -l - Enable logging to buffer\n"
	" -r - reset if one or more objects are updated\n"
	" spi - updates boot image on spi flash\n"
	" bus and cs should be passed together, if missing, 0 is assumed.\n"
	" image_address - address at which image is located in RAM\n"
	" image_size    - size of image in hex\n"
	" eg. to load on spi0 chipselect 0\n"
	" bootimgup spi 0:0 $fileaddr $filesize\n"
	" eg. to load on spi1 chipselect 1\n"
	" bootimgup spi 1:1 $fileaddr $filesize\n"
	"\n"
	" mmc - updates boot image on mmc card/chip\n"
	" eg. to load on device 0\n"
	" bootimgup mmc 0 $fileaddr $filesize\n"
	" eg. to load on device 1. If device id not given, 0 is used\n"
	" bootimgup mmc 1 $fileaddr $filesize\n"
	" image_address - address at which image is located in RAM\n"
	" image_size    - size of image in hex\n"
	" image_address, image_size should be passed together,\n"
	" passing only one of them treated as invalid.\n"
	"\n"
	" If not given, then $fileaddr and $filesize values in\n"
	" environment are used, otherwise fail to update.\n"
#ifdef CONFIG_CMD_BOOTIMGUP_CUST_SIG
	" -sig [signature address] [signature size size]\n"
	"  Address and size of image signature\n"
	" If missing, the environment variables $sigaddr and $sigsize will be used.\n"
#endif
);

const struct {
	enum smc_version_ret retcode;
	const char *str;
} vinfo_retstr[] = {
	{ VERSION_OK, "Success" },
	{ FIRMWARE_LAYOUT_CHANGED, "Firmware layout has changed" },
	{ TOO_MANY_OBJECTS, "Too many objects in flash" },
	{ INVALID_DEVICE_TREE, "Invalid device tree" },
	{ VERSION_NOT_SUPPORTED, "U-Boot descriptor version not supported" }
};

const char *vret_to_str(enum smc_version_ret retcode)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(vinfo_retstr); i++) {
		if (vinfo_retstr[i].retcode == retcode)
			return vinfo_retstr[i].str;
	}
	return "Unknown error code!";
}

const struct {
	enum smc_version_entry_retcode retcode;
	const char *str;
} ventry_retstr[] = {
	{ RET_OK, "Success" },
	{ RET_NOT_FOUND, "Specified object not found" },
	{ RET_TIM_INVALID, "Invalid TIM for object" },
	{ RET_BAD_HASH, "Object hash invalid"},
	{ RET_NOT_ENOUGH_MEMORY, "Out of memory" },
	{ RET_NAME_MISMATCH, "Error with name" },
	{ RET_TIM_NO_VERSION, "TIM for object is missing version info" },
	{ RET_TIM_NO_HASH, "TIM for object is missing hash info" },
	{ RET_HASH_ENGINE_ERROR, "Hash engine failure" },
	{ RET_HASH_NO_MATCH, "Object has invalid hash" },
	{ RET_IMAGE_TOO_BIG, "Object is too big" },
	{ RET_DEVICE_TREE_ENTRY_ERROR, "Error in device tree regarding object" },
	{ RET_BACKUP_IO_ERROR, "Error copying flash" },
};

const char *ventry_ret_to_str(enum smc_version_entry_retcode retcode)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ventry_retstr); i++) {
		if (ventry_retstr[i].retcode == retcode)
			return ventry_retstr[i].str;
	}
	return "Unknown object return code";
}

static void print_hash(u8 hash[], u8 hash_size)
{
	int i;

	printf("SHA%u: 0x", hash_size * 8);
	for (i = 0; i < hash_size; i++)
		printf("%02x", hash[i]);
	printf("\n");
}

static void print_version(struct tim_opaque_data_version_info *vi,
			  const char *indent)
{
	printf("%sMajor version: %d\n", indent, vi->major_version);
	printf("%sMinor version: %d\n", indent, vi->minor_version);
	printf("%sRevision:      %d\n", indent, vi->revision_number);
	printf("%sRevision type: %d\n", indent, vi->revision_type);
	printf("%sDate [yyyy-mm-dd]: %04d-%d-%d\n", indent,
	       vi->year, vi->month, vi->day);
	printf("%sTime (UTC):        %d:%02d\n", indent, vi->hour, vi->minute);
	printf("%sFlags:             0x%x\n", indent, vi->flags);
	printf("%sCustomer version: 0x%08x\n", indent, vi->customer_version);
	printf("%sVersion string:  %s\n", indent, vi->version_string);
}

static int do_get_version_info(struct cmd_tbl *cmdtp, int flag, int argc,
			       char * const argv[])
{
	struct smc_version_info *vinfo;
	unsigned long value;
	int ret;
	bool mmc = false;
	bool spi = false;
	bool verify = false;
	bool backup = false;
	int i;
	enum command_ret_t cmd_ret = CMD_RET_SUCCESS;

	vinfo = malloc(sizeof(struct smc_version_info));
	if (!vinfo) {
		printf("Error allocating version info\n");
		return CMD_RET_FAILURE;
	}
	memset(vinfo, 0, sizeof(*vinfo));

	vinfo->magic_number = VERSION_MAGIC;
	vinfo->version = VERSION_INFO_VERSION;

	argv++;
	argc--;

	while (argc > 0) {
		pr_debug("Parsing argument \"%s\"\n", argv[0]);
		if (!strcmp(argv[0], "-v")) {
			verify = true;
			argv++;
			argc--;
			continue;
		}
		if (!strcmp(argv[0], "-b")) {
			backup = true;
			argv++;
			argc--;
			continue;
		}
		if (!strcmp(argv[0], "mmc")) {
			argv++;
			argc--;
			if (argc < 1) {
				printf("mmc missing device ID\n");
				cmd_ret = CMD_RET_USAGE;
				goto end;
			}
			ret = strict_strtoul(argv[0], 0, &value);
			if (ret) {
				printf("Error parsing mmc device/bus number\n");
				cmd_ret = CMD_RET_USAGE;
				goto end;
			}
			vinfo->version_flags |= UPDATE_FLAG_EMMC;
			vinfo->bus = value;
			pr_debug("MMC device: %lu\n", value);
			argv++;
			argc--;
			mmc = true;
			continue;
		}
		if (!strcmp(argv[0], "spi")) {
			char *end;

			spi = true;
			argv++;
			argc--;
			vinfo->bus = 0;
			if (argc > 0) {
				vinfo->bus = simple_strtoul(argv[0], &end, 0);
				if (end && *end == ':')
					vinfo->cs = simple_strtoul(end + 1, NULL, 0);
				argv++;
				argc--;
			}
			continue;
		}
		printf("Unknown argument %s\n", argv[0]);
		cmd_ret = CMD_RET_USAGE;
		goto end;
	}
	if (mmc && spi) {
		printf("Only specify mmc or spi, not both\n");
		cmd_ret = CMD_RET_USAGE;
		goto end;
	}
	if (!mmc && !spi) {
		printf("Error: either SPI or eMMC must be specified\n");
		cmd_ret = CMD_RET_USAGE;
		goto end;
	}
	vinfo->num_objects = SMC_MAX_VERSION_ENTRIES;

	if (verify)
		vinfo->version_flags |= SMC_VERSION_CHECK_VALIDATE_HASH;
	if (backup)
		vinfo->version_flags |= VERSION_FLAG_BACKUP;

	pr_debug("%s: Calling smc_spi_verify(%p)...\n", __func__, &vinfo);
	ret = smc_spi_verify(vinfo);
	if (ret) {
		printf("Error verifying flash: %s\n",
		       vret_to_str((enum smc_version_ret)vinfo->retcode));
		if (vinfo->retcode == TOO_MANY_OBJECTS) {
			printf("Too many flash objects (%d) in flash for descriptor.  Max supported is %d\n",
			       vinfo->num_objects, SMC_MAX_VERSION_ENTRIES);
		}
		cmd_ret = CMD_RET_FAILURE;
		goto end;
	}

	if (vinfo->num_objects > SMC_MAX_VERSION_ENTRIES) {
		/* This should never happen */
		printf("Error: descriptor reports too many (%d) objects!\n",
		       vinfo->num_objects);
		cmd_ret = CMD_RET_FAILURE;
		goto end;
	}

	printf("Found %d objects\n", vinfo->num_objects);
	for (i = 0; i < vinfo->num_objects; i++) {
		struct smc_version_info_entry *object = &vinfo->objects[i];

		printf("Object %d: %s\n", i, object->name);
		if (object->retcode != RET_OK) {
			printf("  Error: %s\n",
			       ventry_ret_to_str(object->retcode));
			printf("  Log: %s\n", object->log);
		}
		printf("  Object TIM address in flash: 0x%llx\n",
		       object->tim_address);
		printf("  Object address in flash:     0x%llx\n",
		       object->object_address);
		printf("  Maximum allowed object size: 0x%llx\n",
		       object->max_size);
		printf("  Object size:                 0x%llx\n",
		       object->object_size);
		printf("  Object flags:                0x%x\n", object->flags);
		if (vinfo->version_flags & SMC_VERSION_CHECK_VALIDATE_HASH) {
			printf("  Object hash:                 ");
			print_hash(object->obj_hash, object->hash_size);
		}
		if (object->hash_size) {
			printf("  TIM hash:                    ");
			print_hash(object->tim_hash, object->hash_size);
		} else {
			printf("  TIM hash missing\n");
		}
		printf("  Version:\n");
		print_version(&object->version, "    ");
		printf("\n");
	}
	cmd_ret = CMD_RET_SUCCESS;

end:
	free(vinfo);
	return cmd_ret;
}

U_BOOT_CMD(bootimgversion, 5, 1, do_get_version_info,
	   "Display version information of all modules",
	   " <[-v]> <[-b]> <mmc [devid] | spi [bus[:cs]]>\n"
	   " -v - verify hashes contained in the image\n"
	   " -b - verify backup area of flash\n")

static int do_copy_image(struct cmd_tbl *cmdtp, int flag, int argc,
			 char * const argv[])
{
	struct smc_version_info *vinfo;
	unsigned long value;
	int ret;
	bool src_backup_offset = false;
	bool dst_backup_offset = false;
	bool src_mmc = false;
	bool dst_mmc = false;
	bool src_spi = false;
	bool dst_spi = false;
	bool src_media = true;
	bool force_clone = false;
	bool skip_sorce_check = false;
	enum command_ret_t cmd_ret = CMD_RET_SUCCESS;

	vinfo = malloc(sizeof(struct smc_version_info));
	if (!vinfo) {
		printf("Error allocating version info\n");
		return CMD_RET_FAILURE;
	}
	memset(vinfo, 0, sizeof(*vinfo));

	vinfo->magic_number = VERSION_MAGIC;
	vinfo->version = VERSION_INFO_VERSION;
	vinfo->num_objects = SMC_MAX_VERSION_ENTRIES;

	argv++;
	argc--;

	while (argc > 0) {
		pr_debug("Parsing argument \"%s\"\n", argv[0]);
		if (!strcmp(argv[0], "-b")) {
			src_backup_offset = true;
			argv++;
			argc--;
			continue;
		}
		if (!strcmp(argv[0], "-B")) {
			dst_backup_offset = true;
			argv++;
			argc--;
			continue;
		}
		if (!strcmp(argv[0], "-f")) {
			force_clone = true;
			argv++;
			argc--;
			continue;
		}
		if (!strcmp(argv[0], "-x")) {
			skip_sorce_check = true;
			argv++;
			argc--;
			continue;
		}
		if (!strcmp(argv[0], "mmc")) {
			argv++;
			argc--;
			if (argc < 1) {
				printf("mmc missing device ID\n");
				cmd_ret = CMD_RET_USAGE;
				goto done;
			}
			ret = strict_strtoul(argv[0], 0, &value);
			if (ret) {
				printf("Error parsing mmc device/bus number\n");
				cmd_ret = CMD_RET_USAGE;
				goto done;
			}
			if (src_media) {
				pr_debug("Setting source MMC to bus %ld\n", value);
				src_mmc = true;
				vinfo->bus = value;
				pr_debug("Source MMC device: %lu\n", value);
				src_media = false;
			} else {
				pr_debug("Setting destination MMC to bus %ld\n", value);
				dst_mmc = true;
				vinfo->target_bus = value;
				pr_debug("Destination MMC device: %lu\n", value);
			}
			argv++;
			argc--;
			continue;
		}
		if (!strcmp(argv[0], "spi")) {
			char *end;

			argv++;
			argc--;

			if (src_media) {
				src_spi = true;
				vinfo->bus = 0;
				if (argc > 0 && argv[0][0] != '-') {
					vinfo->bus = simple_strtoul(argv[0], &end, 0);
					if (end && *end == ':')
						vinfo->cs = simple_strtoul(end + 1, NULL, 0);
				}
				pr_debug("Setting source SPI to bus:cs %d:%d\n",
					 vinfo->bus, vinfo->cs);
				src_media = false;
			} else {
				dst_spi = true;
				vinfo->target_bus = 0;
				if (argc > 0 && argv[0][0] != '-') {
					vinfo->target_bus =
						simple_strtoul(argv[0], &end, 0);
					if (end && *end == ':')
						vinfo->target_cs =
							simple_strtoul(end + 1,
								       NULL, 0);
				}
				pr_debug("Setting destination SPI to bus:cs %d:%d\n",
					 vinfo->target_bus, vinfo->target_cs);
			}
			argv++;
			argc--;
			continue;
		}
	}
	if ((!src_mmc && !src_spi) || (!dst_mmc && !dst_spi)) {
		printf("Error: SPI or MMC must be specified for source and destination\n");
		pr_debug("src:dst mmc: %d:%d, spi: %d:%d\n", src_mmc, dst_mmc,
			 src_spi, dst_spi);
		cmd_ret = CMD_RET_USAGE;
		goto done;
	}

	vinfo->version_flags = SMC_VERSION_CHECK_VALIDATE_HASH |
			      SMC_VERSION_COPY_TO_BACKUP_FLASH;
	if (src_backup_offset)
		vinfo->version_flags |= VERSION_FLAG_BACKUP;
	if (dst_backup_offset)
		vinfo->version_flags |= SMC_VERSION_COPY_TO_BACKUP_OFFSET;
	if (src_mmc)
		vinfo->version_flags |= VERSION_FLAG_EMMC;
	if (dst_mmc)
		vinfo->version_flags |= SMC_VERSION_COPY_TO_BACKUP_EMMC;
	if (force_clone)
		vinfo->version_flags |= SMC_VERSION_FORCE_COPY_OBJECTS;
	if (skip_sorce_check)
		vinfo->version_flags |= SMC_VERSION_SKIP_FAIL_CHECK;

	pr_debug("%s: Calling smc_spi_verify(%p, flags: 0x%x)...\n",
		 __func__, &vinfo, vinfo->version_flags);
	ret = smc_spi_verify(vinfo);
	if (ret) {
		printf("Error copying flash: %s\n",
		       vret_to_str((enum smc_version_ret)vinfo->retcode));
		cmd_ret = CMD_RET_FAILURE;
	}
	cmd_ret =  CMD_RET_SUCCESS;

done:
	free(vinfo);
	return cmd_ret;
}

U_BOOT_CMD(bootimgcopy, 8, 0, do_copy_image,
	   "Copy firmware image between flash devices",
	   " [<-b>] <mmc [devid] | spi [bus[:cs]]> [<-B>] <mmc [devid] | spi [bus[:cs]]>\n"
	   " -b - specify backup location in source storage\n"
	   " -B - specify backup location in target storage\n"
	   " -f - force clone operation\n"
	   " -x - skip source verification check");
