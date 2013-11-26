/* Doc in wcsnrtombs.c */

#include <reent.h>
#include <newlib.h>
#include <wchar.h>

size_t
_DEFUN (_wcsrtombs_r, (r, dst, src, len, ps),
	struct _reent *r _AND
	char *dst _AND
	const wchar_t **src _AND
	size_t len _AND
	mbstate_t *ps)
{
  return _wcsnrtombs_r (r, dst, src, (size_t) -1, len, ps);
} 

#ifndef _REENT_ONLY
size_t
_DEFUN (wcsrtombs, (dst, src, len, ps),
	char *__restrict dst _AND
	const wchar_t **__restrict src _AND
	size_t len _AND
	mbstate_t *__restrict ps)
{
  return _wcsnrtombs_r (_REENT, dst, src, (size_t) -1, len, ps);
}
#endif /* !_REENT_ONLY */
