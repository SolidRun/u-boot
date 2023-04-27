/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#ifndef __TLV_EEPROM_H_
#define __TLV_EEPROM_H_

#if !defined(CONFIG_EEPROM_TLV_LIB) && !defined(CONFIG_SPL_EEPROM_TLV_LIB)

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

#if CONFIG_IS_ENABLED(CMD_TLV_EEPROM)

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

#else /* !CONFIG_IS_ENABLED(CMD_TLV_EEPROM) */

static inline int read_tlv_eeprom(void *eeprom, int offset, int len, int dev)
{
	return -ENOSYS;
}

static inline int write_tlv_eeprom(void *eeprom, int len)
{
	return -ENOSYS;
}

static inline int
read_tlvinfo_tlv_eeprom(void *eeprom, struct tlvinfo_header **hdr,
			struct tlvinfo_tlv **first_entry, int dev)
{
	return -ENOSYS;
}

#endif /* CONFIG_IS_ENABLED(CMD_TLV_EEPROM) */

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

#else

/*
 *  The Definition of the TlvInfo EEPROM format can be found at onie.org or
 *  github.com/onie
 */

#include <dm/device.h>
#include <i2c_eeprom.h>
#include <stdbool.h>

/* tlv library internal state, per each tlv structure */
struct tlvinfo_priv;

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
#define TLV_INFO_HEADER_SIZE    sizeof(struct tlvinfo_header)
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_INFO_MAX_LEN        2048
#define TLV_TOTAL_LEN_MAX       (TLV_INFO_MAX_LEN - TLV_INFO_HEADER_SIZE)

/*
 * TlvInfo TLV: Layout of a TLV field
 */
struct __attribute__ ((__packed__)) tlvinfo_tlv {
	u8  type;
	u8  length;
	u8  value[];
};

#define TLV_INFO_ENTRY_SIZE      sizeof(struct tlvinfo_tlv)
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
#define MAX_TLV_DEVICES			2

/*
 * EEPROM<->TLV API
 */

/**
 * Find EEPROM device by index.
 *
 * @index: index of eeprom in the system, 0 = first.
 * @return: handle to eeprom device on success, error pointer otherwise.
 */
struct udevice *tlv_eeprom_get_by_index(unsigned int index);

/**
 * Read TLV formatted data from EEPROM.
 *
 * @dev: EEPROM device handle.
 * @offset: Offset into EEPROM to read from.
 * @buffer: Buffer for storing TLV structure.
 * @buffer_size: Size of the buffer.
 * @return: Buffer initialised with TLV structure, or error pointer.
 */
struct tlvinfo_priv *const tlv_eeprom_read(struct udevice *dev, int offset, u8 *buffer, size_t buffer_size);

/**
 * Write TLV formatted data to EEPROM.
 *
 * @dev: EEPROM device handle.
 * @offset: Offset into EEPROM to write to.
 * @tlv: Pointer to TLV structure.
 * @return: Status code.
 */
int tlv_eeprom_write(struct udevice *dev, int offset, struct tlvinfo_priv *priv);

/*
 * TLV API
 */

/**
 * Initialise new TLV structure.
 *
 * @buffer: Buffer for storing TLV structure.
 * @buffer_size: Size of the buffer.
 * @return: Buffer initialised with TLV structure, or error pointer.
 */
struct tlvinfo_priv *const tlv_init(u8 *buffer, size_t buffer_size);

/**
 * Access TLV Header
 */
struct tlvinfo_header *const tlv_header_get(struct tlvinfo_priv *const priv);

/**
 * Add new entry to TLV structure.
 *
 * @priv: Pointer to TLV structure.
 * @offset: Pointer inside TLV structure where to insert new element. May be NULL to insert at the end before CRC, otherwise the new entry shall be inserted before the specified reference element..
 * @code: TLV code number for this entry.
 * @size: Data size for this entry.
 * @return: Pointer to TLV entry, or error pointer.
 */
struct tlvinfo_tlv *const tlv_entry_add(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const offset, u8 code, u8 size);

/**
 * Remove entry from TLV structure.
 *
 * @tlv: Pointer to TLV structure.
 * @entry: Pointer to TLV entry.
 * @return: Status code.
 */
int tlv_entry_remove(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const entry);

/**
 * Get the next TLV entry.
 *
 * @tlv: Pointer to TLV structure.
 * @offset: Start search after this entry; Pass NULL to search from the beginning.
 * @return: Pointer to TLV entry, or error code.
 */
struct tlvinfo_tlv *const tlv_entry_next(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const offset);

/**
 * Get the next TLV entry by code.
 *
 * @tlv: Pointer to TLV structure.
 * @code: TLV code number to search.
 * @offset: Start search after this entry; Pass NULL to search from the beginning.
 * @return: Pointer to TLV entry, or error code.
 */
struct tlvinfo_tlv *const tlv_entry_next_by_code(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const offset, u8 code);

/**
 * Update existing CRC entry, or add new one if missing.
 *
 * @tlv: Pointer to TLV structure.
 */
int tlv_crc_update(struct tlvinfo_priv *const priv);

/*
 * TLV data get/set API
 * (Convenience wrappers around struct tlvinfo_tlv)
 */

/**
 * Get TLV entry binary data.
 *
 * @entry: Pointer to TLV entry.
 * @buffer: Destination buffer for data.
 * @size: Size of the buffer.
 * @return: Status code.
 */
int tlv_entry_get_raw(struct tlvinfo_tlv *const entry, u8 *buffer, size_t size);

/**
 * Set TLV entry binary data.
 *
 * @entry: Pointer to TLV entry.
 * @values: Source buffer with data.
 * @size: Length of the data.
 * @return: Status code.
 *
 * Note: The size of entries can not be changed!
 */
int tlv_entry_set_raw(struct tlvinfo_tlv *const entry, const u8 *values, size_t length);

/**
 * Get TLV entry data as null-terminated string.
 *
 * @entry: Pointer to TLV entry.
 * @buffer: Destination buffer for string.
 * @size: Size of the buffer.
 * @return: Status code.
 */
int tlv_entry_get_string(struct tlvinfo_tlv *const entry, char *buffer, size_t size);

/**
 * Set TLV entry data from null-terminated string.
 *
 * @entry: Pointer to TLV entry.
 * @values: Source buffer with string.
 * @return: Status code.
 *
 * Note: The size of entries can not be changed!
 */
int tlv_entry_set_string(struct tlvinfo_tlv *const entry, const char *string);

/**
 * Get TLV entry data as uint8 value.
 *
 * @entry: Pointer to TLV entry.
 * @buffer: Destination buffer for data.
 * @return: Status code.
 */
int tlv_entry_get_uint8(struct tlvinfo_tlv *const entry, u8 *buffer);

/**
 * Set TLV entry data from uint8 value.
 *
 * @entry: Pointer to TLV entry.
 * @value: Source value.
 * @return: Status code.
 *
 * Note: The size of entries can not be changed!
 */
int tlv_entry_set_uint8(struct tlvinfo_tlv *const entry, const u8 value);

/**
 * Get TLV entry data as uint16 value.
 *
 * @entry: Pointer to TLV entry.
 * @buffer: Destination buffer for data.
 * @return: Status code.
 */
int tlv_entry_get_uint16(struct tlvinfo_tlv *const entry, u16 *buffer);

/**
 * Set TLV entry data from uint16 value.
 *
 * @entry: Pointer to TLV entry.
 * @value: Source value.
 * @return: Status code.
 *
 * Note: The size of fields can not be changed!
 */
int tlv_entry_set_uint16(struct tlvinfo_tlv *const entry, const u16 value);

/**
 * Get TLV entry data as uint32 value.
 *
 * @entry: Pointer to TLV entry.
 * @buffer: Destination buffer for data.
 * @return: Status code.
 */
int tlv_entry_get_uint32(struct tlvinfo_tlv *const entry, u32 *buffer);

/**
 * Set TLV entry data from uint16 value.
 *
 * @entry: Pointer to TLV entry.
 * @value: Source value.
 * @return: Status code.
 *
 * Note: The size of fields can not be changed!
 */
int tlv_entry_set_uint32(struct tlvinfo_tlv *const entry, const u32 value);

#endif
#endif /* __TLV_EEPROM_H_ */
