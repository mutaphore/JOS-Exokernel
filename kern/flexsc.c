// FlexSC kernel functions

#include <kern/flexsc.h>

static uint32_t cur_thr = -1;

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

void *scpage_alloc() 
{
   static int sc_pgnum = 0; 
   struct FscEntry *entry;
   int i;
   
   if (sc_pgnum >= NSCPAGES)
      return NULL;   // Out of syscall pages to allocate

   entry = (struct FscEntry *)scpages[sc_pgnum];
   for (i = 0; i < NSCENTRIES; i++) {
      memset(entry + i, 0, sizeof(struct FscEntry));
      entry->status = FSC_FREE;
   }

   return scpages[sc_pgnum++];
}

int scthread_task() 
{
   int i;

   for    
      

}

void scthread_run(uint32_t thr_id) {

}

void scthread_sched() 
{
   uint32_t prev_thr = 0;
 
   if (cur_thr >= 0)
      prev_thr = cur_thr;

   cur_thr = (cur_thr + 1) % NSCTHREADS;
   
   do {
      if (scthreads[cur_thr].thr_status == THR_FREE)
         scthread_run(cur_thr)
      cur_thr = (cur_thr + 1) % NSCTHREADS;
   } while(cur_thr != prev_thr);
   
   if (scthreads[prev_thr].thr_status == THR_RUNNING)
      scthread_run(prev_thr); 
}
