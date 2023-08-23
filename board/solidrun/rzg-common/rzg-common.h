/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __RZG_COMMON_H__
#define __RZG_COMMON_H__

enum carrier_boards
{
	CARRIER_UNRECOGNIZED = 0,
	CARRIER_HB_MATE,
	CARRIER_HB_RIPPLE,
	CARRIER_HB_PULSE,
	CARRIER_HB_EXTENDED,
};

enum vbus_out_type
{
	VBUS_OUT_PP = 0,
	VBUS_OUT_OD,
};

#define CARRIER_SKU_MAX_SIZE 25

int get_carrier(void);

void rzg_sd_emmc_init(void);

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_SYSTEM_SETUP)
int rzg_preboot_sd_emmc_setup(void *blob, struct bd_info *bd);
#endif

#endif //__RZG_COMMON_H__
