#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autoUpdate
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/autoUpdate
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=tools for auto update firmare
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/autoUpdate/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/autoUpdate $(1)/usr/sbin/autoUpdate
endef

$(eval $(call BuildPackage,autoUpdate))
