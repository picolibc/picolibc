/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswblank_l (wint_t c, struct __locale_t *locale)
{
  (void) locale;
#ifdef _MB_CAPABLE
  c = _jp2uc_l (c, locale);
  enum category cat = category (c);
  // exclude "<noBreak>"?
  return cat == CAT_Zs
      || c == '\t';
#else
  return c < 0x100 ? isblank (c) : 0;
#endif /* _MB_CAPABLE */
}
