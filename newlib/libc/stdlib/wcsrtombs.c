#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <errno.h>

size_t
_DEFUN (_wcsrtombs_r, (r, dst, src, len, ps),
	struct _reent *r _AND
	char *dst _AND
	const wchar_t **src _AND
	size_t len _AND
	mbstate_t *ps)
{
  char *ptr = dst;
  char buff[10];
  int i, n;
  int count;
  wint_t wch;

#ifdef MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(r);
      ps = &(_REENT_WCSRTOMBS_STATE(r));
    }
#endif

  n = (int)len;

  while (n > 0)
    {
      wchar_t *pwcs = (wchar_t *)(*src);
      int count = ps->__count;
      wint_t wch = ps->__value.__wch;
      int bytes = _wctomb_r (r, buff, *pwcs, ps);
      if (bytes == -1)
	{
	  r->_errno = EILSEQ;
	  ps->__count = 0;
	  return (size_t)-1;
	}
      if (bytes <= n)
	{
	  for (i = 0; i < bytes; ++i)
	    *ptr++ = buff[i];
          
	  if (*pwcs == 0x00)
	    {
	      *src = NULL;
	      ps->__count = 0;
	      return (size_t)(ptr - dst - 1);
	    }
	  ++(*src);
	}
      else
	{
	  /* not enough room, we must back up state to before _wctomb_r call */
	  ps->__count = count;
	  ps->__value.__wch = wch;
	}
      n -= bytes;
    }

  return (size_t)(ptr - dst);
} 

#ifndef _REENT_ONLY
size_t
_DEFUN (wcsrtombs, (dst, src, len, ps),
	char *dst _AND
	const wchar_t **src _AND
	size_t len _AND
	mbstate_t *ps)
{
  return _wcsrtombs_r (_REENT, dst, src, len, ps);
}
#endif /* !_REENT_ONLY */
