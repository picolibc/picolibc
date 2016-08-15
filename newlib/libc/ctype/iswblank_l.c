#include <_ansi.h>
#include <wctype.h>

int
iswblank_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswblank (c);
}
