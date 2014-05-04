# Copyright (C) 2008 FON Wireless Ltd.
# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=raAted
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/raAted
  SECTION:=ralink
  CATEGORY:=Ralink
  TITLE:=Ralink RT2880 ATED
endef

define Package/raAted/description
	Ralink RT2880 ATED
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -std=gnu99" LDFLAGS="$(TARGET_LDFLAGS)" \
		$(MAKE) -C $(PKG_BUILD_DIR)
endef

define Package/raAted/install
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ated $(1)/sbin/
endef

$(eval $(call BuildPackage,raAted))

