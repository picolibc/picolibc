/*
FUNCTION
	<<duplocale>>---duplicate a locale object

INDEX
	duplocale

INDEX
	_duplocale_r

ANSI_SYNOPSIS
	#include <locale.h>
	locale_t duplocale(locale_t <[locobj]>);

	locale_t _duplocale_r(void *<[reent]>, locale_t <[locobj]>);

DESCRIPTION
The <<duplocale>> function shall create a duplicate copy of the locale
object referenced by the <[locobj]> argument.

If the <[locobj]> argument is LC_GLOBAL_LOCALE, duplocale() shall create
a new locale object containing a copy of the global locale determined by
the setlocale() function.

The behavior is undefined if the <[locobj]> argument is not a valid locale
object handle.

RETURNS
Upon successful completion, the <<duplocale>> function shall return a
handle for a new locale object.  Otherwise <<duplocale>> shall return
(locale_t) <<0>> and set errno to indicate the error.

PORTABILITY
<<duplocale>> is POSIX-1.2008.
*/

#include <newlib.h>
#include <reent.h>
#include <stdlib.h>
#include "setlocale.h"

struct __locale_t *
_duplocale_r (struct _reent *p, struct __locale_t *locobj)
{
  struct __locale_t tmp_locale, *new_locale;
  int i;

#ifndef _MB_CAPABLE
  return __get_C_locale ();
#else /* _MB_CAPABLE */
  /* LC_GLOBAL_LOCALE denotes the global locale. */
  if (locobj == LC_GLOBAL_LOCALE)
    locobj = __get_global_locale ();
  /* The "C" locale is used statically, never copied. */
  else if (locobj == __get_C_locale ())
    return __get_C_locale ();
  /* Copy locale content. */
  tmp_locale = *locobj;
#ifdef __HAVE_LOCALE_INFO__
  for (i = 1; i < _LC_LAST; ++i)
    if (locobj->lc_cat[i].buf)
      {
	/* If the object is not a "C" locale category, copy it.  Just call
	   __loadlocale.  It knows what to do to replicate the category. */
	tmp_locale.lc_cat[i].ptr = NULL;
	tmp_locale.lc_cat[i].buf = NULL;
	tmp_locale.categories[i][0] = '\0';	/* __loadlocale tests this! */
	if (!__loadlocale (&tmp_locale, i, locobj->categories[i]))
	  goto error;
      }
#endif /* __HAVE_LOCALE_INFO__ */
  /* Allocate new locale_t. */
  new_locale = (struct __locale_t *) _calloc_r (p, 1, sizeof *new_locale);
  if (!new_locale)
    goto error;

  *new_locale = tmp_locale;
  return new_locale;

error:
  /* An error occured while we had already (potentially) allocated memory.
     Free memory and return NULL.  errno is supposed to be set already. */
#ifdef __HAVE_LOCALE_INFO__
  while (--i > 0)
    if (tmp_locale.lc_cat[i].buf)
      {
	_free_r (p, (void *) tmp_locale.lc_cat[i].ptr);
	_free_r (p, tmp_locale.lc_cat[i].buf);
      }
#endif /* __HAVE_LOCALE_INFO__ */

  return NULL;
#endif /* _MB_CAPABLE */
}

#ifndef _REENT_ONLY
struct __locale_t *
duplocale (struct __locale_t *locobj)
{
  return _duplocale_r (_REENT, locobj);
}
#endif
