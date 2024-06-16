// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * https://spdx.org/licenses
 *
 * Copyright (C) 2024 Marvell
 *
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/ptrace.h>
#include <asm/arch/smc.h>
#include <asm/psci.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

ssize_t smc_dram_size(unsigned int node)
{
	struct pt_regs regs;

	regs.regs[0] = CN20K_DRAM_SIZE;
	regs.regs[1] = node;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t smc_flsf_fw_booted(void)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_FSAFE_PR_BOOT_SUCCESS;
	smc_call(&regs);

	return regs.regs[0];
}

ssize_t smc_flsf_clr_force_2ndry(void)
{
	struct pt_regs regs;

	regs.regs[0] = OCTEONTX2_FSAFE_CLR_FORCE_SEC;
	smc_call(&regs);

	return regs.regs[0];
}

/*
 * Perform EFI Application Image load to DRAM in ATF
 *
 * x1 - Image location
 * x2 - Pointer to store image size
 *
 * Return:
 *	x0:
 *		0 -- Success
 *		-1 -- Invalid Arguments
 *		-2 -- SPI_CONFIG_ERR
 *		-3 -- SPI_MMAP_ERR
 *		-5 -- EIO
 */
int smc_load_efi_img(u64 img_addr, u64 *img_size)
{
	struct pt_regs regs;

	regs.regs[0] = PLAT_OCTEONTX_LOAD_EFI_APP;
	regs.regs[1] = img_addr;
	smc_call(&regs);

	*img_size = regs.regs[1];
	return regs.regs[0];
}

/*
 * Get RVU Reserved Memory Region Info
 *
 * Return:
 *	x0:
 *		0 -- Success
 *	x1 - region start address
 *	x2 - region size
 */
int smc_rvu_rsvd_reg_info(u64 *reg_addr, u64 *reg_size)
{
	struct pt_regs regs;

	regs.regs[0] = PLAT_OCTEONTX_RVU_RSVD_REG_INFO;
	smc_call(&regs);

	*reg_addr = regs.regs[1];
	*reg_size = regs.regs[2];
	return regs.regs[0];
}
