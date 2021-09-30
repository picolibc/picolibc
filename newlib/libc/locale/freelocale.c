/*
Copyright (c) 1996 - 2002 FreeBSD Project
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
4. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/*
FUNCTION
	<<freelocale>>---free resources allocated for a locale object

INDEX
	freelocale

INDEX
	_freelocale_r

SYNOPSIS
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

#define _DEFAULT_SOURCE
#include <newlib.h>
#include <stdlib.h>
#include "setlocale.h"

void
freelocale (struct __locale_t *locobj)
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
	free ((void *) locobj->lc_cat[i].ptr);
	free (locobj->lc_cat[i].buf);
      }
#endif /* __HAVE_LOCALE_INFO__ */
  free (locobj);
#endif /* _MB_CAPABLE */
}
