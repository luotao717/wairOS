#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wifidog_ext
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/wifidog_ext
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=wifidog
  TITLE:=wifidog authenication function to remote server tools
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/wifidog_ext/install
	$(INSTALL_DIR) $(1)/bin $(1)/etc $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/bin/
	$(CP) ./files/etc/* $(1)/etc/
	$(INSTALL_BIN) ./files/wifidog_ext.init $(1)/etc/init.d/wifidog_ext
endef

$(eval $(call BuildPackage,wifidog_ext))
