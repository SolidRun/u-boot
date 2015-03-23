
#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <cavium/thunderx.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define TWSI_BUS_FREQ 100000
#define TWSI_THP 24

#define MIO_TWSX_SW_TWSI(num)	  (0x87E0D0001000ULL + (num << 24))
#define MIO_TWSX_SW_TWSI_EXT(num) (0x87E0D0001018ULL + (num << 24))

union rst_boot {
	uint64_t u;
	struct rst_boot_s {
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

union mio_twsx_sw_twsi {
	uint64_t u;
	struct mio_twsx_sw_twsi_s {
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

union mio_twsx_sw_twsi_ext {
	uint64_t u;
	struct mio_twsx_sw_twsi_ext_s {
		int data:32;
		int ia:8;
	} s;
};

int thunderx_twsix_init(int bus)
{
	int io_clock_hz;
	int N_divider;
	int M_divider;
	union mio_twsx_sw_twsi sw_twsi;
	union rst_boot rst_boot;

	rst_boot.u = readl(RST_BOOT);

	io_clock_hz = rst_boot.s.pnr_mul * PLL_REF_CLK;

	/* Set the TWSI clock to a conservative TWSI_BUS_FREQ.  Compute the
	 * clocks M divider based on the SCLK.
	 * TWSI freq = (core freq) / (20 x (M+1) x (thp+1) x 2^N)
	 * M = ((core freq) / (20 x (TWSI freq) x (thp+1) x 2^N)) - 1 */
	for (N_divider = 0; N_divider < 8; N_divider++) {
		M_divider = (io_clock_hz / (20 * TWSI_BUS_FREQ *
					   (TWSI_THP + 1) *
					   (1 << N_divider))) - 1;
		if (M_divider < 16)
			break;
	}

	sw_twsi.u = 0;
	sw_twsi.s.v = 1;		/* Clear valid bit */
	sw_twsi.s.op = 0x6;		/* See EOP field */
	sw_twsi.s.r = 0;		/* Select CLKCTL when R = 0 */
	sw_twsi.s.eop_ia = 3;	/* R=0 selects CLKCTL, R=1 selects STAT */
	sw_twsi.s.data = ((M_divider & 0xf) << 3) | ((N_divider & 0x7) << 0);

	/* Only init non-slave ports */
	writeq(sw_twsi.u, MIO_TWSX_SW_TWSI(bus));

	return 0;
}

int64_t thunderx_twsix_rd_ia(int twsi_id, uint8_t dev_addr,
			     uint16_t addr,
			     int addr_len, int num_bytes)
{
	union mio_twsx_sw_twsi sw_twsi_val;
	union mio_twsx_sw_twsi_ext twsi_ext;
	int retry_limit = 5;
	int timeout;

retry:
	twsi_ext.u = 0;
	sw_twsi_val.u = 0;
	sw_twsi_val.s.v = 1;
	sw_twsi_val.s.r = 1;
	sw_twsi_val.s.sovr = 1;
	sw_twsi_val.s.size = num_bytes - 1;
	sw_twsi_val.s.addr = dev_addr;

	if (addr_len > 0) {
		sw_twsi_val.s.op = 1;
		sw_twsi_val.s.ia = (addr >> 3) & 0x1f;
		sw_twsi_val.s.eop_ia = addr & 0x7;
		if (addr_len == 2) {
			sw_twsi_val.s.eia = 1;
			twsi_ext.s.ia = addr >> 8;
			writeq(twsi_ext.u, MIO_TWSX_SW_TWSI_EXT(twsi_id));
		}
	}

	writeq(sw_twsi_val.u, MIO_TWSX_SW_TWSI(twsi_id));

	timeout = 1000;

	do {
		sw_twsi_val.u = readq(MIO_TWSX_SW_TWSI(twsi_id));
		mdelay(1);
		timeout--;
	} while (sw_twsi_val.s.v && retry_limit);

	if (!timeout) {
		printf("TWSI%d: Timeout waiting for operation to complete\n",
		       twsi_id);
		return -1;
	}

	sw_twsi_val.u = readq(MIO_TWSX_SW_TWSI(twsi_id));

	if (!sw_twsi_val.s.r) {
		/* Check the reason of the failure.
		** Lost arbitration : 0x38, 0x68, 0xB0, 0x78
		** Core busy as slave: 0x80, 0x88, 0xA0, 0xA8, 0xB8, 0xC0, 0xC8
		*/
		switch (sw_twsi_val.s.data) {
		case 0x38:
		case 0x68:
		case 0xB0:
		case 0x78:
		case 0x80:
		case 0x88:
		case 0xA0:
		case 0xA8:
		case 0xB8:
		case 0xC8:
			if (retry_limit-- > 0)
				goto retry;
		default:
			return -1;
		}
		/* For all other errors, return an error code */
	}

	return (sw_twsi_val.s.data & (0xFFFFFFFF >> (32 - num_bytes * 8)));
}

int thunderx_twsix_wr_ia(int twsi_id, uint8_t dev_addr, uint16_t addr,
			 int addr_len, uint64_t data, int num_bytes)
{
		union mio_twsx_sw_twsi sw_twsi_val;
		union mio_twsx_sw_twsi_ext twsi_ext;
		int timeout;
		int to;

		twsi_ext.u = 0;

		sw_twsi_val.u = 0;
		sw_twsi_val.s.v = 1;
		sw_twsi_val.s.sovr = 1;
		sw_twsi_val.s.size = num_bytes - 1;
		sw_twsi_val.s.addr = dev_addr;
		sw_twsi_val.s.data = 0xFFFFFFFF & data;

		if (addr_len > 0) {
			sw_twsi_val.s.op = 1;
			sw_twsi_val.s.ia = (addr >> 3) & 0x1f;
			sw_twsi_val.s.eop_ia = addr & 0x7;
		}

		if (addr_len == 2) {
			sw_twsi_val.s.eia = 1;
			twsi_ext.s.ia = addr >> 8;
		}

		if (num_bytes > 4)
			twsi_ext.s.data = data >> 32;

		writeq(twsi_ext.u, MIO_TWSX_SW_TWSI_EXT(twsi_id));
		writeq(sw_twsi_val.u, MIO_TWSX_SW_TWSI(twsi_id));

		timeout = 1000;

		do {
			sw_twsi_val.u = readq(MIO_TWSX_SW_TWSI(twsi_id));
			mdelay(1);
			timeout--;
		} while (sw_twsi_val.s.v && timeout);

		if (!timeout) {
			printf("TWSI%d: Timeout waiting for operation to complete\n", twsi_id);
			return -1;
		}

		/* Poll until reads succeed, or polling times out */
		to = 100;
		while (to-- > 0) {
			if (thunderx_twsix_rd_ia(twsi_id, dev_addr, 0, 1, 0) >= 0)
				break;
		}
		if (to <= 0)
			return -1;

		return 0;
}

static void thunderx_i2c_init(struct i2c_adapter *adap,
					int speed, int slaveaddr)
{
	thunderx_twsix_init(adap->hwadapnr);
}

static int thunderx_i2c_probe(struct i2c_adapter *adap, uchar chip)
{
	int ret;

	ret = thunderx_twsix_rd_ia(adap->hwadapnr, chip, 0, 1, 1);

	return (ret >= 0);
}

static int thunderx_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
					int alen, uchar *buffer, int len)
{
	int64_t bytes;

	int ret = 0, i;
	int toread;

	if (alen < 0 || alen > 2)
		return -1;

	while (len) {
		toread = MIN(len, 4);
		bytes = thunderx_twsix_rd_ia(adap->hwadapnr, chip, addr, alen, toread);

		if (bytes < 0) {
			ret = -1;
			break;
		}

		len -= toread;
		addr += toread;

		for (i = 0; i < toread; i++) {
			*buffer++ = bytes >> (3 - i) * 8;
		}
	}

	return ret;
}

static int thunderx_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
					int alen, uchar *buffer, int len)
{
	int64_t bytes;

	int ret = 0, i;
	int towrite;

	if (alen < 0 || alen > 2)
		return -1;

	while (len) {
		towrite = MIN(len, 8);
		bytes = 0;
		for (i = 0; i < towrite; i++) {
			bytes |= *buffer << (3 - i) * 8;
			buffer++;
		}

		bytes = thunderx_twsix_wr_ia(adap->hwadapnr, chip, addr, alen, bytes, towrite);

		if (bytes < 0) {
			ret = -1;
			break;
		}
		len -= towrite;
		addr += towrite;
	}
	return ret;
}

static unsigned int thunderx_i2c_set_bus_speed(struct i2c_adapter *adap,
						unsigned int speed)
{
	if (speed != adap->speed)
		return -1;
	return speed;
}

/*
 * Register thunderx i2c adapters
 */
U_BOOT_I2C_ADAP_COMPLETE(thunderx_0, thunderx_i2c_init, thunderx_i2c_probe,
			 thunderx_i2c_read, thunderx_i2c_write,
			 thunderx_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_THUNDERX_SPEED_0,
			 CONFIG_SYS_I2C_THUNDERX_SLAVE_0, 0)

U_BOOT_I2C_ADAP_COMPLETE(thunderx_1, thunderx_i2c_init, thunderx_i2c_probe,
			 thunderx_i2c_read, thunderx_i2c_write,
			 thunderx_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_THUNDERX_SPEED_1,
			 CONFIG_SYS_I2C_THUNDERX_SLAVE_1, 1)
