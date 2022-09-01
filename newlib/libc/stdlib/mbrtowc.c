/*
Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com>
 */
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "local.h"

#ifndef _REENT_ONLY
size_t
mbrtowc (wchar_t *__restrict pwc,
	const char *__restrict s,
	size_t n,
	mbstate_t *__restrict ps)
{
  int retval = 0;

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      static NEWLIB_THREAD_LOCAL mbstate_t _mbrtowc_state;
      ps = &_mbrtowc_state;
    }
#endif

  if (s == NULL)
    retval = __MBTOWC (NULL, "", 1, ps);
  else
    retval = __MBTOWC (pwc, s, n, ps);

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
