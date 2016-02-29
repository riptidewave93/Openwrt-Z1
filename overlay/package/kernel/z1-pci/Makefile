#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=z1-pci
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define KernelPackage/z1-pci
  SUBMENU:=Other modules
  TITLE:=Cisco Meraki Z1 PCIe Wifi Support
  DEPENDS:=+kmod-ath9k
  FILES:=$(PKG_BUILD_DIR)/z1-pci.ko
  AUTOLOAD:=$(call AutoProbe,z1-pci)
  KCONFIG:=
endef

define KernelPackage/z1-pci/description
  Kernel module to support Cisco's Meraki Z1 PCIe Wifi.
endef

EXTRA_KCONFIG:= \
	CONFIG_Z1_PCI=m

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

$(eval $(call KernelPackage,z1-pci))
