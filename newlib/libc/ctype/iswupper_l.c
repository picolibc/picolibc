/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswupper_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  c = _jp2uc_l (c, locale);
  // The wide-character class "upper" contains at least those characters wc
  // which are equal to towupper(wc) and different from towlower(wc).
  enum category cat = category (c);
  return cat == CAT_Lu || (cat == CAT_LC && towupper (c) == c);
#else
  return c < 0x100 ? isupper (c) : 0;
#endif /* _MB_CAPABLE */
}
