#include <string.h>
#include <stdbool.h>

char *stpcpy(char *dst, const char *src)
{
  return __libc_strcpy(dst, src, false);
}
