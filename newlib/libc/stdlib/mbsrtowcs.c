#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <errno.h>

size_t
_DEFUN (_mbsrtowcs_r, (r, dst, src, n, ps), 
	struct _reent *r _AND
	wchar_t *dst _AND
	const char **src _AND
	size_t n _AND
	mbstate_t *ps)
{
  wchar_t *ptr = dst;
  size_t max = n;
  int bytes;

#ifdef MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(r);
      ps = &(_REENT_MBSRTOWCS_STATE(r));
    }
#endif

  while (n > 0)
    {
      bytes = _mbtowc_r (r, ptr, *src, MB_CUR_MAX, ps);
      if (bytes > 0)
	{
	  *src += bytes;
	  ++ptr;
	  --n;
	}
      else if (bytes == -2)
	{
	  *src += MB_CUR_MAX;
	}
      else if (bytes == 0)
	{
	  *src = NULL;
	  return (size_t)(ptr - dst);
	}
      else
	{
	  ps->__count = 0;
	  r->_errno = EILSEQ;
	  return (size_t)-1;
	}
    }

  return (size_t)max;
}

#ifndef _REENT_ONLY
size_t
_DEFUN (mbsrtowcs, (dst, src, len, ps),
	wchar_t *dst _AND
	const char **src _AND
	size_t len _AND
	mbstate_t *ps)
{
  return _mbsrtowcs_r (_REENT, dst, src, len, ps);
}
#endif /* !_REENT_ONLY */
