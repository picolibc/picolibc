#if defined __thumb__

#include "../../string/strcmp.c"

#else

#include <string.h>
#include "xscale.h"

int
strcmp (const char *s1, const char *s2)
{
  asm (PRELOADSTR ("%0") : : "r" (s1));
  asm (PRELOADSTR ("%0") : : "r" (s2));

#ifndef __OPTIMIZE_SIZE__
  if (((long)s1 & 3) == ((long)s2 & 3))
    {
      int result;

      /* Skip unaligned part.  */
      while ((long)s1 & 3)
	{
	  if (*s1 == '\0' || *s1 != *s2)
	    goto out;
	  s1++;
	  s2++;
	}

  /* Load two constants:
     lr = 0xfefefeff [ == ~(0x80808080 << 1) ]
     ip = 0x80808080  */

      asm (
       "ldr	r2, [%1, #0]
	ldr	r3, [%2, #0]
	cmp	r2, r3
	bne	2f

	mov	ip, #0x80
	add	ip, ip, #0x8000
	add	ip, ip, ip, lsl #16
	mvn	lr, ip, lsl #1

0:
	ldr	r2, [%1, #0]
	add	r3, r2, lr
	bic	r3, r3, r2
	tst	r3, ip
	beq	1f
	mov	%0, #0x0
	b	3f
1:
	ldr	r2, [%1, #4]!
	ldr	r3, [%2, #4]!
"	PRELOADSTR("%1") "
"	PRELOADSTR("%2") "
	cmp	r2, r3
	beq	0b"

       /* The following part could be done in a C loop as well, but it needs
	  to be assembler to save some cycles in the case where the optimized
	  loop above finds the strings to be equal.  */
"
2:
	ldrb	r2, [%1, #0]
"	PRELOADSTR("%1") "
"	PRELOADSTR("%2") "
	cmp	r2, #0x0
	beq	1f
	ldrb	r3, [%2, #0]
	cmp	r2, r3
	bne	1f
0:
	ldrb	r3, [%1, #1]!
	add	%2, %2, #1
	ands	ip, r3, #0xff
	beq	1f
	ldrb	r3, [%2]
	cmp	ip, r3
	beq	0b
1:
	ldrb	lr, [%1, #0]
	ldrb	ip, [%2, #0]
	rsb	%0, ip, lr
3:
"

       : "=r" (result), "=&r" (s1), "=&r" (s2)
       : "1" (s1), "2" (s2)
       : "lr", "ip", "r2", "r3", "cc");
      return result;
    }
#endif

  while (*s1 != '\0' && *s1 == *s2)
    {
      asm (PRELOADSTR("%0") : : "r" (s1));
      asm (PRELOADSTR("%0") : : "r" (s2));
      s1++;
      s2++;
    }
 out:
  return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

#endif
