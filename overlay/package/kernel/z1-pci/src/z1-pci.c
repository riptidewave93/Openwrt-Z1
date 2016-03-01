/*
 *  Initialize Cisco Meraki Z1's PCIe AR9280 Wifi
 *
 *  Copyright (C) 2016 Christian Lamparter <chunkeey@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/completion.h>
#include <linux/firmware.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>

#include "../arch/mips/ath79/pci-ath9k-fixup.h"

struct z1_ctx {
	struct completion eeprom_load;
};

static void z1_fw_cb(const struct firmware *fw, void *context)
{
	struct pci_dev *pdev = (struct pci_dev *) context;
	struct z1_ctx *ctx = (struct z1_ctx *) pci_get_drvdata(pdev);
	struct ath9k_platform_data *pdata = dev_get_platdata(&pdev->dev);
	struct pci_bus *bus;

	complete(&ctx->eeprom_load);

	if (!fw) {
		dev_err(&pdev->dev, "no '%s' eeprom file received.",
		       pdata->eeprom_name);
		goto release;
	}

	if (fw->size > sizeof(pdata->eeprom_data)) {
		dev_err(&pdev->dev, "loaded data is too big.");
		goto release;
	}

	pci_lock_rescan_remove();
	bus = pdev->bus;

	memcpy(pdata->eeprom_data, fw->data, sizeof(pdata->eeprom_data));
	/* eeprom has been successfully loaded - pass the data to ath9k
	 * but remove the eeprom_name, so it doesn't try to load it too.
	 */
	pdata->eeprom_name = NULL;

	pci_enable_ath9k_fixup(0, pdata->eeprom_data);
	pci_stop_and_remove_bus_device(pdev);
	/* the device should come back with the proper
	 * ProductId. But we have to initiate a rescan.
	 */
	pci_rescan_bus(bus);
	pci_unlock_rescan_remove();

release:
	release_firmware(fw);
}

static int z1_probe(struct pci_dev *pdev,
		    const struct pci_device_id *id)
{
	struct z1_ctx *ctx;
	struct ath9k_platform_data *pdata;
	int err = 0;

	if (pcim_enable_device(pdev))
		return -EIO;

	/* we now have a valid dev->platform_data */
	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata || !pdata->eeprom_data) {
		dev_err(&pdev->dev, "platform data missing or no eeprom file defined.");
		return -ENODEV;
	}

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		dev_err(&pdev->dev, "failed to alloc device context.");
		return -ENOMEM;
	}
	init_completion(&ctx->eeprom_load);

	pci_set_drvdata(pdev, ctx);
	err = request_firmware_nowait(THIS_MODULE, true, pdata->eeprom_name,
				      &pdev->dev, GFP_KERNEL, pdev, z1_fw_cb);
	if (err) {
		dev_err(&pdev->dev, "failed to request caldata (%d).", err);
		kfree(ctx);
	}
	return err;
}

static void z1_remove(struct pci_dev *pdev)
{
	struct z1_ctx *ctx = pci_get_drvdata(pdev);

	if (ctx) {
		wait_for_completion(&ctx->eeprom_load);
		pci_set_drvdata(pdev, NULL);
	}
}

static const struct pci_device_id z1_pci_table[] = {
	/* PCIe Owl Emulation */
	{ PCI_VDEVICE(ATHEROS, 0xff1c) }, /* * PCI-E */
	{ },
};
MODULE_DEVICE_TABLE(pci, z1_pci_table);

static struct pci_driver z1_driver = {
	.name		= "z1-pci",
	.id_table	= z1_pci_table,
	.probe		= z1_probe,
	.remove		= z1_remove,
};
module_pci_driver(z1_driver);

MODULE_LICENSE("GPL");
