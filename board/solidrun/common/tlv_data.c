// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 SolidRun
 */

#include <common.h>
#include <compiler.h>
#include <linux/err.h>
#include <tlv_eeprom.h>
#include "tlv_data.h"

static void store_product_name(struct tlvinfo_tlv *tlv_entry,
			       struct tlv_data *td)
{
	int len;
	char *dest;

	dest = td->tlv_product_name;
	len = min_t(unsigned int, tlv_entry->length,
		    sizeof(td->tlv_product_name) - 1);
	memcpy(dest, tlv_entry->value, len);
}

static void store_part_number(struct tlvinfo_tlv *tlv_entry,
			      struct tlv_data *td)
{
	int len;
	char *dest;

	dest = td->tlv_part_number;
	len = min_t(unsigned int, tlv_entry->length,
		    sizeof(td->tlv_part_number) - 1);
	memcpy(dest, tlv_entry->value, len);
}

static void store_mac_size(struct tlvinfo_tlv *tlv_entry,
			   struct tlv_data *td)
{
	td->tlv_mac_count = (tlv_entry->value[0] << 8) | tlv_entry->value[1];
	pr_debug("%s: read mac count = %u\n", __func__, td->tlv_mac_count);
}

static void store_mac_base(struct tlvinfo_tlv *tlv_entry,
			   struct tlv_data *td)
{
	char *dest;

	dest = td->tlv_mac_base;
	memcpy(dest, tlv_entry->value, 6);
	pr_debug("%s: read mac base = %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
}

static void parse_tlv_vendor_ext(struct tlvinfo_tlv *tlv_entry,
				 struct tlv_data *td)
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

		/* extension with additional data field for number of ddr channels */
		if (tlv_entry->length >= 7) {
			td->ram_channels = val[6];
		}
		break;
	case SR_TLV_CODE_KIT_NUMBER:
		if (tlv_entry->length > 257)
			break;
		dest = td->tlv_kit_number;
		len = min_t(unsigned int, tlv_entry->length - 5,
			    sizeof(td->tlv_kit_number) - 1);
		memcpy(dest, &tlv_entry->value[5], len);
		break;
	default:
		break;
	};

	return;
}

static void parse_tlv_data(u8 *eeprom, struct tlvinfo_priv *tlv,
			   struct tlv_data *td)
{
	struct tlvinfo_tlv *entry;

	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_PRODUCT_NAME);
	if (!IS_ERR(entry))
		store_product_name(entry, td);

	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_PART_NUMBER);
	if (!IS_ERR(entry))
		store_part_number(entry, td);

	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_MAC_SIZE);
	if (!IS_ERR(entry))
		store_mac_size(entry, td);

	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_MAC_BASE);
	if (!IS_ERR(entry))
		store_mac_base(entry, td);

	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_VENDOR_EXT);
	if (!IS_ERR(entry))
		parse_tlv_vendor_ext(entry, td);
}

int read_tlv_data(int dev_num, struct tlv_data *td)
{
	u8 eeprom_data[TLV_TOTAL_LEN_MAX];
	struct tlvinfo_priv *priv;

	priv = tlv_eeprom_read(tlv_eeprom_get_by_index(dev_num), 0, eeprom_data, ARRAY_SIZE(eeprom_data));
	if (IS_ERR(priv))
		return PTR_ERR(priv);

	parse_tlv_data(eeprom_data, priv, td);

	return 0;
}

bool sr_product_is(const struct tlv_data *td, const char *product)
{
	/* Allow prefix sub-string match */
	if (strncmp(td->tlv_product_name, product, strlen(product)) == 0)
		return true;

	return false;
}
