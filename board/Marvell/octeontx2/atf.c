// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <asm/io.h>

#include <asm/system.h>
#include <asm/arch/octeontx2_svc.h>
#include <asm/arch/atf.h>

#include <asm/psci.h>

#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

ssize_t atf_dram_size(unsigned int node)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_DRAM_SIZE;
	regs.regs[1] = node;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_disable_rvu_lfs(unsigned int node)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_DISABLE_RVU_LFS;
	regs.regs[1] = node;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_configure_ooo(unsigned int val)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_CONFIG_OOO;
	regs.regs[1] = val;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_flsf_fw_booted(void)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_FSAFE_PR_BOOT_SUCCESS;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_flsf_clr_force_2ndry(void)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_FSAFE_CLR_FORCE_SEC;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_mdio_dbg_read(int cgx_lmac, int mode, int phyaddr, int devad,
			  int reg)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_MDIO_DBG_READ;
	regs.regs[1] = cgx_lmac;
	regs.regs[2] = mode;
	regs.regs[3] = phyaddr;
	regs.regs[4] = devad;
	regs.regs[5] = reg;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_mdio_dbg_write(int cgx_lmac, int mode, int phyaddr, int devad,
			   int reg, int val)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_MDIO_DBG_WRITE;
	regs.regs[1] = cgx_lmac;
	regs.regs[2] = mode;
	regs.regs[3] = phyaddr;
	regs.regs[4] = devad;
	regs.regs[5] = reg;
	regs.regs[6] = val;
	smc_call(&regs);

	return regs.regs[0];
}

/*
 * on entry,
 *   nonce_len: <= 0, query for buffer address
 *               > 0 specifies nonce length
 *
 * returns,
 *   signed value: <0 - error code
 *                  0 - success
 */
ssize_t atf_attest(long nonce_len)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX_ATTESTATION_QUERY;
	/* X1 - nonce len */
	regs.regs[1] = nonce_len;
	/* X2 - subfunction (useful for adding future cmd extensions) */
	regs.regs[2] = 0;

	smc_call(&regs);

	return regs.regs[0];
}
