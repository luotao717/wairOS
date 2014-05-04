#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
# copyright wiairOS V1.0
# leo 2014.05
include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=ar79
BOARDNAME:=Atheros AR7xxx/AR9xxx
FEATURES:=squashfs jffs2 targz
CFLAGS:=-Os -pipe -mips32r2 -mtune=mips32r2 -fno-caller-saves
SUBTARGETS:=generic

LINUX_VERSION:=2.6.32.27

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug kmod-wdt-ath79 swconfig \
	kmod-ledtrig-default-on kmod-ledtrig-timer kmod-ledtrig-netdev \
 	uboot-envtools

define Target/Description
	Build firmware images for Atheros AR7xxx/AR9xxx based boards.
endef

$(eval $(call BuildTarget))

