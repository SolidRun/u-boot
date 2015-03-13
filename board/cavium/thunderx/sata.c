/** @file
#
#  Copyright (c) 2014, Cavium Inc. All rights reserved.<BR>
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

#include <common.h>
#include <ahci.h>
#include <scsi.h>
#include <sata.h>
#include <asm/io.h>
#include <cavm-csrs-sata.h>
#include <cavm-csrs-mio_boot.h>


uintptr_t sata_baseaddress(int dev)
{
	union satax_uctl_ctl uctl_ctl;

	uctl_ctl.u = readq(SATAX_UCTL_CTL(dev));

	if (!uctl_ctl.s.sata_uahc_rst &&
		!uctl_ctl.s.sata_uctl_rst &&
		uctl_ctl.s.a_clk_en) {
		return SATAX_PF_BAR0(dev);
	} else {
		return 0;
	}
}
