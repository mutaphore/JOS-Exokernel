#include <kern/e1000.h>

// LAB 6: Your driver code here

int e1000_attach(struct pci_func *pcif) {

   // Enable the e1000 on PCI bus
   pci_func_enable(pcif);
   // Map MMIO region in DRAM to physical addrs on the device
   bar0addr = pcif->reg_base[0];
   bar0 = mmio_map_region(bar0addr, pcif->reg_size[0]);
   // Check the status register contents must be 0x80080783
   cprintf("e1000 device status register: %08x\n", bar0[REG(E1000_STATUS)]);

   // Add map of transcript descriptor array at a 16-byte aligned address
   physaddr_t pa = PADDR(tdarr); 
   boot_map_region(kern_pgdir, TDSTART, sizeof(tdarr), pa, PTE_W | PTE_P);

   // Save TD info into registers
   bar0[REG(E1000_TDBAL)] = TDSTART;
   bar0[REG(E1000_TDLEN)] = NUMTD * sizeof(struct tx_desc);
   // Reset head and tail regs
   bar0[REG(E1000_TDH)] = 0x0;
   bar0[REG(E1000_TDT)] = 0x0;
   // Setup TCTL
   bar0[REG(E1000_TCTL)] = 0;
   bar0[REG(E1000_TCTL)] |= E1000_TCTL_EN;
   bar0[REG(E1000_TCTL)] |= E1000_TCTL_PSP; 
   bar0[REG(E1000_TCTL)] |= 0x10 << 4;    // TCTL_CT
   bar0[REG(E1000_TCTL)] |= 0x40 << 12;   // TCTL_COLD 
   // Setup TIPG
   bar0[REG(E1000_TIPG)] = 0;
   bar0[REG(E1000_TIPG)] |= 10;         //TIPG_IPGT
   bar0[REG(E1000_TIPG)] |= 4 << 10;    //TIPG_IPGR1
   bar0[REG(E1000_TIPG)] |= 6 << 20;    //TIPG_IPGR2

   return 1;
}
