#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <errno.h>

size_t
wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
{
  int retval = 0;
  _REENT_CHECK_MISC(_REENT);

  if (s == NULL)
    retval = _wctomb_r (_REENT, "", wc, ps);
  else
    retval = _wctomb_r (_REENT, s, wc, ps);

  if (retval == -1)
    {
      _REENT->_errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}
