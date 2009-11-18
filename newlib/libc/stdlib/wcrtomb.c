#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

size_t
_DEFUN (_wcrtomb_r, (ptr, s, wc, ps),
	struct _reent *ptr _AND
	char *s _AND
	wchar_t wc _AND
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
    retval = __wctomb (ptr, buf, L'\0', __locale_charset (), ps);
  else
    retval = __wctomb (ptr, s, wc, __locale_charset (), ps);

  if (retval == -1)
    {
      ps->__count = 0;
      ptr->_errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}

#ifndef _REENT_ONLY
size_t
_DEFUN (wcrtomb, (s, wc, ps),
	char *s _AND
	wchar_t wc _AND
	mbstate_t *ps)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  return _wcrtomb_r (_REENT, s, wc, ps);
#else
  int retval = 0;
  char buf[10];

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(_REENT);
      ps = &(_REENT_WCRTOMB_STATE(_REENT));
    }
#endif

  if (s == NULL)
    retval = __wctomb (_REENT, buf, L'\0', __locale_charset (), ps);
  else
    retval = __wctomb (_REENT, s, wc, __locale_charset (), ps);

  if (retval == -1)
    {
      ps->__count = 0;
      _REENT->_errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
#endif /* !_REENT_ONLY */
