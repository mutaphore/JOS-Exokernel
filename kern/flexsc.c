// FlexSC kernel functions

#include <kern/flexsc.h>

void test_flex()
{
   int i = 0;
   struct Env *e; 

   while (1)
      cprintf("Flex Thread running\n");
/*
   envid2env(0, &e, 0); 

   for (i = 0; i < 100; i++)
      cprintf("%d Hello World! I'm flexsc %08x, stack at %08x\n", \
              i, e->env_id, e->env_tf.tf_esp);

   env_destroy(curenv);
*/
}

void flexsc_init() {
   struct FscThread *walker;

   for (walker = scthreads; walker < scthreads + NSCTHREADS; walker++) {
      memset(walker, 0, sizeof(struct FscThread));
      walker->thr_status = THR_FREE;
   }
   
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
