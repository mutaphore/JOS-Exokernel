#include <kern/e1000.h>

// LAB 6: Your driver code here

char test_packet[100] = {"-- This is a test Packet --"};

int e1000_attach(struct pci_func *pcif) {

   // Enable the e1000 on PCI bus
   pci_func_enable(pcif);
   // Map MMIO region in DRAM to physical addrs on the device
   bar0addr = pcif->reg_base[0];
   bar0 = mmio_map_region(bar0addr, pcif->reg_size[0]);
   // Check the status register contents must be 0x80080783
   cprintf("e1000: status %08x\n", bar0[REG(E1000_STATUS)]);
   // Initialize tranmit descriptors and registers
   trans_init();
   // Send a test packet
   trans_pckt(test_packet, 100);

   return 1;
}

void trans_init() {
   struct PageInfo *page;
   int i, error;

   // All mem addresses must be physical!

   // Allocate mem for transcript descriptor array
   if (!(page = page_alloc(ALLOC_ZERO)))
      panic("tdarr_alloc: out of memory");
   if ((error = page_insert(kern_pgdir, page, (void *)TDSTART, PTE_PCD | PTE_W | PTE_P)) < 0)
      panic("tdarr_alloc: %e", error);
   
   tdarr = (struct tx_desc *)TDSTART;

   // Initialize transmit descriptor fields
   for (i = 0; i < NUMTDS; i++) {
      // Buffer address
      tdarr[i].addr = PADDR(pbuf[i]);
      // Buffer size
      tdarr[i].lower.flags.length = PBUFSIZE;
      // Report status when packet is transmitted
      tdarr[i].lower.data |= E1000_TXD_CMD_RS; 
      tdarr[i].lower.data |= E1000_TXD_CMD_EOP; 
   }

   // Save TD info into registers
   bar0[REG(E1000_TDBAL)] = page2pa(page);  
   bar0[REG(E1000_TDLEN)] = NUMTDS * sizeof(struct tx_desc);

   // Setup head and tail regs
   bar0[REG(E1000_TDH)] = 0;
   bar0[REG(E1000_TDT)] = 0;
   head = &bar0[REG(E1000_TDH)];   
   tail = &bar0[REG(E1000_TDT)];   

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
}

int trans_pckt(void *pckt, uint32_t len) {
   physaddr_t buf;

   // Cannot transmit packet larger than buffer
   if (len > PBUFSIZE)
      return -E_PCKT_SIZE;
   // Check if transmit queue is full and the next slot is not ready
   if (NEXTNDX == *head && !(NEXTTD->upper.data & E1000_TXD_STAT_DD)) { 
      // Drop the packet for now
      cprintf("Packet dropped\n");
      return -E_PCKT_DROP;
   }
   buf = (physaddr_t)CURTD->addr;
   memcpy(KADDR(buf), pckt, len);
   *tail = NEXTNDX;
   return 0;   
}
