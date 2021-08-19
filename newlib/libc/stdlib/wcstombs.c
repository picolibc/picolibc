/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
FUNCTION
<<wcstombs>>---minimal wide char string to multibyte string converter

INDEX
	wcstombs

SYNOPSIS
	#include <stdlib.h>
	size_t wcstombs(char *restrict <[s]>, const wchar_t *restrict <[pwc]>, size_t <[n]>);

DESCRIPTION
When _MB_CAPABLE is not defined, this is a minimal ANSI-conforming 
implementation of <<wcstombs>>.  In this case,
all wide-characters are expected to represent single bytes and so
are converted simply by casting to char.

When _MB_CAPABLE is defined, this routine uses a state variable to
allow state dependent decoding.  The result is based on the locale
setting which may be restricted to a defined set of locales.

RETURNS
This implementation of <<wcstombs>> returns <<0>> if
<[s]> is <<NULL>> or is the empty string; 
it returns <<-1>> if _MB_CAPABLE and one of the
wide-char characters does not represent a valid multi-byte character;
otherwise it returns the minimum of: <<n>> or the
number of bytes that are transferred to <<s>>, not including the
nul terminator.

If the return value is -1, the state of the <<pwc>> string is
indeterminate.  If the input has a length of 0, the output
string will be modified to contain a wchar_t nul terminator if
<<n>> > 0.

PORTABILITY
<<wcstombs>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<wcstombs>> requires no supporting OS subroutines.
*/

#ifndef _REENT_ONLY

#include <newlib.h>
#include <stdlib.h>
#include <wchar.h>
#include "local.h"

size_t
wcstombs (char          *__restrict s,
        const wchar_t *__restrict pwcs,
        size_t         n)
{
#ifdef _MB_CAPABLE
  mbstate_t state;
  state.__count = 0;

  char *ptr = s;
  size_t max = n;
  char buff[8];
  int i, bytes, num_to_copy;

  if (s == NULL)
    {
      size_t num_bytes = 0;
      while (*pwcs != 0)
	{
	  bytes = __WCTOMB (buff, *pwcs++, &state);
	  if (bytes == -1)
	    return -1;
	  num_bytes += bytes;
	}
      return num_bytes;
    }
  else
    {
      while (n > 0)
        {
          bytes = __WCTOMB (buff, *pwcs, &state);
          if (bytes == -1)
            return -1;
          num_to_copy = (n > (size_t) bytes ? bytes : (int)n);
          for (i = 0; i < num_to_copy; ++i)
            *ptr++ = buff[i];

          if (*pwcs == 0x00)
            return ptr - s - (n >= (size_t) bytes);
          ++pwcs;
          n -= num_to_copy;
        }
      return max;
    }
#else /* not _MB_CAPABLE */
  int count = 0;
  
  if (n != 0) {
    do {
      if ((*s++ = (char) *pwcs++) == 0)
	break;
      count++;
    } while (--n != 0);
  }
  
  return count;
#endif /* not _MB_CAPABLE */
}

#endif /* !_REENT_ONLY */
