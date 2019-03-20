/* Version of sbrk for no operating system.  */

#include "config.h"
#include <_syslist.h>
#include <_ansi.h>
#include <errno.h>

void *
_sbrk (incr)
     int incr;
{
   extern char   end; /* Set by linker.  */
   static char * heap_end;
   char *        prev_heap_end;

   if (heap_end == 0)
     heap_end = & end;

   prev_heap_end = heap_end;
   heap_end += incr;

   return (void *) prev_heap_end;
}

#ifdef REENTRANT_SYSCALLS_STUBS

void *
_sbrk_r (struct _reent *ptr,
     int incr)
{
  char *ret;

  errno = 0;
  if ((ret = (char *)(_sbrk (incr))) == (void *) -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

#endif /* REENTRANT_SYSCALLS_STUBS */
