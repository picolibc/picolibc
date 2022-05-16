#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

#ifdef _REENT_THREAD_LOCAL
_Thread_local _mbstate_t _tls_wcrtomb_state;
#endif

size_t
_wcrtomb_r (struct _reent *ptr,
	char *s,
	wchar_t wc,
	mbstate_t *ps)
{
  int retval = 0;
  char buf[10];

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(ptr);
      ps = &(_REENT_WCRTOMB_STATE(ptr));
    }
#endif

  if (s == NULL)
    retval = __WCTOMB (ptr, buf, L'\0', ps);
  else
    retval = __WCTOMB (ptr, s, wc, ps);

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
wcrtomb (char *__restrict s,
	wchar_t wc,
	mbstate_t *__restrict ps)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  return _wcrtomb_r (_REENT, s, wc, ps);
#else
  int retval = 0;
  struct _reent *reent = _REENT;
  char buf[10];

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(reent);
      ps = &(_REENT_WCRTOMB_STATE(reent));
    }
#endif

  if (s == NULL)
    retval = __WCTOMB (reent, buf, L'\0', ps);
  else
    retval = __WCTOMB (reent, s, wc, ps);

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
