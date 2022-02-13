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
 * COmmon routine to call call registered atexit-like routines.
 */


#include <stdlib.h>
#include <sys/lock.h>
#include "atexit.h"

/* Make this a weak reference to avoid pulling in free.  */
#ifndef MALLOC_PROVIDED
void free(void *) _ATTRIBUTE((__weak__));
#endif

#ifdef _WANT_REGISTER_FINI

/* If "__libc_fini" is defined, finalizers (either
   "__libc_fini_array", or "_fini", as appropriate) will be run after
   all user-specified atexit handlers.  For example, you can define
   "__libc_fini" to "_fini" in your linker script if you want the C
   library, rather than startup code, to register finalizers.  If you
   do that, then your startup code need not contain references to
   "atexit" or "exit".  As a result, only applications that reference
   "exit" explicitly will pull in finalization code.

   The choice of whether to register finalizers from libc or from
   startup code is deferred to link-time, rather than being a
   configure-time option, so that the same C library binary can be
   used with multiple BSPs, some of which register finalizers from
   startup code, while others defer to the C library.  */
extern char __libc_fini __attribute__((weak));

/* Register the application finalization function with atexit.  These
   finalizers should run last.  Therefore, we want to call atexit as
   soon as possible.  */
static void 
register_fini(void) __attribute__((constructor (0)));

static void 
register_fini(void)
{
  if (&__libc_fini) {
#ifdef _HAVE_INITFINI_ARRAY
    extern void __libc_fini_array (void);
    atexit (__libc_fini_array);
#else
    extern void _fini (void);
    atexit (_fini);
#endif
  }
}

#endif /* _WANT_REGISTER_FINI  */

/*
 * Call registered exit handlers.  If D is null then all handlers are called,
 * otherwise only the handlers from that DSO are called.
 */

void 
__call_exitprocs (int code, void *d)
{
  register struct _atexit *p;
  struct _atexit **lastp;
  register struct _on_exit_args * args;
  register int n;
  int i;
  void (*fn) (void);


  __LIBC_LOCK();

 restart:

  p = _atexit;
  lastp = &_atexit;
  while (p)
    {
      args = &p->_on_exit_args;
      for (n = p->_ind - 1; n >= 0; n--)
	{
	  int ind;

	  i = 1 << n;

	  /* Skip functions not from this dso.  */
	  if (d && (!args || args->_dso_handle[n] != d))
	    continue;

	  /* Remove the function now to protect against the
	     function calling exit recursively.  */
	  fn = p->_fns[n];
	  if (n == p->_ind - 1)
	    p->_ind--;
	  else
	    p->_fns[n] = NULL;

	  /* Skip functions that have already been called.  */
	  if (!fn)
	    continue;

	  ind = p->_ind;

	  /* Call the function.  */
	  if (!args || (args->_fntypes & i) == 0)
	    fn ();
	  else if ((args->_is_cxa & i) == 0)
	    (*((void (*)(int, void *)) fn))(code, args->_fnargs[n]);
	  else
	    (*((void (*)(void *)) fn))(args->_fnargs[n]);

	  /* The function we called call atexit and registered another
	     function (or functions).  Call these new functions before
	     continuing with the already registered functions.  */
	  if (ind != p->_ind || *lastp != p)
	    goto restart;
	}

#if !defined (_ATEXIT_DYNAMIC_ALLOC) || !defined (MALLOC_PROVIDED)
      break;
#else
      /* Move to the next block.  Free empty blocks except the last one,
	 which is part of _GLOBAL_REENT.  */
      if (p->_ind == 0 && p->_next)
	{
	  /* Remove empty block from the list.  */
	  *lastp = p->_next;
	  free (p);
	  p = *lastp;
	}
      else
	{
	  lastp = &p->_next;
	  p = p->_next;
	}
#endif
    }
    __LIBC_UNLOCK();
}
