/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#ifndef __TLV_EEPROM_H_
#define __TLV_EEPROM_H_

/*
 *  The Definition of the TlvInfo EEPROM format can be found at onie.org or
 *  github.com/onie
 */

/*
 * TlvInfo header: Layout of the header for the TlvInfo format
 *
 * See the end of this file for details of this eeprom format
 */
struct __attribute__ ((__packed__)) tlvinfo_header {
	char    signature[8]; /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
	u8      version;      /* 0x08        Structure version    */
	u16     totallen;     /* 0x09 - 0x0A Length of all data which follows */
};

// Header Field Constants
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_INFO_MAX_LEN        2048
#define TLV_TOTAL_LEN_MAX       (TLV_INFO_MAX_LEN - \
				sizeof(struct tlvinfo_header))

/*
 * TlvInfo TLV: Layout of a TLV field
 */
struct __attribute__ ((__packed__)) tlvinfo_tlv {
	u8  type;
	u8  length;
	u8  value[0];
};

/* Maximum length of a TLV value in bytes */
#define TLV_VALUE_MAX_LEN        255

/**
 *  The TLV Types.
 *
 *  Keep these in sync with tlv_code_list in cmd/tlv_eeprom.c
 */
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_DIAG_VERSION   0x2E
#define TLV_CODE_SERVICE_TAG    0x2F
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE

/* how many EEPROMs can be used */
#define TLV_MAX_DEVICES			2

/**
 * read_tlv_eeprom - Read the EEPROM binary data from the hardware
 * @eeprom: Pointer to buffer to hold the binary data
 * @offset: Offset within EEPROM block to read data from
 * @len   : Maximum size of buffer
 * @dev   : EEPROM device to read
 *
 * Note: this routine does not validate the EEPROM data.
 *
 */

int read_tlv_eeprom(void *eeprom, int offset, int len, int dev);

/**
 * write_tlv_eeprom - Write the entire EEPROM binary data to the hardware
 * @eeprom: Pointer to buffer to hold the binary data
 * @len   : Maximum size of buffer
 * @dev   : EEPROM device to write
 *
 * Note: this routine does not validate the EEPROM data.
 *
 */
int write_tlv_eeprom(void *eeprom, int len, int dev);

/**
 * read_tlvinfo_tlv_eeprom - Read the TLV from EEPROM, and validate
 * @eeprom: Pointer to buffer to hold the binary data. Must point to a buffer
 *          of size at least TLV_INFO_MAX_LEN.
 * @hdr   : Points to pointer to TLV header (output)
 * @first_entry : Points to pointer to first TLV entry (output)
 * @dev   : EEPROM device to read
 *
 * Store the raw EEPROM data from EEPROM @dev in the @eeprom buffer. If TLV is
 * valid set *@hdr and *@first_entry.
 *
 * Returns 0 when read from EEPROM is successful, and the data is valid.
 * Returns <0 error value when EEPROM read fails. Return -EINVAL when TLV is
 * invalid.
 *
 */

int read_tlvinfo_tlv_eeprom(void *eeprom, struct tlvinfo_header **hdr,
			    struct tlvinfo_tlv **first_entry, int dev);

/**
 * Write TLV data to the EEPROM.
 *
 * - Only writes length of actual tlv data
 * - updates checksum
 *
 * @eeprom: Pointer to buffer to hold the binary data. Must point to a buffer
 *          of size at least TLV_INFO_MAX_LEN.
 * @dev   : EEPROM device to write
 *
 */
int write_tlvinfo_tlv_eeprom(void *eeprom, int dev);

/**
 *  tlvinfo_find_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM.
 *  An offset from the beginning of the EEPROM is returned in the
 *  eeprom_index parameter if the TLV is found.
 */
bool tlvinfo_find_tlv(u8 *eeprom, u8 tcode, int *eeprom_index);

/**
 *  tlvinfo_update_crc
 *
 *  This function updates the CRC-32 TLV. If there is no CRC-32 TLV, then
 *  one is added. This function should be called after each update to the
 *  EEPROM structure, to make sure the CRC is always correct.
 *
 * @eeprom: Pointer to buffer to hold the binary data. Must point to a buffer
 *          of size at least TLV_INFO_MAX_LEN.
 */
void tlvinfo_update_crc(u8 *eeprom);

/**
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 *
 * @eeprom: Pointer to buffer to hold the binary data. Must point to a buffer
 *          of size at least TLV_INFO_MAX_LEN.
 */
bool tlvinfo_check_crc(u8 *eeprom);

/**
 *  is_valid_tlvinfo_header
 *
 *  Perform sanity checks on the first 11 bytes of the TlvInfo EEPROM
 *  data pointed to by the parameter:
 *      1. First 8 bytes contain null-terminated ASCII string "TlvInfo"
 *      2. Version byte is 1
 *      3. Total length bytes contain value which is less than or equal
 *         to the allowed maximum (2048-11)
 *
 */
static inline bool is_valid_tlvinfo_header(struct tlvinfo_header *hdr)
{
	return ((strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) &&
		(hdr->version == TLV_INFO_VERSION) &&
		(be16_to_cpu(hdr->totallen) <= TLV_TOTAL_LEN_MAX));
}

/**
 *  is_valid_tlv
 *
 *  Perform basic sanity checks on a TLV field. The TLV is pointed to
 *  by the parameter provided.
 *      1. The type code is not reserved (0x00 or 0xFF)
 */
static inline bool is_valid_tlvinfo_entry(struct tlvinfo_tlv *tlv)
{
	return((tlv->type != 0x00) && (tlv->type != 0xFF));
}

#endif /* __TLV_EEPROM_H_ */
