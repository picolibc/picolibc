/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswlower_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  c = _jp2uc_l (c, locale);
  // The wide-character class "lower" contains at least those characters wc
  // which are equal to towlower(wc) and different from towupper(wc).
  enum category cat = category (c);
  return cat == CAT_Ll || (cat == CAT_LC && towlower (c) == c);
#else
  return c < 0x100 ? islower (c) : 0;
#endif /* _MB_CAPABLE */
}
