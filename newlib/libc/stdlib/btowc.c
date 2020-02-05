/* Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com> */
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
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

  retval = __MBTOWC (&pwc, (const char *) &b, 1, &mbs);

  if (retval != 0 && retval != 1)
    return WEOF;

  return (wint_t)pwc;
}
