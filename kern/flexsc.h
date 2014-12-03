#ifndef JOS_KERN_FLEXSC_H
#define JOS_KERN_FLEXSC_H

#include <inc/flexsc.h>
#include <inc/stdio.h>
#include <inc/string.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/syscall.h>

#define NSCPAGES 5
#define NSCTHREADS 3

struct FscThread scthreads[NSCTHREADS];
unsigned char thrstacks[NSCTHREADS][KSTKSIZE];
struct FscThread *fsc_free_list;

__attribute__((__aligned__(PGSIZE)))
struct FscPage scpages[NSCPAGES];

void flex_start();
void test_flex();

void flexsc_init();
void *scpage_alloc();
int scthread_spawn(struct Env *parent, struct FscPage *scpage);
int scthread_task(FscPage *scpage);

#endif
