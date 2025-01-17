/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#define _DEFAULT_SOURCE
#include <ctype.h>
#include <wctype.h>
#include "local.h"

int
iswupper_l (wint_t c, struct __locale_t *locale)
{
  (void) locale;
#ifdef _MB_CAPABLE
  uint16_t cat = __ctype_table_lookup (c, locale);
  return (cat & CLASS_upper) || ((cat & CLASS_case) && towlower_l(c, locale) != c);
#else
  return c < 0x100 ? isupper (c) : 0;
#endif /* _MB_CAPABLE */
}
