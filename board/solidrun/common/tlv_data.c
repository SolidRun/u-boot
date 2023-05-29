// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 SolidRun
 */

#include <common.h>
#include <compiler.h>
#include <tlv_eeprom.h>
#include "tlv_data.h"

static void store_product_name(struct tlvinfo_tlv *tlv_entry,
			       struct tlv_data *td, int index)
{
	int len;
	char *dest;

	dest = td->tlv_product_name[index];
	len = min_t(unsigned int, tlv_entry->length,
		    sizeof(td->tlv_product_name[index]) - 1);
	memcpy(dest, tlv_entry->value, len);
}

static void store_part_number(struct tlvinfo_tlv *tlv_entry,
			      struct tlv_data *td, int index)
{
	int len;
	char *dest;

	dest = td->tlv_part_number[index];
	len = min_t(unsigned int, tlv_entry->length,
		    sizeof(td->tlv_part_number[index]) - 1);
	memcpy(dest, tlv_entry->value, len);
}

static void store_mac_size(struct tlvinfo_tlv *tlv_entry,
			   struct tlv_data *td, int index)
{
	td->tlv_mac_count[index] = (tlv_entry->value[0] << 8) | tlv_entry->value[1];
	pr_debug("%s: read mac count = %u\n", __func__, td->tlv_mac_count[index]);
}

static void store_mac_base(struct tlvinfo_tlv *tlv_entry,
			   struct tlv_data *td, int index)
{
	char *dest;

	dest = td->tlv_mac_base[index];
	memcpy(dest, tlv_entry->value, 6);
	pr_debug("%s: read mac base = %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
}

static void parse_tlv_vendor_ext(struct tlvinfo_tlv *tlv_entry,
				 struct tlv_data *td, int index)
{
	u8 *val = tlv_entry->value;
	u32 pen; /* IANA Private Enterprise Numbers */
	int len;
	char *dest;

	if (tlv_entry->length < 5) /* 4 bytes PEN + at least 1 byte type */
		return;

	/* PEN is big endian */
	pen = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
	/* Not a real PEN */
	if (pen != 0xffffffff)
		return;

	switch (val[4]) {
	case SR_TLV_CODE_RAM_SIZE:
		if (tlv_entry->length != 6)
			break;
		td->ram_size = val[5];
		break;
	case SR_TLV_CODE_KIT_NUMBER:
		if (tlv_entry->length > 257)
			break;
		dest = td->tlv_kit_number[index];
	        len = min_t(unsigned int, tlv_entry->length,
			    sizeof(td->tlv_kit_number[index]) - 1);
		memcpy(dest, tlv_entry->value, len);
		break;	
	default:
		break;
	};
		
	return;
}

static void parse_tlv_data(u8 *eeprom, struct tlvinfo_header *hdr,
			   struct tlvinfo_tlv *entry, struct tlv_data *td, int index)
{
	unsigned int tlv_offset, tlv_len;

	tlv_offset = sizeof(struct tlvinfo_header);
	tlv_len = sizeof(struct tlvinfo_header) + be16_to_cpu(hdr->totallen);
	while (tlv_offset < tlv_len) {
		entry = (struct tlvinfo_tlv *)&eeprom[tlv_offset];

		switch (entry->type) {
		case TLV_CODE_PRODUCT_NAME:
			store_product_name(entry, td, index);
			break;
		case TLV_CODE_PART_NUMBER:
			store_part_number(entry, td, index);
			break;
		case TLV_CODE_MAC_SIZE:
			store_mac_size(entry, td, index);
			break;
		case TLV_CODE_MAC_BASE:
			store_mac_base(entry, td, index);
			break;
		case TLV_CODE_VENDOR_EXT:
			parse_tlv_vendor_ext(entry, td, index);
			break;
		default:
			break;
		}

		tlv_offset += sizeof(struct tlvinfo_tlv) + entry->length;
	}
}

void read_tlv_data(struct tlv_data *td)
{
	u8 eeprom_data[TLV_TOTAL_LEN_MAX];
	struct tlvinfo_header *tlv_hdr;
	struct tlvinfo_tlv *tlv_entry;
	int ret, i;

	for (i = 0; i < TLV_MAX_DEVICES; i++) {
		ret = read_tlvinfo_tlv_eeprom(eeprom_data, &tlv_hdr,
					      &tlv_entry, i);
		if (ret < 0)
			continue;
		parse_tlv_data(eeprom_data, tlv_hdr, tlv_entry, td, i);
	}
}

bool sr_product_is(const struct tlv_data *td, const char *product)
{
	/* Allow prefix sub-string match */
	if (strncmp(td->tlv_product_name[0], product, strlen(product)) == 0)
		return true;
	if (strncmp(td->tlv_product_name[1], product, strlen(product)) == 0)
		return true;

	return false;
}
