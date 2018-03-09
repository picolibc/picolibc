/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <wctype.h>
#include "local.h"

int
iswctype_l (wint_t c, wctype_t desc, struct __locale_t *locale)
{
  switch (desc)
    {
    case WC_ALNUM:
      return iswalnum_l (c, locale);
    case WC_ALPHA:
      return iswalpha_l (c, locale);
    case WC_BLANK:
      return iswblank_l (c, locale);
    case WC_CNTRL:
      return iswcntrl_l (c, locale);
    case WC_DIGIT:
      return iswdigit_l (c, locale);
    case WC_GRAPH:
      return iswgraph_l (c, locale);
    case WC_LOWER:
      return iswlower_l (c, locale);
    case WC_PRINT:
      return iswprint_l (c, locale);
    case WC_PUNCT:
      return iswpunct_l (c, locale);
    case WC_SPACE:
      return iswspace_l (c, locale);
    case WC_UPPER:
      return iswupper_l (c, locale);
    case WC_XDIGIT:
      return iswxdigit_l (c, locale);
    default:
      return 0; /* eliminate warning */
    }

  /* otherwise unknown */
  return 0;
}
