// FlexSC kernel functions

#include <kern/flexsc.h>

void flexsc_init() {

   

}

void *scpage_alloc() {
   static int cur_page = 0; 
   
   if (cur_page < MAXSCPAGES)
      return scpages[cur_page++];
   else
      return NULL;   // Out of scpages
}
