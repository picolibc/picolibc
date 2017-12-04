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

#ifndef REENTRANT_SYSCALLS_PROVIDED

/* We use the errno variable used by the system dependent layer.  */
#undef errno
int errno;

#endif

/* Interim cleanup code */

void
cleanup_glue (struct _reent *ptr,
     struct _glue *glue)
{
  /* Have to reclaim these in reverse order: */
  if (glue->_next)
    cleanup_glue (ptr, glue->_next);

  _free_r (ptr, glue);
}

void
_reclaim_reent (struct _reent *ptr)
{
  if (ptr != _impure_ptr)
    {
      /* used by mprec routines. */
#ifdef _REENT_SMALL
      if (ptr->_mp)	/* don't bother allocating it! */
      {
#endif
      if (_REENT_MP_FREELIST(ptr))
	{
	  int i;
	  for (i = 0; i < _Kmax; i++) 
	    {
	      struct _Bigint *thisone, *nextone;
	
	      nextone = _REENT_MP_FREELIST(ptr)[i];
	      while (nextone)
		{
		  thisone = nextone;
		  nextone = nextone->_next;
		  _free_r (ptr, thisone);
		}
	    }    

	  _free_r (ptr, _REENT_MP_FREELIST(ptr));
	}
      if (_REENT_MP_RESULT(ptr))
	_free_r (ptr, _REENT_MP_RESULT(ptr));
#ifdef _REENT_SMALL
      }
#endif

#ifdef _REENT_SMALL
      if (ptr->_emergency)
	_free_r (ptr, ptr->_emergency);
      if (ptr->_mp)
	_free_r (ptr, ptr->_mp);
      if (ptr->_r48)
	_free_r (ptr, ptr->_r48);
      if (ptr->_localtime_buf)
	_free_r (ptr, ptr->_localtime_buf);
      if (ptr->_asctime_buf)
	_free_r (ptr, ptr->_asctime_buf);
	  if (ptr->_signal_buf)
	_free_r (ptr, ptr->_signal_buf);
	  if (ptr->_misc)
	_free_r (ptr, ptr->_misc);
#endif

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

      if (ptr->_cvtbuf)
	_free_r (ptr, ptr->_cvtbuf);
    /* We should free _sig_func to avoid a memory leak, but how to
	   do it safely considering that a signal may be delivered immediately
	   after the free?
	  if (ptr->_sig_func)
	_free_r (ptr, ptr->_sig_func);*/

      if (ptr->__sdidinit)
	{
	  /* cleanup won't reclaim memory 'coz usually it's run
	     before the program exits, and who wants to wait for that? */
	  ptr->__cleanup (ptr);

	  if (ptr->__sglue._next)
	    cleanup_glue (ptr, ptr->__sglue._next);
	}

      /* Malloc memory not reclaimed; no good way to return memory anyway. */

    }
}
