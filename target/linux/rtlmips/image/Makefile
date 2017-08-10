#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME := rtl-loader
PKG_BUILD_DIR := $(KDIR)/$(PKG_NAME)

LOAD_START_ADDR=0x80C00000
FLASH_OFFSET=40000

BOOT_OFFSET=0
LINUX_OFFSET=$(FLASH_OFFSET)
ROOTFS_OFFSET=300000

#devices
ifeq ($(SUBTARGET),generic)
include ./generic.mk
endif

define Build/Clean
	$(MAKE) -C $(PKG_NAME) clean
endef

define Image/Compile
	$(call Image/Prepare)
	$(MAKE) -C $(PKG_BUILD_DIR) \
    LINUX_DIR=$(LINUX_DIR) \
    TARGET_DIR=$(KDIR) \
    LOAD_START_ADDR=$(LOAD_START_ADDR) \
    FLASH_OFFSET=$(FLASH_OFFSET) \
    CROSS_COMPILE=$(TARGET_CROSS)
endef

define Image/Clean
	rm -fr $(PKG_BUILD_DIR)
endef

define Image/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -a $(PKG_NAME)/src/* $(PKG_BUILD_DIR)

	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-raw
	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-64k
	$(STAGING_DIR_HOST)/bin/padjffs2 $(KDIR)/root.squashfs-64k 64
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.squashfs of=$(KDIR)/root.squashfs-128k bs=128k conv=sync
endef

define Image/Build
	$(call Image/Compile)
	cp $(KDIR)/root.squashfs-64k $(BIN_DIR)/$(IMG_PREFIX)-rootfs.squashfs-64k
	cp $(KDIR)/root.squashfs-128k $(BIN_DIR)/$(IMG_PREFIX)-rootfs.squashfs-128k
	cp $(PKG_BUILD_DIR)/linux.bin $(BIN_DIR)/$(IMG_PREFIX)-kernel.bin
	
	mkdir -p $(KDIR_TMP)
	cp $(KDIR)/root.squashfs $(KDIR_TMP)/

	dd if=/dev/zero of=$(KDIR_TMP)/sysupgrade.bin bs=1k count=2816
	#0x2C0000(2816k)+0x40000=0x300000
	dd if=$(PKG_BUILD_DIR)/nfjrom \
	   of=$(KDIR_TMP)/sysupgrade.bin bs=64k conv=sync,notrunc,nocreat

	dd if=$(KDIR)/root.squashfs-128k \
	   of=$(KDIR_TMP)/sysupgrade.bin bs=1 seek=2883568 conv=sync,notrunc,nocreat
	   #‭(0x300000-0x40000=0x2C0000)2883584‬-16=(0x2BFFF0)2883568
	
	$(STAGING_DIR_HOST)/bin/cvimg linux-ro \
		$(KDIR_TMP)/sysupgrade.bin \
		$(KDIR)/sysupgrade.bin $(LOAD_START_ADDR) $(FLASH_OFFSET) $(CV_SIGNATURE)

	cp $(KDIR)/sysupgrade.bin $(BIN_DIR)/$(IMG_PREFIX)-sysupgrade.bin
endef

$(eval $(call BuildImage))