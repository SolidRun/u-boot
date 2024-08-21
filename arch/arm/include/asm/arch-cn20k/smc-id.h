/* SPDX-License-Identifier: BSD-2-Clause-Patent
 * https://spdx.org/licenses
 *
 * Copyright (C) 2024 Marvell
 *
 */

#ifndef __SMC_ID_H__
#define __SMC_ID_H__

/* SMC function IDs for general purpose queries */

#define CN20K_SVC_CALL_COUNT	0xc200ff00
#define CN20K_SVC_UID		0xc200ff01

#define CN20K_SVC_VERSION		0xc200ff03

/* OcteonTX Service Calls version numbers */
#define CN20K_VERSION_MAJOR	0x1
#define CN20K_VERSION_MINOR	0x0

/* x1 - node number */
#define CN20K_DRAM_SIZE		0xc2000301

#define OCTEONTX2_DISABLE_RVU_LFS		0xc2000b01

/* fail safe */
#define OCTEONTX2_FSAFE_PR_BOOT_SUCCESS		0xc2000b02
#define OCTEONTX2_FSAFE_CLR_FORCE_SEC		0xc2000b03
#define PLAT_OCTEONTX_LOAD_SWITCH_FW		0xc2000b06
#define PLAT_OCTEONTX_RVU_RSVD_REG_INFO		0xc2000b07
#define PLAT_OCTEONTX_LOAD_EFI_APP		0xc2000b08

#endif /* __SMC_ID_H__ */
