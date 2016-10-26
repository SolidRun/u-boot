/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/bitops.h>
#include <dt-bindings/gpio/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_BIT(x)		(1 << x)

#define GPIO_RX_DAT		(0x0)
#define GPIO_TX_SET		(0x8)
#define GPIO_TX_CLR		(0x10)

#define GPIO_BIT_CFG(x)		(0x400 + 8 * (x))
#define GPIO_BIT_CFG_FN(x)	(((x) >> 16) & 0x3ff)
#define GPIO_BIT_CFG_TX_OE(x)	((x) & 0x1)
#define GPIO_BIT_CFG_RX_DAT(x)	((x) & 0x1)

static const int DIRECTION_INPUT  = 0;
static const int DIRECTION_OUTPUT = 1;

struct thunderx_gpio {
	void __iomem *baseaddr;
};

static int thunderx_gpio_dir_input(struct udevice *dev, unsigned offset)
{
	struct thunderx_gpio *gpio = dev_get_priv(dev);

	clrbits_le64(gpio->baseaddr + GPIO_BIT_CFG(offset),
		     (0x3ffUL << 16) | 1UL);

	return 0;
}

static int thunderx_gpio_dir_output(struct udevice *dev, unsigned offset,
				    int value)
{
	struct thunderx_gpio *gpio = dev_get_priv(dev);

	if (value) {
		setbits_le64(gpio->baseaddr + GPIO_TX_SET,
			   GPIO_BIT(offset));
	} else {
		setbits_le64(gpio->baseaddr + GPIO_TX_CLR,
			   GPIO_BIT(offset));
	}

	clrsetbits_le64(gpio->baseaddr + GPIO_BIT_CFG(offset),
		  (0x3ffUL << 16), 1UL);


	return 0;
}

static int thunderx_gpio_get_value(struct udevice *dev,
				   unsigned offset)
{
	struct thunderx_gpio *gpio = dev_get_priv(dev);
	u64 reg = readq(gpio->baseaddr + GPIO_RX_DAT);

	return (reg & GPIO_BIT(offset)) != 0;
}

static int thunderx_gpio_set_value(struct udevice *dev,
				   unsigned offset, int value)
{
	struct thunderx_gpio *gpio = dev_get_priv(dev);

	if (value) {
		setbits_le64(gpio->baseaddr + GPIO_TX_SET,
			     GPIO_BIT(offset));
	} else {
		setbits_le64(gpio->baseaddr + GPIO_TX_CLR,
			     GPIO_BIT(offset));
	}

	return 0;

}

static int thunderx_gpio_get_function(struct udevice *dev,
				      unsigned offset)
{
	struct thunderx_gpio *gpio = dev_get_priv(dev);
	u64 pinsel = readl(gpio->baseaddr + GPIO_BIT_CFG(offset));

	if (GPIO_BIT_CFG_FN(pinsel))
		return GPIOF_FUNC;
	else if (GPIO_BIT_CFG_TX_OE(pinsel))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int thunderx_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			    struct fdtdec_phandle_args *args)
{
	desc->offset = args->args[0];
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;

	return 0;
}

static const struct dm_gpio_ops thunderx_gpio_ops = {
	.direction_input	= thunderx_gpio_dir_input,
	.direction_output	= thunderx_gpio_dir_output,
	.get_value		= thunderx_gpio_get_value,
	.set_value		= thunderx_gpio_set_value,
	.get_function		= thunderx_gpio_get_function,
	.xlate			= thunderx_gpio_xlate,
};

static int thunderx_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct thunderx_gpio *priv = dev_get_priv(dev);
	u32    pins[2];
	char *end;
	int ret;

	priv->baseaddr = (void *)dev_get_addr(dev);

	ret = fdtdec_get_int_array(gd->fdt_blob, dev->of_offset, "pins",
				   pins, 2);

	if (ret < 0)
		return ret;

	uc_priv->gpio_base  = pins[0];
	uc_priv->gpio_count = pins[1] - pins[0];
	uc_priv->bank_name  = strdup(dev->name);
	end = strchr(uc_priv->bank_name, '@');
	end[0] = 'A' + dev->seq;
	end[1] = '\0';

	return 0;
}


static const struct udevice_id thunderx_gpio_ids[] = {
	{ .compatible = "cavium,thunderx-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_thunderx) = {
	.name	= "gpio_thunderx",
	.id	= UCLASS_GPIO,
	.of_match = thunderx_gpio_ids,
	.probe = thunderx_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct thunderx_gpio),
	.ops	= &thunderx_gpio_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
