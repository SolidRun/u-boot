/*
 * (C) Copyright 2014-2016, Cavium Networks, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Extracted from armv8/start.S
 */

#include <config.h>
#include <linux/linkage.h>
#include <cavium/thunderx_svc.h>

#include <asm/macro.h>

ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

	ldr	x0, =THUNDERX_FDT_GET       /* Copy the FDT to end of u-boot binary*/
	adr	x1, _end
	ldr	x2, =CONFIG_BDK_FDT_SIZE
	smc	#0

	ldr	x0, =THUNDERX_DRAM_SIZE
	mov	x1, xzr
	smc	#0			/* Get the DRAM size */

	mov	x23, #2			/* Select L2 */
	msr	csselr_el1, x23
	mrs	x23, ccsidr_el1
	ubfx	x20, x23, #13, #15	/* x20 = Sets - 1 */
	ubfx	x21, x23, #3, #10	/* x21 = Assoc(Ways) - 1 */

	ubfx	x22, x23, #0, #2
	add	x22, x22, #4
	mov	x23, #1
	lsl	x22, x23, x22		/* x22 = Line Size */

	add	x20, x20, #1		/* x20 = Sets */
	add	x21, x21, #1		/* x21 = Ways */

	mul	x20, x20, x21		/* x20 = Sets * Ways */
	mul	x20, x20, x22		/* x20 = L2size */

	sub	x21, x0, x20		/* x21 = last address - L2size */

	mrs	x23, dczid_el0
	ubfx	x1, x23, #4, #1		/* x1 = DZP bit */
	cbnz	x1, 3f			/* If dc zva is prohibited, do nothing */
	and	x23, x23, #15
	add	x23, x23, #2
	mov	x24, #1
	lsl	x23, x24, x23		/* ZVA block size */

1:
	dc	zva, x21		/* Zero the end of DRAM block by block */
	add	x21, x21, x23
	cmp	x21, x0
	b.lt	1b

	sub	x21, x0, x20		/* x21 = last address - L2size */
2:
	sys	#0,c11,C1,#2, x21	/* Flush and invalidate L2 line by line */
	add	x21, x21, x22
	cmp	x21, x0
	b.lt	2b

3:
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)

