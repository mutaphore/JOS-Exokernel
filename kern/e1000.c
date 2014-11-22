#include <kern/e1000.h>
#include <kern/sched.h>

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
   struct PageInfo *page, *tdpage;
   int i, error;

   // All mem addresses must be physical!

   // Allocate mem for transmit descriptor array
   if (!(tdpage = page_alloc(ALLOC_ZERO)))
      panic("tdarr_alloc: out of memory");
   if ((error = page_insert(kern_pgdir, tdpage, \
       (void *)TDSTART, PTE_PCD | PTE_W | PTE_P)) < 0)
      panic("tdarr_alloc: %e", error);
   
   tdarr = (struct tx_desc *)TDSTART;
   // Initialize transmit descriptor fields
   for (i = 0; i < NUMTDS; i++) {
      // Buffer address
      page = page_lookup(kern_pgdir, (void *)(TBUFMAP + i * PGSIZE), NULL);
      tdarr[i].addr = page2pa(page);
      // Buffer size
      tdarr[i].lower.flags.length = PBUFSIZE;
      // Report status when packet is transmitted
      tdarr[i].lower.data |= E1000_TXD_CMD_RS; 
      // Report end of packet
      tdarr[i].lower.data |= E1000_TXD_CMD_EOP; 
   }

   // Save TD info into registers
   bar0[REG(E1000_TDBAL)] = page2pa(tdpage);  
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
   bar0[REG(E1000_TIPG)] = 10;           //TIPG_IPGT
   bar0[REG(E1000_TIPG)] |= 6 << 10;     //TIPG_IPGR1
   bar0[REG(E1000_TIPG)] |= 4 << 20;    //TIPG_IPGR2
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

   // Zero-copy if pckt is NULL
   if (pckt) {
      buf = KADDR((physaddr_t)CURTD->addr);
      memcpy(buf, pckt, len);
   }
   // Move tail pointer forward
   *ttail = NEXTTNDX;

   return 0;   
}

void recv_init() {
   struct PageInfo *page, *rdpage;
   union Nsipc *nsipcbuf;
   int i, error;

   // Setup mac address register
   bar0[REG(E1000_RAL)] = MACL;
   bar0[REG(E1000_RAH)] = MACH; 
   bar0[REG(E1000_RAH)] |= E1000_RAH_AV;   // Address valid 

   // Initialize Multicast table array
   bar0[REG(E1000_MTA)] = 0;
/*
   // Flow control setup
   bar0[REG(E1000_FCAL)] = 0x00C28001;
   bar0[REG(E1000_FCAH)] = 0x0100; 
   bar0[REG(E1000_FCT)] = 0x8808;
   bar0[REG(E1000_CTRL)] |= E1000_CTRL_RFCE;
   bar0[REG(E1000_CTRL)] |= E1000_CTRL_FD;
*/
   // Allocate memory for receive descriptor array
   if (!(rdpage = page_alloc(ALLOC_ZERO)))
      panic("rdarr alloc: out of memory");
   if ((error = page_insert(kern_pgdir, rdpage, \
       (void *)RDSTART, PTE_PCD | PTE_W | PTE_P)) < 0)
      panic("rdarr alloc: %e", error);

   rdarr = (struct rx_desc *)RDSTART;

   // Set buffer addr to descriptors
   for (i = 0; i < NUMRDS; i++) {
      page = page_lookup(kern_pgdir, (void *)(RBUFMAP + i * PGSIZE), NULL);
      rdarr[i].buffer_addr = page2pa(page) + sizeof(int);   // Nsipc jp_data offset
   } 

   // Save RD info into registers
   bar0[REG(E1000_RDBAL)] = page2pa(rdpage);  
   bar0[REG(E1000_RDLEN)] = NUMRDS * sizeof(struct rx_desc);

   // Setup head and tail regs
   rhead = &bar0[REG(E1000_RDH)];   
   rtail = &bar0[REG(E1000_RDT)];   
   *rhead = 0;
   *rtail = NUMRDS - 1;

   // Setup interrupts
   bar0[REG(E1000_IMC)] = 0xFFFFFFFF;
   bar0[REG(E1000_IMS)] = E1000_ICR_RXDMT0;
   bar0[REG(E1000_ICS)] = E1000_ICR_RXDMT0;

   // Setup RCTL
   bar0[REG(E1000_RCTL)] = 0;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_EN;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_SZ_2048;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_SECRC;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_BAM;
   bar0[REG(E1000_RCTL)] |= E1000_RCTL_RDMTS_HALF;    //Interrupt when 1/2 left
}

// Receive a packet and copy its contents to store
// Returns the length of packet received or < 0 on error.
int recv_pckt(void *store) {
   uint32_t len;   
   void *buf;

   if (NEXTRD->status & E1000_RXD_STAT_DD) {
      *rtail = NEXTRNDX;
      len = CURRD->length; 
      // Zero-copy if store is NULL
      if (store) {
         buf = KADDR((physaddr_t)CURRD->buffer_addr);
         memcpy(store, buf, len);
      }
      CURRD->status = 0;   // Clear status telling e1000 its ready for reuse
      return len;
   }
   return -E_PCKT_NONE;
}
