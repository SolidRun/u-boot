# SPDX-License-Identifier: GPL-2.0+
# (C) Copyright 2014, Cavium Inc.
#
#

obj-y	:= thunderx.o atf.o
obj-$(CONFIG_OF_LIBFDT) += fdt.o psci.o powerburn_dp.o

ccflags-y += -Icsr/include

obj-$(CONFIG_SATA_AHCI) += sata.o
obj-$(CONFIG_PCI) += pcie.o
obj-$(CONFIG_CMD_VNIC) += cmd_vnic.o
