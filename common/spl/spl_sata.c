/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Derived work from spl_usb.c
 */

#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <sata.h>
#include <scsi.h>
#include <errno.h>
#include <fat.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR
/* Dummy value to make the compiler happy */
#define CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR 0x100
#endif

static int spl_sata_load_image_raw(struct spl_image_info *spl_image,
		struct blk_desc *stor_dev, unsigned long sector)
{
	struct image_header *header;
	unsigned long count;
	u32 image_size_sectors;
	int ret;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
					 sizeof(struct image_header));
	count = blk_dread(stor_dev, sector, 1, header);
	if (count == 0)
		return -EIO;

	ret = spl_parse_image_header(spl_image, header);
	if (ret)
		return ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	count = blk_dread(stor_dev, sector, image_size_sectors,
			(void *)spl_image->load_addr);
	if (count != image_size_sectors)
		return -EIO;

	return 0;
}

static int spl_sata_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int err;
	struct blk_desc *stor_dev;
#ifndef CONFIG_AHCI
	err = sata_initialize();
#else
	err = init_sata(CONFIG_SPL_SATA_BOOT_DEVICE);
#endif
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: sata init failed: err - %d\n", err);
#endif
		return err;
	} else {
#ifndef CONFIG_AHCI
		stor_dev = sata_get_dev(CONFIG_SPL_SATA_BOOT_DEVICE);
#endif
#ifdef CONFIG_SCSI
		/* try to recognize storage devices immediately */
		scsi_scan(false);
		stor_dev = blk_get_devnum_by_type(IF_TYPE_SCSI, 0);
#endif
		if (!stor_dev)
			return -ENODEV;
	}

#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
		err = -ENOSYS;

		if (IS_ENABLED(CONFIG_SPL_FS_FAT)) {
			err = spl_load_image_fat(spl_image, stor_dev,
					CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
					CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
		} else if (IS_ENABLED(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)) {
			err = spl_sata_load_image_raw(spl_image, stor_dev,
				CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
		}
	}
	if (err) {
		puts("Error loading sata device\n");
		return err;
	}

	return 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);
