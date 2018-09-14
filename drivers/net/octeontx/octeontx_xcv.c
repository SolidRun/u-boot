/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#include <config.h>
#include <common.h>
#include <errno.h>
#include <net.h>
#include <dm.h>
#include <pci.h>
#include <misc.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <asm/io.h>

#ifdef CONFIG_OF_LIBFDT
 #include <linux/libfdt.h>
 #include <fdt_support.h>
#endif

#include <asm/arch/octeontx_xcv.h>
#include <asm/arch/octeontx_smi.h>
#include <asm/arch/octeontx_vnic.h>

struct lxcv {
	void __iomem		*reg_base;
	struct pci_dev		*pdev;
};

struct lxcv *xcv;

/* Initialize XCV block */
void xcv_init_hw(void)
{
	//union cavm_xcvx_ctl xcv_ctl;
	union cavm_xcvx_reset reset;
	union cavm_xcvx_dll_ctl xcv_dll_ctl;
	//union cavm_xcvx_comp_ctl xcv_comp_ctl;

	/* Take the DLL out of reset */
	reset.u = readq(CAVM_XCVX_RESET);
	reset.s.dllrst = 0;
	writeq(reset.u, CAVM_XCVX_RESET);

	/* Take the clock tree out of reset */
	reset.u = readq(CAVM_XCVX_RESET);
	reset.s.clkrst = 0;
	writeq(reset.u, CAVM_XCVX_RESET);

	/* Once the 125MHz ref clock is stable, wait 10us for DLL to lock *.
 */
	udelay(10);

	/* Optionally, bypass the DLL setting */
	xcv_dll_ctl.u = readq(CAVM_XCVX_DLL_CTL);
	xcv_dll_ctl.s.clkrx_set = 0;
	xcv_dll_ctl.s.clkrx_byp = 1;
	xcv_dll_ctl.s.clktx_byp = 0;
	writeq(xcv_dll_ctl.u, CAVM_XCVX_DLL_CTL);

	/* Enable the compensation controller */
	reset.u = readq(CAVM_XCVX_RESET);
	reset.s.comp = 1;
	writeq(reset.u, CAVM_XCVX_RESET);
	reset.u = readq(CAVM_XCVX_RESET);

	/* Wait for 1040 reference clock cycles for the compensation state 
	   machine lock. */
	udelay(100); 

	/* Enable the XCV block */
	reset.u = readq(CAVM_XCVX_RESET);
	reset.s.enable = 1;
	writeq(reset.u, CAVM_XCVX_RESET);

	/* set XCV(0)_RESET[CLKRST] to 1 */
	reset.u = readq(CAVM_XCVX_RESET);
	reset.s.clkrst = 1;
	writeq(reset.u, CAVM_XCVX_RESET);
}

/* 
 * Configure XCV link based on the speed
 * link_up   : Set to 1 when link is up otherwise 0
 * link_speed: The speed of the link.
 */ 
void xcv_setup_link(bool link_up, int link_speed)
{
	union cavm_xcvx_ctl xcv_ctl;
	union cavm_xcvx_reset reset;
	//union cavm_xcvx_comp_ctl xcv_comp_ctl;
	union cavm_xcvx_batch_crd_ret xcv_crd_ret;
	int speed = 2;

	/* Check RGMII link */
	if (link_speed == 100)
		speed = 1;
	else if (link_speed == 10)
		speed = 0;

	if (link_up) {
		/* Set operating speed */
		xcv_ctl.u = readq(CAVM_XCVX_CTL);
		xcv_ctl.s.speed = speed;
		writeq(xcv_ctl.u, CAVM_XCVX_CTL);

		/* Datapaths come out of reset
		   - The datapath resets will disengage BGX from the
		     RGMII interface
		   - XCV will continue to return TX credits for each tick
		     that is sent on the TX data path */
		reset.u = readq(CAVM_XCVX_RESET);
		reset.s.tx_dat_rst_n = 1;
		reset.s.rx_dat_rst_n = 1;
		writeq(reset.u, CAVM_XCVX_RESET);

		/* Enable packet flow */
		reset.u = readq(CAVM_XCVX_RESET);
		reset.s.tx_pkt_rst_n = 1;
		reset.s.rx_pkt_rst_n = 1;
		writeq(reset.u, CAVM_XCVX_RESET);

		xcv_crd_ret.u = readq(CAVM_XCVX_BATCH_CRD_RET);
		xcv_crd_ret.s.crd_ret = 1;
		writeq(xcv_crd_ret.u, CAVM_XCVX_BATCH_CRD_RET);
	} else {
		/* Enable packet flow */
		reset.u = readq(CAVM_XCVX_RESET);
		reset.s.tx_pkt_rst_n = 0;
		reset.s.rx_pkt_rst_n = 0;
		writeq(reset.u, CAVM_XCVX_RESET);
		reset.u = readq(CAVM_XCVX_RESET);
	}
}

int octeontx_xcv_probe(struct udevice *dev)
{
	size_t size;

	xcv = dev_get_priv(dev);
	if (xcv == NULL) {
		return -ENOMEM;
	}

	xcv->reg_base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);

	return 0;
}

static const struct misc_ops octeontx_xcv_ops = {
};

static const struct udevice_id octeontx_xcv_ids[] = {
	{ .compatible = "cavium,xcv" },
	{}
};

U_BOOT_DRIVER(octeontx_xcv) = {
        .name   = "octeontx_xcv",
        .id     = UCLASS_MISC,
        .probe  = octeontx_xcv_probe,
        .of_match = octeontx_xcv_ids,
        .ops    = &octeontx_xcv_ops,
        .priv_auto_alloc_size = sizeof(struct lxcv),
};

