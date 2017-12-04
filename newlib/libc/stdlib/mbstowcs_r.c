#include <stdlib.h>
#include <wchar.h>
#include "local.h"

size_t
_mbstowcs_r (struct _reent *r,
        wchar_t       *__restrict pwcs,
        const char    *__restrict s,
        size_t         n,
        mbstate_t     *state)
{
  size_t ret = 0;
  char *t = (char *)s;
  int bytes;

  if (!pwcs)
    n = (size_t) 1; /* Value doesn't matter as long as it's not 0. */
  while (n > 0)
    {
      bytes = __MBTOWC (r, pwcs, t, MB_CUR_MAX, state);
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
