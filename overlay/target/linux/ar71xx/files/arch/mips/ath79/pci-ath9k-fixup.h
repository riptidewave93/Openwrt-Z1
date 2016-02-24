#ifndef _PCI_ATH9K_FIXUP
#define _PCI_ATH9K_FIXUP

void pci_enable_ath9k_fixup(unsigned slot, u16 *cal_data) __init;
void pci_enable_ath9k_fixup_file(unsigned slot, const char *cal_file) __init;

#endif /* _PCI_ATH9K_FIXUP */
