// SPDX-License-Identifier: GPL-2.0+
// Cadence XSPI flash controller driver
// Copyright (C) 2020-21 Cadence

#include <dm.h>
#include <spi.h>
#include <spi-mem.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/sizes.h>
#include <linux/bitfield.h>
#include <linux/log2.h>
#include <asm/arch/board.h>
#include <linux/iopoll.h>

#define CDNS_XSPI_MAGIC_NUM_VALUE	0x6522
#define CDNS_XSPI_MAX_BANKS		8
#define CDNS_XSPI_NAME			"cadence-xspi"

/*
 * Note: below are additional auxiliary registers to
 * configure XSPI controller pin-strap settings
 */

/* PHY DQ timing register */
#define CDNS_XSPI_CCP_PHY_DQ_TIMING		0x0000

/* PHY DQS timing register */
#define CDNS_XSPI_CCP_PHY_DQS_TIMING		0x0004

/* PHY gate loopback control register */
#define CDNS_XSPI_CCP_PHY_GATE_LPBCK_CTRL	0x0008

/* PHY DLL slave control register */
#define CDNS_XSPI_CCP_PHY_DLL_SLAVE_CTRL	0x0010

/* DLL PHY control register */
#define CDNS_XSPI_DLL_PHY_CTRL			0x1034

/* Command registers */
#define CDNS_XSPI_CMD_REG_0			0x0000
#define CDNS_XSPI_CMD_REG_1			0x0004
#define CDNS_XSPI_CMD_REG_2			0x0008
#define CDNS_XSPI_CMD_REG_3			0x000C
#define CDNS_XSPI_CMD_REG_4			0x0010
#define CDNS_XSPI_CMD_REG_5			0x0014

/* Command status registers */
#define CDNS_XSPI_CMD_STATUS_REG		0x0044

/* Controller status register */
#define CDNS_XSPI_CTRL_STATUS_REG		0x0100
#define CDNS_XSPI_INIT_COMPLETED		BIT(16)
#define CDNS_XSPI_INIT_LEGACY			BIT(9)
#define CDNS_XSPI_INIT_FAIL			BIT(8)
#define CDNS_XSPI_CTRL_BUSY			BIT(7)
#define CDNS_XSPI_GCMD_BUSY                     BIT(3)

/* Controller interrupt status register */
#define CDNS_XSPI_INTR_STATUS_REG		0x0110
#define CDNS_XSPI_STIG_DONE			BIT(23)
#define CDNS_XSPI_SDMA_ERROR			BIT(22)
#define CDNS_XSPI_SDMA_TRIGGER			BIT(21)
#define CDNS_XSPI_CMD_IGNRD_EN			BIT(20)
#define CDNS_XSPI_DDMA_TERR_EN			BIT(18)
#define CDNS_XSPI_CDMA_TREE_EN			BIT(17)
#define CDNS_XSPI_CTRL_IDLE_EN			BIT(16)

#define CDNS_XSPI_TRD_COMP_INTR_STATUS		0x0120
#define CDNS_XSPI_TRD_ERR_INTR_STATUS		0x0130
#define CDNS_XSPI_TRD_ERR_INTR_EN		0x0134

/* Controller interrupt enable register */
#define CDNS_XSPI_INTR_ENABLE_REG		0x0114
#define CDNS_XSPI_INTR_EN			BIT(31)
#define CDNS_XSPI_STIG_DONE_EN			BIT(23)
#define CDNS_XSPI_SDMA_ERROR_EN			BIT(22)
#define CDNS_XSPI_SDMA_TRIGGER_EN		BIT(21)

#define CDNS_XSPI_INTR_MASK (CDNS_XSPI_INTR_EN | \
	CDNS_XSPI_STIG_DONE_EN  | \
	CDNS_XSPI_SDMA_ERROR_EN | \
	CDNS_XSPI_SDMA_TRIGGER_EN)

/* Controller config register */
#define CDNS_XSPI_CTRL_CONFIG_REG		0x0230
#define CDNS_XSPI_CTRL_WORK_MODE		GENMASK(6, 5)

#define CDNS_XSPI_WORK_MODE_DIRECT		0
#define CDNS_XSPI_WORK_MODE_STIG		1
#define CDNS_XSPI_WORK_MODE_ACMD		3

/* SDMA trigger transaction registers */
#define CDNS_XSPI_SDMA_SIZE_REG			0x0240
#define CDNS_XSPI_SDMA_TRD_INFO_REG		0x0244
#define CDNS_XSPI_SDMA_DIR			BIT(8)

/* Controller features register */
#define CDNS_XSPI_CTRL_FEATURES_REG		0x0F04
#define CDNS_XSPI_NUM_BANKS			GENMASK(25, 24)
#define CDNS_XSPI_DMA_DATA_WIDTH		BIT(21)
#define CDNS_XSPI_NUM_THREADS			GENMASK(3, 0)

/* Controller version register */
#define CDNS_XSPI_CTRL_VERSION_REG		0x0F00
#define CDNS_XSPI_MAGIC_NUM			GENMASK(31, 16)
#define CDNS_XSPI_CTRL_REV			GENMASK(7, 0)

/* STIG Profile 1.0 instruction fields (split into registers) */
#define CDNS_XSPI_CMD_INSTR_TYPE		GENMASK(6, 0)
#define CDNS_XSPI_CMD_P1_R1_ADDR0		GENMASK(31, 24)
#define CDNS_XSPI_CMD_P1_R2_ADDR1		GENMASK(7, 0)
#define CDNS_XSPI_CMD_P1_R2_ADDR2		GENMASK(15, 8)
#define CDNS_XSPI_CMD_P1_R2_ADDR3		GENMASK(23, 16)
#define CDNS_XSPI_CMD_P1_R2_ADDR4		GENMASK(31, 24)
#define CDNS_XSPI_CMD_P1_R3_ADDR5		GENMASK(7, 0)
#define CDNS_XSPI_CMD_P1_R3_CMD			GENMASK(23, 16)
#define CDNS_XSPI_CMD_P1_R3_NUM_ADDR_BYTES	GENMASK(30, 28)
#define CDNS_XSPI_CMD_P1_R4_ADDR_IOS		GENMASK(1, 0)
#define CDNS_XSPI_CMD_P1_R4_CMD_IOS		GENMASK(9, 8)
#define CDNS_XSPI_CMD_P1_R4_BANK		GENMASK(14, 12)

/* STIG data sequence instruction fields (split into registers) */
#define CDNS_XSPI_CMD_DSEQ_R2_DCNT_L		GENMASK(31, 16)
#define CDNS_XSPI_CMD_DSEQ_R3_DCNT_H		GENMASK(15, 0)
#define CDNS_XSPI_CMD_DSEQ_R3_NUM_OF_DUMMY	GENMASK(25, 20)
#define CDNS_XSPI_CMD_DSEQ_R4_BANK		GENMASK(14, 12)
#define CDNS_XSPI_CMD_DSEQ_R4_DATA_IOS		GENMASK(9, 8)
#define CDNS_XSPI_CMD_DSEQ_R4_DIR		BIT(4)

/* STIG command status fields */
#define CDNS_XSPI_CMD_STATUS_COMPLETED		BIT(15)
#define CDNS_XSPI_CMD_STATUS_FAILED		BIT(14)
#define CDNS_XSPI_CMD_STATUS_DQS_ERROR		BIT(3)
#define CDNS_XSPI_CMD_STATUS_CRC_ERROR		BIT(2)
#define CDNS_XSPI_CMD_STATUS_BUS_ERROR		BIT(1)
#define CDNS_XSPI_CMD_STATUS_INV_SEQ_ERROR	BIT(0)

#define CDNS_XSPI_STIG_DONE_FLAG		BIT(0)
#define CDNS_XSPI_TRD_STATUS			0x0104

#define MODE_NO_OF_BYTES			GENMASK(25, 24)
#define MODEBYTES_COUNT                         1

/* Helper macros for filling command registers */
#define CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_1(op, data_phase) ( \
	FIELD_PREP(CDNS_XSPI_CMD_INSTR_TYPE, (data_phase) ? \
		CDNS_XSPI_STIG_INSTR_TYPE_1 : CDNS_XSPI_STIG_INSTR_TYPE_0) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R1_ADDR0, (op)->addr.val & 0xff))

#define CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_2(op) ( \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R2_ADDR1, ((op)->addr.val >> 8)  & 0xFF) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R2_ADDR2, ((op)->addr.val >> 16) & 0xFF) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R2_ADDR3, ((op)->addr.val >> 24) & 0xFF) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R2_ADDR4, ((op)->addr.val >> 32) & 0xFF))

#define CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_3(op, modebytes) ( \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R3_ADDR5, ((op)->addr.val >> 40) & 0xFF) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R3_CMD, (op)->cmd.opcode) | \
	FIELD_PREP(MODE_NO_OF_BYTES, (modebytes)) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R3_NUM_ADDR_BYTES, (op)->addr.nbytes))

#define CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_4(op, chipsel) ( \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R4_ADDR_IOS, ilog2((op)->addr.buswidth)) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R4_CMD_IOS, ilog2((op)->cmd.buswidth)) | \
	FIELD_PREP(CDNS_XSPI_CMD_P1_R4_BANK, chipsel))

#define CDNS_XSPI_CMD_FLD_DSEQ_CMD_1(op) \
	FIELD_PREP(CDNS_XSPI_CMD_INSTR_TYPE, CDNS_XSPI_STIG_INSTR_TYPE_DATA_SEQ)

#define CDNS_XSPI_CMD_FLD_DSEQ_CMD_2(op) \
	FIELD_PREP(CDNS_XSPI_CMD_DSEQ_R2_DCNT_L, (op)->data.nbytes & 0xFFFF)

#define CDNS_XSPI_CMD_FLD_DSEQ_CMD_3(op, dummycnt) ( \
	FIELD_PREP(CDNS_XSPI_CMD_DSEQ_R3_DCNT_H, \
		  ((op)->data.nbytes >> 16) & 0xffff) | \
	FIELD_PREP(CDNS_XSPI_CMD_DSEQ_R3_NUM_OF_DUMMY, \
		  (op)->dummy.buswidth != 0 ? \
		  (((dummycnt) * 8) / (op)->dummy.buswidth) : \
		  0))

#define CDNS_XSPI_CMD_FLD_DSEQ_CMD_4(op, chipsel) ( \
	FIELD_PREP(CDNS_XSPI_CMD_DSEQ_R4_BANK, chipsel) | \
	FIELD_PREP(CDNS_XSPI_CMD_DSEQ_R4_DATA_IOS, \
		ilog2((op)->data.buswidth)) | \
	FIELD_PREP(CDNS_XSPI_CMD_DSEQ_R4_DIR, \
		((op)->data.dir == SPI_MEM_DATA_IN) ? \
		CDNS_XSPI_STIG_CMD_DIR_READ : CDNS_XSPI_STIG_CMD_DIR_WRITE))

/* clock config register */
#define CDNS_XSPI_CLK_CTRL_REG		      0x4020
#define CDNS_XSPI_CLK_ENABLE                  BIT(0)
#define CDNS_XSPI_CLK_DIV                     GENMASK(4, 1)

/* Clock macros */
#define CDNS_XSPI_CLOCK_IO_HZ 800000000
#define CDNS_XSPI_CLOCK_DIVIDED(div) ((CDNS_XSPI_CLOCK_IO_HZ) / (div))

#define CDNS_XSPI_CTRL_WORK_MODE_STIG           0x01

/*PHY default values*/
#define REGS_DLL_PHY_CTRL	  0x00000707
#define CTB_RFILE_PHY_CTRL	  0x00004000
#define RFILE_PHY_TSEL		  0x00000000
#define RFILE_PHY_DQ_TIMING	  0x00000101
#define RFILE_PHY_DQS_TIMING	  0x00700404
#define RFILE_PHY_GATE_LPBK_CTRL  0x00200030
#define RFILE_PHY_DLL_MASTER_CTRL 0x00800000
#define RFILE_PHY_DLL_SLAVE_CTRL  0x0000ff01

/*PHY config rtegisters*/
#define CDNS_XSPI_RF_MINICTRL_REGS_DLL_PHY_CTRL			0x1034
#define CDNS_XSPI_PHY_CTB_RFILE_PHY_CTRL			0x2080
#define CDNS_XSPI_PHY_CTB_RFILE_PHY_TSEL			0x2084
#define CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DQ_TIMING		0x2000
#define CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DQS_TIMING		0x2004
#define CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_GATE_LPBK_CTRL	0x2008
#define CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DLL_MASTER_CTRL	0x200c
#define CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DLL_SLAVE_CTRL	0x2010
#define CDNS_XSPI_DATASLICE_RFILE_PHY_DLL_OBS_REG_0		0x201c

#define CDNS_XSPI_DLL_RST_N BIT(24)
#define CDNS_XSPI_DLL_LOCK  BIT(0)

#if IS_ENABLED(CONFIG_CADENCE_XSPI_WORKAROUND_GPIO)
	#define SPI1_CLK 38
	#define SPI1_CS0 40
	#define SPI1_CS1 41
	#define SPI1_IO0 30
	#define SPI1_IO1 31

	#define SPI0_CLK 24
	#define SPI0_CS0 26
	#define SPI0_CS1 27
	#define SPI0_IO0 16
	#define SPI0_IO1 17
#endif

enum cdns_xspi_stig_instr_type {
	CDNS_XSPI_STIG_INSTR_TYPE_0,
	CDNS_XSPI_STIG_INSTR_TYPE_1,
	CDNS_XSPI_STIG_INSTR_TYPE_DATA_SEQ = 127,
};

enum cdns_xspi_sdma_dir {
	CDNS_XSPI_SDMA_DIR_READ,
	CDNS_XSPI_SDMA_DIR_WRITE,
};

enum cdns_xspi_stig_cmd_dir {
	CDNS_XSPI_STIG_CMD_DIR_READ,
	CDNS_XSPI_STIG_CMD_DIR_WRITE,
};

enum cdns_xspi_sdma_size {
	CDNS_XSPI_SDMA_SIZE_8B = 0,
	CDNS_XSPI_SDMA_SIZE_64B = 1,
};

struct cdns_xspi_dev {
	void __iomem *iobase;
	void __iomem *auxbase;
	void __iomem *sdmabase;

	int irq;
	int cur_cs;
	unsigned int sdmasize;

	bool sdma_error;

	void *in_buffer;
	const void *out_buffer;

	u8 hw_num_banks;
	enum cdns_xspi_sdma_size read_size;
	int xspi_bus;
	int spi_mem_avalible;
	int mode;

#if IS_ENABLED(CONFIG_ARCH_CN10K)
	int current_xfer_qword;
#endif
};

#if IS_ENABLED(CONFIG_ARCH_CN10K) || IS_ENABLED(CONFIG_ARCH_CN20K)
const int cdns_xspi_clk_div_list[] = {
	4,	//0x0 = Divide by 4.   SPI clock is 200 MHz.
	6,	//0x1 = Divide by 6.   SPI clock is 133.33 MHz.
	8,	//0x2 = Divide by 8.   SPI clock is 100 MHz.
	10,	//0x3 = Divide by 10.  SPI clock is 80 MHz.
	12,	//0x4 = Divide by 12.  SPI clock is 66.666 MHz.
	16,	//0x5 = Divide by 16.  SPI clock is 50 MHz.
	18,	//0x6 = Divide by 18.  SPI clock is 44.44 MHz.
	20,	//0x7 = Divide by 20.  SPI clock is 40 MHz.
	24,	//0x8 = Divide by 24.  SPI clock is 33.33 MHz.
	32,	//0x9 = Divide by 32.  SPI clock is 25 MHz.
	40,	//0xA = Divide by 40.  SPI clock is 20 MHz.
	50,	//0xB = Divide by 50.  SPI clock is 16 MHz.
	64,	//0xC = Divide by 64.  SPI clock is 12.5 MHz.
	128,	//0xD = Divide by 128. SPI clock is 6.25 MHz.
	-1	//End of list
};

static void iowrite8_rep(void *addr, const uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		writeb(buf[i], addr);
}

static void ioread8_rep(void *addr, uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		buf[i] = readb(addr);
}

static bool cdns_xspi_reset_dll(struct cdns_xspi_dev *cdns_xspi)
{
	u32 dll_cntrl = readl(cdns_xspi->iobase + CDNS_XSPI_RF_MINICTRL_REGS_DLL_PHY_CTRL);
	u32 dll_lock;

	/*Reset DLL*/
	dll_cntrl |= CDNS_XSPI_DLL_RST_N;
	writel(dll_cntrl, cdns_xspi->iobase + CDNS_XSPI_RF_MINICTRL_REGS_DLL_PHY_CTRL);

	/*Wait for DLL lock*/
	return readl_relaxed_poll_timeout(cdns_xspi->iobase +
		CDNS_XSPI_INTR_STATUS_REG,
		dll_lock, ((dll_lock & CDNS_XSPI_DLL_LOCK) == 1), 10000);
}

//Static confiuration of PHY
static bool cdns_xspi_configure_phy(struct cdns_xspi_dev *cdns_xspi)
{
	writel(REGS_DLL_PHY_CTRL, cdns_xspi->iobase + CDNS_XSPI_RF_MINICTRL_REGS_DLL_PHY_CTRL);
	writel(CTB_RFILE_PHY_CTRL, cdns_xspi->iobase + CDNS_XSPI_PHY_CTB_RFILE_PHY_CTRL);
	writel(RFILE_PHY_TSEL, cdns_xspi->iobase + CDNS_XSPI_PHY_CTB_RFILE_PHY_TSEL);
	writel(RFILE_PHY_DQ_TIMING, cdns_xspi->iobase + CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DQ_TIMING);
	writel(RFILE_PHY_DQS_TIMING, cdns_xspi->iobase + CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DQS_TIMING);
	writel(RFILE_PHY_GATE_LPBK_CTRL, cdns_xspi->iobase + CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_GATE_LPBK_CTRL);
	writel(RFILE_PHY_DLL_MASTER_CTRL, cdns_xspi->iobase + CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DLL_MASTER_CTRL);
	writel(RFILE_PHY_DLL_SLAVE_CTRL, cdns_xspi->iobase + CDNS_XSPI_PHY_DATASLICE_RFILE_PHY_DLL_SLAVE_CTRL);

	return cdns_xspi_reset_dll(cdns_xspi);
}

// Find max avalible clocl
static bool cdns_xspi_setup_clock(struct cdns_xspi_dev *cdns_xspi, int requested_clk)
{
	int i = 0;
	int clk_val;
	u32 clk_reg;
	bool update_clk = false;

	while (cdns_xspi_clk_div_list[i] > 0) {
		clk_val = CDNS_XSPI_CLOCK_DIVIDED(cdns_xspi_clk_div_list[i]);
		if (clk_val <= requested_clk)
			break;
		i++;
	}

	if (cdns_xspi_clk_div_list[i] == -1) {
		log_debug("Unable to find clock divider for CLK: %d - setting 6.25MHz\n",
		       requested_clk);
		i = 0x0D;
	} else {
		log_debug("Found clk div: %d, clk val: %d\n", cdns_xspi_clk_div_list[i],
			  CDNS_XSPI_CLOCK_DIVIDED(cdns_xspi_clk_div_list[i]));
	}

	clk_reg = readl(cdns_xspi->iobase + CDNS_XSPI_CLK_CTRL_REG);

	if (FIELD_GET(CDNS_XSPI_CLK_DIV, clk_reg) != i) {
		clk_reg &= ~CDNS_XSPI_CLK_ENABLE;
		writel(clk_reg, cdns_xspi->auxbase + CDNS_XSPI_CLK_CTRL_REG);
		clk_reg &= ~CDNS_XSPI_CLK_DIV;
		clk_reg |= FIELD_PREP(CDNS_XSPI_CLK_DIV, i);
		clk_reg |= CDNS_XSPI_CLK_ENABLE;
		update_clk = true;
	}

	if (update_clk)
		writel(clk_reg, cdns_xspi->iobase + CDNS_XSPI_CLK_CTRL_REG);

	return update_clk;
}
#endif
static void cdns_xspi_set_interrupts(struct cdns_xspi_dev *cdns_xspi,
				     bool enabled)
{
	u32 intr_enable;

	if (!cdns_xspi->irq)
		return;

	intr_enable = readl(cdns_xspi->iobase + CDNS_XSPI_INTR_ENABLE_REG);
	if (enabled)
		intr_enable |= CDNS_XSPI_INTR_MASK;
	else
		intr_enable &= ~CDNS_XSPI_INTR_MASK;
	writel(intr_enable, cdns_xspi->iobase + CDNS_XSPI_INTR_ENABLE_REG);
}

static int cdns_xspi_controller_init(struct cdns_xspi_dev *cdns_xspi)
{
	u32 ctrl_ver;
	u32 ctrl_features;
	u16 hw_magic_num;

	ctrl_ver = readl(cdns_xspi->iobase + CDNS_XSPI_CTRL_VERSION_REG);
	hw_magic_num = FIELD_GET(CDNS_XSPI_MAGIC_NUM, ctrl_ver);
	if (hw_magic_num != CDNS_XSPI_MAGIC_NUM_VALUE) {
		log_err("Incorrect XSPI magic nunber: %x, expected: %x\n",
			hw_magic_num, CDNS_XSPI_MAGIC_NUM_VALUE);
		return -EIO;
	}

	ctrl_features = readl(cdns_xspi->iobase + CDNS_XSPI_CTRL_FEATURES_REG);
	cdns_xspi->hw_num_banks = FIELD_GET(CDNS_XSPI_NUM_BANKS, ctrl_features);
	cdns_xspi_set_interrupts(cdns_xspi, false);

	return 0;
}

static int cdns_xspi_probe(struct udevice *bus)
{
	struct cdns_xspi_dev *cdns_xspi = dev_get_priv(bus);
	int ret;

#if IS_ENABLED(CONFIG_ARCH_CN10K) || IS_ENABLED(CONFIG_ARCH_CN20K)
	cdns_xspi_setup_clock(cdns_xspi, 25000000);
	cdns_xspi_configure_phy(cdns_xspi);
#endif

	ret = cdns_xspi_controller_init(cdns_xspi);
	if (ret) {
		dev_err(bus, "Failed to initialize controller\n");
		return ret;
	}

	return 0;
}

#if IS_ENABLED(CONFIG_CADENCE_XSPI_WORKAROUND_GPIO)
static void prepare_gpio(int bus, int mem_present)
{
	if (mem_present)
		smc_gpio_as_sw(bus);

	if (bus == 1) {
		gpio_request(SPI1_CLK, "spi1_clk");
		gpio_request(SPI1_CS0, "spi1_cs0");
		gpio_request(SPI1_CS1, "spi1_cs1");
		gpio_request(SPI1_IO0, "spi1_io0");
		gpio_request(SPI1_IO1, "spi1_io1");
	} else {
		gpio_request(SPI0_CLK, "spi0_clk");
		gpio_request(SPI0_CS0, "spi0_cs0");
		gpio_request(SPI0_CS1, "spi0_cs1");
		gpio_request(SPI0_IO0, "spi0_io0");
		gpio_request(SPI0_IO1, "spi0_io1");
	}
	if (mem_present)
		smc_gpio_as_spi(bus);
	else
		cdns_xspi_fix_gpio_config(bus);
}
#endif

static int cdns_xspi_ofdata_to_platdata(struct udevice *bus)
{
	struct cdns_xspi_dev *plat = dev_get_priv(bus);
	ofnode node;
	u32 property;

	plat->iobase = (void __iomem *)ofnode_get_addr_index(dev_ofnode(bus), 0);
	plat->sdmabase = (void __iomem *)ofnode_get_addr_index(dev_ofnode(bus), 1);
	plat->auxbase = (void __iomem *)ofnode_get_addr_index(dev_ofnode(bus), 2);
	plat->irq = 0;
	plat->spi_mem_avalible = 0;

	if (ofnode_read_u32(dev_ofnode(bus), "cdns,read-size", &plat->read_size)) {
		dev_info(bus, "Failed to get read_size. Using 8 bit.\n");
		plat->read_size = 0;
	}

	if ((IS_ENABLED(CONFIG_ARCH_CN10K) && ((u64)plat->iobase == 0x805000000000)) ||
	    (IS_ENABLED(CONFIG_ARCH_CN20K) && ((u64)plat->iobase == 0xCF1100000000)))
		plat->xspi_bus = 1;
	else
		plat->xspi_bus = 0;

	ofnode_for_each_subnode(node, dev_ofnode(bus)) {
		if (ofnode_read_u32(node, "reg", &property)) {
			dev_err(bus, "Couldn't determine CS value\n");
			return -ENXIO;
		}
	}

	ofnode_for_each_subnode(node, dev_ofnode(bus)) {
		if (ofnode_device_is_compatible(node, "spi-flash"))
			plat->spi_mem_avalible = 1;
	}
#if IS_ENABLED(CONFIG_CADENCE_XSPI_WORKAROUND_GPIO)
	prepare_gpio(plat->xspi_bus, plat->spi_mem_avalible);
#endif
	debug("%s: regbase=%llx ahbbase=%llx sdma-base=%llx xspi=%d read_size=%d mem=%d\n",
	      __func__, (u64)plat->iobase, (u64)plat->auxbase, (u64)plat->sdmabase,
	      plat->xspi_bus, plat->read_size, plat->spi_mem_avalible);

	return 0;
}

static void cdns_ioreadq(void __iomem  *addr, void *buf, int len)
{
	int i = 0;
	int rcount = len / 8;
	int rcount_nf = len % 8;
	u64 tmp;
	u64 *buf64 = (uint64_t *)buf;

	if (((uint64_t)buf % 8) == 0) {
		for (i = 0; i < rcount; i++)
			*buf64++ = readq(addr);
	} else {
		for (i = 0; i < rcount; i++) {
			tmp = readq(addr);
			memcpy(buf + (i * 8), &tmp, 8);
		}
	}

	if (rcount_nf != 0) {
		tmp = readq(addr);
		memcpy(buf + (i * 8), &tmp, rcount_nf);
	}
}

static void cdns_iowriteq(void __iomem *addr, const void *buf, int len)
{
	int i = 0;
	int rcount = len / 8;
	int rcount_nf = len % 8;
	u64 tmp;
	u64 *buf64 = (uint64_t *)buf;

	if (((uint64_t)buf % 8) == 0) {
		for (i = 0; i < rcount; i++)
			writeq(*buf64++, addr);
	} else {
		for (i = 0; i < rcount; i++) {
			memcpy(&tmp, buf + (i * 8), 8);
			writeq(tmp, addr);
		}
	}

	if (rcount_nf != 0) {
		memcpy(&tmp, buf + (i * 8), rcount_nf);
		writeq(tmp, addr);
	}
}

static void cdns_xspi_sdma_memread(struct cdns_xspi_dev *cdns_xspi, enum cdns_xspi_sdma_size size, int len)
{
	switch (size) {
	case CDNS_XSPI_SDMA_SIZE_8B:
		ioread8_rep(cdns_xspi->sdmabase,
			    cdns_xspi->in_buffer, len);
		break;
	case CDNS_XSPI_SDMA_SIZE_64B:
		cdns_ioreadq(cdns_xspi->sdmabase, cdns_xspi->in_buffer, len);
		break;
	}
}

static void cdns_xspi_sdma_memwrite(struct cdns_xspi_dev *cdns_xspi, enum cdns_xspi_sdma_size size, int len)
{
	switch (size) {
	case CDNS_XSPI_SDMA_SIZE_8B:
		iowrite8_rep(cdns_xspi->sdmabase,
			     cdns_xspi->out_buffer, len);
		break;
	case CDNS_XSPI_SDMA_SIZE_64B:
		cdns_iowriteq(cdns_xspi->sdmabase, cdns_xspi->in_buffer, len);
		break;
	}
}

static void cdns_xspi_sdma_handle(struct cdns_xspi_dev *cdns_xspi)
{
	u32 sdma_size, sdma_trd_info;
	u8 sdma_dir;

	sdma_size = readl(cdns_xspi->iobase + CDNS_XSPI_SDMA_SIZE_REG);
	sdma_trd_info = readl(cdns_xspi->iobase + CDNS_XSPI_SDMA_TRD_INFO_REG);
	sdma_dir = FIELD_GET(CDNS_XSPI_SDMA_DIR, sdma_trd_info);

	switch (sdma_dir) {
	case CDNS_XSPI_SDMA_DIR_READ:
		cdns_xspi_sdma_memread(cdns_xspi,
				       cdns_xspi->read_size,
				       sdma_size);
		break;

	case CDNS_XSPI_SDMA_DIR_WRITE:
		cdns_xspi_sdma_memwrite(cdns_xspi,
					cdns_xspi->read_size,
					sdma_size);
		break;
	}
}

static int cdns_xspi_wait_for_controller_idle(struct cdns_xspi_dev *cdns_xspi)
{
	u32 ctrl_stat;

	return readl_relaxed_poll_timeout(cdns_xspi->iobase +
		CDNS_XSPI_CTRL_STATUS_REG,
		ctrl_stat, ((ctrl_stat & CDNS_XSPI_CTRL_BUSY) == 0), 1000);
}

static int cdns_xspi_stig_ready(struct cdns_xspi_dev *cdns_xspi)
{
	u32 ctrl_stat;

	if (!otx_is_platform(PLATFORM_ASIM))
		return readl_relaxed_poll_timeout(cdns_xspi->iobase +
			CDNS_XSPI_CTRL_STATUS_REG,
			ctrl_stat, ((ctrl_stat & CDNS_XSPI_GCMD_BUSY) == 0), 1000);
	else
		return 0;
}

bool cdns_xspi_sdma_ready(struct cdns_xspi_dev *cdns_xspi)
{
	u32 ctrl_stat;

	if (!otx_is_platform(PLATFORM_ASIM))
		return readl_relaxed_poll_timeout(cdns_xspi->iobase +
			CDNS_XSPI_INTR_STATUS_REG,
			ctrl_stat, ((ctrl_stat & CDNS_XSPI_SDMA_TRIGGER)), 1000);
	else
		return 0;
}

static int cdns_verify_stig_mode_config(struct cdns_xspi_dev *cdns_xspi)
{
	int cntrl = readl(cdns_xspi->iobase + CDNS_XSPI_CTRL_CONFIG_REG);

	if (FIELD_GET(CDNS_XSPI_CTRL_WORK_MODE, cntrl) != CDNS_XSPI_CTRL_WORK_MODE_STIG)
		return cdns_xspi_controller_init(cdns_xspi);

	return 0;
}

static int cdns_xspi_check_command_status(struct cdns_xspi_dev *cdns_xspi)
{
	int ret = 0;
	u32 cmd_status = readl(cdns_xspi->iobase + CDNS_XSPI_CMD_STATUS_REG);

	if (otx_is_platform(PLATFORM_ASIM))
		return 0;

	if (cmd_status & CDNS_XSPI_CMD_STATUS_COMPLETED) {
		if ((cmd_status & CDNS_XSPI_CMD_STATUS_FAILED) != 0) {
			if (cmd_status & CDNS_XSPI_CMD_STATUS_DQS_ERROR) {
				log_err("Incorrect DQS pulses detected\n");
				ret = -EPROTO;
			}
			if (cmd_status & CDNS_XSPI_CMD_STATUS_CRC_ERROR) {
				log_err("CRC error received\n");
				ret = -EPROTO;
			}
			if (cmd_status & CDNS_XSPI_CMD_STATUS_BUS_ERROR) {
				log_err("Error resp on system DMA interface\n");
				ret = -EPROTO;
			}
			if (cmd_status & CDNS_XSPI_CMD_STATUS_INV_SEQ_ERROR) {
				log_err("Invalid command sequence detected\n");
				ret = -EPROTO;
			}
		}
	} else {
		log_err("Fatal err - command not completed\n");
		ret = -EPROTO;
	}

	return ret;
}

static void cdns_xspi_trigger_command(struct cdns_xspi_dev *cdns_xspi,
				      u32 cmd_regs[6])
{
	writel(cmd_regs[5], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_5);
	writel(cmd_regs[4], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_4);
	writel(cmd_regs[3], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_3);
	writel(cmd_regs[2], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_2);
	writel(cmd_regs[1], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_1);
	writel(cmd_regs[0], cdns_xspi->iobase + CDNS_XSPI_CMD_REG_0);
}

static int cdns_xspi_send_stig_command(struct cdns_xspi_dev *cdns_xspi,
				       const struct spi_mem_op *op,
				       bool data_phase)
{
	u32 cmd_regs[6];
	u32 cmd_status;
	int ret;
	int dummybytes = op->dummy.nbytes;

#if IS_ENABLED(CONFIG_CADENCE_XSPI_WORKAROUND_GPIO)
	if (cdns_xspi->spi_mem_avalible)
		smc_gpio_as_spi(cdns_xspi->xspi_bus);
#endif

	ret = cdns_xspi_wait_for_controller_idle(cdns_xspi);
	if (ret < 0)
		return -EIO;
	if (cdns_verify_stig_mode_config(cdns_xspi)) {
		printf("Failed to configure xSPI");
		return -1;
	}

	writel(FIELD_PREP(CDNS_XSPI_CTRL_WORK_MODE, CDNS_XSPI_WORK_MODE_STIG),
	       cdns_xspi->iobase + CDNS_XSPI_CTRL_CONFIG_REG);

	cdns_xspi_set_interrupts(cdns_xspi, true);
	cdns_xspi->sdma_error = false;

	memset(cmd_regs, 0, sizeof(cmd_regs));
	cmd_regs[1] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_1(op, data_phase);
	cmd_regs[2] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_2(op);
	//if (dummybytes != 0 && op->cmd.opcode != 0x5A) {
	if (dummybytes != 0) {
		cmd_regs[3] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_3(op, 1);
		dummybytes--;
	} else {
		cmd_regs[3] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_3(op, 0);
	}
	cmd_regs[4] = CDNS_XSPI_CMD_FLD_P1_INSTR_CMD_4(op,
						       cdns_xspi->cur_cs);

	cdns_xspi_trigger_command(cdns_xspi, cmd_regs);

	if (data_phase) {
		cmd_regs[0] = CDNS_XSPI_STIG_DONE_FLAG;
		cmd_regs[1] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_1(op);
		cmd_regs[2] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_2(op);
		cmd_regs[3] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_3(op, dummybytes);
		cmd_regs[4] = CDNS_XSPI_CMD_FLD_DSEQ_CMD_4(op,
							   cdns_xspi->cur_cs);

		cdns_xspi->in_buffer = op->data.buf.in;
		cdns_xspi->out_buffer = op->data.buf.out;

		cdns_xspi_trigger_command(cdns_xspi, cmd_regs);

		if (cdns_xspi->irq) {
			if (cdns_xspi->sdma_error) {
				cdns_xspi_set_interrupts(cdns_xspi, false);
				return -EIO;
			}
		} else {
			if (cdns_xspi_sdma_ready(cdns_xspi))
				return -EIO;
		}
		cdns_xspi_sdma_handle(cdns_xspi);
	}

	if (cdns_xspi->irq) {
		cdns_xspi_set_interrupts(cdns_xspi, false);
	} else {
		if (cdns_xspi_stig_ready(cdns_xspi))
			return -EIO;
	}

	cmd_status = cdns_xspi_check_command_status(cdns_xspi);
	if (cmd_status)
		return -EPROTO;

	return 0;
}

static int cdns_xspi_exec_op(struct spi_slave *slave,
			     const struct spi_mem_op *op)
{
	int ret = 0;
	struct dm_spi_slave_plat *slave_dev = dev_get_parent_plat(slave->dev);
	struct udevice *dev = slave->dev->parent;
	struct cdns_xspi_dev *cdns_xspi = dev_get_priv(dev);
	bool data_phase = (op->data.dir != SPI_MEM_NO_DATA);

	/* Uboot is generating Read enable CMD with direction out and data
	 *  nbytes = 0
	 *  In that case instruction glueing must be disabled
	 */
	if (op->data.dir == SPI_MEM_DATA_OUT && op->data.nbytes == 0)
		data_phase = false;

	cdns_xspi->cur_cs = slave_dev->cs;

	log_debug("SPI opcode: %X buswidth: %d\n",
		  op->cmd.opcode, op->cmd.buswidth);
	log_debug("SPI addr: nbytes: %X buswidth: %d, val: %llX\n",
		  op->addr.nbytes, op->addr.buswidth, op->addr.val);
	log_debug("SPI data: direction: %X, len: %d, ptr: %p\n",
		  op->data.dir, op->data.nbytes, op->data.buf.in);
	log_debug("SPI dummy: nbytes: %d, buswidth: %d\n",
		  op->dummy.nbytes, op->dummy.buswidth);
	if (data_phase)
		log_debug("Data Phase enabled\n");
	else
		log_debug("Data Phase disabled\n");
	log_debug("--------------------------------------------------\n\n");

	ret = cdns_xspi_send_stig_command(cdns_xspi, op, data_phase);

	return ret;
}

#if defined(CONFIG_ARCH_CN10K)
int board_acquire_flash_arb(bool acquire);
#endif

static int cdns_xspi_claim_bus(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_ARCH_CN10K)) {
		if (board_acquire_flash_arb(true))
			board_acquire_flash_arb(false);
	}

	return 0;
}

static int cdns_xspi_release_bus(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_ARCH_CN10K))
		board_acquire_flash_arb(false);

	return 0;
}

static int cdns_xspi_set_speed(struct udevice *bus, uint max_hz)
{
	if ((IS_ENABLED(CONFIG_ARCH_CN10K) || IS_ENABLED(CONFIG_ARCH_CN20K)))
		cdns_xspi_setup_clock(dev_get_priv(bus), max_hz);

	return 0;
}

static int cdns_xspi_set_mode(struct udevice *bus, uint mode)
{
	struct cdns_xspi_dev *cdns_xspi = dev_get_priv(bus);

	cdns_xspi->mode = mode;

	return 0;
}

static bool cdns_xspi_supports_op(struct spi_slave *slave,
				  const struct spi_mem_op *op)
{
	return spi_mem_default_supports_op(slave, op);
}

#if IS_ENABLED(CONFIG_ARCH_CN10K)
#if IS_ENABLED(CONFIG_CADENCE_XSPI_WORKAROUND_GPIO)
void cdns_cs_change(int bus, int cs, int active)
{
	int cs0 = bus == 0 ? SPI0_CS0 : SPI1_CS0;
	int cs1 = bus == 0 ? SPI0_CS1 : SPI1_CS1;

	if (cs == 0)
		gpio_set_value(cs0, active);
	else
		gpio_set_value(cs1, active);
}

void cdns_soft_spi_scl(int bus, int val)
{
	int scl = bus == 0 ? SPI0_CLK : SPI1_CLK;

	gpio_set_value(scl, val);
}

void cdns_soft_spi_sdo(int bus, int val)
{
	int sdo = bus == 0 ? SPI0_IO0 : SPI1_IO0;

	gpio_set_value(sdo, val);
}

int cdns_soft_spi_getval_sdi(int bus)
{
	int sdi = bus == 0 ? SPI0_IO1 : SPI1_IO1;

	return gpio_get_value(sdi);
}

int cdns_xspi_fix_gpio_config(int bus)
{
	if (bus == 1) {
		gpio_direction_output(SPI1_CLK, 1);
		gpio_direction_output(SPI1_CS0, 1);
		gpio_direction_output(SPI1_CS1, 1);
		gpio_direction_output(SPI1_IO0, 1);
		gpio_direction_input(SPI1_IO1);
	} else {
		gpio_direction_output(SPI0_CLK, 1);
		gpio_direction_output(SPI0_CS0, 1);
		gpio_direction_output(SPI0_CS1, 1);
		gpio_direction_output(SPI0_IO0, 1);
		gpio_direction_input(SPI0_IO1);
	}

	return 0;
}

static int cdns_xspi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct dm_spi_slave_plat *slave_dev = dev_get_parent_plat(dev);
	struct cdns_xspi_dev *cdns_xspi = dev_get_priv(dev->parent);
	int bus = cdns_xspi->xspi_bus;
	int cs  = slave_dev->cs;

	uchar		tmpdin  = 0;
	uchar		tmpdout = 0;
	const u8	*txd = dout;
	u8		*rxd = din;
	int		cpha = !!(cdns_xspi->mode & SPI_CPHA);
	int		cidle = !!(cdns_xspi->mode & SPI_CPOL);
	unsigned int	j;

	if (cdns_xspi->spi_mem_avalible && smc_gpio_as_sw(bus))
		cdns_xspi_fix_gpio_config(bus);

	if (flags & SPI_XFER_BEGIN)
		cdns_cs_change(bus, cs, 0);

	for (j = 0; j < bitlen; j++) {
		/*
		 * Check if it is time to work on a new byte.
		 */
		if ((j % 8) == 0) {
			if (txd)
				tmpdout = *txd++;
			else
				tmpdout = 0;
			if (j != 0) {
				if (rxd)
					*rxd++ = tmpdin;
			}
			tmpdin  = 0;
		}

		/*
		 * CPOL 0: idle is low (0), active is high (1)
		 * CPOL 1: idle is high (1), active is low (0)
		 */

		/*
		 * drive bit
		 *  CPHA 1: CLK from idle to active
		 */
		if (cpha)
			cdns_soft_spi_scl(bus, !cidle);
		cdns_soft_spi_sdo(bus, !!(tmpdout & 0x80));

		/*
		 * sample bit
		 *  CPHA 0: CLK from idle to active
		 *  CPHA 1: CLK from active to idle
		 */
		if (!cpha)
			cdns_soft_spi_scl(bus, !cidle);
		else
			cdns_soft_spi_scl(bus, cidle);
		tmpdin	<<= 1;
		tmpdin	|= cdns_soft_spi_getval_sdi(bus);
		tmpdout	<<= 1;

		/*
		 * drive bit
		 *  CPHA 0: CLK from active to idle
		 */
		if (!cpha)
			cdns_soft_spi_scl(bus, cidle);
	}
	/*
	 * If the number of bits isn't a multiple of 8, shift the last
	 * bits over to left-justify them.  Then store the last byte
	 * read in.
	 */
	if (rxd) {
		if ((bitlen % 8) != 0)
			tmpdin <<= 8 - (bitlen % 8);
		*rxd++ = tmpdin;
	}

	if (flags & SPI_XFER_END)
		cdns_cs_change(bus, cs, 1);

	return 0;
}
#else

#define SPIX_XFER_FUNC_CTRL 0x8210
#define SPIX_XFER_FUNC_CTRL_READ_DATA(i) (0x8000 + 8 * (i))

#define XFER_SOFT_RESET_BP 11
#define XFER_CS_N_HOLD_BP 6
#define XFER_RECEIVE_ENABLE_BP 4
#define XFER_FUNC_ENABLE_BP 3
#define XFER_CLK_CAPTURE_POL_BP 2
#define XFER_CLK_DRIVE_POL_BP 1
#define XFER_FUNC_START_BP 0

#define XFER_QWORD_COUNT 32
#define XFER_QWORD_BYTECOUNT 8

static int cdns_xspi_prepare_generic(int cs, const void *dout, int len, int glue, u32 *cmd_regs)
{
	u8 *data = (u8 *)dout;
	int i;
	int data_counter = 0;

	memset(cmd_regs, 0x00, 6 * 4);

	if (len > 7) {
		for (i = (len >= 10 ? 2 : len - 8); i >= 0 ; i--)
			cmd_regs[3] |= data[data_counter++] << (8 * i);
	}
	if (len > 3) {
		for (i = (len >= 7 ? 3 : len - 4); i >= 0; i--)
			cmd_regs[2] |= data[data_counter++] << (8 * i);
	}
	for (i = (len >= 3 ? 2 : len - 1); i >= 0 ; i--)
		cmd_regs[1] |= data[data_counter++] << (8 + 8 * i);

	cmd_regs[1] |= 96;
	cmd_regs[3] |= len << 24;
	cmd_regs[4] |= cs << 12;

	if (glue == 1)
		cmd_regs[4] |= 1 << 28;

	return 0;
}

static int cdns_xspi_prepare_transfer(int cs, int dir, int len, u32 *cmd_regs)
{
	memset(cmd_regs, 0x00, 6 * 4);

	cmd_regs[1] |= 127;
	cmd_regs[2] |= len << 16;
	cmd_regs[4] |= dir << 4; //dir = 0 read, dir =1 write
	cmd_regs[4] |= cs << 12;

	return 0;
}

unsigned char reverse_bits(unsigned char num)
{
	unsigned int count = sizeof(num) * 8 - 1;
	unsigned int reverse_num = num;

	num >>= 1;
	while (num) {
		reverse_num <<= 1;
		reverse_num |= num & 1;
		num >>= 1;
		count--;
	}
	reverse_num <<= count;
	return reverse_num;
}

static void cdns_xspi_read_single_qword(struct cdns_xspi_dev *cdns_xspi, u8 **buffer)
{
	u64 d = readq(cdns_xspi->iobase + SPIX_XFER_FUNC_CTRL_READ_DATA(cdns_xspi->current_xfer_qword));
	u8 *ptr = (u8 *)&d;
	int k;

	for (k = 0; k < 8; k++) {
		u8 val = reverse_bits((ptr[k]));
		**buffer = val;
		*buffer = *buffer + 1;
	}

	cdns_xspi->current_xfer_qword++;
	cdns_xspi->current_xfer_qword %= XFER_QWORD_COUNT;
}

static void cdns_xspi_finish_read(struct cdns_xspi_dev *cdns_xspi, u8 **buffer, u32 data_count)
{
	u64 d = readq(cdns_xspi->iobase + SPIX_XFER_FUNC_CTRL_READ_DATA(cdns_xspi->current_xfer_qword));
	u8 *ptr = (u8 *)&d;
	int k;

	for (k = 0; k < data_count % XFER_QWORD_BYTECOUNT; k++) {
		u8 val = reverse_bits((ptr[k]));
		**buffer = val;
		*buffer = *buffer + 1;
	}

	cdns_xspi->current_xfer_qword++;
	cdns_xspi->current_xfer_qword %= XFER_QWORD_COUNT;
}

static int cdns_xspi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	const int max_len = XFER_QWORD_BYTECOUNT * XFER_QWORD_COUNT;
	struct dm_spi_slave_plat *slave_dev = dev_get_parent_plat(dev);
	struct cdns_xspi_dev *cdns_xspi = dev_get_priv(dev->parent);
	int cs  = slave_dev->cs;
	int bytecount = bitlen / XFER_QWORD_BYTECOUNT;
	int current_cycle_count;
	u8 *txd = dout ? (u8 *)dout : din;
	u8 *rxd = (u8 *)din;

	if (flags & SPI_XFER_BEGIN) {
		u32 xfer_control = readl(cdns_xspi->iobase + SPIX_XFER_FUNC_CTRL);
		cdns_xspi->current_xfer_qword = 0;
		xfer_control |= (1 << XFER_RECEIVE_ENABLE_BP |
				 1 << XFER_CLK_CAPTURE_POL_BP |
				 1 << XFER_FUNC_START_BP |
				 (1 << cs) << XFER_CS_N_HOLD_BP) |
				 1 << XFER_SOFT_RESET_BP;
		xfer_control &= ~(1 << XFER_FUNC_ENABLE_BP | 1 << XFER_CLK_DRIVE_POL_BP);
		writel(xfer_control, cdns_xspi->iobase + SPIX_XFER_FUNC_CTRL);
	}

	current_cycle_count = bytecount > max_len ? max_len : bytecount;
	cdns_xspi->in_buffer = txd + 1;
	cdns_xspi->out_buffer = txd + 1;

	while (bytecount) {
		u32 cmd_regs[6];

		if (current_cycle_count < 10) {
			cdns_xspi_prepare_generic(cs, txd, current_cycle_count, false, cmd_regs);
			cdns_xspi_trigger_command(cdns_xspi, cmd_regs);
			if (cdns_xspi_stig_ready(cdns_xspi))
				return -EIO;
		} else {
			cdns_xspi_prepare_generic(cs, txd, 1, true, cmd_regs);
			cdns_xspi_trigger_command(cdns_xspi, cmd_regs);
			cdns_xspi_prepare_transfer(cs, 1, current_cycle_count - 1, cmd_regs);
			cdns_xspi_trigger_command(cdns_xspi, cmd_regs);
			if (cdns_xspi_sdma_ready(cdns_xspi))
				return -EIO;
			cdns_xspi_sdma_handle(cdns_xspi);
			if (cdns_xspi_stig_ready(cdns_xspi))
				return -EIO;

			cdns_xspi->in_buffer += current_cycle_count;
			cdns_xspi->out_buffer += current_cycle_count;
		}

		if (rxd) {
			int j;

			for (j = 0; j < current_cycle_count / 8; j++)
				cdns_xspi_read_single_qword(cdns_xspi, &rxd);
			cdns_xspi_finish_read(cdns_xspi, &rxd, current_cycle_count);
		} else {
			cdns_xspi->current_xfer_qword += current_cycle_count / XFER_QWORD_BYTECOUNT;
			if (current_cycle_count % XFER_QWORD_BYTECOUNT)
				cdns_xspi->current_xfer_qword++;

			cdns_xspi->current_xfer_qword %= XFER_QWORD_COUNT;
		}

		bytecount = bytecount - current_cycle_count;
		current_cycle_count = bytecount > max_len ? max_len : bytecount;
	}

	if (flags & SPI_XFER_END) {
		u32 xfer_control = readl(cdns_xspi->iobase + SPIX_XFER_FUNC_CTRL);

		xfer_control &= ~(1 << XFER_RECEIVE_ENABLE_BP |
				  1 << XFER_SOFT_RESET_BP);
		writel(xfer_control, cdns_xspi->iobase + SPIX_XFER_FUNC_CTRL);
	}

	return 0;
}
#endif
#endif

static const struct spi_controller_mem_ops cdns_mem_ops = {
	.supports_op = cdns_xspi_supports_op,
	.exec_op = cdns_xspi_exec_op,
};

static struct dm_spi_ops cdns_spi_ops = {
	.set_mode	= cdns_xspi_set_mode,
	.set_speed	= cdns_xspi_set_speed,
	.claim_bus	= cdns_xspi_claim_bus,
	.release_bus	= cdns_xspi_release_bus,
	.mem_ops	= &cdns_mem_ops,
#if IS_ENABLED(CONFIG_ARCH_CN10K)
	.xfer		= cdns_xspi_xfer,
#endif
};

static const struct udevice_id cdns_xspi_ids[] = {
	{ .compatible = "cdns,xspi-nor" },
	{ }
};

U_BOOT_DRIVER(cadence_spi) = {
	.name = "cadence_spi",
	.id = UCLASS_SPI,
	.of_match = cdns_xspi_ids,
	.ops = &cdns_spi_ops,
	.of_to_plat = cdns_xspi_ofdata_to_platdata,
	.priv_auto = sizeof(struct cdns_xspi_dev),
	.probe = cdns_xspi_probe,
	.flags = DM_FLAG_OS_PREPARE,
};
