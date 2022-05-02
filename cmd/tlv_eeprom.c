// SPDX-License-Identifier: GPL-2.0+
/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Copyright (C) 2013 Curt Brune <curt@cumulusnetworks.com>
 * Copyright (C) 2014 Srideep <srideep_devireddy@dell.com>
 * Copyright (C) 2013 Miles Tseng <miles_tseng@accton.com>
 * Copyright (C) 2014,2016 david_yang <david_yang@accton.com>
 * Copyright (C) 2022 Josua Mayer <josua@solid-run.com>
 */

#include <command.h>
#include <linux/kernel.h>
#include <stdio.h>
#include <tlv_eeprom.h>
#include <vsprintf.h>

/* File scope function prototypes */
static void show_eeprom(int devnum, u8 *eeprom);
static void show_tlv_devices(int current_dev);
static inline const char *tlv_type2name(u8 type);
static void decode_tlv(struct tlvinfo_tlv *tlv);
static int do_tlv_eeprom(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
static void show_tlv_code_list(void);

/* The EERPOM contents after being read into memory */
static u8 eeprom[TLV_INFO_MAX_LEN];

#define to_header(p) ((struct tlvinfo_header *)p)
#define to_entry(p) ((struct tlvinfo_tlv *)p)

/**
 *  show_eeprom
 *
 *  Display the contents of the EEPROM
 */
static void show_eeprom(int devnum, u8 *eeprom)
{
	int tlv_end;
	int curr_tlv;
#ifdef DEBUG
	int i;
#endif
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	struct tlvinfo_tlv    *eeprom_tlv;

	if (!is_valid_tlvinfo_header(eeprom_hdr)) {
		printf("EEPROM does not contain data in a valid TlvInfo format.\n");
		return;
	}

	printf("TLV: %u\n", devnum);
	printf("TlvInfo Header:\n");
	printf("   Id String:    %s\n", eeprom_hdr->signature);
	printf("   Version:      %d\n", eeprom_hdr->version);
	printf("   Total Length: %d\n", be16_to_cpu(eeprom_hdr->totallen));

	printf("TLV Name             Code Len Value\n");
	printf("-------------------- ---- --- -----\n");
	curr_tlv = TLV_INFO_HEADER_SIZE;
	tlv_end  = TLV_INFO_HEADER_SIZE + be16_to_cpu(eeprom_hdr->totallen);
	while (curr_tlv < tlv_end) {
		eeprom_tlv = to_entry(&eeprom[curr_tlv]);
		if (!is_valid_tlvinfo_entry(eeprom_tlv)) {
			printf("Invalid TLV field starting at EEPROM offset %d\n",
			       curr_tlv);
			return;
		}
		decode_tlv(eeprom_tlv);
		curr_tlv += TLV_INFO_ENTRY_SIZE + eeprom_tlv->length;
	}

	printf("Checksum is %s.\n",
	       tlvinfo_check_crc(eeprom) ? "valid" : "invalid");

#ifdef DEBUG
	printf("EEPROM dump: (0x%x bytes)", TLV_INFO_MAX_LEN);
	for (i = 0; i < TLV_INFO_MAX_LEN; i++) {
		if ((i % 16) == 0)
			printf("\n%02X: ", i);
		printf("%02X ", eeprom[i]);
	}
	printf("\n");
#endif
}

/**
 *  Struct for displaying the TLV codes and names.
 */
struct tlv_code_desc {
	u8    m_code;
	char *m_name;
};

/**
 *  List of TLV codes and names.
 */
static struct tlv_code_desc tlv_code_list[] = {
	{ TLV_CODE_PRODUCT_NAME,   "Product Name"},
	{ TLV_CODE_PART_NUMBER,    "Part Number"},
	{ TLV_CODE_SERIAL_NUMBER,  "Serial Number"},
	{ TLV_CODE_MAC_BASE,       "Base MAC Address"},
	{ TLV_CODE_MANUF_DATE,     "Manufacture Date"},
	{ TLV_CODE_DEVICE_VERSION, "Device Version"},
	{ TLV_CODE_LABEL_REVISION, "Label Revision"},
	{ TLV_CODE_PLATFORM_NAME,  "Platform Name"},
	{ TLV_CODE_ONIE_VERSION,   "ONIE Version"},
	{ TLV_CODE_MAC_SIZE,       "MAC Addresses"},
	{ TLV_CODE_MANUF_NAME,     "Manufacturer"},
	{ TLV_CODE_MANUF_COUNTRY,  "Country Code"},
	{ TLV_CODE_VENDOR_NAME,    "Vendor Name"},
	{ TLV_CODE_DIAG_VERSION,   "Diag Version"},
	{ TLV_CODE_SERVICE_TAG,    "Service Tag"},
	{ TLV_CODE_VENDOR_EXT,     "Vendor Extension"},
	{ TLV_CODE_CRC_32,         "CRC-32"},
};

/**
 *  Look up a TLV name by its type.
 */
static inline const char *tlv_type2name(u8 type)
{
	char *name = "Unknown";
	int   i;

	for (i = 0; i < ARRAY_SIZE(tlv_code_list); i++) {
		if (tlv_code_list[i].m_code == type) {
			name = tlv_code_list[i].m_name;
			break;
		}
	}

	return name;
}

/*
 *  decode_tlv
 *
 *  Print a string representing the contents of the TLV field. The format of
 *  the string is:
 *      1. The name of the field left justified in 20 characters
 *      2. The type code in hex right justified in 5 characters
 *      3. The length in decimal right justified in 4 characters
 *      4. The value, left justified in however many characters it takes
 *  The validity of EEPROM contents and the TLV field have been verified
 *  prior to calling this function.
 */
#define DECODE_NAME_MAX     20

/*
 * The max decode value is currently for the 'raw' type or the 'vendor
 * extension' type, both of which have the same decode format.  The
 * max decode string size is computed as follows:
 *
 *   strlen(" 0xFF") * TLV_VALUE_MAX_LEN + 1
 *
 */
#define DECODE_VALUE_MAX    ((5 * TLV_VALUE_MAX_LEN) + 1)

static void decode_tlv(struct tlvinfo_tlv *tlv)
{
	char name[DECODE_NAME_MAX];
	char value[DECODE_VALUE_MAX];
	int i;

	strncpy(name, tlv_type2name(tlv->type), DECODE_NAME_MAX);

	switch (tlv->type) {
	case TLV_CODE_PRODUCT_NAME:
	case TLV_CODE_PART_NUMBER:
	case TLV_CODE_SERIAL_NUMBER:
	case TLV_CODE_MANUF_DATE:
	case TLV_CODE_LABEL_REVISION:
	case TLV_CODE_PLATFORM_NAME:
	case TLV_CODE_ONIE_VERSION:
	case TLV_CODE_MANUF_NAME:
	case TLV_CODE_MANUF_COUNTRY:
	case TLV_CODE_VENDOR_NAME:
	case TLV_CODE_DIAG_VERSION:
	case TLV_CODE_SERVICE_TAG:
		memcpy(value, tlv->value, tlv->length);
		value[tlv->length] = 0;
		break;
	case TLV_CODE_MAC_BASE:
		sprintf(value, "%02X:%02X:%02X:%02X:%02X:%02X",
			tlv->value[0], tlv->value[1], tlv->value[2],
			tlv->value[3], tlv->value[4], tlv->value[5]);
		break;
	case TLV_CODE_DEVICE_VERSION:
		sprintf(value, "%u", tlv->value[0]);
		break;
	case TLV_CODE_MAC_SIZE:
		sprintf(value, "%u", (tlv->value[0] << 8) | tlv->value[1]);
		break;
	case TLV_CODE_VENDOR_EXT:
		value[0] = 0;
		for (i = 0; (i < (DECODE_VALUE_MAX / 5)) && (i < tlv->length);
				i++) {
			sprintf(value, "%s 0x%02X", value, tlv->value[i]);
		}
		break;
	case TLV_CODE_CRC_32:
		sprintf(value, "0x%02X%02X%02X%02X",
			tlv->value[0], tlv->value[1],
			tlv->value[2], tlv->value[3]);
		break;
	default:
		value[0] = 0;
		for (i = 0; (i < (DECODE_VALUE_MAX / 5)) && (i < tlv->length);
				i++) {
			sprintf(value, "%s 0x%02X", value, tlv->value[i]);
		}
		break;
	}

	name[DECODE_NAME_MAX - 1] = 0;
	printf("%-20s 0x%02X %3d %s\n", name, tlv->type, tlv->length, value);
}

/**
 *  show_tlv_code_list - Display the list of TLV codes and names
 */
static void show_tlv_code_list(void)
{
	int i;

	printf("TLV Code    TLV Name\n");
	printf("========    =================\n");
	for (i = 0; i < ARRAY_SIZE(tlv_code_list); i++) {
		printf("0x%02X        %s\n",
		       tlv_code_list[i].m_code,
		       tlv_code_list[i].m_name);
	}
}

/**
 *  do_tlv_eeprom
 *
 *  This function implements the tlv_eeprom command.
 */
static int do_tlv_eeprom(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char cmd;
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	static unsigned int current_dev;
	/* Set to devnum if we've read EEPROM into memory */
	static int has_been_read = -1;

	// If no arguments, read the EERPOM and display its contents
	if (argc == 1) {
		if (has_been_read != current_dev) {
			if (read_tlv_eeprom(eeprom, 0, TLV_INFO_MAX_LEN, current_dev) == 0)
				has_been_read = current_dev;
		}
		show_eeprom(current_dev, eeprom);
		return 0;
	}

	// We only look at the first character to the command, so "read" and
	// "reset" will both be treated as "read".
	cmd = argv[1][0];

	// Read the EEPROM contents
	if (cmd == 'r') {
		has_been_read = -1;
		if (read_tlv_eeprom(eeprom, 0, TLV_INFO_MAX_LEN, current_dev) == 0) {
			printf("EEPROM data loaded from device to memory.\n");
			has_been_read = current_dev;
		}
		return 0;
	}

	// Subsequent commands require that the EEPROM has already been read.
	if (has_been_read != current_dev) {
		printf("Please read the EEPROM data first, using the 'tlv_eeprom read' command.\n");
		return 0;
	}

	// Handle the commands that don't take parameters
	if (argc == 2) {
		switch (cmd) {
		case 'w':   /* write */
			write_tlvinfo_tlv_eeprom(eeprom, current_dev);
			break;
		case 'e':   /* erase */
			strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
			eeprom_hdr->version = TLV_INFO_VERSION;
			eeprom_hdr->totallen = cpu_to_be16(0);
			tlvinfo_update_crc(eeprom);
			printf("EEPROM data in memory reset.\n");
			break;
		case 'l':   /* list */
			show_tlv_code_list();
			break;
		case 'd':   /* dev */
			show_tlv_devices(current_dev);
			break;
		default:
			cmd_usage(cmdtp);
			break;
		}
		return 0;
	}

	// The set command takes one or two args.
	if (argc > 4) {
		cmd_usage(cmdtp);
		return 0;
	}

	// Set command. If the TLV exists in the EEPROM, delete it. Then if
	// data was supplied for this TLV add the TLV with the new contents at
	// the end.
	if (cmd == 's') {
		int tcode;

		tcode = simple_strtoul(argv[2], NULL, 0);
		tlvinfo_delete_tlv(eeprom, tcode);
		if (argc == 4)
			tlvinfo_add_tlv(eeprom, tcode, argv[3]);
	} else if (cmd == 'd') { /* 'dev' command */
		unsigned int devnum;

		devnum = simple_strtoul(argv[2], NULL, 0);
		if (!exists_tlv_eeprom(devnum)) {
			printf("Invalid device number\n");
			return 0;
		}
		current_dev = devnum;
	} else {
		cmd_usage(cmdtp);
	}

	return 0;
}

/**
 *  This macro defines the tlv_eeprom command line command.
 */
U_BOOT_CMD(tlv_eeprom, 4, 1,  do_tlv_eeprom,
	   "Display and program the system EEPROM data block.",
	   "[read|write|set <type_code> <string_value>|erase|list]\n"
	   "tlv_eeprom\n"
	   "    - With no arguments display the current contents.\n"
	   "tlv_eeprom dev [dev]\n"
	   "    - List devices or set current EEPROM device.\n"
	   "tlv_eeprom read\n"
	   "    - Load EEPROM data from device to memory.\n"
	   "tlv_eeprom write\n"
	   "    - Write the EEPROM data to persistent storage.\n"
	   "tlv_eeprom set <type_code> <string_value>\n"
	   "    - Set a field to a value.\n"
	   "    - If no string_value, field is deleted.\n"
	   "    - Use 'tlv_eeprom write' to make changes permanent.\n"
	   "tlv_eeprom erase\n"
	   "    - Reset the in memory EEPROM data.\n"
	   "    - Use 'tlv_eeprom read' to refresh the in memory EEPROM data.\n"
	   "    - Use 'tlv_eeprom write' to make changes permanent.\n"
	   "tlv_eeprom list\n"
	   "    - List the understood TLV codes and names.\n"
	);

static void show_tlv_devices(int current_dev)
{
	unsigned int dev;

	for (dev = 0; dev < TLV_MAX_DEVICES; dev++)
		if (exists_tlv_eeprom(dev))
			printf("TLV: %u%s\n", dev,
			       (dev == current_dev) ? " (*)" : "");
}
