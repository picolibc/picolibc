#include <newlib.h>
#include <reent.h>
#include <stdlib.h>
#include "setlocale.h"

struct __locale_t *
_uselocale_r (struct _reent *p, struct __locale_t *newloc)
{
  struct __locale_t *current_locale;

  current_locale = __get_locale_r (p);
  if (!current_locale)
    current_locale = LC_GLOBAL_LOCALE;
  if (newloc == LC_GLOBAL_LOCALE)
    p->_locale = NULL;
  else if (newloc)
    p->_locale = newloc;
  return current_locale;
}

#ifndef _REENT_ONLY
struct __locale_t *
uselocale (struct __locale_t *newloc)
{
  return _uselocale_r (_REENT, newloc);
}
#endif
