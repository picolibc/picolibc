#include <string.h>
#include "aeabi.h"

/* Set memory like memset, but different argument order and no return
   value required.  */
void
__aeabi_memset (void *dest, size_t n, int c)
{
  memset (dest, c, n);
}

/* Versions of the above which may assume memory alignment.  */
strong_alias (__aeabi_memset, __aeabi_memset4)
strong_alias (__aeabi_memset, __aeabi_memset8)
