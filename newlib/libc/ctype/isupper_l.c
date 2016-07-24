/*
FUNCTION
<<isupper_l>>---uppercase character predicate

INDEX
isupper_l

ANSI_SYNOPSIS
#include <ctype.h>
int isupper_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isupper_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
uppercase letters (<<A>>--<<Z>>), and 0 for other characters.  It is
defined only when <<isascii>>(<[c]>) is true or <[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isupper_l>>'.

RETURNS
<<isupper_l>> returns non-zero if <[c]> is a uppercase letter (A-Z).

PORTABILITY
<<isupper_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>

#undef isupper_l

int
isupper_l (int c, struct __locale_t *locale)
{
  return (__locale_ctype_ptr_l (locale)[c+1] & (_U|_L)) == _U;
}

