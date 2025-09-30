/*
 * Renesas USB
 *
 * Copyright (C) 2011 Renesas Solutions Corp.
 * Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>
 *
 *
 * Ported to u-boot
 * Copyright (C) 2016 GlobalLogic
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef RENESAS_USB_H
#define RENESAS_USB_H
#include <linux/usb/ch9.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>

typedef unsigned long uintptr_t;

//#define USBHS_DEBUG_IRQ
#ifdef USBHS_DEBUG_IRQ
#define pr_irq  printf
#else
#define pr_irq(...)  do{}while(0)
#endif


// #define USBHS_DEBUG

#ifdef USBHS_DEBUG
#define pr_dbg  printf
#else
#define pr_dbg(...)  do{}while(0)
#endif

/* avoid conflict with <dm/device.h> */
#ifdef dev_dbg
#undef dev_dbg
#endif
#ifdef dev_vdbg
#undef dev_vdbg
#endif
#ifdef dev_info
#undef dev_info
#endif
#ifdef dev_err
#undef dev_err
#endif
#ifdef dev_warn
#undef dev_warn
#endif

#define dev_dbg(dev, fmt, args...)		\
	debug(fmt, ##args)
#define dev_vdbg(dev, fmt, args...)		\
	debug(fmt, ##args)
#define dev_info(dev, fmt, args...)		\
	printf(fmt, ##args)
#define dev_err(dev, fmt, args...)		\
	printf(fmt, ##args)
#define dev_warn(dev, fmt, args...)		\
	printf(fmt, ##args)

#if defined(CONFIG_ARCH_RZMPU)
#define iowrite32_rep writesl
#define ioread32_rep readsl
#define iowrite8 writeb
#define ioread32 readl
#else

#ifndef iowrite32_rep
#define iowrite32_rep __raw_writesl
#endif

#ifndef ioread32_rep
#define ioread32_rep __raw_readsl
#endif

#define iowrite8 writeb
#define ioread32 readl
#endif

#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))


struct platform_device {
	const char	*name;
	int		id;
	struct device	dev;
	u32		num_resources;
	struct resource	*resource;

	char *driver_override; /* Driver name to force a match */

	/* MFD cell pointer */
	struct mfd_cell *mfd_cell;

	/* arch specific additions */
	void *dev_platform_data;
};


/*
 * module type
 *
 * it will be return value from get_id
 */
enum {
	USBHS_HOST = 0,
	USBHS_GADGET,
	USBHS_MAX,
};

/*
 * callback functions table for driver
 *
 * These functions are called from platform for driver.
 * Callback function's pointer will be set before
 * renesas_usbhs_platform_callback :: hardware_init was called
 */
struct renesas_usbhs_driver_callback {
	int (*notify_hotplug)(void *pdev);
};

/*
 * callback functions for platform
 *
 * These functions are called from driver for platform
 */
struct renesas_usbhs_platform_callback {

	/*
	 * option:
	 *
	 * Hardware init function for platform.
	 * it is called when driver was probed.
	 */
	int (*hardware_init)(struct platform_device *pdev);

	/*
	 * option:
	 *
	 * Hardware exit function for platform.
	 * it is called when driver was removed
	 */
	int (*hardware_exit)(struct platform_device *pdev);

	/*
	 * option:
	 *
	 * for board specific clock control
	 */
	int (*power_ctrl)(struct platform_device *pdev,
			   void __iomem *base, int enable);

	/*
	 * option:
	 *
	 * Phy reset for platform
	 */
	int (*phy_reset)(struct platform_device *pdev);

	/*
	 * get USB ID function
	 *  - USBHS_HOST
	 *  - USBHS_GADGET
	 */
	int (*get_id)(struct platform_device *pdev);

	/*
	 * get VBUS status function.
	 */
	int (*get_vbus)(struct platform_device *pdev);

	/*
	 * option:
	 *
	 * VBUS control is needed for Host
	 */
	int (*set_vbus)(struct platform_device *pdev, int enable);
};

/*
 * parameters for renesas usbhs
 *
 * some register needs USB chip specific parameters.
 * This struct show it to driver
 */

struct renesas_usbhs_driver_pipe_config {
	u8 type;	/* USB_ENDPOINT_XFER_xxx */
	u16 bufsize;
	u8 bufnum;
	bool double_buf;
};
#define RENESAS_USBHS_PIPE(_type, _size, _num, _double_buf)	{	\
			.type = (_type),		\
			.bufsize = (_size),		\
			.bufnum = (_num),		\
			.double_buf = (_double_buf),	\
	}

struct renesas_usbhs_driver_param {
	/*
	 * pipe settings
	 */
	struct renesas_usbhs_driver_pipe_config *pipe_configs;
	int pipe_size; /* pipe_configs array size */

	/*
	 * option:
	 *
	 * for BUSWAIT :: BWAIT
	 * see
	 *	renesas_usbhs/common.c :: usbhsc_set_buswait()
	 * */
	int buswait_bwait;

	/*
	 * option:
	 *
	 * delay time from notify_hotplug callback
	 */
	int detection_delay; /* msec */

	/*
	 * option:
	 *
	 * dma id for dmaengine
	 * The data transfer direction on D0FIFO/D1FIFO should be
	 * fixed for keeping consistency.
	 * So, the platform id settings will be..
	 *	.d0_tx_id = xx_TX,
	 *	.d1_rx_id = xx_RX,
	 * or
	 *	.d1_tx_id = xx_TX,
	 *	.d0_rx_id = xx_RX,
	 */
	int d0_tx_id;
	int d0_rx_id;
	int d1_tx_id;
	int d1_rx_id;
	int d2_tx_id;
	int d2_rx_id;
	int d3_tx_id;
	int d3_rx_id;

	/*
	 * option:
	 *
	 * pio <--> dma border.
	 */
	int pio_dma_border; /* default is 64byte */

	uintptr_t type;
	u32 enable_gpio;

	/*
	 * option:
	 */
	u32 has_otg:1; /* for controlling PWEN/EXTLP */
	u32 has_sudmac:1; /* for SUDMAC */
	u32 has_usb_dmac:1; /* for USB-DMAC */
#if defined(CONFIG_R9A07G044L) || defined(CONFIG_R9A07G044C) || defined(CONFIG_R9A07G043U) || defined(CONFIG_R9A07G054L) || defined(CONFIG_ARCH_RZMPU)
	u32 has_cnen:1;
	u32 cfifo_byte_addr:1;	/* CFIFO is byte addressable */
#endif
#define USBHS_USB_DMAC_XFER_SIZE	32	/* hardcode the xfer size */
};

#define USBHS_TYPE_RCAR_GEN2	1
#define USBHS_TYPE_RCAR_GEN3	2
#if defined(CONFIG_R9A07G044L) || defined(CONFIG_R9A07G044C) || defined(CONFIG_R9A07G043U) || defined(CONFIG_R9A07G054L) || defined(CONFIG_ARCH_RZMPU)
#define USBHS_TYPE_G2L		5
#endif

/*
 * option:
 *
 * platform information for renesas_usbhs driver.
 */
struct renesas_usbhs_platform_info {
	/*
	 * option:
	 *
	 * platform set these functions before
	 * call platform_add_devices if needed
	 */
	struct renesas_usbhs_platform_callback	platform_callback;

	/*
	 * driver set these callback functions pointer.
	 * platform can use it on callback functions
	 */
	struct renesas_usbhs_driver_callback	driver_callback;

	/*
	 * option:
	 *
	 * driver use these param for some register
	 */
	struct renesas_usbhs_driver_param	driver_param;
};


/*
* Migrated from Device.h
*/
#ifdef dev_get_drvdata
#undef dev_get_drvdata
#endif


/*
* Migrated from Phy.c
*/

struct phy;

/**
 * struct phy_ops - set of function pointers for performing phy operations
 * @init: operation to be performed for initializing phy
 * @exit: operation to be performed while exiting
 * @power_on: powering on the phy
 * @power_off: powering off the phy
 * @owner: the module owner containing the ops
 */
struct phy_ops {
	int	(*init)(struct phy *phy);
	int	(*exit)(struct phy *phy);
	int	(*power_on)(struct phy *phy);
	int	(*power_off)(struct phy *phy);
	struct module *owner;
};

/**
 * struct phy_attrs - represents phy attributes
 * @bus_width: Data path width implemented by PHY
 */
struct phy_attrs {
	u32			bus_width;
};

/**
 * struct phy - represents the phy device
 * @dev: phy device
 * @id: id of the phy device
 * @ops: function pointers for performing phy operations
 * @init_data: list of PHY consumers (non-dt only)
 * @mutex: mutex to protect phy_ops
 * @init_count: used to protect when the PHY is used by multiple consumers
 * @power_count: used to protect when the PHY is used by multiple consumers
 * @phy_attrs: used to specify PHY specific attributes
 */
struct phy {
	struct device		dev;
	int			id;
	const struct phy_ops	*ops;
	struct mutex		mutex;
	int			init_count;
	int			power_count;
	struct phy_attrs	attrs;
//	struct regulator	*pwr;
};

struct rcar_gen3_data {
	void __iomem *base;
	struct clk *clk;
};

struct rcar_gen3_chan {
	struct rcar_gen3_data usb2;
	struct phy *phy;
	bool has_otg;
};


static inline void *dev_get_drvdata(const struct device *dev)
{
	return dev->driver_data;
}

#ifdef dev_set_drvdata
#undef dev_set_drvdata
#endif

static inline void dev_set_drvdata(struct device *dev, void *data)
{
	dev->driver_data = data;
}

/*
* Migrated from Platform.h
*/

#ifdef platform_set_drvdata
#undef platform_set_drvdata
#endif

static inline void platform_set_drvdata(struct platform_device *pdev,
					void *data)
{
	dev_set_drvdata(&pdev->dev, data);
}

static inline void *platform_get_drvdata(struct platform_device *pdev)
{
	return dev_get_drvdata(&pdev->dev);
}

static inline void phy_set_drvdata(struct phy *phy, void *data)
{
	dev_set_drvdata(&phy->dev, data);
}

static inline void *phy_get_drvdata(struct phy *phy)
{
	return dev_get_drvdata(&phy->dev);
}


/*
 * macro for platform
 */
#define renesas_usbhs_get_info(pdev)\
	((struct renesas_usbhs_platform_info *)(pdev)->dev_platform_data)

#define renesas_usbhs_call_notify_hotplug(pdev)				\
	({								\
		struct renesas_usbhs_driver_callback *dc;		\
		dc = &(renesas_usbhs_get_info(pdev)->driver_callback);	\
		if (dc && dc->notify_hotplug)				\
			dc->notify_hotplug(pdev);			\
	})


#if defined(CONFIG_R9A07G044L) || defined(CONFIG_R9A07G044C) || defined(CONFIG_R9A07G043U) || defined(CONFIG_R9A07G054L) || defined(CONFIG_ARCH_RZMPU)
#define PHY_BASE	0x11c50200
#define RCAR3_PHY_DEVICE "RZG2L-PHY "
#define USBHS_BASE	0x11c60000
#define RCAR3_USBHS_DEVICE "RZG2L-USBHS "
#else /* !defined(CONFIG_R9A07G044L) */
#define PHY_BASE	0xee080200
#define RCAR3_PHY_DEVICE "R-CAR3-PHY "
#define USBHS_BASE	0xe6590000
#define RCAR3_USBHS_DEVICE "R-CAR3-USBHS "
#endif


typedef irqreturn_t (*irq_handler_t)(int, void *);

int
devm_request_irq(struct device *dev, unsigned int irq, irq_handler_t handler,
		 unsigned long irqflags, const char *devname, void *dev_id);

struct phy *devm_phy_create(struct device *dev, void *node,
			    const struct phy_ops *ops);



#endif /* RENESAS_USB_H */
