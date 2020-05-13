/*
Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com>
 */
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

  return __WCTOMB ((char *) pmb, wc, &mbs) == 1 ? (int) pmb[0] : EOF;
}
