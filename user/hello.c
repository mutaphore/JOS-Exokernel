// Test priority scheduling

#include <inc/lib.h>

volatile int counter;

void
umain(int argc, char **argv)
{
	envid_t who;
   envid_t parent = sys_getenvid();
   int i;
   
   // Set the parent to highest priority
   sys_env_set_priority(0, ENV_PR_HIGHEST);   
   
   // Fork some children
   for (i = 0; i < 10; i++) {
      if ((who = fork()) == 0) {
         if (i % 2 == 0) {
            // Low priority children (even)
            sys_env_set_priority(0, ENV_PR_LOWEST);   
            //sys_env_set_priority(0, ENV_PR_MEDIUM);
            break;
         }
         else { 
            // High priority children (odd)
            sys_env_set_priority(0, ENV_PR_HIGHEST);
            //sys_env_set_priority(0, ENV_PR_MEDIUM);
            break;
         }
      }
   }
   
   // Yield the parent
   if (i == 10) {
      cprintf("I'm the parent, done forking and exiting ...\n");
      sys_yield();
      return;
   }

   while (envs[ENVX(parent)].env_status != ENV_FREE)
      asm volatile("pause");
    
   sys_yield();   
   cprintf("Hello, I'm child number %d\n", i);
   return;
}

