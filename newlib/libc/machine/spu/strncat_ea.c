/*
(C) Copyright IBM Corp. 2007, 2008

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

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

#include <string.h>
#include <stddef.h>
#include "ea_internal.h"
#include <ea.h>

COMPAT_EA_ALIAS (strncat_ea);

/*
 * Not the fastest thing ever since it reads through the data once on
 * strlen and once on memcpy.
 */
__ea char *
strncat_ea (__ea char *dest, __ea const char *src, size_ea_t n)
{
  size_ea_t length_src;
  size_ea_t length_dest;
  __ea char *new_dest;
  size_ea_t smaller_length;

  length_src = strlen_ea (src);
  length_dest = strlen_ea (dest);
  new_dest = dest + length_dest;
  smaller_length = length_src < n ? length_src : n;
  memcpy_ea ((__ea void *) new_dest, (__ea void *) src, smaller_length);
  /* null out last character */
  memset_ea ((__ea void *) (new_dest + smaller_length), 0, 1);

  return dest;
}
