#/* SPDX-License-Identifier: BSD-2-Clause-Patent
# * https://spdx.org/licenses
# *
# * Copyright (C) 2024 Marvell
# *
# */

obj-y	:= board.o smc.o soc-utils.o
obj-$(CONFIG_OF_LIBFDT) += board-fdt.o
obj-${CONFIG_TARGET_CN20K_A} += switch.o
