/*
FUNCTION
	<<tolower_l>>---translate characters to lowercase

INDEX
	tolower_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int tolower_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<tolower_l>> is a macro which converts uppercase characters to lowercase,
leaving all other characters unchanged.  It is only defined when
<[c]> is an integer in the range <<EOF>> to <<255>>.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<tolower_l>> returns the lowercase equivalent of <[c]> when it is a
character between <<A>> and <<Z>>, and <[c]> otherwise.

PORTABILITY
<<tolower_l>> is POSIX-1.2008.
programs.

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
#include "../locale/setlocale.h"
#endif

int
tolower_l (int c, struct __locale_t *locale)
{
#if defined (_MB_EXTENDED_CHARSETS_ISO) || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
  if ((unsigned char) c <= 0x7f) 
    return isupper_l (c, locale) ? c - 'A' + 'a' : c;
  else if (c != EOF && __locale_mb_cur_max_l (locale) == 1
	   && isupper_l (c, locale))
    {
      char s[MB_LEN_MAX] = { c, '\0' };
      wchar_t wc;
      mbstate_t state;

      memset (&state, 0, sizeof state);
      if (locale->mbtowc (_REENT, &wc, s, 1, &state) >= 0
	  && locale->wctomb (_REENT, s,
			     (wchar_t) towlower_l ((wint_t) wc, locale),
			     &state) == 1)
	c = (unsigned char) s[0];
    }
  return c;
#else
  return isupper_l (c, locale) ? (c) - 'A' + 'a' : c;
#endif
}
