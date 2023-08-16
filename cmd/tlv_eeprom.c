// SPDX-License-Identifier: GPL-2.0+
/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Copyright (C) 2013 Curt Brune <curt@cumulusnetworks.com>
 * Copyright (C) 2014 Srideep <srideep_devireddy@dell.com>
 * Copyright (C) 2013 Miles Tseng <miles_tseng@accton.com>
 * Copyright (C) 2014,2016 david_yang <david_yang@accton.com>
 * Copyright (C) 2023 Josua Mayer <josua@solid-run.com>
 */

#include <command.h>
#include <linux/err.h>
#include <net.h>
#include <stdio.h>
#include <tlv_eeprom.h>
#include <vsprintf.h>

DECLARE_GLOBAL_DATA_PTR;

static void decode_tlv(struct tlvinfo_tlv *tlv);
static bool tlvinfo_add_tlv(struct tlvinfo_priv *header, int tcode, char *strval);
static int set_mac(char *buf, const char *string);
static int set_date(char *buf, const char *string);
static int set_bytes(char *buf, const char *string, int *converted_accum);

/* The EEPROM contents after being read into memory */
static u8 eeprom[TLV_INFO_MAX_LEN];

static void show_tlv_devices(int current_dev);

static inline bool is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

/**
 *  is_hex
 *
 *  Tests if character is an ASCII hex digit
 */
static inline u8 is_hex(char p)
{
	return (((p >= '0') && (p <= '9')) ||
		   ((p >= 'A') && (p <= 'F')) ||
		   ((p >= 'a') && (p <= 'f')));
}

/**
 *  show_eeprom
 *
 *  Display the contents of the EEPROM
 */
static void show_eeprom(int devnum, struct tlvinfo_priv *tlv)
{
#ifdef DEBUG
	int i;
#endif
	struct tlvinfo_tlv    *entry = NULL;
	struct tlvinfo_header *eeprom_hdr = tlv_header_get(tlv);

	printf("TLV: %u\n", devnum);
	printf("TlvInfo Header:\n");
	printf("   Id String:    %s\n", eeprom_hdr->signature);
	printf("   Version:      %d\n", eeprom_hdr->version);
	printf("   Total Length: %d\n", be16_to_cpu(eeprom_hdr->totallen));

	printf("TLV Name             Code Len Value\n");
	printf("-------------------- ---- --- -----\n");
	entry = tlv_entry_next(tlv, entry);
	while (!IS_ERR(entry)) {
		decode_tlv(entry);
		entry = tlv_entry_next(tlv, entry);
	}
	if (PTR_ERR(entry) != -ENOENT)
		printf("Failed to get next entry: %ld\n", PTR_ERR(entry));

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
void show_tlv_code_list(void)
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
int do_tlv_eeprom(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char cmd;
	static struct tlvinfo_priv *tlv;
	static unsigned int current_dev;
	struct tlvinfo_tlv *entry;
	int ret;

	// If no arguments, read the EERPOM and display its contents
	if (argc == 1) {
		if (!tlv) {
			tlv = tlv_eeprom_read(tlv_eeprom_get_by_index(current_dev),
								  0, eeprom, ARRAY_SIZE(eeprom));
			if (IS_ERR(tlv)) {
				printf("Failed to read EEPROM data from device: %ld\n",
					   PTR_ERR(tlv));

				printf("Creating new TLV structure in-memory: ");
				tlv = tlv_init(eeprom, ARRAY_SIZE(eeprom));
				if (IS_ERR(tlv)) {
					printf("Error %i\n", (int)PTR_ERR(tlv));
					tlv = NULL;
				} else
					printf("Okay\n");

				return 0;
			}
		}
		show_eeprom(current_dev, tlv);
		return 0;
	}

	// We only look at the first character to the command, so "read" and
	// "reset" will both be treated as "read".
	cmd = argv[1][0];

	// select device
	if (cmd == 'd') {
		 /* 'dev' command */
		unsigned int devnum;

		devnum = simple_strtoul(argv[2], NULL, 0);
		if (devnum >= MAX_TLV_DEVICES) {
			printf("Invalid device number\n");
			return 0;
		}
		current_dev = devnum;
		tlv = NULL;

		return 0;
	}

	// Read the EEPROM contents
	if (cmd == 'r') {
		tlv = tlv_eeprom_read(tlv_eeprom_get_by_index(current_dev),
								 0, eeprom, ARRAY_SIZE(eeprom));
		if (IS_ERR(tlv)) {
			if (tlv == -EINVAL) {
				printf("Bad TLV data, initilising new EEPROM \n");
				tlv = tlv_init(eeprom, ARRAY_SIZE(eeprom));
			} else {
				printf("Failed to read EEPROM data from device: %ld\n",
					PTR_ERR(tlv));
				tlv = NULL;
			}
		}
		printf("EEPROM data loaded from device to memory.\n");
		return 0;
	}

	// Subsequent commands require that the EEPROM has already been read.
	if (!tlv) {
		printf("Please read the EEPROM data first, using the 'tlv_eeprom read' command.\n");
		return 0;
	}

	// Handle the commands that don't take parameters
	if (argc == 2) {
		switch (cmd) {
		case 'w':   /* write */
			ret = tlv_eeprom_write(tlv_eeprom_get_by_index(current_dev), 0, tlv);
			if (ret)
				printf("Programming failed: %i\n", ret);
			else
				printf("Programming passed.\n");
			break;
		case 'e':   /* erase */
			tlv = tlv_init(eeprom, ARRAY_SIZE(eeprom));
			if (IS_ERR(tlv)) {
				printf("Failed to initiailise TLV structure in memory: %ld\n",
					   PTR_ERR(tlv));
				return 0;
			}

			printf("EEPROM data in memory reset.\n");
			break;
		case 'l':   /* list */
			show_tlv_code_list();
			break;
		case 'd':   /* dev */
			show_tlv_devices(current_dev);
			break;
		default:
			return CMD_RET_USAGE;
		}
		return 0;
	}

	// The set command takes one or two args.
	if (argc > 4)
		return CMD_RET_USAGE;

	// Set command. If the TLV exists in the EEPROM, delete it. Then if
	// data was supplied for this TLV add the TLV with the new contents at
	// the end.
	if (cmd == 's') {
		int tcode;

		tcode = simple_strtoul(argv[2], NULL, 0);
		entry = tlv_entry_next_by_code(tlv, NULL, tcode);
		ret = tlv_entry_remove(tlv, tlv_entry_next_by_code(tlv, NULL, tcode));
		if (ret && ret != -ENOENT) {
			printf("Failed to remove existing tlv entry: %d.\n", ret);
			return 0;
		}
		if (argc == 4)
			tlvinfo_add_tlv(tlv, tcode, argv[3]);
		else {
			// update CRC
			ret = tlv_crc_update(tlv);
			if (ret)
				printf("Failed to update CRC: %d\n", ret);
		}
	} else {
		return CMD_RET_USAGE;
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

/**
 *  tlvinfo_add_tlv
 *
 *  This function adds a TLV to the EEPROM, converting the value (a string) to
 *  the format in which it will be stored in the EEPROM.
 */
#define MAX_TLV_VALUE_LEN   256
static bool tlvinfo_add_tlv(struct tlvinfo_priv *tlv, int tcode, char *strval)
{
	int ret;
	struct tlvinfo_tlv *entry;
	int new_tlv_len = 0;
	u32 value;
	char data[MAX_TLV_VALUE_LEN];

	// Encode each TLV type into the format to be stored in the EERPOM
	switch (tcode) {
	case TLV_CODE_PRODUCT_NAME:
	case TLV_CODE_PART_NUMBER:
	case TLV_CODE_SERIAL_NUMBER:
	case TLV_CODE_LABEL_REVISION:
	case TLV_CODE_PLATFORM_NAME:
	case TLV_CODE_ONIE_VERSION:
	case TLV_CODE_MANUF_NAME:
	case TLV_CODE_MANUF_COUNTRY:
	case TLV_CODE_VENDOR_NAME:
	case TLV_CODE_DIAG_VERSION:
	case TLV_CODE_SERVICE_TAG:
		strncpy(data, strval, MAX_TLV_VALUE_LEN);
		new_tlv_len = min_t(size_t, MAX_TLV_VALUE_LEN, strlen(strval));
		break;
	case TLV_CODE_DEVICE_VERSION:
		value = simple_strtoul(strval, NULL, 0);
		if (value >= 256) {
			printf("ERROR: Device version must be 255 or less. Value supplied: %u",
			       value);
			return false;
		}
		data[0] = value & 0xFF;
		new_tlv_len = 1;
		break;
	case TLV_CODE_MAC_SIZE:
		value = simple_strtoul(strval, NULL, 0);
		if (value >= 65536) {
			printf("ERROR: MAC Size must be 65535 or less. Value supplied: %u",
			       value);
			return false;
		}
		data[0] = (value >> 8) & 0xFF;
		data[1] = value & 0xFF;
		new_tlv_len = 2;
		break;
	case TLV_CODE_MANUF_DATE:
		if (set_date(data, strval) != 0)
			return false;
		new_tlv_len = 19;
		break;
	case TLV_CODE_MAC_BASE:
		if (set_mac(data, strval) != 0)
			return false;
		new_tlv_len = 6;
		break;
	case TLV_CODE_CRC_32:
		printf("WARNING: The CRC TLV is set automatically and cannot be set manually.\n");
		return false;
	case TLV_CODE_VENDOR_EXT:
	default:
		if (set_bytes(data, strval, &new_tlv_len) != 0)
			return false;
		break;
	}

	entry = tlv_entry_add(tlv, NULL, tcode, new_tlv_len);
	if (IS_ERR(entry) && PTR_ERR(entry) == -ENOMEM) {
		printf("ERROR: There is not enough room in the EERPOM to save data.\n");
		return false;
	} else if (IS_ERR(entry)) {
		printf("Failed to create new tlv entry: %ld\n", PTR_ERR(entry));
		return false;
	}

	// copy data into new entry
	ret = tlv_entry_set_raw(entry, data, new_tlv_len);
	if (ret) {
		printf("Failed to set new tlv entry value: %d\n", ret);
		return false;
	}

	// update CRC
	ret = tlv_crc_update(tlv);
	if (ret)
		printf("Failed to update CRC: %d\n", ret);

	return true;
}

/**
 *  set_mac
 *
 *  Converts a string MAC address into a binary buffer.
 *
 *  This function takes a pointer to a MAC address string
 *  (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number).
 *  The string format is verified and then converted to binary and
 *  stored in a buffer.
 */
static int set_mac(char *buf, const char *string)
{
	char *p = (char *)string;
	int   i;
	int   err = 0;
	char *end;

	if (!p) {
		printf("ERROR: NULL mac addr string passed in.\n");
		return -1;
	}

	if (strlen(p) != 17) {
		printf("ERROR: MAC address strlen() != 17 -- %zu\n", strlen(p));
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	for (i = 0; i < 17; i++) {
		if ((i % 3) == 2) {
			if (p[i] != ':') {
				err++;
				printf("ERROR: mac: p[%i] != :, found: `%c'\n",
				       i, p[i]);
				break;
			}
			continue;
		} else if (!is_hex(p[i])) {
			err++;
			printf("ERROR: mac: p[%i] != hex digit, found: `%c'\n",
			       i, p[i]);
			break;
		}
	}

	if (err != 0) {
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	/* Convert string to binary */
	for (i = 0, p = (char *)string; i < 6; i++) {
		buf[i] = p ? hextoul(p, &end) : 0;
		if (p)
			p = (*end) ? end + 1 : end;
	}

	if (!is_valid_ethaddr((u8 *)buf)) {
		printf("ERROR: MAC address must not be 00:00:00:00:00:00, a multicast address or FF:FF:FF:FF:FF:FF.\n");
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	return 0;
}

/**
 *  set_date
 *
 *  Validates the format of the data string
 *
 *  This function takes a pointer to a date string (i.e. MM/DD/YYYY hh:mm:ss)
 *  and validates that the format is correct. If so the string is copied
 *  to the supplied buffer.
 */
static int set_date(char *buf, const char *string)
{
	int i;

	if (!string) {
		printf("ERROR: NULL date string passed in.\n");
		return -1;
	}

	if (strlen(string) != 19) {
		printf("ERROR: Date strlen() != 19 -- %zu\n", strlen(string));
		printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
		       string);
		return -1;
	}

	for (i = 0; string[i] != 0; i++) {
		switch (i) {
		case 2:
		case 5:
			if (string[i] != '/') {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
				       string);
				return -1;
			}
			break;
		case 10:
			if (string[i] != ' ') {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
				       string);
				return -1;
			}
			break;
		case 13:
		case 16:
			if (string[i] != ':') {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
				       string);
				return -1;
			}
			break;
		default:
			if (!is_digit(string[i])) {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n",
				       string);
				return -1;
			}
			break;
		}
	}

	strcpy(buf, string);
	return 0;
}

/**
 *  set_bytes
 *
 *  Converts a space-separated string of decimal numbers into a
 *  buffer of bytes.
 *
 *  This function takes a pointer to a space-separated string of decimal
 *  numbers (i.e. "128 0x55 0321") with "C" standard radix specifiers
 *  and converts them to an array of bytes.
 */
static int set_bytes(char *buf, const char *string, int *converted_accum)
{
	char *p = (char *)string;
	int   i;
	uint  byte;

	if (!p) {
		printf("ERROR: NULL string passed in.\n");
		return -1;
	}

	/* Convert string to bytes */
	for (i = 0, p = (char *)string; (i < TLV_VALUE_MAX_LEN) && (*p != 0);
			i++) {
		while ((*p == ' ') || (*p == '\t') || (*p == ',') ||
		       (*p == ';')) {
			p++;
		}
		if (*p != 0) {
			if (!is_digit(*p)) {
				printf("ERROR: Non-digit found in byte string: (%s)\n",
				       string);
				return -1;
			}
			byte = simple_strtoul(p, &p, 0);
			if (byte >= 256) {
				printf("ERROR: The value specified is greater than 255: (%u) in string: %s\n",
				       byte, string);
				return -1;
			}
			buf[i] = byte & 0xFF;
		}
	}

	if (i == TLV_VALUE_MAX_LEN && (*p != 0)) {
		printf("ERROR: Trying to assign too many bytes (max: %d) in string: %s\n",
		       TLV_VALUE_MAX_LEN, string);
		return -1;
	}

	*converted_accum = i;
	return 0;
}

static void show_tlv_devices(int current_dev)
{
	int dev;
	int i;

	for (i = 0; i < MAX_TLV_DEVICES; i++)
		if (!IS_ERR(tlv_eeprom_get_by_index(dev)))
			printf("TLV: %u%s\n", dev,
			       (dev == current_dev) ? " (*)" : "");
}

/**
 *  mac_read_from_eeprom
 *
 *  Read the MAC addresses from EEPROM
 *
 *  This function reads the MAC addresses from EEPROM and sets the
 *  appropriate environment variables for each one read.
 *
 *  The environment variables are only set if they haven't been set already.
 *  This ensures that any user-saved variables are never overwritten.
 *
 *  This function must be called after relocation.
 */
int mac_read_from_eeprom(void)
{
	unsigned int i;
	struct tlvinfo_header *eeprom_hdr;
	struct tlvinfo_tlv *eeprom_tlv;
	uint16_t maccount;
	u8 macbase[6];
	struct tlvinfo_priv *tlv;
	int devnum = 0; // TODO: support multiple EEPROMs

	puts("EEPROM: ");

	tlv = tlv_eeprom_read(tlv_eeprom_get_by_index(devnum), 0, eeprom, ARRAY_SIZE(eeprom));
	if (IS_ERR(tlv)) {
		printf("Read failed.\n");
		return -1;
	}
	eeprom_hdr = tlv_header_get(tlv);

	maccount = 0;
	eeprom_tlv = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_MAC_SIZE);
	if (!IS_ERR(eeprom_tlv))
		tlv_entry_get_uint16(eeprom_tlv, &maccount);

	memcpy(macbase, "\0\0\0\0\0\0", 6);
	eeprom_tlv = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_MAC_BASE);
	if (!IS_ERR(eeprom_tlv))
		tlv_entry_get_raw(eeprom_tlv, macbase, ARRAY_SIZE(macbase));

	for (i = 0; i < maccount; i++) {
		if (is_valid_ethaddr(macbase)) {
			char ethaddr[18];
			char enetvar[11];

			sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
				macbase[0], macbase[1], macbase[2],
				macbase[3], macbase[4], macbase[5]);
			sprintf(enetvar, i ? "eth%daddr" : "ethaddr", i);
			/* Only initialize environment variables that are blank
			 * (i.e. have not yet been set)
			 */
			if (!env_get(enetvar))
				env_set(enetvar, ethaddr);

			macbase[5]++;
			if (macbase[5] == 0) {
				macbase[4]++;
				if (macbase[4] == 0) {
					macbase[3]++;
					if (macbase[3] == 0) {
						macbase[0] = 0;
						macbase[1] = 0;
						macbase[2] = 0;
					}
				}
			}
		}
	}

	printf("%s v%u len=%u\n", eeprom_hdr->signature, eeprom_hdr->version,
	       be16_to_cpu(eeprom_hdr->totallen));

	return 0;
}

/**
 *  populate_serial_number - read the serial number from EEPROM
 *
 *  This function reads the serial number from the EEPROM and sets the
 *  appropriate environment variable.
 *
 *  The environment variable is only set if it has not been set
 *  already.  This ensures that any user-saved variables are never
 *  overwritten.
 *
 *  This function must be called after relocation.
 */
int populate_serial_number(void)
{
	char serialstr[257];
	struct tlvinfo_priv *tlv;
	struct tlvinfo_tlv *eeprom_tlv;
	int devnum = 0; // TODO: support multiple EEPROMs

	if (env_get("serial#"))
		return 0;

	tlv = tlv_eeprom_read(tlv_eeprom_get_by_index(devnum), 0, eeprom, ARRAY_SIZE(eeprom));
	if (IS_ERR(tlv)) {
		printf("Read failed.\n");
		return -1;
	}

	eeprom_tlv = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_SERIAL_NUMBER);
	if (!IS_ERR(eeprom_tlv)) {
		tlv_entry_get_string(eeprom_tlv, serialstr, ARRAY_SIZE(serialstr));
		env_set("serial#", serialstr);
	}

	return 0;
}
