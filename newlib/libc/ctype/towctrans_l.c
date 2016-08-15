#include <_ansi.h>
#include <wctype.h>

wint_t
towctrans_l (wint_t c, wctrans_t w, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return towctrans (c, w);
}
