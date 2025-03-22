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

SYNOPSIS
	#include <stdlib.h>
	void exit(int <[code]>);

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

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <unistd.h>	/* for _exit() declaration */
#ifndef __TINY_STDIO
#include "../stdio/local.h"
#include <stdio.h>
#endif
#include "atexit.h"

/*
 * Exit, flushing stdio buffers if necessary.
 */

void
exit (int code)
{
  /* Refer to comments in __atexit.c and pico-onexit.c for why this is weak */
  void __call_exitprocs (int, void *) __weak;
  if (__call_exitprocs)
    __call_exitprocs (code, NULL);

#ifndef __TINY_STDIO
  if (_stdio_cleanup)
    (*_stdio_cleanup) ();
#endif
  _exit (code);
}
