/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswgraph_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  //return iswprint (c, locale) && !iswspace (c, locale);
  c = _jp2uc_l (c, locale);
  enum category cat = category (c);
  return cat != -1
      && cat != CAT_Cc && cat != CAT_Cf
      && cat != CAT_Cs // Surrogate
      && cat != CAT_Zs
      && cat != CAT_Zl && cat != CAT_Zp // Line/Paragraph Separator
      ;
#else
  return iswprint_l (c, locale) && !iswspace_l (c, locale);
#endif /* _MB_CAPABLE */
}
