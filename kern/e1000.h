#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

// Mac address
#define MACL 0x52540012
#define MACH 0x3456

// E1000 Vendor and Device ID's (82540EM in QEMU)
#define E1000_VENDORID 0x8086
#define E1000_DEVICEID 0x100E

/* Register Set. (82543, 82544)
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 */
// TX
#define E1000_STATUS   0x00008  /* Device Status - RO */
#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */
// RX
#define E1000_RCTL     0x00100  /* RX Control - RW */
#define E1000_RAL       0x05400  /* Receive Address Low - RW Array */
#define E1000_RAH       0x05404  /* Receive Address High - RW Array */
#define E1000_MTA      0x05200  /* Multicast Table Array - RW Array */
#define E1000_RDBAL    0x02800  /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH    0x02804  /* RX Descriptor Base Address High - RW */
#define E1000_RDLEN    0x02808  /* RX Descriptor Length - RW */
#define E1000_RDH      0x02810  /* RX Descriptor Head - RW */
#define E1000_RDT      0x02818  /* RX Descriptor Tail - RW */
#define E1000_RDTR     0x02820  /* RX Delay Timer - RW */

/* Transmit Control */
#define E1000_TCTL_RST    0x00000001    /* software reset */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */

/* Receive Control */
#define E1000_RCTL_RST            0x00000001    /* Software reset */
#define E1000_RCTL_EN             0x00000002    /* enable */
#define E1000_RCTL_SBP            0x00000004    /* store bad packet */
#define E1000_RCTL_UPE            0x00000008    /* unicast promiscuous enable */
#define E1000_RCTL_MPE            0x00000010    /* multicast promiscuous enab */
#define E1000_RCTL_LPE            0x00000020    /* long packet enable */

/* Transmit Descriptor bit definitions */
#define E1000_TXD_DTYP_D     0x00100000 /* Data Descriptor */
#define E1000_TXD_DTYP_C     0x00000000 /* Context Descriptor */
#define E1000_TXD_POPTS_IXSM 0x01       /* Insert IP checksum */
#define E1000_TXD_POPTS_TXSM 0x02       /* Insert TCP/UDP checksum */
#define E1000_TXD_CMD_EOP    0x01000000 /* End of Packet */
#define E1000_TXD_CMD_IFCS   0x02000000 /* Insert FCS (Ethernet CRC) */
#define E1000_TXD_CMD_IC     0x04000000 /* Insert Checksum */
#define E1000_TXD_CMD_RS     0x08000000 /* Report Status */
#define E1000_TXD_CMD_RPS    0x10000000 /* Report Packet Sent */
#define E1000_TXD_CMD_DEXT   0x20000000 /* Descriptor extension (0 = legacy) */
#define E1000_TXD_CMD_VLE    0x40000000 /* Add VLAN tag */
#define E1000_TXD_CMD_IDE    0x80000000 /* Enable Tidv register */
#define E1000_TXD_STAT_DD    0x00000001 /* Descriptor Done */
#define E1000_TXD_STAT_EC    0x00000002 /* Excess Collisions */
#define E1000_TXD_STAT_LC    0x00000004 /* Late Collisions */
#define E1000_TXD_STAT_TU    0x00000008 /* Transmit underrun */
#define E1000_TXD_CMD_TCP    0x01000000 /* TCP packet */
#define E1000_TXD_CMD_IP     0x02000000 /* IP packet */
#define E1000_TXD_CMD_TSE    0x04000000 /* TCP Seg enable */
#define E1000_TXD_STAT_TC    0x00000004 /* Tx Underrun */

// Transmit Descriptors
#define NUMTDS 8              // Number of transmit descriptors
#define TDSTART 0xF00D0000    // Arbitrary mem address of TD array
#define PBUFSIZE 1518         // Buffer size in bytes = max size of a packet

struct tx_desc {
   uint64_t addr;       /* Address of the descriptor's data buffer */
   union {
      uint32_t data;
      struct {
         uint16_t length;    /* Data buffer length */
         uint8_t cso;        /* Checksum offset */
         uint8_t cmd;        /* Descriptor control */
      } flags;
   } lower;
   union {
      uint32_t data;
      struct {
         uint8_t status;     /* Descriptor status */
         uint8_t css;        /* Checksum start */
         uint16_t special;
      } fields;
   } upper;
};

struct tx_desc *tdarr;        // Transmit Descriptors
char tbuf[NUMTDS][PBUFSIZE];  // Transmit Packet Buffers

// Receive Descriptors

#define RDSTART   0xF00E0000  // Arbitrary mem address of RD array
#define RBUFSIZE  2048        // 1 of the standard HW buf sizes   
#define NUMRDS 8              // Number of receive descriptors

struct rx_desc {
    uint64_t buffer_addr; /* Address of the descriptor's data buffer */
    uint16_t length;     /* Length of data DMAed into data buffer */
    uint16_t csum;       /* Packet checksum */
    uint8_t status;      /* Descriptor status */
    uint8_t errors;      /* Descriptor Errors */
    uint16_t special;
};

struct rx_desc *rdarr;        // Receive Descriptors
char rbuf[NUMRDS][RBUFSIZE];  // Receive Packet Buffers

// Register addresses
physaddr_t bar0addr; 
volatile uint32_t *bar0;
volatile uint32_t *thead;  // *head is an index
volatile uint32_t *ttail;  // *tail is an index
volatile uint32_t *rhead;  // *head is an index
volatile uint32_t *rtail;  // *tail is an index

// Convert register byte offset to an index into bar0 array
#define REG(byte) ((byte)/4)
// Get the current descriptor pointed to by tail
#define CURTD (tdarr + *ttail)
#define CURRD (rdarr + *rtail)
// Get the next descriptor after tail
#define NEXTTNDX ((*ttail + 1) % NUMTDS)
#define NEXTRNDX ((*rtail + 1) % NUMRDS)
#define NEXTTD (tdarr + NEXTTNDX)
#define NEXTRD (rdarr + NEXTRNDX)

int e1000_attach(struct pci_func *pcif);
void trans_init();
int trans_pckt(void *pckt, uint32_t len);
void recv_init();

#endif	// JOS_KERN_E1000_H
