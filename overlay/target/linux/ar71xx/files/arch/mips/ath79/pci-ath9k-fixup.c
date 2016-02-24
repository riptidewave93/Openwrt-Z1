/*
 *  Atheros AP94 reference board PCI initialization
 *
 *  Copyright (C) 2009-2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

struct ath9k_fixup {
	u16		*cal_data;
	const char	*cal_file;
	unsigned	slot;
};

static int ath9k_num_fixups;
static struct ath9k_fixup ath9k_fixups[2];

static void ath9k_pci_fix_dev(struct pci_dev *dev, const u16 *cal_data)
{
	void __iomem *mem;
	u16 cmd;
	u32 bar0;
	u32 val;

	if (*cal_data != 0xa55a) {
		pr_err("pci %s: invalid calibration data\n", pci_name(dev));
		return;
	}

	pr_info("pci %s: fixup device configuration\n", pci_name(dev));

	mem = ioremap(AR71XX_PCI_MEM_BASE, 0x10000);
	if (!mem) {
		pr_err("pci %s: ioremap error\n", pci_name(dev));
		return;
	}

	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &bar0);

	switch (ath79_soc) {
	case ATH79_SOC_AR7161:
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0,
				       AR71XX_PCI_MEM_BASE);
		break;
	case ATH79_SOC_AR7240:
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0xffff);
		break;

	case ATH79_SOC_AR7241:
	case ATH79_SOC_AR7242:
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0x1000ffff);
		break;
	case ATH79_SOC_AR9344:
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0x1000ffff);
		break;

	default:
		BUG();
	}

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config_word(dev, PCI_COMMAND, cmd);

	/* set pointer to first reg address */
	cal_data += 3;
	while (*cal_data != 0xffff) {
		u32 reg;
		reg = *cal_data++;
		val = *cal_data++;
		val |= (*cal_data++) << 16;

		__raw_writel(val, mem + reg);
		udelay(100);
	}

	pci_read_config_dword(dev, PCI_VENDOR_ID, &val);
	dev->vendor = val & 0xffff;
	dev->device = (val >> 16) & 0xffff;

	pci_read_config_dword(dev, PCI_CLASS_REVISION, &val);
	dev->revision = val & 0xff;
	dev->class = val >> 8; /* upper 3 bytes */

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd &= ~(PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
	pci_write_config_word(dev, PCI_COMMAND, cmd);

	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, bar0);

	iounmap(mem);
}

static void ath9k_pci_fixup_fw_cb(const struct firmware *fw, void *ctx)
{
	struct pci_dev *dev = (struct pci_dev *)ctx;

	if (fw) {
		ath9k_pci_fix_dev(dev, kmemdup(fw->data, fw->size, GFP_KERNEL));
		release_firmware(fw);
	}
}

static void ath9k_pci_fixup(struct pci_dev *dev)
{
	unsigned int i;
	int err;

	for (i = 0; i < ath9k_num_fixups; i++) {
		if (ath9k_fixups[i].slot != PCI_SLOT(dev->devfn))
			continue;

		if (ath9k_fixups[i].cal_file != NULL) {
#ifndef OPTION2
			device_initialize(&dev->dev);
			err = request_firmware_nowait(THIS_MODULE, true,
						      ath9k_fixups[i].cal_file,
						      &dev->dev, GFP_KERNEL,
						      dev,
						      ath9k_pci_fixup_fw_cb);
#else
			err = request_firmware_nowait(THIS_MODULE, true,
						      ath9k_fixups[i].cal_file,
						      NULL, GFP_KERNEL,
						      dev,
						      ath9k_pci_fixup_fw_cb);
#endif
			if (err) {
				printk(KERN_ERR "failed to request '%s' caldata (%d).\n",
				       ath9k_fixups[i].cal_file, err);
			}
		}

		if (ath9k_fixups[i].cal_data == NULL)
			continue;

		ath9k_pci_fix_dev(dev, ath9k_fixups[i].cal_data);
		break;
	}
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_ATHEROS, PCI_ANY_ID, ath9k_pci_fixup);

void __init pci_enable_ath9k_fixup(unsigned slot, u16 *cal_data)
{
	if (ath9k_num_fixups >= ARRAY_SIZE(ath9k_fixups))
		return;

	ath9k_fixups[ath9k_num_fixups].slot = slot;
	ath9k_fixups[ath9k_num_fixups].cal_data = cal_data;
	ath9k_num_fixups++;
}

void __init pci_enable_ath9k_fixup_file(unsigned int slot,
				        const char *cal_file)
{
	if (ath9k_num_fixups >= ARRAY_SIZE(ath9k_fixups))
		return;

	ath9k_fixups[ath9k_num_fixups].slot = slot;
	ath9k_fixups[ath9k_num_fixups].cal_file = cal_file;
	ath9k_num_fixups++;
}

