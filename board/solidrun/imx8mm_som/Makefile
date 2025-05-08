#
# Copyright 2018 NXP
# Copyright 2025 Josua Mayer
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mm_som.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_nxpevk.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_micron_MT53D512M32D2.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_samsung_K4F6E3S4HM-SGLC.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_samsung_K4F8E3S4HD-MGCL.o

endif
