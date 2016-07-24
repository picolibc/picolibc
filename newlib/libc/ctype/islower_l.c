
/*
FUNCTION
<<islower_l>>---lowercase character predicate

INDEX
islower_l

ANSI_SYNOPSIS
#include <ctype.h>
int islower_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<islower_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
minuscules (lowercase alphabetic characters), and 0 for other characters.
It is defined only if <[c]> is representable as an unsigned char or if
<[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef islower_l>>'.

RETURNS
<<islower_l>> returns non-zero if <[c]> is a lowercase letter (<<a>>--<<z>>).

PORTABILITY
<<islower_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>

#undef islower_l

int
islower_l (int c, struct __locale_t *locale)
{
  return (__locale_ctype_ptr_l (locale)[c+1] & (_U|_L)) == _L;
}
