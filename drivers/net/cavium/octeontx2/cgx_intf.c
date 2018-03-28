/*
 * Copyright (C) 2018 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <errno.h>

#include "cgx_intf.h"

static uint64_t cgx_rd_scrx(u8 cgx, u8 lmac, u8 index)
{
	uint64_t addr;

	addr = (index == 1) ? CGX_CMR_SCRATCH1 : CGX_CMR_SCRATCH0;
	addr += CGX_SHIFT(cgx) + CMR_SHIFT(lmac);
	return readq(addr);
}

static void cgx_wr_scrx(u8 cgx, u8 lmac, u8 index, uint64_t val)
{
	uint64_t addr;

	addr = (index == 1) ? CGX_CMR_SCRATCH1 : CGX_CMR_SCRATCH0;
	addr += CGX_SHIFT(cgx) + CMR_SHIFT(lmac);
	writeq(val, addr);
}

static uint64_t cgx_rd_scr0(u8 cgx, u8 lmac)
{
	return cgx_rd_scrx(cgx, lmac, 0);
}

static uint64_t cgx_rd_scr1(u8 cgx, u8 lmac)
{
	return cgx_rd_scrx(cgx, lmac, 1);
}

static void cgx_wr_scr0(u8 cgx, u8 lmac, uint64_t val)
{
	return cgx_wr_scrx(cgx, lmac, 0, val);
}

static void cgx_wr_scr1(u8 cgx, u8 lmac, uint64_t val)
{
	return cgx_wr_scrx(cgx, lmac, 1, val);
}

void set_ownership(u8 cgx, u8 lmac, u8 val)
{
	union cgx_scratchx1 scr1;
	scr1.u = cgx_rd_scr1(cgx, lmac);
	scr1.s.own_status = val;
	cgx_wr_scr1(cgx, lmac, scr1.u);		
}

int wait_for_ownership(u8 cgx, u8 lmac)
{
	union cgx_scratchx1 scr1;
	union cgx_scratchx0 scr0;
	int timeout = 5000;
	
	scr1.u = cgx_rd_scr1(cgx, lmac);
	scr0.u = cgx_rd_scr0(cgx, lmac);
	
	while (scr1.s.own_status == CGX_OWN_FIRMWARE &&
		scr0.s.evt_sts.ack) {
		if (timeout-- < 0)
			return -ETIMEDOUT;
		mdelay(1);
		scr1.u = cgx_rd_scr1(cgx, lmac);
		scr0.u = cgx_rd_scr0(cgx, lmac);
	}
	return 0;
}

int cgx_intf_req(u8 cgx, u8 lmac, u8 cmd, uint64_t *rsp)
{
	union cgx_scratchx1 scr1;
	union cgx_scratchx0 scr0;
	int timeout = 500;

	if (wait_for_ownership(cgx, lmac))
		return -ETIMEDOUT;

	set_ownership(cgx, lmac, CGX_OWN_NON_SECURE_FW);

	/* send command */
	scr1.u = cgx_rd_scr1(cgx, lmac);
	scr1.s.cmd.id = cmd;
	cgx_wr_scr1(cgx, lmac, scr1.u);

	set_ownership(cgx, lmac, CGX_OWN_FIRMWARE);

	/* wait for response */
	do {
		scr0.u = cgx_rd_scr0(cgx, lmac);
		mdelay(10);
	} while (timeout-- && (scr0.s.evt_sts.ack != CGX_EVT_CMD_RESP));
	if (timeout < 0)
		return -1;	
	/* wait for release */
	do {
		scr1.u = cgx_rd_scr1(cgx, lmac);
		mdelay(1);
	} while (scr1.s.own_status == CGX_OWN_FIRMWARE);

	set_ownership(cgx, lmac, CGX_OWN_NON_SECURE_FW);

	*rsp = scr0.u;

	/* clear ownership and ack */
	scr0.u = cgx_rd_scr0(cgx, lmac);
	scr0.s.evt_sts.ack = 0;
	cgx_wr_scr0(cgx, lmac, scr0.u);
	set_ownership(cgx, lmac, CGX_OWN_NONE);

	return 0;
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
}

int cgx_intf_get_link_sts(u8 cgx, u8 lmac, u8 *lnk_sts)
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

int cgx_intf_link_up_dwn(u8 cgx, u8 lmac, u8 up_dwn, u8 *lnk_sts)
{
	union cgx_scratchx0 scr0;
	int ret;

	if (up_dwn)
		ret = cgx_intf_req(cgx, lmac,
					 CGX_CMD_LINK_BRING_UP, &scr0.u);
	else
		ret = cgx_intf_req(cgx, lmac,
					 CGX_CMD_LINK_BRING_DOWN, &scr0.u);
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

	int ret;

	ret = cgx_intf_req(0, 0,
				CGX_CMD_INTF_SHUTDOWN, &scr0.u);
}




