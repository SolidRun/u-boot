/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __RZG_COMMON_H__
#define __RZG_COMMON_H__

#include "../rzg-common/rzg2l-regs.h"

enum carrier_boards
{
	CARRIER_UNRECOGNIZED = 0,
	CARRIER_HB_MATE,
	CARRIER_HB_RIPPLE,
	CARRIER_HB_PULSE,
	CARRIER_HB_PRO,
	CARRIER_HB_IIOT,
	CARRIER_HB_EU205,
};

enum vbus_out_type
{
	VBUS_OUT_PP = 0,
	VBUS_OUT_OD,
};

enum sd_emmc_select_type
{
	SDIO_SELECT_EMMC = 0,
	SDIO_SELECT_SD,
};


#define CARRIER_SKU_MAX_SIZE 25

int rzg_get_carrier(void);
void rzg_sd_emmc_init(void);
void rzg_set_bootsource_env(void);
void rzg_carrier_usb_init(int carrier);

#endif //__RZG_COMMON_H__
