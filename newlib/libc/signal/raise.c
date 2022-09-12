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
/* Embedded systems may want the simulated signals if no other form exists,
   but UNIX versions will want to use the host facilities.
   Define SIMULATED_SIGNALS when you want to use the simulated versions.
*/

/*
FUNCTION
<<raise>>---send a signal

INDEX
	raise
INDEX
	_raise_r

SYNOPSIS
	#include <signal.h>
	int raise(int <[sig]>);

	int _raise_r(void *<[reent]>, int <[sig]>);

DESCRIPTION
Send the signal <[sig]> (one of the macros from `<<sys/signal.h>>').
This interrupts your program's normal flow of execution, and allows a signal
handler (if you've defined one, using <<signal>>) to take control.

The alternate function <<_raise_r>> is a reentrant version.  The extra
argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
The result is <<0>> if <[sig]> was successfully raised, <<1>>
otherwise.  However, the return value (since it depends on the normal
flow of execution) may not be visible, unless the signal handler for
<[sig]> terminates with a <<return>> or unless <<SIG_IGN>> is in
effect for this signal.

PORTABILITY
ANSI C requires <<raise>>, but allows the full set of signal numbers
to vary from one implementation to another.

Required OS subroutines: <<getpid>>, <<kill>>.
*/

#ifndef SIGNAL_PROVIDED

int _dummy_raise;

#else

#include <signal.h>

int
raise (int sig)
{
  return kill (getpid (), sig);
}

#endif /* SIGNAL_PROVIDED */
