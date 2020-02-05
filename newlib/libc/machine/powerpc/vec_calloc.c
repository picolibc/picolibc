/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

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
<<vec_calloc>>---allocate space for arrays

INDEX
	vec_calloc

INDEX
	_vec_calloc_r

SYNOPSIS
	#include <stdlib.h>
	void *vec_calloc(size_t <[n]>, size_t <[s]>);
	void *vec_calloc_r(void *<[reent]>, size_t <n>, <size_t> <[s]>);

DESCRIPTION
Use <<vec_calloc>> to request a block of memory sufficient to hold an
array of <[n]> elements, each of which has size <[s]>.

The memory allocated by <<vec_calloc>> comes out of the same memory pool
used by <<vec_malloc>>, but the memory block is initialized to all zero
bytes.  (To avoid the overhead of initializing the space, use
<<vec_malloc>> instead.)

The alternate function <<_vec_calloc_r>> is reentrant.
The extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
If successful, a pointer to the newly allocated space.

If unsuccessful, <<NULL>>.

PORTABILITY
<<vec_calloc>> is an non-ANSI extension described in the AltiVec Programming
Interface Manual.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <string.h>
#include <stdlib.h>

#ifndef _REENT_ONLY

void *
vec_calloc (size_t n,
	size_t size)
{
  return _vec_calloc_r (_REENT, n, size);
}

#endif
