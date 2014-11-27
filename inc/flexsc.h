#ifndef JOS_INC_FLEXSC_H
#define JOS_INC_FLEXSC_H

#define MAXSCARGS 5

enum ScStatus {
   FSC_FREE = 0,
   FSC_SUBMITTED,
   FSC_BUSY, 
   FSC_DONE
};

struct ScEntry {
   int32_t sc_num;         // Syscall number   
   uint32_t num_args;      // Number of arguments
   uint32_t args[MAXSCARGS];       // Arguments
   enum ScStatus status;   // Syscall status 
   int32_t ret;            // Syscall return value
};

#endif
