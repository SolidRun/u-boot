/**
 * (C) Copyright 2016, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 **/

#ifndef __THUNDERX_CLOCK_H__

/**
 * Returns the I/O clock speed in Hz
 */
u64 thunderx_get_io_clock(void);

/**
 * Returns the core clock speed in Hz
 */
u64 thunderx_get_core_clock(void);

#endif /* __THUNDERX_CLOCK_H__ */
