/*
Copyright (c) 2004 Paul Brook <paul@codesourcery.com> 

Common routine to implement atexit-like functionality.

This is also the key function to be configured as lite exit, a size-reduced
implementation of exit that doesn't invoke clean-up functions such as _fini
or global destructors.

Default (without lite exit) call graph is like:
start -> atexit -> __register_exitproc
start -> __libc_init_array -> __cxa_atexit -> __register_exitproc
on_exit -> __register_exitproc
start -> exit -> __call_exitprocs

Here an -> means arrow tail invokes arrow head. All invocations here
are non-weak reference in current newlib.

Lite exit makes some of above calls as weak reference, so that size expansive
functions __register_exitproc and __call_exitprocs may not be linked. These
calls are:
start w-> atexit
cxa_atexit w-> __register_exitproc
exit w-> __call_exitprocs

Lite exit also makes sure that __call_exitprocs will be referenced as non-weak
whenever __register_exitproc is referenced as non-weak.

Thus with lite exit libs, a program not explicitly calling atexit or on_exit
will escape from the burden of cleaning up code. A program with atexit or on_exit
will work consistently to normal libs.

Lite exit is enabled with --enable-lite-exit, and is controlled with macro
LITE_EXIT.
 */
/*
 * Implementation of __cxa_atexit.
 */

#include <stddef.h>
#include <stdlib.h>
#include <sys/lock.h>
#include "atexit.h"

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
