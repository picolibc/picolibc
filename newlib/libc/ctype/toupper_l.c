/*
FUNCTION
	<<toupper_l>>---translate characters to uppercase

INDEX
	toupper_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int toupper_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<toupper_l>> is a macro which converts lowercase characters to uppercase,
leaving all other characters unchanged.  It is only defined when
<[c]> is an integer in the range <<EOF>> to <<255>>.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<toupper_l>> returns the uppercase equivalent of <[c]> when it is a
character between <<a>> and <<z>>, and <[c]> otherwise.

PORTABILITY
<<toupper_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>
#if defined (_MB_EXTENDED_CHARSETS_ISO) \
    || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include <../locale/setlocale.h>
#endif

int
toupper_l (int c, struct __locale_t *locale)
{
#if defined (_MB_EXTENDED_CHARSETS_ISO) \
    || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
  if ((unsigned char) c <= 0x7f)
    return islower_l (c, locale) ? c - 'a' + 'A' : c;
  else if (c != EOF && __locale_mb_cur_max_l (locale) == 1
	   && islower_l (c, locale))
    {
      char s[MB_LEN_MAX] = { c, '\0' };
      wchar_t wc;
      mbstate_t state;

      memset (&state, 0, sizeof state);
      if (locale->mbtowc (_REENT, &wc, s, 1, &state) >= 0
	  && locale->wctomb (_REENT, s,
			     (wchar_t) towupper_l ((wint_t) wc, locale),
			     &state) == 1)
	c = (unsigned char) s[0];
    }
  return c;
#else
  return islower_l (c, locale) ? c - 'a' + 'A' : c;
#endif
}
