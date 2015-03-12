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

#define DEBUG

#include <common.h>
#include <pci.h>
#include <cavm-csrs.h>

#define PCI_VENDOR_CAVIUM 0x177d
#define PCI_DEV_CAVIUM_RC 0xa100

#define RTARGET_MASK 0x1f;
#define OPCODE_SIZE 4;

void spec_sync_handler(struct pt_regs *pt_regs, unsigned int esr)
{
	uint32_t opcode;
	uintptr_t far;

	int ecam, el, Rt;

	/* get faulting address */
	el = current_el();

	if (el == 1) {
		asm volatile("mrs %0, far_el1" : "=r" (far) : : "cc");
	} else if (el == 2) {
		asm volatile("mrs %0, far_el2" : "=r" (far) : : "cc");
	} else {
		asm volatile("mrs %0, far_el3" : "=r" (far) : : "cc");
	}

	opcode = readl(pt_regs->elr);

#if 0
	debug("FAR: %lx\n", far);
	show_regs(pt_regs);
	debug("Faulting opcode: %lx\n", opcode);
#endif

	for (ecam = 0; ecam < CONFIG_THUNDER_ECAMS; ecam++) {
		if (far >= ECAMX_PF_BAR2(ecam) &&
				far < ECAMX_PF_BAR2(ecam) + ECAMX_PF_BAR2_SIZE) {
			pt_regs->elr += OPCODE_SIZE;
			Rt = opcode & RTARGET_MASK;
			pt_regs->regs[Rt] = ~0UL;

			return;
		}
	}

	printf("Fault at: %lx\n", far);
	show_regs(pt_regs);
	panic("Resetting CPU...");
	return;
}

#if 1

int rc_is_on(unsigned int rc)
{
	union pemx_on pemx_on;

	if (rc > 5)
		return 0;

	pemx_on.u = readq(PEMX_ON(rc));

	return ((pemx_on.s.pemon != 0) && (pemx_on.s.pemoor != 0));
}

#define PCI_RC_READ(size)												\
static int thunderx_rc_rd_config_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size *val)				\
{																		\
	u32 b, d, f;														\
	union pemx_cfg_rd pemx_cfg_rd;										\
																		\
	pemx_cfg_rd.u = 0;													\
	b = PCI_BUS(dev);													\
	d = PCI_DEV(dev);													\
	f = PCI_FUNC(dev);													\
	if ((b == 0) && (f == 0) && rc_is_on(d - 1)) {						\
		pemx_cfg_rd.s.addr = offset & ~0x3;								\
		writeq(pemx_cfg_rd.u, PEMX_CFG_RD(d - 1));						\
		udelay(10);														\
		pemx_cfg_rd.u = readq(PEMX_CFG_RD(d - 1));						\
		*val = (u##size)(pemx_cfg_rd.s.data >> (8 * (offset & 0x3)));	\
	} else {															\
		*val = (u##size)~0UL;											\
	}																	\
	return 0;															\
}

PCI_RC_READ(8)
PCI_RC_READ(16)
PCI_RC_READ(32)

#define PCI_RC_WRITE(size)												\
static int thunderx_rc_wr_config_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size val)					\
{																		\
	u32 b, d, f;														\
	union pemx_cfg_rd pemx_cfg_rd;										\
																		\
	pemx_cfg_rd.u = 0;													\
	b = PCI_BUS(dev) & 0x3f;											\
	d = PCI_DEV(dev);													\
	f = PCI_FUNC(dev);													\
	if ((b == 0) && (f == 0) && rc_is_on(d - 1)) {						\
		pemx_cfg_rd.s.addr = offset & ~0x3;								\
		pemx_cfg_rd.s.data = val << (8 * (offset & 0x3));				\
		writeq(pemx_cfg_rd.u, PEMX_CFG_WR(d - 1));						\
		udelay(10);														\
	}																	\
	return 0;															\
}

PCI_RC_WRITE(8)
PCI_RC_WRITE(16)
PCI_RC_WRITE(32)

#define write8	writeb
#define write16	writew
#define write32	writel

#define read8	readb
#define read16	readw
#define read32	readl

#define PCI_READ(size)												\
static int thunderx_rd_config_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size *val)			\
{																	\
	u32 b, d, f;													\
	u8 hdrtype;														\
	union ecam_cfg_addr_s address;									\
	int ecam = (uintptr_t)hose->priv_data;							\
																	\
	b = PCI_BUS(dev) - hose->first_busno;							\
	d = PCI_DEV(dev);												\
	f = PCI_FUNC(dev);												\
																	\
	address.u = 0;													\
	address.s.func = (d << 3) | (f << 0);							\
	address.s.bus  = b;												\
	address.s.addr = PCI_HEADER_TYPE;								\
	address.u += ECAMX_PF_BAR2(ecam);								\
																	\
	hdrtype = read8(address.u);										\
																	\
	address.u = 0;													\
	address.s.func = (d << 3) | (f << 0);							\
	address.s.bus  = b;												\
	address.s.addr = offset;										\
	address.u += ECAMX_PF_BAR2(ecam);								\
																	\
	*val = read##size(address.u);									\
																	\
	debug("%02x.%02x.%02x: %x -> %x\n", b, d, f,					\
				offset, *val);										\
	if (hdrtype == PCI_HEADER_TYPE_BRIDGE) {						\
		switch (offset) {											\
		case PCI_PRIMARY_BUS:										\
		case PCI_SECONDARY_BUS:										\
		case PCI_SUBORDINATE_BUS:									\
			*val += hose->first_busno;								\
		}															\
	}																\
																	\
																	\
	return 0;														\
}

PCI_READ(8)
PCI_READ(16)
PCI_READ(32)

#define PCI_WRITE(size)												\
static int thunderx_wr_config_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size val)				\
{																	\
	u32 b, d, f;													\
	u8 hdrtype;														\
	union ecam_cfg_addr_s address;									\
	int ecam = (uintptr_t)hose->priv_data;							\
																	\
	b = PCI_BUS(dev) - hose->first_busno;							\
	d = PCI_DEV(dev);												\
	f = PCI_FUNC(dev);												\
	dev = PCI_BDF(b, d, f);											\
																	\
	address.u = 0;													\
	address.s.func = (d << 3) | (f << 0);							\
	address.s.bus  = b;												\
	address.s.addr = PCI_HEADER_TYPE;								\
	address.u += ECAMX_PF_BAR2(ecam);								\
																	\
	hdrtype = read8(address.u);										\
																	\
	debug("%02x.%02x.%02x: %x <- %x\n", b, d, f,					\
					offset, val);									\
	if (hdrtype == PCI_HEADER_TYPE_BRIDGE) {						\
		switch (offset) {											\
		case PCI_PRIMARY_BUS:										\
		case PCI_SECONDARY_BUS:										\
		case PCI_SUBORDINATE_BUS:									\
			val -= hose->first_busno;								\
		}															\
	}																\
																	\
	address.u = 0;													\
	address.s.func = (d << 3) | (f << 0);							\
	address.s.bus  = b;												\
	address.s.addr = offset;										\
	address.u += ECAMX_PF_BAR2(ecam);								\
																	\
	write##size(val, address.u);									\
																	\
	return 0;														\
}

PCI_WRITE(8)
PCI_WRITE(16)
PCI_WRITE(32)

static struct pci_controller pcie_hose[CONFIG_THUNDER_ECAMS];

void pci_init_board(void)
{
	unsigned int bus = 0;
	uintptr_t ecam;

	int ret;

	for (ecam = 0; ecam < CONFIG_THUNDER_ECAMS; ecam++) {
		debug("%s: %d, ecam: %d\n", __FUNCTION__, __LINE__, ecam);
		pcie_hose[ecam].first_busno = bus;
		pcie_hose[ecam].last_busno = 0xff;

		pcie_hose[ecam].region_count = 0;
		pcie_hose[ecam].priv_data    = (void*)ecam;

		pci_set_ops(&pcie_hose[ecam],
					thunderx_rd_config_u8,
					thunderx_rd_config_u16,
					thunderx_rd_config_u32,
					thunderx_wr_config_u8,
					thunderx_wr_config_u16,
					thunderx_wr_config_u32);

		pci_register_hose(&pcie_hose[ecam]);

		ret = pci_hose_scan(&pcie_hose[ecam]);

		debug("%s: %d, ret: %d\n", __FUNCTION__, __LINE__, ret);
		if (ret > 0) {
			bus = ret;
			pcie_hose[ecam].last_busno = bus;
			bus++;
		}
	}


#if 0
	pci_set_region(hose->regions + 0,
			CONFIG_SYS_PCI_MEMORY_BUS,
			CONFIG_SYS_PCI_MEMORY_PHYS,
			CONFIG_SYS_PCI_MEMORY_SIZE,
			PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region(hose->regions + 1,
			CONFIG_SYS_PCI_MEM_BUS,
			CONFIG_SYS_PCI_MEM_PHYS,
			CONFIG_SYS_PCI_MEM_SIZE,
			PCI_REGION_MEM);

	/* ISA/PCI memory space */
	pci_set_region(hose->regions + 2,
			CONFIG_SYS_ISA_MEM_BUS,
			CONFIG_SYS_ISA_MEM_PHYS,
			CONFIG_SYS_ISA_MEM_SIZE,
			PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region(hose->regions + 3,
			CONFIG_SYS_PCI_IO_BUS,
			CONFIG_SYS_PCI_IO_PHYS,
			CONFIG_SYS_PCI_IO_SIZE,
			PCI_REGION_IO);

	/* ISA/PCI I/O space */
	pci_set_region(hose->regions + 4,
			CONFIG_SYS_ISA_IO_BUS,
			CONFIG_SYS_ISA_IO_PHYS,
			CONFIG_SYS_ISA_IO_SIZE,
			PCI_REGION_IO);

	hose->region_count = 5;

	/* FIXME: TODO */
#endif

}

int pci_config_fixed(struct pci_controller *hose, pci_dev_t dev)
{
	u16 vendor, device;
	int ret = 0;

	pci_hose_read_config_word(hose, dev, PCI_DEVICE_ID, &device);
	pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

	if (vendor == PCI_VENDOR_CAVIUM) {
		ret = device != PCI_DEV_CAVIUM_RC;
	}

	if (!ret) {
		debug("Non-fixed configuration at %02x:%02x.%02x\n", PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
	}

	return ret;
}

#endif /* CONFIG_PCI */
