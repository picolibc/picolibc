/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#include <_ansi.h>
#include <wctype.h>

wctype_t
wctype_l (const char *c, struct __locale_t *locale)
{
  return wctype (c);
}
