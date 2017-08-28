#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/RTB
  NAME:=Router Boards package
  PACKAGES:=
endef

define Profile/RTB/Description
	Realtek SOC,Package RTB mode support
endef

$(eval $(call Profile,RTB))
