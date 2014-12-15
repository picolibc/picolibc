/*
 * Copyright (c) 2014 Red Hat, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
FUNCTION
<<__fsetlocking>>---set or query locking mode on FILE stream

INDEX
	__fsetlocking

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdio_ext.h>
	int __fsetlocking(FILE *<[fp]>, int <[type]>);

DESCRIPTION
This function sets how the stdio functions handle locking of FILE <[fp]>.
The following values describe <[type]>:

<<FSETLOCKING_INTERNAL>> is the default state, where stdio functions
automatically lock and unlock the stream.

<<FSETLOCKING_BYCALLER>> means that automatic locking in stdio functions
is disabled. Applications which set this take all responsibility for file
locking themselves.

<<FSETLOCKING_QUERY>> returns the current locking mode without changing it.

RETURNS
<<__fsetlocking>> returns the current locking mode of <[fp]>.

PORTABILITY
This function originates from Solaris and is also provided by GNU libc.

No supporting OS subroutines are required.
*/

#ifndef __rtems__

#include <_ansi.h>
#include <stdio.h>
#include <stdio_ext.h>
#include "local.h"

int
_DEFUN(__fsetlocking, (fp, type),
       FILE * fp _AND
       int type)
{
  int result;
  CHECK_INIT(_REENT, fp);
  result = (fp->_flags2 & __SNLK) ? FSETLOCKING_BYCALLER : FSETLOCKING_INTERNAL;
  switch (type)
    {
    case FSETLOCKING_BYCALLER:
      fp->_flags2 |= __SNLK;
      break;
    case FSETLOCKING_INTERNAL:
      fp->_flags2 &= ~__SNLK;
      break;
    case FSETLOCKING_QUERY:
    default:
      break;
    }
  return result;
}

#endif /* __rtems__ */
