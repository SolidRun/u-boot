/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#ifndef __OCTEONTX2_SVC_H__
#define __OCTEONTX2_SVC_H__

/* SMC function IDs for general purpose queries */

#define OCTEONTX2_SVC_CALL_COUNT		0x4300ff00
#define OCTEONTX2_SVC_UID		0x4300ff01

#define OCTEONTX2_SVC_VERSION		0x4300ff03

/* OcteonTX Service Calls version numbers */
#define OCTEONTX2_VERSION_MAJOR	0x0
#define OCTEONTX2_VERSION_MINOR	0x2

/* x1 - node number
 */
#define OCTEONTX2_DRAM_SIZE		0x43000301
#define OCTEONTX2_NODE_COUNT		0x43000601

#endif /* __OCTEONTX2_SVC_H__ */
