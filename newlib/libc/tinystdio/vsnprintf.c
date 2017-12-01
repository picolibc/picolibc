/* Copyright (c) 2003, Joerg Wunsch
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

/* $Id: vsnprintf.c 1944 2009-04-01 23:12:20Z arcanum $ */

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include "sectionname.h"

ATTRIBUTE_CLIB_SECTION
int
vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
	FILE f;
	int i;

	f.flags = __SWR | __SSTR;
	f.buf = s;
	/* Restrict max output length to 32767, as snprintf() return
	   signed int. The fputc() function uses a signed comparison
	   between estimated len and f.size field. So we can write a
	   negative value into f.size in the case of n was 0. Note,
	   that f.size will be a max number of nonzero symbols.	*/
	if ((int)n < 0)
		n = (unsigned)INT_MAX + 1;		/* 32768 */
	f.size = n - 1;				/* -1,0,...32767 */

	i = vfprintf(&f, fmt, ap);

	/* We use f.size (not 'n') as this is more effective: two 'ld'
	   instructions vs. two 'push+pop' and 'movw'.	*/
	if (f.size >= 0)
		s[f.len < f.size ? f.len : f.size] = 0;

	return i;
}
