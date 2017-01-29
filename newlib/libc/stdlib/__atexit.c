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
 *  are non-weak reference in current newlib/libgloss.
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
#include <reent.h>
#include <sys/lock.h>
#include "atexit.h"

/* Make this a weak reference to avoid pulling in malloc.  */
void * malloc(size_t) _ATTRIBUTE((__weak__));

#ifdef _LITE_EXIT
/* As __call_exitprocs is weak reference in lite exit, make a
   non-weak reference to it here.  */
const void * __atexit_dummy = &__call_exitprocs;
#endif

#ifndef __SINGLE_THREAD__
extern _LOCK_RECURSIVE_T __atexit_recursive_mutex;
#endif

#ifdef _REENT_GLOBAL_ATEXIT
static struct _atexit _global_atexit0 = _ATEXIT_INIT;
# define _GLOBAL_ATEXIT0 (&_global_atexit0)
#else
# define _GLOBAL_ATEXIT0 (&_GLOBAL_REENT->_atexit0)
#endif

/*
 * Register a function to be performed at exit or on shared library unload.
 */

int
_DEFUN (__register_exitproc,
	(type, fn, arg, d),
	int type _AND
	void (*fn) (void) _AND
	void *arg _AND
	void *d)
{
  struct _on_exit_args * args;
  register struct _atexit *p;

#ifndef __SINGLE_THREAD__
  __lock_acquire_recursive(__atexit_recursive_mutex);
#endif

  p = _GLOBAL_ATEXIT;
  if (p == NULL)
    {
      _GLOBAL_ATEXIT = p = _GLOBAL_ATEXIT0;
#ifdef _REENT_SMALL
      extern struct _on_exit_args * const __on_exit_args _ATTRIBUTE ((weak));
      if (&__on_exit_args != NULL)
	p->_on_exit_args_ptr = __on_exit_args;
#endif	/* def _REENT_SMALL */
    }
  if (p->_ind >= _ATEXIT_SIZE)
    {
#ifndef _ATEXIT_DYNAMIC_ALLOC
#ifndef __SINGLE_THREAD__
      __lock_release_recursive(__atexit_recursive_mutex);
#endif
      return -1;
#else
      /* Don't dynamically allocate the atexit array if malloc is not
	 available.  */
      if (!malloc)
	{
#ifndef __SINGLE_THREAD__
	  __lock_release_recursive(__atexit_recursive_mutex);
#endif
	  return -1;
	}

      p = (struct _atexit *) malloc (sizeof *p);
      if (p == NULL)
	{
#ifndef __SINGLE_THREAD__
	  __lock_release_recursive(__atexit_recursive_mutex);
#endif
	  return -1;
	}
      p->_ind = 0;
      p->_next = _GLOBAL_ATEXIT;
      _GLOBAL_ATEXIT = p;
#ifndef _REENT_SMALL
      p->_on_exit_args._fntypes = 0;
      p->_on_exit_args._is_cxa = 0;
#else
      p->_on_exit_args_ptr = NULL;
#endif
#endif
    }

  if (type != __et_atexit)
    {
#ifdef _REENT_SMALL
      args = p->_on_exit_args_ptr;
      if (args == NULL)
	{
#ifndef _ATEXIT_DYNAMIC_ALLOC
#ifndef __SINGLE_THREAD__
	  __lock_release_recursive(__atexit_recursive_mutex);
#endif
	  return -1;
#else
	  if (malloc)
	    args = malloc (sizeof * p->_on_exit_args_ptr);

	  if (args == NULL)
	    {
#ifndef __SINGLE_THREAD__
	      __lock_release(__atexit_recursive_mutex);
#endif
	      return -1;
	    }
	  args->_fntypes = 0;
	  args->_is_cxa = 0;
	  p->_on_exit_args_ptr = args;
#endif
	}
#else
      args = &p->_on_exit_args;
#endif
      args->_fnargs[p->_ind] = arg;
      args->_fntypes |= (1 << p->_ind);
      args->_dso_handle[p->_ind] = d;
      if (type == __et_cxa)
	args->_is_cxa |= (1 << p->_ind);
    }
  p->_fns[p->_ind++] = fn;
#ifndef __SINGLE_THREAD__
  __lock_release_recursive(__atexit_recursive_mutex);
#endif
  return 0;
}
