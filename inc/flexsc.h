#ifndef JOS_INC_FLEXSC_H
#define JOS_INC_FLEXSC_H

#include <inc/types.h>
#include <inc/trap.h>
#include <inc/memlayout.h>

enum FscStatus {
   FSC_FREE = 0,
   FSC_SUBMITTED,
   FSC_BUSY, 
   FSC_DONE
};

struct FscEntry {
   int32_t sc_num;            // Syscall number  
   uint32_t num_args;         // Number of arguments
   uint32_t args[5];          // Arguments
   enum FscStatus status;     // Syscall status 
   int32_t ret;               // Syscall return value
   int8_t _pad[28];           // Pad it up to 64 bytes
};

enum {
   THR_FREE = 0,
   THR_RUNNABLE,
   THR_RUNNING
};

struct FscThread {
   struct Trapframe thr_tf;    // Saved registers
   pde_t *thr_pgdir;           // Page dir cloned from registering process
   unsigned thr_status;
};

// Number of flexsc entries per syscall page
#define SCENTRIES (PGSIZE / sizeof(struct ScEntry))

#endif
