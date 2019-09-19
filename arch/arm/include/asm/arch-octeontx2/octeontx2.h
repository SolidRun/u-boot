/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#ifndef __OCTEONTX2_H__
#define __OCTEONTX2_H__

#define CN98XX	0xB1
#define CN96XX	0xB2
#define CN95XX	0xB3
#define LOKI	0xB4

#define is_board_model(model)	(g_cavm_bdt.prod_id == (model))

/** Reg offsets */
#define CAVM_RST_BOOT		0x87E006001600ULL
#define CAVM_RST_CHIP_DOM_W1S	0x87E006001810ULL

#define CAVM_CPC_BOOT_OWNERX(a)	0x86D000000160ULL + (8 * a)

#define CAVM_GTI_CWD_POKEX	0x802000050000ULL
#define CAVM_GTI_CWD_WDOGX	0x802000040000ULL

/* attestation definitions shared with ATF (see 'plat_octeontx.h') */

#define ATTESTATION_MAGIC_ID 0x5f415454 /* "_ATT" */

enum sw_attestation_tlv_type {
	ATT_IMG_INIT_BIN,
	ATT_IMG_ATF_BL1,
	ATT_IMG_BOARD_DT,
	ATT_IMG_LINUX_DT,
	ATT_IMG_SCP_TBL1FW,
	ATT_IMG_MCP_TBL1FW,
	ATT_IMG_AP_TBL1FW,
	ATT_IMG_ATF_BL2,
	ATT_IMG_ATF_BL31,
	ATT_IMG_ATF_BL33,
	ATT_SIG_NONCE,

	ATT_TLV_TYPE_COUNT,
};

typedef struct sw_attestation_tlv {
	u16 type_be;   /* sw_attestation_tlv_type */
	u16 length_be;
	u8 value[0];   /* array of 'length_be' bytes */
} sw_attestation_tlv_t;

#define SW_ATT_INFO_NONCE_MAX_LEN  256

typedef struct sw_attestation_info_hdr {
	u32 magic_be;
	u16 tlv_len_be;
	u16 total_len_be;
	u16 certificate_len_be;
	u16 signature_len_be;
	union {
		sw_attestation_tlv_t tlv_list[0];
		s8 input_nonce[0];
	};
} __packed sw_attestation_info_hdr_t;

/** Structure definitions */
/**
 * Register (NCB32b) cpc_boot_owner#
 *
 * CPC Boot Owner Registers These registers control an external arbiter
 * for the boot device (SPI/eMMC) across multiple external devices. There
 * is a register for each requester: _ \<0\> - SCP          - reset on
 * SCP reset _ \<1\> - MCP          - reset on MCP reset _ \<2\> - AP
 * Secure    - reset on core reset _ \<3\> - AP Nonsecure - reset on core
 * reset  These register is only writable to the corresponding
 * requestor(s) permitted with CPC_PERMIT.
 */
union cavm_cpc_boot_ownerx {
        u32 u;
        struct cavm_cpc_boot_ownerx_s {
                u32 boot_req                         : 1;
                u32 reserved_1_7                     : 7;
                u32 boot_wait                        : 1;
                u32 reserved_9_31                    : 23;
        } s;
        /* struct cavm_cpc_boot_ownerx_s cn; */
};


/** Function definitions */
void octeontx2_parse_board_info(void);
void octeontx2_board_get_mac_addr(u8 index, u8 *mac_addr);
void acquire_flash_arb(bool acquire);

/** Board data definitions */
struct cavm_bdt {
	u8 resv[6];
	u8 prod_id;
	u8 alt_pkg;
	char type[16];
};
extern struct cavm_bdt g_cavm_bdt;

#endif /* __OCTEONTX2_H__ */
