// hello, world
#include <inc/lib.h>

struct FscPage scpage;

void
umain(int argc, char **argv)
{
   int i, r;
/*
   for (i = 0; i < 100; i++)
      cprintf("I am user environment %08x\n", thisenv->env_id);
*/
   if ((r = flexsc_register(&scpage)) < 0)
      panic("Failed to register with FlexSC: %e");

   scpage.entries[0].sc_num = 5678;
   cprintf("DEBUG sc_num %d\n", scpage.entries[0].sc_num);
   cprintf("DEBUG status %d\n", scpage.entries[0].status);

   cprintf("Waiting for system calls to complete... \n");
   flexsc_wait();
}
