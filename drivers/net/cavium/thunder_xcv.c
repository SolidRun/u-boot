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

#include <cavium/thunderx_smi.h>
#include <cavium/thunderx_vnic.h>

#include "cavm-arch.h"
#define	PCI_DEVICE_ID_THUNDER_XCV	0xA056

struct lxcv {
	void __iomem		*reg_base;
	struct pci_dev		*pdev;
};

struct lxcv *xcv;

int xcv_setup_link(bool link_up, int link_speed)
{
	xcvx_ctl_t xcv_ctl;
	xcvx_reset_t reset;
	xcvx_dll_ctl_t xcv_dll_ctl;
	xcvx_comp_ctl_t xcv_comp_ctl;
	int speed = 2;
	int credits;
	int ret;

	/* Now check RGMII link */
	if (link_speed == 100)
		speed = 1;
	else if (link_speed == 10)
		speed = 0;

	reset.u = readq(CSR_PA(0, XCVX_RESET(0)));
	credits = link_up && !reset.s.enable;
	xcv_ctl.u = readq(CSR_PA(0, XCVX_CTL(0)));

printf("link_up = %d, speed = %d, xcv_ctl.speed = %d\n", link_up, link_speed, speed, xcv_ctl.s.speed);
	if (link_up && (!reset.s.enable || (xcv_ctl.s.speed != speed))) {
		/* Enable the XCV block */
		reset.u = readq(CSR_PA(0, XCVX_RESET(0)));
		reset.s.enable = 1;
		writeq(reset.u, CSR_PA(0, XCVX_RESET(0)));

		/* set operating mode */
		xcv_ctl.u = readq(CSR_PA(0, XCVX_CTL(0)));
		xcv_ctl.s.speed = speed;
		writeq(xcv_ctl.u, CSR_PA(0, XCVX_CTL(0)));

		/* Configure DLL - enable or bypass */
		xcv_dll_ctl.u = readq(CSR_PA(0, XCVX_DLL_CTL(0)));
		xcv_dll_ctl.s.clkrx_set = 0;
		xcv_dll_ctl.s.clkrx_byp = 1;
		xcv_dll_ctl.s.clktx_byp = 0;
		writeq(xcv_dll_ctl.u, CSR_PA(0, XCVX_DLL_CTL(0)));

		/* Enable */
		xcv_dll_ctl.u = readq(CSR_PA(0, XCVX_DLL_CTL(0)));
		xcv_dll_ctl.s.refclk_sel = 0;
		writeq(xcv_dll_ctl.u, CSR_PA(0, XCVX_DLL_CTL(0)));
		reset.u = readq(CSR_PA(0, XCVX_RESET(0)));
		reset.s.dllrst = 0;
		writeq(reset.u, CSR_PA(0, XCVX_RESET(0)));

		/* Delay seems to be needed so XCV_DLL_CTL[CLK_SET] works */
		udelay(10);

		/* Configure compensation controller - enable or disable */
		xcv_comp_ctl.u = readq(CSR_PA(0, XCVX_COMP_CTL(0)));
		xcv_comp_ctl.s.drv_byp = 0;
		writeq(xcv_comp_ctl.u, CSR_PA(0, XCVX_COMP_CTL(0)));

		/* Enable */
		reset.u = readq(CSR_PA(0, XCVX_RESET(0)));
		reset.s.comp = 1;
		writeq(reset.u, CSR_PA(0, XCVX_RESET(0)));

		/* setup the RXC. CLKRST must be zero for loopback */
		xcv_ctl.u = readq(CSR_PA(0, XCVX_CTL(0)));
		reset.u = readq(CSR_PA(0, XCVX_RESET(0)));
		reset.s.clkrst = !xcv_ctl.s.lpbk_int;
		writeq(reset.u, CSR_PA(0, XCVX_RESET(0)));

		/* Datapaths come out of reset
		   - The datapath resets will disengage BGX from the
		     RGMII interface
		   - XCV will continue to return TX credits for each tick
		     that is sent on the TX data path */
		reset.u = readq(CSR_PA(0, XCVX_RESET(0)));
		reset.s.tx_dat_rst_n = 1; 
		reset.s.rx_dat_rst_n = 1; 
		writeq(reset.u, CSR_PA(0, XCVX_RESET(0)));
printf("done with rx/tx data rst\n");
	}

	/* Enable the packet flow
	   - The datapath resets will disengage BGX from the RGMII interface
	   - XCV will continue to return TX credits for each tick that is sent
	     on the TX data path */
	reset.u = readq(CSR_PA(0, XCVX_RESET(0)));
	reset.s.tx_dat_rst_n = link_up; 
	reset.s.rx_dat_rst_n = link_up; 
	writeq(reset.u, CSR_PA(0, XCVX_RESET(0)));

	/* Full reset when link is down */
	if (!link_up) {
		/* Wait 2*MTU in time */
		mdelay(10000);
		/* Reset the world */
		writeq(0, CSR_PA(0, XCVX_RESET(0)));
	}

	/* Grant TX credits */
	if (credits) {
		xcvx_batch_crd_ret_t xcv_crd_ret;
		xcv_crd_ret.u = readq(CSR_PA(0, XCVX_BATCH_CRD_RET(0)));
		xcv_crd_ret.s.crd_ret = 1;
		writeq(xcv_crd_ret.u, CSR_PA(0, XCVX_BATCH_CRD_RET(0)));
	}
	return 0;
}

int thunderx_xcv_probe(struct udevice *dev)
{
	size_t size;

	xcv = dev_get_priv(dev);
	if (xcv == NULL) {
		return -ENOMEM;
	}

	xcv->reg_base = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);
printf("xcv_reg_base = 0x%llx\n", xcv->reg_base);

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
