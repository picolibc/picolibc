#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <errno.h>
#include <string.h>

size_t
mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
  int retval = 0;
  _REENT_CHECK_MISC(_REENT);

  if (s == NULL)
    retval = _mbtowc_r (_REENT, pwc, "", 1, ps);
  else
    retval = _mbtowc_r (_REENT, pwc, s, n, ps);

  if (*pwc == NULL)
    memset (ps, '\0', sizeof (mbstate_t));

  if (retval == -1)
    {
      _REENT->_errno = EILSEQ;
      return (size_t)(-1);
    }
  else
    return (size_t)retval;
}
