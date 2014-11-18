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
   // Initialize receive descriptors and registers
   recv_init(); 

   // Send a test packet
   //trans_pckt(test_packet, 100);

   return 1;
}

void trans_init() {
   struct PageInfo *page;
   int i, error;

   // All mem addresses must be physical!

   // Allocate mem for transmit descriptor array
   if (!(page = page_alloc(ALLOC_ZERO)))
      panic("tdarr_alloc: out of memory");
   if ((error = page_insert(kern_pgdir, page, (void *)TDSTART, PTE_PCD | PTE_W | PTE_P)) < 0)
      panic("tdarr_alloc: %e", error);
   
   tdarr = (struct tx_desc *)TDSTART;
   // Initialize transmit descriptor fields
   for (i = 0; i < NUMTDS; i++) {
      // Buffer address
      tdarr[i].addr = PADDR(tbuf[i]);
      // Buffer size
      tdarr[i].lower.flags.length = PBUFSIZE;
      // Report status when packet is transmitted
      tdarr[i].lower.data |= E1000_TXD_CMD_RS; 
      // Report end of packet
      tdarr[i].lower.data |= E1000_TXD_CMD_EOP; 
      // Others
      //tdarr[i].lower.data |= E1000_TXD_CMD_EOP; 
   }

   // Save TD info into registers
   bar0[REG(E1000_TDBAL)] = page2pa(page);  
   bar0[REG(E1000_TDLEN)] = NUMTDS * sizeof(struct tx_desc);

   // Setup head and tail regs
   thead = &bar0[REG(E1000_TDH)];   
   ttail = &bar0[REG(E1000_TDT)];   
   *thead = 0;
   *ttail = 0;

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
   void *buf;

   // Cannot transmit packet larger than buffer
   if (len > PBUFSIZE)
      return -E_PCKT_SIZE;

   // Check if transmit queue is full and the next slot is not ready
   if (NEXTTNDX == *thead && 
      !(NEXTTD->upper.data & E1000_TXD_STAT_DD)) { 
      // Drop the packet for now
      cprintf("Packet dropped\n");
      return -E_PCKT_DROP;
   }
   
   // Set the descriptor packet length
   CURTD->lower.flags.length = len;
   // Copy packet into buffer
   buf = KADDR((physaddr_t)CURTD->addr);
   memcpy(buf, pckt, len);
   // Move tail pointer forward
   *ttail = NEXTTNDX;

   return 0;   
}

void recv_init() {
   struct PageInfo *page;
   int i, error;

   // Setup mac address register
   bar0[REG(E1000_RAL)] = MACL;
   bar0[REG(E1000_RAH)] = MACH; 
   bar0[REG(E1000_RAH)] |= E1000_RAH_AV;   // Address valid 
   // Initialize Multicast table array
   bar0[REG(E1000_MTA)] = 0;

   // Allocate memory for receive descriptor array
   if (!(page = page_alloc(ALLOC_ZERO)))
      panic("rdarr alloc: out of memory");
   if ((error = page_insert(kern_pgdir, page, (void *)RDSTART, PTE_PCD | PTE_W | PTE_P)) < 0)
      panic("rdarr alloc: %e", error);

   rdarr = (struct rx_desc *)RDSTART;

   // Initialize receive descriptor fields
   for (i = 0; i < NUMRDS; i++) {
      // Receive Buffer address
      rdarr[i].buffer_addr = PADDR(rbuf[i]);
   }
   
   // Save RD info into registers
   bar0[REG(E1000_RDBAL)] = page2pa(page);  
   bar0[REG(E1000_RDLEN)] = NUMRDS << 7;

   // Setup head and tail regs
   rhead = &bar0[REG(E1000_RDH)];   
   rtail = &bar0[REG(E1000_RDT)];   
   *rhead = 0;
   *rtail = NUMRDS - 1;
   
   // Setup RCTL
   bar0[REG(E1000_RCTL)] = 0;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_SZ_2048;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_SECRC;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_BAM;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_EN;
}

// Receive a packet and copy its contents to store
// Returns the length of packet received or < 0 on error.
int recv_pckt(void *store) {
   uint32_t len = 0;
   void *buf;      

   // Check if no more packets have been received
   if (!NEXTRD->status) {
      cprintf("No packets received %d\n", *rhead);
      return -E_PCKT_NONE;
   } 

   *rtail = NEXTRNDX;
   buf = KADDR((physaddr_t)CURRD->buffer_addr);
   len = CURRD->length; 

   memcpy(store, buf, len);
   CURRD->status &= 0;
   
   return len; 
}

