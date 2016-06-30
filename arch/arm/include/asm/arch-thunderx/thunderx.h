/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/
#ifndef __THUNDERX_H__
#define __THUNDERX_H__

#define CSR_PA(node, csr) ((csr) | ((uint64_t)(node) << 44))

#define RST_BOOT 0x87E006001600ULL

#endif
