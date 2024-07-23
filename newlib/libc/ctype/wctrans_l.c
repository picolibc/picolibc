/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <sys/cdefs.h>
#include <wctype.h>

wctrans_t
wctrans_l (const char *c, struct __locale_t *locale)
{
  (void) locale;
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return wctrans (c);
}
