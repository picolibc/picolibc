#include <reent.h>
#include <stdlib.h>
#include <string.h>

char *
_strdup_r (struct _reent *reent_ptr,
        const char   *str)
{
  size_t len = strlen (str) + 1;
  char *copy = _malloc_r (reent_ptr, len);
  if (copy)
    {
      memcpy (copy, str, len);
    }
  return copy;
}
