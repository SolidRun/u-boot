#
# Copyright 2019 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mp_solidrun.o mmc.o ../common/tlv_data.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_1gb_samsung_micron.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_patch_2gb_samsung.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_3gb_micron.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_4gb_samsung_micron.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_patch_8gb_micron.o
endif
