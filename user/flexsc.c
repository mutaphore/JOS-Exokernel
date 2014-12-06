#include <inc/lib.h>

//__attribute__((__aligned__(PGSIZE)))
//struct FscPage scpage;

char *test_str = "This is a FlexSC test\n";

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

   cprintf("More system calls... \n");

   r = flex_getenvid();
   cprintf("My envid is %08x\n", r);

//   while (1)
//      sys_yield();
}
