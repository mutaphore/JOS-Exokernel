// FlexSC kernel functions

#include <kern/flexsc.h>

static struct FscThread cur_thr = NULL;

void test_flex()
{
   int i = 0;
   struct Env *e; 

   while (1)
      cprintf("Flex Thread running\n");
}

void flexsc_init() 
{
   struct FscThread *thr;
   int i = 0;

   for (thr = scthreads; thr < scthreads + NSCTHREADS; walker++) {
      memset(thr, 0, sizeof(struct FscThread));
      thr->thr_status = THR_FREE;
      thr->thr_id = i++;
   }
   
   return; 
}

// Allocates a system call page
void *scpage_alloc() 
{
   static int sc_pgnum = 0; 
   struct FscEntry *entry;
   int i;
   
   if (sc_pgnum >= NSCPAGES)
      return NULL;   // Out of syscall pages to allocate

   entry = scpages[sc_pgnum];
   for (i = 0; i < NSCENTRIES; i++) {
      memset(entry + i, 0, sizeof(struct FscEntry));
      entry->status = FSC_FREE;
   }

   return scpages[sc_pgnum++];
}

// Creates a system call thread that shares address space
// with parent. Has a separate stack. In many ways this 
// is similar to fork/sfork or clone in Linux.
int scthread_spawn(struct Env *parent)
{
   struct PageInfo *page;
   pte_t ptEntry;
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
   e->env_tf.tf_eip =    
   // Set the thread runnable
   e->env_status = ENV_RUNNABLE;
   
   return e->env_id;  
}

int scthread_task(FscEntry *scpage) 
{
   struct FscEntry *entry = scpage;

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

void scthread_run(uint32_t thr_id) 
{
   
   if (cur_thr->thr_status == THR_RUNNING)
      cur_thr->thr_status = THR_RUNNABLE; 

   cur_thr = scthreads[thr_id]; 
   cur_thr->thr_status = THR_RUNNING;
      
}

void scthread_sched() 
{
   uint32_t cur = 0, prev = 0;
 
   if (cur_thr) {
      prev = cur_thr->thr_id;
      cur = (prev + 1) % NSCTHREADS;
   }

   do {
      if (scthreads[cur].thr_status == THR_FREE ||
          scthreads[cur].thr_status == THR_RUNNABLE)
         scthread_run(cur)
      cur = (cur + 1) % NSCTHREADS;
   } while(cur != prev);
   
   if (scthreads[prev].thr_status == THR_RUNNING)
      scthread_run(prev); 
}
