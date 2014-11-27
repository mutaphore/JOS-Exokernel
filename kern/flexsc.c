// FlexSC kernel functions

#include <kern/flexsc.h>

void flexsc_init() {

   return; 

}

void *scpage_alloc() {
   static int cur_page = 0; 
   
   if (cur_page >= MAXSCPAGES)
      return NULL;   // Out of scpages

   return scpages[cur_page++];
}

void scthread_spawn() {

   return;

}
