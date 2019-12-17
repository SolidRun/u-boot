// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * Written By:
 * Konstantin Porotchkin <kostap@marvell.com>
 *
 * Based on Linux driver:
 *   - drivers/uio/pcie-armada-dw-ep.c
 *     authored by Yehuda Yitschak <yehuday@marvell.com>
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

DECLARE_GLOBAL_DATA_PTR;

struct uio_pci {
	struct udevice  *ep;
	struct resource	*host_map;
};

/* make sure we have at least one mem regions to map the host ram */
#define MAX_BAR_MAP		4
#define PCIE_EP_ALL_BARS	0x3f


static int uio_pci_ep_probe(struct udevice *dev)
{
	struct uio_pci *uio_pci = dev_get_priv(dev);
	struct pci_ep_header hdr;
	struct pci_bar bar;
	struct resource res;
	int    bar_id, ret;
	char res_name[20];
	u16 bar_mask = 0;
	u32 val;

	/* Expecting single PCI EP device */
	uclass_first_device(UCLASS_PCI_EP, &uio_pci->ep);
	if (!uio_pci->ep) {
		printf("Failed to find PCI EP driver\n");
		return -EFAULT;
	}

	/* Configure the EP PCIe header */
	memset(&hdr, 0, sizeof(hdr));
	hdr.vendorid = PCI_VENDOR_ID_MARVELL;

	ret = dev_read_u32(dev, "device-id", &val);
	if (ret) {
		printf("missing device-id from DT node\n");
		return ret;
	}
	hdr.deviceid = val;

	ret = dev_read_u32(dev, "vf-device-id", &val);
	if (ret) {
		printf("missing vf-device-id from DT node\n");
		return ret;
	}
	hdr.vf_deviceid = val;

	ret = dev_read_u32(dev, "class-code", &val);
	if (ret) {
		printf("missing class-code from DT node\n");
		return ret;
	}
	hdr.baseclass_code = val;

	ret = dev_read_u32(dev, "subclass-code", &val);
	if (ret) {
		printf("missing subclass-code from DT node\n");
		return ret;
	}
	hdr.subclass_code = val;

	pci_ep_write_header(uio_pci->ep, 0, &hdr);

	/* Setup the BARs according to device tree */
	for (bar_id = 0; bar_id < MAX_BAR_MAP; bar_id++) {
		snprintf(res_name, 5, "bar%d", bar_id);
		ret = dev_read_resource_byname(dev, res_name, &res);
		if (ret)
			continue;

		bar_mask |= 1 << bar_id;

		bar.barno = bar_id;
		bar.phys_addr = res.start;
		bar.size = resource_size(&res);
		if (!is_power_of_2(bar.size)) {
			printf("BAR-%d size in not power of 2\n", bar_id);
			return -EINVAL;
		}

		/* Now create the BAR to match the memory region */
		bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			    PCI_BASE_ADDRESS_MEM_TYPE_32;
		pci_ep_set_bar(uio_pci->ep, 0, &bar);

		/* First 2 BARs in HW are 64 bit BARs and consume 2 BAR slots */
		if (bar_id < 4) {
			bar_id++;
			bar_mask |= 1 << bar_id;
		}
	}

	bar_mask = PCIE_EP_ALL_BARS & ~bar_mask;
	for (bar_id = 0; bar_mask >>= 1; bar_id++) {
		if (bar_mask & 1)
			pci_ep_clear_bar(uio_pci->ep, 0, bar_id);
	}

	/* remap host RAM to local memory space  using shift mapping.
	 * i.e. address 0x0 in host becomes uio_pci->host_map->start.
	 */
	ret = dev_read_resource_byname(dev, "host-map", uio_pci->host_map);
	if (ret) {
		printf("Device tree missing host mappings\n");
		return -ENODEV;
	}

	pci_ep_map_addr(uio_pci->ep, 0, uio_pci->host_map->start,
			0, resource_size(uio_pci->host_map));

	/* Finally, allow the PCIe RC to detect us */
	pci_ep_start(uio_pci->ep);

	printf("Registered UIO PCI EP successfully\n");

	return 0;
}

static int uio_pci_ep_remove(struct udevice *dev)
{
	struct uio_pci *uio_pci = dev_get_priv(dev);
	int    bar_id;

	for (bar_id = 0; bar_id < MAX_BAR_MAP; bar_id++)
		pci_ep_clear_bar(uio_pci->ep, 0, bar_id);

	return 0;
}

const struct udevice_id uio_pci_ep_match[] = {
	{ .compatible = "marvell,pci-ep-uio" },
	{ }
};

U_BOOT_DRIVER(cdns_pcie) = {
	.name	= "marvell,pci-ep-uio",
	.id	= UCLASS_NOP,
	.of_match = uio_pci_ep_match,
	.probe = uio_pci_ep_probe,
	.remove = uio_pci_ep_remove,
	.priv_auto_alloc_size = sizeof(struct uio_pci),
};
