/* Parts of this code are originally taken from FreeBSD. */
/*
 * Copyright (c) 1996 - 2002 FreeBSD Project
 * Copyright (c) 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
FUNCTION
	<<duplocale>>---duplicate a locale object

INDEX
	duplocale

INDEX
	_duplocale_r

SYNOPSIS
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

#define _DEFAULT_SOURCE
#include <newlib.h>
#include <stdlib.h>
#include "setlocale.h"

struct __locale_t *
duplocale (struct __locale_t *locobj)
{
  struct __locale_t tmp_locale, *new_locale;
  int i;

  (void) locobj;
  (void) i;
  (void) new_locale;
  (void) tmp_locale;
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
  new_locale = (struct __locale_t *) calloc (1, sizeof *new_locale);
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
	free ((void *) tmp_locale.lc_cat[i].ptr);
	free (tmp_locale.lc_cat[i].buf);
      }
#endif /* __HAVE_LOCALE_INFO__ */

  return NULL;
#endif /* _MB_CAPABLE */
}
