#include <reent.h>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "local.h"

int
wctob (wint_t wc)
{
  mbstate_t mbs;
  unsigned char pmb[MB_LEN_MAX];

  if (wc == WEOF)
    return EOF;

  /* Put mbs in initial state. */
  memset (&mbs, '\0', sizeof (mbs));

  _REENT_CHECK_MISC(_REENT);

  return __wctomb (_REENT, (char *) pmb, wc, __locale_charset (), &mbs) == 1
	  ? (int) pmb[0] : EOF;
}
