#ifndef JOS_INC_FLEXSC_H
#define JOS_INC_FLEXSC_H

#include <inc/types.h>
#include <inc/trap.h>
#include <inc/memlayout.h>

#define THRSTKTOP 0xFEED0000  // Kernel thread stack top
#define NSCENTRIES 64

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

struct FscPage {
   struct FscEntry entries[NSCENTRIES];
};

// Number of flexsc entries per syscall page

enum {
   THR_FREE = 0,
   THR_RUNNABLE,
   THR_RUNNING
};

struct FscThread {
   struct PushRegs thr_regs;
   uint32_t thr_eflags;
   uintptr_t thr_esp;
   uintptr_t thr_eip;
   unsigned thr_status; 
   uint32_t thr_id;
};


#endif
