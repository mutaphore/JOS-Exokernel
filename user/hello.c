// Test priority scheduling

#include <inc/lib.h>

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
            sys_env_set_priority(0, ENV_PR_LOW);   
            break;
         }
         else { 
            // High priority children (odd)
            sys_env_set_priority(0, ENV_PR_HIGH);
            break;
         }
      }
   }
   
   // Yield the parent
   if (who != 0) {
      cprintf("I'm the parent, done forking and exiting ...\n");
      sys_yield();
      return;
   }

   // Children wait for the parent to finish
   while (envs[ENVX(parent)].env_status != ENV_FREE)
      asm volatile("pause");
    
   sys_yield();   
   cprintf("Hello, I'm %s priority child number %d\n", 
    i % 2 == 0 ? "LOW" : "HIGH", i);

   return;
}

