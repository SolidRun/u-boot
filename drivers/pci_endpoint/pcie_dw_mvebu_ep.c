// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * Written By:
 * Konstantin Porotchkin <kostap@marvell.com>
 *
 * Based on :
 *   - pci_endpoint/pcie-cadence-ep.c
 * and Linux driver:
 *   - drivers/pci/endpoint/pcie-armada-dw-ep.c
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci_ep.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/log2.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <asm/io.h>
#include "pcie_dw_mvebu_ep.h"

DECLARE_GLOBAL_DATA_PTR;

static int pcie_dw_mvebu_ep_setup_bar(struct udevice *dev, uint func_id,
				      struct pci_bar *bar)
{
	struct armada_pcie_ep *ep = dev_get_priv(dev);
	void *pl_regs = ep->pl_regs;
	u32  v = 0;
	u32  region_indx = get_in_region_idx(func_id, bar->barno);
	void *bar_addr = cfg_func_base(ep, func_id,
				PCI_BASE_ADDRESS_0 + (bar->barno * 4));
	void *bar_mask = cfg_shadow_func_base(ep, func_id,
				PCI_BASE_ADDRESS_0 + (bar->barno * 4));

	if (bar->flags & PCI_BASE_ADDRESS_SPACE_IO) {
		v = bar->flags & (~PCI_BASE_ADDRESS_IO_MASK);
		writel(v, bar_addr);
	} else {
		/* clear the top 32 bits of the size */
		if (bar->flags & PCI_BASE_ADDRESS_MEM_TYPE_64) {
			writel((bar->size - 1) >> 32, bar_mask + 4);
			writel(0, bar_addr + 4);
		}
		v = bar->flags & (~PCI_BASE_ADDRESS_MEM_MASK);
		writel(v, bar_addr);
	}

	/* Set size and enable bar */
	v = ((bar->size - 1) & U32_MAX) | BAR_ENABLE_MASK;
	writel(v, bar_mask);

	/* Setup the internal target for the BAR.
	 * When the PCIe host accesses the bar
	 * it will reach the space defined by "addr" and "size"
	 * This code is moved from a separate function in Linux
	 * driver: armada_pcie_ep_bar_map()
	 */
	v = PCIE_ATU_REGION_INBOUND | region_indx;
	writel(v, pl_regs + PCIE_ATU_VIEWPORT);

	bar->phys_addr = bar->phys_addr & ~(bar->size - 1);
	v = lower_32_bits(bar->phys_addr);
	writel(v, pl_regs + PCIE_ATU_LOWER_TARGET);

	v = upper_32_bits(bar->phys_addr);
	writel(v, pl_regs + PCIE_ATU_UPPER_TARGET);

	v = (func_id & PCIE_ATU_CR1_FUNC_MASK) << PCIE_ATU_CR1_FUNC_OFF;
	writel(v, pl_regs + PCIE_ATU_CR1);

	v = PCIE_ATU_CR2_REGION_EN |
	    PCIE_ATU_CR2_BAR_EN |
	    (bar->barno << PCIE_ATU_CR2_BAR_OFF);
	writel(v, pl_regs + PCIE_ATU_CR2);

	return 0;
}

static int pcie_dw_mvebu_ep_disable_bar(struct udevice *dev, uint func_num,
					enum pci_barno barnum)
{
	struct armada_pcie_ep *ep = dev_get_priv(dev);
	void *bar_mask = cfg_shadow_func_base(ep, func_num,
					      PCI_BASE_ADDRESS_0);

	writel(0, bar_mask + barnum * 4);

	return 0;
}

static int pcie_dw_mvebu_ep_enable(struct udevice *dev)
{
	struct armada_pcie_ep *ep = dev_get_priv(dev);
	u32 v;

	v = readl(ep->lm_regs + PCIE_GLOBAL_CTRL);
	v &= ~PCIE_GLOBAL_CTRL_CRS_EN;
	writel(v, ep->lm_regs + PCIE_GLOBAL_CTRL);

	return 0;
}

/*
 * Remap the host memory space to the local memory space.
 * By default the memory spaces conflict so we must offset the
 * host memory space in our local memory space
 */
static int pcie_dw_mvebu_ep_remap_host(struct udevice *dev, uint func_id,
				       phys_addr_t addr, u64 pci_addr,
				       size_t size)
{
	struct armada_pcie_ep *ep = dev_get_priv(dev);
	void *pl_regs = ep->pl_regs;
	u32  v, region = 0;
	u64  remain_size = size;

	/* ATU window size must be power of 2 */
	if (!is_power_of_2(size))
		return -EINVAL;

	while (remain_size > 0) {
		if (region > MAX_ATU_REGIONS) {
			printf("Insufficient ATU regions to map hosts\n");
			return -1;
		}

		v = PCIE_ATU_REGION_OUTBOUND;
		v |= get_out_region_idx(func_id, region);
		writel(v, pl_regs + PCIE_ATU_VIEWPORT);

		writel(addr & U32_MAX, pl_regs + PCIE_ATU_LOWER_BASE);
		writel(addr >> 32, pl_regs + PCIE_ATU_UPPER_BASE);
		writel(pci_addr & U32_MAX, pl_regs + PCIE_ATU_LOWER_TARGET);
		writel(pci_addr >> 32, pl_regs + PCIE_ATU_UPPER_TARGET);

		if (remain_size > MAX_ATU_SIZE)
			v = MAX_ATU_SIZE - 1;
		else
			v = remain_size - 1;
		writel(v, pl_regs + PCIE_ATU_LIMIT);

		v = (func_id & PCIE_ATU_CR1_FUNC_MASK) << PCIE_ATU_CR1_FUNC_OFF;
		writel(v, pl_regs + PCIE_ATU_CR1);

		v = PCIE_ATU_CR2_REGION_EN;
		writel(v, pl_regs + PCIE_ATU_CR2);

		region++;
		addr += MAX_ATU_SIZE;
		pci_addr += MAX_ATU_SIZE;
		remain_size -= MAX_ATU_SIZE;
	}

	return 0;
}

/* setup the PCIe configuration header */
static int pcie_dw_armada_write_header(struct udevice *dev, uint func_id,
				       struct pci_ep_header *hdr)
{
	struct armada_pcie_ep *ep = dev_get_priv(dev);
	void *cfg_addr = cfg_func_base(ep, func_id, 0);

	writew(hdr->vendorid, cfg_addr + PCI_VENDOR_ID);
	writew(hdr->deviceid, cfg_addr + PCI_DEVICE_ID);
	writew(hdr->vf_deviceid, cfg_addr + PCIE_SRIOV_DEVID_OFFSET);

	writeb(hdr->revid,  cfg_addr + PCI_REVISION_ID);
	writeb(hdr->progif_code,  cfg_addr + PCI_CLASS_PROG);
	writew((hdr->baseclass_code << 8) | hdr->subclass_code,
	       cfg_addr + PCI_CLASS_DEVICE);

	writew(hdr->subsys_id,  cfg_addr + PCI_SUBSYSTEM_ID);
	writew(hdr->subsys_vendor_id,
	       cfg_addr + PCI_SUBSYSTEM_VENDOR_ID);

	return 0;
}

static int pcie_dw_mvebu_ep_probe(struct udevice *dev)
{
	struct armada_pcie_ep *ep = dev_get_priv(dev);
	struct resource res;
	void *p;

	/* Get registers bases and remap */
	dev_read_resource_byname(dev, "lm", &res);
	p = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(p)) {
		printf("couldn't remap lm regs base %pR\n", &res);
		return PTR_ERR(p);
	}
	ep->lm_regs = p;

	dev_read_resource_byname(dev, "core", &res);
	p = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(p)) {
		printf("couldn't remap core regs base %pR\n", &res);
		return PTR_ERR(p);
	}
	ep->regs = p;
	ep->pl_regs = p;

	dev_read_resource_byname(dev, "shadow_core", &res);
	p = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(p)) {
		printf("couldn't remap shadow regs base %pR\n", &res);
		return PTR_ERR(p);
	}
	ep->shadow_regs = p;

	/* Disable Function 0. Set the vendor ID to 0xFFFFFFFF to avoid
	 * detection until the EP is fully configured
	 */
	writel(0xffffffff, cfg_func_base(ep, 0, PCI_VENDOR_ID));

	return 0;
}

static int pcie_dw_mvebu_ep_remove(struct udevice *dev)
{
	return 0;
}

static const struct pci_ep_ops pcie_dw_mvebu_ep_ops = {
	.write_header = pcie_dw_armada_write_header,
	.map_addr = pcie_dw_mvebu_ep_remap_host,
	.set_bar = pcie_dw_mvebu_ep_setup_bar,
	.clear_bar = pcie_dw_mvebu_ep_disable_bar,
	.start = pcie_dw_mvebu_ep_enable,
};

static const struct udevice_id pcie_dw_mvebu_ep_ids[] = {
	{ .compatible = "marvell,armada-pcie-ep" },
	{ }
};

U_BOOT_DRIVER(pcie_dw_armada_ep) = {
	.name			= "armada-pcie-ep",
	.id			= UCLASS_PCI_EP,
	.of_match		= pcie_dw_mvebu_ep_ids,
	.ops			= &pcie_dw_mvebu_ep_ops,
	.probe			= pcie_dw_mvebu_ep_probe,
	.remove			= pcie_dw_mvebu_ep_remove,
	.priv_auto_alloc_size	= sizeof(struct armada_pcie_ep),
};
