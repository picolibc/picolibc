/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

/*
FUNCTION
<<exit>>---end program execution

INDEX
	exit

ANSI_SYNOPSIS
	#include <stdlib.h>
	void exit(int <[code]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	void exit(<[code]>)
	int <[code]>;

DESCRIPTION
Use <<exit>> to return control from a program to the host operating
environment.  Use the argument <[code]> to pass an exit status to the
operating environment: two particular values, <<EXIT_SUCCESS>> and
<<EXIT_FAILURE>>, are defined in `<<stdlib.h>>' to indicate success or
failure in a portable fashion.

<<exit>> does two kinds of cleanup before ending execution of your
program.  First, it calls all application-defined cleanup functions
you have enrolled with <<atexit>>.  Second, files and streams are
cleaned up: any pending output is delivered to the host system, each
open file or stream is closed, and files created by <<tmpfile>> are
deleted.

RETURNS
<<exit>> does not return to its caller.

PORTABILITY
ANSI C requires <<exit>>, and specifies that <<EXIT_SUCCESS>> and
<<EXIT_FAILURE>> must be defined.

Supporting OS subroutines required: <<_exit>>.
*/

#include <stdlib.h>
#include <unistd.h>	/* for _exit() declaration */
#include <reent.h>

#ifndef _REENT_ONLY

/*
 * Exit, flushing stdio buffers if necessary.
 */

void 
_DEFUN (exit, (code),
	int code)
{
  register struct _atexit *p;
  register struct _on_exit_args * args;
  register int n;
  int i;

#ifdef _REENT_SMALL
  p = &_GLOBAL_REENT->_atexit;
  args = p->_on_exit_args_ptr;
  
  if (args == NULL)
    {
      for (n = p->_ind; n--;)
        p->_fns[n] ();
    }
  else
    {
      for (n = p->_ind - 1, i = (n >= 0) ? (1 << n) : 0; n >= 0; --n, i >>= 1)
        if (args->_fntypes & i)
          (*((void (*)(int, void *)) p->_fns[n]))(code, args->_fnargs[n]);
        else
          p->_fns[n] ();
    }
#else
  p = _GLOBAL_REENT->_atexit;
  while (p)
    {
      args = & p->_on_exit_args;
  
      for (n = p->_ind - 1, i = (n >= 0) ? (1 << n) : 0; n >= 0; --n, i >>= 1)
        if (args->_fntypes & i)
          (*((void (*)(int, void *)) p->_fns[n]))(code, args->_fnargs[n]);
        else
          p->_fns[n] ();

      p = p->_next;
    }
#endif

  if (_GLOBAL_REENT->__cleanup)
    (*_GLOBAL_REENT->__cleanup) (_GLOBAL_REENT);
  _exit (code);
}

#endif
