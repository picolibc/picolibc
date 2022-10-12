/*
Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com>
 */
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

#ifndef _REENT_ONLY
size_t
wcrtomb (char *__restrict s,
	wchar_t wc,
	mbstate_t *__restrict ps)
{
  int retval = 0;
  char buf[10];

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      static NEWLIB_THREAD_LOCAL mbstate_t _wcrtomb_state;
      ps = &_wcrtomb_state;
    }
#endif

  if (s == NULL)
    retval = __WCTOMB (buf, L'\0', ps);
  else
    retval = __WCTOMB (s, wc, ps);

  if (retval == -1)
    {
      ps->__count = 0;
      _REENT_ERRNO(reent) = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}
#endif /* !_REENT_ONLY */
