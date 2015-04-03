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
#include <cavium/atf.h>
#include <cavium/thunderx.h>

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct thunderx_xhci {
	struct xhci_hccr *hcd;
};

#define USBHX_PF_BAR0(x)	(0x868000000000ull + (x) * (1ull << 36))
#define USBHX_UCTL_CTL(x)	(0x868000100000ull + (x) * (1ull << 36))

#define UCTL_CTL_CSCLK_EN	(1 << 4)
#define UCTL_CTL_HCLK_EN	(1 << 30)

#define UCTL_CTL_CLK_EN		(UCTL_CTL_CSCLK_EN | UCTL_CTL_HCLK_EN)


static struct thunderx_xhci thunderx[CONFIG_USB_MAX_CONTROLLER_COUNT];

#define XHCI_PER_NODE 2

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct thunderx_xhci *ctx = &thunderx[index];
	int node = index / XHCI_PER_NODE;
	u64 uctl_ctl;

	index %= XHCI_PER_NODE;

	if (node >= atf_node_count()) {
		*hccr = NULL;
		*hcor = NULL;
		return -ENODEV;
	}

	uctl_ctl = readq(CSR_PA(node, USBHX_UCTL_CTL(index)));

	debug("ThunderX-xhci %d at node %d uctl_ctl: %lx\n", index, node,
	      (unsigned long)uctl_ctl);

	if ((uctl_ctl & UCTL_CTL_CLK_EN) != UCTL_CTL_CLK_EN)
		return -ENODEV;

	ctx->hcd = (struct xhci_hccr *)CSR_PA(node, USBHX_PF_BAR0(index));

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
