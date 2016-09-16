/*
 * Copyright (C) 2016 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <dm.h>
#include <pci.h>
#include <misc.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/errno.h>

#ifdef CONFIG_OF_LIBFDT
 #include <libfdt.h>
 #include <fdt_support.h>
#endif

#include <asm/arch/thunderx_smi.h>
#include <asm/arch/thunderx_vnic.h>

#include "cavm-arch.h"
#define	PCI_DEVICE_ID_THUNDER_XCV	0xA056

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
	reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
	reset.s.dllrst = 0;
	writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));

	/* Take the clock tree out of reset */
	reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
	reset.s.clkrst = 0;
	writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));

	/* Once the 125MHz ref clock is stable, wait 10us for DLL to lock *.
 */
	udelay(10);

	/* Optionally, bypass the DLL setting */
	xcv_dll_ctl.u = readq(CSR_PA(0, CAVM_XCVX_DLL_CTL(0)));
	xcv_dll_ctl.s.clkrx_set = 0;
	xcv_dll_ctl.s.clkrx_byp = 1;
	xcv_dll_ctl.s.clktx_byp = 0;
	writeq(xcv_dll_ctl.u, CSR_PA(0, CAVM_XCVX_DLL_CTL(0)));

	/* Enable the compensation controller */
	reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
	reset.s.comp = 1;
	writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));
	reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));

	/* Wait for 1040 reference clock cycles for the compensation state 
	   machine lock. */
	udelay(100); 

	/* Enable the XCV block */
	reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
	reset.s.enable = 1;
	writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));

	/* set XCV(0)_RESET[CLKRST] to 1 */
	reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
	reset.s.clkrst = 1;
	writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));
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
		xcv_ctl.u = readq(CSR_PA(0, CAVM_XCVX_CTL(0)));
		xcv_ctl.s.speed = speed;
		writeq(xcv_ctl.u, CSR_PA(0, CAVM_XCVX_CTL(0)));

		/* Datapaths come out of reset
		   - The datapath resets will disengage BGX from the
		     RGMII interface
		   - XCV will continue to return TX credits for each tick
		     that is sent on the TX data path */
		reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
		reset.s.tx_dat_rst_n = 1;
		reset.s.rx_dat_rst_n = 1;
		writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));

		/* Enable packet flow */
		reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
		reset.s.tx_pkt_rst_n = 1;
		reset.s.rx_pkt_rst_n = 1;
		writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));

		xcv_crd_ret.u = readq(CSR_PA(0, CAVM_XCVX_BATCH_CRD_RET(0)));
		xcv_crd_ret.s.crd_ret = 1;
		writeq(xcv_crd_ret.u, CSR_PA(0, CAVM_XCVX_BATCH_CRD_RET(0)));
	} else {
		/* Enable packet flow */
		reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
		reset.s.tx_pkt_rst_n = 0;
		reset.s.rx_pkt_rst_n = 0;
		writeq(reset.u, CSR_PA(0, CAVM_XCVX_RESET(0)));
		reset.u = readq(CSR_PA(0, CAVM_XCVX_RESET(0)));
	}
}

int thunderx_xcv_probe(struct udevice *dev)
{
	size_t size;

	xcv = dev_get_priv(dev);
	if (xcv == NULL) {
		return -ENOMEM;
	}

	xcv->reg_base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);

	return 0;
}

static const struct misc_ops thunderx_xcv_ops = {
};

static const struct udevice_id thunderx_xcv_ids[] = {
	{ .compatible = "cavium,xcv" },
	{}
};

U_BOOT_DRIVER(thunderx_xcv) = {
        .name   = "thunderx_xcv",
        .id     = UCLASS_MISC,
        .probe  = thunderx_xcv_probe,
        .of_match = thunderx_xcv_ids,
        .ops    = &thunderx_xcv_ops,
        .priv_auto_alloc_size = sizeof(struct lxcv),
};

static struct pci_device_id thunderx_xcv_supported[] = {
        { PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_THUNDER_XCV) },
        {}
};

U_BOOT_PCI_DEVICE(thunderx_xcv, thunderx_xcv_supported);
