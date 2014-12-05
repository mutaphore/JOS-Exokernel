//////////////////////////////////////////////
//
// JOS + FlexSC
// Author: Dewei Chen
// 
//////////////////////////////////////////////

// FlexSC kernel functions

#include <kern/flexsc.h>

// For debugging
void test_flex(int num)
{
   int i = 0;
   struct Env *e; 

   for (i = 0; i < 100; i++) {
      cprintf("Test FlexSC Thread running %d, args %08x\n", i, num);
      //lock_kernel();
      // Before yielding save return point!!
      //sched_yield();
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

   // Initialize entries in syscall page
   entry = scpages[sc_pgnum].entries;
   for (i = 0; i < NSCENTRIES; i++) {
      memset(&entry[i], 0, sizeof(struct FscEntry));
      entry[i].status = FSC_FREE;
   }

   return &scpages[sc_pgnum++];
}

// Allocates a kernel stack
void *kstk_alloc(void) 
{
   static uint32_t kstkno = 0; 
   
   if (kstkno < NSCTHREADS)
      return (void *)(THRSTKTOP - kstkno++ * KSTKSIZE);
   
   return NULL;
}

// Creates a syscall thread that shares address space
// with parent. Has a separate stack. In many ways this 
// is similar to fork/sfork or clone in Linux. Returns
// the env_id of the syscall thread, < 0 on error.
int scthread_spawn(struct Env *parent)
{
   struct PageInfo *page;
   pte_t *ptEntry;
   struct Env *e;
   uint32_t pn;
   int r, perm = 0;
   void *addr;
   
   if ((r = env_alloc(&e, parent->env_id)) < 0)
      return r;

   // Copy all pages below USTACKTOP to thread environment
   for (pn = 0; pn < PGNUM(USTACKTOP - PGSIZE); pn++) {
      addr = (void *)(pn * PGSIZE);
      if (!(page = page_lookup(parent->env_pgdir, addr, &ptEntry)))
         continue;
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
   e->env_tf.tf_eip = (uintptr_t)scthread_task;
   // Allocate kernel stack   
   e->env_tf.tf_esp = (uintptr_t)kstk_alloc();
   // This thread starts off asleep 
   e->env_status = ENV_NOT_RUNNABLE;
 
   return e->env_id;  
}

void scthread_yield(void)
{
   return;
}

// Kills a scthread
void scthread_kill(void)
{
   lock_kernel();

   curenv->env_status = ENV_DYING;
   sched_yield();

   return;
}

// Wakes up a scthread
void scthread_run(struct Env *thr)
{
   thr->env_status = ENV_RUNNABLE;

   return;
}

// This is the function that every syscall thread starts at.
void scthread_task(void)
{
   struct FscEntry *entry = curenv->scpage->entries;
   int i = 0;

   while (1) {
      entry += i;
      if (entry->status == FSC_SUBMIT) {
         entry->status = FSC_BUSY;
         entry->ret = syscall(entry->syscall, entry->args[0], entry->args[1], 
                              entry->args[2], entry->args[3], entry->args[4]);
         entry->status = FSC_DONE;
      }

      if (entry->status == FSC_DONE)
         break;

      i = (i + 1) % NSCENTRIES;
   }
   
   curenv->link->env_status = ENV_RUNNABLE;
   
   scthread_kill(); 

   return;
}
