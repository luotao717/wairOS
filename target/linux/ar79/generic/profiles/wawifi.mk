#
# Copyright (C) 2013-2014 wiair OS 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/WAFI_R1001
	NAME:=Atheros WAFI-R1001 reference board
#	PACKAGES:=kmod-usb-core kmod-usb2
endef

define Profile/WAFI_R1001/Description
	Package set optimized for the Atheros 9341 : WAFI-R1001 reference board for wawifi.
endef

$(eval $(call Profile,WAFI_R1001))
