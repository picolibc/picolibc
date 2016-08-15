#include <_ansi.h>
#include <newlib.h>
#include <wctype.h>
#include "local.h"

wint_t
towlower_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return towlower (c);
}
