#include <string.h>
#include "aeabi.h"

/* Copy memory like memmove, but no return value required.  */
void
__aeabi_memmove (void *dest, const void *src, size_t n)
{
  memmove (dest, src, n);
}

/* Versions of the above which may assume memory alignment.  */
strong_alias (__aeabi_memmove, __aeabi_memmove4)
strong_alias (__aeabi_memmove, __aeabi_memmove8)
