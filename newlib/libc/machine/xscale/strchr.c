#if defined __thumb__

#include "../../string/strchr.c"

#else

#include <string.h>
#include "xscale.h"

char *
strchr (const char *s, int c)
{
  unsigned int c2;
  asm (PRELOADSTR ("%0") : : "r" (s));

  c &= 0xff;

#ifndef __OPTIMIZE_SIZE__
  /* Skip unaligned part.  */
  if ((long)s & 3)
    {
      s--;
      do
	{
	  int c2 = *++s;
	  if (c2 == c)
	    return (char *)s;
	  if (c2 == '\0')
	    return 0;
	}
      while (((long)s & 3) != 0);
    }

  c2 = c + (c << 8);
  c2 += c2 << 16;

  /* Load two constants:
     R6 = 0xfefefeff [ == ~(0x80808080 << 1) ]
     R5 = 0x80808080  */

  asm (PRELOADSTR ("%0") "
	mov	r5, #0x80
	add	r5, r5, #0x8000
	add	r5, r5, r5, lsl #16
	mvn	r6, r5, lsl #1

	sub	%0, %0, #4
0:
	ldr	r1, [%0, #4]!
"	PRELOADSTR ("%0") "
	add	r3, r1, r6
	bic	r3, r3, r1
	ands	r2, r3, r5
	bne	1f
	eor	r2, r1, %1
	add	r3, r2, r6
	bic	r3, r3, r2
	ands	r1, r3, r5
	beq	0b
1:"
       : "=&r" (s)
       : "r" (c2), "0" (s)
       : "r2", "r3", "r5", "r6", "cc");
#endif

  while (*s && *s != c)
    s++;
  if (*s == c)
    return (char *)s;
  return NULL;
}

#endif
