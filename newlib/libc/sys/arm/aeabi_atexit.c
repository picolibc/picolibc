#include <stdlib.h>

/* forward declaration */
extern int __cxa_atexit (void (*) (void *), void *, void *);

/* Register a function to be called by exit or when a shared library
   is unloaded.  This routine is like __cxa_atexit, but uses the
   calling sequence required by the ARM EABI.  */
int
__aeabi_atexit (void *arg, void (*func) (void *), void *d)
{
  return __cxa_atexit (func, arg, d);
}
