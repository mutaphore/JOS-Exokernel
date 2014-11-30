#ifndef JOS_KERN_FLEXSC_H
#define JOS_KERN_FLEXSC_H

#include <inc/flexsc.h>
#include <inc/stdio.h>
#include <inc/string.h>

#include <kern/env.h>
#include <kern/pmap.h>

#define NSCPAGES 5
#define NSCTHREADS 30

extern struct Segdesc gdt[];

struct FscThread *scthreads;
struct FscThread *fsc_free_list;
struct Taskstate fsc_ts;

__attribute__((__aligned__(PGSIZE)))
char scpages[NSCPAGES][PGSIZE];

void flex_start();

void test_flex();
void flexsc_init();
void *scpage_alloc();
void scthread_spawn();

#endif
