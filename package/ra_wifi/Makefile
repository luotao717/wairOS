
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=RalinkAP
PKG_VERSION:=2.7.1.5
PKG_RELEASE:=0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/RalinkAP
    SUBMENU:=Wireless Drivers
    TITLE:=AP driver for Ralink SoC wireless chipsets
    FILES:=$(PKG_BUILD_DIR)/rt2860v2_ap/rt2860v2_ap.$(LINUX_KMOD_SUFFIX)
    AUTOLOAD:=$(call AutoLoad,50,rt2860v2_ap)
endef

define KernelPackage/RalinkAP/description
 This package contains a driver for Ralink SoC chipsets.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

#define KernelPackage/RalinkAP/config
#	source "$(SOURCE)/Config-AP.in"
#endef

EXTRA_KCONFIG:= \
	CONFIG_RALINK_MT7620=y \
	CONFIG_RT2860V2_AP=m \
	CONFIG_RT2860V2_AP_LED=y \
	CONFIG_RT2860V2_AP_RTMP_INTERNAL_TX_ALC=y \
	CONFIG_RT2860V2_AP_V24_DATA_STRUCTURE=y \
	CONFIG_RT2860V2_AP_MBSS=y \
	CONFIG_NEW_MBSSID_MODE=y \
	CONFIG_RT2860V2_AP_80211N_DRAFT3=y \
	CONFIG_RT2860V2_AP_WSC=y \
	CONFIG_RT2860V2_AP_WSC_V2=y \
	CONFIG_RT2860V2_AP_LLTD=y \
	CONFIG_RT2860V2_AP_COC=y \
	CONFIG_RT2860V2_AP_IDS=y \
	CONFIG_RT2860V2_AP_CARRIER=y 
	#CONFIG_RT2860V2_AP_DFS=y \
	#CONFIG_RT2860V2_AP_IGMP_SNOOP=y 
	#CONFIG_RT2860V2_AP_APCLI=y \
	CONFIG_RT2860V2_AP_MAC_REPEATER=y \
	CONFIG_RT2860V2_AP_NETIF_BLOCK=y \
	CONFIG_RT2860V2_AP_WDS=y \
	CONFIG_RT2860V2_AP_DLS=y \
	CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION=y \

#EXTRA_CFLAGS+= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	LINUXINCLUDE="-I$(LINUX_DIR)/include -I$(LINUX_DIR)/arch/mips/include -include linux/autoconf.h \
				 -I$(PKG_BUILD_DIR)/rt2860v2/include -I$(PKG_BUILD_DIR)/rt2860v2/ate/include" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(warning hello)
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

#define Build/Compile
#	$(MAKE) -C "$(PKG_BUILD_DIR)" \
#		$(MAKE_OPTS)
#endef


#define Build/Compile
#	$(MAKE) -C "$(PKG_BUILD_DIR)" \
                CC="$(TARGET_CC)" \
                LD="$(TARGET_CROSS)ld" \
                ARCH="$(LINUX_KARCH)" \
                PWD="$(PKG_BUILD_DIR)/Module" \
                KERNDIR="$(LINUX_DIR)" \
                PATCHLEVEL="$(patsubst 2.%,%,$(KERNEL))"
#endef

define Package/RalinkAP-util
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= (utility)
endef

define Package/RalinkAP-util/conffiles
/etc/Wireless/RT2860AP/RT2860AP.dat
endef

define Package/RalinkAP-util/description
This package contains the RalinkAP command-line utility.
endef

define Package/RalinkAP-util/install
	$(INSTALL_DIR) $(1)/etc/Wireless/RT2860AP
	$(CP) ./files/MT7620_AP_2T2R-4L_V13.BIN $(1)/etc/Wireless/RT2860AP/
	$(CP) ./files/RT2860AP.dat $(1)/etc/Wireless/RT2860AP/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) ./files/wireless.conf $(1)/etc/config/wireless
	$(INSTALL_DIR) $(1)/lib/wifi/
	$(INSTALL_BIN) ./files/RT5392AP.sh $(1)/lib/wifi/
endef

$(eval $(call BuildPackage,RalinkAP-util))
$(eval $(call KernelPackage,RalinkAP))
