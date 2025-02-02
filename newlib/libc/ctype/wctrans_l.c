/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <wctype.h>

wctrans_t
wctrans_l (const char *c, locale_t locale)
{
  (void) locale;
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return wctrans (c);
}
