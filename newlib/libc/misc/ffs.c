/*
Copyright (c) 1981, 1993
The Regents of the University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
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
	<<ffs>>---find first bit set in a word

INDEX
	ffs

SYNOPSIS
	#include <strings.h>
	int ffs(int <[word]>);

DESCRIPTION

<<ffs>> returns the first bit set in a word.

RETURNS
<<ffs>> returns 0 if <[c]> is 0, 1 if <[c]> is odd, 2 if <[c]> is a multiple of
2, etc.

PORTABILITY
<<ffs>> is not ANSI C.

No supporting OS subroutines are required.  */

#include <strings.h>

/*
 * GCC calls "ffs" for __builtin_ffs when the target doesn't have
 * custom code and INT_TYPE_SIZE < BITS_PER_WORD so we can't use that
 * builtin to implement ffs here. Instead, fall back to the ctz code
 * instead.
 */
#if defined(_HAVE_BUILTIN_FFS) && defined(__GNUC__) && __INT_WIDTH__ != __LONG_WIDTH__
#undef _HAVE_BUILTIN_FFS
#endif

int
ffs(int i)
{
#ifdef _HAVE_BUILTIN_FFS
	return (__builtin_ffs(i));
#elif defined(_HAVE_BUILTIN_CTZ)
	if (i == 0)
		return 0;
	return __builtin_ctz((unsigned int)i) + 1;
#else
  int r;

  if (!i)
    return 0;

  r = 0;
  for (;;)
    {
      if (((1 << r++) & i) != 0)
	break;
    }
  return r;
#endif
}
