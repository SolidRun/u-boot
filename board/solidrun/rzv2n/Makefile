#
# board/renesas/salvator-x/Makefile
#
# Copyright (C) 2015 Renesas Electronics Corporation
#
# SPDX-License-Identifier: GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	:= ../rcar-common/gen3-spl.o
else
obj-y	:= rzv2n-solidrun.o ../rcar-common/v2-common.o
obj-y 	+= ../rzg-common/
endif
