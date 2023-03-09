/*
Copyright (c) 2001, 2009 Xilinx, Inc.  All rights reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1.  Redistributions source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 

2.  Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 

3.  Neither the name of Xilinx nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* NetWare can not use this implementation of abort.  It provides its
   own version of abort in clib.nlm.  If we can not use clib.nlm, then
   we must write abort in sys/netware.  */

#ifdef ABORT_PROVIDED

int _dummy_abort = 1;

#else

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

Before terminating your program, <<abort>> raises the exception <<SIGABRT>>
(using `<<raise(SIGABRT)>>').  If you have used <<signal>> to register
an exception handler for this condition, that handler has the
opportunity to retain control, thereby avoiding program termination.

In this implementation, <<abort>> does not perform any stream- or
file-related cleanup (the host environment may do so; if not, you can
arrange for your program to do its own cleanup with a <<SIGABRT>>
exception handler).

RETURNS
<<abort>> does not return to its caller.

PORTABILITY
ANSI C requires <<abort>>.

Supporting OS subroutines required: <<_exit>> and optionally, <<write>>.
*/

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void
abort (void)
{
#ifdef ABORT_MESSAGE
  write (2, "Abort called\n", sizeof ("Abort called\n")-1);
#endif

  while (1)
    {
      exit(1);
    }
}
#endif