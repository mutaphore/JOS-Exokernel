// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
   
   pte_t ptEntry = uvpt[PGNUM(addr)];
   // Check for a write and to a copy-on-write page
   if (!(err & FEC_WR && ptEntry & PTE_COW)) {
      cprintf("addr %08x err %08x COW? %d\n", addr, err, ptEntry & PTE_COW);
      panic("Not a write or a copy-on-write page");
   }
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
   
   // Address must be page aligned! 
   void *algn_addr = ROUNDDOWN(addr, PGSIZE);

   // Allocate new page at PFTEMP
   if ((r = sys_page_alloc(0, PFTEMP, PTE_P | PTE_U | PTE_W)) < 0)
      panic("pgfault: sys_page_alloc %e", r);
   // Copy contents of page containing faulted addr to temp page
   memmove(PFTEMP, algn_addr, PGSIZE);
   // Map to the "old" address, now with read and write permissions
   if ((r = sys_page_map(0, PFTEMP, 0, algn_addr, PTE_P | PTE_U | PTE_W)) < 0)
      panic("pgfault: sys_page_map %e", r);
   // Unmap the temporary address
   if ((r = sys_page_unmap(0, PFTEMP)) < 0)   
      panic("pgfault: sys_page_unmap %e", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
   pte_t ptEntry;
   pde_t pdEntry;
   void *addr = (void *)(pn * PGSIZE);

	// LAB 4: Your code here.
   
   pdEntry = uvpd[PDX(addr)]; 
   if (pdEntry & PTE_P) {
      ptEntry = uvpt[pn];
      if (ptEntry & PTE_P) {
         if (ptEntry & PTE_SHARE) {
            // Share this page with parent
            if ((r = sys_page_map(0, addr, envid, addr, PTE_SHARE | PTE_U | PTE_W | PTE_P)) < 0)
               panic("duppage: sys_page_map page to be shared %e", r);
         }
         else if (ptEntry & PTE_W || ptEntry & PTE_COW) {
            // Map to new env COW
            if ((r = sys_page_map(0, addr, envid, addr, PTE_COW | PTE_U | PTE_P)) < 0)
               panic("duppage: sys_page_map to new env %e", r);
            // Remap our own to COW
            if ((r = sys_page_map(0, addr, 0, addr, PTE_COW | PTE_U | PTE_P)) < 0)
               panic("duppage: sys_page_map for remap %e", r);
         }
         else {
            // Just directly map pages that are present but not W or COW
            if ((r = sys_page_map(0, addr, envid, addr, PTE_P | PTE_U)) < 0)
               panic("duppage: sys_page_map to new env PTE_P %e", r);
         }
      }
   }

/*
   // Map to new env COW
   if ((r = sys_page_map(0, addr, envid, addr, PTE_COW | PTE_U | PTE_P)) < 0)
      panic("duppage: sys_page_map to new env %e", r);
   // Remap our own to COW
   if ((r = sys_page_map(0, addr, 0, addr, PTE_COW | PTE_U | PTE_P)) < 0)
      panic("duppage: sys_page_map for remap %e", r);
*/
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
   
   envid_t envid;
   uint32_t pn;
   void *addr;
   int r;
   pte_t ptEntry;
   pde_t pdEntry;

   // Set up our own page fault handler
   set_pgfault_handler(pgfault);
   // Create a child environment
   envid = sys_exofork();

   if (envid < 0)
      panic("sys_exofork: %e", envid);
   if (envid == 0) {
      // We're the child

      // Global var thisenv refers to the parent, fix it
      thisenv = &envs[ENVX(sys_getenvid())];
      return 0;
   }
   
   // We're the parent
   
   // Copy address space (not including exception stack) to child
   for (pn = 0; pn < PGNUM(UXSTACKTOP - PGSIZE); pn++)
      duppage(envid, pn);

   // Create exception stack page for child
   addr = (void *)(UXSTACKTOP - PGSIZE);
   if ((r = sys_page_alloc(envid, addr, PTE_P | PTE_U | PTE_W)) < 0)
      panic("fork: sys_page_alloc UXSTACK %e", r);
   
   // Set up the child's page fault handler
   sys_env_set_pgfault_upcall(envid, thisenv->env_pgfault_upcall);
   
   // Set child to be runnable
   if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) 
      panic("fork: sys_env_set_status %e", r);

   return envid;
}

// Challenge!
int
sfork(void)
{
	return -E_INVAL;
}
