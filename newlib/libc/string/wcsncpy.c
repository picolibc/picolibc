/*
Copyright (c)1999 Citrus Project,
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/*
FUNCTION
	<<wcsncpy>>---copy part of a wide-character string 

SYNOPSIS
	#include <wchar.h>
	wchar_t *wcsncpy(wchar_t *__restrict <[s1]>,
			const wchar_t *__restrict <[s2]>, size_t <[n]>);

DESCRIPTION
	The <<wcsncpy>> function copies not more than <[n]> wide-character codes
	(wide-character codes that follow a null wide-character code are not
	copied) from the array pointed to by <[s2]> to the array pointed to
	by <[s1]>. If copying takes place between objects that overlap, the
	behaviour is undefined.  Note that if <[s1]> contains more than <[n]>
	wide characters before its terminating null, the result is not
	null-terminated.

	If the array pointed to by <[s2]> is a wide-character string that is
	shorter than <[n]> wide-character codes, null wide-character codes are
	appended to the copy in the array pointed to by <[s1]>, until <[n]>
	wide-character codes in all are written. 

RETURNS
	The <<wcsncpy>> function returns <[s1]>; no return value is reserved to
	indicate an error. 

PORTABILITY
ISO/IEC 9899; POSIX.1.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <wchar.h>

wchar_t *
wcsncpy (wchar_t *__restrict s1,
	const wchar_t *__restrict s2,
	size_t n)
{
  wchar_t *dscan=s1;

  while(n > 0)
    {
      --n;
      if((*dscan++ = *s2++) == L'\0')  break;
    }
  while(n-- > 0)  *dscan++ = L'\0';

  return s1;
}
