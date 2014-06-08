#
# Copyright (C) 2013-2014 internet wifi 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/WIFI_ROUTER
	NAME:=Richerlink WIFI-ROUTER reference board
	PACKAGES:=kmod-usb-core kmod-usb2
endef

define Profile/WIFI_ROUTER/Description
	Package set optimized for the Ralink MT7620: WIFI-ROUTER reference board.
endef

$(eval $(call Profile,WIFI_ROUTER))

define Profile/WIFI_ROUTER_POE
	NAME:=Richerlink WIFI-ROUTER-POE reference board
endef

define Profile/WIFI_ROUTER_POE/Description
	Package set optimized for the Ralink MT7620: WIFI-ROUTER-POE reference board.
endef

$(eval $(call Profile,WIFI_ROUTER_POE))
