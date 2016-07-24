/*
FUNCTION
<<isdigit_l>>---decimal digit predicate

INDEX
isdigit_l

ANSI_SYNOPSIS
#include <ctype.h>
int isdigit_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isdigit_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
decimal digits, and 0 for other characters.  It is defined only if <[c]>
is representable as an unsigned char or if <[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isdigit_l>>'.

RETURNS
<<isdigit_l>> returns non-zero if <[c]> is a decimal digit (<<0>>--<<9>>).

PORTABILITY
<<isdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isdigit_l

int
isdigit_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & _N;
}
