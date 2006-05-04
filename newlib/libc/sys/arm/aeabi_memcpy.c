#include <string.h>
#include "aeabi.h"

/* Copy memory like memcpy, but no return value required.  */
void
__aeabi_memcpy (void *dest, const void *src, size_t n)
{
  memcpy (dest, src, n);
}

/* Versions of the above which may assume memory alignment.  */
strong_alias (__aeabi_memcpy, __aeabi_memcpy4)
strong_alias (__aeabi_memcpy, __aeabi_memcpy8)
