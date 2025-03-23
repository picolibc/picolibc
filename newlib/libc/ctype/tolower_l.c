/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <ctype.h>
#if defined (__MB_EXTENDED_CHARSETS_ISO) || defined (__MB_EXTENDED_CHARSETS_WINDOWS)
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include "locale_private.h"
#endif

int
tolower_l (int c, locale_t locale)
{
#if defined (__MB_EXTENDED_CHARSETS_ISO) || defined (__MB_EXTENDED_CHARSETS_WINDOWS)
  if ((unsigned char) c <= 0x7f) 
    return isupper_l (c, locale) ? c - 'A' + 'a' : c;
  else if (c != EOF && __locale_mb_cur_max_l (locale) == 1
	   && isupper_l (c, locale))
    {
      char s[MB_LEN_MAX] = { c, '\0' };
      wchar_t wc;
      mbstate_t state;

      memset (&state, 0, sizeof state);
      if (__MBTOWC_L(locale) (&wc, s, 1, &state) >= 0
	  && __WCTOMB_L(locale) (s,
			     (wchar_t) towlower_l ((wint_t) wc, locale),
			     &state) == 1)
	c = (unsigned char) s[0];
    }
  return c;
#else
  return isupper_l (c, locale) ? (c) - 'A' + 'a' : c;
#endif
}
