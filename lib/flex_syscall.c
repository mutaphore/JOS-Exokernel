#include <inc/lib.h>
#include <inc/flexsc.h>

// Exception-less system call interface

static inline int32_t
flex_syscall(int num, int check, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
   static int en = 0;    // Keeps track of the syscall entry we're on

   scpage.entries[en].syscall = num;
   scpage.entries[en].args[0] = a0;
   scpage.entries[en].args[1] = a1;
   scpage.entries[en].args[2] = a2;
   scpage.entries[en].args[3] = a3;
   scpage.entries[en].args[4] = a4;
   scpage.entries[en].status = FSC_SUBMIT;

   while (scpage.entries[en].status != FSC_DONE)
      ;
}
