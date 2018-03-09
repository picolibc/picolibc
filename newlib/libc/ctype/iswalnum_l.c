/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswalnum_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  //return iswalpha (c) || iswdigit (c);
  c = _jp2uc_l (c, locale);
  enum category cat = category (c);
  return cat == CAT_LC || cat == CAT_Lu || cat == CAT_Ll || cat == CAT_Lt
      || cat == CAT_Lm || cat == CAT_Lo
      || cat == CAT_Nl // Letter_Number
      || cat == CAT_Nd // Decimal_Number
      ;
#else
  return c < (wint_t)0x100 ? isalnum (c) : 0;
#endif /* _MB_CAPABLE */
}
