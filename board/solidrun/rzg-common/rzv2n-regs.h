/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __RZV2N_REGS_H__
#define __RZV2N_REGS_H__


/* PFC */
#define PFC_BASE			0x10410000

#define PWPR				(PFC_BASE + 0x3C04)
#define PWPR_REGWE_A			BIT(6)
#define	PWPR_REGWE_B			BIT(5)

#define	P_2A				(PFC_BASE + 0x002A)
#define	PM_2A				(PFC_BASE + 0x0154)
#define	PMC_2A				(PFC_BASE + 0x022A)
#define PFC_PMC26			(PFC_BASE + 0x0226)
#define PFC_PFC26			(PFC_BASE + 0x0498)
#define PFC_PMC29			(PFC_BASE + 0x0229)
#define PFC_PFC29			(PFC_BASE + 0x04A4)
#define PFC_OSCBYPS                     (PFC_BASE + 0x3C00)

#define PFC_OEN				(PFC_BASE + 0x3C40)
#define PFC_OEN_OEN0			BIT(0)
#define PFC_OEN_OEN1			BIT(1)

#define	PMC_20				(PFC_BASE + 0x0220)
#define	PFC_20				(PFC_BASE + 0x0480)
#define PFC_PWPR                        (PFC_BASE + 0x3C04)

#define ICU_IPTSR_REG			0x10400060

/* CPG */
#define CPG_BASE			0x10420000
#define CPG_SSEL0			(CPG_BASE + 0x0300)
#define CPG_SSEL1			(CPG_BASE + 0x0304)
#define CPG_CLKON_ETH0			(CPG_BASE + 0x062C)
#define CPG_CLKMON_ETH0			(CPG_BASE + 0x0814)
#define CPG_RESET_ETH			(CPG_BASE + 0x092C)
#define CPG_RESETMON_ETH		(CPG_BASE + 0x0A14)

#define	CPG_CLKON_9			(CPG_BASE + 0x0624)
#define	CPG_RST_9			(CPG_BASE + 0x0924)
#define	CPG_RST_10			(CPG_BASE + 0x0928)

#define CPG_RST_USB			(CPG_BASE + 0x0928)
#define CPG_RSTMON4_USB			(CPG_BASE + 0x0A10)
#define CPG_RSTMON5_USB			(CPG_BASE + 0x0A14)
#define CPG_CLKON_USB			(CPG_BASE + 0x062C)
#define CPG_CLKMON_USB			(CPG_BASE + 0x0814)

/* USB */
#define USBPHY20_BASE			(0x15830000)
#define USBPHY20_RESET			(USBPHY20_BASE + 0x000u)

#define USB20_BASE			(0x15800000)
#define USBF_BASE			(0x15820000)

#define COMMCTRL			0x800
#define HcRhDescriptorA			0x048
#define LPSTS				0x102
#define USB2_PHY_UTMICTRL2		0xb04
#define USB2_PHY_RESET			0x000
#define USB2_PHY_OTGR			0x600

/* ADC */
#define SYS_ADC_CFG			0x10431600


#endif // __RZV2N_REGS_H__
