#include <reent.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "local.h"

int
wctob (wint_t c)
{
  mbstate_t mbs;
  int retval = 0;
  unsigned char pwc;

  /* Put mbs in initial state. */
  memset (&mbs, '\0', sizeof (mbs));

  _REENT_CHECK_MISC(_REENT);

  retval = __wctomb (_REENT, &pwc, c, __locale_charset (), &mbs);

  if (c == EOF || retval != 1)
    return WEOF;
  else
    return (int)pwc;
}
