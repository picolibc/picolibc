#if defined __thumb__

#include "../../string/memcmp.c"

#else

#include <string.h>
#include "xscale.h"

int
memcmp (const void * s1, const void * s2, size_t len)
{
  int result;
  asm (
#ifndef __OPTIMIZE_SIZE__ 
"		
	cmp	%2, #0x3	@ Is the length a multiple of four ?
	bls	6f		@ no  = goto SLOW CHECK
	and	r2, %0, #0x3	@ get alignment of first pointer
	and	r3, %1, #0x3	@ get alignment of second pointer
	cmp	r2, r3		@ Do the two pointers share the same alignment ?
	bne	6f		@ no = goto SLOW CHECK
	mov	lr, %0		@ copy first pointer into LR
	mov	r4, %1		@ copy second pointer into R4
	cmp	r2, #0x0	@ Are we comparing word aligned pointers ?
	beq	3f		@ yes = goto START WORD CHECK LOOP
	b	1f		@ jump to LOOP TEST
0:			       @ LOOP START
	ldrb	r2, [lr], #1	@ load byte from LR, post inc.
"	PRELOADSTR("lr") "	@ preload 
	ldrb	r3, [r4], #1	@ load byte from R4, post inc.
"	PRELOADSTR("r4") "	@ preload
	cmp	r2, r3		@ are the two bytes the same ?
	bne	5f		@ no = goto EXIT
	tst	lr, #0x3	@ has the LR become word aligned ?
	bne     1f		@ no = skip the next test
	cmp     %2, #4		@ is the count >= 4 ?
	bhs     3f		@ yes = goto START WORD CHECK LOOP
1:			       @ LOOP TEST
	sub	%2, %2, #1	@ decrement count by one
	cmn	%2, #0x1	@ has the count reached -1 ?
	bne	0b		@ no = loop back to LOOP START
	b	4f		@ goto PASS END

0:			       @ ??
	cmp	%2, #0x7	@ Is the count a multiple of 8 ?
	bls	3f		@ no = goto ???
	ldmia	lr,{r2, r3}	@ get two words from first pointer, post inc
	ldmia	r4,{r5, r6}	@ get two words from second pointer, post inc
	sub	%2, %2, #0x4	@ decrement count by 4
	cmp	r2, r5		@ has the count reached ????
	bne	1f		@ no = goto 
	sub	%2, %2, #0x4	@ decrement the count by 4
	add	lr, lr, #0x4	@ add 4 to first pointer
	add	r4, r4, #0x4	@ add 4 to second pointer
	cmp	r3, r6		@ ???
	beq	0b		@ goto ???
1:			       @ ??
	add	%2, %2, #0x4	@ Add four to count
	sub	%0, lr, #0x4	@ decrement first pointer by 4
	sub	%1, r4, #0x4	@ decrement second pointer by 4
	b	6f		@ goto SLOW CHECK

3:			       @ START WORD CHECK LOOP
	cmp	%2, #0x3	@ is the count <= 3 ?
	bls	1f		@ yes = goto CHECK BYTES BY HAND
	ldr	r2, [lr], #4	@ get word from LR, post inc
	ldr	r3, [r4], #4	@ get word from R4, post inc
	sub	%2, %2, #4	@ decrement count by 4
	cmp	r2, r3		@ are the two words the same ?
	bne	1f		@ no = goto CHECK WORD CONTENTS
0:			       @ WORD CHECK LOOP
	cmp	%2, #0x3	@ is the count <= 3 ?
	bls	1f		@ yes = goto CHECK BYTES BY HAND
	ldr	r2, [lr], #4	@ load word from LR, post inc
"	PRELOADSTR("lr") "	@ preload
	ldr	r3, [r4], #4	@ load word from R4, post inc
"	PRELOADSTR("r4") "	@ preload
	sub	%2, %2, #4	@ decrement count by 4
	cmp	r2, r3		@ are the two words the same ?
	beq	0b		@ yes = goto WORD CHECK LOOP
1:			       @ CHECK BYTES BY HAND
	sub	%0, lr, #0x4	@ move LR back a word and put into first pointer
	sub	%1, r4, #0x4	@ move R4 back a word and put into second pointer
	add	%2, %2, #4	@ increment the count by 4
				@ fall through into SLOW CHECK"
#endif /* !__OPTIMIZE_SIZE__ */
"
6:			       @ SLOW CHECK
	sub	%2, %2, #1	@ Decrement the count by one
	cmn	%2, #0x1	@ Has the count reached -1 ?
	beq	4f		@ Yes - we are finished, goto PASS END
0:			       @ LOOP1
	ldrb	r2, [%0], #1	@ get byte from first pointer
"	PRELOADSTR("%0") "	@ preload first pointer
	ldrb	r3, [%1], #1	@ get byte from second pointer
"	PRELOADSTR("%1") "	@ preload second pointer
	cmp	r2, r3		@ compare the two loaded bytes
	bne	5f		@ if they are not equal goto EXIT
	sub	%2, %2, #1	@ decremented count by 1
	cmn	%2, #0x1	@ has the count reached -1 ?
	bne	0b		@ no = then go back to LOOP1
4:			       @ PASS END
	mov	r3, r2		@ Default return value is 0
5:			       @ EXIT
	rsb	%0, r3, r2	@ return difference between last two bytes loaded"
       : "=r" (result), "=&r" (s2), "=&r" (len)
       : "0" (s1), "1" (s2), "2" (len)
       : "r2", "r3", "r4", "r5", "r6", "cc", "lr");
  return result;
}
#endif
