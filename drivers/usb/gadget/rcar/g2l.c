// SPDX-License-Identifier: GPL-2.0
/*
 * Renesas USB driver RZ/A2 initialization and power control
 *
 * Copyright (C) 2019 Chris Brandt
 * Copyright (C) 2019 Renesas Electronics Corporation
 */

#include <linux/delay.h>
#include <linux/io.h>
#include "common.h"
#include "rza.h"

static int usbhs_g2l_power_ctrl(struct platform_device *pdev,
				void __iomem *base, int enable)
{
	struct usbhs_priv *priv = usbhs_pdev_to_priv(pdev);
	int retval = 0;
#if !defined(CONFIG_R9A09G057) || !defined(CONFIG_R9A09G056)
	if (!priv->phy)
		return -ENODEV;
#endif

	if (enable) {
		//iretval = phy_init(priv->phy);
		usbhs_bset(priv, SUSPMODE, SUSPM, SUSPM);
		udelay(100);    /* Wait for PLL to become stable */
	} else {
		usbhs_bset(priv, SUSPMODE, SUSPM, 0);
		//phy_power_off(priv->phy);
		//phy_exit(priv->phy);
	}

	return retval;
}

static int usbhs_g2l_get_id(struct platform_device *pdev)
{
	return USBHS_GADGET;
}

const struct renesas_usbhs_platform_callback usbhs_g2l_ops = {
	.power_ctrl = usbhs_g2l_power_ctrl,
	.get_id = usbhs_g2l_get_id,
};
