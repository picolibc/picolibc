/*
FUNCTION
	<<uselocale>>---free resources allocated for a locale object

INDEX
	uselocale

INDEX
	_uselocale_r

SYNOPSIS
	#include <locale.h>
	locale_t uselocale(locale_t <[locobj]>);

	locale_t _uselocale_r(void *<[reent]>, locale_t <[locobj]>);

DESCRIPTION
The <<uselocale>> function shall set the current locale for the current
thread to the locale represented by newloc.

The value for the newloc argument shall be one of the following:

1. A value returned by the <<newlocale>> or <<duplocale>> functions

2. The special locale object descriptor LC_GLOBAL_LOCALE

3. (locale_t) <<0>>

Once the <<uselocale>> function has been called to install a thread-local
locale, the behavior of every interface using data from the current
locale shall be affected for the calling thread. The current locale for
other threads shall remain unchanged.

If the newloc argument is (locale_t) <<0>>, the object returned is the
current locale or LC_GLOBAL_LOCALE if there has been no previous call to
<<uselocale>> for the current thread.

If the newloc argument is LC_GLOBAL_LOCALE, the thread shall use the
global locale determined by the <<setlocale>> function.

RETURNS
Upon successful completion, the <<uselocale>> function shall return the
locale handle from the previous call for the current thread, or
LC_GLOBAL_LOCALE if there was no such previous call.  Otherwise,
<<uselocale>> shall return (locale_t) <<0>> and set errno to indicate
the error.


PORTABILITY
<<uselocale>> is POSIX-1.2008.
*/

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
