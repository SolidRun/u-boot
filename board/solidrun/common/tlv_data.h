/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 SolidRun
 */

#ifndef __BOARD_SR_COMMON_H_
#define __BOARD_SR_COMMON_H_

#define TLV_MAX_DEVICES			2

struct tlv_data {
	/* Store product name of both SOM and carrier */
	char tlv_product_name[TLV_MAX_DEVICES][32];
	char tlv_part_number[TLV_MAX_DEVICES][257];
	char tlv_kit_number[TLV_MAX_DEVICES][257];
	unsigned char tlv_mac_base[TLV_MAX_DEVICES][6];
	u16 tlv_mac_count[TLV_MAX_DEVICES];
	unsigned int ram_size;
	uint8_t ram_channels;
};

/*
 * SolidRun TLV vendor extension data format
 * (compatible with format used on Armada 388 Clearfog by Baruch Siach)
 */
struct __attribute__((__packed__)) sr_tlv_ext {
	u32 pen; // IANA Private Enterprise Number
	u8 code; // identification code for data
	u8 data[];
};

// SolidRun TLV vendor extension data codes
enum sr_tlv_code {
	SR_TLV_CODE_KIT_NUMBER  = 0x10,
	SR_TLV_CODE_RAM_SIZE    = 0x81,
};

void read_tlv_data(struct tlv_data *td);
bool sr_product_is(const struct tlv_data *td, const char *product);

#endif /* __BOARD_SR_COMMON_H_ */
