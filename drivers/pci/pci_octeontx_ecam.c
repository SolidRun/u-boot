/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <pci.h>

#include <asm/io.h>


DECLARE_GLOBAL_DATA_PTR;

struct octeontx_pci {
	unsigned int type;

	struct fdt_resource cfg;
	struct fdt_resource bus;
};

static int pci_octeontx_ecam_read_config(struct udevice *bus, pci_dev_t bdf,
					 uint offset, ulong *valuep,
					 enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;

	b = PCI_BUS(bdf) + pcie->bus.start - hose->first_busno;
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

static int pci_octeontx_ecam_write_config(struct udevice *bus, pci_dev_t bdf,
					 uint offset, ulong valuep,
					 enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;

	b = PCI_BUS(bdf) + pcie->bus.start - hose->first_busno;
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

static int pci_octeontx_pem_read_config(struct udevice *bus, pci_dev_t bdf,
					uint offset, ulong *valuep,
					enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;
	u8  hdrtype;
	u8  pri_bus = pcie->bus.start + 1 - hose->first_busno;
	u32 bus_offs = (pri_bus << 16) | (pri_bus << 8) | (pri_bus << 0);

	b = PCI_BUS(bdf) + 1 - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 24) | (d << 19) | (f << 16);

	address += pcie->cfg.start;

	*valuep = pci_conv_32_to_size(~0UL, offset, size);

	if (b == 1 && d > 0)
		return 0;

	switch (size) {
	case PCI_SIZE_8:
		*valuep = readb(address + offset);
		break;
	case PCI_SIZE_16:
		*valuep = readw(address + offset);
		break;
	case PCI_SIZE_32:
		*valuep = readl(address + offset);
		break;
	default:
		printf("Invalid size\n");
	}

	hdrtype = readb(address + PCI_HEADER_TYPE);

	if ((hdrtype == PCI_HEADER_TYPE_BRIDGE) &&
	    (offset >= PCI_PRIMARY_BUS) &&
	    (offset <= PCI_SUBORDINATE_BUS) &&
	    *valuep != pci_conv_32_to_size(~0UL, offset, size)) {
		*valuep -= pci_conv_32_to_size(bus_offs, offset, size);
	}
/*
	debug("%02x.%02x.%02x: u%d %x (%lx) -> %lx\n",
	      b, d, f, size, offset, address, *valuep);
*/
	return 0;
}

static int pci_octeontx_pem_write_config(struct udevice *bus, pci_dev_t bdf,
					 uint offset, ulong value,
					 enum pci_size_t size)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(bus);
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	uintptr_t address;
	u32 b, d, f;
	u8  hdrtype;
	u8  pri_bus = pcie->bus.start + 1 - hose->first_busno;
	u32 bus_offs = (pri_bus << 16) | (pri_bus << 8) | (pri_bus << 0);

	b = PCI_BUS(bdf) + 1 - hose->first_busno;
	d = PCI_DEV(bdf);
	f = PCI_FUNC(bdf);

	address = (b << 24) | (d << 19) | (f << 16);

	address += pcie->cfg.start;

	hdrtype = readb(address + PCI_HEADER_TYPE);

	if ((hdrtype == PCI_HEADER_TYPE_BRIDGE) &&
	    (offset >= PCI_PRIMARY_BUS) &&
	    (offset <= PCI_SUBORDINATE_BUS) &&
	    (value != pci_conv_32_to_size(~0UL, offset, size))) {
		value +=  pci_conv_32_to_size(bus_offs, offset, size);
	}

	if (b == 1 && d > 0)
		return 0;

	switch (size) {
	case PCI_SIZE_8:
		writeb(value, address + offset);
		break;
	case PCI_SIZE_16:
		writew(value, address + offset);
		break;
	case PCI_SIZE_32:
		writel(value, address + offset);
		break;
	default:
		printf("Invalid size\n");
	}
/*
	debug("%02x.%02x.%02x: u%d %x (%lx) <- %lx\n",
	      b, d, f, size, offset, address, value);
*/
	return 0;
}

static int pci_octeontx_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}

static int pci_octeontx_ecam_probe(struct udevice *dev)
{
	struct octeontx_pci *pcie = (void *)dev_get_priv(dev);
	int err;

	err = fdt_get_resource(gd->fdt_blob, dev->node.of_offset, "reg", 0,
			       &pcie->cfg);

	if (err) {
		printf("Error reading resource: %s\n", fdt_strerror(err));
		return err;
	}

	err = fdtdec_get_pci_bus_range(gd->fdt_blob, dev->node.of_offset,
				       &pcie->bus);

	if (err) {
		printf("Error reading resource: %s\n", fdt_strerror(err));
		return err;
	}

	return 0;
}

static const struct dm_pci_ops pci_octeontx_ecam_ops = {
	.read_config	= pci_octeontx_ecam_read_config,
	.write_config	= pci_octeontx_ecam_write_config,
};

static const struct udevice_id pci_octeontx_ecam_ids[] = {
	{ .compatible = "cavium,pci-host-thunder-ecam" },
	{ .compatible = "cavium,pci-host-octeontx-ecam" },
	{ .compatible = "pci-host-ecam-generic" },
	{ }
};

static const struct dm_pci_ops pci_octeontx_pem_ops = {
	.read_config	= pci_octeontx_pem_read_config,
	.write_config	= pci_octeontx_pem_write_config,
};

static const struct udevice_id pci_octeontx_pem_ids[] = {
	{ .compatible = "cavium,pci-host-thunder-pem" },
	{ }
};

U_BOOT_DRIVER(pci_octeontx_ecam) = {
	.name	= "pci_octeontx_ecam",
	.id	= UCLASS_PCI,
	.of_match = pci_octeontx_ecam_ids,
	.ops	= &pci_octeontx_ecam_ops,
	.ofdata_to_platdata = pci_octeontx_ofdata_to_platdata,
	.probe	= pci_octeontx_ecam_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_pci),
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(pci_octeontx_pcie) = {
	.name	= "pci_octeontx_pem",
	.id	= UCLASS_PCI,
	.of_match = pci_octeontx_pem_ids,
	.ops	= &pci_octeontx_pem_ops,
	.ofdata_to_platdata = pci_octeontx_ofdata_to_platdata,
	.probe	= pci_octeontx_ecam_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_pci),
};
