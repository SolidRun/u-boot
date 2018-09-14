#ifndef __CAVM_CSRS_LMT_H__
#define __CAVM_CSRS_LMT_H__
/* This file is auto-generated.  Do not edit */
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * Cavium LMT.
 *
 * This file is auto generated.  Do not edit.
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

static inline u64 CAVM_LMT_LF_LMTCANCEL(void)
	__attribute__ ((pure, always_inline));
static inline u64 CAVM_LMT_LF_LMTCANCEL(void)
{
	return 0x400;
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
	return 0 + 8 * a;
}

#endif /* __CAVM_CSRS_LMT_H__ */
