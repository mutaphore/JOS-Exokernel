#ifndef JOS_KERN_FLEXSC_H
#define JOS_KERN_FLEXSC_H

#include <inc/flexsc.h>
#include <inc/mmu.h>

// Maximum syscall pages (flexsc processes) allowed
#define MAXSCPAGES 5

__attribute__((__aligned__(PGSIZE)))
char scpages[MAXSCPAGES][PGSIZE];

void flexsc_init();
void *scpage_alloc();
void scthread_spawn();

#endif
