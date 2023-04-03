/* Copyright (c) 2002, Joerg Wunsch
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

#include <stdio.h>
#include "stdio_private.h"
#include <sys/cdefs.h>
#include <wchar.h>

wint_t
fgetwc(FILE *stream)
{
        union {
                wchar_t wc;
                char c[sizeof(wchar_t)];
        } u;
        unsigned i;
        int sc;
        __ungetc_t unget;

	if ((stream->flags & __SRD) == 0)
		return WEOF;

	if ((unget = __atomic_exchange_ungetc(&stream->unget, 0)) != 0)
		return (wint_t) (unget - 1);

        for (i = 0; i < sizeof(wchar_t); i++) {
                sc = stream->get(stream);
                if (sc < 0)
                        return WEOF;
                u.c[i] = (char) sc;
        }

	return (wint_t) u.wc;
}

#ifdef _HAVE_ALIAS_ATTRIBUTE
__strong_reference(fgetwc, getwc);
#elif !defined(getwc)
wint_t getwc(FILE *stream) { return fgetwc(stream); }
#endif
