
include $(TOPDIR)/rules.mk

PKG_NAME:=qca-hostap
PKG_RELEASE:=4a396d68cab17a3508e33878df1f8bad3603798f
PKG_VERSION:=0.0.0

# PKG_SOURCE:=$(TOPDIR)/qca/src/qca-hostap
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.gz
PKG_SOURCE_URL:=$(TOPDIR)/qca/src/qca-hostap
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_RELEASE)

LOCAL_TYPE=$(strip \
		$(if $(findstring supplicant,$(BUILD_VARIANT)),supplicant, \
		hostap \
	))
LOCAL_VARIANT=$(patsubst supplicant-%,%,$(BUILD_VARIANT))

PKG_BUILD_DEPENDS:=qca-wifi
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

# include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk

define Package/qca-hostap/Default
  SECTION:=QCA
  CATEGORY:=QCA Proprietary software
  URL:=http://www.qca.qualcomm.com
  MAINTAINER:=Qualcomm Atheros
endef

define Package/qca-hostap
  $(call Package/qca-hostap/Default)
  TITLE:=Hostap authenticator
  DEPENDS:=kmod-qca-wifi @(!(PACKAGE_hostapd))
  VARIANT:=default
endef

define Package/qca-hostap/description
 This package contains the IEEE 802.1x/WPA/EAP/RADIUS Authenticator.
endef

define Package/qca-hostapd-cli
  $(call Package/qca-hostap/Default)
  DEPENDS:=qca-hostap
  TITLE:=Hostap authenticator command line interface
endef

define Package/qca-hostapd-cli/description
 This package contains a command line utility to control the
 IEEE 802.1x/WPA/EAP/RADIUS Authenticator.
endef

define Package/qca-wpa-supplicant
  $(call Package/qca-hostap/Default)
  TITLE:=WPA Supplicant
  DEPENDS:=kmod-qca-wifi @(!(PACKAGE_wpa-supplicant))
  VARIANT:=supplicant-default
endef

define Package/qca-wpa-supplicant/Description
  WPA Supplicant
endef

define Package/qca-wpa-supplicant-macsec
  $(call Package/qca-wpa-supplicant)
  TITLE:=WPA Supplicant (MACSEC)
  DEPENDS+=+TARGET_ipq806x:kmod-qca-nss-macsec
  VARIANT:=supplicant-macsec
endef

define Package/qca-wpa-supplicant-macsec/Description
  WPA Supplicant with MACSEC support for IPQ8066.
endef

define Package/qca-wpa-cli
  $(call Package/qca-hostap/Default)
  DEPENDS:=@PACKAGE_qca-wpa-supplicant||PACKAGE_qca-wpa-supplicant-macsec
  TITLE:=WPA Supplicant command line interface
endef

define Package/qca-wpa-cli/Description
  WPA Supplicant control utility
endef


ifneq ($(wildcard $(PKG_BUILD_DIR)/.config_*),$(subst .configured_,.config_,$(STAMP_CONFIGURED)))
  define Build/Configure/rebuild
	$(FIND) $(PKG_BUILD_DIR) -name \*.o -or -name \*.a | $(XARGS) rm -f
	rm -f $(PKG_BUILD_DIR)/hostapd/hostapd
	rm -f $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant
	rm -f $(PKG_BUILD_DIR)/.config_*
	touch $(subst .configured_,.config_,$(STAMP_CONFIGURED))
  endef
endif

define Package/qca-hostap/config
$(call Package/$(PKG_NAME)/override_source_path,$(PKG_NAME))
$(call Package/$(PKG_NAME)/override_version,$(PKG_NAME),$(PKG_SUPPORTED_VERSION))
endef

define Build/Configure
	$(Build/Configure/rebuild)
	$(if $(wildcard ./files/hostapd-$(LOCAL_VARIANT).config), \
		$(CP) ./files/hostapd-$(LOCAL_VARIANT).config $(PKG_BUILD_DIR)/hostapd/.config \
	)
	$(CP) ./files/wpa_supplicant-$(LOCAL_VARIANT).config $(PKG_BUILD_DIR)/wpa_supplicant/.config
endef

TARGET_CPPFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/qca-nss-macsec

ifneq ($(LOCAL_TYPE),hostap)
  ifeq ($(LOCAL_VARIANT),macsec)
    TARGET_LDFLAGS += -lfal -L$(STAGING_DIR)/usr/lib
  endif
endif

define Build/RunMake
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR)/$(1) \
	$(TARGET_CONFIGURE_OPTS) \
	LIBS="$(TARGET_LDFLAGS)" \
	$(2)
endef

define Build/Compile/qca-hostap
	$(call Build/RunMake,hostapd, \
		hostapd hostapd_cli \
	)
endef

define Build/Compile/qca-supplicant
	$(call Build/RunMake,wpa_supplicant, \
		wpa_cli wpa_supplicant \
	)
endef

define Build/Compile
	$(Build/Compile/qca-$(LOCAL_TYPE))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/common/wpa_ctrl.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/common/ieee802_11_defs.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/includes.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/build_config.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/eloop.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/os.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/common.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/wpa_debug.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/wpabuf.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/linux_ioctl.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/linux_wext.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/netlink.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/priv_netlink.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/common/wpa_ctrl.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/netlink.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/eloop.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/wpa_debug.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/common.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/os_unix.o $(1)/usr/lib/
endef


define Install/qca-hostap
	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_DATA) ./files/hostapd.sh $(1)/lib/wifi/hostapd.sh
	$(INSTALL_DIR) $(1)/usr/sbin
endef

define Install/qca-wpa-supplicant
	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_DATA) ./files/wpa_supplicant.sh $(1)/lib/wifi/wpa_supplicant.sh
	$(INSTALL_DIR) $(1)/usr/sbin
endef

define Package/qca-hostap/install
	$(call Install/qca-hostap,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd $(1)/usr/sbin/
endef

ifneq ($(LOCAL_TYPE),supplicant)
  define Package/qca-hostapd-cli/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/hotplug.d/button
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd_cli $(1)/usr/sbin/
	$(INSTALL_DATA) ./files/wps-hotplug.sh $(1)/etc/hotplug.d/button/50-wps
	$(INSTALL_DATA) ./files/wps-reset-hotplug.sh $(1)/etc/hotplug.d/button/51-wps-reset

	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_BIN) ./files/wps-hostapd-update-uci.sh \
			$(1)/lib/wifi/wps-hostapd-update-uci
  endef
endif

define Package/qca-wpa-supplicant/install
	$(call Install/qca-wpa-supplicant,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant $(1)/usr/sbin/
endef

define Package/qca-wpa-supplicant-macsec/install
	$(call Package/qca-wpa-supplicant/install,$(1))
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) ./files/macsec.init $(1)/etc/init.d/macsec
	$(INSTALL_BIN) ./files/macsec.config $(1)/etc/config/macsec
endef

ifneq ($(LOCAL_TYPE),hostap)
  define Package/qca-wpa-cli/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_cli $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/button
	$(INSTALL_DATA) ./files/wps-supplicant-hotplug.sh \
			$(1)/etc/hotplug.d/button/52-wps-supplicant

	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_BIN) ./files/wps-supplicant-update-uci.sh \
			$(1)/lib/wifi/wps-supplicant-update-uci
  endef
endif

$(eval $(call BuildPackage,qca-hostap))
$(eval $(call BuildPackage,qca-wpa-supplicant))
$(eval $(call BuildPackage,qca-wpa-supplicant-macsec))
$(eval $(call BuildPackage,qca-hostapd-cli))
$(eval $(call BuildPackage,qca-wpa-cli))
