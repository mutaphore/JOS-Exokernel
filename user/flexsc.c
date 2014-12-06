#include <inc/lib.h>

char *test_str = "This is a FlexSC test\n";

void
umain(int argc, char **argv)
{
   int i, r;
   envid_t who;
/*
   if ((r = flexsc_register()) < 0)
      panic("Failed to register with FlexSC: %e");

   // Batch system calls
   for (i = 0; i < 5; i++) 
      flex_cputs(test_str, strlen(test_str));

   cprintf("Waiting for system calls to complete... \n");
   flexsc_wait();

   cprintf("More system calls... \n");

   r = flex_getenvid();
   cprintf("I'm the parent, my envid is %08x\n", r);

   if ((r = flex_page_alloc(r, (void *)0xF00000, PTE_W | PTE_U | PTE_P)) < 0)
      panic("Failed flex_page_alloc");

   *(int *)0xF00000 = 0x12345678;
  
   for (i = 0; i < 5; i++) {
      r = flex_time_msec();
      cprintf("Time is %d\n", r);
   }
*/
   if ((who = fork()) != 0) {
      if ((r = flexsc_register()) < 0)
         panic("Failed to register with FlexSC: %e");
      flexsc_wait();
      flex_cputs(test_str, strlen(test_str));
      // Parent send a value to child
      while (flex_ipc_try_send(who, 0x888, (void *)UTOP, 0) == -E_IPC_NOT_RECV)
         ;
         //cprintf("Sending IPC to child %08x\n", who);
      return;
   }
   // We are child
   if ((r = flexsc_register()) < 0)
      panic("Failed to register with FlexSC: %e");
   flexsc_wait();

   r = flex_getenvid();
   cprintf("I'm the child, my envid is %08x\n", r);

   while (1)
      sys_yield();
/*
   if (flex_ipc_recv((void *)UTOP) == 0)
      cprintf("Received %08x via IPC!\n", thisenv->env_ipc_value);
 
   flex_yield();
*/
}
