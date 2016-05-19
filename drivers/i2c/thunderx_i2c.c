
#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>
#include <cavium/thunderx.h>

#define PCI_DEVICE_ID_THUNDER_TWSI	0xa012

#define TWSI_BUS_FREQ	100000
#define TWSI_THP	24

#define TWSI_SW_TWSI		0x0
#define TWSI_TWSI_SW		0x8
#define TWSI_INT		0x10
#define TWSI_SW_TWSI_EXT	0x18

union rst_boot {
	u64 u;
	struct {
		int rboot_pin:1;
		int rboot:1;
		int lboot:10;
		int lboot_ext23:6;
		int lboot_ext45:6;
		int reserved_24_29:6;
		int lboot_oci:3;
		int pnr_mul:6;
		int reserved_39_39:1;
		int c_mul:7;
		int reserved_47_54:8;
		int dis_scan:1;
		int dis_huk:1;
		int vrm_err:1;
		int jt_tstmode:1;
		int ckill_ppdis:1;
		int trusted_mode:1;
		int ejtagdis:1;
		int jtcsrdis:1;
		int chipkill:1;
	} s;
};

union twsx_sw_twsi {
	u64 u;
	struct {
		int data:32;
		int eop_ia:3;
		int ia:5;
		int addr:10;
		int scr:2;
		int size:3;
		int sovr:1;
		int r:1;
		int op:4;
		int eia:1;
		int slonly:1;
		int v:1;
	} s;
};

union twsx_sw_twsi_ext {
	u64 u;
	struct {
		int data:32;
		int ia:8;
	} s;
};


enum {
	TWSI_OP_WRITE	= 0,
	TWSI_OP_READ	= 1,
};

enum {
	TWSI_EOP_CLK_CTL = 3,
	TWSI_SW_EOP_IA   = 6,
};

enum {
	TWSI_SLAVEADD     = 0,
	TWSI_DATA         = 1,
	TWSI_CTL          = 2,
	TWSI_CLKCTL       = 3,
	TWSI_STAT         = 3,
	TWSI_SLAVEADD_EXT = 4,
	TWSI_RST          = 7,
};

enum {
	TWSI_CTL_AAK	= BIT(2),
	TWSI_CTL_IFLG	= BIT(3),
	TWSI_CTL_STP	= BIT(4),
	TWSI_CTL_STA	= BIT(5),
	TWSI_CTL_ENAB	= BIT(6),
	TWSI_CTL_CE	= BIT(7),
};

struct thunderx_twsi {
	int			id;
	int			speed;
	void			*baseaddr;
};

#define RST_BOOT_PNR_MUL(Val)  ((Val >> 33) & 0x1F)

static u64 twsi_write_sw(void *baseaddr, union twsx_sw_twsi twsi_sw)
{
	unsigned long timeout = 50000;

	twsi_sw.s.r = 0;
	twsi_sw.s.v = 1;

	writeq(twsi_sw.u, baseaddr + TWSI_SW_TWSI);

	do {
		twsi_sw.u = readq(baseaddr + TWSI_SW_TWSI);
	} while (twsi_sw.s.v != 0 && timeout--);

	return twsi_sw.u;
}

static u64 twsi_read_sw(void *baseaddr, union twsx_sw_twsi twsi_sw)
{
	unsigned long timeout = 50000;

	twsi_sw.s.r = 1;
	twsi_sw.s.v = 1;

	writeq(twsi_sw.u, baseaddr + TWSI_SW_TWSI);

	do {
		twsi_sw.u = readq(baseaddr + TWSI_SW_TWSI);
	} while (twsi_sw.s.v != 0 && timeout--);

	return twsi_sw.u;
}

static void twsi_write_ctl(void *baseaddr, u8 data)
{
	union twsx_sw_twsi twsi_sw;

	twsi_sw.u = 0;

	twsi_sw.s.op	 = TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_CTL;
	twsi_sw.s.data	 = data;

	twsi_write_sw(baseaddr, twsi_sw);
}

static u8 twsi_read_ctl(void *baseaddr)
{
	union twsx_sw_twsi twsi_sw;

	twsi_sw.u	 = 0;
	twsi_sw.s.op	 = TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_CTL;

	return twsi_read_sw(baseaddr, twsi_sw);
}

static int twsi_wait(void *baseaddr)
{
	unsigned int timeout = 50000;

	u8 twsi_ctl;

	do {
		twsi_ctl = twsi_read_ctl(baseaddr);
		twsi_ctl &= TWSI_CTL_IFLG;
	} while (timeout-- && !twsi_ctl);

	return timeout;
}

static void twsi_start(void *baseaddr)
{
	twsi_write_ctl(baseaddr, TWSI_CTL_STA | TWSI_CTL_ENAB);
}

static void twsi_stop(void *baseaddr)
{
	twsi_write_ctl(baseaddr, TWSI_CTL_STP | TWSI_CTL_ENAB);
}

static int twsi_write_data(void *baseaddr, u8  slave_addr,
			   u8 *buffer, unsigned int length)
{
	union twsx_sw_twsi twsi_sw;
	unsigned int curr = 0;

	twsi_start(baseaddr);
	twsi_wait(baseaddr);

	twsi_sw.u	 = 0;
	twsi_sw.s.op	 = TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_DATA;
	twsi_sw.s.data	 = (u32) (slave_addr << 1) | TWSI_OP_WRITE;

	twsi_write_sw(baseaddr, twsi_sw);
	twsi_write_ctl(baseaddr, TWSI_CTL_ENAB);

	twsi_wait(baseaddr);

	while (curr < length) {
		twsi_sw.u	 = 0;
		twsi_sw.s.op	 = TWSI_SW_EOP_IA;
		twsi_sw.s.eop_ia = TWSI_DATA;
		twsi_sw.s.data	 = buffer[curr++];

		twsi_write_sw(baseaddr, twsi_sw);
		twsi_write_ctl(baseaddr, TWSI_CTL_ENAB);

		twsi_wait(baseaddr);
	}

	twsi_stop(baseaddr);

	return 0;
}

static int twsi_read_data(void *baseaddr, u8 slave_addr,
		   u8 *buffer, unsigned int length)
{
	union twsx_sw_twsi twsi_sw;
	unsigned int curr = 0;


	twsi_start(baseaddr);
	twsi_wait(baseaddr);

	twsi_sw.u	 = 0;
	twsi_sw.s.op	 = TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_DATA;

	twsi_sw.s.data  = (u32) (slave_addr << 1) | TWSI_OP_READ;

	twsi_write_sw(baseaddr, twsi_sw);
	twsi_write_ctl(baseaddr, TWSI_CTL_ENAB);

	twsi_wait(baseaddr);

	while (curr < length) {
		twsi_write_ctl(baseaddr, TWSI_CTL_ENAB |
				(curr < length - 1) ? TWSI_CTL_AAK : 0);

		twsi_wait(baseaddr);

		twsi_sw.u = twsi_read_sw(baseaddr, twsi_sw);
		buffer[curr++] = twsi_sw.s.data;
	}

	twsi_stop(baseaddr);

	return 0;
}

static int twsi_init(void *baseaddr, unsigned int speed)
{
	int io_clock_hz;
	int n_div;
	int m_div;
	union twsx_sw_twsi sw_twsi;
	union rst_boot rst_boot;

	rst_boot.u = readl(RST_BOOT);

	io_clock_hz = rst_boot.s.pnr_mul * PLL_REF_CLK;

	/* Set the TWSI clock to a conservative TWSI_BUS_FREQ.  Compute the
	 * clocks M divider based on the SCLK.
	 * TWSI freq = (core freq) / (20 x (M+1) x (thp+1) x 2^N)
	 * M = ((core freq) / (20 x (TWSI freq) x (thp+1) x 2^N)) - 1 */
	for (n_div = 0; n_div < 8; n_div++) {
		m_div = io_clock_hz / (20 * speed * (TWSI_THP + 1));
		m_div /= 1 << n_div;
		m_div -= 1;
		if (m_div < 16)
			break;
	}

	sw_twsi.u = 0;
	sw_twsi.s.v = 1;		/* Clear valid bit */
	sw_twsi.s.op = 0x6;		/* See EOP field */
	sw_twsi.s.r = 0;		/* Select CLKCTL when R = 0 */
	sw_twsi.s.eop_ia = 3;	/* R=0 selects CLKCTL, R=1 selects STAT */
	sw_twsi.s.data = ((m_div & 0xf) << 3) | ((n_div & 0x7) << 0);

	/* Only init non-slave ports */
	writeq(sw_twsi.u, baseaddr + TWSI_SW_TWSI);

	return 0;
}

static int thunderx_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			     int nmsgs)
{
	struct thunderx_twsi *twsi = dev_get_priv(bus);
	int ret;

	debug("thunderx_i2c_xfer: %d messages\n", nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		debug("thunderx_i2c_xfer: chip=0x%x, len=0x%x\n",
		      msg->addr, msg->len);
		if (msg->flags & I2C_M_RD) {
			ret = twsi_read_data(twsi->baseaddr, msg->addr,
					     msg->buf, msg->len);
		} else {
			ret = twsi_write_data(twsi->baseaddr, msg->addr,
					      msg->buf, msg->len);
		}
		if (ret) {
			debug("thunderx_i2c_xfer: error sending\n");
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int thunderx_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct thunderx_twsi *twsi = dev_get_priv(dev);

	twsi->speed = speed;
	twsi_init(twsi->baseaddr, twsi->speed);

	return 0;
}

static int thunderx_i2c_probe(struct udevice *dev)
{
	struct thunderx_twsi *twsi = dev_get_priv(dev);
	pci_dev_t bdf = dm_pci_get_bdf(dev);

	debug("TWSI PCI device: %x\n", bdf);
	dev->req_seq = PCI_FUNC(bdf);

	twsi->baseaddr = dm_pci_map_bar(dev, 0, PCI_REGION_MEM);

	debug("TWSI bus %d at %p\n",dev->seq, twsi->baseaddr);

	twsi_init(twsi->baseaddr, TWSI_BUS_FREQ);

	return 0;
}

static const struct dm_i2c_ops thunderx_i2c_ops = {
	.xfer		= thunderx_i2c_xfer,
	.set_bus_speed	= thunderx_i2c_set_bus_speed,
};

static const struct udevice_id thunderx_i2c_ids[] = {
	{ .compatible = "cavium,thunderx-i2c" },
	{ }
};

U_BOOT_DRIVER(thunderx_pci_twsi) = {
	.name	= "i2c_thunderx",
	.id	= UCLASS_I2C,
	.of_match = thunderx_i2c_ids,
	.probe	= thunderx_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct thunderx_twsi),
	.ops	= &thunderx_i2c_ops,
};

static struct pci_device_id thunderx_pci_twsi_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_THUNDER_TWSI) },
	{},
};

U_BOOT_PCI_DEVICE(thunderx_pci_twsi, thunderx_pci_twsi_supported);
