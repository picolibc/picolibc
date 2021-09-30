/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <wctype.h>
#include "local.h"

wint_t
towupper_l (wint_t c, struct __locale_t *locale)
{
  (void) locale;
#ifdef _MB_CAPABLE
  return towctrans_l (c, WCT_TOUPPER, locale);
#else
  return towupper (c);
#endif /* _MB_CAPABLE */
}
