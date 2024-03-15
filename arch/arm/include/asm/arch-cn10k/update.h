// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __UPDATE_H__
#define __UPDATE_H__

/** Size for update log in bytes */
#define UPDATE_LOG_SIZE		0x40000

/* The following is taken from libtim.h used by ATF */
#define VERSION_STRING_LENGTH	32

/**
 * Version data structure
 */
struct tim_opaque_data_version_info {
	u8	major_version;	/** Major version number */
	u8	minor_version;	/** Minor version number */
	u8	revision_number;/** Revision number */
	u8	revision_type;	/** Revision type (TBD) */
	u16	year;		/** GIT Year */
	u8	month;		/** GIT Month */
	u8	day;		/** GIT Day */
	u8	hour;		/** GIT Hour */
	u8	minute;		/** GIT Minute */
	u16	flags;		/** Flags (TBD) */
	u32	customer_version;/** Customer defined version number */
	/**
	 * String representation of version
	 * The version string contains:
	 * XX.YY.ZZ YYYYMMDD-HHmm (customer hex)
	 * Where:
	 * XX: Major version number
	 * YY: Minor version number
	 * ZZ: Revision number
	 * YYYY: 4-digit year
	 * MM: 2-digit month
	 * DD: 2-digit day of month
	 * HH: Hour
	 * mm: Minute
	 *
	 * The timestamp is extracted from GIT
	 */
	u8	version_string[VERSION_STRING_LENGTH];
} __attribute__((packed, aligned(4)));

/*
 * The following is copied from the ATF file
 * include/drivers/marvell/octeontx/tim_update.h.  This should be updated
 * whenever ATF is updated.  The current version is 0x200.
 */
#define SPI_CONFIG_ERR		2
#define SPI_MMAP_ERR		3
#define SPI_IMG_VALIDATE_ERR	4
#define SPI_IMG_UPDATE_ERR	5
#define SPI_BAD_MAGIC_NUMBER	6
#define SPI_BAD_PARAMETER	7
#define SPI_ALREADY_IN_PROGRESS	8
#define VER_MAX_NAME_LENGTH	32
#define SMC_MAX_OBJECTS		32
#define VERSION_DATA_LENGTH	32

#ifndef BIT
# define BIT(x)	(1UL << (x)))
#endif

#define VERIFY_LOG_SIZE		1024

/**
 * TIM0 is special and needs to be handled different than other TIMs.
 */
#define TIM0_FILENAME		"tim0.timb"
#define TIM0_FDT_NAME		"tim0"

/**
 * The default endpoint script is also special since it does not load
 * the GSERP firmware
 */
#define EP_TIM_FILENAME		"ep_script-cn10xx.timb"

enum update_ret {
	/** No errors */
	UPDATE_OK = 0,
	/** Error with the CPIO image */
	UPDATE_CPIO_ERROR = -1,
	/** Invalid TIM found in update */
	UPDATE_TIM_ERROR = -2,
	/** One or more files failed hash check */
	UPDATE_HASH_ERROR = -3,
	/** Update authentication error */
	UPDATE_AUTH_ERROR = -4,
	/** I/O error reading or writing to the flash */
	UPDATE_IO_ERROR = -5,
	/**
	 * Error found that requires all objects to be updated,
	 * i.e. a corrupt object found in the existing flash
	 */
	UPDATE_REQUIRE_FULL = -6,
	/** Out of resources, too many files, etc. */
	UPDATE_NO_MEM = -7,
	/** Problem found with device tree firmware-update section */
	UPDATE_DT_ERROR = -8,
	/** Incomplete file grouping found */
	UPDATE_GROUP_ERROR = -9,
	/** Location or size of an object invalid */
	UPDATE_LOCATION_ERROR = -10,
	/** Unsupported media */
	UPDATE_INVALID_MEDIA = -11,
	/** Invalid alignment of update file */
	UPDATE_BAD_ALIGNMENT = -12,
	/** TIM is missing in an object */
	UPDATE_MISSING_TIM = -13,
	/** File is missing in an object */
	UPDATE_MISSING_FILE = -14,
	/** TIM is missing in flash */
	UPDATE_TIM_MISSING = -15,
	/** I/O issue with eHSM component */
	UPDATE_EHSM_ERROR = -16,
	/** Update rejected due to version check */
	UPDATE_VERSION_CHECK_FAIL = -17,
	/** Bad magic number in update descriptor */
	UPDATE_BAD_DESC_MAGIC = -18,
	/** Unsupported version in update descriptor */
	UPDATE_BAD_DESC_VERSION = -19,
	/** Error mapping update to secure memory */
	UPDATE_MMAP_ERROR = -20,
	/** More space is needed in the work buffer. */
	UPDATE_WORK_BUFFER_TOO_SMALL = -21,
	/**
	 * Returned if an update or clone operation is already running on
	 * another core.
	 */
	UPDATE_ALREADY_IN_PROGRESS = -22,
	/** Failed to register to the media IO driver */
	UPDATE_IO_DEV_REGISTER_ERROR = -23,
	/** Failed to open media I/O driver */
	UPDATE_IO_DEV_OPEN_ERROR = -24,
	/** Error initializing IO device */
	UPDATE_IO_DEV_INIT_ERROR = -25,
	/** Object count exceeds SMC_MAX_OBJECTS */
	UPDATE_TOO_MANY_OBJECTS = -26,
	/** Unknown error */
	UPDATE_UNKNOWN_ERROR = -1000,
};

enum tim_object_update_retcode {
	OBJ_UPDATE_OK = 0,
	OBJ_UPDATE_SKIP_VERSION_MATCH = 1,
	OBJ_UPDATE_SKIP_DATA_MATCH = 2,
	OBJ_UPDATE_FORCED = 3,
	OBJ_UPDATE_GROUP_FORCED = 4,
	OBJ_UPDATE_INVALID_TIM = 128,
	OBJ_UPDATE_INVALID_VERSION = 129,
	/** Old flash image source hash does not match TIM */
	OBJ_UPDATE_SRC_FLASH_HASH_FAIL = 130,
	OBJ_UPDATE_SRC_FLASH_VERIFICATION_FAIL = 131,
	OBJ_UPDATE_VERSION_DATA_MISSING = 132,
};

enum object_hash_type {
	HASH_NONE = 0,
	HASH_SHA256 = 0x20,
	HASH_SHA3_256 = 0x23,
	HASH_SHA384 = 0x30,
	HASH_SHA3_384 = 0x33,
	HASH_SHA512 = 0x40,
	HASH_SHA3_512 = 0x43
};

/**
 * Output data for updating each object in the update file.  Each entry
 * should be 256 bytes.
 */
struct smc_update_obj_info {
	u8	tim_name[VER_MAX_NAME_LENGTH];		/** TIM binary name */
	u8	object_name[VER_MAX_NAME_LENGTH];	/** Object name */
	u8	old_version_data[VERSION_DATA_LENGTH];
	u8	new_version_data[VERSION_DATA_LENGTH];
	u8	object_hash[512 / 8];			/** Hash of object */
	union {
		u32 : 32;
		enum object_hash_type hash_type;	/** Hash type */
	};
	union {
		u32 : 32;
		enum tim_object_update_retcode	retcode;/** Return code */
	};
	u64	tim_address;		/** Media address of TIM */
	u64	tim_size;		/** Size of TIM in bytes */
	u64	data_address;		/** Object media address */
	u64	data_size;		/** Object size in bytes */
	u64	bytes_written;		/** Number of bytes written */
	u64	reserved[2];		/** Reserved for future growth */
};

/**
 * Note: the following needs to be updated in U-Boot and other update tools
 * whenever this is changed.
 */
#define UPDATE_MAGIC			0x55504454	/* UPDT */
/** Minimum allowed update version */
#define UPDATE_MIN_VERSION		0x0001
/** Minimum version that includes log support */
#define UPDATE_LOG_VERSION		0x0100
/** Minimum version with per-object return data */
#define UPDATE_OBJ_RETCODE_VERSION	0x0200
/** Current smc_update_descriptor version */
#define UPDATE_VERSION			0x0200

#define UPDATE_VERSION_0100_size	sizeof(struct smc_update_0100_descriptor)
#define UPDATE_VERSION_0200_size	sizeof(struct smc_update_descriptor)
/** Set to update secondary location */
#define UPDATE_FLAG_BACKUP		BIT(0)
/** Set to update eMMC instead of SPI */
#define UPDATE_FLAG_EMMC		BIT(1)
/** Erase eMMC partition data */
#define UPDATE_FLAG_ERASE_PART		BIT(2)
/** Don't perform version check */
#define UPDATE_FLAG_IGNORE_VERSION	BIT(3)
/** Always overwrite even if data matches */
#define UPDATE_FLAG_FORCE_WRITE		BIT(4)
/** Erase configuration data after update */
#define UPDATE_FLAG_ERASE_CONFIG	BIT(5)
/** Log update progress */
#define UPDATE_FLAG_LOG_PROGRESS	BIT(6)
/** Don't perform hash verification */
#define UPDATE_FLAG_IGNORE_HASH		BIT(7)
/** Debug */
#define UPDATE_FLAG_DEBUG		BIT(8)
/** Set when user parameters are passed */
#define UPDATE_FLAG_USER_PARMS		BIT(15)

/** Offset from the beginning of the flash where the backup image is located */
#define BACKUP_IMAGE_OFFSET		0x2000000

#define SIZE_SMC_UPDATE_DESCRIPTOR_0	80
#define SIZE_SMC_UPDATE_DESCRIPTOR_1	160

/**
 * This descriptor is passed by U-Boot or other software performing an update
 */
struct smc_update_descriptor_0100 {
	u32	magic;			/** UPDATE_MAGIC */
	u16	version;		/** Version of descriptor */
	u16	update_flags;		/** Flags passed to update process */
	u64	image_addr;		/** Address of image (CPIO file) */
	u64	image_size;		/** Size of image (CPIO file) */
	u32	bus;			/** SPI BUS number */
	u32	cs;			/** SPI chip select number */
	u32	async_operation;	/** use asynchronus SPI operations */
	u32	retcode;		/** Return code for async operations */
	u64	user_addr;		/** Passed to customer function */
	u64	user_size;		/** Passed to customer function */
	u64	user_flags;		/** Passed to customer function */
	uintptr_t	work_buffer;	/** Used for compressed objects */
	u64	work_buffer_size;	/** Size of work buffer */
	uintptr_t	output_console;	/** Text output console for update info */
	u32	output_console_size;	/** Console buffer size in bytes */
	u32	output_console_end;	/** Not used yet */
	u64	reserved2[8];
};

/**
 * This descriptor is passed by U-Boot or other software performing an update
 */
struct smc_update_descriptor {
	u32	magic;			/** UPDATE_MAGIC */
	u16	version;		/** Version of descriptor */
	u16	update_flags;		/** Flags passed to update process */
	u64	image_addr;		/** Address of image (CPIO file) */
	u64	image_size;		/** Size of image (CPIO file) */
	u32	bus;			/** SPI BUS number */
	u32	cs;			/** SPI chip select number */
	u32	async_operation;	/** use asynchronus SPI operations */
	u32	retcode;		/** Return code for async operations */
	u64	user_addr;		/** Passed to customer function */
	u64	user_size;		/** Passed to customer function */
	u64	user_flags;		/** Passed to customer function */
	uintptr_t	work_buffer;	/** Used for compressed objects */
	u64	work_buffer_size;	/** Size of work buffer */
	uintptr_t	output_console;	/** Text output console for update info */
	u32	output_console_size;	/** Console buffer size in bytes */
	u32	output_console_end;	/** Not used yet */
	u64	reserved2[8];
	struct smc_update_obj_info object_retinfo[SMC_MAX_OBJECTS];
};

#define smc_update_descr_obj_retcode smc_update_descriptor

struct smc_update_descriptor_prev {
	u32	magic;			/** UPDATE_MAGIC */
	u16	version;		/** Version of descriptor */
	u16	update_flags;		/** Flags passed to update process */
	u64	image_addr;		/** Address of image (CPIO file) */
	u64	image_size;		/** Size of image (CPIO file) */
	u32	bus;			/** SPI BUS number */
	u32	cs;			/** SPI chip select number */
	u32	async_operation;	/** use asynchronus SPI operations */
	u32	reserved;		/** Space to add stuff */
	u64	user_addr;		/** Passed to customer function */
	u64	user_size;		/** Passed to customer function */
	u64	user_flags;		/** Passed to customer function */
	uintptr_t	work_buffer;	/** Used for compressed objects */
	u64	work_buffer_size;	/** Size of work buffer */
	struct smc_update_obj_info object_retinfo[SMC_MAX_OBJECTS];
};


/* Read Flash */

/** Minimum allowed read version */
#define READ_MIN_VERSION		0x0000
/** Minimum version that includes log support */
#define READ_LOG_VERSION		0x0100
/** Current smc_read_flash_descriptor version */
#define READ_VERSION			0x0100

/** Log progress */
#define READ_FLAG_LOG_PROGRESS		BIT(0)
/** Debug */
#define READ_FLAG_DEBUG			BIT(1)

/**
 * This descriptor is used to read data from flash
 */
struct smc_read_flash_descriptor {
	u64	addr;			/** Physical buffer address */
	u64	offset;			/** Offset in flash */
	u64	length;			/** Length to read */
	u32	bus;			/** SPI BUS number */
	u32	cs;			/** SPI chip select number */
	u32	async_spi;		/** Async SPI operations */
	u16	version;		/** Version of descriptor */
	u16	read_flags;		/** Flags passed to read process */
	uintptr_t output_console;	/** Text output console */
	u32	output_console_size;	/** Console buffer size in bytes */
	u32	output_console_end;	/** Not used yet */
	u64	reserved[8];		/** Space to add stuff */
};

struct smc_read_flash_descriptor_prev {
	u64	addr;			/** Physical buffer address */
	u64	offset;			/** Offset in flash */
	u64	length;			/** Length to read */
	u32	bus;			/** SPI BUS number */
	u32	cs;			/** SPI chip select number */
	u32	async_spi;		/** Async SPI operations */
	u32	reserved;		/** Space to add stuff */
};

/** This is used for each object (version entry) */
enum smc_version_entry_retcode {
	RET_OK = 0,
	RET_NOT_FOUND = 1,
	RET_TIM_INVALID = 2,
	RET_BAD_HASH = 3,
	RET_NOT_ENOUGH_MEMORY = 4,
	/**
	 * If the names mismatch, this return code is set and the actual name
	 * found is copied into the name field.
	 */
	RET_NAME_MISMATCH = 5,
	RET_TIM_NO_VERSION = 6,
	RET_TIM_NO_HASH = 7,
	RET_HASH_ENGINE_ERROR = 8,
	RET_HASH_NO_MATCH = 9,
	/**
	 * This is returned if the length reported by the TIM header is greater
	 * than the maximum size allowed by the device tree entry.  The TIM
	 * image size will be reported as the entry size.
	 */
	RET_IMAGE_TOO_BIG = 10,
	RET_DEVICE_TREE_ENTRY_ERROR = 11,
	/** I/O error occurred during the copy operation */
	RET_BACKUP_IO_ERROR = 12
};

struct smc_version_info_entry {
	char	name[VER_MAX_NAME_LENGTH];
	struct tim_opaque_data_version_info version;
	u8	tim_hash[512 / 8];	/** Hash value stored in the TIM */
	u8	obj_hash[512 / 8];	/** Calculated hash value */
	u64	tim_address;		/** Address of TIM in flash */
	u64	tim_size;		/** Size of TIM in bytes */
	u64	max_size;		/** Maximum space for object and TIM */
	u64	object_size;		/** Size of flash object in bytes */
	u64	object_address;	/** Address of object in flash */
	u16	hash_size;		/** Size of hash in bytes */
	u16	flags;			/** Flags for this object */
	enum smc_version_entry_retcode retcode;	/** Return code if error */
	u64	perform_clone;         /** run clone operation on that image */
	u64	reserved[6];		/** Reserved for future growth */
	u8	log[VERIFY_LOG_SIZE];	/** Log for object */
};

#define VERSION_FLAG_BACKUP	BIT(0)	/** Set to use backup offset */

/**
 * Set if objects are stored in eMMC, leave zero for SPI NOR
 */
#define VERSION_FLAG_EMMC	BIT(1)

/**
 * If this bit is set, only the object names specified in the objects
 * will be checked, otherwise, all objects will be checked and any data
 * in the objects array will be ignored.
 */
#define SMC_VERSION_CHECK_SPECIFIC_OBJECTS	BIT(2)

/**
 * If set, either the specified or all of the objects will have their hashes
 * verified, otherwise, no verification will be performed.
 */
#define SMC_VERSION_CHECK_VALIDATE_HASH		BIT(3)

/**
 * Set this to copy objects to the backup flash after verification.
 * Do not set this and SCM_VERSION_COPY_TO_BACKUP_EMMC.
 */
#define SMC_VERSION_COPY_TO_BACKUP_FLASH	BIT(4)

/**
 * Set this to copy objects to the backup eMMC after verification.
 * Do not set this and SCM_VERSION_COPY_TO_BACKUP_FLASH.
 */
#define SMC_VERSION_COPY_TO_BACKUP_EMMC		BIT(5)

/**
 * Set this to copy objects to the backup flash offset after verification.
 */
#define SMC_VERSION_COPY_TO_BACKUP_OFFSET	BIT(6)

/**
 * Set this to force copy all objects into backup storage
 */
#define SMC_VERSION_FORCE_COPY_OBJECTS		BIT(7)

/**
 * Set this to enable async operations
 */
#define SMC_VERSION_ASYNC_OPERATION			BIT(8)

/**
 * Set this to skip failed images, instead of faili whole clone operation
 */
#define SMC_VERSION_SKIP_FAIL_CHECK			BIT(9)

/**
 * Set this to skip failed images, instead of faili whole clone operation
 */
#define SMC_VERSION_ERASE_EBF_CONFIG		BIT(10)

/**
 * Set this to store log progress in buffer
 */
#define SMC_VERSION_LOG_PROGRESS			BIT(11)

/**
 * Set this to get debug info
 */
#define SMC_VERSION_DEBUG					BIT(12)


/**
 * Maximum number of objects that can return the version info
 */
#define SMC_MAX_VERSION_ENTRIES			32

/** Return code for version info */
enum smc_version_ret {
	VERSION_OK,			/** Header is good */
	/**
	 * The firmware layout has changed so not all objects can be
	 * verified.  Objects are located using the device tree which
	 * is initialized at boot time.
	 */
	FIRMWARE_LAYOUT_CHANGED,
	/**
	 * If the number of objects exceeds the num_objects field then this
	 * is returned and num_objects will contain the number of objects
	 * found.
	 */
	TOO_MANY_OBJECTS,
	INVALID_DEVICE_TREE,		/** firmware-layout section missing */
	VERSION_NOT_SUPPORTED,		/** Version descriptor not supported */
	/** SMC_VERSION_CHECK_VALIDATE_HASH must be set */
	BACKUP_SRC_NOT_VALIDATED,
	/** An object failed the verification stage */
	BACKUP_SRC_FAILED_VALIDATION,
	/** Both the source and destination are the same */
	BACKUP_SRC_AND_DEST_ARE_SAME,
	/** An I/O error with the source occurred copying an object */
	BACKUP_IO_SRC_ERROR,
	/** An I/O error with the destination occurred writing an object */
	BACKUP_IO_DST_ERROR,
	/** An I/O error with the destination occurred erasing the media */
	BACKUP_IO_ERASE_ERROR,
};

#define VERSION_MAGIC		0x4e535256	/** VRSN */
#define VERSION_INFO_VERSION	0x0103		/** 1.3 */

#define VERSION_MIN_VERSION	 0x0100

/** Minimum version that includes force clone support */
#define VERSION_FORCE_CLONE_MIN_VERSION	 0x0102

/** Minimum version that includes log support */
#define VERSION_LOG_MIN_VERSION	 0x0103

/** Minimum version that includes eMMC support */

struct smc_version_info {
	u32	magic_number;		/** VRSN */
	u16	version;		/** Version of descriptor */
	u16	version_flags;		/** Flags passed to version process */
	u32	bus;			/** SPI BUS number */
	u32	cs;			/** SPI chip select number */
	u32	target_bus;		/** Target bus used for copying */
	u32	target_cs;		/** Target CS used for copying */
	/*
	 * Note that currently the work buffers are not used since the images
	 * are read from flash in chunks for verification purposes.
	 */
	uintptr_t work_buffer_addr;	/** Used to decompress objects */
	u64	work_buffer_size;	/** Size of decompression buffer */
	enum smc_version_ret	retcode;
	/**
	 * On entry, if all objects are to be verified then this contains
	 * the maximum number of objects to verify and this specifies the size
	 * of the objects array.
	 * On exit this will contain the number of objects actually verified.
	 *
	 * If the return code is TOO_MANY_OBJECTS then this will return the
	 * actual number of objects which will be greater than the number
	 * of available entries.
	 */
	u32	num_objects;
	u32	timeout;		/** Timeout in ms */
	u32	reserved32;		/** Pad to 64 bits */
	uintptr_t output_console;	/** Text output console */
	u32	output_console_size;	/** Console buffer size in bytes */
	u32	output_console_end;	/** Not used yet */
	u64	reserved[2];		/** Reserved for future growth */
	/** Array of objects to verify */
	struct smc_version_info_entry objects[SMC_MAX_VERSION_ENTRIES];
};

#endif /* __UPDATE_H__ */

/* vim:set sw=8 noet */
/* kate: tab-indent on; indent-width 8; mixedindent off; indent-mode cstyle; */
