/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <wctype.h>

int
iswxdigit_l (wint_t c, struct __locale_t *locale)
{
  (void) locale;
  return ((c >= (wint_t)'0' && c <= (wint_t)'9') ||
	  (c >= (wint_t)'a' && c <= (wint_t)'f') ||
	  (c >= (wint_t)'A' && c <= (wint_t)'F'));
}

