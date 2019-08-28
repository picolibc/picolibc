/*
FUNCTION
	<<reent>>---definition of impure data.
	
INDEX
	reent

DESCRIPTION
	This module defines the impure data area used by the
	non-reentrant functions, such as strtok.
*/

#include <stdlib.h>
#include <reent.h>

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

/* Interim cleanup code */

void
cleanup_glue (struct _reent *ptr,
     struct _glue *glue)
{
  /* Have to reclaim these in reverse order: */
  if (glue->_next)
    cleanup_glue (ptr, glue->_next);

  free (glue);
}


#ifdef __weak_alias
__attribute__((weak)) void mprec_free(void);
#else
void mprec_free(void);
#endif

void
_reclaim_reent (struct _reent *ptr)
{
  if (ptr != _impure_ptr)
    {
      /* used by mprec routines. */
      mprec_free();

#ifndef _REENT_GLOBAL_ATEXIT
      /* atexit stuff */
# ifdef _REENT_SMALL
      if (ptr->_atexit && ptr->_atexit->_on_exit_args_ptr)
	_free_r (ptr, ptr->_atexit->_on_exit_args_ptr);
# else
      if ((ptr->_atexit) && (ptr->_atexit != &ptr->_atexit0))
	{
	  struct _atexit *p, *q;
	  for (p = ptr->_atexit; p != &ptr->_atexit0;)
	    {
	      q = p;
	      p = p->_next;
	      _free_r (ptr, q);
	    }
	}
# endif
#endif

    /* We should free _sig_func to avoid a memory leak, but how to
	   do it safely considering that a signal may be delivered immediately
	   after the free?
	  if (ptr->_sig_func)
	_free_r (ptr, ptr->_sig_func);*/

#ifndef TINY_STDIO
      if (ptr->__sdidinit)
	{
	  /* cleanup won't reclaim memory 'coz usually it's run
	     before the program exits, and who wants to wait for that? */
	  ptr->__cleanup (ptr);

	  if (ptr->__sglue._next)
	    cleanup_glue (ptr, ptr->__sglue._next);
	}
#endif

      /* Malloc memory not reclaimed; no good way to return memory anyway. */

    }
}
