#ifndef JOS_INC_FLEXSC_H
#define JOS_INC_FLEXSC_H

#include <inc/types.h>

enum ScStatus {
   FSC_FREE = 0,
   FSC_SUBMITTED,
   FSC_BUSY, 
   FSC_DONE
};

struct ScEntry {
   int32_t sc_num;            // Syscall number   
   uint32_t num_args;         // Number of arguments
   uint32_t args[5];          // Arguments
   enum ScStatus status;      // Syscall status 
   int32_t ret;               // Syscall return value
   int8_t _pad[28];           // Pad it up to 64 bytes
};

// Number of flexsc entries per syscall page
#define SCENTRIES (PGSIZE / sizeof(struct ScEntry))

#endif
