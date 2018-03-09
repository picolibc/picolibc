/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswpunct_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  //return !iswalnum (c) && iswgraph (c);
  c = _jp2uc_l (c, locale);
  enum category cat = category (c);
  return cat == CAT_Pc || cat == CAT_Pd || cat == CAT_Pe || cat == CAT_Pf || cat == CAT_Pi || cat == CAT_Po || cat == CAT_Ps
      || cat == CAT_Sm // Math Symbols
      // the following are included for backwards consistency:
      || cat == CAT_Sc // Currency Symbols
      || cat == CAT_Sk // Modifier_Symbol
      || cat == CAT_So // Other_Symbol
      || cat == CAT_No // Other_Number
      ;
#else
  return c < (wint_t)0x100 ? ispunct (c) : 0;
#endif /* _MB_CAPABLE */
}
