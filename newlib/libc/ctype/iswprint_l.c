/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#define _DEFAULT_SOURCE
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswprint_l (wint_t c, struct __locale_t *locale)
{
  (void) locale;
#ifdef _MB_CAPABLE
  c = _jp2uc_l (c, locale);
  enum category cat = category (c);
  return cat != -1
      && cat != CAT_Cc && cat != CAT_Cf
      && cat != CAT_Cs // Surrogate
      ;
#else
  return c < (wint_t)0x100 ? isprint (c) : 0;
#endif /* _MB_CAPABLE */
}
