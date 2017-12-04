#include <reent.h>
#include <stdlib.h>
#include <string.h>

char *
_strndup_r (struct _reent *reent_ptr,
        const char   *str,
        size_t n)
{
  const char *ptr = str;
  size_t len;
  char *copy;

  while (n-- > 0 && *ptr)
    ptr++;

  len = ptr - str;

  copy = _malloc_r (reent_ptr, len + 1);
  if (copy)
    {
      memcpy (copy, str, len);
      copy[len] = '\0';
    }
  return copy;
}
