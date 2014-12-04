#ifndef JOS_KERN_FLEXSC_H
#define JOS_KERN_FLEXSC_H

#include <inc/flexsc.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/error.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/syscall.h>

#define THRSTKTOP 0xFEED0000  // Kernel thread stack top
#define NSCTHREADS 5          // Number of syscall threads
#define NSCPAGES NSCTHREADS   // Number of syscall pages

__attribute__((__aligned__(PGSIZE)))
struct FscPage scpages[NSCPAGES];
__attribute__((__aligned__(PGSIZE)))
char kthrstacks[NSCTHREADS][KSTKSIZE];

void flex_start();
void test_flex(char *msg);

void flexsc_init(void);
struct FscPage *scpage_alloc();
int scthread_spawn(struct Env *parent, struct FscPage *scpage);
int scthread_task(struct FscPage *scpage);

#endif
