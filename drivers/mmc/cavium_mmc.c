/***********************license start************************************
 * Copyright (c) 2012 - 2016 Cavium Inc. (support@cavium.com).  All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *     * Neither the name of Cavium Inc. nor the names of
 *       its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
 * REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
 * DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
 * PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
 * POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
 * OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 *
 *
 * For any questions regarding licensing please contact
 * support@cavium.com
 *
 ***********************license end**************************************/

/*
 * Due to how MMC is implemented in the OCTEON processor it is not
 * possible to use the generic MMC support.  However, this code
 * implements all of the MMC support found in the generic MMC driver
 * and should be compatible with it for the most part.
 *
 * Currently both MMC and SD/SDHC/SDXC are supported.
 */

#include <common.h>
#ifdef DEBUG
#include <command.h>
#endif
#include <dm.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include "mmc_private.h"
#include <errno.h>
#include <pci.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <asm/arch/fdt-helper.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/cavium_mmc.h>
#include <asm/arch/clock.h>
#include <linux/list.h>
#include <div64.h>
#include <watchdog.h>
#include <console.h>	/* for ctrlc */

#ifdef DEBUG
#define DEBUG_CSR
#define DEBUG_REGISTERS
#endif

/** Name of our driver */
#define CAVIUM_MMC_DRIVER_NAME			"mmc_cavium"

#ifndef CONFIG_CAVIUM_MMC_MAX_FREQUENCY
# define CONFIG_CAVIUM_MMC_MAX_FREQUENCY	52000000
#endif

/* Enable support for SD as well as MMC */
#define CONFIG_CAVIUM_MMC_SD

#define POWER_ON_TIME		40	/** See SD 4.1 spec figure 6-5 */

/**
 * Timeout used when waiting for commands to complete.  We need to keep this
 * above the hardware watchdog timeout which is usually limited to 1000ms
 */
#define WATCHDOG_COUNT		(1100)	/* in msecs */

/**
 * Long timeout for commands which might take a while to complete.
 */
#define MMC_TIMEOUT_LONG	1000

/**
 * Erase timeout
 */
#define MMC_TIMEOUT_ERASE	0

/**
 * Short timeout used for most commands in msecs
 */
#define MMC_TIMEOUT_SHORT	20

#ifndef CONFIG_CAVIUM_MAX_MMC_SLOT
# define CONFIG_CAVIUM_MAX_MMC_SLOT		4
#endif

#ifndef CONFIG_CAVIUM_MMC_MIN_BUS_SPEED_HZ
# define CONFIG_CAVIUM_MMC_MIN_BUS_SPEED_HZ		400000
#endif

#ifndef CONFIG_CAVIUM_MMC_SD
# undef IS_SD
# define IS_SD(x) (0)
#endif

#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
/** Maximum continguous read supported by the hardware */
# define CONFIG_SYS_MMC_MAX_BLK_COUNT	8191
#elif CONFIG_SYS_MMC_MAX_BLK_COUNT > 8191
# error CONFIG_SYS_MMC_MAX_BLK_COUNT is too large! Must not exceed 8191!
#endif

/* The hardware automatically sets the response type based on the command
 * number, however, the expected response type is for MMC devices and not
 * SD devices.  The XOR types are used to override these built-in settings.
 */
#define MMC_CMD_FLAG_CTYPE_XOR(x)	(((x) & 3) << 16)
#define MMC_CMD_FLAG_RTYPE_XOR(x)	(((x) & 7) << 20)
#define MMC_CMD_FLAG_OFFSET(x)		(((x) & 0x3f) << 24)
/** Ignore CRC errors in the responses */
#define MMC_CMD_FLAG_IGNORE_CRC_ERR	(1 << 30)
/** Strip the CRC from the response */
#define MMC_CMD_FLAG_STRIP_CRC		(1 << 31)

DECLARE_GLOBAL_DATA_PTR;

static int init_time = 1;

int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value);
static int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd);

static int mmc_send_cmd_timeout(struct mmc *mmc, struct mmc_cmd *cmd,
				struct mmc_data *data, uint32_t flags,
				uint timeout);

static int mmc_send_cmd_flags(struct mmc *mmc, struct mmc_cmd *cmd,
			      struct mmc_data *data, uint32_t flags);

static int mmc_send_acmd(struct mmc *mmc, struct mmc_cmd *cmd,
			 struct mmc_data *data, uint32_t flags);

static int cavium_mmc_set_ios(struct udevice *dev);

static void mmc_switch_dev(struct mmc *mmc);

int cavium_mmc_getwp(struct udevice *dev)
	__attribute__((weak, alias("__cavium_mmc_getwp")));

int cavium_mmc_getcd(struct udevice *dev)
	__attribute__((weak, alias("__cavium_mmc_getcd")));

int cavium_mmc_init(struct mmc *mmc)
	__attribute__((weak, alias("__cavium_mmc_init")));

/**
 * This is the external mmc_send_cmd function.  It was required that
 * the internal version support flags so this version is required.
 */
static int cavium_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			       struct mmc_data *data);

static const struct udevice_id cavium_mmc_ids[] = {
	{ .compatible = "cavium,thunder-8890-mmc" },
	{ .compatible = "cavium,mmc" },
	{ },
};

static const struct dm_mmc_ops cavium_mmc_ops = {
	.send_cmd = cavium_mmc_send_cmd,
	.set_ios = cavium_mmc_set_ios,
	.get_cd = cavium_mmc_getcd,
	.get_wp = cavium_mmc_getwp,
};

#ifdef CONFIG_CAVIUM_MMC_SD
static int sd_set_ios(struct mmc *mmc);
#endif

/**
 * Gets the configuration of a MMC interface and all of the slots
 *
 * @param[in]	blob	pointer to flat device tree blob
 * @param	node	node of MMC device in the device tree
 * @param[out]	host	interface configuration
 *
 * @return	0 for success, -1 on error.
 */
static int cavium_mmc_get_config(struct udevice *dev);

#ifdef DEBUG
const char *mmc_reg_str(uint64_t reg)
{
	switch (reg) {
#ifdef __mips
	case MIO_NDF_DMA_CFG:		return "MIO_NDF_DMA_CFG";
	case MIO_NDF_DMA_INT:		return "MIO_NDF_DMA_INT";
	case MIO_NDF_DMA_INT_EN:	return "MIO_NDF_DMA_INT_EN";
#endif
	case MIO_EMM_DMA_CFG:		return "MIO_EMM_DMA_CFG";
	case MIO_EMM_DMA_ADR:		return "MIO_EMM_DMA_ADR";
	case MIO_EMM_DMA_INT:		return "MIO_EMM_DMA_INT";
	case MIO_EMM_CFG:		return "MIO_EMM_CFG";
	case MIO_EMM_MODEX(0):		return "MIO_EMM_MODE0";
	case MIO_EMM_MODEX(1):		return "MIO_EMM_MODE1";
	case MIO_EMM_MODEX(2):		return "MIO_EMM_MODE2";
	case MIO_EMM_MODEX(3):		return "MIO_EMM_MODE3";
	case MIO_EMM_SWITCH:		return "MIO_EMM_SWITCH";
	case MIO_EMM_DMA:		return "MIO_EMM_DMA";
	case MIO_EMM_CMD:		return "MIO_EMM_CMD";
	case MIO_EMM_RSP_STS:		return "MIO_EMM_RSP_STS";
	case MIO_EMM_RSP_LO:		return "MIO_EMM_RSP_LO";
	case MIO_EMM_RSP_HI:		return "MIO_EMM_RSP_HI";
	case MIO_EMM_INT:		return "MIO_EMM_INT";
#ifdef __mips
	case MIO_EMM_INT_EN:		return "MIO_EMM_INT_EN";
#endif
	case MIO_EMM_WDOG:		return "MIO_EMM_WDOG";
	case MIO_EMM_SAMPLE:		return "MIO_EMM_SAMPLE";
	case MIO_EMM_STS_MASK:		return "MIO_EMM_STS_MASK";
	case MIO_EMM_RCA:		return "MIO_EMM_RCA";
	case MIO_EMM_BUF_IDX:		return "MIO_EMM_BUF_IDX";
	case MIO_EMM_BUF_DAT:		return "MIO_EMM_BUF_DAT";
	default:			return "UNKNOWN";
	}
}
#endif

/**
 * Obtains a field from the multi-word CSD register
 *
 * @param[in]	mmc	MMC data structure
 * @param	start	starting bit offset [0-127]
 * @param	end	ending bit offset [0-127]
 *
 * @return	field extracted from the CSD register
 */
static uint32_t get_csd_bits(const struct mmc *mmc, unsigned start,
			     unsigned end)
{
	const uint32_t *csd = mmc->csd;
	int start_off = 3 - start / 32;
	int end_off = 3 - end / 32;
	int start_bit = start & 0x1f;
	int end_bit = end & 0x1f;
	uint32_t val;
	uint32_t mask;
	if (start_off == end_off) {
		val = csd[start_off] >> start_bit;
		mask = (1 << (end - start + 1)) - 1;
		val &= mask;
	} else {
		mask = (1ULL << (end_bit + 1)) - 1;
		val = (csd[end_off] & mask) << (32 - start_bit);
		mask = (1 << (32 - start_bit)) - 1;
		val |= (csd[start_off] >> start_bit) & mask;
	}
	return val;
}

/**
 * Writes to a CSR register
 *
 * @param[in]	mmc	pointer to mmc data structure
 * @param	reg	register offset
 * @param	value	value to write to register
 */
static inline void mmc_write_csr(const struct mmc *mmc, uint64_t reg,
				 uint64_t value)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	struct cavium_mmc_host *host = slot->host;
	void *addr = host->base_addr + reg;

#ifdef DEBUG_CSR
	printf("        %s: %s(0x%p) <= 0x%llx\n", __func__, mmc_reg_str(reg),
	       addr, value);
#endif
	writeq(value, addr);
}

/**
 * Reads from a CSR register
 *
 * @param[in]	mmc	pointer to mmc data structure
 * @param	reg	register offset to read from
 *
 * @return	value of the register
 */
static inline uint64_t mmc_read_csr(const struct mmc *mmc, uint64_t reg)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	struct cavium_mmc_host *host = slot->host;
	uint64_t value = readq(host->base_addr + reg);
#ifdef DEBUG_CSR
	printf("        %s: %s(0x%p) => 0x%llx\n", __func__,
	       mmc_reg_str(reg), host->base_addr + reg,
	       value);
#endif
	return value;
}

#ifdef DEBUG
static void mmc_print_status(uint32_t status)
{
#ifdef DEBUG_STATUS
	static const char * const state[] = {
		"Idle",		/* 0 */
		"Ready",	/* 1 */
		"Ident",	/* 2 */
		"Standby",	/* 3 */
		"Tran",		/* 4 */
		"Data",		/* 5 */
		"Receive",	/* 6 */
		"Program",	/* 7 */
		"Dis",		/* 8 */
		"Btst",		/* 9 */
		"Sleep",	/* 10 */
		"reserved",	/* 11 */
		"reserved",	/* 12 */
		"reserved",	/* 13 */
		"reserved",	/* 14 */
		"reserved"	/* 15 */ };
	if (status & R1_APP_CMD)
		puts("MMC ACMD\n");
	if (status & R1_SWITCH_ERROR)
		puts("MMC switch error\n");
	if (status & R1_READY_FOR_DATA)
		puts("MMC ready for data\n");
	printf("MMC %s state\n", state[R1_CURRENT_STATE(status)]);
	if (status & R1_ERASE_RESET)
		puts("MMC erase reset\n");
	if (status & R1_WP_ERASE_SKIP)
		puts("MMC partial erase due to write protected blocks\n");
	if (status & R1_CID_CSD_OVERWRITE)
		puts("MMC CID/CSD overwrite error\n");
	if (status & R1_ERROR)
		puts("MMC undefined device error\n");
	if (status & R1_CC_ERROR)
		puts("MMC device error\n");
	if (status & R1_CARD_ECC_FAILED)
		puts("MMC internal ECC failed to correct data\n");
	if (status & R1_ILLEGAL_COMMAND)
		puts("MMC illegal command\n");
	if (status & R1_COM_CRC_ERROR)
		puts("MMC CRC of previous command failed\n");
	if (status & R1_LOCK_UNLOCK_FAILED)
		puts("MMC sequence or password error in lock/unlock device command\n");
	if (status & R1_CARD_IS_LOCKED)
		puts("MMC device locked by host\n");
	if (status & R1_WP_VIOLATION)
		puts("MMC attempt to program write protected block\n");
	if (status & R1_ERASE_PARAM)
		puts("MMC invalid selection of erase groups for erase\n");
	if (status & R1_ERASE_SEQ_ERROR)
		puts("MMC error in sequence of erase commands\n");
	if (status & R1_BLOCK_LEN_ERROR)
		puts("MMC block length error\n");
	if (status & R1_ADDRESS_ERROR)
		puts("MMC address misalign error\n");
	if (status & R1_OUT_OF_RANGE)
		puts("MMC address out of range\n");
#endif
}
#endif

static void mmc_print_registers(struct mmc *mmc)
{
#ifdef DEBUG_REGISTERS
	struct cavium_mmc_slot *slot = mmc->priv;
#ifdef __mips
	struct cavium_mmc_host *host = slot->host;
	union mio_ndf_dma_cfg ndf_dma_cfg;
	union mio_ndf_dma_int ndf_dma_int;
#endif
	union mio_emm_dma_cfg emm_dma_cfg;
	union mio_emm_dma_adr emm_dma_adr;
	union mio_emm_dma_int emm_dma_int;
	union mio_emm_cfg emm_cfg;
	union mio_emm_modex emm_mode;
	union mio_emm_switch emm_switch;
	union mio_emm_dma emm_dma;
	union mio_emm_cmd emm_cmd;
	union mio_emm_rsp_sts emm_rsp_sts;
	union mio_emm_rsp_lo emm_rsp_lo;
	union mio_emm_rsp_hi emm_rsp_hi;
	union mio_emm_int emm_int;
	union mio_emm_wdog emm_wdog;
	union mio_emm_sample emm_sample;
	union mio_emm_sts_mask emm_sts_mask;
	union mio_emm_rca emm_rca;
	int bus;

	static const char * const bus_width_str[] = {
		"1-bit data bus (power on)",
		"4-bit data bus",
		"8-bit data bus",
		"reserved (3)",
		"reserved (4)",
		"4-bit data bus (dual data rate)",
		"8-bit data bus (dual data rate)",
		"reserved (7)",
		"reserved (8)",
		"invalid (9)",
		"invalid (10)",
		"invalid (11)",
		"invalid (12)",
		"invalid (13)",
		"invalid (14)",
		"invalid (15)",
	};

	static const char * const ctype_xor_str[] = {
		"No data",
		"Read data into Dbuf",
		"Write data from Dbuf",
		"Reserved"
	};

	static const char * const rtype_xor_str[] = {
		"No response",
		"R1, 48 bits",
		"R2, 136 bits",
		"R3, 48 bits",
		"R4, 48 bits",
		"R5, 48 bits",
		"Reserved 6",
		"Reserved 7"
	};

	if (!mmc) {
		printf("%s: Error: MMC data structure required\n", __func__);
		return;
	}

	printf("%s: bus id: %u\n", __func__, slot->bus_id);
#ifdef __mips
	if (host->use_ndf) {
		ndf_dma_cfg.u = mmc_read_csr(mmc, MIO_NDF_DMA_CFG);
		printf("MIO_NDF_DMA_CFG:                0x%016llx\n",
		       ndf_dma_cfg.u);
		printf("    63:    en:                  %s\n",
		       ndf_dma_cfg.s.en ? "enabled" : "disabled");
		printf("    62:    rw:                  %s\n",
		       ndf_dma_cfg.s.rw ? "write" : "read");
		printf("    61:    clr:                 %s\n",
		       ndf_dma_cfg.s.clr ? "clear" : "not clear");
		printf("    59:    swap32:              %s\n",
		       ndf_dma_cfg.s.swap32 ? "yes" : "no");
		printf("    58:    swap16:              %s\n",
		       ndf_dma_cfg.s.swap16 ? "yes" : "no");
		printf("    57:    swap8:               %s\n",
		       ndf_dma_cfg.s.swap8 ? "yes" : "no");
		printf("    56:    endian:              %s\n",
		       ndf_dma_cfg.s.endian ? "little" : "big");
		printf("    36-55: size:                %u\n",
		       ndf_dma_cfg.s.size);
		printf("    0-35:  adr:                 0x%llx\n",
		       (uint64_t)ndf_dma_cfg.s.adr);

		ndf_dma_int.u = mmc_read_csr(mmc, MIO_NDF_DMA_INT);
		printf("\nMIO_NDF_DMA_INT:              0x%016llx\n",
		       ndf_dma_int.u);
		printf("    0:     Done:    %s\n",
		       ndf_dma_int.s.done ? "yes" : "no");

		emm_cfg.u = mmc_read_csr(mmc, MIO_EMM_CFG);
		printf("\nMIO_EMM_CFG:                  0x%016llx\n",
		       emm_cfg.u);
		printf("    16:    boot_fail:           %s\n",
		       emm_cfg.s.boot_fail ? "yes" : "no");
		printf("    3:     bus_ena3:            %s\n",
		       emm_cfg.s.bus_ena & 0x08 ? "yes" : "no");
		printf("    2:     bus_ena2:            %s\n",
		       emm_cfg.s.bus_ena & 0x04 ? "yes" : "no");
		printf("    1:     bus_ena1:            %s\n",
		       emm_cfg.s.bus_ena & 0x02 ? "yes" : "no");
		printf("    0:     bus_ena0:            %s\n",
		       emm_cfg.s.bus_ena & 0x01 ? "yes" : "no");
	} else
#endif
	{
		emm_dma_cfg.u = mmc_read_csr(mmc, MIO_EMM_DMA_CFG);
		printf("MIO_EMM_DMA_CFG:                0x%016llx\n",
		       emm_dma_cfg.u);
		printf("    63:    en:                  %s\n",
		       emm_dma_cfg.s.en ? "enabled" : "disabled");
		printf("    62:    rw:                  %s\n",
		       emm_dma_cfg.s.rw ? "write" : "read");
		printf("    61:    clr:                 %s\n",
		       emm_dma_cfg.s.clr ? "clear" : "not clear");
		printf("    59:    swap32:              %s\n",
		       emm_dma_cfg.s.swap32 ? "yes" : "no");
		printf("    58:    swap16:              %s\n",
		       emm_dma_cfg.s.swap16 ? "yes" : "no");
		printf("    57:    swap8:               %s\n",
		       emm_dma_cfg.s.swap8 ? "yes" : "no");
		printf("    56:    endian:              %s\n",
		       emm_dma_cfg.s.endian ? "little" : "big");
		printf("    36-55: size:                %u\n",
		       emm_dma_cfg.s.size);

		emm_dma_adr.u = mmc_read_csr(mmc, MIO_EMM_DMA_ADR);
		printf("MIO_EMM_DMA_ADR:        0x%016llx\n", emm_dma_adr.u);
		printf("    0-49:  adr:                 0x%llx\n",
		       (uint64_t)emm_dma_adr.s.adr);

		emm_dma_int.u = mmc_read_csr(mmc, MIO_EMM_DMA_INT);
		printf("\nMIO_EMM_DMA_INT:              0x%016llx\n",
		       emm_dma_int.u);
		printf("    1:     FIFO:                %s\n",
		       emm_dma_int.s.fifo ? "yes" : "no");
		printf("    0:     Done:                %s\n",
		       emm_dma_int.s.done ? "yes" : "no");

		emm_cfg.u = mmc_read_csr(mmc, MIO_EMM_CFG);
		printf("\nMIO_EMM_CFG:                  0x%016llx\n",
		       emm_cfg.u);
#ifdef __mips
		printf("    16:    boot_fail:           %s\n",
		       emm_cfg.s.boot_fail ? "yes" : "no");
#endif
		printf("    3:     bus_ena3:            %s\n",
		       emm_cfg.s.bus_ena & 0x08 ? "yes" : "no");
		printf("    2:     bus_ena2:            %s\n",
		       emm_cfg.s.bus_ena & 0x04 ? "yes" : "no");
		printf("    1:     bus_ena1:            %s\n",
		       emm_cfg.s.bus_ena & 0x02 ? "yes" : "no");
		printf("    0:     bus_ena0:            %s\n",
		       emm_cfg.s.bus_ena & 0x01 ? "yes" : "no");
	}
	for (bus = 0; bus < 4; bus++) {
		emm_mode.u = mmc_read_csr(mmc, MIO_EMM_MODEX(bus));
		printf("\nMIO_EMM_MODE%u:               0x%016llx\n",
		       bus, emm_mode.u);
		printf("    48:    hs_timing:           %s\n",
		       emm_mode.s.hs_timing ? "yes" : "no");
		printf("    40-42: bus_width:           %s\n",
		       bus_width_str[emm_mode.s.bus_width]);
		printf("    32-35: power_class          %u\n",
		       emm_mode.s.power_class);
		printf("    16-31: clk_hi:              %u\n",
		       emm_mode.s.clk_hi);
		printf("    0-15:  clk_lo:              %u\n",
		       emm_mode.s.clk_lo);
	}

	emm_switch.u = mmc_read_csr(mmc, MIO_EMM_SWITCH);
	printf("\nMIO_EMM_SWITCH:               0x%016llx\n", emm_switch.u);
	printf("    60-61: tbus_id:             %u\n", emm_switch.s.bus_id);
	printf("    59:    switch_exe:          %s\n",
	       emm_switch.s.switch_exe ? "yes" : "no");
	printf("    58:    switch_err0:         %s\n",
	       emm_switch.s.switch_err0 ? "yes" : "no");
	printf("    57:    switch_err1:         %s\n",
	       emm_switch.s.switch_err1 ? "yes" : "no");
	printf("    56:    switch_err2:         %s\n",
	       emm_switch.s.switch_err2 ? "yes" : "no");
	printf("    48:    hs_timing:           %s\n",
	       emm_switch.s.hs_timing ? "yes" : "no");
	printf("    42-40: bus_width:           %s\n",
	       bus_width_str[emm_switch.s.bus_width]);
	printf("    32-35: power_class:         %u\n",
	       emm_switch.s.power_class);
	printf("    16-31: clk_hi:              %u\n",
	       emm_switch.s.clk_hi);
	printf("    0-15:  clk_lo:              %u\n", emm_switch.s.clk_lo);

	emm_dma.u = mmc_read_csr(mmc, MIO_EMM_DMA);
	printf("\nMIO_EMM_DMA:                  0x%016llx\n", emm_dma.u);
	printf("    60-61: bus_id:              %u\n", emm_dma.s.bus_id);
	printf("    59:    dma_val:             %s\n",
	       emm_dma.s.dma_val ? "yes" : "no");
	printf("    58:    sector:              %s mode\n",
	       emm_dma.s.sector ? "sector" : "byte");
	printf("    57:    dat_null:            %s\n",
	       emm_dma.s.dat_null ? "yes" : "no");
	printf("    51-56: thres:               %u\n", emm_dma.s.thres);
	printf("    50:    rel_wr:              %s\n",
	       emm_dma.s.rel_wr ? "yes" : "no");
	printf("    49:    rw:                  %s\n",
	       emm_dma.s.rw ? "write" : "read");
	printf("    48:    multi:               %s\n",
	       emm_dma.s.multi ? "yes" : "no");
	printf("    32-47: block_cnt:           %u\n",
	       emm_dma.s.block_cnt);
	printf("    0-31:  card_addr:           0x%x\n",
	       emm_dma.s.card_addr);

	emm_cmd.u = mmc_read_csr(mmc, MIO_EMM_CMD);
	printf("\nMIO_EMM_CMD:                  0x%016llx\n", emm_cmd.u);
	printf("    60-61: bus_id:              %u\n", emm_cmd.s.bus_id);
	printf("    59:    cmd_val:             %s\n",
	       emm_cmd.s.cmd_val ? "yes" : "no");
	printf("    55:    dbuf:                %u\n", emm_cmd.s.dbuf);
	printf("    49-54: offset:              %u\n", emm_cmd.s.offset);
	printf("    41-42: ctype_xor:           %s\n",
	       ctype_xor_str[emm_cmd.s.ctype_xor]);
	printf("    38-40: rtype_xor:           %s\n",
	       rtype_xor_str[emm_cmd.s.rtype_xor]);
	printf("    32-37: cmd_idx:             %u\n", emm_cmd.s.cmd_idx);
	printf("    0-31:  arg:                 0x%x\n", emm_cmd.s.arg);

	emm_rsp_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);
	printf("\nMIO_EMM_RSP_STS:              0x%016llx\n", emm_rsp_sts.u);
	printf("    60-61: bus_id:              %u\n", emm_rsp_sts.s.bus_id);
	printf("    59:    cmd_val:             %s\n",
	       emm_rsp_sts.s.cmd_val ? "yes" : "no");
	printf("    58:    switch_val:          %s\n",
	       emm_rsp_sts.s.switch_val ? "yes" : "no");
	printf("    57:    dma_val:             %s\n",
	       emm_rsp_sts.s.dma_val ? "yes" : "no");
	printf("    56:    dma_pend:            %s\n",
	       emm_rsp_sts.s.dma_pend ? "yes" : "no");
	printf("    28:    dbuf_err:            %s\n",
	       emm_rsp_sts.s.dbuf_err ? "yes" : "no");
	printf("    23:    dbuf:                %u\n", emm_rsp_sts.s.dbuf);
	printf("    22:    blk_timeout:         %s\n",
	       emm_rsp_sts.s.blk_timeout ? "yes" : "no");
	printf("    21:    blk_crc_err:         %s\n",
	       emm_rsp_sts.s.blk_crc_err ? "yes" : "no");
	printf("    20:    rsp_busybit:         %s\n",
	       emm_rsp_sts.s.rsp_busybit ? "yes" : "no");
	printf("    19:    stp_timeout:         %s\n",
	       emm_rsp_sts.s.stp_timeout ? "yes" : "no");
	printf("    18:    stp_crc_err:         %s\n",
	       emm_rsp_sts.s.stp_crc_err ? "yes" : "no");
	printf("    17:    stp_bad_sts:         %s\n",
	       emm_rsp_sts.s.stp_bad_sts ? "yes" : "no");
	printf("    16:    stp_val:             %s\n",
	       emm_rsp_sts.s.stp_val ? "yes" : "no");
	printf("    15:    rsp_timeout:         %s\n",
	       emm_rsp_sts.s.rsp_timeout ? "yes" : "no");
	printf("    14:    rsp_crc_err:         %s\n",
	       emm_rsp_sts.s.rsp_crc_err ? "yes" : "no");
	printf("    13:    rsp_bad_sts:         %s\n",
	       emm_rsp_sts.s.rsp_bad_sts ? "yes" : "no");
	printf("    12:    rsp_val:             %s\n",
	       emm_rsp_sts.s.rsp_val ? "yes" : "no");
	printf("    9-11:  rsp_type:            %s\n",
	       rtype_xor_str[emm_rsp_sts.s.rsp_type]);
	printf("    7-8:   cmd_type:            %s\n",
	       ctype_xor_str[emm_rsp_sts.s.cmd_type]);
	printf("    1-6:   cmd_idx:             %u\n",
	       emm_rsp_sts.s.cmd_idx);
	printf("    0:     cmd_done:            %s\n",
	       emm_rsp_sts.s.cmd_done ? "yes" : "no");

	emm_rsp_lo.u = mmc_read_csr(mmc, MIO_EMM_RSP_LO);
	printf("\nMIO_EMM_RSP_STS_LO:           0x%016llx\n", emm_rsp_lo.u);

	emm_rsp_hi.u = mmc_read_csr(mmc, MIO_EMM_RSP_HI);
	printf("\nMIO_EMM_RSP_STS_HI:           0x%016llx\n", emm_rsp_hi.u);

	emm_int.u = mmc_read_csr(mmc, MIO_EMM_INT);
	printf("\nMIO_EMM_INT:                  0x%016llx\n", emm_int.u);
	printf("    6:    switch_err:           %s\n",
	       emm_int.s.switch_err ? "yes" : "no");
	printf("    5:    switch_done:          %s\n",
	       emm_int.s.switch_done ? "yes" : "no");
	printf("    4:    dma_err:              %s\n",
	       emm_int.s.dma_err ? "yes" : "no");
	printf("    3:    cmd_err:              %s\n",
	       emm_int.s.cmd_err ? "yes" : "no");
	printf("    2:    dma_done:             %s\n",
	       emm_int.s.dma_done ? "yes" : "no");
	printf("    1:    cmd_done:             %s\n",
	       emm_int.s.cmd_done ? "yes" : "no");
	printf("    0:    buf_done:             %s\n",
	       emm_int.s.buf_done ? "yes" : "no");

	emm_wdog.u = mmc_read_csr(mmc, MIO_EMM_WDOG);
	printf("\nMIO_EMM_WDOG:                 0x%016llx (%u)\n",
	       emm_wdog.u, emm_wdog.s.clk_cnt);

	emm_sample.u = mmc_read_csr(mmc, MIO_EMM_SAMPLE);
	printf("\nMIO_EMM_SAMPLE:               0x%016llx\n", emm_sample.u);
	printf("    16-25: cmd_cnt:             %u\n", emm_sample.s.cmd_cnt);
	printf("    0-9:   dat_cnt:             %u\n", emm_sample.s.dat_cnt);

	emm_sts_mask.u = mmc_read_csr(mmc, MIO_EMM_STS_MASK);
	printf("\nMIO_EMM_STS_MASK:             0x%016llx\n", emm_sts_mask.u);

	emm_rca.u = mmc_read_csr(mmc, MIO_EMM_RCA);
	printf("\nMIO_EMM_RCA:                  0x%016llx\n", emm_rca.u);
	printf("    0-15:  card_rca:            %u\n", emm_rca.s.card_rca);
	puts("\n");
#endif /* DEBUG_REGISTERS */
}

/**
 * Enables the MMC bus, disabling NOR flash and other boot bus device access
 *
 * @param mmc - bus to enable
 *
 * NOTE: This is only needed for CN61XX, CNF7000 and CN7000.  All newer Octeon
 * and OcteonTX devices have a separate physical bus.
 */
static void mmc_enable(struct mmc *mmc)
{
#ifdef __mips
	union mio_emm_cfg emm_cfg;
	struct cavium_mmc_slot *slot = mmc->priv;

	debug("%s(%d)\n", __func__, slot->bus_id);

	emm_cfg.u = mmc_read_csr(mmc, MIO_EMM_CFG);
	emm_cfg.s.bus_ena |= 1 << slot->bus_id;
	mmc_write_csr(mmc, MIO_EMM_CFG, emm_cfg.u);

	debug("%s: Wrote 0x%llx to MIO_EMM_CFG\n", __func__, emm_cfg.u);
	debug("%s: MIO_EMM_MODE: 0x%llx\n", __func__,
	      mmc_read_csr(mmc, MIO_EMM_MODEX(slot->bus_id)));
	debug("%s: MIO_EMM_SWITCH: 0x%llx\n", __func__,
	      mmc_read_csr(mmc, MIO_EMM_SWITCH));
#endif
}

/**
 * Disables the MMC bus, enabling NOR flash and other boot bus device access
 * Note that there is an errata for a couple of Octeon (MIPS) processors where
 * sometimes the disable does not properly reset the subsystem so in this case
 * the process is repeated.
 *
 * @param mmc - bus to disable
 */
static void mmc_disable(struct mmc *mmc)
{
#ifdef __mips
	union mio_emm_cfg emm_cfg;
	struct cavium_mmc_slot *slot = mmc->priv;
	debug("%s(%d):\n", __func__, slot->bus_id);
again:
	emm_cfg.u = mmc_read_csr(mmc, MIO_EMM_CFG);
	emm_cfg.s.bus_ena &= ~(1 << slot->bus_id);
	debug("before:\n");
	mmc_print_registers(mmc);
	mmc_write_csr(mmc, MIO_EMM_CFG, emm_cfg.u);
	debug("after:\n");
	mmc_print_registers(mmc);

	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		/* McBuggin 26703 - EMMC CSR reset doesn't consistantly work
		 *
		 * Software workaround:
		 * 1. Clear the MIO_EMM_CFG[BUS_ENA] bits/
		 * 2. Read MIO_EMM_MODE0[CLK_HI] for the value 2500 which
		 *    indicates the CSRs are in reset.
		 * 3. If not in reset, set the MIO_EMM_CFG[BUS_ENA] bits and
		 *    repeat steps 1-3, otherwise exit.
		 */
		if (emm_cfg.s.bus_ena == 0) {
			union mio_emm_modex emm_mode;

			emm_mode.u = mmc_read_csr(mmc, MIO_EMM_MODEX(0));
			if (emm_mode.s.clk_hi != 2500) {
				debug("%s: reset failed, clk_hi: %d, try again\n",
				      __func__, (int)emm_mode.s.clk_hi);
				emm_cfg.s.bus_ena |= (1 << slot->bus_id);
				mmc_write_csr(mmc, MIO_EMM_CFG, emm_cfg.u);
				mdelay(1);
				goto again;
			}
		}
	}
#endif
}

/**
 * Sets the hardware MMC watchdog in microseconds
 *
 * @param[in]	mmc	pointer to MMC data structure
 * @param	timeout	How long to wait in usecs
 */
static void mmc_set_watchdog(const struct mmc *mmc, ulong timeout)
{
	union mio_emm_wdog emm_wdog;
	uint64_t val;
	struct cavium_mmc_slot *slot = mmc->priv;

	val = ((u64)timeout * mmc->clock) / 1000000;
	if (val >= (1 << 26)) {
		debug("%s: warning: timeout %lu exceeds max value %llu, truncating\n",
		      __func__, timeout,
		     (uint64_t)(((1ULL << 26) - 1) * 1000000ULL) / mmc->clock);
		val = (1 << 26) - 1;
	}
	emm_wdog.u = 0;
	emm_wdog.s.clk_cnt = val;

	debug("%s(%p(%s), %lu) clock: %u, period: %u, clk_cnt: %llu)\n",
	      __func__, mmc, mmc->cfg->name, timeout,
	      mmc->clock, slot->clk_period, emm_wdog.u);
	mmc_write_csr(mmc, MIO_EMM_WDOG, emm_wdog.u);
}

/**
 * Fills in the DMA configuration registers, clears interrupts, sets the
 * watchdog and starts the transfer to or from the MMC/SD card.
 *
 * @param mmc	pointer to MMC data structure
 * @param write	whether this is a write operation or not
 * @param clear	whether this is a DMA abort operation or not
 * @param block	starting block number to read/write
 * @param adr	physical address to DMA from/to
 * @param size	Number of blocks to transfer
 * @param timeout	timeout to set watchdog less than.
 */
static void mmc_start_dma(const struct mmc *mmc, bool write, bool clear,
			  uint32_t block, uint64_t adr, uint32_t size,
			  int timeout)
{
	const struct cavium_mmc_slot *slot = mmc->priv;
#ifdef __mips
	union mio_ndf_dma_cfg ndf_dma_cfg;
	union mio_ndf_dma_int ndf_dma_int;
#endif
	union mio_emm_dma_cfg emm_dma_cfg;
	union mio_emm_dma_adr emm_dma_adr;
	union mio_emm_dma_int emm_dma_int;
	union mio_emm_dma emm_dma;
	union mio_emm_int emm_int;

	debug("%s(%p(%d), %s, %s, 0x%x, 0x%llx, 0x%x, %d)\n", __func__, mmc,
	      slot->bus_id, write ? "true" : "false", clear ? "true" : "false",
	      block, adr, size, timeout);
#ifdef __mips
	if (host->use_ndf) {
		ndf_dma_int.u = 0;
		ndf_dma_int.s.done = 1;
		mmc_write_csr(mmc, MIO_NDF_DMA_INT, ndf_dma_int.u);

		ndf_dma_cfg.u = 0;
		ndf_dma_cfg.s.en = 1;
		ndf_dma_cfg.s.rw = !!write;
		ndf_dma_cfg.s.clr = !!clear;
		ndf_dma_cfg.s.size =
				((uint64_t)(size * mmc->read_bl_len) / 8) - 1;
		ndf_dma_cfg.s.adr = adr;
		debug("%s: Writing 0x%llx to mio_ndf_dma_cfg\n",
		      __func__, ndf_dma_cfg.u);
		mmc_write_csr(mmc, MIO_NDF_DMA_CFG, ndf_dma_cfg.u);
	} else
#endif
	{
		emm_dma_int.u = 0;
		emm_dma_int.s.done = 1;
		emm_dma_int.s.fifo = 1;
		mmc_write_csr(mmc, MIO_EMM_DMA_INT, emm_dma_int.u);

		emm_dma_cfg.u = 0;
		emm_dma_cfg.s.en = 1;
		emm_dma_cfg.s.rw = !!write;
		emm_dma_cfg.s.clr = !!clear;
		emm_dma_cfg.s.size =
				((uint64_t)(size * mmc->read_bl_len) / 8) - 1;
#if __BYTE_ORDER != __BIG_ENDIAN
		emm_dma_cfg.s.endian = 1;
#endif

		emm_dma_adr.u = 0;
		emm_dma_adr.s.adr = adr;
		debug("%s: Writing 0x%llx to mio_emm_dma_cfg and 0x%llx to mio_emm_dma_adr\n",
		      __func__, emm_dma_cfg.u, emm_dma_adr.u);
		mmc_write_csr(mmc, MIO_EMM_DMA_ADR, emm_dma_adr.u);
		mmc_write_csr(mmc, MIO_EMM_DMA_CFG, emm_dma_cfg.u);
	}
	emm_dma.u = 0;
	emm_dma.s.bus_id = slot->bus_id;
	emm_dma.s.dma_val = 1;
	emm_dma.s.rw = write;
	emm_dma.s.sector = mmc->high_capacity ? 1 : 0;
	/* NOTE: For SD we can only support multi-block transfers if
	 * bit 33 (CMD_SUPPORT) is set in the SCR register.
	 */
	if ((size > 1) &&
	    ((IS_SD(mmc) && (slot->flags & OCTEON_MMC_FLAG_SD_CMD23)) ||
	     !IS_SD(mmc)))
		emm_dma.s.multi = 1;
	else
		emm_dma.s.multi = 0;

	emm_dma.s.block_cnt = size;
	if (!mmc->high_capacity)
		block *= mmc->read_bl_len;
	emm_dma.s.card_addr = block;

	debug("%s: card address: 0x%x, size: %d, multi: %d\n",
	      __func__, block, size, emm_dma.s.multi);
	/* Clear interrupt */
	emm_int.u = mmc_read_csr(mmc, MIO_EMM_INT);
	mmc_write_csr(mmc, MIO_EMM_INT, emm_int.u);

	mmc_set_watchdog(mmc, timeout * 1000 - 1000);

	debug("%s: Writing 0x%llx to mio_emm_dma\n", __func__, emm_dma.u);
	mmc_write_csr(mmc, MIO_EMM_DMA, emm_dma.u);
}

/**
 * This function must be called when it is possible that the MMC bus has changed
 *
 * @param mmc - pointer to MMC data structure
 */
static inline void mmc_switch_dev(struct mmc *mmc)
{
	union mio_emm_switch emm_switch;
	union mio_emm_sample emm_sample;
	union mio_emm_rca emm_rca;
	union mio_emm_sts_mask emm_sts_mask;
	struct cavium_mmc_slot *slot = mmc->priv;
	struct cavium_mmc_host *host = slot->host;

	debug("%s(%s): bus_id: %d, last: %d\n", __func__, mmc->cfg->name,
	      host->cur_slotid, host->last_slotid);

	if (host->cur_slotid == host->last_slotid) {
		debug("%s: No change\n", __func__);
		return;
	}
	if (!mmc->has_init)
		debug("%s: %s is not initialized\n", __func__, mmc->cfg->name);
	emm_switch.u = mmc_read_csr(mmc, MIO_EMM_SWITCH);
#if defined(DEBUG)
	emm_rca.u = mmc_read_csr(mmc, MIO_EMM_RCA);
	debug("%s(%s): Switching from:\n"
	      "    bus id: %d, clock period: %d, width: %d, rca: 0x%x, high speed: %d to\n"
	      "    bus id: %d, clock period: %d, width: %d, rca: 0x%x, high speed: %d\n",
	      __func__, mmc->cfg->name,
	      host->last_slotid, emm_switch.s.clk_lo * 2,
	      emm_switch.s.bus_width, emm_rca.s.card_rca,
	      emm_switch.s.hs_timing,
	      slot->bus_id, slot->clk_period, slot->bus_width, mmc->rca,
	      !!(mmc->card_caps & MMC_MODE_HS));
#endif
	/* mmc_write_csr(mmc, MIO_EMM_CFG, 1 << slot->bus_id);*/
	emm_switch.s.bus_width = slot->bus_width;
	emm_switch.s.hs_timing = mmc->clock > 20000000;
	debug("%s: hs timing: %d, caps: 0x%x, bus_width: %d (%d bits)\n",
	      __func__, emm_switch.s.hs_timing, mmc->card_caps,
	      (int)emm_switch.s.bus_width, mmc->bus_width);
	emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
	emm_switch.s.clk_lo = (slot->clk_period + 1) / 2;
	emm_switch.s.bus_id = slot->bus_id;
	emm_switch.s.power_class = slot->power_class;
	debug("%s: Setting MIO_EMM_SWITCH to 0x%llx\n", __func__, emm_switch.u);
	emm_switch.s.bus_id = 0;
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	udelay(100);
	emm_switch.s.bus_id = slot->bus_id;
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);

	debug("Switching RCA from 0x%llx to 0x%x\n",
	      mmc_read_csr(mmc, MIO_EMM_RCA), mmc->rca);
	emm_rca.u = 0;
	emm_rca.s.card_rca = mmc->rca;
	mmc_write_csr(mmc, MIO_EMM_RCA, emm_rca.u);
	host->last_slotid = host->cur_slotid;
	mdelay(100);
	/* Update the watchdog to 100 ms */
	mmc_set_watchdog(mmc, 100000);

	emm_sample.u = 0;
	emm_sample.s.cmd_cnt = slot->cmd_clk_skew;
	emm_sample.s.dat_cnt = slot->dat_clk_skew;
	mmc_write_csr(mmc, MIO_EMM_SAMPLE, emm_sample.u);
	/* Set status mask */
	emm_sts_mask.u = 0;
	emm_sts_mask.s.sts_msk = 1 << 7 | 1 << 22 | 1 << 23 | 1 << 19;
	mmc_write_csr(mmc, MIO_EMM_STS_MASK, emm_sts_mask.u);

	debug("%s: MIO_EMM_MODE(%d): 0x%llx, MIO_EMM_SWITCH: 0x%llx\n",
	      __func__, slot->bus_id,
	      mmc_read_csr(mmc, MIO_EMM_MODEX(slot->bus_id)),
	      mmc_read_csr(mmc, MIO_EMM_SWITCH));
	mdelay(10);
	mmc_print_registers(mmc);
}

/**
 * Reads one or more sectors into memory
 *
 * @param mmc	mmc data structure
 * @param src	source sector number
 * @param dst	pointer to destination address to read into
 * @param size	number of sectors to read
 *
 * @return number of sectors read
 */
int cavium_mmc_read(struct mmc *mmc, u64 src, uchar *dst, int size)
{
	struct mmc_cmd cmd;
	ulong start_time;
	uint64_t dma_addr;
	union mio_emm_dma emm_dma;
	union mio_emm_dma_int emm_dma_int;
	union mio_emm_rsp_sts rsp_sts;
	union mio_emm_int emm_int;
	union mio_emm_sts_mask emm_sts_mask;
#ifdef __mips
	union mio_ndf_dma_int ndf_dma_int;
#endif
	int timeout;
	int dma_retry_count = 0;
	bool timed_out = false;
	struct cavium_mmc_slot *slot = mmc->priv;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);

	debug("%s(%s, src: 0x%llx, dst: 0x%p, size: %d)\n", __func__,
	      mmc->cfg->name, src, dst, size);
	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return 0;
	}
#ifdef DEBUG
	memset(dst, 0xEE, size * mmc->read_bl_len);
#endif
	mmc_switch_dev(mmc);
	if (!IS_SD(mmc) || (IS_SD(mmc) && mmc->high_capacity)) {
		debug("Setting block length to %d\n", mmc->read_bl_len);
		mmc_set_blocklen(mmc, mmc->read_bl_len);
	}

	/* Enable appropriate errors */
	emm_sts_mask.u = 0;
	emm_sts_mask.s.sts_msk = R1_BLOCK_READ_MASK;
	mmc_write_csr(mmc, MIO_EMM_STS_MASK, emm_sts_mask.u);
	debug("%s: MIO_EMM_STS_MASK: 0x%llx\n", __func__, emm_sts_mask.u);
	dma_addr = (uint64_t)dm_pci_virt_to_mem(mmc->dev, dst);
	debug("%s: dma address: 0x%llx\n", __func__, dma_addr);

	timeout = 1000 + size;
	mmc_set_watchdog(mmc, timeout * 1000 - 100);

	invalidate_dcache_range((ulong)src,
				(ulong)src + size * bdesc->blksz);
	mmc_start_dma(mmc, false, false, src, dma_addr, size, timeout);

retry_dma:
	debug("%s: timeout: %d\n", __func__, timeout);
	start_time = get_timer(0);
#ifdef __mips
	if (host->use_ndf) {
		do {
			ndf_dma_int.u = mmc_read_csr(mmc, MIO_NDF_DMA_INT);
#ifdef DEBUG
			debug("%s: ndf_dma_int: 0x%llx, ndf_dma_cfg: 0x%llx, mio_emm_dma: 0x%llx, rsp sts: 0x%llx, time: %lu\n",
			      __func__, ndf_dma_int.u,
			      mmc_read_csr(mmc, MIO_NDF_DMA_CFG),
			      mmc_read_csr(mmc, MIO_EMM_DMA),
			      mmc_read_csr(mmc, MIO_EMM_RSP_STS),
			      get_timer(start_time));
#endif
			if (ndf_dma_int.s.done) {
				mmc_write_csr(mmc, MIO_NDF_DMA_INT,
					      ndf_dma_int.u);
				debug("%s: DMA completed normally\n", __func__);
				break;
			}

			WATCHDOG_RESET();
			timed_out = get_timer(start_time) > timeout;
			rsp_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);

			if (rsp_sts.s.dma_pend) {
				printf("%s: DMA error, rsp status: 0x%llx\n",
				       __func__, rsp_sts.u);
				break;
			}
		} while (!timed_out);
		timed_out |= !ndf_dma_int.s.done;
	} else
#endif
	{
		do {
			emm_dma_int.u = mmc_read_csr(mmc,
						       MIO_EMM_DMA_INT);
			debug("%s: mio_emm_dma_int: 0x%llx\n", __func__,
			      emm_dma_int.u);

			if (emm_dma_int.s.done) {
				mmc_write_csr(mmc, MIO_EMM_DMA_INT,
					      emm_dma_int.u);
				break;
			}

			WATCHDOG_RESET();
		} while (get_timer(start_time) < timeout);
		timed_out = !emm_dma_int.s.done;
	}
	rsp_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);
	debug("%s: rsp_sts: 0x%llx\n", __func__, rsp_sts.u);
	if (timed_out || rsp_sts.s.dma_val || rsp_sts.s.dma_pend) {
		emm_int.u = mmc_read_csr(mmc, MIO_EMM_INT);
		mmc_print_registers(mmc);
		if (timed_out) {
			printf("%s(%s, 0x%llx, %p, %d)\n",
			       __func__, mmc->cfg->name, src, dst, size);
			printf("MMC read DMA timeout, status: 0x%llx, interrupt status: 0x%llx\n",
			       rsp_sts.u, emm_int.u);
			printf("    MIO_EMM_DMA: 0x%llx, last command: %d\n",
			       emm_dma.u, rsp_sts.s.cmd_idx);
			printf("    MIO_EMM_RSP_LO: 0x%llx, HI: 0x%llx\n",
			       mmc_read_csr(mmc, MIO_EMM_RSP_LO),
			       mmc_read_csr(mmc, MIO_EMM_RSP_HI));
			printf("    MIO_EMM_MODE(%d): 0x%llx\n", slot->bus_id,
			       mmc_read_csr(mmc, MIO_EMM_MODEX(slot->bus_id)));
			printf("    MIO_EMM_DMA_CFG: 0x%llx\n",
			       mmc_read_csr(mmc, MIO_EMM_DMA_CFG));
		}
		if (rsp_sts.s.blk_timeout)
			printf("Block timeout error detected\n");
		if (rsp_sts.s.blk_crc_err)
			printf("Block CRC error detected\n");
		if (dma_retry_count++ < 3) {
			/* DMA still pending, terminate it */
#ifdef DEBUG
			if (rsp_sts.s.dma_pend)
				debug("%s: rsp_sts_low: 0x%llx\n", __func__,
				      mmc_read_csr(mmc, MIO_EMM_RSP_LO));
#endif
			emm_dma.u = mmc_read_csr(mmc, MIO_EMM_DMA);
			emm_dma.s.dma_val = 1;
			emm_dma.s.dat_null = 1;
			mmc_write_csr(mmc, MIO_EMM_DMA, emm_dma.u);
			start_time = get_timer(0);
			do {
				rsp_sts.u = mmc_read_csr(mmc,
							   MIO_EMM_RSP_STS);
				if (rsp_sts.s.dma_val == 0)
					break;
				udelay(1);
			} while (get_timer(start_time) < timeout);
			timeout += 1000 + size;
#ifdef DEBUG
			print_buffer(0, dst, 1, 512, 0);
#endif
			debug("Retrying MMC read DMA\n");
			goto retry_dma;
		} else {
			printf("mmc read block %llu, size %d DMA failed, terminating...\n",
			       src, size);
			emm_dma.s.dma_val = 1;
			emm_dma.s.dat_null = 1;
			mmc_write_csr(mmc, MIO_EMM_DMA, emm_dma.u);
			timeout = 1000 + size;
			start_time = get_timer(0);
#ifdef __mips
			if (host->use_ndf)
				do {
					ndf_dma_int.u =
						mmc_read_csr(mmc,
							     MIO_NDF_DMA_INT);
					if (ndf_dma_int.s.done)
						break;
					udelay(1);
					WATCHDOG_RESET();
					timed_out =
					    (get_timer(start_time) > timeout);
				} while (!timed_out);
			else
#endif
				do {
					emm_dma_int.u =
						mmc_read_csr(mmc, MIO_EMM_DMA_INT);
					if (emm_dma_int.s.done)
						break;
					udelay(1);
					WATCHDOG_RESET();
					timed_out =
					    (get_timer(start_time) > timeout);
				} while (!timed_out);
			if (timed_out)
				puts("Error: MMC read DMA failed to terminate!\n");

			return 0;
		}
	}

	if (dma_retry_count)
		debug("Success after %d DMA retries\n", dma_retry_count);

	if (timed_out) {
		printf("MMC read block %llu timed out\n", src);
		debug("Read status 0x%llx\n", rsp_sts.u);
		emm_dma.u = mmc_read_csr(mmc, MIO_EMM_DMA);
		debug("EMM_DMA: 0x%llx\n", emm_dma.u);

		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (cavium_mmc_send_cmd(mmc->dev, &cmd, NULL))
			printf("Error sending stop transmission cmd\n");
		return 0;
	}
	debug("Read status 0x%llx\n", rsp_sts.u);

	emm_dma.u = mmc_read_csr(mmc, MIO_EMM_DMA);
	debug("EMM_DMA: 0x%llx\n", emm_dma.u);
#if defined(DEBUG)
	print_buffer(0, dst, 1, 512, 0);
#endif
	debug("%s(%s): return %d\n", __func__, mmc->cfg->name,
	      size - emm_dma.s.block_cnt);
	return size - emm_dma.s.block_cnt;
}

/**
 * Writes sectors to MMC device
 *
 * @param[in,out] mmc - MMC device to write to
 * @param start - starting sector to write to
 * @param size - number of sectors to write
 * @param src - pointer to source address of buffer containing sectors
 *
 * @return number of sectors or 0 if none.
 *
 * NOTE: This checks the GPIO write protect if it is present
 */
static ulong
mmc_write(struct mmc *mmc, ulong start, int size, const void *src)
{
	uint64_t dma_addr;
	union mio_emm_dma emm_dma;
	union mio_emm_dma_int emm_dma_int;
	union mio_emm_rsp_sts rsp_sts;
	union mio_emm_int emm_int;
#ifdef __mips
	union mio_ndf_dma_int ndf_dma_int;
#endif
	struct mmc_cmd cmd;
	int timeout;
	int dma_retry_count = 0;
	int rc;
	ulong start_time;
	bool timed_out = false;
	struct cavium_mmc_slot *slot = mmc->priv;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);

	debug("%s(start: %lu, size: %d, src: 0x%p)\n", __func__, start,
	      size, src);
	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return 0;
	}

	mmc_switch_dev(mmc);

	/* Poll for ready status */
	timeout = 10000;	/* 10 seconds */
	start_time = get_timer(0);
	timed_out = true;
	do {
		memset(&cmd, 0, sizeof(cmd));
		cmd.cmdidx = MMC_CMD_SEND_STATUS;
		cmd.cmdarg = mmc->rca << 16;
		cmd.resp_type = MMC_RSP_R1;
		rc = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
		if (rc) {
			printf("%s: Error getting device status\n", __func__);
			return 0;
		} else if (cmd.response[0] & R1_READY_FOR_DATA) {
			timed_out = false;
			break;
		}
		udelay(1);
		timed_out = (get_timer(start_time) > timeout);
	} while (!timed_out);
	debug("%s: Device status: 0x%x\n", __func__, cmd.response[0]);
	if (timed_out) {
		printf("%s: Device timed out waiting for empty buffer, response: 0x%x\n",
		       __func__, cmd.response[0]);
		return 0;
	}

	flush_dcache_range((ulong)src,
			   (ulong)src + size * bdesc->blksz);
	dma_addr = (uint64_t)dm_pci_virt_to_mem(mmc->dev, (void *)src);
	timeout = 5000 + 5000 * size;

	mmc_start_dma(mmc, true, false, start, dma_addr, size, timeout);

retry_dma:
	start_time = get_timer(0);
	do {
#ifdef DEBUG
		if (ctrlc()) {
			printf("Interrupted by user\n");
			break;
		}
#endif

		rsp_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);
		if (((rsp_sts.s.dma_val == 0) || (rsp_sts.s.dma_pend == 1)) &&
		    rsp_sts.s.cmd_done)
			break;

		WATCHDOG_RESET();
		udelay(1);
		timed_out = (get_timer(start_time) >= timeout);
	} while (!timed_out);

	if (timed_out) {
		printf("%s: write command completion timeout for cmd %d\n",
		       __func__, rsp_sts.s.cmd_idx);
	}
	/*rsp_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);*/
	debug("emm_rsp_sts: 0x%llx, cmd: %d, response: 0x%llx\n",
	      rsp_sts.u, rsp_sts.s.cmd_idx,
	      mmc_read_csr(mmc, MIO_EMM_RSP_LO));
	if (rsp_sts.s.cmd_val || timed_out || rsp_sts.s.dma_val ||
	    rsp_sts.s.dma_pend) {
		emm_dma.u = mmc_read_csr(mmc, MIO_EMM_DMA);
		emm_int.u = mmc_read_csr(mmc, MIO_EMM_INT);
#ifdef __mips
		printf("%s: Error detected: MIO_EMM_RSP_STS: 0x%llx, MIO_EMM_DMA: 0x%llx,\n"
		       "    %s: 0x%llx, timeout: %d\n",
		       __func__, rsp_sts.u, emm_dma.u,
		       host->use_ndf ? "MIO_NDF_DMA_CFG" : "MIO_EMM_DMA_CFG",
		       host->use_ndf ? mmc_read_csr(mmc, MIO_NDF_DMA_CFG) :
		       mmc_read_csr(mmc, MIO_EMM_DMA_CFG), timeout);
#else
		printf("%s: Error detected: MIO_EMM_RSP_STS: 0x%llx, MIO_EMM_DMA: 0x%llx,\n"
		       "    MIO_EMM_DMA_CFG: 0x%llx, timeout: %d\n",
		       __func__, rsp_sts.u, emm_dma.u,
		       mmc_read_csr(mmc, MIO_EMM_DMA_CFG), timeout);
#endif
		printf("Last command index: %d\n", rsp_sts.s.cmd_idx);
		printf("emm_int: 0x%llx\n", emm_int.u);
		mdelay(10);
		rsp_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);
		printf("Re-read rsp_sts: 0x%llx, cmd_idx: %d\n", rsp_sts.u,
		       rsp_sts.s.cmd_idx);
		printf("  RSP_LO: 0x%llx\n", mmc_read_csr(mmc, MIO_EMM_RSP_LO));
		if (timed_out) {
			printf("%s(mmc, 0x%lx, %d, 0x%p)\n",
			       __func__, start, size, src);
			printf("MMC write DMA timeout, status: 0x%llx, interrupt status: 0x%llx\n",
			       rsp_sts.u, emm_int.u);
			printf("    MIO_EMM_DMA: 0x%llx, last command: %d\n",
			       emm_dma.u, rsp_sts.s.cmd_idx);
		} else {
			if (rsp_sts.s.blk_timeout)
				printf("Block timeout error detected\n");
			if (rsp_sts.s.blk_crc_err)
				printf("Block CRC error detected\n");
			if (rsp_sts.s.dma_val)
				printf("DMA still valid\n");
		}

		if (dma_retry_count++ < 3 && rsp_sts.s.dma_pend) {
			/* DMA still pending, terminate it */
			emm_dma.s.dma_val = 1;
			timeout = 2000 * size;
			cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
			cmd.cmdarg = 0;
			cmd.resp_type = MMC_RSP_R1b;
			cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
			mmc_write_csr(mmc, MIO_EMM_DMA, emm_dma.u);
			debug("Retrying MMC write DMA\n");
			goto retry_dma;
		} else {
			emm_dma.s.dma_val = 1;
			emm_dma.s.dat_null = 1;
			mmc_write_csr(mmc, MIO_EMM_DMA, emm_dma.u);
			start_time = get_timer(0);
#ifdef __mips
			if (host->use_ndf)
				do {
					ndf_dma_int.u =
						mmc_read_csr(mmc,
							     MIO_NDF_DMA_INT);
					if (ndf_dma_int.s.done)
						break;
					udelay(1);
				} while (get_timer(start_time) < timeout);
			else
#endif
				do {
					emm_dma_int.u =
						mmc_read_csr(mmc,
							     MIO_EMM_DMA_INT);
					if (emm_dma_int.s.done)
						break;
					udelay(1);
				} while (get_timer(start_time) < timeout);
			if (timeout <= 0)
				puts("Error: MMC write DMA failed to terminate!\n");
			return 0;
		}
	}

	if (dma_retry_count && !timed_out)
		debug("Success after %d DMA retries\n", dma_retry_count);

	if (timed_out) {
		printf("MMC write block %lu timed out\n", start);
		debug("Write status 0x%llx\n", rsp_sts.u);
		emm_dma.u = mmc_read_csr(mmc, MIO_EMM_DMA);
		debug("EMM_DMA: 0x%llx\n", emm_dma.u);

		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (cavium_mmc_send_cmd(mmc->dev, &cmd, NULL))
			printf("Error sending stop transmission cmd\n");
		return 0;
	}
	debug("Write status 0x%llx\n", rsp_sts.u);

	/* Poll status if we can't send data right away */
	if (!((rsp_sts.s.cmd_idx == MMC_CMD_SEND_STATUS) &&
	      rsp_sts.s.cmd_done &&
	      ((mmc_read_csr(mmc, MIO_EMM_RSP_LO) >> 8) &
	       R1_READY_FOR_DATA))) {
		/* Poll for ready status */
		timeout = 10000;	/* 10 seconds */
		start_time = (get_timer(0));
		do {
			memset(&cmd, 0, sizeof(cmd));
			cmd.cmdidx = MMC_CMD_SEND_STATUS;
			cmd.cmdarg = mmc->rca << 16;
			cmd.resp_type = MMC_RSP_R1;
			rc = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
			if (rc) {
				printf("%s: Error getting post device status\n",
				       __func__);
				return 0;
			}
			if (cmd.response[0] & R1_READY_FOR_DATA)
				break;
			mdelay(1);
			timed_out = get_timer(start_time) > timeout;
		} while (!timed_out);
		if (timed_out) {
			printf("%s: Device timed out waiting for empty buffer\n",
			       __func__);
			return 0;
		}
	}

	emm_dma.u = mmc_read_csr(mmc, MIO_EMM_DMA);
	debug("EMM_DMA: 0x%llx\n", emm_dma.u);

	return size - emm_dma.s.block_cnt;
}

static ulong mmc_erase_t(struct mmc *mmc, ulong start, lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	ulong end;
	int err, start_cmd, end_cmd;

	if (mmc->high_capacity) {
		end = start + blkcnt - 1;
	} else {
		end = (start + blkcnt - 1) * mmc->write_bl_len;
		start *= mmc->write_bl_len;
	}

	if (!mmc_getcd(mmc)) {
		mmc->has_init = 0;
		printf("%s: Error: Card not detected\n", __func__);
		return 0;
	}

	if (IS_SD(mmc)) {
		start_cmd = SD_CMD_ERASE_WR_BLK_START;
		end_cmd = SD_CMD_ERASE_WR_BLK_END;
	} else {
		start_cmd = MMC_CMD_ERASE_GROUP_START;
		end_cmd = MMC_CMD_ERASE_GROUP_END;
	}

	cmd.cmdidx = start_cmd;
	cmd.cmdarg = start;
	cmd.resp_type = MMC_RSP_R1;

	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = end_cmd;
	cmd.cmdarg = end;
	cmd.resp_type = MMC_RSP_R1b;

	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = MMC_CMD_ERASE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R1b;

	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err) {
		printf("%s err: %d\n", __func__, err);
		goto err_out;
	}

	return 0;

err_out:
	printf("mmc erase failed, err: %d\n", err);
	return err;
}

/**
 * Reads blkcnt sectors from the specified device into the specified buffer
 * @param block_dev	Device to read from
 * @param start		Starting sector number
 * @param blkcnt	Number of sectors to read
 * @param[out] dst	Pointer to buffer to contain the data
 *
 * @return number of blocks read or 0 if error
 */
unsigned long mmc_bread(struct udevice *dev, lbaint_t start,
			       lbaint_t blkcnt, void *dst)
{
	debug("%s ->1 dev %p\n", __func__, dev);
	debug("%s ->1 parent %p\n", __func__, dev->parent);
	lbaint_t cur, blocks_todo = blkcnt;
	struct cavium_mmc_host *host = dev_get_priv(dev->parent);
	debug("%s ->2 %p\n", __func__, host);
	struct cavium_mmc_slot *slot = &host->slots[host->cur_slotid];
	struct mmc *mmc = slot->mmc;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);
	unsigned char bounce_buffer[4096];

	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return 0;
	}
	debug("%s(%d %llu, %llu, %p) mmc: %p\n", __func__, bdesc->devnum,
	      (uint64_t)start, (uint64_t)blkcnt, dst, mmc);
	if (!mmc) {
		printf("%s: MMC device %d not found\n", __func__, bdesc->devnum);
		return 0;
	}

	if (blkcnt == 0)
		return 0;

	if ((start + blkcnt) > bdesc->lba) {
		printf("MMC: block number 0x%llx exceeds max(0x%llx)\n",
		       (uint64_t)(start + blkcnt),
		       (uint64_t)bdesc->lba);
		return 0;
	}

	if (!mmc_getcd(mmc)) {
		mmc->has_init = 0;
		printf("%s: Error: Card not detected\n", __func__);
		return 0;
	}

	mmc_enable(mmc);
	if (((ulong)dst) & 7) {
		debug("%s: Using bounce buffer due to alignment\n", __func__);
		do {
			if (cavium_mmc_read(mmc, start, bounce_buffer, 1) != 1)
				return 0;
			memcpy(dst, bounce_buffer, mmc->read_bl_len);
			WATCHDOG_RESET();
			dst += mmc->read_bl_len;
			start++;
			blocks_todo--;
		} while (blocks_todo > 0);
	} else {
		do {
			cur = min(blocks_todo, (lbaint_t)(mmc->cfg->b_max));
			if (cavium_mmc_read(mmc, start, dst, cur) != cur) {
				blkcnt = 0;
				break;
			}
			WATCHDOG_RESET();
			blocks_todo -= cur;
			start += cur;
			dst += cur * mmc->read_bl_len;
		} while (blocks_todo > 0);
	}
	mmc_disable(mmc);

	return blkcnt;
}

/**
 * Writes blkcnt sectors to the specified device from the specified buffer
 * @param block_dev	Device to write to
 * @param start		Starting sector number
 * @param blkcnt	Number of sectors to write
 * @param[in] src	Pointer to buffer that contains the data
 *
 * @return number of blocks written or 0 if error
 */
ulong mmc_bwrite(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		 const void *src)
{
	lbaint_t cur, blocks_todo = blkcnt;
	struct cavium_mmc_host *host = dev_get_priv(dev->parent);
	struct cavium_mmc_slot *slot = &host->slots[host->cur_slotid];
	struct mmc *mmc = slot->mmc;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);
	int dev_num;
	unsigned char bounce_buffer[4096];

	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return 0;
	}
	dev_num = bdesc->devnum;
	debug("%s(%d, %llu, %llu, %p)\n", __func__, dev_num, (uint64_t)start,
	      (uint64_t)blkcnt, src);
	if (!mmc) {
		printf("MMC Write: device %d not found\n", dev_num);
		return 0;
	}

	if (blkcnt == 0)
		return 0;
	if ((start + blkcnt) > bdesc->lba) {
		printf("MMC: block number 0x%llx exceeds max(0x%llx)\n",
		       (uint64_t)(start + blkcnt),
		       (uint64_t)bdesc->lba);
		return 0;
	}
	if (!mmc_getcd(mmc)) {
		mmc->has_init = 0;
		printf("%s: Error: Card not detected\n", __func__);
		return 0;
	}

	if (mmc_getwp(mmc)) {
		printf("%s: Failed due to write protect switch\n", __func__);
		return 0;
	}
	mmc_enable(mmc);
	if (((ulong)src) & 7) {
		debug("%s: Using bounce buffer due to alignment\n", __func__);
		do {
			memcpy(bounce_buffer, src, mmc->write_bl_len);
			if (mmc_write(mmc, start, 1, bounce_buffer) != 1)
				return 0;
			WATCHDOG_RESET();
			src += mmc->write_bl_len;
			start++;
			blocks_todo--;
		} while (blocks_todo > 0);
	} else {
		do {
			ulong ret;
			cur = min(blocks_todo, (lbaint_t)(mmc->cfg->b_max));
			ret = mmc_write(mmc, start, cur, src);
			if (ret != cur) {
				printf("%s: ERROR: Not enough blocks written (%lu != %u)\n",
				       __func__, ret, (u32)cur);
				blkcnt = 0;
				break;
			}
			WATCHDOG_RESET();
			blocks_todo -= cur;
			start += cur;
			src += cur * mmc->write_bl_len;
		} while (blocks_todo > 0);
	}
	mmc_disable(mmc);
	return blkcnt;
}

/**
 * Performs a block erase operation on a MMC or SD device
 *
 * @param	block_dev	Device to erase
 * @param	start		Starting block number
 * @param	blkcnt		Number of blocks to erase
 *
 * @return	Number of sectors actually erased or 0 if error
 */
ulong mmc_berase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt)
{
	int err = 0;
	struct cavium_mmc_host *host = dev_get_priv(dev->parent);
	struct cavium_mmc_slot *slot = &host->slots[host->cur_slotid];
	struct mmc *mmc = slot->mmc;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);
	int dev_num = bdesc->devnum;

	if (!mmc) {
		printf("%s: MMC device not found\n", __func__);
		return 0;
	}
	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return 0;
	}

	debug("%s(%d, 0x%llx, 0x%llx)\n", __func__, dev_num,
	      (unsigned long long)start, (unsigned long long)blkcnt);
	if ((start % mmc->erase_grp_size) || (blkcnt % mmc->erase_grp_size))
		printf("\n\nCaution!  Your device's erase group is 0x%x\n"
		       "The erase range would be changed to 0x%llx~0x%llx\n\n",
		       mmc->erase_grp_size,
		       (uint64_t)(start & ~(mmc->erase_grp_size - 1)),
		       (uint64_t)((start + blkcnt + mmc->erase_grp_size)
				  & ~(mmc->erase_grp_size - 1)) - 1);

	mmc_enable(mmc);
	err = mmc_erase_t(mmc, start, blkcnt);
	mmc_disable(mmc);

	return err ? 0 : blkcnt;
}

/**
 * Given an array of 32-bit values, calculate the CRC7 on it while ignoring
 * the last byte.
 *
 * @param[in]	data	data to calculate CRC on
 * @param	count	number of words to calculate over
 *
 * @return	crc7 value left shifted by 1 with LSB set to 1.
 */
static uint8_t mmc_crc7_32(const uint32_t *data, int count)
{
	uint8_t crc = 0;
	int index;
	uint8_t d;
	int shift = 24;
	int i;

	count = count * sizeof(uint32_t) - 1;

	for (index = 0; index < count; index++) {
		d = (data[index / sizeof(uint32_t)] >> shift) & 0xff;
		shift = shift >= 8 ? shift - 8 : 24;
		for (i = 0; i < 8; i++) {
			crc <<= 1;
			if ((d & 0x80) ^ (crc & 0x80))
				crc ^= 0x09;
			d <<= 1;
		}
	}
	crc = (crc << 1) | 1;

	return crc;
}

/**
 * Sets the DSR register in the MMC data structure, does not write to hw
 *
 * @param	mmc	mmc data structure
 * @param	val	value to set DSR to
 *
 * @return	0 for success
 */
int mmc_set_dsr(struct mmc *mmc, u16 val)
{
	mmc->dsr = val;
	return 0;
}

/**
 * Prints out detailed device information
 *
 * @param	mmc	MMC data structure pointer
 */
void print_mmc_device_info(struct mmc *mmc)
{
	const struct cavium_mmc_slot *slot = mmc->priv;
	const char *type;
	const char *version;
	const uint8_t *ext_csd = slot->ext_csd;
	uint32_t card_type;
	int prev = 0;
	int i;
	static const char *cbx_str[4] = {
		"Card (removable)",
		"BGA (Discrete embedded)",
		"POP",
		"Reserved"
	};
	static const char *pwr_classes[16] = {
		"100", "120", "150", "180", "200", "220", "250", "300",
		"350", "400", "450", "500", "600", "700", "800", ">800",
	};
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);

	printf("Register base address: %p\n", slot->host->base_addr);
	debug("MIO_EMM_MODE: 0x%llx\n",
	      mmc_read_csr(mmc, MIO_EMM_MODEX(slot->bus_id)));

	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return;
	}
	if (!mmc_getcd(mmc)) {
		mmc->has_init = 0;
		printf("Card not detected\n");
		return;
	}

	card_type = slot->ext_csd[EXT_CSD_CARD_TYPE];
	if (IS_SD(mmc)) {
		if (mmc->high_capacity)
			type = "SDHC or SDXC";
		else
			type = "SD";
	} else {
		type = "MMC";
	}

	switch (mmc->version) {
#ifdef CONFIG_CAVIUM_MMC_SD
	case SD_VERSION_2:
	case SD_VERSION_3:
	case SD_VERSION_4:	/* Decode versions 2-4 here */
		if (mmc->scr[0] & (1 << 16)) {
			if (mmc->scr[0] & (1 << 10))
				version = "SD 4.XX";
			else
				version = "SD 3.0X";
		} else {
			version = "SD 2.00";
		}
		break;
	case SD_VERSION_1_10:
		version = "SD 1.10";
		break;
	case SD_VERSION_1_0:
		version = "SD 1.0";
		break;
#endif
	case MMC_VERSION_4:
		switch (slot->ext_csd[EXT_CSD_REV]) {
		case 0:
			version = "MMC v4.0";
			break;
		case 1:
			version = "MMC v4.1";
			break;
		case 2:
			version = "MMC v4.2";
			break;
		case 3:
			version = "MMC v4.3";
			break;
		case 4:
			version = "MMC v4.4 (obsolete)";
			break;
		case 5:
			version = "MMC v4.41";
			break;
		case 6:
			version = "MMC v4.5/4.51";
			break;
		case 7:
			version = "MMC v5.0/v5.01";
			break;
		case 8:
			version = "MMC v5.1";
			break;
		default:
			version = "MMC > v5.1";
			break;
		}
		break;
	case MMC_VERSION_4_41:
		version = "MMC v4.41";
		break;
	case MMC_VERSION_4_5:
		version = "MMC v4.5/v4.51";
		break;
	case MMC_VERSION_5_0:
		version = "MMC v5.0/v5.01";
		break;
	case MMC_VERSION_5_1:
		version = "MMC v5.1";
		break;
	case MMC_VERSION_3:
		version = "MMC 3";
		break;
	case MMC_VERSION_2_2:
		version = "MMC 2.2";
		break;
	case MMC_VERSION_1_4:
		version = "MMC 1.4";
		break;
	case MMC_VERSION_1_2:
		version = "MMC 1.2";
		break;
	case MMC_VERSION_UNKNOWN:
		version = "MMC Unknown";
		break;
	default:
		version = "Unknown";
		break;
	}

	printf("CID:                   0x%08x 0x%08x 0x%08x 0x%08x\n",
	       mmc->cid[0], mmc->cid[1], mmc->cid[2], mmc->cid[3]);
	printf("Name:                  %s\n", mmc->cfg->name);
	printf("Type:                  %s\n", type);
	printf("Version:               %s\n", version);
	printf("Manufacturer ID:       0x%02x\n", (mmc->cid[0] >> 24) & 0xff);
	if (IS_SD(mmc)) {
		printf("OEM ID:                %c%c\n",
		       (mmc->cid[0] >> 16) & 0xff, (mmc->cid[0] >> 8) & 0xff);
	} else {
		printf("Device Type:           %s\n",
		       cbx_str[(mmc->cid[0] >> 16) & 3]);
		printf("OEM ID:                0x%02x\n",
		       (mmc->cid[0] >> 8) & 0xff);
	}
	printf("Vendor:                %s\n", bdesc->vendor);
	printf("Product:               %s\n", bdesc->product);
	printf("Revision:              %s\n", bdesc->revision);
	if (IS_SD(mmc)) {
		printf("Manufacturing Date:    %d/%d\n",
		       (mmc->cid[3] >> 8) & 0xf,
		       ((mmc->cid[3] >> 12) & 0xff) + 2000);
	} else {
		int start_year;
		if ((slot->ext_csd[EXT_CSD_REV] > 4) &&
		    ((mmc->cid[3] >> 8) & 0xf) <= 12)
			start_year = 2013;
		else
			start_year = 1997;
		printf("Manufacturing Date:    %d/%d\n",
		       (mmc->cid[3] >> 12) & 0xf,
		       ((mmc->cid[3] >> 8) & 0xf) + start_year);
	}
	printf("CID CRC7:              0x%02x (%svalid)\n", mmc->cid[3] & 0xff,
	       (mmc->cid[3] & 0xff) == mmc_crc7_32(mmc->cid, 4) ? "" : "in");
	printf("Capacity:              %llu bytes (%llu blocks)\n",
	       mmc->capacity, mmc->capacity / mmc->read_bl_len);
	printf("Read block length:     %u\n", mmc->read_bl_len);
	printf("Write block length:    %u\n", mmc->write_bl_len);
	printf("High capacity:         %s\n",
	       mmc->high_capacity ? "yes" : "no");
	printf("Bus width:             %u bits\n", mmc->bus_width);
	printf("Bus frequency:         %u\n", mmc->clock);
	if (!mmc->card_caps & MMC_MODE_HS)
		printf("Transfer frequency:    %u\n", mmc->tran_speed);
	printf("Bus DDR:               %s\n", mmc->ddr_mode ? "yes" : "no");
	if (!IS_SD(mmc)) {
		printf("Erase group size:      %u\n", mmc->erase_grp_size);
		if (mmc->version >= MMC_VERSION_4_41) {
			bool has_enh =
				(mmc->part_support & ENHNCD_SUPPORT) != 0;
			bool usr_enh =
				has_enh && (mmc->part_attr & EXT_CSD_ENH_USR);
			puts("HC WP Group Size:      ");
			print_size(((u64)mmc->hc_wp_grp_size) << 9, "\n");

			puts("User Capacity:         ");
			print_size(mmc->capacity_user, usr_enh ? " ENH" : "");
			if (mmc->wr_rel_set & EXT_CSD_WR_DATA_REL_USR)
				puts(" WRREL\n");
			else
				putc('\n');
			if (usr_enh) {
				puts("User Enhanced Start:   ");
				print_size(mmc->enh_user_start, "\n");
				puts("User Enhanced Size:    ");
				print_size(mmc->enh_user_size, "\n");
			}
			puts("Boot Capacity:         ");
			print_size(mmc->capacity_boot,
				   has_enh ? " ENH\n" : "\n");
			puts("RPBM Capacity:         ");
			print_size(mmc->capacity_rpmb,
				   has_enh ? " ENH\n" : "\n");
			for (i = 0; i < ARRAY_SIZE(mmc->capacity_gp); i++) {
				bool is_enh = has_enh &&
					(mmc->part_attr & EXT_CSD_ENH_GP(i));
				if (mmc->capacity_gp[i]) {
					printf("GP%i Capacity:          ",
					       i + 1);
					print_size(mmc->capacity_gp[i],
						   is_enh ? " ENH" : "");
					if (mmc->wr_rel_set &
					    EXT_CSD_WR_DATA_REL_GP(i))
						puts(" WRREL\n");
					else
						putc('\n');
				}
			}
		}
	}
	printf("Relative Card Address: 0x%x\n", mmc->rca);
	printf("Device is %sremovable\n", slot->non_removable ? "non-" : "");
	if (IS_SD(mmc)) {
		const char *sd_security;
		uint8_t sd_spec, sd_spec3, sd_spec4;
		const char *spec_ver;
		const char *bus_widths;

		sd_spec = (mmc->scr[0] >> 24) & 0xf;
		sd_spec3 = (mmc->scr[0] >> 15) & 1;
		sd_spec4 = (mmc->scr[0] >> 10) & 1;
		printf("SCR register:          0x%08x %08x\n",
		       mmc->scr[0], mmc->scr[1]);
		printf(" structure version:    %s\n",
		       (mmc->scr[0] & 0xf0000000) ? "Unknown" : "1.0");
		if ((sd_spec == 0) && (sd_spec3 == 0) && (sd_spec4 == 0))
			spec_ver = "1.0 and 1.01";
		else if ((sd_spec == 1) && (sd_spec3 == 0) && (sd_spec4 == 0))
			spec_ver = "1.10";
		else if ((sd_spec == 2) && (sd_spec3 == 0) && (sd_spec4 == 0))
			spec_ver = "2.00";
		else if ((sd_spec == 2) && (sd_spec3 == 1) && (sd_spec4 == 0))
			spec_ver = "3.0X";
		else if ((sd_spec == 2) && (sd_spec3 == 1) && (sd_spec4 == 1))
			spec_ver = "4.XX";
		else
			spec_ver = "Reserved";
		printf(" Specification ver:    %s\n", spec_ver);
		printf(" Data stat after erase: %d\n", (mmc->scr[0] >> 23) & 1);
		switch ((mmc->scr[0] >> 20) & 7) {
		case 0:
			sd_security = "None";
			break;
		case 1:
			sd_security = "Not Used";
			break;
		case 2:
			sd_security = "SDSC Card (Security Version 1.01)";
			break;
		case 3:
			sd_security = "SDHC Card (Security Version 2.00)";
			break;
		case 4:
			sd_security = "SDXC Card (Security Version 3.xx)";
			break;
		default:
			sd_security = "Reserved/Unknown";
			break;
		}
		printf(" SD Security:          %s\n", sd_security);
		switch ((mmc->scr[0] >> 16) & 0xf) {
		case 1:
			bus_widths = "1 bit (DAT0)";
			break;
		case 4:
			bus_widths = "4 bits (DAT0-3)";
			break;
		case 5:
			bus_widths = "1 bit (DAT0) and 4 bits (DAT0-3)";
			break;
		default:
			bus_widths = "Unknown";
		}
		printf(" SD Bus Widths:        %s\n", bus_widths);
		if ((mmc->scr[0] >> 11) & 7)
			printf("SD Extended Security supported\n");

		if (mmc->scr[0] & 8)
			printf(" Extension Register Mult-Block (CMD58/59) supported\n");
		if (mmc->scr[0] & 4)
			printf(" Extension Register Single Block (CMD48/49) supported\n");
		if (mmc->scr[0] & 2)
			printf(" SD Set Block Count (CMD23) supported\n");
		if (mmc->scr[0] & 1)
			printf(" SDXC Speed Class Control (CMD20) supported\n");
	} else {
		if (card_type != 0 && mmc->version >= MMC_VERSION_4) {
			puts("Supported bus speeds: ");
			if (card_type & EXT_CSD_CARD_TYPE_26) {
				puts(" 26MHz");
				prev = 1;
			}
			if (card_type & EXT_CSD_CARD_TYPE_52) {
				if (prev)
					putc(',');
				puts(" 52MHz");
				prev = 1;
			}
			if (card_type & EXT_CSD_CARD_TYPE_DDR_1_8V) {
				if (prev)
					putc(',');
				puts(" DDR 1.8V, 3V");
				prev = 1;
			}
			if (card_type & EXT_CSD_CARD_TYPE_DDR_1_2V) {
				if (prev)
					putc(',');
				puts(" DDR 1.2V");
				prev = 1;
			}
			if (card_type & EXT_CSD_CARD_TYPE_HS200_1_8V) {
				if (prev)
					putc(',');
				puts(" HS200 1.8V");
				prev = 1;
			}
			if (card_type & EXT_CSD_CARD_TYPE_HS200_1_2V) {
				if (prev)
					putc(',');
				puts(" HS200 1.2V");
				prev = 1;
			}
			if (card_type & EXT_CSD_CARD_TYPE_HS400_1_8V) {
				if (prev)
					putc(',');
				puts(" HS400 1.8V");
				prev = 1;
			}
			if (card_type & EXT_CSD_CARD_TYPE_HS400_1_2V) {
				if (prev)
					putc(',');
				puts(" HS400 1.2V");
				prev = 1;
			}
			puts("\n");
		}
		printf("Current power Class:   %smA\n",
		       pwr_classes[ext_csd[EXT_CSD_POWER_CLASS] & 0xF]);
		printf("Power 4-bit@52MHz:     %smA\n",
		       pwr_classes[ext_csd[EXT_CSD_PWR_CL_52_360] & 0xF]);
		printf("Power 8-bit@52MHz:     %smA\n",
		       pwr_classes[(ext_csd[EXT_CSD_PWR_CL_52_360] >> 4) & 0xF]);
		printf("Power 4-bit@26MHz:     %smA\n",
		       pwr_classes[ext_csd[EXT_CSD_PWR_CL_26_360] & 0xF]);
		printf("Power 8-bit@26MHz:     %smA\n",
		       pwr_classes[(ext_csd[EXT_CSD_PWR_CL_26_360] >> 4) & 0xF]);
		printf("Power 4-bit@52MHz DDR: %smA\n",
		       pwr_classes[ext_csd[EXT_CSD_PWR_CL_DDR_52_360] & 0xF]);
		printf("Power 8-bit@52MHz DDR: %smA\n",
		       pwr_classes[(ext_csd[EXT_CSD_PWR_CL_DDR_52_360] >> 4) & 0xF]);
	}
	printf("OCR register:          0x%x\n", mmc->ocr);
	printf("CSD register:          0x%08x 0x%08x 0x%08x 0x%08x\n",
	       mmc->csd[0], mmc->csd[1], mmc->csd[2], mmc->csd[3]);
	if (!IS_SD(mmc)) {
		static const char * const rst_n_function[] = {
			"RST_n signal is temporarily disabled",
			"RST_n signal is permanently enabled",
			"RST_n signal is permanently disabled",
			"Reserved"
		};
		const char *str;
		int life;

		printf("RST_n_FUNCTION:        %s\n",
		       rst_n_function[ext_csd[EXT_CSD_RST_N_FUNCTION] & 3]);
		switch (slot->ext_csd[267]) {
		case 0:
			str = "Not defined";
			break;
		case 1:
			str = "Normal";
			break;
		case 2:
			str = "Warning";
			break;
		case 3:
			str = "Urgent";
			break;
		default:
			str = "Reserved";
			break;
		}
		printf("Pre EOL:               %s\n", str);

		puts("Life time estimation A: ");
		if (ext_csd[268] > 0 && ext_csd[268] < 0xb) {
			life = ext_csd[268] * 10;
			printf("Device Life time used: %d - %d\n",
			       life - 10, life);
		} else if (ext_csd[268] == 0xb) {
			puts("Device has exceeded its maximum estimated life time\n");
		} else {
			puts("Device life time unknown\n");
		}

		puts("Life time estimation B: ");
		if (ext_csd[269] > 0 && ext_csd[269] < 0xb) {
			life = ext_csd[269] * 10;
			printf("Device Life time used: %d - %d\n",
			       life - 10, life);
		} else if (ext_csd[269] == 0xb) {
			puts("Device has exceeded its maximum estimated life time\n");
		} else {
			puts("Device life time unknown\n");
		}

		puts("Extended CSD register:");
		for (i = 0; i < 512; i++) {
			if (i % 16 == 0)
				printf("\n%3u: ", i);
			if (i % 16 == 8)
				puts("- ");
			printf("%02x ", (uint32_t)ext_csd[i]);
		}
		puts("\n");
	}
}

/**
 * Sets the transfer block size which is usually 512 bytes
 *
 * @param mmc	pointer to mmc data structure
 * @param len	block length to use
 *
 * @return 0 for success, error otherwise
 */
int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;
	int err;

	debug("%s(%s, %d)\n", __func__, mmc->cfg->name, len);
	mmc_switch_dev(mmc);

	if (mmc->card_caps & MMC_MODE_DDR_52MHz) {
		debug("MMC set block length skipped in DDR mode\n");
		return 0;
	}

	cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = len;

	debug("%s: Setting block length to %d\n", __func__, len);
	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err)
		printf("%s: Error setting block length to %d\n", __func__, len);

	return err;
}

static int mmc_set_capacity(struct mmc *mmc, int part_num)
{
	const struct cavium_mmc_slot *slot = mmc->priv;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);

	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return -1;
	}
	debug("%s(%s, %d)\n", __func__, mmc->cfg->name, part_num);
	switch (part_num) {
	case 0:
		mmc->capacity = mmc->capacity_user;
		break;
	case 1:
	case 2:
		mmc->capacity = mmc->capacity_boot;
		break;
	case 3:
		mmc->capacity = mmc->capacity_rpmb;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		mmc->capacity = mmc->capacity_gp[part_num - 4];
		break;
	default:
		return -1;
	}

	debug("%s: capacity now %llu\n", __func__, mmc->capacity);
	bdesc->lba = lldiv(mmc->capacity, mmc->read_bl_len);

	return 0;
}

int mmc_switch_part(struct mmc *mmc, unsigned int part_num)
{
	const struct cavium_mmc_slot *slot = mmc->priv;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);
	int ret;

	if (!mmc)
		return -1;
	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return -1;
	}

	debug("%s(%u): mmc: %s\n", __func__, part_num,
	      mmc->cfg->name);
	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			 (mmc->part_config & ~PART_ACCESS_MASK)
			 | (part_num & PART_ACCESS_MASK));

	/*
	 * Set the capacity if the switch succeeded or was intended
	 * to return to representing the raw device.
	 */
	if ((ret == 0) || ((ret == -ENODEV) && (part_num == 0))) {
		ret = mmc_set_capacity(mmc, part_num);
		bdesc->hwpart = part_num;
	}

	return ret;
}

int mmc_hwpart_config(struct mmc *mmc,
		      const struct mmc_hwpart_conf *conf,
		      enum mmc_hwpart_conf_mode mode)
{
	u8 part_attrs = 0;
	u32 enh_size_mult;
	u32 enh_start_addr;
	u32 gp_size_mult[4];
	u32 max_enh_size_mult;
	u32 tot_enh_size_mult = 0;
	u8 wr_rel_set;
	int i, pidx, err;
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);

	if (mode < MMC_HWPART_CONF_CHECK || mode > MMC_HWPART_CONF_COMPLETE)
		return -EINVAL;

	if (IS_SD(mmc) || (mmc->version < MMC_VERSION_4_41)) {
		printf("eMMC >= 4.4 required for enhanced user data area\n");
		return -EMEDIUMTYPE;
	}

	if (!(mmc->part_support & PART_SUPPORT)) {
		printf("Card does not support partitioning\n");
		return -EMEDIUMTYPE;
	}

	if (!mmc->hc_wp_grp_size) {
		printf("Card does not define HC WP group size\n");
		return -EMEDIUMTYPE;
	}

	/* check partition alignment and total enhanced size */
	if (conf->user.enh_size) {
		if (conf->user.enh_size % mmc->hc_wp_grp_size ||
		    conf->user.enh_start % mmc->hc_wp_grp_size) {
			puts("User data enhanced area not HC WP group size aligned\n");
			return -EINVAL;
		}
		part_attrs |= EXT_CSD_ENH_USR;
		enh_size_mult = conf->user.enh_size / mmc->hc_wp_grp_size;
		if (mmc->high_capacity)
			enh_start_addr = conf->user.enh_start;
		else
			enh_start_addr = (conf->user.enh_start << 9);
	} else {
		enh_size_mult = 0;
		enh_start_addr = 0;
	}
	tot_enh_size_mult += enh_size_mult;

	for (pidx = 0; pidx < 4; pidx++) {
		if (conf->gp_part[pidx].size % mmc->hc_wp_grp_size) {
			printf("GP%i partition not HC WP group size aligned\n",
			       pidx + 1);
			return -EINVAL;
		}
		gp_size_mult[pidx] =
			conf->gp_part[pidx].size / mmc->hc_wp_grp_size;

		if (conf->gp_part[pidx].size && conf->gp_part[pidx].enhanced) {
			part_attrs |= EXT_CSD_ENH_GP(pidx);
			tot_enh_size_mult += gp_size_mult[pidx];
		}
	}

	if (part_attrs && !(mmc->part_support & ENHNCD_SUPPORT)) {
		printf("Card does not support enhanced attribute\n");
		return -EMEDIUMTYPE;
	}

	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err)
		return err;

	max_enh_size_mult =
		(ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT+2] << 16) +
		(ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT+1] << 8) +
		ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT];
	if (tot_enh_size_mult > max_enh_size_mult) {
		printf("Total enhanced size exceeds maximum (%u > %u)\n",
		       tot_enh_size_mult, max_enh_size_mult);
		return -EMEDIUMTYPE;
	}

	/* The default value of EXT_CSD_WR_REL_SET is device
	 * dependent, the values can only be changed if the
	 * EXT_CSD_HS_CTRL_REL bit is set. The values can be
	 * changed only once and before partitioning is completed. */
	wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];
	if (conf->user.wr_rel_change) {
		if (conf->user.wr_rel_set)
			wr_rel_set |= EXT_CSD_WR_DATA_REL_USR;
		else
			wr_rel_set &= ~EXT_CSD_WR_DATA_REL_USR;
	}
	for (pidx = 0; pidx < 4; pidx++) {
		if (conf->gp_part[pidx].wr_rel_change) {
			if (conf->gp_part[pidx].wr_rel_set)
				wr_rel_set |= EXT_CSD_WR_DATA_REL_GP(pidx);
			else
				wr_rel_set &= ~EXT_CSD_WR_DATA_REL_GP(pidx);
		}
	}

	if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET] &&
	    !(ext_csd[EXT_CSD_WR_REL_PARAM] & EXT_CSD_HS_CTRL_REL)) {
		puts("Card does not support host controlled partition write reliability settings\n");
		return -EMEDIUMTYPE;
	}

	if (ext_csd[EXT_CSD_PARTITION_SETTING] &
	    EXT_CSD_PARTITION_SETTING_COMPLETED) {
		printf("Card already partitioned\n");
		return -EPERM;
	}

	if (mode == MMC_HWPART_CONF_CHECK)
		return 0;

	/* Partitioning requires high-capacity size definitions */
	if (!(ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01)) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ERASE_GROUP_DEF, 1);

		if (err)
			return err;

		ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;

		/* update erase group size to be high-capacity */
		mmc->erase_grp_size =
			ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;
	}

	/* all OK, write the configuration */
	for (i = 0; i < 4; i++) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ENH_START_ADDR+i,
				 (enh_start_addr >> (i*8)) & 0xFF);
		if (err)
			return err;
	}
	for (i = 0; i < 3; i++) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ENH_SIZE_MULT+i,
				 (enh_size_mult >> (i*8)) & 0xFF);
		if (err)
			return err;
	}
	for (pidx = 0; pidx < 4; pidx++) {
		for (i = 0; i < 3; i++) {
			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
					 EXT_CSD_GP_SIZE_MULT + pidx * 3 + i,
					 (gp_size_mult[pidx] >>
							(i * 8)) & 0xFF);
			if (err)
				return err;
		}
	}
	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PARTITIONS_ATTRIBUTE, part_attrs);
	if (err)
		return err;

	if (mode == MMC_HWPART_CONF_SET)
		return 0;

	/* The WR_REL_SET is a write-once register but shall be
	 * written before setting PART_SETTING_COMPLETED. As it is
	 * write-once we can only write it when completing the
	 * partitioning. */
	if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET]) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_WR_REL_SET, wr_rel_set);
		if (err)
			return err;
	}

	/* Setting PART_SETTING_COMPLETED confirms the partition
	 * configuration but it only becomes effective after power
	 * cycle, so we do not adjust the partition related settings
	 * in the mmc struct. */

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PARTITION_SETTING,
			 EXT_CSD_PARTITION_SETTING_COMPLETED);
	if (err)
		return err;

	return 0;
}

__weak int board_mmc_getwp(struct mmc *mmc)
{
	return -1;
}

__weak int board_mmc_getcd(struct mmc *mmc)
{
	return -1;
}

/**
 * Wait for a command to respond
 *
 * @param	mmc	MMC device
 * @param	bus_id	bus ID to wait on
 * @param	cmd_idx	command index to wait on
 * @param	flags	command flags
 * @param	timeout	timeout in ms, if 0 wait forever
 *
 * @return	0 if command returned within the timeout, ETIMEDOUT if command
 *		timed out or -1 if error.
 */
static int
oct_mmc_wait_cmd(struct mmc *mmc, int bus_id, int cmd_idx, int flags,
		 uint timeout)
{
	union mio_emm_rsp_sts emm_rsp_sts;
	unsigned long base_time;
	ulong time = 0;

	debug("%s(%s bus: %d, cmd: 0x%x, flags: 0x%x, timeout: %u\n", __func__,
	      mmc->cfg->name, bus_id, cmd_idx, flags, timeout);
	base_time = get_timer(0);

	do {
		emm_rsp_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);
		if (emm_rsp_sts.s.cmd_done || emm_rsp_sts.s.rsp_timeout)
			break;
		WATCHDOG_RESET();
		time += 1;
	} while (!timeout || (get_timer(base_time) < timeout + 10));

	debug("%s: bus_id: %d, cmd_idx: %d response is 0x%llx after %lu ms (%lu loops)\n",
	      __func__, bus_id, cmd_idx, emm_rsp_sts.u, get_timer(base_time),
	      time);
	if (!emm_rsp_sts.s.cmd_done) {
		debug("%s: Wait for command index %d timed out after %ums\n",
		      __func__, cmd_idx, timeout);
#ifdef DEBUG
		mmc_print_registers(mmc);
#endif
		return ETIMEDOUT;
	}
	if (bus_id >= 0 && emm_rsp_sts.s.bus_id != bus_id) {
		debug("%s: Mismatch bus_id, expected %d for cmd idx %d, got %d\n",
		      __func__, bus_id, cmd_idx, emm_rsp_sts.s.bus_id);
#ifdef DEBUG
		mmc_print_registers(NULL);
#endif
		return -1;	/* Wrong bus ID */
	}
	if (emm_rsp_sts.s.rsp_timeout ||
	    (emm_rsp_sts.s.rsp_crc_err &&
		!(flags & MMC_CMD_FLAG_IGNORE_CRC_ERR)) ||
	    emm_rsp_sts.s.rsp_bad_sts) {
		uint64_t status = mmc_read_csr(mmc, MIO_EMM_RSP_LO) >> 8;
		debug("%s: Bad response for bus id %d, cmd id %d:\n"
		      "    rsp_timeout: %d\n"
		      "    rsp_bad_sts: %d\n"
		      "    rsp_crc_err: %d\n",
		     __func__, bus_id, cmd_idx,
		     emm_rsp_sts.s.rsp_timeout,
		     emm_rsp_sts.s.rsp_bad_sts,
		     emm_rsp_sts.s.rsp_crc_err);
#ifdef DEBUG
		mmc_print_registers(mmc);
#endif
		if (emm_rsp_sts.s.rsp_type == 1 && emm_rsp_sts.s.rsp_bad_sts) {
			debug("    Response status: 0x%llx\n",
			      status & 0xffffffff);
#ifdef DEBUG
			mmc_print_status(status);
			mmc_print_registers(NULL);
#endif
		}
		return -1;
	}
	if (emm_rsp_sts.s.cmd_idx != cmd_idx) {
		debug("%s: response for bus id %d, cmd idx %d mismatch command index %d\n",
		      __func__, bus_id, cmd_idx, emm_rsp_sts.s.cmd_idx);
#ifdef DEBUG
		mmc_print_registers(mmc);
#endif
		return -1;
	}
	return 0;
}

/**
 * Send a command with the specified timeout in milliseconds.
 *
 * @param mmc		pointer to MMC data structure
 * @param cmd		pointer to command data structure
 * @param data		pointer to data descriptor, NULL for none
 * @param flags		flags passed for the command
 * @param timeout	time to wait for command to complete in milliseconds
 *
 * @return		0 for success, error otherwise
 */
static int
mmc_send_cmd_timeout(struct mmc *mmc, struct mmc_cmd *cmd,
		     struct mmc_data *data,
		     uint32_t flags, uint timeout)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	union mio_emm_cmd emm_cmd;
	union mio_emm_buf_idx emm_buf_idx;
	union mio_emm_buf_dat emm_buf_dat;
	uint64_t resp_lo;
	uint64_t resp_hi;
	int i;
	int bus_id = slot->bus_id;

	debug("%s(%s bus: %d, cmd: 0x%x, data: %p, flags: 0x%x, timeout: %u)\n",
	      __func__, mmc->cfg->name, bus_id,
	      cmd->cmdidx, data, flags, timeout);
	mmc_switch_dev(mmc);

	/* Set the hardware timeout */
	mmc_set_watchdog(mmc, timeout ? timeout * 1000 : (1 << 26) - 1);

	/* Clear any interrupts */
	mmc_write_csr(mmc, MIO_EMM_INT, mmc_read_csr(mmc, MIO_EMM_INT));
	emm_cmd.u = 0;
	emm_cmd.s.cmd_val = 1;
	emm_cmd.s.bus_id = bus_id;
	emm_cmd.s.cmd_idx = cmd->cmdidx;
	emm_cmd.s.arg = cmd->cmdarg;
	emm_cmd.s.ctype_xor = (flags >> 16) & 3;
	emm_cmd.s.rtype_xor = (flags >> 20) & 7;
	emm_cmd.s.offset = (flags >> 24) & 0x3f;

	debug("mmc cmd: %d, arg: 0x%x flags: 0x%x reg: 0x%llx, bus id: %d\n",
	      cmd->cmdidx, cmd->cmdarg, flags, emm_cmd.u, bus_id);

	if (data && data->flags & MMC_DATA_WRITE) {
		const char *src = data->src;
		if (!src) {
			printf("%s: Error, source buffer is NULL\n", __func__);
			return -1;
		}
		if (data->blocksize > 512) {
			printf("%s: Error: data size %u exceeds 512\n",
			       __func__, data->blocksize);
		}
		emm_buf_idx.u = 0;
		emm_buf_idx.s.inc = 1;
		mmc_write_csr(mmc, MIO_EMM_BUF_IDX, emm_buf_idx.u);
		for (i = 0; i < (data->blocksize + 7) / 8; i++) {
			memcpy(&emm_buf_dat.u, src, sizeof(emm_buf_dat));
			emm_buf_dat.u = cpu_to_be64(emm_buf_dat.u);
			mmc_write_csr(mmc, MIO_EMM_BUF_DAT, emm_buf_dat.u);
			debug("mmc cmd: buffer 0x%x: 0x%llx\n",
			      i*8, emm_buf_dat.u);
			src += sizeof(emm_buf_dat);
		}
		debug("mmc cmd: wrote %d 8-byte blocks to buffer\n", i);
	}
	mmc_write_csr(mmc, MIO_EMM_CMD, emm_cmd.u);

	if (oct_mmc_wait_cmd(mmc, bus_id, cmd->cmdidx, flags, timeout)) {
		if (!init_time) {
			debug("mmc cmd: Error waiting for bus %d, command index %d to complete\n",
			      bus_id, cmd->cmdidx);
		}
		return ETIMEDOUT;
	}
	debug("%s: Response flags: 0x%x\n", __func__, cmd->resp_type);
	if (!cmd->resp_type & MMC_RSP_PRESENT) {
		debug("%s: no response expected for command index %d, returning\n",
		      __func__, cmd->cmdidx);
		return 0;
	}

	resp_lo = mmc_read_csr(mmc, MIO_EMM_RSP_LO);
	if (cmd->resp_type & MMC_RSP_136) {
		resp_hi = mmc_read_csr(mmc, MIO_EMM_RSP_HI);
		debug("mmc cmd: response hi: 0x%016llx\n", resp_hi);
		cmd->response[0] = resp_hi >> 32;
		cmd->response[1] = resp_hi & 0xffffffff;
		cmd->response[2] = resp_lo >> 32;
		cmd->response[3] = resp_lo & 0xffffffff;
	} else if ((cmd->resp_type & MMC_RSP_CRC) ||
		   (flags & MMC_CMD_FLAG_STRIP_CRC)) {	/* No CRC */
		cmd->response[0] = (resp_lo >> 8) & 0xffffffff;
	} else {
		cmd->response[0] = resp_lo & 0xffffffff;
	}
	debug("mmc cmd: response lo: 0x%016llx\n", resp_lo);
	if (data && data->flags & MMC_DATA_READ) {
		char *dest = data->dest;

		if (!dest) {
			printf("%s: Error, destination buffer NULL!\n",
			       __func__);
			return -1;
		}
		if (data->blocksize > 512) {
			printf("%s: Error: data size %u exceeds 512\n",
			       __func__, data->blocksize);
		}
		emm_buf_idx.u = 0;
		emm_buf_idx.s.inc = 1;
		mmc_write_csr(mmc, MIO_EMM_BUF_IDX, emm_buf_idx.u);
		for (i = 0; i < (data->blocksize + 7) / 8; i++) {
			emm_buf_dat.u = mmc_read_csr(mmc, MIO_EMM_BUF_DAT);
			emm_buf_dat.u = be64_to_cpu(emm_buf_dat.u);
			memcpy(dest, &emm_buf_dat.u, sizeof(emm_buf_dat));
			       dest += sizeof(emm_buf_dat);
		}
	}
	return 0;
}

/**
 * Send a command with the specified flags.
 *
 * @param mmc		pointer to MMC data structure
 * @param cmd		pointer to command data structure
 * @param data		pointer to data descriptor, NULL for none
 * @param flags		flags passed for the command
 *
 * @return		0 for success, error otherwise
 */
static int mmc_send_cmd_flags(struct mmc *mmc, struct mmc_cmd *cmd,
			      struct mmc_data *data, uint32_t flags)
{
	uint timeout;
	/**
	 * This constant has a 1 bit for each command which should have a short
	 * timeout and a 0 for each bit with a long timeout.  Currently the
	 * following commands have a long timeout:
	 *   CMD6, CMD17, CMD18, CMD24, CMD25, CMD32, CMD33, CMD35, CMD36 and
	 *   CMD38.
	 */
	static const uint64_t timeout_short = 0xFFFFFFA4FCF9FFDFULL;

	debug("%s(%s, cmd: %u, arg: 0x%x flags: 0x%x)\n", __func__,
	      mmc->cfg->name, cmd->cmdidx, cmd->cmdarg, flags);
	if (cmd->cmdidx >= 64) {
		printf("%s: Error: command index %d is out of range\n",
		       __func__, cmd->cmdidx);
		return -1;
	}
	if (timeout_short & (1ULL << cmd->cmdidx))
		timeout = MMC_TIMEOUT_SHORT;
	else if (cmd->cmdidx == 6 && IS_SD(mmc))
		timeout = 2560;
	else if (cmd->cmdidx == 38)
		timeout = MMC_TIMEOUT_ERASE;
	else
		timeout = MMC_TIMEOUT_LONG;
	debug("%s: CMD%d: timeout: %u\n", __func__, cmd->cmdidx, timeout);
	return mmc_send_cmd_timeout(mmc, cmd, data, flags, timeout);
}

/**
 * Sends a SD acmd with flags
 *
 * @param mmc		pointer to MMC data structure
 * @param cmd		pointer to command data structure
 * @param data		pointer to data descriptor, NULL for none
 * @param flags		flags passed for the command
 *
 * @return		0 for success, error otherwise
 */
static int mmc_send_acmd(struct mmc *mmc, struct mmc_cmd *cmd,
			 struct mmc_data *data, uint32_t flags)
{
	struct mmc_cmd acmd;
	int err;

	acmd.cmdidx = MMC_CMD_APP_CMD;
	acmd.cmdarg = mmc->rca << 16;
	acmd.resp_type = MMC_RSP_R1;

	if (!IS_SD(mmc)) {
		debug("%s: Error, not SD card\n", __func__);
		return -1;
	}
	err = cavium_mmc_send_cmd(mmc->dev, &acmd, NULL);
	if (err) {
		printf("%s: Error sending ACMD to SD card\n", __func__);
		return err;
	}
	if (cmd)
		return mmc_send_cmd_flags(mmc, cmd, data, flags);
	else
		return 0;
}

/** Change the bus width */
static void mmc_set_bus_width(struct mmc *mmc, uint width)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	mmc->bus_width = min(width, (uint)slot->bus_max_width);
	debug("%s(%s, %u): seting bus_width to %u\n", __func__,
	      mmc->cfg->name, width, mmc->bus_width);
}

static int mmc_pre_idle(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0xf0f0f0f0;	/* Software Reset */
	cmd.resp_type = MMC_RSP_NONE;

	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err)
		debug("%s: error %d\n", __func__, err);
	else
		mdelay(20);
	return err;
}

static int mmc_go_idle(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;
	int i;

	for (i = 0; i < 5; i++) {
		debug("%s: Going idle try %d\n", __func__, i);
		/* Do this 5 times to clear the bus */
		cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_NONE;

		err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
		if (err)
			return err;
	}
	mdelay(20);	/* Wait 20ms */
	return 0;
}

#ifdef CONFIG_CAVIUM_MMC_SD
static int sd_send_relative_addr(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R6;
	err = mmc_send_cmd_timeout(mmc, &cmd, NULL,
				   MMC_CMD_FLAG_RTYPE_XOR(2), 2);
	if (err) {
		printf("%s: error sending command\n", __func__);
		return err;
	}
	mmc->rca = cmd.response[0] >> 16;
	mmc_write_csr(mmc, MIO_EMM_RCA, mmc->rca);
	debug("%s: SD RCA is %d (0x%x)\n", __func__, mmc->rca, mmc->rca);
	debug("%s: MIO_EMM_RCA: 0x%llx\n", __func__,
	      mmc_read_csr(mmc, MIO_EMM_RCA));
	return 0;
}
#endif

/**
 * Send the set relatice address command
 *
 * @param mmc	pointer to mmc data structure
 *
 * @return 0 for success, error otherwise
 */
static int mmc_set_relative_addr(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmdidx = MMC_CMD_SET_RELATIVE_ADDR;
	cmd.cmdarg = mmc->rca << 16;
	cmd.resp_type = MMC_RSP_R1;
	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err)
		printf("%s: Error %d, failed to set RCA to %d\n", __func__,
		       err, mmc->rca);

	return err;
}

static int mmc_select_card(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	debug("%s(%s): entry\n", __func__, mmc->cfg->name);
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmdidx = MMC_CMD_SELECT_CARD;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = mmc->rca << 16;

	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err)
		printf("%s: Error selecting card with rca %d\n",
		       __func__, mmc->rca);
	else
		mmc_write_csr(mmc, MIO_EMM_RCA, mmc->rca);
	return err;
}

/**
 * Get the CID data structure from the MMC/SD device
 *
 * @param mmc	pointer to mmc data structure
 *
 * @return 0 for success, error otherwise
 */
static int mmc_all_send_cid(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;
#ifdef DEBUG
	uint8_t crc7;
#endif

	memset(&cmd, 0, sizeof(cmd));

	debug("%s(%s): Getting CID\n", __func__, mmc->cfg->name);
	cmd.cmdidx = MMC_CMD_ALL_SEND_CID;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = 0;
	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err) {
		debug("%s: Error getting all CID\n", __func__);
		return err;
	}

	memcpy(mmc->cid, &cmd.response[0], 16);
#ifdef DEBUG
	print_buffer(0, mmc->cid, 1, 16, 0);
	debug("        Manufacturer: 0x%x\n", mmc->cid[0] >> 24);
	if (!IS_SD(mmc)) {
		debug("        Device/BGA:   ");
		switch ((mmc->cid[0] >> 16) & 3) {
		case 0:
			debug("Device (removable)\n");
			break;
		case 1:
			debug("BGA (Discrete embedded)\n");
			break;
		case 2:
			debug("POP\n");
			break;
		default:
			debug("Reserved\n");
			break;
		}
	}
	if (IS_SD(mmc)) {
		debug("        OID:          %02x%02x\n",
		      (mmc->cid[0] >> 16) & 0xff,
		      (mmc->cid[0] >> 8) & 0xff);
		debug("        Product Name: %c%c%c%c%c\n", mmc->cid[0] & 0xff,
		      (mmc->cid[1] >> 24) & 0xff, (mmc->cid[1] >> 16) & 0xff,
		      (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);
		debug("        Product Revision: %d.%d\n",
		      (mmc->cid[2] >> 20) & 0xf, (mmc->cid[2] >> 16) & 0xf);
		debug("        Product Serial Number: 0x%x\n",
		      ((mmc->cid[2] & 0xffffff) << 8) |
		      ((mmc->cid[3] >> 24) & 0xff));
		debug("        Manufacturing Date: %d/%d\n",
		      (mmc->cid[3] >> 8) & 0xf,
		      ((mmc->cid[3] >> 12) & 0xff) + 2000);
	} else {
		debug("        OID:          0x%x\n",
		      (mmc->cid[0] >> 8) & 0xFF);
		debug("        Product Name: %c%c%c%c%c%c\n",
		      mmc->cid[0] & 0xff,
		      (mmc->cid[1] >> 24) & 0xff, (mmc->cid[1] >> 16) & 0xff,
		      (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
		      (mmc->cid[2] >> 24) & 0xff);
		debug("        Product Revision: %d.%d\n",
		      (mmc->cid[2] >> 20) & 0xf, (mmc->cid[2] >> 16) & 0xf);
		debug("        Product Serial Number: 0x%x\n",
		      ((mmc->cid[2] & 0xffff) << 16) |
		      ((mmc->cid[3] >> 16) & 0xffff));
		debug("        Manufacturing Date: %d/%d\n",
		      (mmc->cid[3] >> 12) & 0xf,
		      ((mmc->cid[3] >> 8) & 0xf) + 2013);
	}
	crc7 = mmc_crc7_32(mmc->cid, 4);

	debug("        CRC7:               %02x (%svalid)\n", crc7,
	      crc7 == (mmc->cid[3] & 0xff) ? "" : "in");

#endif
	return 0;
}

/**
 * Get the CSD data structure
 *
 * @param mmc	pointer to MMC data structure
 *
 * @return 0 for success, error otherwise
 */
static int mmc_get_csd(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = mmc->rca << 16;
	err = cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
	if (err) {
		printf("%s: Error getting CSD\n", __func__);
		return err;
	}
	mmc->csd[0] = cmd.response[0];
	mmc->csd[1] = cmd.response[1];
	mmc->csd[2] = cmd.response[2];
	mmc->csd[3] = cmd.response[3];
	debug("%s: CSD: 0x%08x 0x%08x 0x%08x 0x%08x\n", __func__,
	      mmc->csd[0], mmc->csd[1], mmc->csd[2], mmc->csd[3]);
	return 0;
}

#ifdef CONFIG_CAVIUM_MMC_SD
static int sd_set_bus_width_speed(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;
#ifdef DEBUG
	struct cavium_mmc_slot *slot = mmc->priv;
	debug("%s(%s) width: %d %d\n", __func__, mmc->cfg->name,
	      mmc->bus_width, slot->bus_width);
#endif

	memset(&cmd, 0, sizeof(cmd));
	cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
	cmd.resp_type = MMC_RSP_R1;

	debug("card caps: 0x%x\n", mmc->card_caps);
	if (mmc->card_caps & MMC_MODE_4BIT) {
		/* Set SD bus width to 4 */
		cmd.cmdarg = 2;

		err = mmc_send_acmd(mmc, &cmd, NULL, 0);
		if (err) {
			printf("%s: Error setting bus width\n", __func__);
			return err;
		}
		mmc_set_bus_width(mmc, 4);
	} else {
		debug("%s(%s): 4-bit mode not supported\n",
		      __func__, mmc->cfg->name);
		/* Set SD bus width to 1 */
		cmd.cmdarg = 0;

		err = mmc_send_acmd(mmc, &cmd, NULL, 0);
		if (err) {
			printf("%s: Error setting bus width\n", __func__);
			return err;
		}
		mmc_set_bus_width(mmc, 1);
	}
	if (mmc->card_caps & MMC_MODE_HS) {
		if (mmc->card_caps & MMC_MODE_DDR_52MHz)
			mmc_set_clock(mmc, 50000000);
		else
			mmc_set_clock(mmc, 25000000);
	} else {
		mmc_set_clock(mmc, 20000000);
	}
	return sd_set_ios(mmc);
}
#endif

static int mmc_set_bus_width_speed(struct mmc *mmc)
{
	debug("%s: card caps: 0x%x\n", __func__, mmc->card_caps);
	if (mmc->card_caps & MMC_MODE_8BIT) {
		debug("%s: Set bus width to 8 bits\n", __func__);
		mmc_set_bus_width(mmc, 8);
	} else if (mmc->card_caps & MMC_MODE_4BIT) {
		debug("%s: Set bus width to 4 bits\n", __func__);
		/* Set the card to use 4 bit */
		mmc_set_bus_width(mmc, 4);
	}
	if (mmc->card_caps & MMC_MODE_HS) {
		debug("%s: Set high-speed mode\n", __func__);
		if (mmc->card_caps & MMC_MODE_HS_52MHz) {
			debug("%s: Set clock speed to 52MHz\n",
			      __func__);
			mmc_set_clock(mmc, 52000000);
		} else {
			mmc_set_clock(mmc, 26000000);
			debug("%s: Set clock speed to 26MHz\n",
			      __func__);
		}
	} else {
		debug("%s: Set clock speed to 20MHz\n", __func__);
		mmc_set_clock(mmc, 20000000);
	}
	mmc_set_ios(mmc);
	return 0;
}

#ifdef CONFIG_CAVIUM_MMC_SD
int sd_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;
	uint32_t flags = MMC_CMD_FLAG_RTYPE_XOR(3) | MMC_CMD_FLAG_STRIP_CRC;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	mmc->rca = 0;

	do {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err) {
			debug("%s(%s): acmd returned %d\n", __func__,
			      mmc->cfg->name, err);
			return err;
		}

		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;

		/*
		 * Most cards do not answer if some reserved bits
		 * in the ocr are set.  However, some controllers
		 * can set bit 7 (reserved low voltages), but
		 * how to manage low voltage SD cards is not yet
		 * specified.
		 */
		cmd.cmdarg = mmc->cfg->voltages & OCR_VOLTAGE_MASK;

		if ((mmc->version == SD_VERSION_1_0) ||
		    (mmc->version == SD_VERSION_1_10)) {
			cmd.cmdarg = 0x00ff8000;
			debug("%s: SD 1.X compliant card, voltages: 0x%x\n",
			      __func__, cmd.cmdarg);
		} else if (mmc->version == SD_VERSION_2) {
			cmd.cmdarg |= OCR_HCS;
			debug("%s: SD 2.0 compliant card, voltagess: 0x%x\n",
			      __func__, cmd.cmdarg);
		} else if ((mmc->version == SD_VERSION_3) ||
			   (mmc->version == SD_VERSION_4)) {
			cmd.cmdarg |= OCR_HCS | OCR_XPC;
			debug("%s: SD 3.0/4.0 compliant card, arg: 0x%x\n",
			      __func__, cmd.cmdarg);
		}

		err = mmc_send_cmd_flags(mmc, &cmd, NULL, flags);
		if (err) {
			debug("%s: Error %d sending SD command, might be MMC\n",
			      __func__, err);
			return err;
		}

		debug("%s response: 0x%x\n", __func__, cmd.response[0]);
		mdelay(1);
	} while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

	if (timeout <= 0) {
		printf("%s: Timed out\n", __func__);
		return ENOTRECOVERABLE;
	}

	if ((mmc->version != SD_VERSION_2) || (mmc->version != SD_VERSION_3) ||
	    (mmc->version != SD_VERSION_4))
		mmc->version = SD_VERSION_1_0;

	mmc->ocr = cmd.response[0];
	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);

	debug("%s: MMC high capacity mode %sdetected.\n",
	      __func__, mmc->high_capacity ? "" : "NOT ");
	return 0;
}
#endif

int mmc_send_op_cond(struct mmc *mmc)
{
	int timeout = WATCHDOG_COUNT;
	struct mmc_cmd cmd;
	int err;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	/* Some cards seem to need this */
	mmc_go_idle(mmc);

	do {
		cmd.cmdidx = MMC_CMD_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = OCR_HCS | mmc->cfg->voltages;

		err = mmc_send_cmd_flags(mmc, &cmd, NULL,
					 MMC_CMD_FLAG_STRIP_CRC);
		if (err) {
			if (!init_time)
				debug("%s: Returned %d\n", __func__, err);
			return err;
		}
		debug("%s: response 0x%x\n", __func__, cmd.response[0]);
		if (cmd.response[0] & OCR_BUSY)
			break;
		mdelay(1);
	} while (timeout--);

	if (timeout <= 0) {
		printf("%s: Timed out!", __func__);
		return ETIMEDOUT;
	}

	mmc->version = MMC_VERSION_UNKNOWN;
	mmc->ocr = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & 0x60000000) == OCR_HCS);

#ifdef DEBUG
	debug("%s: OCR: 0x%x\n", __func__, mmc->ocr);
	if (mmc->ocr & 0x80)
		debug("        1.70-1.95V\n");
	if (mmc->ocr & 0x3f00)
		debug("        2.0-2.6V\n");
	if (mmc->ocr & 0x007f8000)
		debug("        2.7-3.6V\n");
	debug("        Access Mode: %s\n",
	      (mmc->ocr & 0x40000000) == 0x40000000 ? "sector" : "byte");
	debug("        High Capacity: %s\n", mmc->ocr & OCR_HCS ? "yes" : "no");
#endif
	return 0;
}

/**
 * Get the extended CSD register
 *
 * @param mmc	pointer to mmc data structure
 *
 * @return 0 for success, error otherwise
 */
static int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
#ifdef DEBUG
	struct cavium_mmc_slot *slot = mmc->priv;
#endif
	int err;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	mmc_switch_dev(mmc);
	mmc_set_blocklen(mmc, 512);

	cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;
	data.dest = (char *)ext_csd;
	data.blocks = 1;
	data.blocksize = MMC_MAX_BLOCK_LEN;
	data.flags = MMC_DATA_READ;

	err = cavium_mmc_send_cmd(mmc->dev, &cmd, &data);

	if (err) {
		printf("%s: Error getting extended CSD\n", __func__);
	} else {
#ifdef DEBUG
		print_buffer(0, slot->ext_csd, 1, 512, 0);
#endif
	}
	return err;
}

int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;

	debug("%s(%s, 0x%x, 0x%x, 0x%x)\n", __func__, mmc->cfg->name,
	      set, index, value);
	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				(index << 16) | (value << 8) | set;

	return cavium_mmc_send_cmd(mmc->dev, &cmd, NULL);
}

#ifdef CONFIG_CAVIUM_MMC_SD
static int sd_set_ios(struct mmc *mmc)
{
	union mio_emm_switch emm_switch;
	struct cavium_mmc_slot *slot = mmc->priv;
	struct cavium_mmc_host *host = slot->host;
	int clock = mmc->clock;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	debug("%s: clock: %d (max %d), width %d\n",
	      __func__, clock, mmc->cfg->f_max, mmc->bus_width);
	if (mmc->bus_width > 4)
		mmc->bus_width = 4;
	slot->clk_period = (host->sclock + clock - 1) / clock;
	slot->power_class = 10;
	emm_switch.u = 0;
	emm_switch.s.hs_timing = mmc->clock > 20000000;
	debug("%s: hs timing: %d, caps: 0x%x\n", __func__,
	      emm_switch.s.hs_timing, mmc->card_caps);
	/* No DDR for now */
	slot->bus_width = (mmc->bus_width == 4) ? 1 : 0;
	emm_switch.s.bus_width = slot->bus_width;
	emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
	emm_switch.s.clk_lo = (slot->clk_period + 1) / 2;
	debug("%s: clock period: %u\n", __func__, slot->clk_period);
	emm_switch.s.power_class = slot->power_class;
	debug("%s: Writing emm_switch value 0x%llx\n",
	      __func__, emm_switch.u);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	emm_switch.s.bus_id = slot->bus_id;
	udelay(100);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	mdelay(20);
#ifdef DEBUG
	mmc_print_registers(mmc);
#endif
	return 0;
}
#endif

static int cavium_mmc_set_ios(struct udevice *dev)
{
	union mio_emm_switch emm_switch;
	union mio_emm_rsp_sts emm_sts;
	union mio_emm_sample emm_sample;
	int switch_timeout_ms = 2550;
	struct cavium_mmc_host *host = dev_get_priv(dev);
	struct cavium_mmc_slot *slot = &host->slots[host->cur_slotid];
	struct mmc *mmc = slot->mmc;
	int timeout = 2000;
	char cardtype;
	int hs_timing = 0;
	int ddr = 0;
	int bus_width;
	int power_class;
	int clock = mmc->clock;
	uint32_t flags = 0;
	int index;
#ifdef DEBUG
	union mio_emm_rsp_lo emm_rsp_lo;
#endif

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	debug("Starting clock is %uHz\n", clock);
	mmc->card_caps = 0;

	/* Only version 4 supports high speed */
	if (mmc->version < MMC_VERSION_4)
		return -1;

	if (clock == 0) {
		puts("mmc switch: Error, clock is zero!\n");
		return -1;
	}

	mmc_switch_dev(mmc);

	cardtype = slot->ext_csd[EXT_CSD_CARD_TYPE] & 0x3f;
	if (cardtype == 7) {
		ddr = 1;
		mmc->card_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz |
				  MMC_MODE_DDR_52MHz;
		debug("%s: Dual voltage card type supports 52MHz DDR at 1.8 and 3V\n",
		      __func__);
	}
	debug("%s: card type flags (device_type): 0x%x\n", __func__, cardtype);
	hs_timing = false;
	if (cardtype & (1 << 2)) {
		hs_timing = true;
		ddr = true;
		mmc->card_caps |= MMC_MODE_DDR_52MHz;
		debug("        High-Speed eMMC %dMHz %sat 1.2v or 3v\n",
		      52, "DDR ");
	}
	if (cardtype & (1 << 3)) {
		/* Octeon can't take advantage of this mode */
		debug("        High-Speed eMMC %dMHz %sat 1.2v I/O\n",
		      52, "DDR ");
	}
	if (cardtype & EXT_CSD_CARD_TYPE_26) {
		hs_timing = true;
		debug("        High-Speed eMMC %dMHz %sat rated device voltage(s)\n",
		      26, "");
	}
	if (cardtype & EXT_CSD_CARD_TYPE_52) {
		debug("        High-Speed eMMC %dMHz %sat rated device voltage(s)\n",
		      52, "");
		mmc->card_caps |= MMC_MODE_HS_52MHz;
		hs_timing = true;
	}
	if (cardtype & EXT_CSD_CARD_TYPE_DDR_1_8V) {
		debug("        High-Speed DDR eMMC 52MHz at 1.8V or 3V I/O\n");
		hs_timing = true;
		if ((mmc->cfg->voltages & MMC_VDD_165_195) ||
		    env_get("cavium_mmc_ddr"))
			ddr = true;
	}
	if (cardtype & EXT_CSD_CARD_TYPE_DDR_1_2V) {
		debug("        High-Speed DDR eMMC 52MHz at 1.2V I/O\n");
		/* hs_timing = true; */ /* We don't support 1.2V */
		/* DDR only works at 1.2V which OCTEON doesn't support */
	}
	if (cardtype & (1 << 4))
		debug("        HS200 Single Data Rate eMMC 200MHz at 1.8V I/O\n");
	if (cardtype & (1 << 5))
		debug("        HS200 Single Data Rate eMMC 200MHz at 1.2V I/O\n");
	if (cardtype & (1 << 6))
		debug("        HS400 DDR eMMC 200MHz at 1.8V I/O\n");
	if (cardtype & (1 << 7))
		debug("        HS400 DDR eMMC 200MHz at 1.2V I/O\n");
	if (!(cardtype & 0x7))
		hs_timing = false;

	mmc->bus_width = slot->bus_max_width;
	debug("        Max bus width: %d\n", slot->bus_max_width);
	/* Limit bus width to 4 for SD */
	if (IS_SD(mmc) && mmc->bus_width > 4)
		mmc->bus_width = 4;

	/* NOTE: We only use DDR mode for SD cards.  For MMC DDR will be enabled
	 * later after the bus width is detected since DDR mode is not allowed
	 * when detecting the bus width.
	 */
	switch (mmc->bus_width) {
	case 8:
		if (ddr)
			bus_width = EXT_CSD_DDR_BUS_WIDTH_8;
		else
			bus_width = EXT_CSD_BUS_WIDTH_8;
		break;
	case 4:
		if (ddr)
			bus_width = EXT_CSD_DDR_BUS_WIDTH_4;
		else
			bus_width = EXT_CSD_BUS_WIDTH_4;
		break;
	case 1:
		bus_width = EXT_CSD_BUS_WIDTH_1;
		break;
	default:
		printf("%s: Unknown bus width %d\n", __func__, mmc->bus_width);
		return -1;
	}

	if (hs_timing) {
		if ((cardtype & (MMC_HS_52MHZ | MMC_HS_DDR_52MHz_18_3V)) &&
		    (mmc->cfg->f_max >= 50000000)) {
			mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
			clock = min(52000000, (int)(mmc->cfg->f_max));
			debug("High-speed 52MHz timing mode detected\n");
		} else {
			mmc->card_caps |= MMC_MODE_HS;
			clock = min(26000000, (int)(mmc->cfg->f_max));
			debug("High-speed 26MHz timing mode detected\n");
		}
		if (ddr && (mmc->card_caps & MMC_MODE_HS_52MHz)) {
			mmc->card_caps |= MMC_MODE_DDR_52MHz;
			debug("DDR mode enabled\n");
		}
		if (slot->bus_id > 0) {
			/* NOTE:
			 *
			 * There are some issues with the mio_mmc_switch
			 * when the bus id is not 0.  This seems to fix a
			 * problem in a customer board with a Samsung KLMBG4GEND
			 * device which generates CRC errors at 52MHz.  The
			 * switch_exe function below fails when attempting to
			 * change the power class with a CRC error.
			 *
			 * It may be that it has nothing to do with it being
			 * on bus id 1 (bus id 0 is not used on the customer
			 * board).  If this turns out to be a problem with
			 * devices on bus id 0 as well then remove the above
			 * if statement.  In any event, it should be safe to do
			 * this regardless.
			 *
			 * It is also possible that EXT_CSD_BUS_WIDTH also needs
			 * to be manually set here.
			 */
#ifdef DEBUG
			debug("%s: Before manual setting hs timing (185)\n",
			      __func__);
			mmc_send_ext_csd(mmc, slot->ext_csd);
			slot->have_ext_csd = true;
#endif
			if (mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				       EXT_CSD_HS_TIMING, hs_timing ? 1 : 0))
				printf("%s: Error %sing extended CSD HS_TIMING\n",
				       __func__,
				       hs_timing ? "sett" : "clear");
		}
	} else {
		if (mmc->tran_speed)
			clock = min(clock, (int)(mmc->tran_speed));
		else
			clock = min(clock, 20000000);
		debug("High-speed clock mode NOT detected, setting to %dhz\n",
		      clock);
	}

#ifdef DEBUG
	debug("%s: extended CSD before switch_exe\n", __func__);
	mmc_send_ext_csd(mmc, slot->ext_csd);
	slot->have_ext_csd = true;
#endif
	slot->bus_width = bus_width;
	mmc_set_clock(mmc, clock);
	debug("%s: Clock set to %u Hz\n", __func__, mmc->clock);
	switch_timeout_ms = slot->ext_csd[EXT_CSD_GENERIC_CMD6_TIME] * 4;
	if (switch_timeout_ms == 0) {
		switch_timeout_ms = 2550 * 4;
		debug("extended CSD generic cmd6 timeout not specified\n");
	} else {
		debug("extended CSD generic cmd6 timeout %d ms\n",
		      switch_timeout_ms);
	}

	/* Adjust clock skew */
	emm_sample.u = 0;
	emm_sample.s.cmd_cnt = slot->cmd_clk_skew;
	emm_sample.s.dat_cnt = slot->dat_clk_skew;
	mmc_write_csr(mmc, MIO_EMM_SAMPLE, emm_sample.u);
	debug("%s: Setting command clock skew to %d, data to %d sclock cycles\n",
	      __func__, slot->cmd_clk_skew, slot->dat_clk_skew);

again:
	slot->clk_period = (host->sclock + mmc->clock - 1) / mmc->clock;

	debug("%s: Setting clock period to %d for MMC clock: %d, hs: %d\n",
	      __func__, slot->clk_period, mmc->clock, hs_timing);

	/* Set the watchdog since the switch operation can be long */
	mmc_set_watchdog(mmc, switch_timeout_ms * 1000);

	if (clock > 26000000) {
		if (bus_width & (EXT_CSD_DDR_BUS_WIDTH_4 |
				 EXT_CSD_DDR_BUS_WIDTH_8))
			index = EXT_CSD_PWR_CL_DDR_52_360;
		else
			index = EXT_CSD_PWR_CL_52_360;
	} else {
		index = EXT_CSD_PWR_CL_26_360;
	}

	power_class = slot->ext_csd[index];

	if (bus_width & (EXT_CSD_BUS_WIDTH_8 | EXT_CSD_DDR_BUS_WIDTH_8))
		power_class = (power_class >> 4) & 0xf;
	else
		power_class &= 0xf;

	debug("%s: Power class 0x%x selected\n", __func__, power_class);
	emm_switch.u = 0;
	emm_switch.s.bus_id = slot->bus_id;
	emm_switch.s.switch_exe = 1;
	emm_switch.s.hs_timing = hs_timing;
	emm_switch.s.bus_width = bus_width;
	emm_switch.s.power_class = power_class;
	slot->power_class = power_class;
	emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
	emm_switch.s.clk_lo = (slot->clk_period + 1) / 2;
	debug("%s: clock period: %u\n", __func__, slot->clk_period);
	debug("%s: Writing 0x%llx to mio_emm_switch\n",
	      __func__, emm_switch.u);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	udelay(100);

	timeout = (switch_timeout_ms + 10) * 10;
	do {
		emm_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);
		if (!emm_sts.s.switch_val)
			break;
		udelay(100);
	} while (timeout-- > 0);
	if (timeout <= 0) {
		printf("%s: switch command timed out, bus = %d, status=0x%llx\n",
		       __func__, slot->bus_id, emm_sts.u);
		return -1;
	}

	emm_switch.u = mmc_read_csr(mmc, MIO_EMM_SWITCH);
	debug("Switch command response: 0x%llx, switch: 0x%llx\n",
	      emm_sts.u, emm_switch.u);
#if defined(DEBUG)
	emm_rsp_lo.u = mmc_read_csr(mmc, MIO_EMM_RSP_LO);
	debug("Switch response lo: 0x%llx\n", emm_rsp_lo.u);
#endif

	if (emm_sts.s.rsp_crc_err && mmc->clock <= 20000000) {
		uint32_t next_speed;
		if (mmc->clock >= 4000000) {
			next_speed = mmc->clock - 2000000;
		} else if (mmc->clock > 1400000) {
			next_speed = mmc->clock - 1000000;
		} else if (mmc->clock > 400000) {
			next_speed = 400000;
		} else {
			printf("%s: CRC errors at 400KHz, MMC device unusable\n",
			       __func__);
			return -1;
		}
		printf("%s: CRC error communicating with MMC device at %uHz, trying %uHz.\n",
		       __func__, mmc->clock, next_speed);
		mmc->clock = next_speed;
		goto again;
	}

	if ((slot->bus_id > 0) && !emm_switch.s.switch_err0 &&
	    !emm_switch.s.switch_err1 && !emm_switch.s.switch_err2) {
		/* If the bus id is non-zero then the changes might not
		 * actually occur.  In this case, if there are no errors we
		 * apply the changes to bus 0 as well as the current bus id.
		 *
		 * If there are errors below then the operation will be retried
		 * anyway.
		 */
		emm_switch.s.bus_id = 0;
		emm_switch.s.switch_exe = 0;
		emm_switch.s.hs_timing = hs_timing;
		emm_switch.s.bus_width = bus_width;
		emm_switch.s.power_class = power_class;
		emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
		emm_switch.s.clk_lo = (slot->clk_period + 1) / 2;
		debug("%s: clock period: %u\n", __func__, slot->clk_period);
		mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
		mdelay(1);
		emm_switch.s.bus_id = slot->bus_id;
		mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
		mdelay(1);
	}
	if ((emm_switch.s.switch_err1 | emm_switch.s.switch_err2) &&
	    emm_sts.s.rsp_crc_err && mmc->clock > 20000000) {
		debug("%s: emm_switch error detected\n", __func__);
		mmc_print_registers(mmc);
		/* A CRC error occurred so try adjusting some things */
		/* If we're in DDR mode, turn off DDR. */
		switch (bus_width) {
		case EXT_CSD_DDR_BUS_WIDTH_8:
			bus_width = EXT_CSD_BUS_WIDTH_8;
			debug("%s: Switching from DDR 8 to non-DDR 8\n",
			      __func__);
			goto again;
		case EXT_CSD_DDR_BUS_WIDTH_4:
			bus_width = EXT_CSD_BUS_WIDTH_4;
			debug("%s: Switching from DDR 4 to non-DDR 4\n",
			      __func__);
			goto again;
		default:
			break;
		}
		/* A CRC error occurred during the switch operation, try slowing
		 * things down by 5MHz down to about 20MHz.
		 */
		mmc->clock = max(mmc->clock - 5000000, (uint)20000000);
		if (mmc->clock <= 20000000)
			hs_timing = 0;
		slot->clk_period = (host->sclock + mmc->clock - 1) / mmc->clock;
		debug("%s: bus_id %d detected CRC error, slowing clock down to %d and setting clock period to %d cycles\n",
		      __func__, slot->bus_id, mmc->clock, slot->clk_period);
		goto again;
	}
	if (emm_switch.s.switch_err0) {
		/* Error while performing POWER_CLASS switch */
		debug("%s: Error: Could not change power class to %d\n",
		      __func__, power_class);
	}
	if (emm_switch.s.switch_err1) {
		/* Error while performing HS_TIMING switch */
		if (ddr) {
			ddr = 0;
			debug("%s: Turning off DDR mode\n", __func__);
			mmc->card_caps &= ~MMC_MODE_DDR_52MHz;
			if (bus_width == EXT_CSD_DDR_BUS_WIDTH_8)
				bus_width = EXT_CSD_BUS_WIDTH_8;
			else if (bus_width == EXT_CSD_DDR_BUS_WIDTH_4)
				bus_width = EXT_CSD_BUS_WIDTH_4;
			goto again;
		}
		if (hs_timing) {
			if (clock >= 26000000) {
				clock = 26000000;
				debug("%s: Reducing clock to 26MHz\n",
				      __func__);
			} else {
				hs_timing = 0;
				debug("%s: Turning off high-speed timing\n",
				      __func__);
			}
			mmc_set_clock(mmc, clock);
			goto again;
		}
		printf("%s: Error setting hs timing\n", __func__);
		return -1;
	}

	/* CMD19 and CMD14 are only supported for MMC devices and only in
	 * the single data rate mode.  In the dual data rate mode these
	 * commands are illegal.
	 */
	if (!IS_SD(mmc)) {	/* Only MMC supports bus testing */
		debug("Testing bus width %d\n", mmc->bus_width);
		/* Test bus width */
		if (!emm_switch.s.switch_err2 && !ddr &&
		    (bus_width != EXT_CSD_DDR_BUS_WIDTH_8) &&
		    (bus_width != EXT_CSD_DDR_BUS_WIDTH_4)) {
			/* Width succeeded, test the bus */
			struct mmc_cmd mmc_cmd;
			struct mmc_data mmc_data;
			uint8_t buffer[16];

			debug("Testing bus width %d (%d)\n",
			      mmc->bus_width, bus_width);
			mmc_data.src = (char *)buffer;
			mmc_data.blocks = 1;
			mmc_data.flags = MMC_DATA_WRITE;

			switch (mmc->bus_width) {
			case 8:
				buffer[0] = 0x55;
				buffer[1] = 0xaa;
				buffer[2] = 0x00;
				buffer[3] = 0x00;
				buffer[4] = 0x00;
				buffer[5] = 0x00;
				buffer[6] = 0x00;
				buffer[7] = 0x00;
				buffer[8] = 0x05;
				buffer[9] = 0xd4;
				buffer[10] = 0xff;
				mmc_data.blocksize = 11;
				break;
			case 4:
				buffer[0] = 0x5a;
				buffer[1] = 0x00;
				buffer[2] = 0x00;
				buffer[3] = 0x00;
				buffer[4] = 0x99;
				buffer[5] = 0x50;
				buffer[6] = 0x0f;
				mmc_data.blocksize = 7;
				break;
			case 1:
				buffer[0] = 0x80;
				buffer[1] = 0x70;
				buffer[2] = 0x78;
				buffer[3] = 0x01;
				mmc_data.blocksize = 4;
				break;
			default:
				printf("Unknown bus width %d\n",
				       mmc->bus_width);
				return -1;
			}

#if defined(DEBUG)
			print_buffer(0, buffer, 1, mmc_data.blocksize + 7, 0);
#endif
			mmc_cmd.cmdarg = 0;
			mmc_cmd.cmdidx = 19;	/* BUSTEST_W */
			mmc_cmd.resp_type = MMC_RSP_R1;
			if (cavium_mmc_send_cmd(mmc->dev, &mmc_cmd, &mmc_data) != 0)
				puts("Warning: problem sending BUSTEST_W command\n");

			debug("BUSTEST_W response is 0x%x 0x%x 0x%x 0x%x\n",
			      mmc_cmd.response[0], mmc_cmd.response[1],
			      mmc_cmd.response[2], mmc_cmd.response[3]);
			mdelay(1);

			mmc_data.blocksize -= 2;
			memset(buffer, 0, sizeof(buffer));
			mmc_cmd.cmdarg = 0;
			mmc_cmd.cmdidx = 14;	/* BUSTEST_R */
			mmc_cmd.resp_type = MMC_RSP_R1;
			flags = MMC_CMD_FLAG_OFFSET(63);
			memset(buffer, 0, sizeof(buffer));
			mmc_data.dest = (char *)buffer;
			mmc_data.blocks = 1;
			mmc_data.flags = MMC_DATA_READ;
			if (mmc_send_cmd_flags(mmc, &mmc_cmd, &mmc_data, flags))
				puts("Warning: problem sending BUSTEST_R command\n");

			debug("BUSTEST_R response is 0x%x %x %x %x\n",
			      mmc_cmd.response[0], mmc_cmd.response[1],
			      mmc_cmd.response[2], mmc_cmd.response[3]);
#ifdef DEBUG
			mmc_send_ext_csd(mmc, slot->ext_csd);
#endif
#if defined(DEBUG)
			print_buffer(0, buffer, 1, mmc_data.blocksize + 8, 0);
#endif
			switch (bus_width) {
			case EXT_CSD_DDR_BUS_WIDTH_8:
				if (buffer[0] != 0xaa || buffer[1] != 0x55) {
					debug("DDR Bus width 8 test failed, returned 0x%02x%02x, expected 0xAA55, trying bus width 8\n",
					      buffer[0], buffer[1]);
					bus_width = EXT_CSD_DDR_BUS_WIDTH_4;
					mmc->bus_width = 4;
					goto again;
				}
				break;
			case EXT_CSD_DDR_BUS_WIDTH_4:
				if (buffer[0] != 0xa5) {
					debug("DDR Bus width 4 test failed, returned 0x%02x%02x, expected 0xA5, trying bus width %d\n",
					      buffer[0], buffer[1],
					      slot->bus_max_width);
					bus_width = (slot->bus_max_width == 8) ?
							EXT_CSD_BUS_WIDTH_8 :
							EXT_CSD_BUS_WIDTH_4;
					mmc->bus_width = slot->bus_max_width;
					goto again;
				}
				break;
			case EXT_CSD_BUS_WIDTH_8:
				if (buffer[0] != 0xaa || buffer[1] != 0x55) {
					debug("Bus width 8 test failed, returned 0x%02x%02x, expected 0xAA55, trying bus width 4\n",
					      buffer[0], buffer[1]);
					bus_width = EXT_CSD_BUS_WIDTH_4;
					mmc->bus_width = 4;
					goto again;
				}
				break;
			case EXT_CSD_BUS_WIDTH_4:
				if (buffer[0] != 0xa5) {
					debug("DDR bus width 4 test failed, returned 0x%02x, expected 0xA5, trying bus width 1\n",
					      buffer[0]);
					bus_width = EXT_CSD_BUS_WIDTH_1;
					mmc->bus_width = 1;
					goto again;
				}
				break;
			case EXT_CSD_BUS_WIDTH_1:
				if ((buffer[0] & 0xc0) != 0x40) {
					debug("DDR bus width 1 test failed, returned 0x%02x, expected 0x4x, trying bus width 1\n",
					      buffer[0]);
					return -1;
				}
				break;
			default:
				break;
			}
		}

		if (emm_switch.s.switch_err2) {
			/* Error while performing BUS_WIDTH switch */
			switch (bus_width) {
			case EXT_CSD_DDR_BUS_WIDTH_8:
				debug("DDR bus width 8 failed, trying DDR bus width 4\n");
				bus_width = EXT_CSD_DDR_BUS_WIDTH_4;
				mmc->bus_width = 4;
				goto again;
			case EXT_CSD_DDR_BUS_WIDTH_4:
				debug("DDR bus width 4 failed, trying bus width %d\n",
				      slot->bus_max_width);
				bus_width = (slot->bus_max_width == 8) ?
						EXT_CSD_BUS_WIDTH_8
						: EXT_CSD_BUS_WIDTH_4;
				mmc->bus_width = slot->bus_max_width;
				goto again;
			case EXT_CSD_BUS_WIDTH_8:
				debug("Bus width 8 failed, trying bus width 4\n");
				bus_width = EXT_CSD_BUS_WIDTH_4;
				mmc->bus_width = 4;
				goto again;
			case EXT_CSD_BUS_WIDTH_4:
				debug("Bus width 4 failed, trying bus width 1\n");
				bus_width = EXT_CSD_BUS_WIDTH_1;
				mmc->bus_width = 1;
				goto again;
			default:
				printf("%s: Could not set bus width\n",
				       __func__);
				return -1;
			}
		}
	}

	if (ddr && hs_timing && clock >= 26000000) {
		if (clock >= 52000000)
			mmc->card_caps |= MMC_MODE_DDR_52MHz;

		switch (mmc->bus_width) {
		case 8:
			bus_width = EXT_CSD_DDR_BUS_WIDTH_8;
			break;
		case 4:
			bus_width = EXT_CSD_DDR_BUS_WIDTH_4;
			break;
		default:
			puts("Error: MMC DDR mode only supported with bus widths of 4 or 8!\n");
			return -1;
		}
		emm_switch.u = 0;
		emm_switch.s.bus_id = slot->bus_id;
		emm_switch.s.switch_exe = 1;
		emm_switch.s.hs_timing = 1;
		emm_switch.s.bus_width = bus_width;
		emm_switch.s.power_class = slot->power_class;
		emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
		emm_switch.s.clk_lo = (slot->clk_period + 1) / 2;
		debug("%s: clock period: %u\n", __func__, slot->clk_period);
		mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
		udelay(100);

		timeout = switch_timeout_ms + 10;
		do {
			emm_sts.u = mmc_read_csr(mmc, MIO_EMM_RSP_STS);
			if (!emm_sts.s.switch_val)
				break;
			mdelay(1);
		} while (timeout-- > 0);
		if (timeout < 0) {
			printf("Error: MMC timed out when converting to DDR mode\n");
			return -1;
		}
	}
	/* Store the bus width */
	slot->bus_width = emm_switch.s.bus_width;
	mmc->ddr_mode = ddr;
	/* Set watchdog for command timeout */
	mmc_set_watchdog(mmc, 1000000);

	debug("%s: Set hs: %d, ddr: %s, clock: %u, bus width: %d, power class: %d\n",
	      __func__, hs_timing, ddr ? "yes" : "no",
	      mmc->clock, bus_width, power_class);

	/* Re-read the extended CSD register since it has likely changed */
	mmc_send_ext_csd(mmc, slot->ext_csd);
	slot->have_ext_csd = true;

	return 0;
}

/**
 * Set the clock speed.
 *
 * NOTE: The clock speed will be limited to the maximum supported clock speed.
 */
void mmc_set_clock(struct mmc *mmc, uint clock)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	struct cavium_mmc_host *host = slot->host;
	union mio_emm_switch emm_switch;
	unsigned bus;

	debug("%s(%s, %u)\n", __func__, mmc->cfg->name, clock);
	debug("%s: min: %u, max: %u, trans: %u, hs: %u, set: %u\n",
	      __func__, mmc->cfg->f_min, mmc->cfg->f_max, mmc->tran_speed,
	      (mmc->card_caps & MMC_MODE_HS) ? 1 : 0, clock);
	if (clock == 0) {
		printf("%s: ERROR: Cannot set clock to zero!\n", __func__);
		return;
	}
	clock = min(clock, mmc->cfg->f_max);
	clock = max(clock, mmc->cfg->f_min);
	if (mmc->tran_speed && !(mmc->card_caps & MMC_MODE_HS)) {
		clock = min(clock, mmc->tran_speed);
		debug("%s: Limiting clock to trans speed %u\n",
		      __func__, mmc->tran_speed);
	}
	debug("%s: Setting clock to %uHz\n", __func__, clock);
	mmc->clock = clock;
	slot->clk_period = (host->sclock + clock - 1) / clock;

	debug("%s: Reading MIO_EMM_SWITCH\n", __func__);
	/* Write the change to the hardware */
	emm_switch.u = mmc_read_csr(mmc, MIO_EMM_SWITCH);
	emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
	emm_switch.s.clk_lo = emm_switch.s.clk_hi;
	bus = emm_switch.s.bus_id;
	debug("%s: clock period: %u, bus: %u\n", __func__,
	      slot->clk_period, bus);

	emm_switch.s.bus_id = 0;
	emm_switch.s.hs_timing = (mmc->clock > 20000000);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	debug("  mio_emm_switch: 0x%llx\n", mmc_read_csr(mmc, MIO_EMM_SWITCH));
	debug("  mio_emm_mode0: 0x%llx\n", mmc_read_csr(mmc, MIO_EMM_MODEX(0)));
	udelay(1200);
	emm_switch.s.bus_id = bus;
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	udelay(1200);
	debug("  mio_emm_mode%d: 0x%llx\n", bus,
	      mmc_read_csr(mmc, MIO_EMM_MODEX(bus)));

	mmc_set_watchdog(mmc, 1000000);
}

#ifdef CONFIG_CAVIUM_MMC_SD
static int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u32 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	debug("%s(%s, %d, %d, %u, 0x%p)\n",
	      __func__, mmc->cfg->name, mode, group, value, resp);
	/* Switch the frequency */
	cmd.cmdidx = SD_CMD_SWITCH_FUNC;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = (mode << 31) | 0xffffff;
	cmd.cmdarg &= ~(0xf << (group * 4));
	cmd.cmdarg |= value << (group * 4);

	data.dest = (char *)resp;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd_flags(mmc, &cmd, &data,  MMC_CMD_FLAG_CTYPE_XOR(1));
	if (err) {
		printf("%s: failed, rc: %d\n", __func__, err);
		return err;
	}
	memcpy(resp, &cmd.response[0], sizeof(cmd.response));
	return 0;
}

static int sd_change_freq(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	uint32_t scr[2];
	uint32_t switch_status[16];
	struct mmc_data data;
	int timeout;
	struct cavium_mmc_slot *slot = mmc->priv;
	uint32_t flags;
#ifdef DEBUG
	int i;
#endif

	debug("%s(%s)\n", __func__, mmc->cfg->name);

#ifdef DEBUG
	memset(scr, 0x55, sizeof(scr));
	memset(switch_status, 0xaa, sizeof(switch_status));
#endif
	mmc->card_caps = 0;

	cmd.cmdidx = SD_CMD_APP_SEND_SCR;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;
	flags = MMC_CMD_FLAG_RTYPE_XOR(1) | MMC_CMD_FLAG_CTYPE_XOR(1)
		    | MMC_CMD_FLAG_OFFSET(63);

	timeout = 3;

retry_scr:
	data.dest = (char *)&scr;
	data.blocksize = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_acmd(mmc, &cmd, &data, flags);

	if (err) {
		debug("Retrying send SCR\n");
		if (timeout--)
			goto retry_scr;

		return err;
	}

	mmc->scr[0] = __be32_to_cpu(scr[0]);
	mmc->scr[1] = __be32_to_cpu(scr[1]);

	debug("%s: SCR=0x%08x 0x%08x\n", __func__, mmc->scr[0], mmc->scr[1]);
	switch ((mmc->scr[0] >> 24) & 0xf) {
	case 0:
		mmc->version = SD_VERSION_1_0;
		break;
	case 1:
		mmc->version = SD_VERSION_1_10;
		break;
	case 2:
		if (((mmc->scr[0] >> 10) & 1) && ((mmc->scr[0] >> 15) & 1))
			mmc->version = SD_VERSION_4;
		else if ((mmc->scr[0] >> 15) & 1)
			/* NOTE: should check SD version 4 here but
			 * U-Boot doesn't yet define this.
			 */
			mmc->version = SD_VERSION_3;
		else
			mmc->version = SD_VERSION_2;
		break;
	default:
		mmc->version = SD_VERSION_1_0;
		break;
	}

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0) {
		debug("%s: Returning for SD version 1.0\n", __func__);
		return 0;
	}

	timeout = 4;
	while (timeout--) {
		err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1, switch_status);

		if (err) {
			debug("%s: Error calling sd_switch\n", __func__);
			return err;
		}

		/* The high-speed function is busy.  Try again */
		if (!((switch_status[7]) & SD_HIGHSPEED_BUSY)) {
			debug("%s: high speed function is !busy, done\n",
			      __func__);
			break;
		}
		mdelay(1);
	}

#if defined(DEBUG)
	for (i = 0; i < 16; i++) {
		if ((i & 3) == 0)
			debug("\n%02x: ", i * 4);
		debug("%08x ", switch_status[i]);
	}
	puts("\n");
#endif

	if (mmc->scr[0] & SD_DATA_4BIT && slot->bus_max_width >= 4) {
		mmc->card_caps |= MMC_MODE_4BIT;
		mmc->bus_width = 4;
		debug("%s: SD 4 bit mode detected\n", __func__);
	} else {
		debug("%s: SD 4-bit mode NOT detected\n", __func__);
	}
	if (mmc->scr[0] & 2) {
		slot->flags |= OCTEON_MMC_FLAG_SD_CMD23;
		debug("%s: SD CMD23 support detected\n", __func__);
	}
#if defined(DEBUG)
	debug("        max current: %u ma\n", switch_status[0] >> 16);
	if (switch_status[0] & 0xffff) {
		debug("        Group 6 functions supported: ");
		for (i = 0; i < 16; i++)
			if (((switch_status[0] >> i) & 1) != 0)
				debug("%i ", i);
		debug("\n");
	}
	if (switch_status[1] & 0xffff0000) {
		debug("        Group 5 functions supported: ");
		for (i = 0; i < 16; i++)
			if ((switch_status[1] >> (i + 16)) & 1)
				debug("%i ", i);
		debug("\n");
	}
	if (switch_status[1] & 0xffff) {
		debug("        Group 4 functions supported: ");
		for (i = 0; i < 16; i++)
			if (((switch_status[1] >> i) & 1) != 0)
				debug("%i ", i);
		debug("\n");
	}
	if (switch_status[2] & 0xffff0000) {
		debug("        Group 3 functions supported: ");
		for (i = 0; i < 16; i++)
			if (((switch_status[2] >> (i + 16)) & 1) != 0)
				debug("%i ", i);
		debug("\n");
	}
	if (switch_status[2] & 0x0000ffff) {
		debug("        Group 2 functions supported: ");
		for (i = 0; i < 16; i++)
			if (((switch_status[1] >> i) & 1) != 0)
				debug("%i ", i);
		debug("\n");
	}
	if (switch_status[3] & 0xffff0000) {
		debug("        Group 1 functions supported: ");
		for (i = 0; i < 16; i++)
			if (((switch_status[2] >> (i + 16)) & 1) != 0)
				debug("%i ", i);
		debug("\n");
	}
	if (switch_status[3] & 0x0000f000)
		debug("        Group 6 functions selected: 0x%x\n",
		      (switch_status[3] >> 12) & 0xF);
	if (switch_status[3] & 0x00000f00)
		debug("        Group 5 functions selected: 0x%x\n",
		      (switch_status[3] >> 8) & 0xF);

	if (switch_status[3] & 0x000000f0)
		debug("        Group 4 functions selected: 0x%x\n",
		      (switch_status[3] >> 4) & 0xF);
	if (switch_status[3] & 0x0000000f)
		debug("        Group 3 functions selected: 0x%x\n",
		      switch_status[3] & 0xF);
	if (switch_status[4] & 0xf0000000)
		debug("        Group 2 functions selected: 0x%x\n",
		      (switch_status[4] >> 28) & 0xF);
	if (switch_status[4] & 0x0f000000)
		debug("        Group 1 functions selected: 0x%x\n",
		      (switch_status[4] >> 24) & 0xF);
	debug("        Data structure version: %d\n",
	      (switch_status[4] >> 16) & 0xff);

	if (!((switch_status[4] & SD_HIGHSPEED_SUPPORTED)))
		debug("%s: high speed mode not supported\n", __func__);
	else
		debug("%s: high speed mode supported\n", __func__);

#endif	/* DEBUG */

	err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, switch_status);

	if (err) {
		debug("%s: switch failed\n", __func__);
		return err;
	}

#ifdef DEBUG
	for (i = 0; i < 16; i++) {
		if (!(i & 3))
			debug("\n%02x: ", i * 4);
		debug("%08x ", switch_status[i]);
	}
	puts("\n");
#endif
	if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000) {
		mmc->card_caps |= MMC_MODE_HS;
		debug("%s: High speed mode supported\n", __func__);
	}

	return 0;
}
#endif

#ifdef CONFIG_CAVIUM_MMC_SD
static int sd_version_1_x(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;
	uint32_t flags;
	ulong start;

	debug("%s(%s)\n", __func__, mmc->cfg->name);

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	cmd.cmdarg = 0;
	flags = MMC_CMD_FLAG_CTYPE_XOR(1);

	err = mmc_send_cmd_timeout(mmc, &cmd, NULL, flags, 5);
	if (!err) {
		mmc->version = SD_VERSION_2;
		return 0;
	}

	err = mmc_go_idle(mmc);
	if (err)
		debug("%s: mmc_go_idle() returned error\n", __func__);

	start = get_timer(0);
	do {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1;
		err = mmc_send_cmd_flags(mmc, &cmd, NULL, 0);
		if (err) {
			debug("%sL ACMD failed\n", __func__);
			return err;
		}
		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
		cmd.cmdarg = 0x00ff8000;
		cmd.resp_type = MMC_RSP_R3;
		flags = MMC_CMD_FLAG_RTYPE_XOR(3) | MMC_CMD_FLAG_STRIP_CRC;
		err = mmc_send_cmd_flags(mmc, &cmd, NULL, flags);
		if (err) {
			debug("%s: ACMD41 failed\n", __func__);
			return err;
		}
		debug("%s: ACMD41 response: 0x%x\n", __func__, cmd.response[0]);
	} while (!(cmd.response[0] & OCR_BUSY) && get_timer(start) < 100);
	if (!(cmd.response[0] & OCR_BUSY)) {
		debug("%s: ACMD41 timed out\n", __func__);
		return -1;
	}
	return 0;
}
#endif

static int mmc_send_if_cond(struct mmc *mmc)
{
#ifdef CONFIG_CAVIUM_MMC_SD
	struct mmc_cmd cmd;
	int err;
	uint32_t flags;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	/* We only need a very short timeout here, 5ms */
	mmc_set_watchdog(mmc, 5000);

	memset(&cmd, 0, sizeof(cmd));
	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = ((mmc->cfg->voltages & OCR_VOLTAGE_MASK) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;
	flags = MMC_CMD_FLAG_CTYPE_XOR(1) | MMC_CMD_FLAG_RTYPE_XOR(2);

	err = mmc_send_cmd_timeout(mmc, &cmd, NULL, flags, 5);

	if (err) {
		debug("%s failed\n", __func__);
		err = sd_version_1_x(mmc);
		if (err) {
			return err;
		} else {
			debug("%s: detected SD version 1.x\n", __func__);
			mmc->version = SD_VERSION_1_0;
			mmc_go_idle(mmc);
			return 0;
		}
	}

	if ((cmd.response[0] & 0xff) != 0xaa) {
		debug("%s: Unusable error, response is 0x%x\n",
		      __func__, cmd.response[0]);
		return ENOTRECOVERABLE;
	} else {
		mmc->version = SD_VERSION_2;
		debug("%s: SD version 2 detected\n", __func__);
	}
#endif
	return 0;
}

static void mmc_reset_bus(struct mmc *mmc, int preserve_switch)
{
	struct cavium_mmc_slot *slot = (struct cavium_mmc_slot *)mmc->priv;
	union mio_emm_cfg emm_cfg;
	union mio_emm_switch emm_switch;

	debug("%s(%s, %d)\n", __func__, mmc->cfg->name, preserve_switch);
	if (preserve_switch) {
		emm_switch.u = mmc_read_csr(mmc, MIO_EMM_SWITCH);
		if (emm_switch.s.bus_id != slot->bus_id) {
			emm_switch.s.bus_id = slot->bus_id;
			mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
		}
	}

	/* Reset the bus */
	emm_cfg.u = mmc_read_csr(mmc, MIO_EMM_CFG);
	emm_cfg.u &= ~(1 << slot->bus_id);
	mmc_write_csr(mmc, MIO_EMM_CFG, emm_cfg.u);
	mdelay(20);	/* Wait 20ms */
	emm_cfg.u |= 1 << slot->bus_id;
	mmc_write_csr(mmc, MIO_EMM_CFG, emm_cfg.u);

	mdelay(20);

	/* Restore switch settings */
	if (preserve_switch) {
		emm_switch.s.switch_exe = 0;
		emm_switch.s.switch_err0 = 0;
		emm_switch.s.switch_err1 = 0;
		emm_switch.s.switch_err2 = 0;
		emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
		emm_switch.s.clk_lo = (slot->clk_period + 1) / 2;
		debug("%s: clock period: %u\n", __func__, slot->clk_period);
		emm_switch.s.hs_timing = (mmc->clock > 20000000);
		emm_switch.s.bus_id = 0;
		mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
		udelay(100);
		emm_switch.s.bus_id = slot->bus_id;
		mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	}
}

static int mmc_set_dsr_cmd(struct mmc *mmc)
{
	struct mmc_cmd cmd;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	cmd.cmdidx = MMC_CMD_SET_DSR;
	cmd.cmdarg = (mmc->dsr & 0xffff) << 16;
	debug("%s: Setting DSR to 0x%04x\n", __func__, mmc->dsr);
	cmd.resp_type = MMC_RSP_NONE;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

int mmc_startup(struct mmc *mmc)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	struct cavium_mmc_host *host = slot->host;
	u64 cmult, csize, capacity;
	int err;
	uint mult, freq;
	union mio_emm_switch emm_switch;
	union mio_emm_cfg emm_cfg;
	union mio_emm_sts_mask emm_sts_mask;
	union mio_emm_wdog emm_wdog;
	int i;
#ifdef DEBUG
	int classes;
#endif
	uint8_t *ext_csd = slot->ext_csd;
	bool has_parts = false;
	bool part_completed;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);

	debug("%s(%s): bus_id: %d\n", __func__, mmc->cfg->name, slot->bus_id);

	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return -ENODEV;
	}
	mmc->rca = 0;

	/* Clear interrupt status */
	mmc_write_csr(mmc, MIO_EMM_INT, mmc_read_csr(mmc, MIO_EMM_INT));

	/* Enable the bus */
	emm_cfg.u = mmc_read_csr(mmc, MIO_EMM_CFG);
	emm_cfg.u |= (1 << slot->bus_id);
	debug("%s: writing 0x%llx to mio_emm_cfg\n", __func__, emm_cfg.u);
	mmc_write_csr(mmc, MIO_EMM_CFG, emm_cfg.u);
	mdelay(2);

	/* Set clock period */
	slot->clk_period = (host->sclock + mmc->clock - 1) / mmc->clock;

	/* Default to RCA of 1 */
	mmc_write_csr(mmc, MIO_EMM_RCA, 1);

	/* Set the bus speed and width */
	emm_switch.u = 0;
	emm_switch.s.bus_width = EXT_CSD_BUS_WIDTH_1;
	emm_switch.s.power_class = 10;
	emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
	emm_switch.s.clk_lo = emm_switch.s.clk_hi;
	debug("%s: clock period: %u\n", __func__, slot->clk_period);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	emm_switch.s.bus_id = slot->bus_id;
	udelay(1200);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	udelay(1200);

	host->last_slotid = host->cur_slotid;

#ifdef DEBUG
	debug("%s: Set clock period to %d clocks, sclock: %llu\n", __func__,
	      emm_switch.s.clk_hi + emm_switch.s.clk_lo, host->sclock);
#endif
	/* Set watchdog for command timeout */
	if (slot->bus_id == 0)
		emm_wdog.u = 0;
	else
		emm_wdog.u = mmc_read_csr(mmc, MIO_EMM_WDOG);
	emm_wdog.s.clk_cnt = mmc->clock;
	debug("Setting command timeout value to %u\n", emm_wdog.s.clk_cnt);
	mmc_write_csr(mmc, MIO_EMM_WDOG, emm_wdog.u);

	mdelay(10);	/* Wait 10ms */

	/* Set status mask */
	emm_sts_mask.u = 0;
	emm_sts_mask.s.sts_msk = 1 << 7 | 1 << 22 | 1 << 23 | 1 << 19;
	mmc_write_csr(mmc, MIO_EMM_STS_MASK, emm_sts_mask.u);

	/* Reset the card */
	debug("Resetting card\n");
	err = mmc_pre_idle(mmc);
	if (err) {
		mmc_print_registers(mmc);
		if (!init_time)
			printf("%s: Could not enter pre-idle state\n",
			       __func__);
		return err;
	}

	err = mmc_go_idle(mmc);
	if (err) {
		mmc_print_registers(mmc);
		if (!init_time)
			printf("%s: Could not reset MMC card\n", __func__);
		return err;
	}

#ifdef CONFIG_CAVIUM_MMC_SD
	/* Note that this doesn't work on the CN61XX pass 1.0.
	 * The CN61XX pass 1.0 has an errata where only 8-bit wide buses are
	 * supported due to checksum errors on narrower busses.
	 */
	debug("Testing for SD version 2, voltages: 0x%x\n", mmc->cfg->voltages);
	/* Test for SD version 2 */
	err = mmc_send_if_cond(mmc);
#endif
	/* Now try to get the SD card's operating condition */
	if (!err && IS_SD(mmc)) {
#ifdef CONFIG_CAVIUM_MMC_SD
		debug("Getting SD card operating condition\n");
		err = sd_send_op_cond(mmc);
		if (err == ETIMEDOUT) {
			debug("Cannot get SD operating condition, trying MMC\n");
			err = mmc_send_op_cond(mmc);
			if (err) {
				printf("Card not present or defective.  Card did not respond to voltage select!\n");
				return ENOTRECOVERABLE;
			}
		}

		if (err) {
			err = mmc_send_op_cond(mmc);
			if (err) {
				puts("MMC Init: Error recovering after SD Version 2 test\n");
				return ENOTRECOVERABLE;
			}
		}
#endif
	} else {
		mdelay(100);
		/* Clear interrupt status */
		mmc_write_csr(mmc, MIO_EMM_INT, mmc_read_csr(mmc, MIO_EMM_INT));
		debug("Resetting card for MMC\n");
		mmc_pre_idle(mmc);
		mdelay(2);
		mmc_go_idle(mmc);
		mdelay(2);

		debug("Getting MMC card operating condition\n");
		err = mmc_send_op_cond(mmc);
		if (err == ETIMEDOUT) {
			debug("Trying again...\n");
			/* Resetting MMC bus */
			mmc_reset_bus(mmc, true);
			debug("%s: Going idle\n", __func__);
			mmc_pre_idle(mmc);
			mmc_go_idle(mmc);
			err = mmc_send_op_cond(mmc);
			if (err) {
				debug("MMC Init: Card did not respond to voltage select, might be 1.x SD card\n");
				mmc->version = SD_VERSION_1_0;
				mmc_reset_bus(mmc, true);
				mmc_go_idle(mmc);
			}
		} else if (err) {
			debug("Resetting MMC bus\n");
			/* Resetting MMC bus */
			mmc_reset_bus(mmc, true);
			debug("%s: Going idle\n", __func__);
			mmc_pre_idle(mmc);
			mmc_go_idle(mmc);
			mdelay(100);
			err = mmc_send_op_cond(mmc);
			if (err) {
				puts("MMC Init: Error recovering after SD Version 2 test\n");
				return ENOTRECOVERABLE;
			}
		}
	}

	debug("%s: Getting CID\n", __func__);
	err = mmc_all_send_cid(mmc);
	if (err) {
		debug("%s: Error getting CID, card may be missing\n", __func__);
		return err;
	}

	/* For MMC cards, set the Relative Address.
	 * For SD cards, get the Relative address.
	 * This also puts the cards into Standby State.
	 */
	if (IS_SD(mmc)) {
#ifdef CONFIG_CAVIUM_MMC_SD
		debug("%s: Getting SD relative address\n", __func__);
		err = sd_send_relative_addr(mmc);
		if (err) {
			printf("%s: Error getting RCA\n", __func__);
			return err;
		}
#endif
	} else {
		mmc->rca = slot->bus_id + 0x10;	/* RCA must be > 1 */
		debug("%s: Setting MMC relative address to %d\n",
		      __func__, mmc->rca);
		err = mmc_set_relative_addr(mmc);
		if (err) {
			mmc_print_registers(mmc);
			printf("%s: Error setting MMC RCA to %d\n",
			       __func__, mmc->rca);
			return err;
		}
	}

	debug("Getting CSD\n");
	err = mmc_get_csd(mmc);
	if (err) {
		printf("%s: Error getting CSD\n", __func__);
		emm_switch.u = mmc_read_csr(mmc, MIO_EMM_SWITCH);
		debug("clk period: %d\n",
		      emm_switch.s.clk_hi + emm_switch.s.clk_lo);
		return err;
	}
	if (mmc->version == MMC_VERSION_UNKNOWN) {
		int version = get_csd_bits(mmc, 122, 125);

		switch (version) {
		case 0:
			mmc->version = MMC_VERSION_1_2;
			debug("MMC version 1.2 detected\n");
			break;
		case 1:
			mmc->version = MMC_VERSION_1_4;
			debug("MMC version 1.4 detected\n");
			break;
		case 2:
			mmc->version = MMC_VERSION_2_2;
			debug("MMC version 2.2 detected\n");
			break;
		case 3:
			mmc->version = MMC_VERSION_3;
			debug("MMC version 3 detected\n");
			break;
		case 4:
			mmc->version = MMC_VERSION_4;
			debug("MMC version 4 detected\n");
			break;
		default:
			mmc->version = MMC_VERSION_1_2;
			debug("MMC version 1.2 (unknown) detected\n");
			break;
		}
	}

	mmc->dsr_imp = get_csd_bits(mmc, 76, 76);

#ifdef DEBUG
	i = 0;
	classes = get_csd_bits(mmc, 84, 95);
	debug("Classes supported: ");
	while (classes) {
		if (classes & 1)
			debug("%i ", i);
		classes >>= 1;
		i++;
	}
	debug("\n%s: Read block length: %u\n", __func__,
	      1 << ((mmc->csd[1] >> 16) & 0xF));
#endif

	if (IS_SD(mmc)) {
		switch (get_csd_bits(mmc, 96, 103)) {
		case 0x0b:
			mmc->tran_speed = 50000000;
			break;
		case 0x2b:
			mmc->tran_speed = 200000000;
			break;
		case 0x32:
			mmc->tran_speed = 25000000;
			break;
		case 0x5a:
			mmc->tran_speed = 50000000;
			break;
		default:
			printf("Unknown tran_speed value 0x%x in SD CSD register\n",
			       mmc->csd[0] & 0x7f);
			mmc->tran_speed = 25000000;
			break;
		}
	} else {
		const uint32_t tran_speed_freq[8] = {
			10000, 100000, 1000000, 10000000, 0, 0, 0, 0
		};
		const uint32_t tran_mult[16] = {
			 0, 10, 12, 13, 15, 20, 26, 30,
			35, 40, 45, 52, 55, 60, 70, 80
		};

		freq = tran_speed_freq[get_csd_bits(mmc, 96, 98)];
		mult = tran_mult[get_csd_bits(mmc, 99, 102)];

		mmc->tran_speed = freq * mult;
	}

	debug("%s CSD tran_speed: %u\n",
	      IS_SD(mmc) ? "SD" : "MMC", mmc->tran_speed);

	mmc->read_bl_len = 1 << get_csd_bits(mmc, 80, 83);

	if (IS_SD(mmc))
		mmc->write_bl_len = mmc->read_bl_len;
	else
		mmc->write_bl_len = 1 << get_csd_bits(mmc, 22, 25);

	if (mmc->high_capacity) {
		csize = (mmc->csd[1] & 0x3f) << 16
			| (mmc->csd[2] & 0xffff0000) >> 16;
		cmult = 8;
	} else {
		csize = get_csd_bits(mmc, 62, 73);
		cmult = get_csd_bits(mmc, 47, 49);
		debug("  csize: 0x%llx, cmult: 0x%llx\n", csize, cmult);
	}

	debug("  read_bl_len: 0x%x\n", mmc->read_bl_len);
	mmc->capacity_user = (csize + 1) << (cmult + 2);
	mmc->capacity_user *= mmc->read_bl_len;
	mmc->capacity_boot = 0;
	mmc->capacity_rpmb = 0;
	debug("%s: mmc capacity user: %llu, csize: %llu, cmult: %llu\n",
	      __func__, mmc->capacity_user, csize, cmult);
	for (i = 0; i < 4; i++)
		mmc->capacity_gp[i] = 0;

	debug("%s: capacity: %llu bytes (%llu blocks)\n", __func__,
	      mmc->capacity, mmc->capacity / mmc->read_bl_len);
	debug("%s: read to write program time factor: %d\n", __func__,
	      1 << ((mmc->csd[0] >> 26) & 7));

	mmc->erase_grp_size = 1;
	mmc->read_bl_len = min((int)mmc->read_bl_len, 512);
	mmc->write_bl_len = min((int)mmc->write_bl_len, 512);

	/* Select the card and put it into Transfer Mode */

	debug("%s: Selecting card to rca %d\n", __func__, mmc->rca);

	if ((mmc->dsr_imp) && (0xffffffff != mmc->dsr)) {
		if (mmc_set_dsr_cmd(mmc))
			printf("MMC: SET_DSR failed\n");
	}

	err = mmc_select_card(mmc);
	if (err) {
		printf("%s: Error selecting card\n", __func__);
		return err;
	}

	if (IS_SD(mmc)) {
		debug("SD version: ");
		switch (mmc->version) {
		case SD_VERSION_4:
			debug("4\n");
			break;
		case SD_VERSION_3:
			debug("3\n");
			break;
		case SD_VERSION_2:
			debug("2\n");
			break;
		case SD_VERSION_1_0:
			debug("1.0\n");
			break;
		case SD_VERSION_1_10:
			debug("1.10\n");
			break;
		default:
			debug("Undefined\n");
			break;
		}
		debug("sd version 0x%x\n", mmc->version);

		mmc->erase_grp_size = 1;
		mmc->part_config = MMCPART_NOAVAILABLE;
	} else {
		debug("MMC version: ");
		switch (mmc->version) {
		case MMC_VERSION_UNKNOWN:
			debug("UNKNOWN\n");
			break;
		case MMC_VERSION_1_2:
			debug("1.2\n");
			break;
		case MMC_VERSION_1_4:
			debug("1.4\n");
			break;
		case MMC_VERSION_2_2:
			debug("2.2\n");
			break;
		case MMC_VERSION_3:
			debug("3\n");
			break;
		case MMC_VERSION_4:
			debug("4\n");
			break;
		case MMC_VERSION_4_41:
			debug("4.41\n");
			break;
		case MMC_VERSION_4_5:
			debug("4.5\n");
			break;
		case MMC_VERSION_5_0:
			debug("5.0\n");
			break;
		case MMC_VERSION_5_1:
			debug("5.1\n");
			break;
		default:
			debug("Undefined\n");
			break;
		}
		debug("mmc version 0x%x\n", mmc->version);
	}

	if (!IS_SD(mmc) && mmc->version >= MMC_VERSION_4) {
		/* Check ext_csd version and capacity */
		err = mmc_send_ext_csd(mmc, slot->ext_csd);
		if (err) {
			printf("%s: Error: cannot read extended CSD\n",
			       __func__);
			return -1;
		}
		slot->have_ext_csd = true;
		if (ext_csd[EXT_CSD_REV] >= 2) {
			capacity = ext_csd[EXT_CSD_SEC_CNT + 0] << 0  |
				   ext_csd[EXT_CSD_SEC_CNT + 1] << 8  |
				   ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
				   ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
				debug("MMC EXT CSD reports capacity of %llu sectors (0x%llx bytes)\n",
				      capacity, capacity * 512);
			capacity *= 512;
			if (((capacity >> 20) > 2 * 1024) && mmc->high_capacity)
				mmc->capacity_user = capacity;
			debug("%s: Set MMC capacity to %llu\n",
			      __func__, mmc->capacity);
		}
		debug("%s: EXT_CSD_REV: 0x%x\n", __func__,
		      ext_csd[EXT_CSD_REV]);
		switch (ext_csd[EXT_CSD_REV]) {
		case 1:
			mmc->version = MMC_VERSION_4_1;
			break;
		case 2:
			mmc->version = MMC_VERSION_4_2;
			break;
		case 3:
			mmc->version = MMC_VERSION_4_3;
			break;
		case 5:
			mmc->version = MMC_VERSION_4_41;
			break;
		case 6:
			mmc->version = MMC_VERSION_4_5;
			break;
		case 7:
			mmc->version = MMC_VERSION_5_0;
			break;
		case 8:
			mmc->version = MMC_VERSION_5_1;
			break;
		default:
			debug("%s: Unknown extended CSD revision 0x%x\n",
			      __func__, ext_csd[EXT_CSD_REV]);
		}
		debug("%s(%s): version: 0x%x\n", __func__,
		      mmc->cfg->name, mmc->version);
		/* The partition data may be non-zero but it is only
		 * effective if PARTITION_SETTING_COMPLETED is set in
		 * EXT_CSD, so ignore any data if this bit is not set,
		 * except for enabling the high-capacity group size
		 * definition (see below). */
		part_completed = !!(ext_csd[EXT_CSD_PARTITION_SETTING] &
				    EXT_CSD_PARTITION_SETTING_COMPLETED);

		/* store the partition info of emmc */
		mmc->part_support = ext_csd[EXT_CSD_PARTITIONING_SUPPORT];
		if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
		    ext_csd[EXT_CSD_BOOT_MULT])
			mmc->part_config = ext_csd[EXT_CSD_PART_CONF];
		if (part_completed &&
		    (ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & ENHNCD_SUPPORT))
			mmc->part_attr = ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE];

		mmc->capacity_boot = ext_csd[EXT_CSD_BOOT_MULT] << 17;

		mmc->capacity_rpmb = ext_csd[EXT_CSD_RPMB_MULT] << 17;

		for (i = 0; i < 4; i++) {
			int idx = EXT_CSD_GP_SIZE_MULT + i * 3;
			uint mult = (ext_csd[idx + 2] << 16) +
					(ext_csd[idx + 1] << 8) + ext_csd[idx];
			if (mult)
				has_parts = true;
			if (!part_completed)
				continue;
			mmc->capacity_gp[i] = mult;
			mmc->capacity_gp[i] *=
					ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
			mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
			mmc->capacity_gp[i] <<= 19;
		}

		if (part_completed) {
			mmc->enh_user_size =
				(ext_csd[EXT_CSD_ENH_SIZE_MULT+2] << 16) +
				(ext_csd[EXT_CSD_ENH_SIZE_MULT+1] << 8) +
				ext_csd[EXT_CSD_ENH_SIZE_MULT];
			mmc->enh_user_size *=
					ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
			mmc->enh_user_size *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
			mmc->enh_user_size <<= 19;
			mmc->enh_user_start =
				(ext_csd[EXT_CSD_ENH_START_ADDR+3] << 24) +
				(ext_csd[EXT_CSD_ENH_START_ADDR+2] << 16) +
				(ext_csd[EXT_CSD_ENH_START_ADDR+1] << 8) +
				ext_csd[EXT_CSD_ENH_START_ADDR];
			if (mmc->high_capacity)
				mmc->enh_user_start <<= 9;
		}

		/*
		 * Host needs to enable ERASE_GRP_DEF bit if device is
		 * partitioned. This bit will be lost every time after a reset
		 * or power off. This will affect erase size.
		 */
		if (part_completed)
			has_parts = true;
		if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) &&
		    (ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE] & PART_ENH_ATTRIB))
			has_parts = true;
		if (has_parts) {
			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
					 EXT_CSD_ERASE_GROUP_DEF, 1);

			if (err)
				return err;
			else
				ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;
		}

		if (ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01) {
			/* Read out group size from ext_csd */
			mmc->erase_grp_size =
				ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;
			/*
			 * if high capacity and partition setting completed
			 * SEC_COUNT is valid even if it is smaller than 2 GiB
			 * JEDEC Standard JESD84-B45, 6.2.4
			 */
			if (mmc->high_capacity && part_completed) {
				capacity = (ext_csd[EXT_CSD_SEC_CNT]) |
					   (ext_csd[EXT_CSD_SEC_CNT + 1] << 8) |
					   (ext_csd[EXT_CSD_SEC_CNT + 2] << 16) |
					   (ext_csd[EXT_CSD_SEC_CNT + 3] << 24);
				capacity *= MMC_MAX_BLOCK_LEN;
				mmc->capacity_user = capacity;
			}
		} else {
			/* Calculate the group size from the csd value. */
			int erase_gsz, erase_gmul;
			erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
			erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
			mmc->erase_grp_size = (erase_gsz + 1)
						* (erase_gmul + 1);
		}

		mmc->hc_wp_grp_size = 1024
					* ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
					* ext_csd[EXT_CSD_HC_WP_GRP_SIZE];

		mmc->wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];
	}

	err = mmc_set_capacity(mmc, bdesc->hwpart);
	if (err) {
		debug("%s: Error setting capacity\n", __func__);
		return err;
	}

	debug("%s: Changing frequency\n", __func__);
#ifdef CONFIG_CAVIUM_MMC_SD
	if (IS_SD(mmc))
		err = sd_change_freq(mmc);
#endif
	if (!IS_SD(mmc)) {
		mmc_set_ios(mmc);
		err = 0;
	}
	if (err) {
		printf("%s: Error changing frequency\n", __func__);
		return err;
	}

	/* Restrict card's capabilities by what the host can do. */
	debug("%s: MMC card caps: 0x%x, host caps: 0x%x\n",
	      __func__, mmc->card_caps, mmc->cfg->host_caps);
	mmc->card_caps &= mmc->cfg->host_caps;


	if (IS_SD(mmc)) {
#ifdef CONFIG_CAVIUM_MMC_SD
		err = sd_set_bus_width_speed(mmc);
		if (err) {
			printf("%s: Error setting SD bus width and/or speed\n",
			       __func__);
			return err;
		}
#endif
	} else {
		err = mmc_set_bus_width_speed(mmc);
		if (err) {
			printf("%s: Error setting MMC bus width and/or speed\n",
			       __func__);
			return err;
		}
	}
	if (!IS_SD(mmc) || (IS_SD(mmc) && mmc->high_capacity)) {
		err = mmc_set_blocklen(mmc, mmc->read_bl_len);
		if (err) {
			printf("%s: Error setting block length to %d\n",
			       __func__, mmc->read_bl_len);
			return err;
		}
	}

	/* Set watchdog for command timeout again */
	mmc_set_watchdog(mmc, 10000);

	/* Fill in device description */
	debug("%s: Filling in block descriptor\n",  __func__);
	bdesc->lun = 0;
	bdesc->hwpart = 0;
	bdesc->type = 0;
	bdesc->blksz = mmc->read_bl_len;
	bdesc->log2blksz = LOG2(bdesc->blksz);
	bdesc->lba = lldiv(mmc->capacity, mmc->read_bl_len);
	if (IS_SD(mmc)) {
		sprintf(bdesc->vendor, "Man %02x Snr %08x",
			mmc->cid[0] >> 24,
			(mmc->cid[2] << 8) | (mmc->cid[3] >> 24));
		sprintf(bdesc->product, "%c%c%c%c%c",
			mmc->cid[0] & 0xff,
			mmc->cid[1] >> 24, (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);
		sprintf(bdesc->revision, "%d.%d",
			(mmc->cid[2] >> 24) & 0xF, (mmc->cid[2] >> 28) & 0xF);
	} else {
		sprintf(bdesc->vendor, "Man %06x Snr %08x",
			mmc->cid[0] >> 24,
			(mmc->cid[2] << 16) | (mmc->cid[3] >> 16));
		sprintf(bdesc->product, "%c%c%c%c%c%c",
			mmc->cid[0] & 0xff,
			mmc->cid[1] >> 24, (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
			(mmc->cid[2] >> 24) & 0xff);
		sprintf(bdesc->revision, "%d.%d",
			(mmc->cid[2] >> 20) & 0xf, (mmc->cid[2] >> 16) & 0xf);
	}
	debug("%s: %s\n", __func__, bdesc->vendor);
	debug("%s: %s\n", __func__, bdesc->product);
	if (IS_SD(mmc))
		sprintf(bdesc->revision, "%d.%d",
			(mmc->cid[2] >> 28) & 0xf,
			(mmc->cid[2] >> 24) & 0xf);
	else
		sprintf(bdesc->revision, "%d.%d",
			(mmc->cid[2] >> 20) & 0xf,
		(mmc->cid[2] >> 16) & 0xf);
	debug("%s: mmc priv (slot): %p\n", __func__, mmc->priv);
	debug("%s:**** mmc priv (bdesc): %p bdesc->dev %p bdesc->devnum %d\n",
		 __func__, bdesc, bdesc->bdev, bdesc->devnum);
	part_init(bdesc);
	debug("%s: %s\n", __func__, bdesc->revision);

	return 0;
}

/**
 * This is the external mmc_send_cmd function.  It was required that
 * the internal version support flags so this version is required.
 */
static int cavium_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			       struct mmc_data *data)
{
	uint32_t flags = 0;
	int ret;
	static bool acmd;
	struct cavium_mmc_host *host = dev_get_priv(dev);
	struct cavium_mmc_slot *slot = &host->slots[host->cur_slotid];
	struct mmc *mmc = slot->mmc;

	/* Some SD commands require some flags to be changed */
	if (IS_SD(mmc)) {
		switch (cmd->cmdidx) {
		case SD_CMD_SEND_RELATIVE_ADDR:
			flags = MMC_CMD_FLAG_RTYPE_XOR(2);
			break;
		case SD_CMD_SEND_IF_COND:
			flags = MMC_CMD_FLAG_CTYPE_XOR(1) |
				MMC_CMD_FLAG_RTYPE_XOR(2);
			break;
		case 11:
			flags = MMC_CMD_FLAG_CTYPE_XOR(1);
			break;
		case 19:
			flags = MMC_CMD_FLAG_OFFSET(63);
			break;
		case 20:
			flags = MMC_CMD_FLAG_CTYPE_XOR(2);
			break;
		case SD_CMD_ERASE_WR_BLK_START:
		case SD_CMD_ERASE_WR_BLK_END:
			flags = MMC_CMD_FLAG_RTYPE_XOR(1);
			break;
		case SD_CMD_APP_SEND_OP_COND:
			if (acmd)
				flags = MMC_CMD_FLAG_RTYPE_XOR(1);
			break;
		case MMC_CMD_APP_CMD:
			acmd = true;
			break;
		case MMC_CMD_SPI_READ_OCR:
			flags = MMC_CMD_FLAG_RTYPE_XOR(3);
			break;
		case MMC_CMD_SPI_CRC_ON_OFF:
			flags = MMC_CMD_FLAG_RTYPE_XOR(1);
			break;
		default:
			break;
		}
	}
	ret = mmc_send_cmd_flags(mmc, cmd, data, flags);
	if (cmd->cmdidx != MMC_CMD_APP_CMD)
		acmd = false;
	return ret;
}

int __cavium_mmc_getwp(struct udevice *dev)
{
	struct cavium_mmc_host *host = dev_get_priv(dev);
	struct cavium_mmc_slot *slot = &host->slots[host->cur_slotid];
	struct mmc *mmc = slot->mmc;
	int val = 0;
	debug("%s: card \n", __func__);

	if (dm_gpio_is_valid(&slot->wp_gpio)) {
		val = dm_gpio_get_value(&slot->wp_gpio);
		debug("%s(%s): gpio %s pin %d returned %d\n", __func__,
		      mmc->cfg->name,
		      slot->wp_gpio.dev->name, gpio_get_number(&slot->wp_gpio),
		      val);
		val ^= slot->ro_inverted;
	} else {
		debug("%s(%s): No valid WP GPIO\n", __func__, mmc->cfg->name);
	}
	return val;
}

int __cavium_mmc_getcd(struct udevice *dev)
{
	struct cavium_mmc_host *host = dev_get_priv(dev);
	struct cavium_mmc_slot *slot = &host->slots[host->cur_slotid];
	struct mmc *mmc = slot->mmc;
	int bus = slot->bus_id;
	int val = 1;
	debug("%s(%d): card \n", __func__, bus );

	if (dm_gpio_is_valid(&slot->cd_gpio)) {
		val = dm_gpio_get_value(&slot->cd_gpio);
		val ^= slot->cd_inverted;
		debug("%s(%s): gpio %s pin %d returned %d\n", __func__,
		      mmc->cfg->name,
		      slot->cd_gpio.dev->name,
		      gpio_get_number(&slot->cd_gpio), val);
	} else {
		debug("%s(%s): No valid CD GPIO\n", __func__, mmc->cfg->name);
	}
	debug("%s(%d): card %sdetected\n", __func__, bus, val ? "" : "not ");
	return val;
}

/**
 * Controls the power to a MMC device
 *
 * @param mmc	pointer to mmc data structure
 * @param on	true to turn on power, false to turn off power
 */
void __mmc_set_power(struct mmc *mmc, int on)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	int bus = slot->bus_id;
	int val;

	slot->powered = !!on;
	debug("%s(%s, %d)\n", __func__, mmc->cfg->name, on);
	if (dm_gpio_is_valid(&slot->power_gpio)) {
		val = !!on;
		if (!slot->power_active_high)
			val = !val;
		dm_gpio_set_dir_flags(&slot->power_gpio, GPIOD_IS_OUT);
		dm_gpio_set_value(&slot->power_gpio, val);
		debug("%s(%d, %s) set GPIO %s pin %d to %d\n", __func__, bus,
		      on ? "on" : "off", slot->power_gpio.dev->name,
		      gpio_get_number(&slot->power_gpio), val);
		udelay(slot->power_delay);
	}
}
void mmc_set_power(struct mmc *mmc, int on)
	__attribute__((weak, alias("__mmc_set_power")));

#ifndef CONFIG_SUPPORT_EMMC_BOOT
/*
 * Modify EXT_CSD[162] which is RST_n_FUNCTION based on the given value
 * for enable.  Note that this is a write-once field for non-zero values.
 *
 * Returns 0 on success.
 */
int mmc_set_rst_n_function(struct mmc *mmc, u8 enable)
{
	return mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_RST_N_FUNCTION,
			  enable);
}
#endif

static int mmc_probe(bd_t *bis)
{
        int ret, i;
        struct uclass *uc;
        struct udevice *dev;

        ret = uclass_get(UCLASS_MMC, &uc);
        if (ret)
                return ret;

        /*
         * Try to add them in sequence order. Really with driver model we
         * should allow holes, but the current MMC list does not allow that.
         * So if we request 0, 1, 3 we will get 0, 1, 2.
         */
        for (i = 0; ; i++) {
                ret = uclass_get_device_by_seq(UCLASS_MMC, i, &dev);
                if (ret == -ENODEV)
                        break;
        }
        uclass_foreach_dev(dev, uc) {
                ret = device_probe(dev);
                if (ret)
                        printf("%s - probe failed: %d\n", dev->name, ret);
        }

        return 0;
}

int mmc_initialize(bd_t *bis)
{
        static int initialized = 0;
        int ret;
        if (initialized)        /* Avoid initializing mmc multiple times */
                return 0;
        initialized = 1;

#ifndef CONFIG_BLK
#if !CONFIG_IS_ENABLED(MMC_TINY)
        mmc_list_init();
#endif
#endif
        ret = mmc_probe(bis);
        if (ret)
                return ret;

#ifndef CONFIG_SPL_BUILD
        print_mmc_devices(',');
#endif

	return 0;
}

/**
 * Initialize all MMC devices on a board
 *
 * @param bis	pointer to board information structure
 *
 * @return 0 for success, error otherwise
 *
 * TODO: Modify this to support multiple nodes
 */
int cavium_mmc_initialize(struct udevice *dev)
{
	static bool not_first;
	struct cavium_mmc_host *host = dev_get_priv(dev);
	struct cavium_mmc_slot *slot = NULL;
	struct mmc *mmc = NULL;
	int bus_id = 0;
	int rc = -1;
	int repeat;
	int found = 0;
	int slot_index;

	debug("%s ENTER\n", __func__);

	/* The first time through clear out all of the last bus ids per node
	 * for switching between buses.
	 */
	if (!not_first) {
		not_first = true;
		host->last_slotid = -1;
	}

	debug("%s ENTER host %p\n", __func__,host);
	for (slot_index = 0; slot_index < CAVIUM_MAX_MMC_SLOT;
	     slot_index++) {
		slot = &host->slots[slot_index];
	debug("%s ENTER host %p\n", __func__,host);
	debug("%s ENTER slot %p\n", __func__,slot);
		if (slot->mmc && slot->mmc->has_init) {
			debug("%s: Slot %d initialized, clearing\n",
			      __func__, slot_index);
			if (slot->mmc) {
				mmc_destroy(slot->mmc);
			}
			memset(slot, 0, sizeof(*slot));
		}
	}

	debug("%s ENTER\n", __func__);
	rc = cavium_mmc_get_config(dev);
	if (rc) {
		debug("%s: Error getting configuration for host \n",
		      __func__);
		return -1;
	}

	for (slot_index = 0; slot_index < CAVIUM_MAX_MMC_SLOT;
	     slot_index++) {
		slot = &host->slots[slot_index];
		mmc = slot->mmc;
		if (!mmc)
			continue;
		/* Disable all MMC slots and power them down */
		debug("%s: Disabling MMC slot %s\n", __func__, mmc->cfg->name);
		mmc_write_csr(mmc, MIO_EMM_CFG, 0);
		mmc_set_power(mmc, 0);
	}

	mdelay(100);

	/* Power them all up */
	debug("Powering up all devices\n");
	for (slot_index = 0; slot_index < CAVIUM_MAX_MMC_SLOT;
	     slot_index++) {
		slot = &host->slots[slot_index];
		mmc = slot->mmc;
		if (!mmc)
			continue;
		debug("Powering up MMC slot %d\n", slot->bus_id);
		mmc_set_power(slot->mmc, 1);
	}
	found = false;

	for (slot_index = 0; slot_index < CAVIUM_MAX_MMC_SLOT;
	     slot_index++) {
		slot = &host->slots[slot_index];
		bus_id = slot->bus_id;
		mmc = slot->mmc;
		if (!mmc)
			continue;
		host->cur_slotid = slot_index;
		if (host->last_slotid == -1)
			host->last_slotid = host->cur_slotid;
		debug("%s: mmc: %p, host: %p, bus_id: %d\n",
		      __func__, mmc, host, bus_id);

		if (!mmc_getcd(mmc)) {
			debug("%s: Disabling empty slot %s\n",
			      __func__, mmc->cfg->name);
			/* Disable empty slots */
			mmc_disable(mmc);
			debug("%s: Setting %s has_init = 0\n",
			      __func__, mmc->cfg->name);
			mmc->has_init = 0;
			continue;
		}

		for (repeat = 0; repeat < 2; repeat++) {
			debug("%s: Calling mmc_init for %s, try %d\n",
			      __func__, mmc->cfg->name, repeat);
			rc = mmc_init(mmc);
			if (!rc) {
				found = true;
				debug("%s: %s: block dev: \n", __func__,
				      mmc->cfg->name);
				host->last_slotid = host->cur_slotid;
				debug("%s: Setting %s slot initialized true\n",
				      __func__, mmc->cfg->name);
				mmc->has_init = true;
				break;
			} else {
				debug("MMC device %d initialization failed, try %d\n",
				      bus_id, repeat);
				mmc_pre_idle(mmc);
				mmc_go_idle(mmc);
			}
		}
		if (rc) {
			debug("%s: Disabling %s\n", __func__,
			      mmc->cfg->name);
			mmc_disable(mmc);
			debug("%s: Setting %s has_init = 0\n", __func__,
			      mmc->cfg->name);
			mmc->has_init = 0;
		}
		debug("%s: %s: has_init = %d\n", __func__, mmc->cfg->name,
		      mmc->has_init);
	}
	init_time = 0;
	debug("%s: done initializing, found: %s, rc: %d\n", __func__,
	      found ? "true" : "false", rc);

	if (found) {
		debug("%s: Printing devices\n", __func__);
#ifdef DEBUG
		print_mmc_devices(',');
#endif
	} else {
		printf("MMC not available\n");
	}
	debug("%s: exit(%d)\n", __func__, rc);
	return found ? 0 : rc;
}

int __cavium_mmc_init(struct mmc *mmc)
{
	struct cavium_mmc_slot *slot = mmc->priv;
	struct cavium_mmc_host *host = slot->host;
	union mio_emm_switch emm_switch;
	union mio_emm_cfg emm_cfg;
	union mio_emm_sts_mask emm_sts_mask;
	union mio_emm_wdog emm_wdog;

	/* Clear interrupt status */
	mmc_write_csr(mmc, MIO_EMM_INT, mmc_read_csr(mmc, MIO_EMM_INT));

	/* Enable the bus */
	emm_cfg.u = mmc_read_csr(mmc, MIO_EMM_CFG);
	emm_cfg.u |= (1 << slot->bus_id);
	debug("%s: writing 0x%llx to mio_emm_cfg\n", __func__, emm_cfg.u);
	mmc_write_csr(mmc, MIO_EMM_CFG, emm_cfg.u);
	mdelay(2);

	/* Set clock period */
	slot->clk_period = (host->sclock + mmc->clock - 1) / mmc->clock;

	/* Default to RCA of 1 */
	mmc_write_csr(mmc, MIO_EMM_RCA, 1);


	/* Set the bus speed and width */
	emm_switch.u = 0;
	emm_switch.s.bus_width = EXT_CSD_BUS_WIDTH_1;
	emm_switch.s.power_class = 10;
	emm_switch.s.clk_hi = (slot->clk_period + 1) / 2;
	emm_switch.s.clk_lo = emm_switch.s.clk_hi;
	debug("%s: clock period: %u\n", __func__, slot->clk_period);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	emm_switch.s.bus_id = slot->bus_id;
	udelay(1200);
	mmc_write_csr(mmc, MIO_EMM_SWITCH, emm_switch.u);
	udelay(1200);

	host->last_slotid = host->cur_slotid;

	debug("%s: Set clock period to %d clocks, sclock: %llu\n", __func__,
	      emm_switch.s.clk_hi + emm_switch.s.clk_lo, host->sclock);

	/* Set watchdog for command timeout */
	if (slot->bus_id == 0)
		emm_wdog.u = 0;
	else
		emm_wdog.u = mmc_read_csr(mmc, MIO_EMM_WDOG);
	emm_wdog.s.clk_cnt = mmc->clock;
	debug("Setting command timeout value to %u\n", emm_wdog.s.clk_cnt);
	mmc_write_csr(mmc, MIO_EMM_WDOG, emm_wdog.u);

	mdelay(10);	/* Wait 10ms */

	/* Set status mask */
	emm_sts_mask.u = 0;
	emm_sts_mask.s.sts_msk = 1 << 7 | 1 << 22 | 1 << 23 | 1 << 19;
	mmc_write_csr(mmc, MIO_EMM_STS_MASK, emm_sts_mask.u);

	mmc_set_power(mmc, 1);

	mmc_enable(mmc);

	return mmc_pre_idle(mmc);
}

int mmc_start_init(struct mmc *mmc)
{
	int err;
	const struct cavium_mmc_slot *slot = mmc->priv;
	struct blk_desc *bdesc = mmc_get_blk_desc(mmc, slot->bus_id);

	debug("%s(%s): Entry\n", __func__, mmc->cfg->name);

	if (!bdesc) {
		printf("%s couldn't find blk desc\n", __func__);
		return -ENODEV;
	}
	if (!mmc_getcd(mmc)) {
		debug("%s: No card detected for %s\n", __func__,
		      mmc->cfg->name);
		return -ENODEV;
	}

	mmc_set_power(mmc, 1);
        err = cavium_mmc_init(mmc);
        if (err) {
                printf("%s(%s): init returned %d\n", __func__,
                       mmc->cfg->name, err);
                return err;
        }

	mmc->ddr_mode = 0;
	mmc_set_bus_width(mmc, 1);
	mmc_set_clock(mmc, 1);

	err = mmc_go_idle(mmc);
	if (err) {
		printf("%s(%s): go_idle failed\n", __func__, mmc->cfg->name);
		return err;
	}

	bdesc->hwpart = 0;

#ifdef CONFIG_CAVIUM_MMC_SD
	/* Test for SD version 2 */
	err = mmc_send_if_cond(mmc);

	/* Now try to get the SD card's operating condition */
	err = sd_send_op_cond(mmc);
#else
	err = ETIMEDOUT;
#endif
	/* If the command timed out, we check for an MMC card */
	if (err == ETIMEDOUT) {
		err = mmc_send_op_cond(mmc);

		if (err) {
			debug("Card did not respond to voltage select!\n");

			return ENOTRECOVERABLE;
		}
	}

	if (!err)
		mmc->init_in_progress = 1;


	debug("%s: return %d\n", __func__, err);
	return err;
}

static int mmc_complete_init(struct mmc *mmc)
{
	int err = 0;

	debug("%s(%s)\n", __func__, mmc->cfg->name);
	mmc->init_in_progress = 0;

	err = mmc_startup(mmc);

	if (err)
		mmc->has_init = 0;
	else
		mmc->has_init = 1;
	debug("%s(%s): return %d, has_init: %d\n", __func__, mmc->cfg->name,
	      err, mmc->has_init);
	return err;
}

int mmc_init(struct mmc *mmc)
{
	int err = 0;
	ulong start;

	debug("%s(%s)\n", __func__, mmc->cfg->name);

	if (mmc->has_init) {
		debug("%s: Already initialized\n", __func__);
		return 0;
	}

	start = get_timer(0);

	if (!mmc->init_in_progress)
		err = mmc_start_init(mmc);

	if (!err)
		err = mmc_complete_init(mmc);

	debug("%s: %d, time %lu\n", __func__, err, get_timer(start));
	return err;
}

/**
 * Create a new MMC device and links it in
 *
 * @param[in] cfg	pointer to configuration data structure
 * @param[in] priv	pointer to private data
 *
 * @return	pointer to new mmc data structure or NULL if error
 */
struct mmc *mmc_create(const struct mmc_config *cfg, void *priv)
{
	struct mmc *mmc;
	struct blk_desc *bdesc;
	struct cavium_mmc_slot *slot = priv;
	struct udevice *dev = slot->host->dev;
	struct udevice *bdev;
	int ret = -1;

	debug("%s(%p, %p)\n", __func__, cfg, priv);
	if (cfg == NULL || cfg->f_min == 0 || cfg->f_max == 0 ||
		cfg->b_max == 0) {
		printf("%s: config error:\n"
		       "  cfg: %p\n"
		       "  cfg->f_min: %d\n"
		       "  cfg->f_max: %d\n"
		       "  cfg->b_max: %d\n",
		       __func__, cfg, cfg->f_min, cfg->f_max,
			 cfg->b_max);
		return NULL;
	}

	if (!mmc_get_ops(dev))
		return NULL;

	ret = blk_create_devicef(dev, "mmc_blk", "blk", IF_TYPE_MMC,
				slot->bus_id, 512, 0, &bdev);
	if (ret) {
		debug("Cannot create block device\n");
		return NULL;
	}
	bdesc = dev_get_uclass_platdata(bdev);

	mmc = calloc(1, sizeof(*mmc));
	if (mmc == NULL)
		return NULL;
	mmc->cfg = cfg;
	mmc->priv = priv;
	mmc->dev = dev;
	debug("%s**** bdesc %p bdev %p dev %p\n",__func__,
		bdesc, bdev, dev);
	/* the following chunk was mmc_register */

	/* Setup dsr related values */
	mmc->dsr_imp = 0;
	mmc->dsr = 0xffffffff;
	/* Setup the universal parts of the block interface just once */
	bdesc->if_type = IF_TYPE_MMC;
	bdesc->removable = 1;

	/* setup initial part type */
	bdesc->part_type = mmc->cfg->part_type;

	return mmc;
}

void mmc_destroy(struct mmc *mmc)
{
	/* only freeing memory for now */
	debug("%s(%p) ENTRY\n", __func__, mmc);
	free(mmc);
}

/**
 * Translates a voltage number to bits in MMC register
 *
 * @param	voltage	voltage in microvolts
 *
 * @return	MMC register value for voltage
 */
static uint32_t xlate_voltage(uint32_t voltage)
{
	uint32_t volt = 0;

	/* Convert to millivolts */
	voltage /= 1000;
	if (voltage >= 1650 && voltage <= 1950)
		volt |= MMC_VDD_165_195;
	if (voltage >= 2000 && voltage <= 2100)
		volt |= MMC_VDD_20_21;
	if (voltage >= 2100 && voltage <= 2200)
		volt |= MMC_VDD_21_22;
	if (voltage >= 2200 && voltage <= 2300)
		volt |= MMC_VDD_22_23;
	if (voltage >= 2300 && voltage <= 2400)
		volt |= MMC_VDD_23_24;
	if (voltage >= 2400 && voltage <= 2500)
		volt |= MMC_VDD_24_25;
	if (voltage >= 2500 && voltage <= 2600)
		volt |= MMC_VDD_25_26;
	if (voltage >= 2600 && voltage <= 2700)
		volt |= MMC_VDD_26_27;
	if (voltage >= 2700 && voltage <= 2800)
		volt |= MMC_VDD_27_28;
	if (voltage >= 2800 && voltage <= 2900)
		volt |= MMC_VDD_28_29;
	if (voltage >= 2900 && voltage <= 3000)
		volt |= MMC_VDD_29_30;
	if (voltage >= 3000 && voltage <= 3100)
		volt |= MMC_VDD_30_31;
	if (voltage >= 3100 && voltage <= 3200)
		volt |= MMC_VDD_31_32;
	if (voltage >= 3200 && voltage <= 3300)
		volt |= MMC_VDD_32_33;
	if (voltage >= 3300 && voltage <= 3400)
		volt |= MMC_VDD_33_34;
	if (voltage >= 3400 && voltage <= 3500)
		volt |= MMC_VDD_34_35;
	if (voltage >= 3500 && voltage <= 3600)
		volt |= MMC_VDD_35_36;

	return volt;
}

/**
 * Parses the power control for a MMC slot or controller
 *
 * @param[in]		blob		pointer to flat device tree
 * @param		of_offset	regulator node offset
 * @param[in,out]	host		pointer to controller info
 * @param[in,out]	slot		MMC slot for regulator (can be NULL)
 *
 * @return	0 for success, -1 on error or if invalid
 */
static int get_mmc_regulator(const void *blob, int of_offset,
			     struct cavium_mmc_host *host,
			     struct cavium_mmc_slot *slot)
{
	uint32_t min_microvolt;
	uint32_t max_microvolt;
	uint32_t power_delay;
	uint32_t voltages;
	uint32_t low, high;
	int ret;
	bool active_high;

	if (fdt_node_check_compatible(blob, of_offset, "regulator-fixed")) {
		printf("Unknown mmc regulator type, only \"regulator-fixed\" is supported\n");
		return -1;
	}

	min_microvolt = fdtdec_get_int(blob, of_offset,
				       "regulator-min-microvolt", 3300000);
	max_microvolt = fdtdec_get_int(blob, of_offset,
				       "regulator-max-microvolt", 3300000);
	if (min_microvolt > max_microvolt) {
		printf("Invalid minimum voltage %u and maximum voltage %u, min > max\n",
		       min_microvolt, max_microvolt);
		return -1;
	}
	debug("%s: min voltage: %u microvolts, max voltage: %u microvolts\n",
	      __func__, min_microvolt, max_microvolt);
	low = xlate_voltage(min_microvolt);
	high = xlate_voltage(max_microvolt);

	ret = gpio_request_by_name_nodev(offset_to_ofnode(of_offset), "gpio", 0,
					 &slot->power_gpio,
					 GPIOD_IS_OUT);
	/* GPIOs can only be acquired once so if we get an EBUSY error it means
	 * it was likely claimed by another slot if it's shared.  In this case
	 * we just duplicate the GPIO descriptor.
	 */
	if (ret == -EBUSY) {
		struct cavium_mmc_slot *sslot;
		for(int slot_index = 0; slot_index < CAVIUM_MAX_MMC_SLOT;
			slot_index++) {
			sslot = &host->slots[slot_index];
			assert(sslot);
			if (sslot->power_gpio_of_offset == of_offset) {
				debug("%s: Found duplicate link to power\n",
				      __func__);
				memcpy(&slot->power_gpio, &sslot->power_gpio,
				       sizeof(slot->power_gpio));
				slot->power_gpio_of_offset = of_offset;
				break;
			}
		}
		ret = 0;
	}
	if (ret) {
		debug("%s: Error %d: Invalid power GPIO control in fixed supply\n",
		      __func__, ret);
		return -1;
	}
	power_delay = fdtdec_get_int(blob, of_offset, "startup-delay-usec",
				     10000);
	active_high = fdtdec_get_bool(blob, of_offset, "enable-active-high");

	voltages = 0;
	do {
		voltages |= low;
		low <<= 1;
	} while (low <= high);

	slot->power_gpio_of_offset = of_offset;
	slot->power_delay = power_delay;
	slot->power_active_high = active_high;
	slot->cfg.voltages = voltages;
	debug("%s: power delay: %uus, active %s\n", __func__,
	      slot->power_delay,
	      slot->power_active_high ? "high" : "low");
	return 0;
}

/**
 * Gets the configuration of a MMC interface and all of the slots and adds each
 * slot to the list of devices.
 *
 * @param[in]	blob		pointer to flat device tree blob
 * @param	of_offset	offset of MMC device in the device tree
 * @param[out]	host		interface configuration
 *
 * @return	0 for success, -1 on error.
 */
static int cavium_mmc_get_config(struct udevice *dev)
{
	struct cavium_mmc_host *host = dev_get_priv(dev);
	struct cavium_mmc_slot *slot;
	const void *blob = gd->fdt_blob;
	int slot_node;
	int regulator_node;
	int ret;

	debug("%s(%p, %d, %p)\n", __func__, blob, host->of_offset, host);

	host->sclock = octeontx_get_io_clock();
	debug("%s: sclock: %llu\n", __func__, host->sclock);

	slot_node = host->of_offset;
	debug("%s: Reading slots...\n", __func__);
	while ((slot_node = fdt_node_offset_by_compatible(blob, slot_node,
					"mmc-slot")) > 0) {
		int reg = fdtdec_get_int(blob, slot_node, "reg", -1);
		uint voltages[2];
		uint low, high;
		if (reg < 0) {
			printf("Missing reg field for mmc slot in device tree\n");
			return -1;
		}
		if (reg >= CAVIUM_MAX_MMC_SLOT) {
			printf("MMC slot %d is out of range\n", reg);
			return -1;
		}
		slot = &host->slots[reg];
		/* Set back pointer */
		slot->host = host;
		/* Get max frequency */
		slot->cfg.f_max = fdtdec_get_uint(blob, slot_node,
						  "max-frequency",
						  26000000);
		/* Set min frequency */
		slot->cfg.f_min = 400000;
		/* Set maximum number of blocks that can be transferred
		 * in one operation.
		 */
		slot->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

		if (fdtdec_get_bool(blob, slot_node, "cap-sd-highspeed"))
			slot->cfg.host_caps |= MMC_MODE_HS;
		if (fdtdec_get_bool(blob, slot_node, "cap-mmc-highspeed"))
			slot->cfg.host_caps |= MMC_MODE_HS;
		if (fdtdec_get_bool(blob, slot_node, "sd-uhs-sdr25"))
			slot->cfg.host_caps |= MMC_MODE_HS;
		if (fdtdec_get_bool(blob, slot_node, "sd-uhs-sdr50"))
			slot->cfg.host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
		if (fdtdec_get_bool(blob, slot_node, "sd-uhs-ddr50"))
			slot->cfg.host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz |
				MMC_MODE_DDR_52MHz;


		/* Get voltage range */
		if (fdtdec_get_int_array(blob, slot_node,
					 "voltage-ranges", voltages, 2)) {
			slot->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
		} else {
			low = xlate_voltage(voltages[0]);
			high = xlate_voltage(voltages[1]);
			if (low > high || !low || !high) {
				printf("Invalid MMC voltage range [%u-%u] specified\n",
				       low, high);
				return -1;
			}
			slot->cfg.voltages = 0;
			do {
				slot->cfg.voltages |= low;
				low <<= 1;
			} while (low <= high);
			debug("%s: slot %d voltages: 0x%x [%u..%u] [0x%x..0x%x]\n",
			      __func__, reg, slot->cfg.voltages,
			      voltages[0], voltages[1],
			      xlate_voltage(voltages[0]), high);
		}
		/* Get maximum bus width */
		slot->bus_max_width = fdtdec_get_int(blob, slot_node,
						     "cavium,bus-max-width", 8);

		regulator_node = fdtdec_lookup_phandle(blob, slot_node,
						       "vmmc-supply");
		if (regulator_node > 0) {
			ret = get_mmc_regulator(blob, regulator_node,
						host, slot);
			if (ret)
				debug("Error getting mmc regulator\n");

			debug("%s: power gpio number: slot: %d\n",
			      __func__, gpio_get_number(&slot->power_gpio));
		}

		debug("  slot GPIO is%s valid\n",
		      dm_gpio_is_valid(&slot->power_gpio) ? "" : " not");

		gpio_request_by_name_nodev(offset_to_ofnode(slot_node),
					   "cd-gpios", 0,
					   &slot->cd_gpio, GPIOD_IS_IN);
		slot->cd_inverted = fdtdec_get_bool(blob, slot_node,
						    "cd-inverted");
		gpio_request_by_name_nodev(offset_to_ofnode(slot_node),
					   "wp-gpios", 0,
					   &slot->wp_gpio, GPIOD_IS_IN);
		slot->ro_inverted = fdtdec_get_bool(blob, slot_node,
						    "wp-inverted");
		/* Disable 1.8v here */
		if (fdtdec_get_bool(blob, slot_node, "no-1-8-v"))
			slot->cfg.voltages &= ~MMC_VDD_165_195;
		slot->non_removable = fdtdec_get_bool(blob, slot_node,
						      "non-removable");
		slot->of_offset = slot_node;

		slot->cmd_clk_skew = fdtdec_get_int(blob, slot_node,
						    "cavium,cmd-clk-skew", 0);
		slot->dat_clk_skew = fdtdec_get_int(blob, slot_node,
						    "cavium,dat-clk-skew", 0);
		slot->bus_id = reg;

		/* Initialize mmc data structure */
		slot->mmc = mmc_create(&slot->cfg, slot);
		if (!slot->mmc) {
			printf("Error: could not allocate mmc data structure\n");
			return -1;
		}

		slot->mmc->version = MMC_VERSION_UNKNOWN;
		slot->mmc->rca = reg + 0x10;
		slot->mmc->clock = CONFIG_CAVIUM_MMC_MIN_BUS_SPEED_HZ;
		slot->bus_width = EXT_CSD_BUS_WIDTH_1;
		slot->mmc->bus_width = fdtdec_get_int(blob, slot_node,
						      "bus-width", 8);
		if (slot->mmc->bus_width >= 4)
			slot->cfg.host_caps |= MMC_MODE_4BIT;
		if (slot->mmc->bus_width == 8)
			slot->cfg.host_caps |= MMC_MODE_8BIT;

		snprintf(slot->name, CAVIUM_MMC_NAME_LEN, "cavium_mmc%d",
			 slot->bus_id);
		slot->cfg.name = slot->name;
	}
	return 0;
}

#ifdef CONFIG_CMD_BKOPS_ENABLE
int mmc_set_bkops_enable(struct mmc *mmc)
{
	int err;
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);

	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err) {
		puts("Could not get ext_csd register values\n");
		return err;
	}

	if (!(ext_csd[EXT_CSD_BKOPS_SUPPORT] & 0x1)) {
		puts("Background operations not supported on device\n");
		return -EMEDIUMTYPE;
	}

	if (ext_csd[EXT_CSD_BKOPS_EN] & 0x1) {
		puts("Background operations already enabled\n");
		return 0;
	}

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BKOPS_EN, 1);
	if (err) {
		puts("Failed to enable manual background operations\n");
		return err;
	}

	puts("Enabled manual background operations\n");

	return 0;
}
#endif

static int cavium_pci_mmc_probe(struct udevice *dev)
{
	int rc = -1;
	size_t size;
	pci_dev_t bdf = dm_pci_get_bdf(dev);
	struct cavium_mmc_host *host = dev_get_priv(dev);

	debug("%s: Entry\n", __func__);
	memset(host, 0, sizeof(*host));
	host->base_addr = dm_pci_map_bar(dev, 0, &size, PCI_REGION_MEM);
	if (!host->base_addr) {
		debug("%s(%s): MMC not found on MRML bus\n",
		      __func__, dev->name);
		return rc;
	}
	host->dev = dev;
	host->of_offset = dev->node.of_offset;
	dev->req_seq = PCI_FUNC(bdf);

	debug("%s(%s): ", __func__, dev->name);
	debug("  dev:             %p\n"
	      "  driver:          %p\n"
	      "  platdata:        %p\n"
	      "  parent platdata: %p\n"
	      "  uclass platdata: %p\n"
	      "  base address:    %p\n"
	      "  of_offset:       %ld\n"
	      "  parent:          %p\n"
	      "  priv:            %p\n"
	      "  uclass:          %p\n"
	      "  req_seq:         %d\n"
	      "  seq:             %d\n",
	      dev, dev->driver, dev->platdata, dev->parent_platdata,
	      dev->uclass_platdata, host->base_addr,
	      dev->node.of_offset, dev->parent, dev->priv,
	      dev->uclass, dev->req_seq, dev->seq);

	rc = cavium_mmc_initialize(dev);

	return rc;
}

static int cavium_mmc_ofdata_to_platdata(struct udevice *dev)
{
	return 0;
}

#ifdef DEBUG
int do_oct_mmc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	mmc_initialize(gd->bd);
	debug("%s: returning\n", __func__);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(octmmc, 2, 1, do_oct_mmc, "Octeon MMC initialization", NULL);
#endif

U_BOOT_DRIVER(cavium_pci_mmc) = {
	.name	= CAVIUM_MMC_DRIVER_NAME,
	.id	= UCLASS_MMC,
	.of_match = of_match_ptr(cavium_mmc_ids),
	.ofdata_to_platdata = cavium_mmc_ofdata_to_platdata,
	.probe	= cavium_pci_mmc_probe,
	.priv_auto_alloc_size = sizeof(struct cavium_mmc_host),
	.ops = &cavium_mmc_ops,
};

