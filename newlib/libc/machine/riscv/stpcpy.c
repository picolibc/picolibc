#include <stdbool.h>
#include "rv_string.h"

char *stpcpy(char *dst, const char *src)
{
  return __libc_strcpy(dst, src, false);
}
