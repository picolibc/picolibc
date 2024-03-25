/*
FUNCTION
	<<getlocalename_l>>---create or modify a locale object

INDEX
	getlocalename_l

INDEX
	_getlocalename_l_r

SYNOPSIS
	#include <locale.h>
	locale_t getlocalename_l(int <[category]>, locale_t <[locobj]>);

	locale_t _getlocalename_l_r(void *<[reent]>, int <[category]>,
			      locale_t <[locobj]>);

DESCRIPTION
The <<getlocalename_l>> function shall return the locale name for the
given locale category of the locale object locobj, or of the global
locale if locobj is the special locale object LC_GLOBAL_LOCALE.

The category argument specifies the locale category to be queried. If
the value is LC_ALL or is not a supported locale category value (see
<<setlocale>>), <<getlocalename_l>> shall fail.

The behavior is undefined if the locobj argument is neither the special
locale object LC_GLOBAL_LOCALE nor a valid locale object handle.

RETURNS
Upon successful completion, <<getlocalename_l>> shall return a pointer
to a string containing the locale name; otherwise, a null pointer shall
be returned.

If locobj is LC_GLOBAL_LOCALE, the returned string pointer might be
invalidated or the string content might be overwritten by a subsequent
call in the same thread to <<getlocalename_l>> with LC_GLOBAL_LOCALE;
the returned string pointer might also be invalidated if the calling
thread is terminated. Otherwise, the returned string pointer and content
shall remain valid until the locale object locobj is used in a call to
<<freelocale>> or as the base argument in a successful call to
<<newlocale>>.

No errors are defined.

PORTABILITY
<<getlocalename_l>> is POSIX-1.2008 since Base Specification Issue 8
*/

#include <newlib.h>
#include "setlocale.h"

const char *
getlocalename_l (int category, struct __locale_t *locobj)
{
  if (category <= LC_ALL || category > LC_MESSAGES)
    return NULL;
#ifndef _MB_CAPABLE
  (void) locobj;
  return "C";
#else
  if (locobj == LC_GLOBAL_LOCALE)
    {
        static NEWLIB_THREAD_LOCAL char getlocalename_l_buf[32 /*ENCODING + 1*/];
        /* getlocalename_l is supposed to return the value in a
           thread-safe manner.  This requires to copy over the
           category string into thread-local storage. */
        strcpy (getlocalename_l_buf,
	      __get_global_locale ()->categories[category]);
        return getlocalename_l_buf;
    }
  return locobj->categories[category];
#endif
}
