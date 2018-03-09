/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <wctype.h>
#include "local.h"

wint_t
towupper_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  return towctrans_l (c, WCT_TOUPPER, locale);
#else
  return towupper (c);
#endif /* _MB_CAPABLE */
}
