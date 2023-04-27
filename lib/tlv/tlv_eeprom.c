// SPDX-License-Identifier: GPL-2.0+

#define DEBUG

#include <compiler.h>
#include <dm/uclass.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <string.h>
#include <tlv_eeprom.h>
#include <u-boot/crc.h>

/*
 * internal state specific to each TLV structure
 */
struct __attribute__ ((__packed__)) tlvinfo_priv {
	size_t size;
	size_t length;
	struct tlvinfo_header header;
	struct tlvinfo_tlv entries[];
};

#define TLVINFO_PRIV_SIZE sizeof(struct tlvinfo_priv)

/*
 * Calculate offset in bytes relative to end of header
 */
static inline ssize_t tlv_offset(struct tlvinfo_priv *priv, struct tlvinfo_tlv *entry)
{
	return (uint8_t *)entry - (uint8_t *)priv->entries;
}

/*
 * Access tlv entry at specific offset in bytes relative to header
 */
static inline struct tlvinfo_tlv *tlv_at_offset(struct tlvinfo_priv *priv, size_t offset)
{
	u8 *buffer = (void *)priv->entries;
	return (void *)&buffer[offset];
}

/*
 * Move TLV data between offsets
 */
static inline void tlv_move(struct tlvinfo_priv *priv, size_t dest_offset, size_t src_offset) {
	u8 *buffer = (void *)priv->entries;
	memmove(&buffer[dest_offset], &buffer[src_offset], priv->length - src_offset);
}

/*
 * check element is fully inside structure
 */
static inline bool tlv_check_bounds(struct tlvinfo_priv *priv, struct tlvinfo_tlv *entry)
{
	ssize_t offset = tlv_offset(priv, entry);

	if (offset < 0 ||
		offset + TLV_INFO_ENTRY_SIZE > priv->length ||
		offset + TLV_INFO_ENTRY_SIZE + entry->length > priv->length) {
		pr_debug("%s:%d: element at offset %zd outside tlv structure\n", __FILE__, __LINE__, offset);
		return false;
	}

	return true;
}

/**
 *  is_valid_tlvinfo_header
 *
 *  Perform sanity checks on the first 11 bytes of the TlvInfo EEPROM
 *  data pointed to by the parameter:
 *      1. First 8 bytes contain null-terminated ASCII string "TlvInfo"
 *      2. Version byte is 1
 *      3. Total length bytes contain value which is less than or equal
 *         to the allowed maximum (2048-11)
 */
static inline bool is_valid_tlvinfo_header(struct tlvinfo_header *hdr)
{
	return ((strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) &&
			(hdr->version == TLV_INFO_VERSION) &&
			(be16_to_cpu(hdr->totallen) <= TLV_TOTAL_LEN_MAX));
}

/**
 * Calculate TLV Checksum
 */
static inline uint32_t tlvinfo_calc_crc(struct tlvinfo_priv *priv)
{
	uint32_t crc;

	/* calculate crc32 for complete structure, excluding final 4 byte (crc location) */
	crc = crc32(0, (void *)&priv->header, TLV_INFO_HEADER_SIZE + priv->length - 4);

	return crc;
}

/**
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 */
static bool tlvinfo_check_crc(struct tlvinfo_priv *priv)
{
	struct tlvinfo_tlv *entry;
	size_t offset_crc;
	unsigned int       calc_crc;
	unsigned int       stored_crc;

	/* find CRC entry at end */
	offset_crc = priv->length - TLV_INFO_ENTRY_SIZE - 4;
	entry = tlv_at_offset(priv, offset_crc);

	/* check structure contains space for crc entry */
	if (offset_crc < 0 || priv->length < offset_crc + TLV_INFO_ENTRY_SIZE + 4) {
		pr_debug("%s:%d: crc at offset %zd outside tlv structure\n",__FILE__, __LINE__, offset_crc);
		return false;
	}

	/* ensure crc entry is correct */
	if (entry->type != TLV_CODE_CRC_32 || entry->length != 4) {
		pr_debug("%s:%d: crc tlv entry has illegal length or type: Have 0x%x length %u, expect 0xDE length 4\n", __FILE__, __LINE__, entry->type, entry->length);
		return false;
	}

	/* copy stored crc value */
	tlv_entry_get_uint32(entry, &stored_crc);

	/* calculate crc from data */
	calc_crc = tlvinfo_calc_crc(priv);

	/* compare */
	return calc_crc == stored_crc;
}

struct udevice *tlv_eeprom_get_by_index(unsigned int index)
{
	int ret;
	int count_dev = 0;
	struct udevice *dev;

	for (ret = uclass_first_device_check(UCLASS_I2C_EEPROM, &dev);
		 dev;
		 ret = uclass_next_device_check(&dev)) {
		if (ret == 0 && count_dev++ == index)
			return dev;
		if (count_dev >= MAX_TLV_DEVICES)
			break;
	}

	return ERR_PTR(-ENODEV);


/*	struct uclass *uc;
	struct udevice *dev;
	int i;

	uclass_id_foreach_dev(UCLASS_I2C_EEPROM, dev, uc) {
		pr_debug("%s:%d: have eeprom device at index %u\n", __FILE__, __LINE__, i);
		if (i++ == index)
			return dev;
	}

	pr_debug("%s:%d: couldn't find eeprom device index %u\n", __FILE__, __LINE__, index);
	return ERR_PTR(-ENODEV);*/
}

struct tlvinfo_priv *const tlv_eeprom_read(struct udevice *dev, int offset, u8 *buffer, size_t buffer_size)
{
	int ret;
	struct tlvinfo_priv *priv;

	if (!dev) {
		pr_debug("%s:%d: device handle is NULL\n", __FILE__, __LINE__);
		return ERR_PTR(-EINVAL);
	} else if (IS_ERR(dev)) {
		pr_debug("%s:%d: device handle is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(dev));
		return (void *)dev;
	}

	/* check device type */
	if (device_get_uclass_id(dev) != UCLASS_I2C_EEPROM) {
		pr_debug("%s:%d: device handle is not an eeprom\n", __FILE__, __LINE__);
		return ERR_PTR(-EINVAL);
	}

	/* initialise private data */
	priv = tlv_init(buffer, buffer_size);
	if (IS_ERR(priv)) {
		pr_debug("%s:%d: failed to initialise in-memory tlv structure: %i\n", __FILE__, __LINE__, (int)PTR_ERR(priv));
		return priv;
	}

	/* read header */
	ret = i2c_eeprom_read(dev, offset, (void *)&priv->header, TLV_INFO_HEADER_SIZE);
	if (ret) {
		pr_debug("%s:%d: failed to read from eeprom: %i\n", __FILE__, __LINE__, ret);
		return ERR_PTR(ret);
	}

	/* validate header */
	if (!is_valid_tlvinfo_header(&priv->header)) {
		pr_warn("TLV header is invalid!\n");
		return ERR_PTR(-EINVAL);
	}

	/* copy length from header */
	priv->length = be16_to_cpu(priv->header.totallen);

	/* check buffer is sufficient for complete tlv data */
	if (priv->size < priv->length) {
		pr_warn("buffer too small for TLV data: Have %zu, need %zu\n", buffer_size, TLVINFO_PRIV_SIZE + TLV_INFO_HEADER_SIZE + priv->length);
		return ERR_PTR(-ENOBUFS);
	}

	/* read complete tlv data according to size indicated by header */
	ret = i2c_eeprom_read(dev, offset + TLV_INFO_HEADER_SIZE, (void *)&priv->entries, priv->length);
	if (ret) {
		pr_debug("%s:%d: failed to read from eeprom: %i\n", __FILE__, __LINE__, ret);
		return ERR_PTR(ret);
	}

	/* validate checksum */
	if (!tlvinfo_check_crc(priv)) {
		pr_err("TLV Checksum is invalid or missing!\n");
		/* ignore this error to allow inspecting data */
	}

	/* return data */
	return priv;
}

int tlv_eeprom_write(struct udevice *dev, int offset, struct tlvinfo_priv *const priv)
{
	int ret;

	if (!dev) {
		pr_debug("%s:%d: device handle is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	} else if (IS_ERR(dev)) {
		pr_debug("%s:%d: device handle is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(dev));
		return PTR_ERR(dev);
	}

	ret = i2c_eeprom_write(dev, offset, (void *)&priv->header, TLV_INFO_HEADER_SIZE + priv->length);
	if (ret)
		pr_debug("%s:%d: failed to write to eeprom: %i\n", __FILE__, __LINE__, ret);

	return ret;
}

struct tlvinfo_priv *const tlv_init(u8 *buffer, size_t buffer_size)
{
	struct tlvinfo_priv *priv;
	struct tlvinfo_tlv *entry;
	uint32_t crc;

	/* check buffer is sufficient for private data & header */
	if (!buffer || buffer_size < TLVINFO_PRIV_SIZE + TLV_INFO_HEADER_SIZE) {
		pr_debug("%s:%d: buffer insufficient for private data and tlv header: Have %zu, need %zu\n", __FILE__, __LINE__, buffer_size, TLVINFO_PRIV_SIZE + TLV_INFO_HEADER_SIZE);
		return ERR_PTR(-ENOBUFS);
	}

	/* initialise private structure */
	priv = (void *)buffer;
	priv->size = buffer_size - TLVINFO_PRIV_SIZE - TLV_INFO_HEADER_SIZE;
	priv->length = 0;

	/* initialise header */
	strcpy(priv->header.signature, TLV_INFO_ID_STRING);
	priv->header.version = TLV_INFO_VERSION;
	priv->header.totallen = cpu_to_be16(0);

	/* add crc entry */
	entry = tlv_entry_add(priv, NULL, TLV_CODE_CRC_32, 4);
	if (IS_ERR(entry)) {
		pr_debug("%s:%d: failed to create crc entry: %i\n", __FILE__, __LINE__, (int)PTR_ERR(entry));
		return (void *)entry;
	}

	/* calculate crc */
	crc = tlvinfo_calc_crc(priv);
	tlv_entry_set_uint32(entry, crc);

	return priv;
}

struct tlvinfo_header *const tlv_header_get(struct tlvinfo_priv *priv)
{
	if (!priv) {
		pr_debug("%s:%d: private handle is NULL\n", __FILE__, __LINE__);
		return ERR_PTR(-EINVAL);
	} else if (IS_ERR(priv)) {
		pr_debug("%s:%d: private handle is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(priv));
		return (void *)PTR_ERR(priv);
	}

	return &priv->header;
}

struct tlvinfo_tlv *const tlv_entry_add(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const _offset, u8 code, u8 size)
{
	struct tlvinfo_tlv *crc;
	struct tlvinfo_tlv *entry;
	ssize_t offset;

	if (!priv) {
		pr_debug("%s:%d: private handle is NULL\n", __FILE__, __LINE__);
		return ERR_PTR(-EINVAL);
	} else if (IS_ERR(priv)) {
		pr_debug("%s:%d: private handle is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(priv));
		return (void *)PTR_ERR(priv);
	}

	if (IS_ERR(_offset)) {
		pr_debug("%s:%d: offset is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(_offset));
		return (void *)_offset;
	}

	if (_offset) {
		/* check offset element is inside structure */
		if (!tlv_check_bounds(priv, _offset))
			return ERR_PTR(-EINVAL);

		offset = tlv_offset(priv, _offset);
	} else if (priv->length != 0) {
		/* by default insert before CRC */
		crc = tlv_at_offset(priv, priv->length - TLV_INFO_ENTRY_SIZE - 4);

		/* check crc element is inside structure */
		if (!tlv_check_bounds(priv, crc))
			return ERR_PTR(-EINVAL);

		/* check crc element is really crc */
		if (crc->type == TLV_CODE_CRC_32) {
			if (crc->length != 4)
				return ERR_PTR(-EINVAL);

			/* insert before crc */
			offset = tlv_offset(priv, crc);
		} else
			/* otherwise insert at end */
			offset = priv->length;
	} else {
		/* insert at end */
		offset = priv->length;
	}

	/* check buffer is sufficient for new entry */
	if (priv->size < offset + TLV_INFO_ENTRY_SIZE + size) {
		pr_debug("%s:%d: buffer insufficient for additional tlv entry: Have %zu, need %zu\n", __FILE__, __LINE__, TLVINFO_PRIV_SIZE + TLV_INFO_HEADER_SIZE + priv->size, TLVINFO_PRIV_SIZE + TLV_INFO_HEADER_SIZE + offset + TLV_INFO_ENTRY_SIZE + size);
		return ERR_PTR(-ENOBUFS);
	}

	/* move existing data to make space */
	tlv_move(priv, offset + TLV_INFO_ENTRY_SIZE + size, offset);

	/* initialise new entry */
	entry = tlv_at_offset(priv, offset);
	entry->type = code;
	entry->length = size;
	memset(entry->value, 0, size);

	/* update total length */
	priv->length += TLV_INFO_ENTRY_SIZE + size;
	priv->header.totallen = cpu_to_be16(priv->length);

	return entry;
}

int tlv_entry_remove(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const entry)
{
	ssize_t offset;
	size_t end;

	if (!priv) {
		pr_debug("%s:%d: private handle is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	} else if (IS_ERR(priv)) {
		pr_debug("%s:%d: private handle is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(priv));
		return PTR_ERR(priv);
	}

	if (!entry) {
		pr_debug("%s:%d: entry is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	} else if (IS_ERR(entry)) {
		pr_debug("%s:%d: entry is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(entry));
		return PTR_ERR(entry);
	}

	/* calculate internal offset */
	offset = tlv_offset(priv, entry);

	/* check entry within structure */
	if (!tlv_check_bounds(priv, entry))
		return -EINVAL;

	/* move existing data from end into gap */
	end = offset + TLV_INFO_ENTRY_SIZE + entry->length;
	tlv_move(priv, offset, end);

	/* update total length */
	priv->length -= end - offset;
	priv->header.totallen = cpu_to_be16(priv->length);

	return 0;
}

struct tlvinfo_tlv *const tlv_entry_next(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const _offset)
{
	ssize_t offset = 0;
	struct tlvinfo_tlv *entry;

	if (!priv) {
		pr_debug("%s:%d: private handle is NULL\n", __FILE__, __LINE__);
		return ERR_PTR(-EINVAL);
	} else if (IS_ERR(priv)) {
		pr_debug("%s:%d: private handle is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(priv));
		return (void *)PTR_ERR(priv);
	}

	if (IS_ERR(_offset)) {
		pr_debug("%s:%d: reference element is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(_offset));
		return (void *)_offset;
	}

	/* by default search from beginning */
	entry = priv->entries;

	/* search after reference entry, if any */
	if (_offset) {
		offset = tlv_offset(priv, _offset);

		/* check offset element is inside structure */
		if (!tlv_check_bounds(priv, _offset))
			return ERR_PTR(-EINVAL);

		/* seek beyond reference element */
		offset += _offset->length + TLV_INFO_ENTRY_SIZE;
		entry = tlv_at_offset(priv, offset);
	}

	/* check for end of tlv data */
	if (offset == priv->length)
		return ERR_PTR(-ENOENT);

	/* check element is inside structure */
	if (!tlv_check_bounds(priv, entry))
		return ERR_PTR(-EINVAL);

	return entry;
}

struct tlvinfo_tlv *const tlv_entry_next_by_code(struct tlvinfo_priv *const priv, struct tlvinfo_tlv *const offset, u8 code)
{
	struct tlvinfo_tlv *entry;

	for (entry = tlv_entry_next(priv, offset);
		 !IS_ERR(entry);
		 entry = tlv_entry_next(priv, entry))
		if (entry->type == code)
			return entry;

	return ERR_PTR(-ENOENT);
}

int tlv_crc_update(struct tlvinfo_priv *const priv)
{
	struct tlvinfo_tlv *crc;
	size_t offset;
	unsigned int       calc_crc;

	/* find CRC entry at end */
	offset = priv->length - TLV_INFO_ENTRY_SIZE - 4;
	crc = tlv_at_offset(priv, offset);

	/* check structure contains space for crc entry */
	if (offset < 0 || priv->length < offset + TLV_INFO_ENTRY_SIZE + 4) {
		pr_debug("%s:%d: crc at offset %zd outside tlv structure\n",__FILE__, __LINE__, offset);

		/* create new crc entry at end */
		crc = tlv_entry_add(priv, NULL, TLV_CODE_CRC_32, 4);
	}

	/* ensure crc entry is correct */
	if (crc->type != TLV_CODE_CRC_32 || crc->length != 4) {
		pr_debug("%s:%d: crc tlv entry has illegal length or type: Have 0x%x length %u, expect 0xDE length 4\n", __FILE__, __LINE__, crc->type, crc->length);

		/* create new crc entry at end */
		tlv_entry_remove(priv, crc);
		crc = tlv_entry_add(priv, tlv_at_offset(priv, priv->length), TLV_CODE_CRC_32, 4);
	}

	/* calculate crc from data */
	calc_crc = tlvinfo_calc_crc(priv);

	/* update crc value */
	return tlv_entry_set_uint32(crc, calc_crc);
}

int tlv_entry_get_raw(struct tlvinfo_tlv *const entry, u8 *buffer, size_t size)
{
	if (!entry) {
		pr_debug("%s:%d: tlv entry is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	} else if (IS_ERR(entry)) {
		pr_debug("%s:%d: tlv entry is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(entry));
		return PTR_ERR(entry);
	}

	if (!buffer || entry->length > size) {
		pr_debug("%s:%d: buffer insufficient: Have %zu, need %u\n", __FILE__, __LINE__, size, entry->length);
		return -ENOBUFS;
	}

	memcpy(buffer, &entry->value[0], entry->length);

	return 0;
}

int tlv_entry_set_raw(struct tlvinfo_tlv *const entry, const u8 *values, size_t length)
{
	if (!entry) {
		pr_debug("%s:%d: tlv entry is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	} else if (IS_ERR(entry)) {
		pr_debug("%s:%d: tlv entry is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(entry));
		return PTR_ERR(entry);
	}

	if (entry->length < length) {
		pr_debug("%s:%d: tlv entry too small: Have %u, need %zu\n", __FILE__, __LINE__, entry->length, length);
		return -ENOBUFS;
	}

	/* set values */
	memcpy(&entry->value[0], values, length);

	return 0;
}

int tlv_entry_get_string(struct tlvinfo_tlv *const entry, char *buffer, size_t size)
{
	if (!entry) {
		pr_debug("%s:%d: tlv entry is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	} else if (IS_ERR(entry)) {
		pr_debug("%s:%d: tlv entry is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(entry));
		return PTR_ERR(entry);
	}

	if (!buffer || size <= entry->length) {
		pr_debug("%s:%d: buffer insufficient: Have %zu, need %u\n", __FILE__, __LINE__, size, entry->length + 1);
		return -ENOBUFS;
	}

	memcpy(buffer, &entry->value[0], entry->length);
	buffer[entry->length] = '\0';
	return 0;
}

int tlv_entry_set_string(struct tlvinfo_tlv *const entry, const char *string)
{
	if (!entry) {
		pr_debug("%s:%d: tlv entry is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	} else if (IS_ERR(entry)) {
		pr_debug("%s:%d: tlv entry is error: %i\n", __FILE__, __LINE__, (int)PTR_ERR(entry));
		return PTR_ERR(entry);
	}

	if (!string) {
		pr_debug("%s:%d: string is NULL\n", __FILE__, __LINE__);
		return -EINVAL;
	}

	/* copy string */
	strncpy(&entry->value[0], string, entry->length);

	return 0;
}

int tlv_entry_get_uint8(struct tlvinfo_tlv *const entry, u8 *buffer)
{
	return tlv_entry_get_raw(entry, buffer, sizeof(uint8_t));
}

int tlv_entry_set_uint8(struct tlvinfo_tlv *const entry, const u8 value)
{
	return tlv_entry_set_raw(entry, &value, sizeof(value));
}

int tlv_entry_get_uint16(struct tlvinfo_tlv *const entry, u16 *buffer)
{
	int ret;
	uint16_t val;

	ret = tlv_entry_get_raw(entry, (uint8_t *)&val, sizeof(val));
	if (ret)
		return ret;

	*buffer = be16_to_cpu(val);
	return 0;
}

int tlv_entry_set_uint16(struct tlvinfo_tlv *const entry, const u16 value)
{
	uint16_t raw = cpu_to_be16(value);

	return tlv_entry_set_raw(entry, (uint8_t *)&raw, sizeof(raw));
}

int tlv_entry_get_uint32(struct tlvinfo_tlv *const entry, u32 *buffer)
{
	int ret;
	uint32_t val;

	ret = tlv_entry_get_raw(entry, (uint8_t *)&val, sizeof(val));
	if (ret)
		return ret;

	*buffer = be32_to_cpu(val);
	return 0;
}

int tlv_entry_set_uint32(struct tlvinfo_tlv *const entry, const u32 value)
{
	uint32_t raw = cpu_to_be32(value);

	return tlv_entry_set_raw(entry, (uint8_t *)&raw, sizeof(raw));
}
