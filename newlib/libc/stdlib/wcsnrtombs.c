/*
FUNCTION
<<wcsrtombs>>, <<wcsnrtombs>>---convert a wide-character string to a character string

INDEX
	wcsrtombs
INDEX
	_wcsrtombs_r
INDEX
	wcsnrtombs
INDEX
	_wcsnrtombs_r

SYNOPSIS
	#include <wchar.h>
	size_t wcsrtombs(char *__restrict <[dst]>,
			 const wchar_t **__restrict <[src]>, size_t <[len]>,
			 mbstate_t *__restrict <[ps]>);

	#include <wchar.h>
	size_t _wcsrtombs_r(struct _reent *<[ptr]>, char *<[dst]>,
			    const wchar_t **<[src]>, size_t <[len]>,
			    mbstate_t *<[ps]>);

	#include <wchar.h>
	size_t wcsnrtombs(char *__restrict <[dst]>,
			  const wchar_t **__restrict <[src]>,
			  size_t <[nwc]>, size_t <[len]>,
			  mbstate_t *__restrict <[ps]>);

	#include <wchar.h>
	size_t _wcsnrtombs_r(struct _reent *<[ptr]>, char *<[dst]>,
			     const wchar_t **<[src]>, size_t <[nwc]>,
			     size_t <[len]>, mbstate_t *<[ps]>);

DESCRIPTION
The <<wcsrtombs>> function converts a string of wide characters indirectly
pointed to by <[src]> to a corresponding multibyte character string stored in
the array pointed to by <[dst]>.  No more than <[len]> bytes are written to
<[dst]>.

If <[dst]> is NULL, no characters are stored.

If <[dst]> is not NULL, the pointer pointed to by <[src]> is updated to point
to the character after the one that conversion stopped at.  If conversion
stops because a null character is encountered, *<[src]> is set to NULL.

The mbstate_t argument, <[ps]>, is used to keep track of the shift state.  If
it is NULL, <<wcsrtombs>> uses an internal, static mbstate_t object, which
is initialized to the initial conversion state at program startup.

The <<wcsnrtombs>> function behaves identically to <<wcsrtombs>>, except that
conversion stops after reading at most <[nwc]> characters from the buffer
pointed to by <[src]>.

RETURNS
The <<wcsrtombs>> and <<wcsnrtombs>> functions return the number of bytes
stored in the array pointed to by <[dst]> (not including any terminating
null), if successful, otherwise it returns (size_t)-1.

PORTABILITY
<<wcsrtombs>> is defined by C99 standard.
<<wcsnrtombs>> is defined by the POSIX.1-2008 standard.
*/

#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"
#include "../locale/setlocale.h"

size_t
_wcsnrtombs_l (struct _reent *r, char *dst, const wchar_t **src, size_t nwc,
	       size_t len, mbstate_t *ps, struct __locale_t *loc)
{
  char *ptr = dst;
  char buff[10];
  wchar_t *pwcs;
  size_t n;
  int i;

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(r);
      ps = &(_REENT_WCSRTOMBS_STATE(r));
    }
#endif

  /* If no dst pointer, treat len as maximum possible value. */
  if (dst == NULL)
    len = (size_t)-1;

  n = 0;
  pwcs = (wchar_t *)(*src);

  while (n < len && nwc-- > 0)
    {
      int count = ps->__count;
      wint_t wch = ps->__value.__wch;
      int bytes = loc->wctomb (r, buff, *pwcs, ps);
      if (bytes == -1)
	{
	  r->_errno = EILSEQ;
	  ps->__count = 0;
	  return (size_t)-1;
	}
      if (n + bytes <= len)
	{
          n += bytes;
	  if (dst)
	    {
	      for (i = 0; i < bytes; ++i)
	        *ptr++ = buff[i];
	      ++(*src);
	    }
	  if (*pwcs++ == 0x00)
	    {
	      if (dst)
	        *src = NULL;
	      ps->__count = 0;
	      return n - 1;
	    }
	}
      else
	{
	  /* not enough room, we must back up state to before __WCTOMB call */
	  ps->__count = count;
	  ps->__value.__wch = wch;
          len = 0;
	}
    }

  return n;
} 

size_t
_wcsnrtombs_r (struct _reent *r,
	char *dst,
	const wchar_t **src,
	size_t nwc,
	size_t len,
	mbstate_t *ps)
{
  return _wcsnrtombs_l (_REENT, dst, src, nwc, len, ps,
			__get_current_locale ());
}

#ifndef _REENT_ONLY
size_t
wcsnrtombs (char *__restrict dst,
	const wchar_t **__restrict src,
	size_t nwc,
	size_t len,
	mbstate_t *__restrict ps)
{
  return _wcsnrtombs_l (_REENT, dst, src, nwc, len, ps,
			__get_current_locale ());
}
#endif /* !_REENT_ONLY */
