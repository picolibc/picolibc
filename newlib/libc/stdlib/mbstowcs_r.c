#include <stdlib.h>

size_t
_DEFUN (_mbstowcs_r, (reent, pwcs, s, n, state),
        struct _reent *r    _AND         
        wchar_t       *pwcs _AND
        const char    *s    _AND
        size_t         n    _AND
        int           *state)
{
  wchar_t *ptr = pwcs;
  size_t max = n;
  char *t = (char *)s;
  int bytes;

  while (n > 0)
    {
      bytes = _mbtowc_r (r, ptr, t, MB_CUR_MAX, state);
      if (bytes == -1)
        return -1;
      else if (bytes == 0)
        return ptr - pwcs;
      t += bytes;
      ++ptr;
      --n;
    }

  return max;
}   
