#include <stdlib.h>
#include <wchar.h>
#include "local.h"

size_t
_wcstombs_r (struct _reent *r,
        char          *__restrict s,
        const wchar_t *__restrict pwcs,
        size_t         n,
        mbstate_t     *state)
{
  char *ptr = s;
  size_t max = n;
  char buff[8];
  int i, bytes, num_to_copy;

  if (s == NULL)
    {
      size_t num_bytes = 0;
      while (*pwcs != 0)
	{
	  bytes = __WCTOMB (r, buff, *pwcs++, state);
	  if (bytes == -1)
	    return -1;
	  num_bytes += bytes;
	}
      return num_bytes;
    }
  else
    {
      while (n > 0)
        {
          bytes = __WCTOMB (r, buff, *pwcs, state);
          if (bytes == -1)
            return -1;
          num_to_copy = (n > bytes ? bytes : (int)n);
          for (i = 0; i < num_to_copy; ++i)
            *ptr++ = buff[i];
          
          if (*pwcs == 0x00)
            return ptr - s - (n >= bytes);
          ++pwcs;
          n -= num_to_copy;
        }
      return max;
    }
} 
