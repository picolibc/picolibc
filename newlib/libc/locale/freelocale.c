#include <newlib.h>
#include <reent.h>
#include <stdlib.h>
#include "setlocale.h"

void
_freelocale_r (struct _reent *p, struct __locale_t *locobj)
{
  /* Sanity check.  The "C" locale is static, don't try to free it. */
  if (!locobj || locobj == &__C_locale || locobj == LC_GLOBAL_LOCALE)
    return;
#ifdef __HAVE_LOCALE_INFO__
  for (int i = 1; i < _LC_LAST; ++i)
    if (locobj->lc_cat[i].buf)
      {
	_free_r (p, (void *) locobj->lc_cat[i].ptr);
	_free_r (p, locobj->lc_cat[i].buf);
      }
#endif
  _free_r (p, locobj);
}

void
freelocale (struct __locale_t *locobj)
{
  _freelocale_r (_REENT, locobj);
}
