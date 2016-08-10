
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qca-wifi
PKG_REV:=e50658f76e1fe597c2298ddf7d782c6d0d431f03
PKG_VERSION:=0.0.0

# include $(INCLUDE_DIR)/local-development.mk

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_REV).tar.gz
PKG_SOURCE_URL:=$(TOPDIR)/qca/src/qca-wifi
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)

# This value is custom to this package
# The first version will be used as a default and the other selectable
# in the menuconfig
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

# include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk

# These two functions are used to define options based on WLAN config
if_opt_set=       $(if $(filter $(1)=1,$(QCAWLAN_MAKEOPTS)),$(2),)
if_opt_clear=     $(if $(filter $(1)=0,$(QCAWLAN_MAKEOPTS)),$(2),)

# Use the function below to add driver opts depending on menuconfig values
append_if_notnull=QCAWLAN_MAKEOPTS+=$(if $(call qstrip,$(1)),$(2),$(3))

#########################################################
############ WLAN DRIVER BUILD CONFIGURATION ############
#########################################################

QCAWLAN_MODULE_LIST:=

ifneq ($(BUILD_VARIANT),)
QCAWLAN_MAKEOPTS:=$(shell cat $(PKG_SOURCE_URL)/os/linux/configs/config.wlan.$(subst -,.,$(BUILD_VARIANT)))
endif

ifeq ($(ARCH),mips)
HAL_TARGET:=mipsisa32-be-elf
QCAWLAN_MAKEOPTS+=BIG_ENDIAN_HOST=1
endif
ifeq ($(ARCH),arm)
HAL_TARGET:=armv4-le-elf
QCAWLAN_MAKEOPTS+=BUILD_ARM=y
QCAWLAN_MAKEOPTS+=BIG_ENDIAN_HOST=0
endif
ifeq ($(ARCH_PACKAGES),ipq806x)
QCAWLAN_MAKEOPTS+=BUILD_ARM_MSM=y
QCAWLAN_MAKEOPTS+=QCA_NSS_PLATFORM=1
QCAWLAN_MAKEOPTS+=PERE_IP_HDR_ALIGNMENT_WAR=1
endif
ifeq ($(ARCH_PACKAGES),ar71xx)
QCAWLAN_MAKEOPTS+= \
	BUS=dual \
	AH_CAL_IN_DRAM_FROM_HOST=1 \
	ATH_CAL_FROM_FILE_11AC_PCIE="ath10k/QCA988X/hw2.0/board.bin" \
	AH_CAL_RADIOS_AHB=1 \
	AH_CAL_RADIOS_PCI=1 \
	AH_SUPPORT_SCORPION=1
endif

QCAWLAN_MAKEOPTS+= \
	TOOLPATH= \
	TOOLPREFIX=$(TARGET_CROSS) \
	NO_SIMPLE_CONFIG=1 \
	USE_PLATFORM_FRAMEWORK=1 \
	TARGET=$(HAL_TARGET) \
	ARCH=$(LINUX_KARCH) \
	KERNELARCH=$(LINUX_KARCH) \
	KERNELVER=$(LINUX_VERSION) \
	FIRMWARE_REL_PATH=offload \
	PERF_PWR_OFFLOAD_DIR_PATH=$(PKG_BUILD_DIR)/offload \
	QCA_NSS_INCLUDE=$(STAGING_DIR)/usr/include/qca-nss-drv \
	KBUILDPATH=$(LINUX_SRC_DIR) \
	KERNELPATH=$(LINUX_DIR)

MAKEARCH:=$(MAKE) ARCH=$(LINUX_KARCH) CROSS_COMPILE=$(TARGET_CROSS)

# Module list
# This list is filled dynamically based on the WLAN configuration
# It depends on the content of the wlan config file (.profile)
QCAWLAN_MODULE_LIST+=$(PKG_BUILD_DIR)/asf/asf.$(LINUX_KMOD_SUFFIX)
QCAWLAN_MODULE_LIST+=$(PKG_BUILD_DIR)/adf/adf.$(LINUX_KMOD_SUFFIX)
QCAWLAN_MODULE_LIST+=$(PKG_BUILD_DIR)/os/linux/ath_hal/ath_hal.$(LINUX_KMOD_SUFFIX)
QCAWLAN_MODULE_LIST+=$(PKG_BUILD_DIR)/lmac/ratectrl/ath_rate_atheros.$(LINUX_KMOD_SUFFIX)
QCAWLAN_MODULE_LIST+=$(strip $(call if_opt_set, ATH_SUPPORT_DFS, \
	$(PKG_BUILD_DIR)/lmac/dfs/ath_dfs.$(LINUX_KMOD_SUFFIX)))
QCAWLAN_MODULE_LIST+=$(strip $(call if_opt_set, ATH_SUPPORT_SPECTRAL, \
	$(PKG_BUILD_DIR)/lmac/spectral/ath_spectral.$(LINUX_KMOD_SUFFIX)))
QCAWLAN_MODULE_LIST+=$(strip $(call if_opt_set, ATH_SUPPORT_TX99, \
	$(PKG_BUILD_DIR)/lmac/tx99/hst_tx99.$(LINUX_KMOD_SUFFIX)))
QCAWLAN_MODULE_LIST+=$(PKG_BUILD_DIR)/lmac/ath_dev/ath_dev.$(LINUX_KMOD_SUFFIX)
QCAWLAN_MODULE_LIST+=$(PKG_BUILD_DIR)/umac/umac.$(LINUX_KMOD_SUFFIX)
QCAWLAN_MODULE_LIST+=$(PKG_BUILD_DIR)/lmac/ath_pktlog/ath_pktlog.$(LINUX_KMOD_SUFFIX)

QCAWLAN_MODULE_LIST+=$(strip $(call if_opt_set, UNIFIED_SMARTANTENNA, \
	$(PKG_BUILD_DIR)/smartantenna/smart_antenna.$(LINUX_KMOD_SUFFIX)))
QCAWLAN_MODULE_LIST+=$(strip $(call if_opt_set, ATH_SW_WOW_SUPPORT, \
	$(PKG_BUILD_DIR)/wow/sw_wow.$(LINUX_KMOD_SUFFIX)))

#########################################################
##################### CONFIG RULES ######################
#########################################################

define KernelPackage/qca-wifi/Default
  SECTION:=QCA
  CATEGORY:=QCA Proprietary software
  URL:=http://www.qca.qualcomm.com
  MAINTAINER:=Qualcomm Atheros
  TITLE:=QCA Wireless LAN Driver
  DEPENDS:=+libgcc +libc +wireless-tools +librt @(!(PACKAGE_kmod-ath&&PACKAGE_kmod-mac80211))
  FILES:= $(strip $(QCAWLAN_MODULE_LIST))
  AUTOLOAD:=$(call AutoLoad,33,$(notdir $(basename $(strip $(QCAWLAN_MODULE_LIST)))))
  PROVIDES:=kmod-qca-wifi
  MENU:=1
endef

$(eval $(call RequireCommand,uuencode, \
	Please install sharutils which provides uuencode required for build \
))

define KernelPackage/qca-wifi/Description/Default
  This package contains a driver for QCA WLAN chipsets
  It also installs the QCA WLAN tools, such as:
  - wlanconfig
  - athstats
  - pktlogconf
  - ...
endef

define KernelPackage/qca-wifi/config/Default
$(call Package/$(PKG_NAME)/override_source_path,kmod-$(PKG_NAME)$(1))
$(call Package/$(PKG_NAME)/override_version,kmod-$(PKG_NAME)$(1),$(PKG_SUPPORTED_VERSION))
endef

#########################################################
################### INSTALLDEV RULES ####################
#########################################################

QCAWLAN_HEADERS:= \
   adf/include/os/adf_os_types.h \
   asf/include/asf_amem.h \
   offload/include/a_types.h \
   offload/include/ol_params.h \
   include/athdefs.h \
   include/wal_dbg_stats.h \
   offload/include/ol_txrx_stats.h \
   include/ieee80211_defines.h \
   include/_ieee80211.h \
   include/ieee80211.h \
   include/ieee80211_wnm_proto.h \
   include/ieee80211_band_steering_api.h \
   include/if_upperproto.h \
   include/compat.h \
   include/wlan_opts.h \
   include/sys/queue.h \
   include/ieee80211_phytype.h \
   include/ieee80211_rrm.h \
   hal/linux/ah_osdep.h \
   os/linux/include/ath_ald_external.h \
   os/linux/include/ieee80211_external.h \
   os/linux/include/ieee80211_ioctl.h \
   os/linux/include/ieee80211_ev.h \
   os/linux/include/ieee80211_wpc.h \
   os/linux/tools/athrs_ctrl.h \
   lmac/ath_dev/if_athioctl.h \
   lmac/dfs/dfs_ioctl.h \
   lmac/spectral/spectral_ioctl.h \
   umac/include/ieee80211_aow_shared.h \
   include/acfg_wsupp_types.h \
   include/acfg_wsupp_api.h \
   include/acfg_misc.h \
   include/acfg_event_types.h \
   include/acfg_event.h \
   include/acfg_drv_if.h \
   include/acfg_api_types.h \
   include/acfg_api.h \
   include/appbr_if.h \
   include/appbr_types.h \
   include/bypass_types.h


define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include
	$(CP) $(foreach header,$(QCAWLAN_HEADERS),$(PKG_BUILD_DIR)/$(header)) $(STAGING_DIR)/usr/include
endef


#########################################################
################# BUILD/INSTALL RULES ###################
#########################################################

QCAWLAN_TOOL_LIST:= 80211stats athstats athstatsclr apstats pktlogconf pktlogdump wifitool wlanconfig
QCAWLAN_TOOL_LIST+= $(call if_opt_set, ATH_SUPPORT_DFS, radartool)
QCAWLAN_TOOL_LIST+= $(call if_opt_set, ATH_SUPPORT_SPECTRAL, spectraltool)
QCAWLAN_TOOL_LIST+= $(call if_opt_set, ATH_SUPPORT_IBSS_PRIVATE_SECURITY, athadhoc)
QCAWLAN_TOOL_LIST+= $(call if_opt_set, ATH_SUPPORT_AOW, mplay aowstatlog)
QCAWLAN_TOOL_LIST+= $(call if_opt_set, ATH_SUPPORT_TX99, tx99tool)
QCAWLAN_TOOL_LIST+= $(call if_opt_set, DEBUG_TOOLS, dumpregs reg)

Hooks/Prepare/Pre += qca-wifi_update_patches_version
define qca-wifi_update_patches_version

	[ ! -h patches ] || rm patches
	[ ! -d patches-$(PKG_BRANCH) ] || \
		ln -s patches-$(PKG_BRANCH) patches
endef

TARGET_LDFLAGS += -ltlvutil -ltlvtemplate -L$(STAGING_DIR)/usr/lib/wifi
FWINCLUDES := $(PKG_BUILD_DIR)/fwcommon
FWBIN := $(PKG_BUILD_DIR)/fwbin
define Build/Compile
	@echo "Saving the driver configuration flags to $(PKG_BUILD_DIR)/lsdk_flags"
	echo $(QCAWLAN_MAKEOPTS) | sed 's, ,\n,g' > $(PKG_BUILD_DIR)/lsdk_flags
	mkdir -p $(PKG_BUILD_DIR)/root/usr/www/cgi-bin
	mkdir -p $(PKG_BUILD_DIR)/root/sbin
	cp -af $(FWINCLUDES)/* $(PKG_BUILD_DIR)/include/ 
	cp -af $(FWBIN)/* $(PKG_BUILD_DIR)/include/
	($(foreach var,$(strip $(QCAWLAN_MAKEOPTS)),export $(var);) \
	    $(MAKE) -C $(PKG_BUILD_DIR)/hal/linux TARGET=$(HAL_TARGET))
	($(foreach var,$(strip $(QCAWLAN_MAKEOPTS)),export $(var);) \
	    $(MAKE) -C $(PKG_BUILD_DIR)/hal/linux TARGET=$(HAL_TARGET) release)
	($(foreach var,$(strip $(QCAWLAN_MAKEOPTS)),export $(var);) \
	    $(MAKEARCH) -C $(PKG_BUILD_DIR)/os/linux TARGET=$(HAL_TARGET))
	($(foreach var,$(strip $(QCAWLAN_MAKEOPTS)),export $(var);) \
	    $(MAKEARCH) -C $(PKG_BUILD_DIR)/os/linux/tools \
	        INSTALL_ROOT=$(PKG_BUILD_DIR)/root ath_tools wlanconfig)
	($(foreach var,$(strip $(QCAWLAN_MAKEOPTS)),export $(var);) \
	    $(MAKEARCH) -C $(PKG_BUILD_DIR)/offload/tools/systemtools/testcmd_tlv \
		LIBS="$(TARGET_LDFLAGS)" \
	        -f makefile.linux WORKAREA=$(PKG_BUILD_DIR)/offload WLAN_TOP=$(PKG_BUILD_DIR))
	($(foreach var,$(strip $(QCAWLAN_MAKEOPTS)),export $(var);) \
	    $(MAKEARCH) -C $(PKG_BUILD_DIR)/offload/os/linux/tools/athdiag)
endef

define KernelPackage/qca-wifi/install/Default
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/lib/wifi
	$(CP) ./files/* $(1)/
	[ ! -d files-$(PKG_BRANCH) ] || \
		$(CP) ./files-$(PKG_BRANCH)/* $(1)/
	$(INSTALL_BIN) $(foreach tool,$(QCAWLAN_TOOL_LIST),$(PKG_BUILD_DIR)/os/linux/tools/$(tool)) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/offload/tools/systemtools/testcmd_tlv/Linux/athtestcmd $(1)/usr/sbin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/offload/tools/systemtools/testcmd_tlv/Linux/*.so $(1)/usr/lib/wifi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/offload/os/linux/tools/athdiag/athdiag $(1)/usr/sbin
endef

#########################################################
################# VARIANTS DEFINITION ###################
#########################################################

# Default variant (Akronite only)
define KernelPackage/qca-wifi-akronite-perf
$(call KernelPackage/qca-wifi/Default)
  VARIANT:=akronite-perf
  DEPENDS+=@TARGET_ipq806x +TARGET_ipq806x:kmod-qca-nss-drv
endef

define KernelPackage/qca-wifi-akronite-perf/Description
$(call define KernelPackage/qca-wifi/Description/Default)
endef

define KernelPackage/qca-wifi-akronite-perf/config
$(call KernelPackage/qca-wifi/config/Default,-akronite-perf)
endef

define KernelPackage/qca-wifi-akronite-perf/install
$(call KernelPackage/qca-wifi/install/Default,$(1))
endef
$(eval $(call KernelPackage,qca-wifi-akronite-perf))

# Perf variant
define KernelPackage/qca-wifi-perf
$(call KernelPackage/qca-wifi/Default)
  VARIANT:=perf
  DEPENDS+=@TARGET_ar71xx
endef

define KernelPackage/qca-wifi-perf/Description
$(call define KernelPackage/qca-wifi/Description/Default)
endef

define KernelPackage/qca-wifi-perf/config
$(call KernelPackage/qca-wifi/config/Default,-perf)
endef

define KernelPackage/qca-wifi-perf/install
$(call KernelPackage/qca-wifi/install/Default,$(1))
endef
$(eval $(call KernelPackage,qca-wifi-perf))
