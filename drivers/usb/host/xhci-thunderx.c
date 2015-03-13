/**
 * (C) Copyright 2014, Cavium Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
**/


#include <common.h>
#include <usb.h>
#include <asm-generic/errno.h>
#include <linux/compat.h>

#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct thunderx_xhci {
	struct xhci_hccr *hcd;
};

#define USBHX_PF_BAR0(x) (0x868000000000ull + (x) * (1ull << 36))

static struct thunderx_xhci thunderx[CONFIG_USB_MAX_CONTROLLER_COUNT];


int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct thunderx_xhci *ctx = &thunderx[index];

	ctx->hcd = (struct xhci_hccr *)USBHX_PF_BAR0(index);

	*hccr = (ctx->hcd);
	*hcor = (struct xhci_hcor *)
			((uintptr_t) *hccr +
			 HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("ThunderX-xhci: init hccr %p and hcor %p hc_length %d\n",
		*hccr, *hcor,
		(uint32_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

void xhci_hcd_stop(int index)
{
}
