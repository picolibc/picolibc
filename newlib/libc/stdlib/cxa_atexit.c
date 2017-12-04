/*
 * Implementation of __cxa_atexit.
 */

#include <stddef.h>
#include <stdlib.h>
#include <reent.h>
#include <sys/lock.h>
#include "atexit.h"

#ifdef _REENT_SMALL

#include "on_exit_args.h"

/* force linking of static instance of _on_exit_args */
const void * const __cxa_atexit_dummy = &__on_exit_args;

#endif	/* def _REENT_SMALL */

/*
 * Register a function to be performed at exit or DSO unload.
 */

int
__cxa_atexit (void (*fn) (void *),
	void *arg,
	void *d)
{
#ifdef _LITE_EXIT
  /* Refer to comments in __atexit.c for more details of lite exit.  */
  int __register_exitproc (int, void (*fn) (void), void *, void *)
    __attribute__ ((weak));

  if (!__register_exitproc)
    return 0;
  else
#endif
    return __register_exitproc (__et_cxa, (void (*)(void)) fn, arg, d);
}
