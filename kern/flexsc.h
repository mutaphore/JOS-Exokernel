#ifndef JOS_KERN_FLEXSC_H
#define JOS_KERN_FLEXSC_H

#include <inc/flexsc.h>
#include <inc/stdio.h>

#include <kern/env.h>

#define NSCPAGES 5
#define NSCTHREADS 30

extern struct FscThread *scthreads;
extern struct Segdesc gdt[];

__attribute__((__aligned__(PGSIZE)))
char scpages[NSCPAGES][PGSIZE];

void flex_start();

void test_flex();
void flexsc_init();
void *scpage_alloc();
void scthread_spawn();

#endif
