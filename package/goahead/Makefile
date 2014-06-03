#
# Copyright (C) 2010-2012 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=goahead
PKG_RELEASE:=2.5

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/goahead/default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=goahead, lightweight HTTP server
  MAINTAINER:=kony luo <kony_luo@163.com>
endef

define Package/goahead
  $(Package/goahead/default)
  MENU:=1
endef

define Package/goahead/description
 goahead islightweight HTTP server,support asp
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/goahead/install
	$(INSTALL_DIR) $(1)/wwwg
	$(CP) ./files/web/* $(1)/wwwg/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/main-src/goahead $(1)/usr/sbin/goahead
endef

$(eval $(call BuildPackage,goahead))

