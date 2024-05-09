/*
 * Copyright (C) 2021 Marvell.
 * https://spdx.org/licenses
 * SPDX-License-Identifier:    GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <serial.h>
#include <linux/compiler.h>

#ifndef CONFIG_DM_SERIAL
#include <debug_uart.h>
#endif

#define FW_LOGMEM_BASE		(60 * 1024 * 1024)
#define FW_LOGMEM_SIZE		(4 * 1024 * 1024)

struct fw_logbuf_header {
    uint64_t fwlog_base;
    uint64_t fwlog_end;
    uint64_t fwlog_ptr;
    uint64_t wraparound;
};

#ifndef CONFIG_DM_SERIAL
static void _debug_uart_init(void)
{

}

static inline void _debug_uart_putc(int ch)
{

}

DEBUG_UART_FUNCS
#endif

void fwlog_disconnect_ttymem(void)
{
	flush_dcache_range(FW_LOGMEM_BASE, (FW_LOGMEM_BASE + FW_LOGMEM_SIZE));
	return;
}

#ifndef CONFIG_DM_SERIAL
static int fwlog_ttymem_init(void)
{
	return 0;
}

void fwlog_ttymem_putc(const char c)
{
	struct fw_logbuf_header *logbuf = (struct fw_logbuf_header *) FW_LOGMEM_BASE;

	*(char *)logbuf->fwlog_ptr++ = c;
	if (logbuf->fwlog_ptr == logbuf->fwlog_end) {
		logbuf->fwlog_ptr = logbuf->fwlog_base;
		logbuf->wraparound = (uint64_t) 1;
	}
}

struct serial_device fwlog_ttymem_device = {
	.name = "ttymem",
	.start = fwlog_ttymem_init,
	.stop = NULL,
	.setbrg = NULL,
	.getc = NULL,
	.tstc = NULL,
	.putc = fwlog_ttymem_putc,
	.puts = default_serial_puts,
};

void fwlog_serial_initialize(void)
{
	serial_register(&fwlog_ttymem_device);
}
__weak struct serial_device *default_serial_console(void)
{
	return &fwlog_ttymem_device;
}
#endif

#ifdef CONFIG_DM_SERIAL

int fwlog_ttymem_putc(struct udevice *dev, const char c)
{
	struct fw_logbuf_header *logbuf = (struct fw_logbuf_header *) FW_LOGMEM_BASE;

	*(char *)logbuf->fwlog_ptr++ = c;
	if (logbuf->fwlog_ptr == logbuf->fwlog_end) {
		logbuf->fwlog_ptr = logbuf->fwlog_base;
		logbuf->wraparound = (uint64_t) 1;
	}
	return 0;
}

int fwlog_ttymem_probe(struct udevice *dev)
{
	return 0;
}
static const struct dm_serial_ops fwlog_ttymem_ops = {
	.putc = fwlog_ttymem_putc,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id fwlog_ttymem_id[] = {
	{.compatible = "fwlog,ttymem", .data = 0},
	{}
};
#endif

U_BOOT_DRIVER(serial_ttymem) = {
	.name	= "serial_fwlog_ttymem",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(fwlog_ttymem_id),
	.probe = fwlog_ttymem_probe,
	.ops = &fwlog_ttymem_ops,
};

#endif
