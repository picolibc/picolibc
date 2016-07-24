
/*
FUNCTION
<<ispunct_l>>---punctuation character predicate

INDEX
ispunct_l

ANSI_SYNOPSIS
#include <ctype.h>
int ispunct_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<ispunct_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
printable punctuation characters, and 0 for other characters.  It is
defined only if <[c]> is representable as an unsigned char or if <[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef ispunct_l>>'.

RETURNS
<<ispunct_l>> returns non-zero if <[c]> is a printable punctuation character
(<<isgraph(<[c]>) && !isalnum(<[c]>)>>).

PORTABILITY
<<ispunct_l>> is POSIX-1.2008

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef ispunct_l

int
ispunct_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & _P;
}

