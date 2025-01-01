#
# Copyright 2018 NXP
# Copyright 2024 Josua Mayer <josua@solid-run.com>
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += board.o
ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef IMX8MN_LOW_DRIVE_MODE
obj-y += ddr4_timing_ld.o
else
obj-y += ddr4_timing.o
endif
else
obj-y += supercap.o
endif
