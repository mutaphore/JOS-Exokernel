#include <inc/lib.h>

//__attribute__((__aligned__(PGSIZE)))
//struct FscPage scpage;

char *test_str = "This is a test string\n";

void
umain(int argc, char **argv)
{
   int r;

   if ((r = flexsc_register()) < 0)
      panic("Failed to register with FlexSC: %e");

   flex_cputs(test_str, strlen(test_str));
   flex_cputs(test_str, strlen(test_str));
   flex_cputs(test_str, strlen(test_str));
   flex_cputs(test_str, strlen(test_str));

   cprintf("Waiting for system calls to complete... \n");

   flexsc_wait();

   flex_cputs(test_str, strlen(test_str));
   flex_cputs(test_str, strlen(test_str));
   flex_cputs(test_str, strlen(test_str));
   flex_cputs(test_str, strlen(test_str));

   cprintf("Waiting for system calls to complete... \n");

//   flexsc_wait();

   while (1)
      sys_yield();
/*
   scpage.entries[0].syscall = SYS_cputs;
   scpage.entries[0].num_args = 2;
   scpage.entries[0].args[0] = (uintptr_t)test_str;
   scpage.entries[0].args[1] = strlen(test_str);
   scpage.entries[0].status = FSC_SUBMIT;

   cprintf("Waiting for system calls to complete... \n");
   flexsc_wait();

   while (scpage.entries[0].status != FSC_DONE)
      sys_yield();
*/

}
