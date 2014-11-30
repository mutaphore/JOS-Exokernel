.text

.globl flex_start;							\
flex_start:								\
   movl $0xeebfe000, %esp;							\
   call test_flex 
