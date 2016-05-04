/** @file
#
#  Copyright (c) 2014, Cavium Inc. All rights reserved.<BR>
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <pci.h>

#include <asm/io.h>


#define PCI_VENDOR_CAVIUM 0x177d
#define PCI_DEV_CAVIUM_RC 0xa100

#define PCI_CAP_EA 0x14

#define RTARGET_MASK 0x1f;
#define OPCODE_SIZE 4;

DECLARE_GLOBAL_DATA_PTR;

struct thunderx_pci {
	unsigned int type;

	struct fdt_resource cfg;

	struct list_head list;
};

static LIST_HEAD(thunderx_ecams);

static int pci_thunderx_ecam_read_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong *valuep,
				 enum pci_size_t size)
{
	struct thunderx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;

	b = PCI_BUS(bdf) - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 20) | (d << 15) | (f << 12) | offset;

	address += pcie->cfg.start;

	switch (size) {
	case PCI_SIZE_8:
		*valuep = readb(address);
		break;
	case PCI_SIZE_16:
		*valuep = readw(address);
		break;
	case PCI_SIZE_32:
		*valuep = readl(address);
		break;
	};
/*
	debug("%02x.%02x.%02x: u%d %x -> %lx\n",
	      b, d, f, size, offset, *valuep);
*/
	return 0;
}

static int pci_thunderx_ecam_write_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong valuep,
				 enum pci_size_t size)
{
	struct thunderx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;

	b = PCI_BUS(bdf) - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 20) | (d << 15) | (f << 12) | offset;

	address += pcie->cfg.start;

	switch (size) {
	case PCI_SIZE_8:
		writeb(valuep, address);
		break;
	case PCI_SIZE_16:
		writew(valuep, address);
		break;
	case PCI_SIZE_32:
		writel(valuep, address);
		break;
	};
/*
	debug("%02x.%02x.%02x: u%d %x <- %lx\n",
	      b, d, f, size, offset, valuep);
*/
	return 0;
}

static int pci_thunderx_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}

static int pci_thunderx_ecam_probe(struct udevice *dev)
{
	struct thunderx_pci *pcie = (void *)dev_get_priv(dev);
	int err;

	err = fdt_get_resource(gd->fdt_blob, dev->of_offset, "reg", 0, &pcie->cfg);

	if (err) {
		printf("Error reading resource: %s\n", fdt_strerror(err));
		return err;
	}

	return 0;
}

static const struct dm_pci_ops pci_thunderx_ecam_ops = {
	.read_config	= pci_thunderx_ecam_read_config,
	.write_config	= pci_thunderx_ecam_write_config,
};

static const struct udevice_id pci_thunderx_ecam_ids[] = {
	{ .compatible = "cavium,pci-host-thunder-ecam" },
	{ .compatible = "pci-host-ecam-generic" },
	{ }
};

#if 0
static const struct udevice_id pci_thunderx_pem_ids[] = {
	{ .compatible = "cavium,pci-host-thunder-pem" },
	{ }
};
#endif

U_BOOT_DRIVER(pci_thunderx_ecam) = {
	.name	= "pci_thunderx_ecam",
	.id	= UCLASS_PCI,
	.of_match = pci_thunderx_ecam_ids,
	.ops	= &pci_thunderx_ecam_ops,
	.ofdata_to_platdata = pci_thunderx_ofdata_to_platdata,
	.probe	= pci_thunderx_ecam_probe,
	.priv_auto_alloc_size = sizeof(struct thunderx_pci),
};
