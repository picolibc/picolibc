/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
#include <_ansi.h>
#include <ctype.h>

#undef toascii_l

int
toascii_l (int c, struct __locale_t *locale)
{
  return c & 0177;
}
