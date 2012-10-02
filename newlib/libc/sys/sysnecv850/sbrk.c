#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/syscall.h"

caddr_t
_sbrk (int incr)
{
  extern char   heap_start[];	/* Defined by the linker script.  */
  static char * heap_end = NULL;
  char *        prev_heap_end;
  char *        sp = (char *) & sp;

  if (heap_end == NULL)
    heap_end = heap_start;

  prev_heap_end = heap_end;

  if (heap_end + incr > sp)
    {
#define MESSAGE "Heap and stack collision\n"
      _write (1, MESSAGE, sizeof MESSAGE);
      abort ();
    }

  heap_end += incr;

  return (caddr_t) prev_heap_end;
}
