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
#include <pci.h>
#include <cavm-csr.h>
#include <cavium/atf.h>

#define PCI_VENDOR_CAVIUM 0x177d
#define PCI_DEV_CAVIUM_RC 0xa100

#define RTARGET_MASK 0x1f;
#define OPCODE_SIZE 4;

#define ECAMS_PER_NODE 4
#define ECAM_GROUPS 2
#define RCS_PER_NODE 6
#define RCS_PER_SLI 3

struct thunderx_ecam {
	int node;
	int ecam;
};

static struct thunderx_ecam thunderx_ecam[CONFIG_THUNDERX_ECAMS];
static struct pci_controller ecam_hose[CONFIG_THUNDERX_ECAMS];
static struct pci_controller pem_hose[RCS_PER_NODE];


#define RTARGET_MASK 0x1f;
#define OPCODE_SIZE 4;

void do_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	int node, ecam;
	uint32_t opcode;
	uintptr_t far = read_far();
	uintptr_t baseaddr, endaddr;

	int ecam, el, Rt;

	/* get faulting address */

	for (ecam = 0; ecam < CONFIG_THUNDERX_ECAMS; ecam++) {
		node = thunderx_ecam[ecam].node;
		ecam = thunderx_ecam[ecam].node;
		baseaddr = CSR_PA(node, ECAMX_PF_BAR2(ecam));
		endaddr = baseaddr + ECAMX_PF_BAR2_SIZE;
		if (far >= baseaddr && far < endaddr) {
			opcode = readl(pt_regs->elr);
			pt_regs->elr += OPCODE_SIZE;
			Rt = opcode & RTARGET_MASK;
			pt_regs->regs[Rt] = ~0UL;

			return;
		}
	}

	show_regs(pt_regs);
	panic("Resetting CPU...");
	return;
}

static uintptr_t thunderx_mmcfg_addr(unsigned int rc, pci_dev_t dev,
				     unsigned int reg)
{
	union sli_s2m_op_s sli_s2m_op;
	unsigned long node = rc / RCS_PER_NODE;
	unsigned long sli = rc / RCS_PER_SLI;
	unsigned long sli_group = rc % RCS_PER_SLI;

	sli_s2m_op.u = 0;
	sli_s2m_op.s.io = 1;
	sli_s2m_op.s.node = node;
	sli_s2m_op.s.did_hi = 8 + sli;
	sli_s2m_op.s.region = sli_group << 6;

	sli_s2m_op.s.addr = PCI_BUS(dev) << 24;
	sli_s2m_op.s.addr |= PCI_DEV(dev) << 19;
	sli_s2m_op.s.addr |= PCI_FUNC(dev) << 16;
	sli_s2m_op.s.addr |= reg;

	return sli_s2m_op.u;
}

#define write8	writeb
#define write16	writew
#define write32	writel

#define read8	readb
#define read16	readw
#define read32	readl

#define PCI_ECAM_READ(size)							\
static int thunderx_rd_ecam_u##size(struct pci_controller *hose,		\
				pci_dev_t dev, int offset, u##size *val)	\
{										\
	u32 b, d, f;								\
	union ecam_cfg_addr_s address;						\
	struct thunderx_ecam *ecam = hose->priv_data;				\
										\
	b = PCI_BUS(dev) - hose->first_busno;					\
	d = PCI_DEV(dev);							\
	f = PCI_FUNC(dev);							\
										\
	address.u = 0;								\
	address.s.func = (d << 3) | (f << 0);					\
	address.s.bus  = b;							\
	address.s.addr = offset;						\
	address.u += CSR_PA(ecam->node, ECAMX_PF_BAR2(ecam->ecam));		\
										\
	*val = read##size(address.u);						\
	debug("%d.%d::%02x.%02x.%02x: u%d %x -> %x\n",				\
		ecam->node, ecam->ecam, b, d, f, size, offset, *val);		\
										\
	return 0;								\
}

PCI_ECAM_READ(8)
PCI_ECAM_READ(16)
PCI_ECAM_READ(32)

#define PCI_ECAM_WRITE(size)							\
static int thunderx_wr_ecam_u##size(struct pci_controller *hose,		\
				pci_dev_t dev, int offset, u##size val)		\
{										\
	u32 b, d, f;								\
	union ecam_cfg_addr_s address;						\
	struct thunderx_ecam *ecam = hose->priv_data;				\
										\
	b = PCI_BUS(dev) - hose->first_busno;					\
	d = PCI_DEV(dev);							\
	f = PCI_FUNC(dev);							\
	dev = PCI_BDF(b, d, f);							\
	debug("%d.%d::%02x.%02x.%02x: u%d %x <- %x\n",				\
		ecam->node, ecam->ecam, b, d, f, size, offset, val);		\
										\
	address.u = 0;								\
	address.s.func = (d << 3) | (f << 0);					\
	address.s.bus  = b;							\
	address.s.addr = offset;						\
	address.u += CSR_PA(ecam->node, ECAMX_PF_BAR2(ecam->ecam));		\
										\
	write##size(val, address.u);						\
										\
	return 0;								\
}

PCI_ECAM_WRITE(8)
PCI_ECAM_WRITE(16)
PCI_ECAM_WRITE(32)

#define PCI_PEM_READ(size)							\
static int thunderx_rd_pem_u##size(struct pci_controller *hose,			\
				pci_dev_t dev, int offset, u##size *val)	\
{										\
	uintptr_t mmcfg_addr;							\
	u32 b, d, f;								\
	int rc = (uintptr_t)hose->priv_data;					\
										\
	b = PCI_BUS(dev);							\
	d = PCI_DEV(dev);							\
	f = PCI_FUNC(dev);							\
										\
	if (d == 0) {								\
		mmcfg_addr = thunderx_mmcfg_addr(rc, PCI_BDF(b, d, f),		\
						 offset);			\
		*val = read##size(mmcfg_addr);					\
		debug("%02x.%02x.%02x: %x -> %x\n", b, d, f, offset, *val);	\
	} else {								\
		*val = (u##size)~0UL;						\
	}									\
										\
	return 0;								\
}

PCI_PEM_READ(8)
PCI_PEM_READ(16)
PCI_PEM_READ(32)

#define PCI_PEM_WRITE(size)							\
static int thunderx_wr_pem_u##size(struct pci_controller *hose,			\
				pci_dev_t dev, int offset, u##size val)		\
{										\
	uintptr_t mmcfg_addr;							\
	u32 b, d, f;								\
	int rc = (uintptr_t)hose->priv_data;					\
										\
	b = PCI_BUS(dev);							\
	d = PCI_DEV(dev);							\
	f = PCI_FUNC(dev);							\
	debug("%02x.%02x.%02x: %x <- %x\n", b, d, f,				\
					offset, val);				\
										\
	if (d == 0) {								\
		mmcfg_addr = thunderx_mmcfg_addr(rc,				\
						 PCI_BDF(b, d, f), offset);	\
										\
		write##size(val, mmcfg_addr);					\
	}									\
										\
	return 0;								\
}

PCI_PEM_WRITE(8)
PCI_PEM_WRITE(16)
PCI_PEM_WRITE(32)

int rc_is_on(unsigned int rc)
{
	union pemx_on pemx_on;

	if (rc > 5)
		return 0;

	pemx_on.u = readq(PEMX_ON(rc));

	return ((pemx_on.s.pemon != 0) && (pemx_on.s.pemoor != 0));
}

static int thunderx_read_rc_u32(int rc, int offset, u32 * val)
{
	union pemx_cfg_rd pemx_cfg_rd;
	pemx_cfg_rd.u = 0;

	if ((rc >= 0) && rc_is_on(rc)) {
		pemx_cfg_rd.s.addr = offset & ~0x3;
		writeq(pemx_cfg_rd.u, PEMX_CFG_RD(rc));
		pemx_cfg_rd.u = readq(PEMX_CFG_RD(rc));
		*val = pemx_cfg_rd.s.data;
	} else {
		*val = (u32) ~ 0UL;
	}
	return 0;
}

void pci_init_board(void)
{
	long ecam, pem;
	u32 reg;
	u8 sec_bus, sub_bus;

	u64 pci_mem[] = CONFIG_SYS_PCI_MEM_CPU;
	u64 pci_pref[] = CONFIG_SYS_PCI_PREF_CPU;
	u64 pci_io[] = CONFIG_SYS_PCI_IO_CPU;
	uintptr_t baseaddr;
	int ret, node;

	for (ecam = 0; ecam < CONFIG_THUNDERX_ECAMS; ecam++) {
		thunderx_ecam[ecam].node = ecam / ECAMS_PER_NODE;
		thunderx_ecam[ecam].ecam = ecam % ECAMS_PER_NODE;

		if (thunderx_ecam[ecam].node >= atf_node_count())
			break;

		debug("%s: %d, ecam: %ld\n", __FUNCTION__, __LINE__, ecam);

		ecam_hose[ecam].first_busno = pci_last_busno() + 1;
		ecam_hose[ecam].last_busno = 0xff;

		ecam_hose[ecam].region_count = 0;
		ecam_hose[ecam].priv_data = &thunderx_ecam[ecam];

		pci_set_ops(&ecam_hose[ecam],
			    thunderx_rd_ecam_u8,
			    thunderx_rd_ecam_u16,
			    thunderx_rd_ecam_u32,
			    thunderx_wr_ecam_u8,
			    thunderx_wr_ecam_u16,
			    thunderx_wr_ecam_u32);

		pci_register_hose(&ecam_hose[ecam]);

		ret = pci_hose_scan(&ecam_hose[ecam]);

		debug("%s: %d, ret: %d\n", __FUNCTION__, __LINE__, ret);

		if (ret > 0)
			ecam_hose[ecam].last_busno = ret;
	}

	for (pem = 0; pem < CONFIG_THUNDERX_RCS; pem++) {
		if (pem / RCS_PER_NODE >= atf_node_count())
			break;

		if (!rc_is_on(pem))
			continue;

		thunderx_read_rc_u32(pem, PCI_PRIMARY_BUS, &reg);

		sec_bus = (u8) (reg >> 8);
		sub_bus = (u8) (reg >> 16);

		pem_hose[pem].first_busno = sec_bus;
		pem_hose[pem].last_busno = sub_bus;

		pci_set_region(pem_hose[pem].regions + 0,
			       CONFIG_SYS_PCI_MEM_BUS,
			       pci_mem[pem],
			       CONFIG_SYS_PCI_MEM_SIZE,
			       PCI_REGION_MEM);

		pci_set_region(pem_hose[pem].regions + 1,
			       CONFIG_SYS_PCI_PREF_BUS,
			       pci_pref[pem],
			       CONFIG_SYS_PCI_PREF_SIZE,
			       PCI_REGION_MEM | PCI_REGION_PREFETCH);

		pci_set_region(pem_hose[pem].regions + 2,
			       CONFIG_SYS_PCI_IO_BUS,
			       pci_io[pem],
			       CONFIG_SYS_PCI_IO_SIZE,
			       PCI_REGION_IO);

		pem_hose[pem].region_count = 3;

		for (node = 0; node < atf_node_count(); node++) {
			baseaddr = (uintptr_t)node << 40;
			pci_set_region(pem_hose[pem].regions + 3 + node,
				       baseaddr, baseaddr,
				       baseaddr + atf_dram_size(node),
				       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

			pem_hose[pem].region_count++;
		}

		pem_hose[pem].priv_data = (void *)pem;

		pci_set_ops(&pem_hose[pem],
			    thunderx_rd_pem_u8,
			    thunderx_rd_pem_u16,
			    thunderx_rd_pem_u32,
			    thunderx_wr_pem_u8,
			    thunderx_wr_pem_u16,
			    thunderx_wr_pem_u32);

		pci_register_hose(&pem_hose[pem]);
			ret = pci_hose_scan(&pem_hose[pem]);

	}
}

int pci_config_fixed(struct pci_controller *hose, pci_dev_t dev)
{
	u16 vendor, device;
	int ret = 0;

	pci_hose_read_config_word(hose, dev, PCI_DEVICE_ID, &device);
	pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

	if (vendor == PCI_VENDOR_CAVIUM) {
		ret = 1;
	} else {
		debug("Non-fixed configuration at %02x:%02x.%02x\n",
		      PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
	}

	return ret;
}

int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
        return 0;
}

int pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 1;
}

