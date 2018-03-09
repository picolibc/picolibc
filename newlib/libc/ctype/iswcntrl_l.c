/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "categories.h"

int
iswcntrl_l (wint_t c, struct __locale_t *locale)
{
#ifdef _MB_CAPABLE
  c = _jp2uc_l (c, locale);
  enum category cat = category (c);
  return cat == CAT_Cc
      || cat == CAT_Zl || cat == CAT_Zp // Line/Paragraph Separator
      ;
#else
  return c < 0x100 ? iscntrl (c) : 0;
#endif /* _MB_CAPABLE */
}
