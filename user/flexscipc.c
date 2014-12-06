#include <inc/lib.h>

char *test_str = "This is a FlexSC test\n";

void 
umain(int argc, char **argv)
{
   envid_t who;
   int r;

   if ((who = fork()) != 0) {
      // Parent
      if ((r = flexsc_register()) < 0)
         panic("Failed to register with FlexSC: %e");

      flexsc_wait();

      flex_cputs(test_str, strlen(test_str));
      r = flex_getenvid();
      cprintf("I'm the parent %08x\n", r); 

      // Send a value to child
      while (flex_ipc_try_send(who, 0x1234, (void *)UTOP, 0) == -E_IPC_NOT_RECV)
         cprintf("Sending IPC to child %08x\n", who);
    
      cprintf("IPC completed!\n");
      return;
   }   

   // Child
   if ((r = flexsc_register()) < 0)
      panic("Failed to register with FlexSC: %e");
   flexsc_wait();

   r = flex_getenvid();
   cprintf("I'm the child %08x\n", r); 

   if (flex_ipc_recv((void *)UTOP) == 0)
      cprintf("%08x Received %08x via IPC!\n", thisenv->env_id, thisenv->env_ipc_value);
 
   flex_yield();
}
