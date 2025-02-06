/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#define _DEFAULT_SOURCE
#include <wctype.h>

wctype_t
wctype_l (const char *c, locale_t locale)
{
  (void) locale;
  return wctype (c);
}
