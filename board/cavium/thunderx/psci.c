/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/

#define DEBUG
#include <common.h>
#include <asm/io.h>
#include <linux/psci.h>

#include <asm/system.h>
#include <asm/bootm.h>
#include <asm/armv8/mmu.h>

#include <cavium/slt.h>

#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#include <fdt_support.h>

#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long psci_call(int fn, unsigned long arg0, unsigned long arg1,
			unsigned long arg2, unsigned long arg3)
{
	int nodeoffset;
	int len;
	const char *method;

	struct pt_regs regs;

	regs.regs[0] = fn;
	regs.regs[1] = arg0;
	regs.regs[2] = arg1;
	regs.regs[3] = arg2;
	regs.regs[4] = arg3;

	if (!working_fdt) {
		printf("WARNING: No FDT is present.\n");
		return 0;
	}

	nodeoffset = fdt_path_offset(working_fdt, "/psci");

	if (nodeoffset < 0) {
		printf("WARNING: could not find %s: %s.\n", "/psci",
		       fdt_strerror(nodeoffset));
		return 0;
	}

	method = fdt_getprop(working_fdt, nodeoffset, "method", &len);

	if (method == NULL) {
		printf("WARNING: could not get '%s' value: %s.\n", "method",
		       fdt_strerror(len));
		return 0;
	}

	if (!strncmp("smc", method, 3)) {
		smc_call(&regs);
		return regs.regs[0];
	} else if (!strncmp("hvc", method, 3)) {
		hvc_call(&regs);
		return regs.regs[0];
	} else {
		printf("WARNING: Unknown PSCI call method: %s.\n", method);
		return 0;
	}
}

#define STACK_MEMALIGN 16

uintptr_t task_prepare(void *entry, struct pt_regs *args)
{
	uintptr_t sp = (uintptr_t)malloc(CONFIG_AP_STACK_SIZE);
	uintptr_t regs;

	if (!sp)
		return -1;

	debug("Stack top is at: %lx\n", sp);

	sp += CONFIG_AP_STACK_SIZE;
	sp -= CONFIG_AP_STACK_ALIGN;
	sp &= ~(CONFIG_AP_STACK_ALIGN - 1);

	regs = sp - sizeof(args->regs);
	regs -= CONFIG_AP_STACK_ALIGN;
	regs &= ~(CONFIG_AP_STACK_ALIGN - 1);

	debug("Stack bottom is at: %lx\n", sp);

	debug("Entry point is at: %p\n", entry);

	args->regs[18] = (uintptr_t)gd;
	args->regs[29] = (uintptr_t)entry;

	debug("Context: %lx\n", regs);

	memcpy((void *)regs, &args->regs, 30 * sizeof(uintptr_t));

	flush_dcache_range(regs, regs + sizeof(args->regs));

	return regs;
}


#ifdef CONFIG_SLT

void slt_task(void)
{
	unsigned long ret;
	mdelay(100);

#ifdef DEBUG
	puts("Preparing SLT\n");
#endif

	debug("Preparing SLT on core: %lx\n", read_mpidr());
	dcache_enable();
	icache_enable();

	debug("Enabled caches on core: %lx\n", read_mpidr());

	while (1) {
		ret = slt_main();
		debug("Core %lx: status %lx\n", read_mpidr(), ret);
	}
}

int do_sltrun(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	long coreid, version, cinfo, ret;
	const char *state;
	struct pt_regs slt_args;
	uintptr_t sp;

	version = psci_call(PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0, 0);

	if (version != 2) {
		printf("WARNING: Unsupported PSCI version: %ld.%ld.\n",
		       version >> 16, version & 0xffff);
		return CMD_RET_FAILURE;
	}

	debug("SLT entry point is : %p.\n", slt_task);

	switch (argc) {
	case 2:
		coreid = simple_strtol(argv[1], NULL, 16);
		cinfo = psci_call(PSCI_0_2_FN64_AFFINITY_INFO, coreid,
				  0, 0, 0);

		debug("Core %lx state: %ld.\n", coreid, cinfo);

		switch (cinfo) {
		case PSCI_0_2_AFFINITY_LEVEL_ON:
			state = "ON";
			break;
		case PSCI_0_2_AFFINITY_LEVEL_OFF:
			state = "OFF";
			break;
		case PSCI_0_2_AFFINITY_LEVEL_ON_PENDING:
			state = "ON_PENDING";
			break;
		default:
			state = "ERROR";
		}

		if (cinfo < 0) {
			printf("Error while getting the core %lx state: %ld\n",
			       coreid, cinfo);
		} else if (cinfo != PSCI_0_2_AFFINITY_LEVEL_OFF) {
			printf("Core %lx state: %s\n", coreid, state);
		}

		if (cinfo == PSCI_0_2_AFFINITY_LEVEL_OFF) {
			slt_args.regs[0] = 0x1111111111111111;
			slt_args.regs[1] = 0x2222222222222222;
			slt_args.regs[2] = 0x3333333333333333;
			slt_args.regs[3] = 0x4444444444444444;
			slt_args.regs[4] = 0x5555555555555555;
			slt_args.regs[5] = 0x6666666666666666;
			slt_args.regs[6] = 0x7777777777777777;
			slt_args.regs[7] = 0x8888888888888888;

			sp = task_prepare(slt_task, &slt_args);

			ret = psci_call(PSCI_0_2_FN64_CPU_ON, coreid,
					(unsigned long)ap_run, sp, 0);
			debug("Return status: %ld\n", ret);
		}

		break;
	default:
		return CMD_RET_USAGE;
	}
	return 0;
}

U_BOOT_CMD(
	sltrun,   2,   1,     do_sltrun,
	"run SLT routine on a specific CPU core",
	"id\n"
	"    - core id"
);
#endif

void bootm_task(void *entry, void *fdt_addr)
{
	void (*kernel_entry)(void *fdt_addr, void *res0, void *res1,
			void *res2) = entry;

	mdelay(100);

	debug("Running on core: %lx\n", read_mpidr());
	debug("Entry: %p, fdt: %p\n", entry, fdt_addr);

	kernel_entry(fdt_addr, NULL, NULL, NULL);
}

/* Subcommand: GO */
void boot_jump_linux_ap(bootm_headers_t *images, int flag,
			unsigned long bspcore)
{
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);
	unsigned long ret;
	uintptr_t sp;
	struct pt_regs boot_args;

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	announce_and_cleanup(fake);

	if (!fake) {
		boot_args.regs[0] = images->ep;
		boot_args.regs[1] = (uintptr_t)images->ft_addr;

		sp = task_prepare(bootm_task, &boot_args);

		ret = psci_call(PSCI_0_2_FN64_CPU_ON, bspcore,
				(unsigned long)ap_run, sp, 0);

		if (ret != PSCI_RET_SUCCESS) {
			printf("PSCI return status: %ld\n", ret);
		} else {
			psci_call(PSCI_0_2_FN_CPU_OFF, 0, 0, 0, 0);
			printf("ERROR shutting down the core 0\n");
		}
	}
}

#endif

