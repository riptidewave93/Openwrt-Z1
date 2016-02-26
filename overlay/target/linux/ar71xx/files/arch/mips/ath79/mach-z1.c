/*
 *  Cisco Meraki Z1 board support
 *
 *  Copyright (C) 2016 Chris Blake <chrisrblake93@gmail.com>
 *
 *  Based on Cisco Meraki GPL Release r23-20150601 Z1 Device Config
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/platform/ar934x_nfc.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/leds-nu801.h>
#include <linux/firmware.h>
#include <linux/pci.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "pci.h"
#include "pci-ath9k-fixup.h"
#include "machtypes.h"

#define Z1_GPIO_LED_POWER_ORANGE    17

#define Z1_GPIO_NU801_CKI        14
#define Z1_GPIO_NU801_SDI        15

#define Z1_GPIO_XLNA0		18
#define Z1_GPIO_XLNA1		19

#define Z1_GPIO_BTN_RESET    12
#define Z1_KEYS_POLL_INTERVAL    20  /* msecs */
#define Z1_KEYS_DEBOUNCE_INTERVAL  (3 * Z1_KEYS_POLL_INTERVAL)

#define Z1_ETH_SWITCH_PHY 0

static struct gpio_led Z1_leds_gpio[] __initdata = {
	{
		.name = "z1:orange:power",
		.gpio = Z1_GPIO_LED_POWER_ORANGE,
		.active_low  = 1,
	},
};

static struct gpio_keys_button Z1_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = Z1_KEYS_DEBOUNCE_INTERVAL,
		.gpio    = Z1_GPIO_BTN_RESET,
		.active_low  = 1,
	},
};

static struct led_nu801_template tricolor_led_template = {
	.device_name = "z1",
	.name = "tricolor",
	.num_leds = 1,
	.cki = Z1_GPIO_NU801_CKI,
	.sdi = Z1_GPIO_NU801_SDI,
	.lei = -1,
	.ndelay = 500,
	.init_brightness = {
		LED_OFF,
		LED_OFF,
		LED_OFF,
	},
	.default_trigger = "none",
	.led_colors = { "blue", "green", "red" },
};

static struct led_nu801_platform_data tricolor_led_data = {
	.num_controllers = 1,
	.template = &tricolor_led_template,
};

static struct platform_device tricolor_leds = {
	.name = "leds-nu801",
	.id = -1,
	.dev.platform_data = &tricolor_led_data,
};

static struct ar8327_pad_cfg z1_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data z1_ar8327_data = {
	.pad0_cfg = &z1_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info z1_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = Z1_ETH_SWITCH_PHY,
		.platform_data = &z1_ar8327_data,
	},
};

#define EEPROM_CALDATA "pci_wmac0.eeprom"

static struct ath9k_platform_data z1_wmac0_data = {
	.led_pin = -1,
	.eeprom_name = EEPROM_CALDATA,
};

static int z1_pci_plat_dev_init(struct pci_dev *dev)
{
	switch (PCI_SLOT(dev->devfn)) {
	case 0:
		dev->dev.platform_data = &z1_wmac0_data;
		break;
	}

	return 0;
}

static void z1_pci_init(void)
{
	/* The internal PCIe card is very tricky to enable proberly.
	 * We have to get the caldata from the NAND, which is only
	 * accessible once the rootfs is available. That's why we
	 *  0. request the caldata file 'pci_wmac0.eeprom' via
	 *     request_firmware_nowait
	 *  --- wait for the system to be ready to handle the request ---
	 *  1. reentry into z1_fw_cb,with the loaded caldata
	 *  2. perform the PCI fixup.
	 *  3. remove old pci device and rescan the pci bus to find the
	 *     initialized device (productid will update)
	 */

	ath79_pci_set_plat_dev_init(z1_pci_plat_dev_init);
	ath79_register_pci();
}

static void z1_fw_cb(const struct firmware *fw, void *ctx)
{
	struct platform_device *vdev = (struct platform_device *) ctx;

	if (fw) {
		struct pci_dev *pdev;

		pci_lock_rescan_remove();
		pdev = pci_get_device(0x168c, 0xff1c, NULL);
		if (pdev) {
			struct pci_bus *bus = pdev->bus;
			u16 *cal_data = kmemdup(fw->data, fw->size,
						GFP_KERNEL);

			pci_enable_ath9k_fixup(0, cal_data);
			pci_stop_and_remove_bus_device(pdev);

			/* the device should come back with the proper
			 * ProductId. But we have to initiate a rescan.
			 */
			pci_rescan_bus(bus);
		} else {
			printk(KERN_ERR "Failed to find uninitialized pci wmac0\n");
		}
		pci_unlock_rescan_remove();

		release_firmware(fw);
	} else {
		printk(KERN_ERR "caldata request for '" EEPROM_CALDATA "' timed out");
	}
	platform_device_unregister(vdev);
}

static int z1_wmac0_init(void)
{
	struct platform_device *vdev;
	int err = 0;

	/* create a virtual device for the eeprom loader. This is necessary
	 * because request_firmware_nowait needs a proper device for
	 * accounting. In theory, the pci device could be used as well.
	 * However we don't know the state of the device at this point.
	 */
	vdev = platform_device_register_simple("z1_caldata", -1, NULL, 0);
	if (IS_ERR(vdev)) {
		err = PTR_ERR(vdev);
		printk(KERN_ERR "failed to register vdev. (%d).\n", err);
		return err;
	}

	err = request_firmware_nowait(THIS_MODULE, true, EEPROM_CALDATA,
				      &vdev->dev, GFP_KERNEL, vdev, z1_fw_cb);
	if (err) {
		printk(KERN_ERR "failed to request caldata (%d).\n", err);
		platform_device_unregister(vdev);
	}
	return err;
}

static void __init z1_setup(void)
{
	/* NAND */
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_SOFT_BCH);
	ath79_register_nfc();

	/* Eth Config */
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	/* MDIO Interface */
	ath79_register_mdio(1, 0x0);
	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(z1_mdio0_info,
				    ARRAY_SIZE(z1_mdio0_info));

	/* GMAC0 is connected to an AR8327 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(Z1_ETH_SWITCH_PHY);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	/* XLNA */
	ath79_wmac_set_ext_lna_gpio(0, Z1_GPIO_XLNA0);
	ath79_wmac_set_ext_lna_gpio(1, Z1_GPIO_XLNA1);

	/* LEDs and Buttons */
	platform_device_register(&tricolor_leds);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(Z1_leds_gpio),
				 Z1_leds_gpio);
	ath79_register_gpio_keys_polled(-1, Z1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(Z1_gpio_keys),
					Z1_gpio_keys);

	/* USB */
	ath79_register_usb();

	/* Wireless */
	ath79_register_wmac_simple();
	z1_pci_init();
}
MIPS_MACHINE(ATH79_MACH_Z1, "Z1", "Meraki Z1", z1_setup);

device_initcall(z1_wmac0_init);
