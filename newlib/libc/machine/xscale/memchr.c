#if defined __thumb__

#include "../../string/memchr.c"

#else

#include <string.h>
#include "xscale.h"

void *
memchr (const void *start, int c, size_t len)
{
  const char *str = start;

  if (len == 0)
    return 0;

  asm (PRELOADSTR ("%0") : : "r" (start));

  c &= 0xff;

#ifndef __OPTIMIZE_SIZE__
  /* Skip unaligned part.  */
  if ((long)str & 3)
    {
      str--;
      do
	{
	  if (*++str == c)
	    return (void *)str;
	}
      while (((long)str & 3) != 0 && --len > 0);
    }

  if (len > 3)
    {
      unsigned int c2 = c + (c << 8);
      c2 += c2 << 16;

      /* Load two constants:
         R7 = 0xfefefeff [ == ~(0x80808080 << 1) ]
         R6 = 0x80808080  */

      asm (
       "mov	r6, #0x80
	add	r6, r6, #0x8000
	add	r6, r6, r6, lsl #16
	mvn	r7, r6, lsl #1

0:
	cmp	%1, #0x7
	bls	1f

	ldmia	%0!, { r3, r9 }
"	PRELOADSTR ("%0") "
	sub	%1, %1, #8
	eor	r3, r3, %2
	eor	r9, r9, %2
	add	r2, r3, r7
	add	r8, r9, r7
	bic	r2, r2, r3
	bic	r8, r8,	r9
	and	r1, r2, r6
	and	r9, r8, r6
	orrs	r1, r1, r9
	beq	0b

	add	%1, %1, #8
	sub	%0, %0, #8
1:
	cmp	%1, #0x3
	bls	2f

	ldr	r3, [%0], #4
"	PRELOADSTR ("%0") "
	sub	%1, %1, #4
	eor	r3, r3, %2
	add	r2, r3, r7
	bic	r2, r2, r3
	ands	r1, r2, r6
	beq	1b

	sub	%0, %0, #4
	add	%1, %1, #4
2:
"
       : "=&r" (str), "=&r" (len)
       : "r" (c2), "0" (str), "1" (len)
       : "r1", "r2", "r3", "r6", "r7", "r8", "r9", "cc");
    }
#endif

  while (len-- > 0)
    { 
      if (*str == c)
        return (void *)str;
      str++;
    } 

  return 0;
}
#endif
