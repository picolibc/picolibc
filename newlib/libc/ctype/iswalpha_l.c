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
iswalpha_l (wint_t c, locale_t locale)
{
  (void) locale;
#ifdef __MB_CAPABLE
  uint16_t cat = __ctype_table_lookup (c, locale);
  return cat & CLASS_alpha;
#else
  return c < (wint_t)0x100 ? isalpha (c) : 0;
#endif /* __MB_CAPABLE */
}
