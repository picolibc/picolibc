/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <wctype.h>

int
iswdigit_l (wint_t c, struct __locale_t *locale)
{
  (void) locale;
  return c >= (wint_t)'0' && c <= (wint_t)'9';
}
