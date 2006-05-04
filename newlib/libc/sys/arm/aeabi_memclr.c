#include <string.h>
#include "aeabi.h"

/* Clear memory.  */
void
__aeabi_memclr (void *dest, size_t n)
{
  memset (dest, 0, n);
}

/* Versions of the above which may assume memory alignment.  */
strong_alias (__aeabi_memclr, __aeabi_memclr4)
strong_alias (__aeabi_memclr, __aeabi_memclr8)
