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

#define _DEFAULT_SOURCE
#include <newlib.h>
#include <stdlib.h>
#include "setlocale.h"

#ifndef _REENT_ONLY

struct __locale_t *
uselocale (struct __locale_t *newloc)
{
  struct __locale_t *current_locale;

  (void) newloc;
  current_locale = __get_current_locale();
#ifdef __HAVE_LOCALE_INFO__
  if (newloc == LC_GLOBAL_LOCALE)
    _locale = NULL;
  else if (newloc)
    _locale = newloc;
#endif
  return current_locale;
}

#endif
