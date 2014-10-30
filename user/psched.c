// Test priority scheduling

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t who;
   int i;

   // Fork some children
   for (i = 0; i < 10; i++) {
      if ((who = fork()) == 0)
         if (i < 5) {
            // Low priority children
            sys_env_set_priority(0, ENV_PR_LOWEST);   
            break;
         }
         else {
            // High priority children
            sys_env_set_priority(0, ENV_PR_HIGHEST);
            break;
         }
      }
   }

   if (who == 0) {
      cprintf("Hi, I'm child number %d\n", i);
      return;
   }
   cprintf("I'm the parent, done forking and exiting ...\n");
	return;
}

