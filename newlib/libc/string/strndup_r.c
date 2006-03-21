#include <reent.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

char *
_DEFUN (_strndup_r, (reent_ptr, str, n), 
        struct _reent *reent_ptr  _AND
        _CONST char   *str _AND
        size_t n)
{
  size_t len = MIN(strlen (str), n);
  char *copy = _malloc_r (reent_ptr, len + 1);
  if (copy)
    {
      memcpy (copy, str, len);
      copy[len] = '\0';
    }
  return copy;
}
