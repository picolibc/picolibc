#if defined __thumb__

#include "../../string/memmove.c"

#else

#include <string.h>
#include "xscale.h"

static inline void *
do_memcpy (void *dst0, const void *src0, size_t len)
{
  int dummy;
  asm volatile (
#ifndef __OPTIMIZE_SIZE__
       "cmp	%2, #0x3
	bls	3f
	and	lr, %1, #0x3
	and	r3, %0, #0x3
	cmp	lr, r3
	bne	3f
	cmp	lr, #0x0
	beq	2f
	b	1f
0:
	ldrb	r3, [%1], #1
"
	PRELOADSTR ("%1")
"
	tst	%1, #0x3
	strb	r3, [%0], #1
	beq	3f
1:
	sub	%2, %2, #1
	cmn	%2, #1
	bne	0b
2:
	cmp	%2, #0xf
	bls	1f
0:
	ldmia	%1!, { r3, r4, r5, lr }
"
	PRELOADSTR ("%1")
"

	sub	%2, %2, #16
	cmp	%2, #0xf
	stmia	%0!, { r3, r4, r5, lr }
	bhi	0b
1:
	cmp	%2, #0x7
	bls	1f
0:
	ldmia	%1!, { r3, r4 }
"
	PRELOADSTR ("%1")
"

	sub	%2, %2, #8
	cmp	%2, #0x7
	stmia	%0!, { r3, r4 }
	bhi	0b
1:
	cmp	%2, #0x3
	bls	3f
0:
	sub	%2, %2, #4
	ldr	r3, [%1], #4
"
	PRELOADSTR ("%1")
"

	cmp	%2, #0x3
	str	r3, [%0], #4
	bhi	0b
"
#endif /* !__OPTIMIZE_SIZE__ */
"
3:
"
	PRELOADSTR ("%1")
"
	sub	%2, %2, #1
	cmn	%2, #1
	beq	1f
0:
	sub	%2, %2, #1
	ldrb	r3, [%1], #1
"
	PRELOADSTR ("%1")
"
	cmn	%2, #1
	strb	r3, [%0], #1
	bne	0b
1:"
       : "=&r" (dummy), "=&r" (src0), "=&r" (len)
       : "0" (dst0), "1" (src0), "2" (len)
       : "memory", "lr", "r3", "r4", "r5", "cc");
  return dst0;
}

void *
memmove (void *dst, const void *src, size_t len)
{
  char *d = dst;
  const char *s = src;

  if (s < d && d < s + len)
    {
      /* Destructive overlap...have to copy backwards.  */
      s += len;
      d += len;

      while (len--)
	*--d = *--s;

      return dst;
    }
  else
    return do_memcpy (dst, src, len);
}
#endif
