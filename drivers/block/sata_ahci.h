/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 * Terry Lv <r65388@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SATA_AHCI_H__
#define __SATA_AHCI_H__
#include <pci.h>

#define AHCI_MAX_CMD_SLOTS	32

/* Max host controller numbers */
#define SATA_HC_MAX_NUM		16
/* Max command queue depth per host controller */
#define SATA_HC_MAX_CMD		32
/* Max port number per host controller */
#define SATA_HC_MAX_PORT	16  /* TBD */

/* Generic Host Register */
#define SATA_HOST_CAP                0x00 /* host capabilities */
#define SATA_HOST_CTL                0x04 /* global host control */
#define SATA_HOST_IRQ_STAT           0x08 /* interrupt status */
#define SATA_HOST_PORTS_IMPL         0x0c /* bitmap of implemented ports */
#define SATA_HOST_VERSION            0x10 /* AHCI spec. version compliancy */
#define SATA_HOST_CCC_CTL            0x14 
#define SATA_HOST_EM_LOC             0x1C /* Enclosure Management location */
#define SATA_HOST_EM_CTL             0x20 /* Enclosure Management Control */
#define SATA_HOST_CAP2               0x24 /* host capabilities, extended */
#define SATA_HOST_OOBR               0xBC

/* HBA Capabilities Register */
#define SATA_HOST_CAP_S64A		0x80000000
#define SATA_HOST_CAP_SNCQ		0x40000000
#define SATA_HOST_CAP_SSNTF		0x20000000
#define SATA_HOST_CAP_SMPS		0x10000000
#define SATA_HOST_CAP_SSS		0x08000000
#define SATA_HOST_CAP_SALP		0x04000000
#define SATA_HOST_CAP_SAL		0x02000000
#define SATA_HOST_CAP_SCLO		0x01000000
#define SATA_HOST_CAP_ISS_MASK		0x00f00000	/* interface speed support */
#define SATA_HOST_CAP_ISS_OFFSET	20
#define SATA_HOST_CAP_SNZO		0x00080000
#define SATA_HOST_CAP_SAM		0x00040000
#define SATA_HOST_CAP_SPM		0x00020000
#define SATA_HOST_CAP_PMD		0x00008000
#define SATA_HOST_CAP_SSC		0x00004000
#define SATA_HOST_CAP_PSC		0x00002000
#define SATA_HOST_CAP_NCS		0x00001f00
#define SATA_HOST_CAP_CCCS		0x00000080
#define SATA_HOST_CAP_EMS		0x00000040
#define SATA_HOST_CAP_SXS		0x00000020
#define SATA_HOST_CAP_NP_MASK		0x0000001f

/* Global HBA Control Register */
#define SATA_HOST_GHC_AE	0x80000000 /* AHCI enable */
#define SATA_HOST_GHC_IE	0x00000002 /* Interrupt Enable */
#define SATA_HOST_GHC_HR	0x00000001 /* HBA reset; self-clear */

/* Interrupt Status Register */

/* Ports Implemented Register */

/* AHCI Version Register */
#define SATA_HOST_VS_MJR_MASK	0xffff0000
#define SATA_HOST_VS_MJR_OFFSET	16
#define SATA_HOST_VS_MJR_MNR	0x0000ffff

/* Command Completion Coalescing Control */
#define SATA_HOST_CCC_CTL_TV_MASK	0xffff0000
#define SATA_HOST_CCC_CTL_TV_OFFSET		16
#define SATA_HOST_CCC_CTL_CC_MASK	0x0000ff00
#define SATA_HOST_CCC_CTL_CC_OFFSET		8
#define SATA_HOST_CCC_CTL_INT_MASK	0x000000f8
#define SATA_HOST_CCC_CTL_INT_OFFSET	3
#define SATA_HOST_CCC_CTL_EN	0x00000001

/* Command Completion Coalescing Ports */

/* HBA Capabilities Extended Register */
#define SATA_HOST_CAP2_DESO		0x00000020
#define SATA_HOST_CAP2_SADM		0x00000010
#define SATA_HOST_CAP2_SDS		0x00000008
#define SATA_HOST_CAP2_APST		0x00000004
#define SATA_HOST_CAP2_NVMP		0x00000002
#define SATA_HOST_CAP2_BOH		0x00000001

/* BIST Activate FIS Register */
#define SATA_HOST_BISTAFR_NCP_MASK	0x0000ff00
#define SATA_HOST_BISTAFR_NCP_OFFSET	8
#define SATA_HOST_BISTAFR_PD_MASK	0x000000ff
#define SATA_HOST_BISTAFR_PD_OFFSET		0

#define SATA_FIS_BISTAFR_PATTERN_LTDP	0xF1	/* low transition density pattern */
#define SATA_FIS_BISTAFR_PATTERN_HTDP	0xB5	/* high transition density pattern */
#define SATA_FIS_BISTAFR_PATTERN_LFSCP	0xAB	/* low frequency spectral component pattern */
#define SATA_FIS_BISTAFR_PATTERN_SSOP	0x7F	/* simultaneous switching outputs pattern */
#define SATA_FIS_BISTAFR_PATTERN_MFTP	0x78	/* mid frequency test pattern */
#define SATA_FIS_BISTAFR_PATTERN_HFTP	0x4A	/* high frequency test pattern */
#define SATA_FIS_BISTAFR_PATTERN_LFTP	0x7E	/* low frequency test pattern */

#define SATA_FIS_BISTAFR_PD_FAREND_R	0x10	/* far-end retimed */
#define SATA_FIS_BISTAFR_PD_FAREND_T	0xC0	/* far-end transmit only */
#define SATA_FIS_BISTAFR_PD_FAREND_TS	0xE0	/* far-end transmit only with scrambler bypassed */


/* BIST Control Register */
#define SATA_HOST_BISTCR_FERLB		0x00100000
#define SATA_HOST_BISTCR_TXO		0x00040000
#define SATA_HOST_BISTCR_CNTCLR		0x00020000
#define SATA_HOST_BISTCR_NEALB		0x00010000
#define SATA_HOST_BISTCR_LLC_MASK	0x00000700
#define SATA_HOST_BISTCR_LLC_OFFSET	8
#define SATA_HOST_BISTCR_ERREN		0x00000040
#define SATA_HOST_BISTCR_FLIP		0x00000020
#define SATA_HOST_BISTCR_PV		0x00000010
#define SATA_HOST_BISTCR_PATTERN_MASK	0x0000000f
#define SATA_HOST_BISTCR_PATTERN_OFFSET	0

/* BIST FIS Count Register */

/* BIST Status Register */
#define SATA_HOST_BISTSR_FRAMERR_MASK	0x0000ffff
#define SATA_HOST_BISTSR_FRAMERR_OFFSET	0
#define SATA_HOST_BISTSR_BRSTERR_MASK	0x00ff0000
#define SATA_HOST_BISTSR_BRSTERR_OFFSET	16

/* BIST DWORD Error Count Register */

/* OOB Register*/
#define SATA_HOST_OOBR_WE		0x80000000
#define SATA_HOST_OOBR_cwMin_MASK	0x7f000000
#define SATA_HOST_OOBR_cwMAX_MASK	0x00ff0000
#define SATA_HOST_OOBR_ciMin_MASK	0x0000ff00
#define SATA_HOST_OOBR_ciMax_MASK	0x000000ff

/* Timer 1-ms Register */

/* Global Parameter 1 Register */
#define SATA_HOST_GPARAM1R_ALIGN_M	0x80000000
#define SATA_HOST_GPARAM1R_RX_BUFFER	0x40000000
#define SATA_HOST_GPARAM1R_PHY_DATA_MASK	0x30000000
#define SATA_HOST_GPARAM1R_PHY_RST	0x08000000
#define SATA_HOST_GPARAM1R_PHY_CTRL_MASK	0x07e00000
#define SATA_HOST_GPARAM1R_PHY_STAT_MASK	0x001f8000
#define SATA_HOST_GPARAM1R_LATCH_M	0x00004000
#define SATA_HOST_GPARAM1R_BIST_M	0x00002000
#define SATA_HOST_GPARAM1R_PHY_TYPE	0x00001000
#define SATA_HOST_GPARAM1R_RETURN_ERR	0x00000400
#define SATA_HOST_GPARAM1R_AHB_ENDIAN_MASK	0x00000300
#define SATA_HOST_GPARAM1R_S_HADDR	0X00000080
#define SATA_HOST_GPARAM1R_M_HADDR	0X00000040

/* Global Parameter 2 Register */
#define SATA_HOST_GPARAM2R_BIST_M	0x00040000
#define SATA_HOST_GPARAM2R_DEV_CP	0x00004000
#define SATA_HOST_GPARAM2R_DEV_MP	0x00002000
#define SATA_HOST_GPARAM2R_DEV_ENCODE_M	0x00001000
#define SATA_HOST_GPARAM2R_RXOOB_CLK_M	0x00000800
#define SATA_HOST_GPARAM2R_RXOOB_M	0x00000400
#define SATA_HOST_GPARAM2R_TX_OOB_M	0x00000200
#define SATA_HOST_GPARAM2R_RXOOB_CLK_MASK	0x000001ff

/* Port Parameter Register */
#define SATA_HOST_PPARAMR_TX_MEM_M	0x00000200
#define SATA_HOST_PPARAMR_TX_MEM_S	0x00000100
#define SATA_HOST_PPARAMR_RX_MEM_M	0x00000080
#define SATA_HOST_PPARAMR_RX_MEM_S	0x00000040
#define SATA_HOST_PPARAMR_TXFIFO_DEPTH_MASK	0x00000038
#define SATA_HOST_PPARAMR_RXFIFO_DEPTH_MASK	0x00000007

/* Test Register */
#define SATA_HOST_TESTR_PSEL_MASK	0x00070000
#define SATA_HOST_TESTR_TEST_IF		0x00000001

/* Port Register Descriptions */
/* Port# Command List Base Address Register */
#define SATA_PORT_CLB_CLB_MASK		0xfffffc00

/* Port# Command List Base Address Upper 32-Bits Register */

/* Port# FIS Base Address Register */
#define SATA_PORT_FB_FB_MASK		0xfffffff0

/* Port# FIS Base Address Upper 32-Bits Register */

/* Port# Interrupt Status Register */
#define SATA_PORT_IS_CPDS		0x80000000
#define SATA_PORT_IS_TFES		0x40000000
#define SATA_PORT_IS_HBFS		0x20000000
#define SATA_PORT_IS_HBDS		0x10000000
#define SATA_PORT_IS_IFS		0x08000000
#define SATA_PORT_IS_INFS		0x04000000
#define SATA_PORT_IS_OFS		0x01000000
#define SATA_PORT_IS_IPMS		0x00800000
#define SATA_PORT_IS_PRCS		0x00400000
#define SATA_PORT_IS_DMPS		0x00000080
#define SATA_PORT_IS_PCS		0x00000040
#define SATA_PORT_IS_DPS		0x00000020
#define SATA_PORT_IS_UFS		0x00000010
#define SATA_PORT_IS_SDBS		0x00000008
#define SATA_PORT_IS_DSS		0x00000004
#define SATA_PORT_IS_PSS		0x00000002
#define SATA_PORT_IS_DHRS		0x00000001

/* Port# Interrupt Enable Register */
#define SATA_PORT_IE_CPDE		0x80000000
#define SATA_PORT_IE_TFEE		0x40000000
#define SATA_PORT_IE_HBFE		0x20000000
#define SATA_PORT_IE_HBDE		0x10000000
#define SATA_PORT_IE_IFE		0x08000000
#define SATA_PORT_IE_INFE		0x04000000
#define SATA_PORT_IE_OFE		0x01000000
#define SATA_PORT_IE_IPME		0x00800000
#define SATA_PORT_IE_PRCE		0x00400000
#define SATA_PORT_IE_DMPE		0x00000080
#define SATA_PORT_IE_PCE		0x00000040
#define SATA_PORT_IE_DPE		0x00000020
#define SATA_PORT_IE_UFE		0x00000010
#define SATA_PORT_IE_SDBE		0x00000008
#define SATA_PORT_IE_DSE		0x00000004
#define SATA_PORT_IE_PSE		0x00000002
#define SATA_PORT_IE_DHRE		0x00000001

/* Port# Command Register */
#define SATA_PORT_CMD_ICC_MASK		0xf0000000
#define SATA_PORT_CMD_ASP		0x08000000
#define SATA_PORT_CMD_ALPE		0x04000000
#define SATA_PORT_CMD_DLAE		0x02000000
#define SATA_PORT_CMD_ATAPI		0x01000000
#define SATA_PORT_CMD_APSTE		0x00800000
#define SATA_PORT_CMD_ESP		0x00200000
#define SATA_PORT_CMD_CPD		0x00100000
#define SATA_PORT_CMD_MPSP		0x00080000
#define SATA_PORT_CMD_HPCP		0x00040000
#define SATA_PORT_CMD_PMA		0x00020000
#define SATA_PORT_CMD_CPS		0x00010000
#define SATA_PORT_CMD_CR		0x00008000
#define SATA_PORT_CMD_FR		0x00004000
#define SATA_PORT_CMD_MPSS		0x00002000
#define SATA_PORT_CMD_CCS_MASK		0x00001f00
#define SATA_PORT_CMD_FRE		0x00000010
#define SATA_PORT_CMD_CLO		0x00000008
#define SATA_PORT_CMD_POD		0x00000004
#define SATA_PORT_CMD_SUD		0x00000002
#define SATA_PORT_CMD_ST		0x00000001

/* Port# Task File Data Register */
#define SATA_PORT_TFD_ERR_MASK		0x0000ff00
#define SATA_PORT_TFD_STS_MASK		0x000000ff
#define SATA_PORT_TFD_STS_ERR		0x00000001
#define SATA_PORT_TFD_STS_DRQ		0x00000008
#define SATA_PORT_TFD_STS_BSY		0x00000080

/* Port# Signature Register */

/* Port# Serial ATA Status {SStatus} Register */
#define SATA_PORT_SSTS_IPM_MASK		0x00000f00
#define SATA_PORT_SSTS_SPD_MASK		0x000000f0
#define SATA_PORT_SSTS_DET_MASK		0x0000000f

/* Port# Serial ATA Control {SControl} Register */
#define SATA_PORT_SCTL_IPM_MASK		0x00000f00
#define SATA_PORT_SCTL_SPD_MASK		0x000000f0
#define SATA_PORT_SCTL_DET_MASK		0x0000000f

/* Port# Serial ATA Error {SError} Register */
#define SATA_PORT_SERR_DIAG_X		0x04000000
#define SATA_PORT_SERR_DIAG_F		0x02000000
#define SATA_PORT_SERR_DIAG_T		0x01000000
#define SATA_PORT_SERR_DIAG_S		0x00800000
#define SATA_PORT_SERR_DIAG_H		0x00400000
#define SATA_PORT_SERR_DIAG_C		0x00200000
#define SATA_PORT_SERR_DIAG_D		0x00100000
#define SATA_PORT_SERR_DIAG_B		0x00080000
#define SATA_PORT_SERR_DIAG_W		0x00040000
#define SATA_PORT_SERR_DIAG_I		0x00020000
#define SATA_PORT_SERR_DIAG_N		0x00010000
#define SATA_PORT_SERR_ERR_E		0x00000800
#define SATA_PORT_SERR_ERR_P		0x00000400
#define SATA_PORT_SERR_ERR_C		0x00000200
#define SATA_PORT_SERR_ERR_T		0x00000100
#define SATA_PORT_SERR_ERR_M		0x00000002
#define SATA_PORT_SERR_ERR_I		0x00000001

/* Port# Serial ATA Active {SActive} Register */

/* Port# Command Issue Register */

/* Port# Serial ATA Notification Register */

/* Port# DMA Control Register */
#define SATA_PORT_DMACR_RXABL_MASK	0x0000f000
#define SATA_PORT_DMACR_TXABL_MASK	0x00000f00
#define SATA_PORT_DMACR_RXTS_MASK	0x000000f0
#define SATA_PORT_DMACR_TXTS_MASK	0x0000000f

/* Port# PHY Control Register */

/* Port# PHY Status Register */

#define SATA_HC_CMD_HDR_ENTRY_SIZE	sizeof(struct cmd_hdr_entry)

/* DW0
*/
#define CMD_HDR_DI_CFL_MASK	0x0000001f
#define CMD_HDR_DI_CFL_OFFSET	0
#define CMD_HDR_DI_A			0x00000020
#define CMD_HDR_DI_W			0x00000040
#define CMD_HDR_DI_P			0x00000080
#define CMD_HDR_DI_R			0x00000100
#define CMD_HDR_DI_B			0x00000200
#define CMD_HDR_DI_C			0x00000400
#define CMD_HDR_DI_PMP_MASK	0x0000f000
#define CMD_HDR_DI_PMP_OFFSET	12
#define CMD_HDR_DI_PRDTL		0xffff0000
#define CMD_HDR_DI_PRDTL_OFFSET	16

/* prde_fis_len
*/
#define CMD_HDR_PRD_ENTRY_SHIFT	16
#define CMD_HDR_PRD_ENTRY_MASK	0x003f0000
#define CMD_HDR_FIS_LEN_SHIFT	2

/* attribute
*/
#define CMD_HDR_ATTR_RES	0x00000800 /* Reserved bit, should be 1 */
#define CMD_HDR_ATTR_VBIST	0x00000400 /* Vendor BIST */
/* Snoop enable for all descriptor */
#define CMD_HDR_ATTR_SNOOP	0x00000200
#define CMD_HDR_ATTR_FPDMA	0x00000100 /* FPDMA queued command */
#define CMD_HDR_ATTR_RESET	0x00000080 /* Reset - a SRST or device reset */
/* BIST - require the host to enter BIST mode */
#define CMD_HDR_ATTR_BIST	0x00000040
#define CMD_HDR_ATTR_ATAPI	0x00000020 /* ATAPI command */
#define CMD_HDR_ATTR_TAG	0x0000001f /* TAG mask */

#define FLAGS_DMA	0x00000000
#define FLAGS_FPDMA	0x00000001

#define SATA_FLAG_Q_DEP_MASK	0x0000000f
#define SATA_FLAG_WCACHE	0x00000100
#define SATA_FLAG_FLUSH		0x00000200
#define SATA_FLAG_FLUSH_EXT	0x00000400

#define READ_CMD	0
#define WRITE_CMD	1

#define AHCI_PCI_BAR            0x24
#define AHCI_MAX_SG             56 /* hardware max is 64K */
#define AHCI_CMD_SLOT_SZ        32
#define AHCI_MAX_CMD_SLOT       32
#define AHCI_RX_FIS_SZ          256
#define AHCI_CMD_TBL_HDR        0x80
#define AHCI_CMD_TBL_CDB        0x40
#define AHCI_CMD_TBL_SZ         AHCI_CMD_TBL_HDR + (AHCI_MAX_SG * 16)
#define AHCI_PORT_PRIV_DMA_SZ   (AHCI_CMD_SLOT_SZ * AHCI_MAX_CMD_SLOT + \
                                AHCI_CMD_TBL_SZ + AHCI_RX_FIS_SZ)
#define AHCI_CMD_ATAPI          (1 << 5)
#define AHCI_CMD_WRITE          (1 << 6)
#define AHCI_CMD_PREFETCH       (1 << 7)
#define AHCI_CMD_RESET          (1 << 8)
#define AHCI_CMD_CLR_BUSY       (1 << 10)

#define RX_FIS_D2H_REG          0x40    /* offset of D2H Register FIS data */

/* Global controller registers */
#define HOST_CAP                0x00 /* host capabilities */
#define HOST_CTL                0x04 /* global host control */
#define HOST_IRQ_STAT           0x08 /* interrupt status */
#define HOST_PORTS_IMPL         0x0c /* bitmap of implemented ports */
#define HOST_VERSION            0x10 /* AHCI spec. version compliancy */
#define HOST_CAP2               0x24 /* host capabilities, extended */

/* HOST_CTL bits */
#define HOST_RESET              (1 << 0)  /* reset controller; self-clear */
#define HOST_IRQ_EN             (1 << 1)  /* global IRQ enable */
#define HOST_AHCI_EN            (1 << 31) /* AHCI enabled */

/* Registers for each SATA port */
#define PORT_LST_ADDR           0x00 /* command list DMA addr */
#define PORT_LST_ADDR_HI        0x04 /* command list DMA addr hi */
#define PORT_FIS_ADDR           0x08 /* FIS rx buf addr */
#define PORT_FIS_ADDR_HI        0x0c /* FIS rx buf addr hi */
#define PORT_IRQ_STAT           0x10 /* interrupt status */
#define PORT_IRQ_MASK           0x14 /* interrupt enable/disable mask */
#define PORT_CMD                0x18 /* port command */
#define PORT_TFDATA             0x20 /* taskfile data */
#define PORT_SIG                0x24 /* device TF signature */
#define PORT_CMD_ISSUE          0x38 /* command issue */
#define PORT_SCR                0x28 /* SATA phy register block */
#define PORT_SCR_STAT           0x28 /* SATA phy register: SStatus */
#define PORT_SCR_CTL            0x2c /* SATA phy register: SControl */
#define PORT_SCR_ERR            0x30 /* SATA phy register: SError */
#define PORT_SCR_ACT            0x34 /* SATA phy register: SActive */

/* PORT_IRQ_{STAT,MASK} bits */
#define PORT_IRQ_COLD_PRES      (1 << 31) /* cold presence detect */
#define PORT_IRQ_TF_ERR         (1 << 30) /* task file error */
#define PORT_IRQ_HBUS_ERR       (1 << 29) /* host bus fatal error */
#define PORT_IRQ_HBUS_DATA_ERR  (1 << 28) /* host bus data error */
#define PORT_IRQ_IF_ERR         (1 << 27) /* interface fatal error */
#define PORT_IRQ_IF_NONFATAL    (1 << 26) /* interface non-fatal error */
#define PORT_IRQ_OVERFLOW       (1 << 24) /* xfer exhausted available S/G */
#define PORT_IRQ_BAD_PMP        (1 << 23) /* incorrect port multiplier */

#define PORT_IRQ_PHYRDY         (1 << 22) /* PhyRdy changed */
#define PORT_IRQ_DEV_ILCK       (1 << 7) /* device interlock */
#define PORT_IRQ_CONNECT        (1 << 6) /* port connect change status */
#define PORT_IRQ_SG_DONE        (1 << 5) /* descriptor processed */
#define PORT_IRQ_UNK_FIS        (1 << 4) /* unknown FIS rx'd */
#define PORT_IRQ_SDB_FIS        (1 << 3) /* Set Device Bits FIS rx'd */
#define PORT_IRQ_DMAS_FIS       (1 << 2) /* DMA Setup FIS rx'd */
#define PORT_IRQ_PIOS_FIS       (1 << 1) /* PIO Setup FIS rx'd */
#define PORT_IRQ_D2H_REG_FIS    (1 << 0) /* D2H Register FIS rx'd */

#define PORT_IRQ_FATAL          PORT_IRQ_TF_ERR | PORT_IRQ_HBUS_ERR     \
                                | PORT_IRQ_HBUS_DATA_ERR | PORT_IRQ_IF_ERR

#define DEF_PORT_IRQ            PORT_IRQ_FATAL | PORT_IRQ_PHYRDY        \
                                | PORT_IRQ_CONNECT | PORT_IRQ_SG_DONE   \
                                | PORT_IRQ_UNK_FIS | PORT_IRQ_SDB_FIS   \
                                | PORT_IRQ_DMAS_FIS | PORT_IRQ_PIOS_FIS \
                                | PORT_IRQ_D2H_REG_FIS

/* PORT_SCR_STAT bits */
#define PORT_SCR_STAT_DET_MASK  0x3
#define PORT_SCR_STAT_DET_COMINIT 0x1
#define PORT_SCR_STAT_DET_PHYRDY 0x3

/* PORT_CMD bits */
#define PORT_CMD_ATAPI          (1 << 24) /* Device is ATAPI */
#define PORT_CMD_LIST_ON        (1 << 15) /* cmd list DMA engine running */
#define PORT_CMD_FIS_ON         (1 << 14) /* FIS DMA engine running */
#define PORT_CMD_FIS_RX         (1 << 4) /* Enable FIS receive DMA engine */
#define PORT_CMD_CLO            (1 << 3) /* Command list override */
#define PORT_CMD_POWER_ON       (1 << 2) /* Power up device */
#define PORT_CMD_SPIN_UP        (1 << 1) /* Spin up device */
#define PORT_CMD_START          (1 << 0) /* Enable port DMA engine */

#define PORT_CMD_ICC_ACTIVE     (0x1 << 28) /* Put i/f in active state */
#define PORT_CMD_ICC_PARTIAL    (0x2 << 28) /* Put i/f in partial state */
#define PORT_CMD_ICC_SLUMBER    (0x6 << 28) /* Put i/f in slumber state */

#define AHCI_MAX_PORTS          32

#define ATA_FLAG_SATA           (1 << 3)
#define ATA_FLAG_NO_LEGACY      (1 << 4) /* no legacy mode check */
#define ATA_FLAG_MMIO           (1 << 6) /* use MMIO, not PIO */
#define ATA_FLAG_SATA_RESET     (1 << 7) /* (obsolete) use COMRESET */
#define ATA_FLAG_PIO_DMA        (1 << 8) /* PIO cmds via DMA */
#define ATA_FLAG_NO_ATAPI       (1 << 11) /* No ATAPI support */

#define LOWER32(val)	(u32)((u64)(val) & 0xffffffff)
#define UPPER32(val)    (u32)(((u64)(val) & 0xffffffff00000000) >> 32) 

struct ahci_cmd_hdr {
        u32     opts;
        u32     status;  	/* PRD byte Count */
        u32     tbl_addr;
        u32     tbl_addr_hi;
        u32     reserved[4];
};

struct ahci_sg {	/* PRDT: Physical Region Descript or Table */
        u32     addr;
        u32     addr_hi;
        u32     reserved;
        u32     flags_size;
};

struct ahci_ioports {
        u64     cmd_addr;
        u64     scr_addr;
        u64     port_mmio;
        struct ahci_cmd_hdr     *cmd_slot;
        struct ahci_sg          *cmd_tbl_sg;
        u64     cmd_tbl;
        u64     rx_fis;
};

struct ahci_probe_ent {
        pci_dev_t       dev;
        struct ahci_ioports     port[AHCI_MAX_PORTS];
        u32     n_ports;
        u32     hard_port_no;
        u32     host_flags;
        u32     host_set_flags;
        u64     mmio_base;
        u32     pio_mask;
        u32     udma_mask;
        u32     flags;
        u32     cap;       /* cache of HOST_CAP register */
        u32     port_map;  /* cache of HOST_PORTS_IMPL reg */
        u32     link_port_map; /*linkup port map*/
};

 
#endif /* __SATA_AHCI_H__ */ 
