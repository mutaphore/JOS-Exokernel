#include <inc/lib.h>
#include <inc/flexsc.h>

// FlexSC Exception-less system call interface

static inline void
flex_syscall(int num, int check, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, struct FscEntry *entry)
{
   entry->syscall = num;
   entry->args[0] = a1;
   entry->args[1] = a2;
   entry->args[2] = a3;
   entry->args[3] = a4;
   entry->args[4] = a5;
   entry->status = FSC_SUBMIT;
}

// Allocates the next free entry to use on syscall page
struct FscEntry *
entry_alloc()
{
   struct FscEntry *entry = (struct FscEntry *)USCPAGE;
   int i;

   for (i = 0; i < NSCENTRIES; i++) {
      if (entry[i].status == FSC_FREE) {
         entry[i].status = FSC_ALLOC;
         break;
      }
   }
   if (i == NSCENTRIES)
      panic("Ran out of syscall entries!");
   
   return &entry[i];
}

void
flex_cputs(const char *s, size_t len)
{
   struct FscEntry *entry = entry_alloc();

   flex_syscall(SYS_cputs, 0, (uint32_t)s, len, 0, 0, 0, entry);
}

int
flex_cgetc(void)
{
   struct FscEntry *entry = entry_alloc();
   int ret;

   flex_syscall(SYS_cgetc, 0, 0, 0, 0, 0, 0, entry);

   while (entry->status != FSC_DONE)
      ;

   ret = entry->ret;
   entry->status = FSC_FREE;

   return ret;
}

int
flex_env_destroy(envid_t envid)
{
   struct FscEntry *entry = entry_alloc();
   int ret;

   flex_syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0, entry);

   while (entry->status != FSC_DONE)
      ;
   ret = entry->ret;
   entry->status = FSC_FREE;

   return ret;
}

envid_t
flex_getenvid(void)
{
   struct FscEntry *entry = entry_alloc();
   int ret;   

   flex_syscall(SYS_getenvid, 0, 0, 0, 0, 0, 0, entry);

   while (entry->status != FSC_DONE)
      sys_yield();

   ret = entry->ret;
   entry->status = FSC_FREE;
   
   return ret;
}
/*
void
flex_yield(void)
{
   flex_syscall(SYS_yield, 0, 0, 0, 0, 0, 0);
}

int
flex_page_alloc(envid_t envid, void *va, int perm)
{
   return flex_syscall(SYS_page_alloc, 1, envid, (uint32_t) va, perm, 0, 0);
}

int
flex_page_map(envid_t srcenv, void *srcva, envid_t dstenv, void *dstva, int perm)
{
   return flex_syscall(SYS_page_map, 1, srcenv, (uint32_t) srcva, dstenv, (uint32_t) dstva, perm);
}

int
flex_page_unmap(envid_t envid, void *va)
{
   return flex_syscall(SYS_page_unmap, 1, envid, (uint32_t) va, 0, 0, 0);
}

// sys_exofork is inlined in lib.h

int
flex_env_set_status(envid_t envid, int status)
{
   return flex_syscall(SYS_env_set_status, 1, envid, status, 0, 0, 0);
}

int
flex_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
   return flex_syscall(SYS_env_set_trapframe, 1, envid, (uint32_t) tf, 0, 0, 0);
}

int
flex_env_set_pgfault_upcall(envid_t envid, void *upcall)
{
   return flex_syscall(SYS_env_set_pgfault_upcall, 1, envid, (uint32_t) upcall, 0, 0, 0);
}

int
flex_ipc_try_send(envid_t envid, uint32_t value, void *srcva, int perm)
{
   return flex_syscall(SYS_ipc_try_send, 0, envid, value, (uint32_t) srcva, perm, 0);
}

int
flex_ipc_recv(void *dstva)
{
   return flex_syscall(SYS_ipc_recv, 1, (uint32_t)dstva, 0, 0, 0, 0);
}

unsigned int
flex_time_msec(void)
{
   return (unsigned int)flex_syscall(SYS_time_msec, 0, 0, 0, 0, 0, 0);
}

int
flex_net_send_pckt(void *src, uint32_t len)
{
   return flex_syscall(SYS_net_send_pckt, 1, (uint32_t)src, len, 0, 0, 0);
}

int
flex_net_recv_pckt(void *dstva)
{
   return flex_syscall(SYS_net_recv_pckt, 0, (uint32_t)dstva, 0, 0, 0, 0);
}
*/
