/*
Copyright (c) 1994 Cygnus Support.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
endorse or promote products derived from this software without
specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
/*
FUNCTION
<<signal>>---specify handler subroutine for a signal

INDEX
	signal

SYNOPSIS
	#include <signal.h>
	void (*signal(int <[sig]>, void(*<[func]>)(int))) (int);

DESCRIPTION
<<signal>> provides a simple signal-handling implementation for embedded
targets.

<<signal>> allows you to request changed treatment for a particular
signal <[sig]>.  You can use one of the predefined macros <<SIG_DFL>>
(select system default handling) or <<SIG_IGN>> (ignore this signal)
as the value of <[func]>; otherwise, <[func]> is a function pointer
that identifies a subroutine in your program as the handler for this signal.

Some of the execution environment for signal handlers is
unpredictable; notably, the only library function required to work
correctly from within a signal handler is <<signal>> itself, and
only when used to redefine the handler for the current signal value.

Static storage is likewise unreliable for signal handlers, with one
exception: if you declare a static storage location as `<<volatile
sig_atomic_t>>', then you may use that location in a signal handler to
store signal values.

If your signal handler terminates using <<return>> (or implicit
return), your program's execution continues at the point
where it was when the signal was raised (whether by your program
itself, or by an external event).  Signal handlers can also
use functions such as <<exit>> and <<abort>> to avoid returning.

@c FIXME: do we have setjmp.h and assoc fns?

RETURNS
If your request for a signal handler cannot be honored, the result is
<<SIG_ERR>>; a specific error number is also recorded in <<errno>>.

Otherwise, the result is the previous handler (a function pointer or
one of the predefined macros).

PORTABILITY
ANSI C requires <<signal>>.

No supporting OS subroutines are required to link with <<signal>>, but
it will not have any useful effects, except for software generated signals,
without an operating system that can actually raise exceptions.
*/

/*
 * signal.c
 * Original Author:	G. Haley
 *
 * signal associates the function pointed to by func with the signal sig. When
 * a signal occurs, the value of func determines the action taken as follows:
 * if func is SIG_DFL, the default handling for that signal will occur; if func
 * is SIG_IGN, the signal will be ignored; otherwise, the default handling for
 * the signal is restored (SIG_DFL), and the function func is called with sig
 * as its argument. Returns the value of func for the previous call to signal
 * for the signal sig, or SIG_ERR if the request fails.
 */

#ifdef SIGNAL_PROVIDED

int _dummy_simulated_signal;

#else

#define _DEFAULT_SOURCE
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <_syslist.h>
#include <unistd.h>

/* signal info */
static NEWLIB_THREAD_LOCAL void (*(_sig_func[NSIG]))(int);

_sig_func_ptr
signal (int sig, _sig_func_ptr func)
{
  _sig_func_ptr old_func;

  if (sig < 0 || sig >= NSIG)
    {
      errno = EINVAL;
      return SIG_ERR;
    }

  old_func = _sig_func[sig];
  _sig_func[sig] = func;

  return old_func;
}

int
raise (int sig)
{
  _sig_func_ptr func;

  if (sig < 0 || sig >= NSIG)
    {
      errno = EINVAL;
      return -1;
    }

  func = _sig_func[sig];

  if (func == SIG_DFL)
    return kill (getpid (), sig);
  else if (func == SIG_IGN)
    return 0;
  else if (func == SIG_ERR)
    {
      errno = EINVAL;
      return 1;
    }
  else
    {
      _sig_func[sig] = SIG_DFL;
      func (sig);
      return 0;
    }
}

int
__sigtramp (int sig)
{
  _sig_func_ptr func;

  if (sig < 0 || sig >= NSIG)
    {
      return -1;
    }

  func = _sig_func[sig];
  if (func == SIG_DFL)
    return 1;
  else if (func == SIG_ERR)
    return 2;
  else if (func == SIG_IGN)
    return 3;
  else
    {
      _sig_func[sig] = SIG_DFL;
      func (sig);
      return 0;
    }
}

#endif /* !SIGNAL_PROVIDED */
