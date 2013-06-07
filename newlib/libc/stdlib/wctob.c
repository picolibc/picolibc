#include <reent.h>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "local.h"

int
wctob (wint_t wc)
{
  struct _reent *reent;
  mbstate_t mbs;
  unsigned char pmb[MB_LEN_MAX];

  if (wc == WEOF)
    return EOF;

  /* Put mbs in initial state. */
  memset (&mbs, '\0', sizeof (mbs));

  reent = _REENT;
  _REENT_CHECK_MISC(reent);

  return __wctomb (reent, (char *) pmb, wc, __locale_charset (), &mbs) == 1
	  ? (int) pmb[0] : EOF;
}
