/* Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

     Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse
     or promote products derived from this software without specific
     prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
FUNCTION
	<<wctype>>, <<wctype_l>>---get wide-character classification type

INDEX
	wctype

INDEX
	wctype_l

SYNOPSIS
	#include <wctype.h>
	wctype_t wctype(const char *<[c]>);

	#include <wctype.h>
	wctype_t wctype_l(const char *<[c]>, locale_t <[locale]>);


DESCRIPTION
<<wctype>> is a function which takes a string <[c]> and gives back
the appropriate wctype_t type value associated with the string,
if one exists.  The following values are guaranteed to be recognized:
"alnum", "alpha", "blank", "cntrl", "digit", "graph", "lower", "print",
"punct", "space", "upper", and "xdigit".

<<wctype_l>> is like <<wctype>> but performs the function based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<wctype>>, <<wctype_l>> return 0 and sets <<errno>> to <<EINVAL>> if the
given name is invalid.  Otherwise, it returns a valid non-zero wctype_t
value.

PORTABILITY
<<wctype>> is C99.
<<wctype_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <string.h>
#include <reent.h>
#include <wctype.h>
#include <errno.h>
#include "local.h"

wctype_t
_wctype_r (struct _reent *r,
	const char *c)
{
  switch (*c)
    {
    case 'a':
      if (!strcmp (c, "alnum"))
        return WC_ALNUM; 
      else if (!strcmp (c, "alpha"))
        return WC_ALPHA;
      break;
    case 'b':
      if (!strcmp (c, "blank"))
        return WC_BLANK;
      break;
    case 'c':
      if (!strcmp (c, "cntrl"))
        return WC_CNTRL;
      break;
    case 'd':
      if (!strcmp (c, "digit"))
        return WC_DIGIT;
      break;
    case 'g':
      if (!strcmp (c, "graph"))
        return WC_GRAPH;
      break;
    case 'l':
      if (!strcmp (c, "lower"))
        return WC_LOWER;
      break;
    case 'p':
      if (!strcmp (c, "print"))
        return WC_PRINT;
      else if (!strcmp (c, "punct"))
        return WC_PUNCT;
      break;
    case 's':
      if (!strcmp (c, "space"))
        return WC_SPACE;
      break;
    case 'u':
      if (!strcmp (c, "upper"))
        return WC_UPPER;
      break;
    case 'x':
      if (!strcmp (c, "xdigit"))
        return WC_XDIGIT;
      break;
    }

  /* otherwise invalid */
  _REENT_ERRNO(r) = EINVAL;
  return 0;
}

#ifndef _REENT_ONLY
wctype_t
wctype (const char *c)
{
  return _wctype_r (_REENT, c);
}
#endif /* !_REENT_ONLY */
