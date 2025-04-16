/*
Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com>
 */
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

size_t
wcrtomb (char *__restrict s,
	wchar_t wc,
	mbstate_t *__restrict ps)
{
  int retval = 0;
  char buf[10];

#ifdef __MB_CAPABLE
  if (ps == NULL)
    {
      static mbstate_t _wcrtomb_state;
      ps = &_wcrtomb_state;
    }
#endif

  if (s == NULL)
    retval = __WCTOMB (buf, L'\0', ps);
  else
    retval = __WCTOMB (s, wc, ps);

  if (retval == -1)
    {
#ifdef __MB_CAPABLE
      ps->__count = 0;
#endif
      errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}
