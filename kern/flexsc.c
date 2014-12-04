// FlexSC kernel functions

#include <kern/flexsc.h>

void test_flex(int num)
{
   int i = 0;
   struct Env *e; 

   for (i = 0; i < 100; i++) {
      cprintf("Flex Thread running %d, %08x\n", i, num);
      lock_kernel();
      // Before yielding save return point!!
      sched_yield();
   }

   // We must lock when returning from function because we
   // were running unlocked in ring 0 previously.
   lock_kernel();
   curenv->env_status = ENV_DYING;
   sched_yield();
}

void flexsc_init(void)
{
   return; 
}

// Allocates a system call page
struct FscPage *scpage_alloc(void) 
{
   static int sc_pgnum = 0; 
   struct FscEntry *entry;
   int i;
   
   if (sc_pgnum >= NSCPAGES)
      return NULL;   // Out of syscall pages to allocate

   entry = scpages[sc_pgnum].entries;
   for (i = 0; i < NSCENTRIES; i++) {
      memset(entry + i, 0, sizeof(struct FscEntry));
      entry->status = FSC_FREE;
   }

   return &scpages[sc_pgnum++];
}

// Allocates a kernel stack
void *kstk_alloc(void) 
{
   static uint32_t kstkno = 0; 
   
   if (kstkno < NSCTHREADS)
      return kthrstacks[kstkno++] + KSTKSIZE;
   
   return NULL;
}

// Creates a syscall thread that shares address space
// with parent. Has a separate stack. In many ways this 
// is similar to fork/sfork or clone in Linux.
int scthread_spawn(struct Env *parent, struct FscPage *scpage)
{
   struct PageInfo *page;
   pte_t *ptEntry;
   struct Env *e;
   uint32_t pn;
   int r, perm;
   void *addr;
   
   if ((r = env_alloc(&e, parent->env_id)) < 0)
      return r;

   // Copy all pages below USTACKTOP to thread environment
   for (pn = 0; pn < PGNUM(USTACKTOP - PGSIZE); pn++) {
      addr = (void *)(pn * PGSIZE);
      if (!(page = page_lookup(parent->env_pgdir, addr, &ptEntry)))
         return -E_INVAL;
      perm = *ptEntry & 0xFFF; 
      if ((r = page_insert(e->env_pgdir, page, addr, perm)) < 0)
         return r;
   } 

   // Create exception stack page
   addr = (void *)(UXSTACKTOP - PGSIZE);
   if (!(page = page_alloc(ALLOC_ZERO)))
      return -E_NO_MEM; 
   if ((r = page_insert(e->env_pgdir, page, addr, perm)) < 0)
      return r;
   
   // Set env type for debugging
   e->env_type = ENV_TYPE_FLEX;
   // Copy the page fault handler from parent
   e->env_pgfault_upcall = parent->env_pgfault_upcall;
   // Syscall thread will start at syscall task function
   e->env_tf.tf_eip = (uint32_t)scthread_task;
   // Push argument on the stack that thread will be running on
   e->env_tf.tf_esp -= sizeof(struct FscPage *);
   *(uint32_t *)e->env_tf.tf_esp = (uint32_t)scpage;
   // Set the thread runnable
   e->env_status = ENV_RUNNABLE;
   
   return e->env_id;  
}

// This is the function that every syscall thread starts at.
int scthread_task(struct FscPage *scpage) 
{
   struct FscEntry *entry = scpage->entries;

   while (1) {
      if (entry->status == FSC_SUBMITTED) {
         entry->status = FSC_BUSY;
         entry->ret = syscall(entry->sc_num, entry->args[0], entry->args[1], 
                              entry->args[2], entry->args[3], entry->args[4]);
         entry->status = FSC_DONE;
      }
      entry++;
   }
}
