#include <stdlib.h>
#include <wchar.h>
#include "local.h"

size_t
_DEFUN (_mbstowcs_r, (reent, pwcs, s, n, state),
        struct _reent *r    _AND         
        wchar_t       *pwcs _AND
        const char    *s    _AND
        size_t         n    _AND
        mbstate_t     *state)
{
  size_t ret = 0;
  char *t = (char *)s;
  int bytes;

  if (!pwcs)
    n = (size_t) 1; /* Value doesn't matter as long as it's not 0. */
  while (n > 0)
    {
      bytes = __mbtowc (r, pwcs, t, MB_CUR_MAX, __locale_charset (), state);
      if (bytes < 0)
	{
	  state->__count = 0;
	  return -1;
	}
      else if (bytes == 0)
	break;
      t += bytes;
      ++ret;
      if (pwcs)
	{
	  ++pwcs;
	  --n;
	}
    }
  return ret;
}   
