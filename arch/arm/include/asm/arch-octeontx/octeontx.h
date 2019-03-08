/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX_H__
#define __OCTEONTX_H__

#define CN81XX	0xA2
#define CN83XX	0xA3
#define is_board_model(model)  (g_cavm_bdt.prod_id == (model))

#define MAX_LMAC_PER_BGX 4

/** Reg offsets */
#define CAVM_MIO_FUS_DAT2	0x87E003001410ULL

#define CAVM_GTI_CWD_POKEX	0x844000050000ULL
#define CAVM_GTI_CWD_WDOGX	0x844000040000ULL

/** Structure definitions */

/** Function definitions */
void octeontx_parse_board_info(void);
int octeontx_board_has_pmp(void);
void octeontx_parse_phy_info(void);
void octeontx_board_get_ethaddr(int bgx, int lmac, unsigned char *eth);

/** Board data definitions */
struct cavm_bdt {
	u8 resv[6];
	u8 prod_id;
	u8 alt_pkg;
	char type[16];
};
extern struct cavm_bdt g_cavm_bdt;

#endif
