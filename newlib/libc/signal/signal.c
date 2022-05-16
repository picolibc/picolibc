/*
FUNCTION
<<signal>>---specify handler subroutine for a signal

INDEX
	signal
INDEX
	_signal_r

SYNOPSIS
	#include <signal.h>
	void (*signal(int <[sig]>, void(*<[func]>)(int))) (int);

	void (*_signal_r(void *<[reent]>, int <[sig]>, void(*<[func]>)(int))) (int);

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

The alternate function <<_signal_r>> is the reentrant version.
The extra argument <[reent]> is a pointer to a reentrancy structure.

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

/* _init_signal initialises the signal handlers for each signal. This function
   is called by crt0 at program startup.  */

#ifdef SIGNAL_PROVIDED

int _dummy_simulated_signal;

#else

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <reent.h>
#include <_syslist.h>

#ifdef _REENT_THREAD_LOCAL
_Thread_local void (**_tls_sig_func)(int);
#endif

int
_init_signal_r (struct _reent *ptr)
{
  int i;

  if (_REENT_SIG_FUNC(ptr) == NULL)
    {
      _REENT_SIG_FUNC(ptr) = (_sig_func_ptr *)_malloc_r (ptr, sizeof (_sig_func_ptr) * NSIG);
      if (_REENT_SIG_FUNC(ptr) == NULL)
	return -1;

      for (i = 0; i < NSIG; i++)
	_REENT_SIG_FUNC(ptr)[i] = SIG_DFL;
    }

  return 0;
}

_sig_func_ptr
_signal_r (struct _reent *ptr,
	int sig,
	_sig_func_ptr func)
{
  _sig_func_ptr old_func;

  if (sig < 0 || sig >= NSIG)
    {
      _REENT_ERRNO(ptr) = EINVAL;
      return SIG_ERR;
    }

  if (_REENT_SIG_FUNC(ptr) == NULL && _init_signal_r (ptr) != 0)
    return SIG_ERR;
  
  old_func = _REENT_SIG_FUNC(ptr)[sig];
  _REENT_SIG_FUNC(ptr)[sig] = func;

  return old_func;
}

int 
_raise_r (struct _reent *ptr,
     int sig)
{
  _sig_func_ptr func;

  if (sig < 0 || sig >= NSIG)
    {
      _REENT_ERRNO(ptr) = EINVAL;
      return -1;
    }

  if (_REENT_SIG_FUNC(ptr) == NULL)
    func = SIG_DFL;
  else
    func = _REENT_SIG_FUNC(ptr)[sig];

  if (func == SIG_DFL)
    return _kill_r (ptr, _getpid_r (ptr), sig);
  else if (func == SIG_IGN)
    return 0;
  else if (func == SIG_ERR)
    {
      _REENT_ERRNO(ptr) = EINVAL;
      return 1;
    }
  else
    {
      _REENT_SIG_FUNC(ptr)[sig] = SIG_DFL;
      func (sig);
      return 0;
    }
}

int
__sigtramp_r (struct _reent *ptr,
     int sig)
{
  _sig_func_ptr func;

  if (sig < 0 || sig >= NSIG)
    {
      return -1;
    }

  if (_REENT_SIG_FUNC(ptr) == NULL && _init_signal_r (ptr) != 0)
    return -1;

  func = _REENT_SIG_FUNC(ptr)[sig];
  if (func == SIG_DFL)
    return 1;
  else if (func == SIG_ERR)
    return 2;
  else if (func == SIG_IGN)
    return 3;
  else
    {
      _REENT_SIG_FUNC(ptr)[sig] = SIG_DFL;
      func (sig);
      return 0;
    }
}

#ifndef _REENT_ONLY

int 
raise (int sig)
{
  return _raise_r (_REENT, sig);
}

_sig_func_ptr
signal (int sig,
	_sig_func_ptr func)
{
  return _signal_r (_REENT, sig, func);
}

int 
_init_signal (void)
{
  return _init_signal_r (_REENT);
}

int
__sigtramp (int sig)
{
  return __sigtramp_r (_REENT, sig);
}

#endif

#endif /* !SIGNAL_PROVIDED */
