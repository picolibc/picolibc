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

#include "stdio_private.h"

wchar_t *
__STDIO_UNLOCKED(fgetws)(wchar_t *str, int size, FILE *stream)
{
	wchar_t *cp;
	wint_t c;

	if ((stream->flags & __SRD) == 0 || size <= 0)
		return NULL;

	size--;
	for (c = 0, cp = str; c != L'\n' && size > 0; size--, cp++) {
		if ((c = __STDIO_UNLOCKED(getwc)(stream)) == WEOF)
			return NULL;
		*cp = (wchar_t)c;
	}
	*cp = L'\0';

	return str;
}

#if defined(__STDIO_LOCKING) && !defined(_FILE_INCLUDED)
wchar_t *
fgetws(wchar_t *str, int size, FILE *stream)
{
    wchar_t *ret;
    __flockfile(stream);
    ret = __STDIO_UNLOCKED(fgetws)(str, size, stream);
    __funlockfile(stream);
    return ret;
}
#endif
