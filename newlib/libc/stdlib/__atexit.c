/* Copyright (c) 2004 Paul Brook <paul@codesourcery.com> */
/*
 *  Common routine to implement atexit-like functionality.
 *
 *  This is also the key function to be configured as lite exit, a size-reduced
 *  implementation of exit that doesn't invoke clean-up functions such as _fini
 *  or global destructors.
 *
 *  Default (without lite exit) call graph is like:
 *  _start -> atexit -> __register_exitproc
 *  _start -> __libc_init_array -> __cxa_atexit -> __register_exitproc
 *  on_exit -> __register_exitproc
 *  _start -> exit -> __call_exitprocs
 *
 *  Here an -> means arrow tail invokes arrow head. All invocations here
 *  are non-weak reference in current newlib.
 *
 *  Lite exit makes some of above calls as weak reference, so that size expansive
 *  functions __register_exitproc and __call_exitprocs may not be linked. These
 *  calls are:
 *    _start w-> atexit
 *    __cxa_atexit w-> __register_exitproc
 *    exit w-> __call_exitprocs
 *
 *  Lite exit also makes sure that __call_exitprocs will be referenced as non-weak
 *  whenever __register_exitproc is referenced as non-weak.
 *
 *  Thus with lite exit libs, a program not explicitly calling atexit or on_exit
 *  will escape from the burden of cleaning up code. A program with atexit or on_exit
 *  will work consistently to normal libs.
 *
 *  Lite exit is enabled with --enable-lite-exit, and is controlled with macro
 *  _LITE_EXIT.
 */

#include <stddef.h>
#include <stdlib.h>
#include <sys/lock.h>
#include "atexit.h"

/* Make this a weak reference to avoid pulling in malloc.  */
#ifndef MALLOC_PROVIDED
void * malloc(size_t) _ATTRIBUTE((__weak__));
#endif

#ifdef _LITE_EXIT
/* As __call_exitprocs is weak reference in lite exit, make a
   non-weak reference to it here.  */
const void * __atexit_dummy = &__call_exitprocs;
#endif

NEWLIB_THREAD_LOCAL_ATEXIT struct _atexit _atexit0;
NEWLIB_THREAD_LOCAL_ATEXIT struct _atexit *_atexit;

/*
 * Register a function to be performed at exit or on shared library unload.
 */

int
__register_exitproc (int type,
	void (*fn) (void),
	void *arg,
	void *d)
{
  struct _on_exit_args * args;
  register struct _atexit *p;

  __LIBC_LOCK();

  p = _atexit;
  if (p == NULL)
    {
      _atexit = p = &_atexit0;
    }
  if (p->_ind >= _ATEXIT_SIZE)
    {
#if !defined (_ATEXIT_DYNAMIC_ALLOC) || !defined (MALLOC_PROVIDED)
      __LIBC_UNLOCK();
      return -1;
#else
      p = (struct _atexit *) malloc (sizeof *p);
      if (p == NULL)
	{
	  __LIBC_UNLOCK();
	  return -1;
	}
      p->_ind = 0;
      p->_next = _atexit;
      _atexit = p;
#endif
    }

  if (type != __et_atexit)
    {
      args = &p->_on_exit_args;
      args->_fnargs[p->_ind] = arg;
      args->_fntypes |= (1 << p->_ind);
      args->_dso_handle[p->_ind] = d;
      if (type == __et_cxa)
	args->_is_cxa |= (1 << p->_ind);
    }
  p->_fns[p->_ind++] = fn;
  __LIBC_UNLOCK();
  return 0;
}
