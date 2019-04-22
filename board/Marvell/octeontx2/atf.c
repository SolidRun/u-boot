/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
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

ssize_t atf_node_count(void)
{
	struct pt_regs regs;
	regs.regs[0] = OCTEONTX2_NODE_COUNT;

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
