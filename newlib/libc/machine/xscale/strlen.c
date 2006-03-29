#if defined __thumb__ || defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

#include "../../string/strlen.c"

#else

#include <string.h>
#include "xscale.h"

size_t
strlen (const char *str)
{
  _CONST char *start = str;

  /* Skip unaligned part.  */
  if ((long)str & 3)
    {
      str--;
      do
	{
	  if (*++str == '\0')
	    goto out;
	}
      while ((long)str & 3);
    }

  /* Load two constants:
     R4 = 0xfefefeff [ == ~(0x80808080 << 1) ]
     R5 = 0x80808080  */

  asm ("mov	r5, #0x80
	add	r5, r5, #0x8000
	add	r5, r5, r5, lsl #16
	mvn	r4, r5, lsl #1
"

#if defined __ARM_ARCH_5__ || defined __ARM_ARCH_5T__ || defined __ARM_ARCH_5E__ || defined __ARM_ARCH_5TE__

"	tst	%0, #0x7
	ldreqd	r6, [%0]
	beq	1f
	ldr	r2, [%0]
        add     r3, r2, r4
        bic     r3, r3, r2
        ands    r2, r3, r5
	bne	2f
	sub	%0, %0, #4

0:
	ldrd	r6, [%0, #8]!
"
	PRELOADSTR ("%0")
"
1:
	add	r3, r6, r4
	add	r2, r7, r4
	bic	r3, r3, r6
	bic	r2, r2, r7
	and	r3, r3, r5
	and	r2, r2, r5
	orrs	r3, r2, r3
	beq	0b
"
#else

"	sub	%0, %0, #4

0:
	ldr	r6, [%0, #4]!
"
	PRELOADSTR ("%0")
"
	add	r3, r6, r4
	bic	r3, r3, r6
	ands	r3, r3, r5
	beq	0b
"
#endif /* __ARM_ARCH_5[T][E]__ */
"
2:
	ldrb	r3, [%0]
	cmp	r3, #0x0
	beq	1f

0:
	ldrb	r3, [%0, #1]!
"
	PRELOADSTR ("%0")
"
	cmp	r3, #0x0
	bne	0b
1:
"
       : "=r" (str) : "0" (str) : "r2", "r3", "r4", "r5", "r6", "r7");

  out:
  return str - start;
}

#endif
