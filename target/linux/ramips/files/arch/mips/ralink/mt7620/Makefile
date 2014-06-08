#
# Makefile for the Ralink mt7620 SoC specific parts of the kernel
#
# Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.

obj-y	:= irq.o setup.o devices.o mt7620.o clock.o

obj-$(CONFIG_EARLY_PRINTK)		+= early_printk.o

obj-$(CONFIG_MT7620_MACH_WIFI_ROUTER)	+= mach-wifi-router.o

obj-$(CONFIG_MT7620_MACH_WIFI_ROUTER_POE)	+= mach-wifi-router-poe.o
