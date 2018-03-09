/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswspace_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  c = _jp2uc_l (c, locale);
  enum category cat = category (c);
  // exclude "<noBreak>"?
  return cat == CAT_Zs
      || cat == CAT_Zl || cat == CAT_Zp // Line/Paragraph Separator
      || (c >= 0x9 && c <= 0xD);
#else
  return c < 0x100 ? isspace (c) : 0;
#endif /* _MB_CAPABLE */
}
