/* Copyright (c) 1998-2006 Tensilica Inc.  ALL RIGHTS RESERVED.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL TENSILICA
   INCORPORATED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

/* _atexit: This is a simplified version of the standard atexit function.
   It is only intended to be used by crt0 to register the _fini function
   for code in the ELF finalization section.  Using the standard version
   requires that all programs link in malloc, leading to a significant
   increase in code size for programs that would otherwise not need malloc.  */

#include <stddef.h>
#include <stdlib.h>
#include <reent.h>

/* Register a function to be performed at exit.  */

int
_atexit(void (*fn)(void))
{
  register struct _atexit *p;

#ifndef _REENT_SMALL
  if ((p = _REENT->_atexit) == NULL)
    _REENT->_atexit = p = &_REENT->_atexit0;
#else
  p = &_REENT->_atexit;
#endif
  if (p->_ind >= _ATEXIT_SIZE)
    return -1;
  p->_fns[p->_ind++] = fn;
  return 0;
}

