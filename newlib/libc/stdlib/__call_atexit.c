/*
 * COmmon routine to call call registered atexit-like routines.
 */


#include <stdlib.h>
#include <reent.h>
#include <sys/lock.h>
#include "atexit.h"

/* Make this a weak reference to avoid pulling in free.  */
#ifndef MALLOC_PROVIDED
void free(void *) _ATTRIBUTE((__weak__));
#endif

#ifndef __SINGLE_THREAD__
__LOCK_INIT_RECURSIVE(, __atexit_recursive_mutex);
#endif

#ifdef _REENT_GLOBAL_ATEXIT
struct _atexit *_global_atexit = _NULL;
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


#ifndef __SINGLE_THREAD__
  __lock_acquire_recursive(__atexit_recursive_mutex);
#endif

 restart:

  p = _GLOBAL_ATEXIT;
  lastp = &_GLOBAL_ATEXIT;
  while (p)
    {
#ifdef _REENT_SMALL
      args = p->_on_exit_args_ptr;
#else
      args = &p->_on_exit_args;
#endif
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
#ifdef _REENT_SMALL
	  if (args)
	    free (args);
#endif
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
#ifndef __SINGLE_THREAD__
  __lock_release_recursive(__atexit_recursive_mutex);
#endif

}
