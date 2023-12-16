// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * https://spdx.org/licenses
 *
 * Copyright (C) 2024 Marvell
 *
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/global_data.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

u64 get_page_table_size(void)
{
	return 0x80000;
}

void reset_cpu(void)
{
	psci_system_reset();
}
