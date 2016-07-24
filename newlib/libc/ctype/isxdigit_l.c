/*
FUNCTION
<<isxdigit_l>>---hexadecimal digit predicate

INDEX
isxdigit_l

ANSI_SYNOPSIS
#include <ctype.h>
int isxdigit_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isxdigit_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
hexadecimal digits, and <<0>> for other characters.  It is defined only
if <[c]> is representable as an unsigned char or if <[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isxdigit_l>>'.

RETURNS
<<isxdigit_l>> returns non-zero if <[c]> is a hexadecimal digit
(<<0>>--<<9>>, <<a>>--<<f>>, or <<A>>--<<F>>).

PORTABILITY
<<isxdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>


#undef isxdigit_l
int
isxdigit_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & ((_X)|(_N));
}

