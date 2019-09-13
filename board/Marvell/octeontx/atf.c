// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <asm/io.h>

#include <asm/system.h>
#include <asm/arch/octeontx_svc.h>
#include <asm/arch/atf.h>

#include <asm/psci.h>

#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

ssize_t atf_dram_size(unsigned int node)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX_DRAM_SIZE;
	regs.regs[1] = node;
	smc_call(&regs);

	return regs.regs[0];
}

