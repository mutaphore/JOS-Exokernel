// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#include <kern/pmap.h>  // For page alloc/free commands

#define CMDBUF_SIZE	80	// enough for one VGA text line

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
   { "backtrace", "Display backtrace info", mon_backtrace },
   { "alloc_page", "Allocate a page", alloc_page },
   { "page_status", "Check if a page is allocated", page_status },
   { "free_page", "Free a page", free_page },
   { "list_used", "List all used pages and their refs", list_used }
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
   uint32_t ebp, eip, args[5] = {0};
   struct Eipdebuginfo info;
   
   cprintf("Stack backtrace:\n");

   ebp = read_ebp();   // Get the current ebp
   while (ebp != 0x0) {
      // Read in arguments wrt ebp pointer
      __asm__ __volatile__ ("movl 8(%1), %0" : "=r" (args[0]) : "r" (ebp));
      __asm__ __volatile__ ("movl 12(%1), %0" : "=r" (args[1]) : "r" (ebp));
      __asm__ __volatile__ ("movl 16(%1), %0" : "=r" (args[2]) : "r" (ebp));
      __asm__ __volatile__ ("movl 20(%1), %0" : "=r" (args[3]) : "r" (ebp));
      __asm__ __volatile__ ("movl 24(%1), %0" : "=r" (args[4]) : "r" (ebp));
      
      __asm__ __volatile__ ("movl 4(%1), %0" : "=r" (eip) : "r" (ebp)); // Read in return EIP
      debuginfo_eip(eip, &info); // Get eip debug info

      cprintf("  ebp %x  eip %x  args %08x %08x %08x %08x %08x\n",
       ebp, eip, args[0], args[1], args[2], args[3], args[4]); 
      cprintf("       %s:%d: %.*s+%d\n", info.eip_file, info.eip_line, 
       info.eip_fn_namelen, info.eip_fn_name, (eip - info.eip_fn_addr)); 
      
      __asm__ __volatile__ ("movl (%0), %0" : "=r" (ebp) : "0" (ebp));  // Step down stack to find the next ebp
   }	

   return 0;
}

// Extension commands
int alloc_page(int argc, char **argv, struct Trapframe *tf) {
   struct PageInfo *page;

   if ((page = page_alloc(ALLOC_ZERO))) {
      cprintf("%08p\n", page2pa(page));
      return 0;
   }
   else {
      cprintf("Cannot allocate page - out of memory\n");
      return -1;
   }
}

int page_status(int argc, char **argv, struct Trapframe *tf) {
   struct PageInfo *page;
   physaddr_t pa;
   char *end;
  
   if (argc == 2) {
      end = strfind(*(argv + 1), 0);
      pa = (physaddr_t)strtol(*(argv + 1), &end, 16);

      page = pa2page(pa);
      if (page->pp_link)
         cprintf("free\n");
      else
         cprintf("allocated\n");
      return 0;
   }
   else
      return -1;
}

int free_page(int argc, char **argv, struct Trapframe *tf) {
   struct PageInfo *page;
   physaddr_t pa;
   char *end;

   if (argc == 2) {
      end = strfind(*(argv + 1), 0);
      pa = (physaddr_t)strtol(*(argv + 1), &end, 16);

      page = pa2page(pa);
      if (page->pp_ref == 0 && page->pp_link == NULL)
         page_free(page);

      return 0;
   }
   else
      return -1;
}

int list_used(int argc, char **argv, struct Trapframe *tf) {
   struct PageInfo *page;
   pte_t *pte;

   for (page = pages + npages - 1; page >= pages; page--) {

      if (page->pp_link == NULL) {
         page_lookup(kern_pgdir, page2kva(page), &pte);
         cprintf("%08p  UWP:%d%d%d  ref:%d\n", page2pa(page),
          (*pte & PTE_U) >> 2, (*pte & PTE_W) >> 1, 
          *pte & PTE_P, page->pp_ref);
      }
   } 
   return 0; 
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
