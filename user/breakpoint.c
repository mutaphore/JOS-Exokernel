// program to cause a breakpoint trap

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	asm volatile("int $3");
   cprintf("Continuing from breakpoint1!\n");
   cprintf("Continuing from breakpointX!\n");
   cprintf("Continuing from breakpointX!\n");
	asm volatile("int $3");
   cprintf("Continuing from breakpoint2!\n");
   cprintf("Continuing from breakpointX!\n");
   cprintf("Continuing from breakpointX!\n");
	asm volatile("int $3");
   cprintf("Continuing from breakpoint3!\n");
   cprintf("Continuing from breakpointX!\n");
   cprintf("Continuing from breakpointX!\n");
}

