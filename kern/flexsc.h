#ifndef JOS_KERN_FLEXSC_H
#define JOS_KERN_FLEXSC_H

#include <inc/flexsc.h>
#include <inc/mmu.h>

#define NSCPAGES 5
#define NSCTHREADS 30

extern struct FscThread *scthreads;

__attribute__((__aligned__(PGSIZE)))
char scpages[NSCPAGES][PGSIZE];

void flexsc_init();
void *scpage_alloc();
void scthread_spawn();

#endif
