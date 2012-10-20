#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <string.h>
#include "local.h"

wint_t
btowc (int c)
{
  mbstate_t mbs;
  int retval = 0;
  wchar_t pwc;
  unsigned char b;

  if (c == EOF)
    return WEOF;

  b = (unsigned char)c;

  /* Put mbs in initial state. */
  memset (&mbs, '\0', sizeof (mbs));

  _REENT_CHECK_MISC(_REENT);

  retval = __mbtowc (_REENT, &pwc, (const char *) &b, 1,
		     __locale_charset (), &mbs);

  if (retval != 0 && retval != 1)
    return WEOF;

  return (wint_t)pwc;
}
