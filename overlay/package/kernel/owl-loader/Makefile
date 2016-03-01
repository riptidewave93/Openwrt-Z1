#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=owl-loader
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/owl-loader
  SUBMENU:=Other modules
  TITLE:=Owl loader for Atheros PCIe Wifi support
  DEPENDS:=+kmod-ath9k
  FILES:=$(PKG_BUILD_DIR)/owl-loader.ko
  AUTOLOAD:=$(call AutoProbe,owl-loader)
  KCONFIG:=
endef

define KernelPackage/owl-loader/description
  Kernel module to initialize Owl Emulation Devices.
  This is necessary for the Cisco Meraki Z1.
endef

EXTRA_KCONFIG:= \
	CONFIG_OWL_LOADER=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,owl-loader))
