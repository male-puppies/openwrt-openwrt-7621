#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=rtlmips
BOARDNAME:=Realtek MIPS 8198c Default
FEATURES:=squashfs gpio
CPU_TYPE:=1074k
MAINTAINER:=Roy <fengmushu@gmail.com>

#KERNEL_PATCHVER:=3.18
KERNEL_PATCHVER:=4.4

define Target/Description
	Build firmware images for Realtek RTL8198c based boards.
endef

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += #swconfig

$(eval $(call BuildTarget))
