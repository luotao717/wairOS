#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ramips
BOARDNAME:=Ralink MT762X
FEATURES:=squashfs jffs2 targz
CFLAGS:=-Os -pipe -mips32r2 -mtune=mips32r2 -fno-caller-saves
SUBTARGETS:=generic

LINUX_VERSION:=2.6.32.27

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ledtrig-default-on kmod-ledtrig-timer kmod-ledtrig-netdev \
 	uboot-envtools

define Target/Description
	Build firmware images for Ralink MT7620 based boards.
endef

$(eval $(call BuildTarget))
