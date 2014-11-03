#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>
#include <kern/pmap.h>

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

#define E1000_STATUS   0x00008  /* Device Status - RO */

// Register addresses

volatile physaddr_t bar0addr; 
volatile uint32_t *bar0;

# define REGNDX(byte) \
   ((byte)/4)

int e1000_attach(struct pci_func *pcif);

#endif	// JOS_KERN_E1000_H
