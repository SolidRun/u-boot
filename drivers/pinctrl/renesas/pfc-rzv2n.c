// SPDX-License-Identifier: GPL-2.0+
/*
 * Pin controller driver for Renesas RZ/V2N SoCs.
 *
 * Copyright (C) 2023 Renesas Electronics Corporation
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm/device_compat.h>

#define P(n)	(0x0000 + 0x20 + (n))	  /* Port Register */
#define PM(n)	(0x0100 + 0x40 + (n) * 2) /* Port Mode Register */
#define PMC(n)	(0x0200 + 0x20 + (n))	  /* Port Mode Control Register */
#define PFC(n)	(0x0400 + 0x80 + (n) * 4) /* Port Function Control Register */
#define PIN(n)	(0x0800 + 0x10 + (n))	  /* Port Input Register */
#define PWPR	(0x3C04)		  /* Port Write Protection Register */

#define PWPR_REGWE_A	BIT(6)		  /* Bit Write Disable */
#define PWPR_REGWE_B	BIT(5)		  /* PFC Register Write Enable */

#define RZV2N_MAX_PINS_PER_PORT		8

DECLARE_GLOBAL_DATA_PTR;

struct rzv2n_pinctrl_priv {
	void __iomem	*regs;
};

static void rzv2n_pinctrl_set_function(struct rzv2n_pinctrl_priv *priv,
				       u16 port, u8 pin, u8 func)
{
	u32 reg32;
	u8 reg8;

	/* Set GPIO or Func in PMC, then set Func in PFC */
	reg8 = readb(priv->regs + PMC(port));
	reg8 = (reg8 & ~(1 << pin)) | BIT(pin);
	writeb(reg8, priv->regs + PMC(port));

	reg32 = readl(priv->regs + PFC(port));
	reg32 = (reg32 & ~(0x0F << (pin * 4))) | (func << (pin * 4));
	writel(reg32, priv->regs + PFC(port));
}

static int rzv2n_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct rzv2n_pinctrl_priv *priv = dev_get_plat(dev);
	u16 port;
	u16 port_max = (u16)dev_get_driver_data(dev);
	u8 pin, func;
	int i, count;
	const u32 *data;
	u32 cells[port_max * RZV2N_MAX_PINS_PER_PORT];

	data = dev_read_prop(config, "pinmux", &count);
	if (count < 0) {
		debug("%s: bad array size %d\n", __func__, count);
		return -EINVAL;
	}

	count /= sizeof(u32);
	if (count > port_max * RZV2N_MAX_PINS_PER_PORT) {
		debug("%s: unsupported pins array count %d\n",
		      __func__, count);
		return -EINVAL;
	}
	writel(PWPR_REGWE_A, priv->regs + PWPR);

	for (i = 0 ; i < count; i++) {
		cells[i] = fdt32_to_cpu(data[i]);
		func = (cells[i] >> 12) & 0xf;
		port = (cells[i] / RZV2N_MAX_PINS_PER_PORT) & 0x1ff;
		pin = cells[i] % RZV2N_MAX_PINS_PER_PORT;
		if (func > 15 || port >= port_max || pin >= RZV2N_MAX_PINS_PER_PORT) {
			printf("Invalid cell %i in node %s!\n",
			       count, ofnode_get_name(dev_ofnode(config)));
			continue;
		}

		rzv2n_pinctrl_set_function(priv, port, pin, func);
	}

	writel(0, priv->regs + PWPR);

	return 0;
}

const struct pinctrl_ops rzv2n_pinctrl_ops  = {
	.set_state = rzv2n_pinctrl_set_state,
};

static int rzv2n_pinctrl_probe(struct udevice *dev)
{
	struct rzv2n_pinctrl_priv *priv = dev_get_plat(dev);
	ofnode node;

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs) {
		dev_err(dev, "can't get address\n");
		return -EINVAL;
	}

	dev_for_each_subnode(node, dev) {
		struct udevice *gpiodev;

		if (!ofnode_read_bool(node, "gpio-controller"))
			continue;

		device_bind_driver_to_node(dev, "rzv2n-gpio",
					   ofnode_get_name(node),
					   node, &gpiodev);
	}

	return 0;
}

static const struct udevice_id rzv2n_pinctrl_match[] = {
	{ .compatible = "renesas,r9a09g056-pinctrl", .data = 12 },
	{}
};

U_BOOT_DRIVER(rzv2n_pinctrl) = {
	.name		= "rzv2n_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rzv2n_pinctrl_match,
	.probe		= rzv2n_pinctrl_probe,
	.plat_auto	= sizeof(struct rzv2n_pinctrl_priv),
	.ops		= &rzv2n_pinctrl_ops,
};
