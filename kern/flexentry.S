#define FSC_ENTRY(x)
   .text; _ALIGN_TEXT; .globl x; .type x,@function; x:

FSC_ENTRY(flex_start)
   popal
   popfl
   popl %esp
   ret

//.globl flex_start;							\
//flex_start:								\
//   movl $0xeebfe000, %esp
//   call test_flex 

