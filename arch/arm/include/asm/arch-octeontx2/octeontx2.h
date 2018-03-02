/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/
#ifndef __OCTEONTX2_H__
#define __OCTEONTX2_H__

#define CN81XX	0xA2
#define CN93XX	0xB2
#define CAVIUM_IS_MODEL(model)	(p_cavm_bdt->prod_id == model)

/** Reg offsets */
#define CAVM_RST_BOOT		0x87E006001600ULL
#define CAVM_RST_SOFT_RST	0x87E006001680ULL
#define CAVM_MIO_FUS_DAT2	0x87E003001410ULL

#define CAVM_GTI_CWD_POKEX	0x802000050000ULL
#define CAVM_GTI_CWD_WDOGX	0x802000040000ULL

/** Structure definitions */




/** Function definitions */
void octeontx2_parse_board_info(void);

/** Board data definitions */
struct cavm_bdt {
	u8 resv[6];
	u8 prod_id;
	u8 alt_pkg;
	char type[16];
};
extern struct cavm_bdt *p_cavm_bdt;

#endif /* __OCTEONTX2_H__ */
