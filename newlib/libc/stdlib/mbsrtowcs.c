#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <errno.h>

size_t
mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps)
{
  int retval = 0;
  mbstate_t internal;

  _REENT_CHECK_MISC(_REENT);

  retval = _mbstowcs_r (_REENT, dst, *src, len, ps != NULL ? ps : &internal);

  if (retval == -1)
    {
      _REENT->_errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}
