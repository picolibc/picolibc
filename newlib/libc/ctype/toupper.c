/*
FUNCTION
	<<toupper>>, <<toupper_l>>---translate characters to uppercase

INDEX
	toupper

INDEX
	toupper_l

INDEX
	_toupper

SYNOPSIS
	#include <ctype.h>
	int toupper(int <[c]>);
	int _toupper(int <[c]>);

	#include <ctype.h>
	int toupper_l(int <[c]>, locale_t <[locale]>);


DESCRIPTION
<<toupper>> is a macro which converts lowercase characters to uppercase,
leaving all other characters unchanged.  It is only defined when
<[c]> is an integer in the range <<EOF>> to <<255>>.

<<toupper_l>> is like <<toupper>> but performs the function based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining this macro using `<<#undef toupper>>' or `<<#undef toupper_l>>'.

<<_toupper>> performs the same conversion as <<toupper>>, but should
only be used when <[c]> is known to be a lowercase character (<<a>>--<<z>>).

RETURNS
<<toupper>>, <<toupper_l>> return the uppercase equivalent of <[c]> when
<[c]> is a lowercase character, and <[c]> otherwise.

<<_toupper>> returns the uppercase equivalent of <[c]> when it is a
character between <<a>> and <<z>>.  If <[c]> is not one of these
characters, the behaviour of <<_toupper>> is undefined.

PORTABILITY
<<toupper>> is ANSI C.  <<_toupper>> is not recommended for portable programs.
<<toupper_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>
#if defined (_MB_EXTENDED_CHARSETS_ISO) || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#endif

#undef toupper
int
toupper (int c)
{
#if defined (_MB_EXTENDED_CHARSETS_ISO) || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
  if ((unsigned char) c <= 0x7f)
    return islower (c) ? c - 'a' + 'A' : c;
  else if (c != EOF && MB_CUR_MAX == 1 && islower (c))
    {
      char s[MB_LEN_MAX] = { c, '\0' };
      wchar_t wc;
      if (mbtowc (&wc, s, 1) >= 0
	  && wctomb (s, (wchar_t) towupper ((wint_t) wc)) == 1)
	c = (unsigned char) s[0];
    }
  return c;
#else
  return islower (c) ? c - 'a' + 'A' : c;
#endif
}
