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

	printf("\nFault at: %lx\n", far);
	printf("ESR: %x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU...");
	return;
}

#define SLI_PER_NODE 2
#define ECAMS_PER_NODE 4
#define ECAM_GROUPS 2
#define RCS_PER_ECAM 3
#define RCS_PER_NODE 6
#define RCS_PER_SLI 3

static uintptr_t thunderx_mmcfg_addr(unsigned int rc, pci_dev_t dev, unsigned int reg)
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

int rc_is_on(unsigned int rc)
{
	union pemx_on pemx_on;

	if (rc > 5)
		return 0;

	pemx_on.u = readq(PEMX_ON(rc));

	return ((pemx_on.s.pemon != 0) && (pemx_on.s.pemoor != 0));
}

#if 0
const pci_dev_t RC_DEVS[] = {PCI_BDF(0, 0x10, 0),
			PCI_BDF(0, 0x12, 0), PCI_BDF(0, 0x14, 0)};

int thunderx_pci_to_rc(struct pci_controller *hose, pci_dev_t dev)
{
	int  = (uintptr_t)hose->priv_data;
	u32 b, d, f;
	int ret = -1;

	b = PCI_BUS(dev) - hose->first_busno;
	d = PCI_DEV(dev);
	f = PCI_FUNC(dev);

	if ((ecam & 1)) {
		switch (dev) {
		case PCI_BDF(0, 0x10, 0):
			ret = (ecam == 3) ? 3 : 0;
			break;
		case PCI_BDF(0, 0x12, 0):
			ret = (ecam == 3) ? 4 : 1;
			break;
		case PCI_BDF(0, 0x14, 0):
			ret = (ecam == 3) ? 5 : 2;
			break;
		}
	}

	return ret;
}
#endif

static int thunderx_read_rc_u32(int rc, int offset, u32 *val)
{
	union pemx_cfg_rd pemx_cfg_rd;
	pemx_cfg_rd.u = 0;

	if ((rc >= 0) && rc_is_on(rc)) {
		pemx_cfg_rd.s.addr = offset & ~0x3;
		writeq(pemx_cfg_rd.u, PEMX_CFG_RD(rc));
		pemx_cfg_rd.u = readq(PEMX_CFG_RD(rc));
		*val = pemx_cfg_rd.s.data;
	} else {
		*val = (u32)~0UL;
	}
	return 0;
}

#if 0

#define PCI_RC_WRITE(size)											\
static int thunderx_wr_rc_u##size(struct pci_controller *hose,		\
				pci_dev_t dev, int offset, u##size val)				\
{																	\
	union pemx_cfg_wr pemx_cfg_wr;									\
	int rc = thunderx_pci_to_rc(hose, dev);							\
	u32 tmp;														\
	u32 mask = (u##size)~0UL << 8 * (offset & 0x3);					\
																	\
	pemx_cfg_wr.u = 0;												\
																	\
	debug("%02x.%02x.%02x: u%d %x <- %x\n",							\
			PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev),				\
			size, offset, val);										\
	if ((rc >= 0) && rc_is_on(rc)) {								\
		pemx_cfg_wr.s.addr = offset & ~0x3;							\
		thunderx_rd_rc_u32(hose, dev, offset & ~0x3, &tmp);			\
																	\
		debug("0:tmp: %x, mask: %x, val: %x\n", tmp, mask, val);	\
		tmp &= ~mask;												\
		debug("1:tmp: %x, val: %x\n", tmp, val);					\
		tmp |= (u32)val << 8 * (offset & 0x3);						\
		debug("2:tmp: %x\n", tmp);									\
		pemx_cfg_wr.s.data = tmp;									\
		writeq(pemx_cfg_wr.u, PEMX_CFG_WR(rc));						\
	}																\
	return 0;														\
}

PCI_RC_WRITE(8)
PCI_RC_WRITE(16)
PCI_RC_WRITE(32)
#endif

#define PCI_PEM_READ(size)											\
static int thunderx_rd_pem_u##size(struct pci_controller *hose,		\
				pci_dev_t dev, int offset, u##size *val)			\
{																	\
	uintptr_t mmcfg_addr;											\
	u32 b, d, f;													\
	int rc = (uintptr_t)hose->priv_data;							\
																	\
	b = PCI_BUS(dev);												\
	d = PCI_DEV(dev);												\
	f = PCI_FUNC(dev);												\
																	\
	if (d == 1) {													\
		mmcfg_addr = thunderx_mmcfg_addr(rc,						\
										PCI_BDF(b, d, f), offset);	\
		*val = read##size(mmcfg_addr);								\
	} else {														\
		*val = (u##size)~0UL;										\
	}																\
																	\
	return 0;														\
}

PCI_PEM_READ(8)
PCI_PEM_READ(16)
PCI_PEM_READ(32)

#define PCI_PEM_WRITE(size)											\
static int thunderx_wr_pem_u##size(struct pci_controller *hose,		\
				pci_dev_t dev, int offset, u##size val)				\
{																	\
	uintptr_t mmcfg_addr;											\
	u32 b, d, f;													\
	int rc = (uintptr_t)hose->priv_data;							\
																	\
	b = PCI_BUS(dev);												\
	d = PCI_DEV(dev);												\
	f = PCI_FUNC(dev);												\
	debug("%02x.%02x.%02x: %x <- %x\n", b, d, f,					\
					offset, val);									\
																	\
	if (d == 1) {													\
		mmcfg_addr = thunderx_mmcfg_addr(rc,						\
								PCI_BDF(b, d, f), offset);			\
																	\
		write##size(val, mmcfg_addr);								\
	}																\
																	\
	return 0;														\
}

PCI_PEM_WRITE(8)
PCI_PEM_WRITE(16)
PCI_PEM_WRITE(32)

#define PCI_ECAM_READ(size)											\
static int thunderx_rd_ecam_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size *val)			\
{																	\
	u32 b, d, f;													\
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
	address.s.addr = offset;										\
	address.u += ECAMX_PF_BAR2(ecam);								\
																	\
	*val = read##size(address.u);									\
																	\
	return 0;														\
}

PCI_ECAM_READ(8)
PCI_ECAM_READ(16)
PCI_ECAM_READ(32)

#define PCI_ECAM_WRITE(size)										\
static int thunderx_wr_ecam_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size val)				\
{																	\
	u32 b, d, f;													\
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
	address.s.addr = offset;										\
	address.u += ECAMX_PF_BAR2(ecam);								\
																	\
	write##size(val, address.u);									\
																	\
	return 0;														\
}

PCI_ECAM_WRITE(8)
PCI_ECAM_WRITE(16)
PCI_ECAM_WRITE(32)

#if 0
#define PCI_READ(size)												\
static int thunderx_rd_config_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size *val)			\
{																	\
	int ecam = (uintptr_t)hose->priv_data;							\
	int rc = thunderx_pci_to_rc(hose, dev);							\
	u32 b = PCI_BUS(dev) - hose->first_busno;						\
																	\
	if (b > 0 && (ecam & 1)) {										\
		return thunderx_rd_ext_u##size(hose, dev, offset, val);		\
	} else {														\
		return thunderx_rd_ecam_u##size(hose, dev, offset, val);	\
	}																\
}

PCI_READ(8)
PCI_READ(16)
PCI_READ(32)

#define PCI_WRITE(size)												\
static int thunderx_wr_config_u##size(struct pci_controller *hose,	\
				pci_dev_t dev, int offset, u##size val)				\
{																	\
	int ecam = (uintptr_t)hose->priv_data;							\
	int rc = thunderx_pci_to_rc(hose, dev);							\
	u32 b = PCI_BUS(dev) - hose->first_busno;						\
																	\
	if (b > 0 && (ecam & 1)) {										\
		return thunderx_wr_ext_u##size(hose, dev, offset, val);		\
	} else {														\
		return thunderx_wr_ecam_u##size(hose, dev, offset, val);	\
	}																\
}

PCI_WRITE(8)
PCI_WRITE(16)
PCI_WRITE(32)
#endif

static struct pci_controller ecam_hose[ECAMS_PER_NODE];
static struct pci_controller pem_hose[RCS_PER_NODE];

void thunderx_sli_setup(unsigned long node)
{
	unsigned long sli, reg;
	union slix_s2m_regx_acc slix_s2m_regx_acc;

	for (sli = 0; sli < SLI_PER_NODE; sli++) {
		for (reg = 0; reg <= 255; reg++) {
			slix_s2m_regx_acc.u = readq(SLIX_S2M_REGX_ACC(sli, reg));
			slix_s2m_regx_acc.s.ba = 0;
			writeq(slix_s2m_regx_acc.u, SLIX_S2M_REGX_ACC(sli, reg));
		}
	}
}

void pci_init_board(void)
{
	long ecam, pem;
	u32 reg;
	u8 sec_bus, sub_bus;
#if 0
	u64 res_start, res_limit;
#endif
	u64 pci_mem[] = CONFIG_SYS_PCI_MEM_CPU;
	u64 pci_pref[] = CONFIG_SYS_PCI_PREF_CPU;
	u64 pci_io[] = CONFIG_SYS_PCI_IO_CPU;
	int ret;

	thunderx_sli_setup(0);

	for (ecam = 0; ecam < CONFIG_THUNDER_ECAMS; ecam++) {
		debug("%s: %d, ecam: %ld\n", __FUNCTION__, __LINE__, ecam);
		ecam_hose[ecam].first_busno = pci_last_busno() + 1;
		ecam_hose[ecam].last_busno = 0xff;

		ecam_hose[ecam].region_count = 0;
		ecam_hose[ecam].priv_data    = (void*)ecam;

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
		if (ret > 0) {
			ecam_hose[ecam].last_busno = ret;
		}
	}

	for (pem = 0; pem < RCS_PER_NODE; pem++) {
		if (!rc_is_on(pem))
			continue;

		thunderx_read_rc_u32(pem, PCI_PRIMARY_BUS, &reg);
		sec_bus = (u8)(reg >> 8);
		sub_bus = (u8)(reg >> 16);

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
		pem_hose[pem].priv_data    = (void*)pem;

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

#if 1
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
#endif

