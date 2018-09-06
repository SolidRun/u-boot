/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX_SVC_H__
#define __OCTEONTX_SVC_H__

/* SMC function IDs for general purpose queries */

#define OCTEONTX_SVC_CALL_COUNT		0xc200ff00
#define OCTEONTX_SVC_UID		0xc200ff01

#define OCTEONTX_SVC_VERSION		0xc200ff03

/* OcteonTX Service Calls version numbers */
#define OCTEONTX_VERSION_MAJOR	0x1
#define OCTEONTX_VERSION_MINOR	0x0

/* x1 - node number
 */
#define OCTEONTX_DRAM_SIZE		0xc2000301
#define OCTEONTX_NODE_COUNT		0xc2000601

#endif /* __OCTEONTX_SVC_H__ */
