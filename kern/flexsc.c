// FlexSC kernel functions

#include <kern/flexsc.h>

struct FscThread *scthreads;
static struct FscThread *fsc_free_list;

void flexsc_init() {
   struct FscThread *walker;

   for (walker = scthreads; walker < scthreads + NSCTHREADS; walker++)
      walker->thr_status = THR_FREE;

   return; 
}

void *scpage_alloc() {
   static int cur_page = 0; 
   
   if (cur_page >= NSCPAGES)
      return NULL;   // Out of scpages

   return scpages[cur_page++];
}

void scthread_spawn() {

   return;

}
