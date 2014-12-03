/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME(); * where NAME is the argument passed to TRAPHANDLER.  */

#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps


.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

// Standard Intel traps
TRAPHANDLER_NOEC(DIVIDE, T_DIVIDE)
TRAPHANDLER_NOEC(DEBUG, T_DEBUG)
TRAPHANDLER_NOEC(NMI, T_NMI)
TRAPHANDLER_NOEC(BRKPT, T_BRKPT)
TRAPHANDLER_NOEC(OFLOW, T_OFLOW)
TRAPHANDLER_NOEC(BOUND, T_BOUND)
TRAPHANDLER_NOEC(ILLOP, T_ILLOP)
TRAPHANDLER_NOEC(DEVICE, T_DEVICE)
TRAPHANDLER(DBLFLT, T_DBLFLT)
TRAPHANDLER(TSS, T_TSS)
TRAPHANDLER(SEGNP, T_SEGNP)
TRAPHANDLER(STACK, T_STACK)
TRAPHANDLER(GPFLT, T_GPFLT)
TRAPHANDLER(PGFLT, T_PGFLT)
TRAPHANDLER_NOEC(FPERR, T_FPERR)
TRAPHANDLER(ALIGN, T_ALIGN)
TRAPHANDLER_NOEC(MCHK, T_MCHK)
TRAPHANDLER_NOEC(SIMDERR, T_SIMDERR)

// System Call
TRAPHANDLER_NOEC(SYSCALL, T_SYSCALL)

// IRQ (Never pushes error code)
TRAPHANDLER_NOEC(IRQTIMER, IRQ_OFFSET + IRQ_TIMER)
TRAPHANDLER_NOEC(IRQKBD, IRQ_OFFSET + IRQ_KBD)
TRAPHANDLER_NOEC(IRQ2, IRQ_OFFSET + 2)
TRAPHANDLER_NOEC(IRQ3, IRQ_OFFSET + 3)
TRAPHANDLER_NOEC(IRQSERIAL, IRQ_OFFSET + IRQ_SERIAL)
TRAPHANDLER_NOEC(IRQ5, IRQ_OFFSET + 5)
TRAPHANDLER_NOEC(IRQ6, IRQ_OFFSET + 6)
TRAPHANDLER_NOEC(IRQSPURIOUS, IRQ_OFFSET + IRQ_SPURIOUS)
TRAPHANDLER_NOEC(IRQ8, IRQ_OFFSET + 8)
TRAPHANDLER_NOEC(IRQ9, IRQ_OFFSET + 9)
TRAPHANDLER_NOEC(IRQ10, IRQ_OFFSET + 10)
TRAPHANDLER_NOEC(IRQ11, IRQ_OFFSET + 11)
TRAPHANDLER_NOEC(IRQ12, IRQ_OFFSET + 12)
TRAPHANDLER_NOEC(IRQ13, IRQ_OFFSET + 13)
TRAPHANDLER_NOEC(IRQIDE, IRQ_OFFSET + IRQ_IDE)
TRAPHANDLER_NOEC(IRQERROR, IRQ_OFFSET + IRQ_ERROR)

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
   push     %ds            // Push ds reg on stack
   push     %es            // Push es reg on stack
   pushal                  // Push other generic regs on stack
   movw     $GD_KD, %ax    // Change to kernel segment
   movw     %ax, %ds
   movw     %ax, %es 
   pushl    %esp           // %esp is the Trapframe argument passed to trap
   call     trap
