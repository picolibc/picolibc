/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

/*
FUNCTION
<<atexit>>---request execution of functions at program exit

INDEX
	atexit

ANSI_SYNOPSIS
	#include <stdlib.h>
	int atexit (void (*<[function]>)(void));

TRAD_SYNOPSIS
	#include <stdlib.h>
	int atexit ((<[function]>)
	  void (*<[function]>)();

DESCRIPTION
You can use <<atexit>> to enroll functions in a list of functions that
will be called when your program terminates normally.  The argument is
a pointer to a user-defined function (which must not require arguments and
must not return a result).

The functions are kept in a LIFO stack; that is, the last function
enrolled by <<atexit>> will be the first to execute when your program
exits.

There is no built-in limit to the number of functions you can enroll
in this list; however, after every group of 32 functions is enrolled,
<<atexit>> will call <<malloc>> to get space for the next part of the
list.   The initial list of 32 functions is statically allocated, so
you can always count on at least that many slots available.

RETURNS
<<atexit>> returns <<0>> if it succeeds in enrolling your function,
<<-1>> if it fails (possible only if no space was available for
<<malloc>> to extend the list of functions).

PORTABILITY
<<atexit>> is required by the ANSI standard, which also specifies that
implementations must support enrolling at least 32 functions.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <stddef.h>
#include <stdlib.h>
#include <reent.h>
#include <sys/lock.h>

/*
 * Register a function to be performed at exit.
 */

int
_DEFUN (atexit,
	(fn),
	_VOID _EXFUN ((*fn), (_VOID)))
{
  register struct _atexit *p;

#ifndef __SINGLE_THREAD__
  __LOCK_INIT(static, lock);

  __lock_acquire(lock);
#endif
      
  /* _REENT_SMALL atexit() doesn't allow more than the required 32 entries.  */
#ifndef _REENT_SMALL
  if ((p = _GLOBAL_REENT->_atexit) == NULL)
    _GLOBAL_REENT->_atexit = p = &_GLOBAL_REENT->_atexit0;
  if (p->_ind >= _ATEXIT_SIZE)
    {
      if ((p = (struct _atexit *) malloc (sizeof *p)) == NULL)
        {
#ifndef __SINGLE_THREAD__
          __lock_release(lock);
#endif
          return -1;
        }
      p->_ind = 0;
      p->_on_exit_args._fntypes = 0;
      p->_next = _GLOBAL_REENT->_atexit;
      _GLOBAL_REENT->_atexit = p;
    }
#else
  p = &_GLOBAL_REENT->_atexit;
  if (p->_ind >= _ATEXIT_SIZE)
    {
#ifndef __SINGLE_THREAD__
      __lock_release(lock);
#endif
      return -1;
    }
#endif
  p->_fns[p->_ind++] = fn;
#ifndef __SINGLE_THREAD__
  __lock_release(lock);
#endif
  return 0;
}
