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
#include <asm/errno.h>
#include <cavm-csr.h>
#include <cavium/atf.h>

#define PCI_VENDOR_CAVIUM 0x177d
#define PCI_DEV_CAVIUM_RC 0xa100

#define PCI_CAP_EA 0x14

#define RTARGET_MASK 0x1f;
#define OPCODE_SIZE 4;

#define ECAMS_PER_NODE 4
#define ECAM_GROUPS 2
#define RCS_PER_NODE 6
#define RCS_PER_SLI 3

struct resource {
	uintptr_t start;
	uintptr_t end;

	unsigned long type;
};

struct thunderx_ecam {
	int node;
	int ecam;
	bool present;

	uintptr_t baseaddr;

	struct resource memory;
	struct resource buses;
	struct resource pref_mem;
	struct resource ioports;
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

	uint8_t Rt;

	/* get faulting address */

#ifdef DEBUG
	show_regs(pt_regs);
#endif

	for (ecam = 0; ecam < CONFIG_THUNDERX_ECAMS; ecam++) {
		if (!thunderx_ecam[ecam].present)
			break;

		baseaddr = thunderx_ecam[ecam].baseaddr;
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
	if (!ecam->present) {							\
		*val = (u##size)~0UL;						\
		return -ENODEV;							\
	}									\
										\
	b = PCI_BUS(dev) - hose->first_busno;					\
	d = PCI_DEV(dev);							\
	f = PCI_FUNC(dev);							\
										\
	address.u = ecam->baseaddr;						\
	address.s.func = (d << 3) | (f << 0);					\
	address.s.bus  = b;							\
	address.s.addr = offset;						\
										\
	*val = read##size(address.u);						\
	/* debug("%d.%d::%02x.%02x.%02x: u%d %x -> %x\n",			\
		ecam->node, ecam->ecam, b, d, f, size, offset, *val); */	\
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
	if (!ecam->present)							\
		return -ENODEV;							\
										\
	b = PCI_BUS(dev) - hose->first_busno;					\
	d = PCI_DEV(dev);							\
	f = PCI_FUNC(dev);							\
	dev = PCI_BDF(b, d, f);							\
	/* debug("%d.%d::%02x.%02x.%02x: u%d %x <- %x\n",			\
		ecam->node, ecam->ecam, b, d, f, size, offset, val); */		\
										\
	address.u = ecam->baseaddr;						\
	address.s.func = (d << 3) | (f << 0);					\
	address.s.bus  = b;							\
	address.s.addr = offset;						\
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

static int rc_is_on(unsigned int rc)
{
	union pemx_on pemx_on;
	unsigned int node, pem;

	node = rc / RCS_PER_NODE;
	pem = rc % RCS_PER_NODE;

	if (rc > 5)
		return 0;

	pemx_on.u = readq(CSR_PA(node, PEMX_ON(pem)));

	debug("node: %d, pem: %d, pemx_on.u: %lx\n",
	      node, pem, pemx_on.u);

	return ((pemx_on.s.pemon != 0) && (pemx_on.s.pemoor != 0));
}

static int thunderx_read_rc_u32(int rc, int offset, u32 * val)
{
	union pemx_cfg_rd pemx_cfg_rd;
	uintptr_t address;
	unsigned int node, pem;

	pemx_cfg_rd.u = 0;

	node = rc / RCS_PER_NODE;
	pem = rc % RCS_PER_NODE;

	address = CSR_PA(node, PEMX_CFG_RD(pem));

	debug("node: %d, pem: %d, address: %lx\n", node, pem, address);

	*val = (u32) ~0UL;

	if (node >= atf_node_count())
		return -ENODEV;

	if ((pem >= 0) && rc_is_on(pem)) {
		pemx_cfg_rd.s.addr = offset & ~0x3;
		writeq(pemx_cfg_rd.u, address);
		pemx_cfg_rd.u = readq(address);
		*val = pemx_cfg_rd.s.data;
	}

	return 0;
}

#define PCIE_RC_LNK_CAP_STA (32 << 2)
#define PCIE_RC_LNK_STA_DLLA (1 << 29)

int rc_is_ready(unsigned int rc)
{
	u32 lnk_sta;
	int res;

	if (!rc_is_on(rc))
		return 0;

	res = thunderx_read_rc_u32(rc, PCIE_RC_LNK_CAP_STA, &lnk_sta);

	debug("%s: %d, lnk_sta: %x\n", __FUNCTION__, __LINE__, lnk_sta);

	return (res >=0) && (lnk_sta & PCIE_RC_LNK_STA_DLLA);
}

void pci_init_board(void)
{
	long dev, pem;
	struct thunderx_ecam *ecam;
	struct pci_controller *hose;
	u32 reg;
	u8 sec_bus, sub_bus;

	u64 pci_mem[] = CONFIG_SYS_PCI_MEM_CPU;
	u64 pci_pref[] = CONFIG_SYS_PCI_PREF_CPU;
	u64 pci_io[] = CONFIG_SYS_PCI_IO_CPU;
	uintptr_t baseaddr;
	int ret, node;

	for (dev = 0; dev < CONFIG_THUNDERX_ECAMS; dev++) {
		ecam = &thunderx_ecam[dev];
		hose = &ecam_hose[dev];

		ecam->node = dev / ECAMS_PER_NODE;
		ecam->ecam = dev % ECAMS_PER_NODE;

		if (ecam->node >= atf_node_count()) {
			ecam->present = false;
			break;
		}

		ecam->baseaddr = CSR_PA(ecam->node,
					ECAMX_PF_BAR2(ecam->ecam));
		ecam->present = true;

		debug("%s: %d, node: %ld, dev: %ld, baseaddr: %p\n",
		      __FUNCTION__, __LINE__, node, dev, (void *)ecam->baseaddr);

		hose->first_busno = pci_last_busno() + 1;
		hose->last_busno = 0xff;

		hose->region_count = 0;
		hose->priv_data = ecam;

		debug("%s: %d, ecam: %p, pci_last_busno(): %d\n",
		      __FUNCTION__, __LINE__, hose->priv_data, pci_last_busno());


		pci_set_ops(hose,
			    thunderx_rd_ecam_u8,
			    thunderx_rd_ecam_u16,
			    thunderx_rd_ecam_u32,
			    thunderx_wr_ecam_u8,
			    thunderx_wr_ecam_u16,
			    thunderx_wr_ecam_u32);

		pci_register_hose(hose);

		ret = pci_hose_scan(hose);

		debug("%s: %d, ret: %d\n", __FUNCTION__, __LINE__, ret);

		if (ret > 0)
			hose->last_busno = ret;

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
	int res = 0;

	res = pci_hose_find_capability(hose, dev, PCI_CAP_ID_EA);

	if (res)
		return 1;

	pci_hose_read_config_word(hose, dev, PCI_DEVICE_ID, &device);
	pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

	if (vendor == PCI_VENDOR_CAVIUM) {
		res = 1;
	} else {
		debug("Non-fixed configuration at %02x:%02x.%02x\n",
		      PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
	}

	return res;
}

int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
        return 0;
}

int pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 1;
}

static unsigned long pci_ea_set_flags(struct pci_controller *hose,
				      pci_dev_t dev, u8 prop)
{
	unsigned long flags = 0;
	switch (prop) {
	case PCI_EA_P_MEM:
	case PCI_EA_P_VIRT_MEM:
		flags = PCI_REGION_MEM;
		break;
	case PCI_EA_P_MEM_PREFETCH:
	case PCI_EA_P_VIRT_MEM_PREFETCH:
		flags = PCI_REGION_MEM | PCI_REGION_PREFETCH;
		break;
	case PCI_EA_P_IO:
		flags = PCI_REGION_IO;
		break;
	default:
		return 0;
	}

	return flags;
}
#if 0
static struct resource *pci_ea_get_resource(struct pci_dev *dev, u8 bei,
					    u8 prop)
{
	if (bei <= PCI_EA_BEI_BAR5 && prop <= PCI_EA_P_IO)
		return &dev->resource[bei];
#ifdef CONFIG_PCI_IOV
	else if (bei >= PCI_EA_BEI_VF_BAR0 && bei <= PCI_EA_BEI_VF_BAR5 &&
		 (prop == PCI_EA_P_VIRT_MEM ||
		  prop == PCI_EA_P_VIRT_MEM_PREFETCH))
		return &dev->resource[PCI_IOV_RESOURCES +
				      bei - PCI_EA_BEI_VF_BAR0];
#endif
	else if (bei == PCI_EA_BEI_ROM)
		return &dev->resource[PCI_ROM_RESOURCE];
	else
		return NULL;
}
#endif

/* Read an Enhanced Allocation (EA) entry */
static int pci_ea_read(struct pci_controller *hose,
		       pci_dev_t dev, int offset)
{
	struct resource r;
	struct resource *res = &r;
	int ent_offset = offset;
	int ent_size;
	size_t start;
	size_t end;
	unsigned long flags;
	u32 dw0;
	u32 base;
	u32 max_offset;
	u8 prop;
	bool support_64 = (sizeof(uintptr_t) >= 8);

	debug("%s: %d: dev: %x, ent_offset: %x\n", 
	       __FUNCTION__, __LINE__, dev, ent_offset);

	pci_hose_read_config_dword(hose, dev,
				   ent_offset, &dw0);

	debug("%s: %d: dw0: %lx\n", __FUNCTION__, __LINE__, dw0);

	
	ent_offset += sizeof(u32);

	/* Entry size field indicates DWORDs after 1st */
	ent_size = ((dw0 & PCI_EA_ES) + 1) * sizeof(u32);

	if (!(dw0 & PCI_EA_ENABLE)) /* Entry not enabled */
		goto out;

	prop = PCI_EA_PP(dw0);

	debug("EA property: %x\n", prop);

	/*
	 * If the Property is in the reserved range, try the Secondary
	 * Property instead.
	 */
	if (prop > PCI_EA_P_BRIDGE_IO && prop < PCI_EA_P_MEM_RESERVED)
		prop = PCI_EA_SP(dw0);
	if (prop > PCI_EA_P_BRIDGE_IO)
		goto out;

	debug("EA property: %x\n", prop);

#if 0
	res = pci_ea_get_resource(dev, PCI_EA_BEI(dw0), prop);
	if (!res) {
		dev_err(&dev->dev, "Unsupported EA entry BEI: %u\n",
			PCI_EA_BEI(dw0));
		goto out;
	}
#endif

	flags = pci_ea_set_flags(hose, dev, prop);

	/* Read Base */
	pci_hose_read_config_dword(hose, dev, ent_offset, &base);
	start = (base & PCI_EA_FIELD_MASK);
	ent_offset += sizeof(u32);

	/* Read MaxOffset */
	pci_hose_read_config_dword(hose, dev, ent_offset, &max_offset);
	ent_offset += sizeof(u32);

	/* Read Base MSBs (if 64-bit entry) */
	if (base & PCI_EA_IS_64) {
		u32 base_upper;

		pci_hose_read_config_dword(hose, dev,
					   ent_offset, &base_upper);
		ent_offset += sizeof(u32);

		/* entry starts above 32-bit boundary, can't use */
		if (!support_64 && base_upper)
			goto out;

		if (support_64)
			start |= ((u64)base_upper << 32);
	}

	debug("EA (%u,%u) start = %pa\n", PCI_EA_BEI(dw0), prop, &start);

	end = start + (max_offset | 0x03);

	/* Read MaxOffset MSBs (if 64-bit entry) */
	if (max_offset & PCI_EA_IS_64) {
		u32 max_offset_upper;

		pci_hose_read_config_dword(hose, dev,
				      ent_offset, &max_offset_upper);
		ent_offset += sizeof(u32);

		/* entry too big, can't use */
		if (!support_64 && max_offset_upper)
			goto out;

		if (support_64)
			end += ((u64)max_offset_upper << 32);
	}

	debug("EA (%u,%u) end = %pa\n", PCI_EA_BEI(dw0), prop, &end);

	if (end < start) {
		printf("EA Entry crosses address boundary\n");
		goto out;
	}

	if (ent_size != ent_offset - offset) {
		printf("EA Entry Size (%d) does not match length read (%d)\n",
			ent_size, ent_offset - offset);
		goto out;
	}

	res->start = start;
	res->end = end;
	res->type = flags;

out:
	return offset + ent_size;
}

/* Enhanced Allocation Initalization */
int pci_hose_ea_init(struct pci_controller *hose, pci_dev_t dev)
{
	int ea;
	u8 num_ent;
	int offset;
	u8 hdr_type;
	int i;

	/* find PCI EA capability in list */
	ea = pci_hose_find_capability(hose, dev, PCI_CAP_ID_EA);
	if (!ea)
		return 0;

	/* determine the number of entries */
	pci_hose_read_config_byte(hose, dev, ea + PCI_EA_NUM_ENT, &num_ent);
	num_ent &= PCI_EA_NUM_ENT_MASK;

	offset = ea + PCI_EA_FIRST_ENT;

	pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &hdr_type);
	/* Skip DWORD 2 for type 1 functions */
	if (hdr_type == PCI_HEADER_TYPE_BRIDGE)
		offset += sizeof(u32);

	debug("%s: %d: offset: %lx\n", __FUNCTION__, __LINE__, offset);

	/* parse each EA entry */
	for (i = 0; i < num_ent; ++i)
		offset = pci_ea_read(hose, dev, offset);

	return 1;
}


int pci_postscan_config(struct pci_controller *hose, pci_dev_t dev)
{
	struct thunderx_ecam *ecam = hose->priv_data;
	u32 reg;
	u64 bar_start;
	u64 bar_size;
	int bar, res;

	res = pci_hose_ea_init(hose, dev);

	debug("%s: %d: cap: %lx\n", __FUNCTION__, __LINE__, res);

	for (bar = 0; bar <= 5; bar++) {
		bar_start = pci_read_bar32(hose, dev, bar);
		if (reg & PCI_BASE_ADDRESS_MEM_TYPE_64) {
			bar++;
			bar_start |= ((u64)pci_read_bar32(hose, dev, bar) << 32);
			bar_size = 1 << ffs(bar_start);
		}

		debug("%s: %d: bar_start: %lx, bar_size: %lx\n",
		       __FUNCTION__, __LINE__, bar_start, bar_size);
	}

	return 0;
}
