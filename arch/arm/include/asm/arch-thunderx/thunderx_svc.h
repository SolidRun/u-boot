/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#ifndef __THUNDERX_SVC_H__
#define __THUNDERX_SVC_H__

/* SMC function IDs for general purpose queries */

#define THUNDERX_SVC_CALL_COUNT		0x4300ff00
#define THUNDERX_SVC_UID		0x4300ff01

#define THUNDERX_SVC_VERSION		0x4300ff03

/* ThunderX Service Calls version numbers */
#define THUNDERX_VERSION_MAJOR	0x0
#define THUNDERX_VERSION_MINOR	0x2

/* x1 - node number
 */
#define THUNDERX_DRAM_SIZE		0x43000301
#define THUNDERX_NODE_COUNT		0x43000601

#endif /* __THUNDERX_SVC_H__ */
