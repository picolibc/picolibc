/* Copyright (c) 2002, 2005, Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/* $Id: fgetc.c 1944 2009-04-01 23:12:20Z arcanum $ */

#include <stdio.h>
#include "stdio_private.h"
#include <sys/cdefs.h>

int
fgetc(FILE *stream)
{
	int rv;
	__ungetc_t unget;

	if ((stream->flags & __SRD) == 0)
		return EOF;

	if ((unget = __atomic_exchange_ungetc(&stream->unget, 0)) != 0)
                return (unsigned char) (unget - 1);

	rv = stream->get(stream);
	if (rv < 0) {
		/* if != _FDEV_ERR, assume it's _FDEV_EOF */
		stream->flags |= (rv == _FDEV_ERR)? __SERR: __SEOF;
		return EOF;
	}

	return (unsigned char)rv;
}

#ifdef _HAVE_ALIAS_ATTRIBUTE
__strong_reference(fgetc, getc);
#elif !defined(getc)
int getc(FILE *stream) { return fgetc(stream); }
#endif
