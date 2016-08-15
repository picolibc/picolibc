#include <_ansi.h>
#include <wctype.h>

wctrans_t
wctrans_l (const char *c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return wctrans (c);
}
