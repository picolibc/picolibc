#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <errno.h>

size_t
wcsrtombs (char *dst, const wchar_t **src, size_t len, mbstate_t *ps)
{
  int retval = 0;
  _REENT_CHECK_MISC(_REENT);

  retval = _wcstombs_r (_REENT, dst, *src, len, ps);

  if (retval == -1)
    {
      _REENT->_errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}
