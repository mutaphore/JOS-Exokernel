// hello, world
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
   int i;

   for (i = 0; i < 100; i++) {
      cprintf("hello, world\n");
      cprintf("i am environment %08x\n", thisenv->env_id);
   }
}
