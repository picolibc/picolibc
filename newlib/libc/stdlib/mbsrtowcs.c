/* doc in mbsnrtowcs.c */

#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

size_t
_mbsrtowcs_r (struct _reent *r,
	wchar_t *dst,
	const char **src,
	size_t len,
	mbstate_t *ps)
{
  return _mbsnrtowcs_r (r, dst, src, (size_t) -1, len, ps);
}

#ifndef _REENT_ONLY
size_t
mbsrtowcs (wchar_t *__restrict dst,
	const char **__restrict src,
	size_t len,
	mbstate_t *__restrict ps)
{
  return _mbsnrtowcs_r (_REENT, dst, src, (size_t) -1, len, ps);
}
#endif /* !_REENT_ONLY */
