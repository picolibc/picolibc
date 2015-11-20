/*
(C) Copyright IBM Corp. 2009

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include "local.h"

long double
wcstold (const wchar_t *__restrict nptr, wchar_t **__restrict endptr)
{
#ifdef _LDBL_EQ_DBL
/* On platforms where long double is as wide as double.  */
  return wcstod(nptr, endptr);

#else /* This is a duplicate of the code in wcstod.c, but converted to long double.  */

  static const mbstate_t initial;
  mbstate_t mbs;
  long double val;
  char *buf, *end;
  const wchar_t *wcp;
  size_t len;

  while (iswspace (*nptr))
    nptr++;

  /* Convert the supplied numeric wide char string to multibyte.  */
  wcp = nptr;
  mbs = initial;
  if ((len = wcsrtombs (NULL, &wcp, 0, &mbs)) == (size_t)-1)
    {
      if (endptr != NULL)
	*endptr = (wchar_t *) nptr;
      return 0.0L;
    }

  if ((buf = malloc (len + 1)) == NULL)
    return 0.0L;

  mbs = initial;
  wcsrtombs (buf, &wcp, len + 1, &mbs);

  val = strtold (buf, &end);

  /* We only know where the number ended in the _multibyte_
     representation of the string. If the caller wants to know
     where it ended, count multibyte characters to find the
     corresponding position in the wide char string.  */

  if (endptr != NULL)
    {
      /* The only valid multibyte char in a float converted by
	 strtold/wcstold is the radix char.  What we do here is,
	 figure out if the radix char was in the valid leading
	 float sequence in the incoming string.  If so, the
	 multibyte float string is strlen (radix char) - 1 bytes
	 longer than the incoming wide char string has characters.
	 To fix endptr, reposition end as if the radix char was
	 just one byte long.  The resulting difference (end - buf)
	 is then equivalent to the number of valid wide characters
	 in the input string.  */
      len = strlen (localeconv ()->decimal_point);
      if (len > 1)
	{
	  char *d = strstr (buf, localeconv ()->decimal_point);

	  if (d && d < end)
	    end -= len - 1;
	}

      *endptr = (wchar_t *) nptr + (end - buf);
    }

  free (buf);

  return val;
#endif /* _LDBL_EQ_DBL */
}
