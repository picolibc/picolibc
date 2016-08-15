#include <_ansi.h>
#include <wctype.h>

int
iswctype_l (wint_t c, wctype_t desc, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswctype (c, desc);
}
