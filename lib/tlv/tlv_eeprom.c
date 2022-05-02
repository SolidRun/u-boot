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

#include <common.h>
#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <asm/global_data.h>
#include <linux/ctype.h>
#include <u-boot/crc.h>

#include "tlv_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

/* File scope function prototypes */
static int read_eeprom(int devnum, u8 *eeprom);
static int set_mac(char *buf, const char *string);
static int set_date(char *buf, const char *string);
static int set_bytes(char *buf, const char *string, int *converted_accum);

/* The EERPOM contents after being read into memory */
static u8 eeprom[TLV_INFO_MAX_LEN];

static struct udevice *tlv_devices[TLV_MAX_DEVICES];

#define to_header(p) ((struct tlvinfo_header *)p)
#define to_entry(p) ((struct tlvinfo_tlv *)p)

/**
 * Check whether eeprom device exists.
 */
bool exists_tlv_eeprom(int dev)
{
	return dev < TLV_MAX_DEVICES && tlv_devices[dev] != 0;
}

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
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 */
bool tlvinfo_check_crc(u8 *eeprom)
{
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	struct tlvinfo_tlv    *eeprom_crc;
	unsigned int       calc_crc;
	unsigned int       stored_crc;

	// Is the eeprom header valid?
	if (!is_valid_tlvinfo_header(eeprom_hdr))
		return false;

	// Is the last TLV a CRC?
	eeprom_crc = to_entry(&eeprom[TLV_INFO_HEADER_SIZE +
		be16_to_cpu(eeprom_hdr->totallen) - (TLV_INFO_ENTRY_SIZE + 4)]);
	if (eeprom_crc->type != TLV_CODE_CRC_32 || eeprom_crc->length != 4)
		return false;

	// Calculate the checksum
	calc_crc = crc32(0, (void *)eeprom,
			 TLV_INFO_HEADER_SIZE + be16_to_cpu(eeprom_hdr->totallen) - 4);
	stored_crc = (eeprom_crc->value[0] << 24) |
		(eeprom_crc->value[1] << 16) |
		(eeprom_crc->value[2] <<  8) |
		eeprom_crc->value[3];
	return calc_crc == stored_crc;
}

/**
 *  read_eeprom
 *
 *  Read the EEPROM into memory, if it hasn't already been read.
 */
static int read_eeprom(int devnum, u8 *eeprom)
{
	int ret;
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	struct tlvinfo_tlv *eeprom_tlv = to_entry(&eeprom[TLV_INFO_HEADER_SIZE]);

	/* Read the header */
	ret = read_tlv_eeprom((void *)eeprom_hdr, 0, TLV_INFO_HEADER_SIZE, devnum);
	/* If the header was successfully read, read the TLVs */
	if (ret == 0 && is_valid_tlvinfo_header(eeprom_hdr))
		ret = read_tlv_eeprom((void *)eeprom_tlv, TLV_INFO_HEADER_SIZE,
				      be16_to_cpu(eeprom_hdr->totallen), devnum);

	// If the contents are invalid, start over with default contents
	if (!is_valid_tlvinfo_header(eeprom_hdr) ||
	    !tlvinfo_check_crc(eeprom)) {
		strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
		eeprom_hdr->version = TLV_INFO_VERSION;
		eeprom_hdr->totallen = cpu_to_be16(0);
		tlvinfo_update_crc(eeprom);
	}

#ifdef DEBUG
	show_eeprom(devnum, eeprom);
#endif

	return ret;
}

/**
 *  tlvinfo_update_crc
 *
 *  This function updates the CRC-32 TLV. If there is no CRC-32 TLV, then
 *  one is added. This function should be called after each update to the
 *  EEPROM structure, to make sure the CRC is always correct.
 */
void tlvinfo_update_crc(u8 *eeprom)
{
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	struct tlvinfo_tlv    *eeprom_crc;
	unsigned int      calc_crc;
	int               eeprom_index;

	// Discover the CRC TLV
	if (!tlvinfo_find_tlv(eeprom, TLV_CODE_CRC_32, &eeprom_index)) {
		unsigned int totallen = be16_to_cpu(eeprom_hdr->totallen);

		if ((totallen + TLV_INFO_ENTRY_SIZE + 4) > TLV_TOTAL_LEN_MAX)
			return;
		eeprom_index = TLV_INFO_HEADER_SIZE + totallen;
		eeprom_hdr->totallen = cpu_to_be16(totallen + TLV_INFO_ENTRY_SIZE + 4);
	}
	eeprom_crc = to_entry(&eeprom[eeprom_index]);
	eeprom_crc->type = TLV_CODE_CRC_32;
	eeprom_crc->length = 4;

	// Calculate the checksum
	calc_crc = crc32(0, (void *)eeprom,
			 TLV_INFO_HEADER_SIZE + be16_to_cpu(eeprom_hdr->totallen) - 4);
	eeprom_crc->value[0] = (calc_crc >> 24) & 0xFF;
	eeprom_crc->value[1] = (calc_crc >> 16) & 0xFF;
	eeprom_crc->value[2] = (calc_crc >>  8) & 0xFF;
	eeprom_crc->value[3] = (calc_crc >>  0) & 0xFF;
}

/**
 *  write_tlvinfo_tlv_eeprom
 *
 *  Write the TLV data from CPU memory to the hardware.
 */
int write_tlvinfo_tlv_eeprom(void *eeprom, int dev)
{
	int ret = 0;
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	int eeprom_len;

	tlvinfo_update_crc(eeprom);

	eeprom_len = TLV_INFO_HEADER_SIZE + be16_to_cpu(eeprom_hdr->totallen);
	ret = write_tlv_eeprom(eeprom, eeprom_len, dev);
	if (ret) {
		printf("Programming failed.\n");
		return -1;
	}

	printf("Programming passed.\n");
	return 0;
}

/**
 *  tlvinfo_find_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM.
 *  An offset from the beginning of the EEPROM is returned in the
 *  eeprom_index parameter if the TLV is found.
 */
bool tlvinfo_find_tlv(u8 *eeprom, u8 tcode, int *eeprom_index)
{
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	struct tlvinfo_tlv    *eeprom_tlv;
	int eeprom_end;

	// Search through the TLVs, looking for the first one which matches the
	// supplied type code.
	*eeprom_index = TLV_INFO_HEADER_SIZE;
	eeprom_end = TLV_INFO_HEADER_SIZE + be16_to_cpu(eeprom_hdr->totallen);
	while (*eeprom_index < eeprom_end) {
		eeprom_tlv = to_entry(&eeprom[*eeprom_index]);
		if (!is_valid_tlvinfo_entry(eeprom_tlv))
			return false;
		if (eeprom_tlv->type == tcode)
			return true;
		*eeprom_index += TLV_INFO_ENTRY_SIZE + eeprom_tlv->length;
	}
	return(false);
}

/**
 *  tlvinfo_delete_tlv
 *
 *  This function deletes the TLV with the specified type code from the
 *  EEPROM.
 */
bool tlvinfo_delete_tlv(u8 *eeprom, u8 code)
{
	int eeprom_index;
	int tlength;
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	struct tlvinfo_tlv *eeprom_tlv;

	// Find the TLV and then move all following TLVs "forward"
	if (tlvinfo_find_tlv(eeprom, code, &eeprom_index)) {
		eeprom_tlv = to_entry(&eeprom[eeprom_index]);
		tlength = TLV_INFO_ENTRY_SIZE + eeprom_tlv->length;
		memcpy(&eeprom[eeprom_index], &eeprom[eeprom_index + tlength],
		       TLV_INFO_HEADER_SIZE +
		       be16_to_cpu(eeprom_hdr->totallen) - eeprom_index -
		       tlength);
		eeprom_hdr->totallen =
			cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) -
				    tlength);
		tlvinfo_update_crc(eeprom);
		return true;
	}
	return false;
}

/**
 *  tlvinfo_add_tlv
 *
 *  This function adds a TLV to the EEPROM, converting the value (a string) to
 *  the format in which it will be stored in the EEPROM.
 */
#define MAX_TLV_VALUE_LEN   256
bool tlvinfo_add_tlv(u8 *eeprom, int tcode, char *strval)
{
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	struct tlvinfo_tlv *eeprom_tlv;
	int new_tlv_len = 0;
	u32 value;
	char data[MAX_TLV_VALUE_LEN];
	int eeprom_index;

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

	// Is there room for this TLV?
	if ((be16_to_cpu(eeprom_hdr->totallen) + TLV_INFO_ENTRY_SIZE + new_tlv_len) >
			TLV_TOTAL_LEN_MAX) {
		printf("ERROR: There is not enough room in the EERPOM to save data.\n");
		return false;
	}

	// Add TLV at the end, overwriting CRC TLV if it exists
	if (tlvinfo_find_tlv(eeprom, TLV_CODE_CRC_32, &eeprom_index))
		eeprom_hdr->totallen =
			cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) -
					TLV_INFO_ENTRY_SIZE - 4);
	else
		eeprom_index = TLV_INFO_HEADER_SIZE + be16_to_cpu(eeprom_hdr->totallen);
	eeprom_tlv = to_entry(&eeprom[eeprom_index]);
	eeprom_tlv->type = tcode;
	eeprom_tlv->length = new_tlv_len;
	memcpy(eeprom_tlv->value, data, new_tlv_len);

	// Update the total length and calculate (add) a new CRC-32 TLV
	eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) +
			TLV_INFO_ENTRY_SIZE + new_tlv_len);
	tlvinfo_update_crc(eeprom);

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

static int find_tlv_devices(struct udevice **tlv_devices_p)
{
	int ret;
	int count_dev = 0;
	struct udevice *dev;

	for (ret = uclass_first_device_check(UCLASS_I2C_EEPROM, &dev);
			dev;
			ret = uclass_next_device_check(&dev)) {
		if (ret == 0)
			tlv_devices_p[count_dev++] = dev;
		if (count_dev >= TLV_MAX_DEVICES)
			break;
	}

	return (count_dev == 0) ? -ENODEV : 0;
}

static struct udevice *find_tlv_device_by_index(int dev_num)
{
	struct udevice *local_tlv_devices[TLV_MAX_DEVICES] = {};
	struct udevice **tlv_devices_p;
	int ret;

	if (gd->flags & (GD_FLG_RELOC | GD_FLG_SPL_INIT)) {
		/* Assume BSS is initialized; use static data */
		if (tlv_devices[dev_num])
			return tlv_devices[dev_num];
		tlv_devices_p = tlv_devices;
	} else {
		tlv_devices_p = local_tlv_devices;
	}

	ret = find_tlv_devices(tlv_devices_p);
	if (ret == 0 && tlv_devices_p[dev_num])
		return tlv_devices_p[dev_num];

	return NULL;
}

/**
 * read_tlv_eeprom - read the hwinfo from i2c EEPROM
 */
int read_tlv_eeprom(void *eeprom, int offset, int len, int dev_num)
{
	struct udevice *dev;

	if (dev_num >= TLV_MAX_DEVICES)
		return -EINVAL;

	dev = find_tlv_device_by_index(dev_num);
	if (!dev)
		return -ENODEV;

	return i2c_eeprom_read(dev, offset, eeprom, len);
}

/**
 * write_tlv_eeprom - write the hwinfo to i2c EEPROM
 */
int write_tlv_eeprom(void *eeprom, int len, int dev)
{
	if (!(gd->flags & GD_FLG_RELOC))
		return -ENODEV;
	if (!tlv_devices[dev])
		return -ENODEV;

	return i2c_eeprom_write(tlv_devices[dev], 0, eeprom, len);
}

int read_tlvinfo_tlv_eeprom(void *eeprom, struct tlvinfo_header **hdr,
			    struct tlvinfo_tlv **first_entry, int dev_num)
{
	int ret;
	struct tlvinfo_header *tlv_hdr;
	struct tlvinfo_tlv *tlv_ent;

	/* Read TLV header */
	ret = read_tlv_eeprom(eeprom, 0, TLV_INFO_HEADER_SIZE, dev_num);
	if (ret < 0)
		return ret;

	tlv_hdr = eeprom;
	if (!is_valid_tlvinfo_header(tlv_hdr))
		return -EINVAL;

	/* Read TLV entries */
	tlv_ent = to_entry(&tlv_hdr[1]);
	ret = read_tlv_eeprom(tlv_ent, TLV_INFO_HEADER_SIZE,
			      be16_to_cpu(tlv_hdr->totallen), dev_num);
	if (ret < 0)
		return ret;
	if (!tlvinfo_check_crc(eeprom))
		return -EINVAL;

	*hdr = tlv_hdr;
	*first_entry = tlv_ent;

	return 0;
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
	int eeprom_index;
	struct tlvinfo_tlv *eeprom_tlv;
	int maccount;
	u8 macbase[6];
	struct tlvinfo_header *eeprom_hdr = to_header(eeprom);
	int devnum = 0; // TODO: support multiple EEPROMs

	puts("EEPROM: ");

	if (read_eeprom(devnum, eeprom)) {
		printf("Read failed.\n");
		return -1;
	}

	maccount = 1;
	if (tlvinfo_find_tlv(eeprom, TLV_CODE_MAC_SIZE, &eeprom_index)) {
		eeprom_tlv = to_entry(&eeprom[eeprom_index]);
		maccount = (eeprom_tlv->value[0] << 8) | eeprom_tlv->value[1];
	}

	memcpy(macbase, "\0\0\0\0\0\0", 6);
	if (tlvinfo_find_tlv(eeprom, TLV_CODE_MAC_BASE, &eeprom_index)) {
		eeprom_tlv = to_entry(&eeprom[eeprom_index]);
		memcpy(macbase, eeprom_tlv->value, 6);
	}

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
int populate_serial_number(int devnum)
{
	char serialstr[257];
	int eeprom_index;
	struct tlvinfo_tlv *eeprom_tlv;

	if (env_get("serial#"))
		return 0;

	if (read_eeprom(devnum, eeprom)) {
		printf("Read failed.\n");
		return -1;
	}

	if (tlvinfo_find_tlv(eeprom, TLV_CODE_SERIAL_NUMBER, &eeprom_index)) {
		eeprom_tlv = to_entry(&eeprom[eeprom_index]);
		memcpy(serialstr, eeprom_tlv->value, eeprom_tlv->length);
		serialstr[eeprom_tlv->length] = 0;
		env_set("serial#", serialstr);
	}

	return 0;
}
