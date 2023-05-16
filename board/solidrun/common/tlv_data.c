// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 SolidRun
 */

#include <common.h>
#include <compiler.h>
#include <linux/err.h>
#include <tlv_eeprom.h>
#include "tlv_data.h"

#define SR_TLV_CODE_RAM_SIZE	0x81

static void store_product_name(struct tlvinfo_tlv *tlv_entry,
			       struct tlv_data *td)
{
	int len;
	char *dest;

	if (strlen(td->tlv_product_name[0]) == 0)
		dest = td->tlv_product_name[0];
	else if (strlen(td->tlv_product_name[1]) == 0)
		dest = td->tlv_product_name[1];
	else
		return;

	len = min_t(unsigned int, tlv_entry->length,
		    sizeof(td->tlv_product_name[0]) - 1);
	memcpy(dest, tlv_entry->value, len);
}

static void parse_tlv_vendor_ext(struct tlvinfo_tlv *tlv_entry,
				 struct tlv_data *td)
{
	u8 *val = tlv_entry->value;
	u32 pen; /* IANA Private Enterprise Numbers */

	if (tlv_entry->length < 5) /* 4 bytes PEN + at least 1 byte type */
		return;

	/* PEN is big endian */
	pen = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
	/* Not a real PEN */
	if (pen != 0xffffffff)
		return;

	if (val[4] != SR_TLV_CODE_RAM_SIZE)
		return;
	if (tlv_entry->length < 6)
		return;
	td->ram_size = val[5];

	/* extension with additional data field for number of ddr channels */
	if (tlv_entry->length >= 7) {
		td->ram_channels = val[6];
	}
}

static void parse_tlv_data(u8 *eeprom, struct tlvinfo_priv *tlv,
						   struct tlv_data *td)
{
	struct tlvinfo_tlv *entry;

	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_PRODUCT_NAME);
	if (!IS_ERR(entry))
		store_product_name(entry, td);

	entry = tlv_entry_next_by_code(tlv, NULL, TLV_CODE_VENDOR_EXT);
	if (!IS_ERR(entry))
		parse_tlv_vendor_ext(entry, td);
}

void read_tlv_data(struct tlv_data *td)
{
	u8 eeprom_data[TLV_TOTAL_LEN_MAX];
	struct tlvinfo_priv *priv;
	int i;

	for (i = 0; i < 2; i++) {
		priv = tlv_eeprom_read(tlv_eeprom_get_by_index(i), 0, eeprom_data, ARRAY_SIZE(eeprom_data));
		if (IS_ERR(priv))
			continue;
		parse_tlv_data(eeprom_data, priv, td);
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
