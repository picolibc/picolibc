#if defined __thumb__

#include "../../string/memset.c"

#else

#include <string.h>
#include "xscale.h"

void *
memset (void *dst, int c, size_t len)
{
  int dummy;

  asm volatile ("tst	%0, #0x3"
#ifndef __OPTIMIZE_SIZE__
"
	beq	1f
	b	2f
0:
	strb	%1, [%0], #1
	tst	%0, #0x3
	beq	1f
2:
	movs	r3, %2
	sub	%2, %2, #1
	bne	0b
# At this point we know that %2 == len == -1 (since the SUB has already taken
# place).  If we fall through to the 1: label (as the code used to do), the
# CMP will detect this negative value and branch to the 2: label.  This will
# test %2 again, but this time against 0.  The test will fail and the loop
# at 2: will go on for (almost) ever.  Hence the explicit branch to the end
# of the hand written assembly code.
	b       4f
1:
	cmp	%2, #0x3
	bls	2f
	and	%1, %1, #0xff
	orr	lr, %1, %1, asl #8
	cmp	%2, #0xf
	orr	lr, lr, lr, asl #16
	bls	1f
	mov	r3, lr
	mov	r4, lr
	mov	r5, lr
0:
	sub	%2, %2, #16
	stmia	%0!, { r3, r4, r5, lr }
	cmp	%2, #0xf
	bhi	0b
1:
	cmp	%2, #0x7
	bls	1f
	mov	r3, lr
0:
	sub	%2, %2, #8
	stmia	%0!, { r3, lr }
	cmp	%2, #0x7
	bhi	0b
1:
	cmp	%2, #0x3
	bls	2f
0:
	sub	%2, %2, #4
	str	lr, [%0], #4
	cmp	%2, #0x3
	bhi	0b
"
#endif /* !__OPTIMIZE_SIZE__ */
"
2:
	movs	r3, %2
	sub	%2, %2, #1
	beq	4f
0:
	movs	r3, %2
	sub	%2, %2, #1
	strb	%1, [%0], #1
	bne	0b
4:"

       : "=&r" (dummy), "=&r" (c), "=&r" (len)
       : "0" (dst), "1" (c), "2" (len)
       : "memory", "r3", "r4", "r5", "lr");

  return dst;
}
 
#endif
