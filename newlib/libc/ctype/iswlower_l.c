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
iswlower_l (wint_t c, locale_t locale)
{
  (void) locale;
#ifdef __MB_CAPABLE
  // The wide-character class "lower" contains at least those characters wc
  // which are equal to towlower(wc) and different from towupper(wc).
  uint16_t cat = __ctype_table_lookup (c, locale);
  return (cat & CLASS_lower) || ((cat & CLASS_case) && towupper_l(c, locale) != c);
#else
  return c < 0x100 ? islower (c) : 0;
#endif /* __MB_CAPABLE */
}
