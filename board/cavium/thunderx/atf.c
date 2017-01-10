// SPDX-License-Identifier: GPL-2.0+
/**
 * (C) Copyright 2014, Cavium Inc.
**/

#include <common.h>
#include <asm/io.h>

#include <asm/system.h>
#include <asm/arch/thunderx_svc.h>
#include <asm/arch/atf.h>
#include <asm/arch/atf_part.h>

#include <asm/psci.h>

#include <malloc.h>

ssize_t atf_read_mmc(uintptr_t offset, void *buffer, size_t size)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_MMC_READ;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_read_nor(uintptr_t offset, void *buffer, size_t size)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_NOR_READ;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_get_pcount(void)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_PART_COUNT;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_get_part(struct storage_partition *part, unsigned int index)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_GET_PART;
	regs.regs[1] = (uintptr_t)part;
	regs.regs[2] = index;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_erase_nor(uintptr_t offset, size_t size)
{
	struct pt_regs regs;

	regs.regs[0] = THUNDERX_NOR_ERASE;
	regs.regs[1] = offset;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_write_nor(uintptr_t offset, const void *buffer, size_t size)
{
	struct pt_regs regs;

	regs.regs[0] = THUNDERX_NOR_WRITE;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_write_mmc(uintptr_t offset, const void *buffer, size_t size)
{
	struct pt_regs regs;

	regs.regs[0] = THUNDERX_MMC_WRITE;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_dram_size(unsigned int node)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_DRAM_SIZE;
	regs.regs[1] = node;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_node_count(void)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_NODE_COUNT;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_env_count(void)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_ENV_COUNT;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_env_string(size_t index, char *str)
{
	uint64_t *buf = (void *)str;
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_ENV_STRING;
	regs.regs[1] = index;

	smc_call(&regs);

	if (regs.regs > 0) {
		buf[0] = regs.regs[0];
		buf[1] = regs.regs[1];
		buf[2] = regs.regs[2];
		buf[3] = regs.regs[3];

		return 1;
	} else {
		return regs.regs[0];
	}
}

ssize_t atf_get_bdk_fdt(void *buffer, size_t buffer_size)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_FDT_GET;
	regs.regs[1] = (uintptr_t)buffer;
	regs.regs[2] = buffer_size;
	smc_call(&regs);
	return regs.regs[0];
}

