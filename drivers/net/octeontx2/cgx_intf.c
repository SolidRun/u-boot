/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */


#include <common.h>
#include <net.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/list.h>
#include <asm/arch/octeontx2.h>

#include "cgx_intf.h"
#include "cgx.h"

static u64 cgx_rd_scrx(u8 cgx, u8 lmac, u8 index)
{
	u64 addr;

	addr = (index == 1) ? CGX_CMR_SCRATCH1 : CGX_CMR_SCRATCH0;
	addr += CGX_SHIFT(cgx) + CMR_SHIFT(lmac);
	return readq(addr);
}

static void cgx_wr_scrx(u8 cgx, u8 lmac, u8 index, u64 val)
{
	u64 addr;

	addr = (index == 1) ? CGX_CMR_SCRATCH1 : CGX_CMR_SCRATCH0;
	addr += CGX_SHIFT(cgx) + CMR_SHIFT(lmac);
	writeq(val, addr);
}

static u64 cgx_rd_scr0(u8 cgx, u8 lmac)
{
	return cgx_rd_scrx(cgx, lmac, 0);
}

static u64 cgx_rd_scr1(u8 cgx, u8 lmac)
{
	return cgx_rd_scrx(cgx, lmac, 1);
}

static void cgx_wr_scr0(u8 cgx, u8 lmac, u64 val)
{
	return cgx_wr_scrx(cgx, lmac, 0, val);
}

static void cgx_wr_scr1(u8 cgx, u8 lmac, u64 val)
{
	return cgx_wr_scrx(cgx, lmac, 1, val);
}

static inline void set_ownership(u8 cgx, u8 lmac, u8 val)
{
	union cgx_scratchx1 scr1;
	scr1.u = cgx_rd_scr1(cgx, lmac);
	scr1.s.own_status = val;
	cgx_wr_scr1(cgx, lmac, scr1.u);
}

static int wait_for_ownership(u8 cgx, u8 lmac)
{
	union cgx_scratchx1 scr1;
	union cgx_scratchx0 scr0;
	u64 cmrx_int;
	int timeout = 5000;

	do {
	
		scr1.u = cgx_rd_scr1(cgx, lmac);
		scr0.u = cgx_rd_scr0(cgx, lmac);
		/* clear async events if any */
		if ((scr0.s.evt_sts.evt_type == CGX_EVT_ASYNC) &&
			scr0.s.evt_sts.ack) {
			/* clear interrupt */
			cmrx_int = readq(CGX_CMR_INT +
					 CGX_SHIFT(cgx) + CMR_SHIFT(lmac));
			cmrx_int |= 0x2; // Overflw bit
			writeq(cmrx_int, CGX_CMR_INT +
					 CGX_SHIFT(cgx) + CMR_SHIFT(lmac));

			/* clear ack */
			scr0.s.evt_sts.ack = 0;
			cgx_wr_scr0(cgx, lmac, scr0.u);
		}

		if (timeout-- < 0) {
			debug("timeout waiting for ownership\n");
			return -ETIMEDOUT;
		}
		mdelay(1);
	} while ((scr1.s.own_status == CGX_OWN_FIRMWARE) &&
		  scr0.s.evt_sts.ack);

	return 0;
}

int cgx_intf_req(u8 cgx, u8 lmac, u8 cmd, u64 *rsp)
{
	union cgx_scratchx1 scr1;
	union cgx_scratchx0 scr0;
	u64 cmrx_int;
	int timeout = 500;
	int err = 0;

	if (wait_for_ownership(cgx, lmac)) {
		err = -ETIMEDOUT;
		goto error;
	}

	/* send command */
	scr1.u = cgx_rd_scr1(cgx, lmac);
	scr1.s.cmd.id = cmd;
	cgx_wr_scr1(cgx, lmac, scr1.u);

	set_ownership(cgx, lmac, CGX_OWN_FIRMWARE);

	/* wait for response and ownership */
	do {
		scr0.u = cgx_rd_scr0(cgx, lmac);
		scr1.u = cgx_rd_scr1(cgx, lmac);
		mdelay(10);
	} while (timeout-- && ( !scr0.s.evt_sts.ack) &&
		 (scr1.s.own_status == CGX_OWN_FIRMWARE));
	if (timeout < 0) {
		debug("%s timeout waiting for ack\n",__func__);
		err = -ETIMEDOUT;
		goto error;
	}

	if (cmd == CGX_CMD_INTF_SHUTDOWN)
		goto error;

	if (scr0.s.evt_sts.evt_type != CGX_EVT_CMD_RESP) {
		printf("%s received async event instead of cmd resp event\n",
			__func__);
		err = -1;
		goto error;
	}
	if (scr0.s.evt_sts.id != cmd) {
		printf("%s received resp for cmd %d expected cmd %d \n",
				__func__, scr0.s.evt_sts.id, cmd);
		err = -1;
		goto error;
	}
	if (scr0.s.evt_sts.stat != CGX_STAT_SUCCESS) {
		debug("%s failure for cmd %d on cgx %u lmac %u with errcode"
			" %d\n", __func__, cmd, cgx, lmac, scr0.s.link_sts.err_type);
		err = -1;
		goto error;
	}

error:
	/* clear interrupt */
	cmrx_int = readq(CGX_CMR_INT + CGX_SHIFT(cgx) + CMR_SHIFT(lmac));
	cmrx_int |= 0x2; // Overflw bit
	writeq(cmrx_int, CGX_CMR_INT + CGX_SHIFT(cgx) + CMR_SHIFT(lmac));

	/* clear ownership and ack */
	scr0.s.evt_sts.ack = 0;
	cgx_wr_scr0(cgx, lmac, scr0.u);

	*rsp = err ? 0 : scr0.u;

	return err;
}


int cgx_intf_get_mac_addr(u8 cgx, u8 lmac, u8 *mac)
{
	union cgx_scratchx0 scr0;
	int ret;

	ret = cgx_intf_req(cgx, lmac,
				CGX_CMD_GET_MAC_ADDR, &scr0.u);
	if (ret)
		return -1;

	scr0.u >>= 9;
	memcpy(mac, &scr0.u, 6);

	return 0;
}

int cgx_intf_get_ver(u8 cgx, u8 lmac, u8 *ver)
{
	union cgx_scratchx0 scr0;
	int ret;

	ret = cgx_intf_req(cgx, lmac,
				CGX_CMD_GET_FW_VER, &scr0.u);
	if (ret)
		return -1;

	scr0.u >>= 9;
	*ver = scr0.u & 0xFFFF;

	return 0;
}

int cgx_intf_get_link_sts(u8 cgx, u8 lmac, u64 *lnk_sts)
{
	union cgx_scratchx0 scr0;
	int ret;

	ret = cgx_intf_req(cgx, lmac,
				CGX_CMD_GET_LINK_STS, &scr0.u);
	if (ret)
		return -1;

	scr0.u >>= 9;
	/* pass the same format as cgx_lnk_sts_s
	 * err_type:10, speed:4, full_duplex:1, link_up:1
	 */
	*lnk_sts = scr0.u & 0xFFFF;
	return 0;
}

int cgx_intf_link_up_dwn(u8 cgx, u8 lmac, u8 up_dwn, u64 *lnk_sts)
{
	union cgx_scratchx0 scr0;
	int ret;
	u8 cmd;

	cmd = up_dwn ? CGX_CMD_LINK_BRING_UP : CGX_CMD_LINK_BRING_DOWN;

	ret = cgx_intf_req(cgx, lmac, cmd, &scr0.u);
	if (ret)
		return -1;

	scr0.u >>= 9;
	/* pass the same format as cgx_lnk_sts_s
	 * err_type:10, speed:4, full_duplex:1, link_up:1
	 */
	*lnk_sts = scr0.u & 0xFFFF;
	return 0;
}

void cgx_intf_shutdown(void)
{
	union cgx_scratchx0 scr0;

	cgx_intf_req(0, 0, CGX_CMD_INTF_SHUTDOWN, &scr0.u);
}

