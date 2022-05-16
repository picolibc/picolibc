#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "local.h"

#ifdef _REENT_THREAD_LOCAL
_Thread_local _mbstate_t _tls_mbrtowc_state;
#endif

size_t
_mbrtowc_r (struct _reent *ptr,
	wchar_t *pwc,
	const char *s,
	size_t n,
	mbstate_t *ps)
{
  int retval = 0;

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(ptr);
      ps = &(_REENT_MBRTOWC_STATE(ptr));
    }
#endif

  if (s == NULL)
    retval = __MBTOWC (ptr, NULL, "", 1, ps);
  else
    retval = __MBTOWC (ptr, pwc, s, n, ps);

  if (retval == -1)
    {
      ps->__count = 0;
      _REENT_ERRNO(ptr) = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}

#ifndef _REENT_ONLY
size_t
mbrtowc (wchar_t *__restrict pwc,
	const char *__restrict s,
	size_t n,
	mbstate_t *__restrict ps)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  return _mbrtowc_r (_REENT, pwc, s, n, ps);
#else
  int retval = 0;
  struct _reent *reent = _REENT;

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(reent);
      ps = &(_REENT_MBRTOWC_STATE(reent));
    }
#endif

  if (s == NULL)
    retval = __MBTOWC (reent, NULL, "", 1, ps);
  else
    retval = __MBTOWC (reent, pwc, s, n, ps);

  if (retval == -1)
    {
      ps->__count = 0;
      _REENT_ERRNO(reent) = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
#endif /* !_REENT_ONLY */
