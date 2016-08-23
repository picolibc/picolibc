/*
FUNCTION
	<<freelocale>>---free resources allocated for a locale object

INDEX
	freelocale

INDEX
	_freelocale_r

ANSI_SYNOPSIS
	#include <locale.h>
	locale_t freelocale(locale_t <[locobj]>);

	locale_t _freelocale_r(void *<[reent]>, locale_t <[locobj]>);

DESCRIPTION
The <<freelocale>> function shall cause the resources allocated for a
locale object returned by a call to the <<newlocale>> or <<duplocale>>
functions to be released.

The behavior is undefined if the <[locobj]> argument is the special locale
object LC_GLOBAL_LOCALE or is not a valid locale object handle.

Any use of a locale object that has been freed results in undefined
behavior.

RETURNS
None.

PORTABILITY
<<freelocale>> is POSIX-1.2008.
*/

#include <newlib.h>
#include <reent.h>
#include <stdlib.h>
#include "setlocale.h"

void
_freelocale_r (struct _reent *p, struct __locale_t *locobj)
{
  /* Nothing to do on !_MB_CAPABLE targets. */
#ifdef _MB_CAPABLE
  /* Sanity check.  The "C" locale is static, don't try to free it. */
  if (!locobj || locobj == __get_C_locale () || locobj == LC_GLOBAL_LOCALE)
    return;
#ifdef __HAVE_LOCALE_INFO__
  for (int i = 1; i < _LC_LAST; ++i)
    if (locobj->lc_cat[i].buf)
      {
	_free_r (p, (void *) locobj->lc_cat[i].ptr);
	_free_r (p, locobj->lc_cat[i].buf);
      }
#endif /* __HAVE_LOCALE_INFO__ */
  _free_r (p, locobj);
#endif /* _MB_CAPABLE */
}

void
freelocale (struct __locale_t *locobj)
{
  _freelocale_r (_REENT, locobj);
}
