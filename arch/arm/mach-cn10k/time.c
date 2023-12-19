// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <time.h>

extern unsigned long timer_read_counter(void);
#define USEC_PER_SEC	1000000UL
#define MHZ(X)		((X) * (USEC_PER_SEC))

unsigned long timer_get_us(void)
{
	static ulong div;

	if (!div) {
		ulong clk = get_tbclk();

		assert(clk >= MHZ(1)); /* Does not support clock lower than 1MHz */
		div = clk / USEC_PER_SEC;
	}
	return timer_read_counter() / div;
}
