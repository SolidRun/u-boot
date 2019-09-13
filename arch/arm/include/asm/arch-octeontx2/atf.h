/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __ATF_H__
#define __ATF_H__

ssize_t atf_dram_size(unsigned int node);
ssize_t atf_disable_rvu_lfs(unsigned int node);
ssize_t atf_flsf_fw_booted(void);
ssize_t atf_flsf_clr_force_2ndry(void);
ssize_t atf_mdio_dbg_read(int cgx_lmac, int mode, int phyaddr, int devad,
			  int reg);
ssize_t atf_mdio_dbg_write(int cgx_lmac, int mode, int phyaddr, int devad,
			   int reg, int val);
ssize_t atf_attest(long nonce_len);
#endif
