/*
FUNCTION
<<abort>>---abnormal termination of a program

INDEX
	abort

SYNOPSIS
	#include <stdlib.h>
	void abort(void);

DESCRIPTION
Use <<abort>> to signal that your program has detected a condition it
cannot deal with.  Normally, <<abort>> ends your program's execution.

In general implementation, <<abort>> raises the exception <<SIGABRT>>.
But for nds32 target, currently it is not necessary for MCU platform.
We can just call <<_exit>> to terminate program.

RETURNS
<<abort>> does not return to its caller.

PORTABILITY
ANSI C requires <<abort>>.

Supporting OS subroutines required: <<_exit>>.
*/

#include <unistd.h>

void
abort (void)
{
  while (1)
    {
      _exit (1);
    }
}
