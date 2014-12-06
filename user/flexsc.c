#include <inc/lib.h>

char *test_str = "This is a FlexSC test\n";

void
umain(int argc, char **argv)
{
   int i, r;

   if ((r = flexsc_register()) < 0)
      panic("Failed to register with FlexSC: %e");

   // Batch system calls
   for (i = 0; i < 10; i++) 
      flex_cputs(test_str, strlen(test_str));

   cprintf("Waiting for system calls to complete... \n");
   flexsc_wait();

   cprintf("More system calls... \n");

   for (i = 0; i < 5; i++) {
      r = flex_time_msec();
      cprintf("Time is %d\n", r);
   }

   r = flex_getenvid();
   cprintf("My envid is %08x\n", r);

   if ((r = flex_page_alloc(r, (void *)0xF00000, PTE_W | PTE_U | PTE_P)) < 0)
      panic("Failed flex_page_alloc");
   *(int *)0xF00000 = 0x12345678;
}
