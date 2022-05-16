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

void
_reclaim_reent (struct _reent *ptr)
{
#ifndef _REENT_THREAD_LOCAL
  if (ptr != _impure_ptr)
#endif
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
      if (_REENT_EMERGENCY(ptr))
	_free_r (ptr, _REENT_EMERGENCY(ptr));
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

      if (_REENT_CVTBUF(ptr))
	_free_r (ptr, _REENT_CVTBUF(ptr));
    /* We should free _sig_func to avoid a memory leak, but how to
	   do it safely considering that a signal may be delivered immediately
	   after the free?
	  if (_REENT_SIG_FUNC(ptr))
	_free_r (ptr, _REENT_SIG_FUNC(ptr));*/

      if (_REENT_CLEANUP(ptr))
	{
	  /* cleanup won't reclaim memory 'coz usually it's run
	     before the program exits, and who wants to wait for that? */
	  _REENT_CLEANUP(ptr) (ptr);
	}

      /* Malloc memory not reclaimed; no good way to return memory anyway. */

    }
}
