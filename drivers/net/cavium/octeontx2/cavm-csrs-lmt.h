#ifndef __CAVM_CSRS_LMT_H__
#define __CAVM_CSRS_LMT_H__
/* This file is auto-generated. Do not edit */

/***********************license start***************
 * Copyright (c) 2003-2018  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM  NETWORKS MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * Cavium LMT.
 *
 * This file is auto generated. Do not edit.
 *
 */

/**
 * Register (RVU_PFVF_BAR2) lmt_lf_lmtcancel
 *
 * RVU VF LMT Cancel Register
 */
union cavm_lmt_lf_lmtcancel {
	u64 u;
	struct cavm_lmt_lf_lmtcancel_s {
		u64 data                             : 64;
	} s;
	/* struct cavm_lmt_lf_lmtcancel_s cn; */
};

static inline u64 CAVM_LMT_LF_LMTCANCEL
	__attribute__ ((pure, always_inline));
static inline u64 CAVM_LMT_LF_LMTCANCEL
{
	if (CAVIUM_IS_MODEL(CAVIUM_CN9XXX))
		return 0x400;
	return -1;
}

/**
 * Register (RVU_PFVF_BAR2) lmt_lf_lmtline#
 *
 * RVU VF LMT Line Registers
 */
union cavm_lmt_lf_lmtlinex {
	u64 u;
	struct cavm_lmt_lf_lmtlinex_s {
		u64 data                             : 64;
	} s;
	/* struct cavm_lmt_lf_lmtlinex_s cn; */
};

static inline u64 CAVM_LMT_LF_LMTLINEX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CAVM_LMT_LF_LMTLINEX(u64 a)
{
	if (CAVIUM_IS_MODEL(CAVIUM_CN9XXX) && (a <= 15))
		return 0 + 8 * ((a) & 0xf);
	return -1;
}

#endif /* __CAVM_CSRS_LMT_H__ */
