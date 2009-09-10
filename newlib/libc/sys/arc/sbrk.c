#include <sys/types.h>
#include <sys/errno.h>

extern char __start_heap;
extern char __end_heap;

/* FIXME: We need a semaphore here.  */

caddr_t 
_sbrk_r (struct _reent *r, size_t nbytes)
{
  static char* heap_ptr = NULL;
  char* prev_heap_ptr;

  if (heap_ptr == NULL) 
  {
    heap_ptr = &__start_heap;
  }

  /* Align the 'heap_ptr' so that memory will always be allocated at word
     boundaries. */
  heap_ptr = (char *) ((((unsigned long) heap_ptr) + 7) & ~7);
  prev_heap_ptr = heap_ptr;

  if ((heap_ptr + nbytes) < &__end_heap) 
  {
    heap_ptr += nbytes;
    return (caddr_t) prev_heap_ptr;
  } 

  __errno_r (r) = ENOMEM;
  return (caddr_t) -1;
}
