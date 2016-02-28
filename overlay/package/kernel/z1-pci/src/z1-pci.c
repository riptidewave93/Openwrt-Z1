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
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/firmware.h>
#include <linux/pci.h>

#include "../arch/mips/ath79/pci-ath9k-fixup.h"
#include "../arch/mips/ath79/dev-ap9x-pci.h"

#define EEPROM_CALDATA "pci_wmac0.eeprom"

static void z1_fw_cb(const struct firmware *fw, void *ctx)
{
	struct platform_device *vdev = (struct platform_device *) ctx;
	struct pci_bus *bus;
	struct pci_dev *pdev;
	struct ath9k_platform_data *pdata;
	int slot;

	if (!fw) {
		printk(KERN_ERR "no '" EEPROM_CALDATA "' received.\n");
		goto release;
	}

	pci_lock_rescan_remove();
	pdev = pci_get_device(0x168c, 0xff1c, NULL);
	if (!pdev) {
		printk(KERN_ERR "Failed to find uninitialized pci wmac0.\n");
		goto unlock;
	}

	bus = pdev->bus;
	slot = pcibios_plat_dev_init(pdev);
	pdata = ap9x_pci_get_wmac_data(slot);

	if (!pdata) {
		printk(KERN_ERR "No platform data\n");
		goto unlock;
	}

	if (fw->size > sizeof(pdata->eeprom_data)) {
		printk(KERN_ERR "loaded data is too big.\n");
		goto unlock;
	}

	memcpy(pdata->eeprom_data, fw->data, sizeof(pdata->eeprom_data));
	pdata->eeprom_name = NULL;

	pci_enable_ath9k_fixup(0, pdata->eeprom_data);
	pci_stop_and_remove_bus_device(pdev);
	/* the device should come back with the proper
	 * ProductId. But we have to initiate a rescan.
	 */

	print_hex_dump_bytes("PDAT:", DUMP_PREFIX_OFFSET, pdata, sizeof(*pdata));
	pci_rescan_bus(bus);

unlock:
	pci_unlock_rescan_remove();

release:
	release_firmware(fw);
	platform_device_unregister(vdev);
}

static int z1_wmac0_init(void)
{
	struct ath9k_platform_data *pdata = ap9x_pci_get_wmac_data(0);
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

static void z1_wmac0_exit(void)
{
	/* request_firmware_nowait will grab a instance of this module */
}

module_init(z1_wmac0_init);
module_exit(z1_wmac0_exit);
MODULE_LICENSE("GPL");

