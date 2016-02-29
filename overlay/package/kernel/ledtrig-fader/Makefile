#
# Copyright (C) 2016 Chris Blake <chrisrblake93@gmail.com>
# Based on Cisco Meraki GPL Drop linux-3.4-r23-20150601
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=LEDTRIG-FADER
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ledtrig-fader
  SUBMENU:=LED modules
  TITLE:=LED Fading Trigger
  FILES:=$(PKG_BUILD_DIR)/ledtrig-fader.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-fader)
  KCONFIG:=
endef

define KernelPackage/ledtrig-fader/description
  Kernel module to support LED fade trigger. LEDs configured to use the fade trigger will
  fade gradually between each of them. For example if you have three leds (RGB) each with
  variable brightness, if you add all three to the fader trigger they will gradually fade
  from R -> G -> B -> R in a loop.If you add a single LED to the fader, it will fade in
  and out similar to the LED breathing on Macs.
  If unsure, say N.
endef

EXTRA_KCONFIG:= \
	CONFIG_LEDS_TRIGGER_FADER=m

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

$(eval $(call KernelPackage,ledtrig-fader))
