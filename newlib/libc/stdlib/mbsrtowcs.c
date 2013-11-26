/* doc in mbsnrtowcs.c */

#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

size_t
_DEFUN (_mbsrtowcs_r, (r, dst, src, len, ps), 
	struct _reent *r _AND
	wchar_t *dst _AND
	const char **src _AND
	size_t len _AND
	mbstate_t *ps)
{
  return _mbsnrtowcs_r (r, dst, src, (size_t) -1, len, ps);
}

#ifndef _REENT_ONLY
size_t
_DEFUN (mbsrtowcs, (dst, src, len, ps),
	wchar_t *__restrict dst _AND
	const char **__restrict src _AND
	size_t len _AND
	mbstate_t *__restrict ps)
{
  return _mbsnrtowcs_r (_REENT, dst, src, (size_t) -1, len, ps);
}
#endif /* !_REENT_ONLY */
