/*
 * Implementation of __cxa_atexit.
 */

#include <stddef.h>
#include <stdlib.h>
#include <reent.h>
#include <sys/lock.h>
#include "atexit.h"

/*
 * Register a function to be performed at exit or DSO unload.
 */

int
_DEFUN (__cxa_atexit,
	(fn, arg, d),
	void (*fn) (void *) _AND
	void *arg _AND
	void *d)
{
#ifdef _LITE_EXIT
  /* Refer to comments in __atexit.c for more details of lite exit.  */
  int __register_exitproc _PARAMS ((int, void (*fn) (void), _PTR, _PTR))
    __attribute__ ((weak));

  if (!__register_exitproc)
    return 0;
  else
#endif
    return __register_exitproc (__et_cxa, (void (*)(void)) fn, arg, d);
}
