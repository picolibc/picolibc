/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
FUNCTION
<<mbstowcs>>---minimal multibyte string to wide char converter

INDEX
	mbstowcs

SYNOPSIS
	#include <stdlib.h>
	int mbstowcs(wchar_t *restrict <[pwc]>, const char *restrict <[s]>, size_t <[n]>);

DESCRIPTION
When _MB_CAPABLE is not defined, this is a minimal ANSI-conforming 
implementation of <<mbstowcs>>.  In this case, the
only ``multi-byte character sequences'' recognized are single bytes,
and they are ``converted'' to wide-char versions simply by byte
extension.

When _MB_CAPABLE is defined, this uses a state variable to allow state
dependent decoding.  The result is based on the locale setting which
may be restricted to a defined set of locales.

RETURNS
This implementation of <<mbstowcs>> returns <<0>> if
<[s]> is <<NULL>> or is the empty string; 
it returns <<-1>> if _MB_CAPABLE and one of the
multi-byte characters is invalid or incomplete;
otherwise it returns the minimum of: <<n>> or the
number of multi-byte characters in <<s>> plus 1 (to
compensate for the nul character).
If the return value is -1, the state of the <<pwc>> string is
indeterminate.  If the input has a length of 0, the output
string will be modified to contain a wchar_t nul terminator.

PORTABILITY
<<mbstowcs>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<mbstowcs>> requires no supporting OS subroutines.
*/

#ifndef _REENT_ONLY

#include <newlib.h>
#include <stdlib.h>
#include <wchar.h>
#include "local.h"

size_t
mbstowcs (wchar_t *__restrict pwcs,
        const char *__restrict s,
        size_t n)
{
#ifdef _MB_CAPABLE
  mbstate_t state;
  state.__count = 0;
  
  size_t ret = 0;
  char *t = (char *)s;
  int bytes;

  if (!pwcs)
    n = (size_t) 1; /* Value doesn't matter as long as it's not 0. */
  while (n > 0)
    {
      bytes = __MBTOWC (pwcs, t, MB_CUR_MAX, &state);
      if (bytes < 0)
	{
	  state.__count = 0;
	  return -1;
	}
      else if (bytes == 0)
	break;
      t += bytes;
      ++ret;
      if (pwcs)
	{
	  ++pwcs;
	  --n;
	}
    }
  return ret;
#else /* not _MB_CAPABLE */
  
  int count = 0;
  
  if (n != 0) {
    do {
      if ((*pwcs++ = (wchar_t) *s++) == 0)
	break;
      count++;
    } while (--n != 0);
  }
  
  return count;
#endif /* not _MB_CAPABLE */
}

#endif /* !_REENT_ONLY */
