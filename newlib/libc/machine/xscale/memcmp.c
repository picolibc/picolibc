#if defined __thumb__

#include "../../string/memcmp.c"

#else

#include <string.h>
#include "xscale.h"

int
memcmp (const void *s1, const void *s2, size_t len)
{
  int result;
  asm (
#ifndef __OPTIMIZE_SIZE__ 
"		
	cmp	%2, #0x3
	bls	6f
	and	r2, %0, #0x3
	and	r3, %1, #0x3
	cmp	r2, r3
	bne	6f
	mov	lr, %0
	mov	r4, %1
	cmp	r2, #0x0
	beq	3f
	b	1f
0:
	ldrb	r2, [lr], #1	@ zero_extendqisi2
"	PRELOADSTR("lr") "
	ldrb	r3, [r4], #1	@ zero_extendqisi2
"	PRELOADSTR("r4") "
	cmp	r2, r3
	bne	5f
	tst	lr, #0x3
	beq	3f
1:
	sub	%2, %2, #1
	cmn	%2, #0x1
	bne	0b
	b	4f

0:
	cmp	%2, #0x7
	bls	3f
	ldmia	lr,{r2, r3}
	ldmia	r4,{r5, r6}
	sub	%2, %2, #0x4
	cmp	r2, r5
	bne	1f
	sub	%2, %2, #0x4
	add	lr, lr, #0x4
	add	r4, r4, #0x4
	cmp	r3, r6
	beq	0b
1:
	add	%2, %2, #0x4
	sub	%0, lr, #0x4
	sub	%1, r4, #0x4
	b	6f
3:
	cmp	%2, #0x3
	bls	1f
	ldr	r2, [lr], #4
	ldr	r3, [r4], #4
	sub	%2, %2, #4
	cmp	r2, r3
	bne	1f
0:
	cmp	%2, #0x3
	bls	1f
	ldr	r2, [lr], #4
"	PRELOADSTR("lr") "
	ldr	r3, [r4], #4
"	PRELOADSTR("r4") "
	sub	%2, %2, #4
	cmp	r2, r3
	beq	0b
1:
	sub	%0, lr, #0x4
	sub	%1, r4, #0x4
	add	%2, %2, #4
"
#endif /* !__OPTIMIZE_SIZE__ */
"
6:				
	sub	%2, %2, #1
	cmn	%2, #0x1
	beq	4f
0:
	ldrb	r2, [%0], #1	@ zero_extendqisi2
"	PRELOADSTR("%0") "
	ldrb	r3, [%1], #1	@ zero_extendqisi2
"	PRELOADSTR("%1") "
	cmp	r2, r3
	bne	5f
	sub	%2, %2, #1
	cmn	%2, #0x1
	bne	0b
4:		
	mov	r3, r2
5:		
	rsb	%0, r3, r2"
       : "=r" (result), "=&r" (s2), "=&r" (len)
       : "0" (s1), "1" (s2), "2" (len)
       : "r2", "r3", "r4", "r5", "r6", "cc");
  return result;
}
#endif
