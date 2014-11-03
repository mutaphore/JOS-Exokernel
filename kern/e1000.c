#include <kern/e1000.h>

// LAB 6: Your driver code here

int e1000_attach(struct pci_func *pcif) {

   // Enable the e1000 on PCI bus
   pci_func_enable(pcif);
   // Map MMIO region in DRAM to physical addrs on the device
   bar0_va = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);


   return 1;
}
