/*
FUNCTION
	<<isalnum_l>>---alphanumeric character predicate

INDEX
	isalnum_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int isalnum_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isalnum_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
alphabetic or numeric ASCII characters, and <<0>> for other arguments.
It is defined only if <[c]> is representable as an unsigned char or if
<[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isalnum_l>>'.

RETURNS
<<isalnum_l>> returns non-zero if <[c]> is a letter (<<a>>--<<z>> or
<<A>>--<<Z>>) or a digit (<<0>>--<<9>>).

PORTABILITY
<<isalnum_l>> is POSIX-1.2008.

No OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isalnum_l

int
isalnum_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & (_U|_L|_N);
}
