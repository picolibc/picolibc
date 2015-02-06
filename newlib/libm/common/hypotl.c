/*
(C) Copyright IBM Corp. 2009

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

#include <math.h>
#include <errno.h>
#include "fdlibm.h"

long double
hypotl (long double x, long double y)
{
#ifdef _LDBL_EQ_DBL

  /* On platforms where long double is as wide as double.  */
  return hypot(x, y);

#else

  long double z;

  z = __ieee754_hypotl (x, y);

  if (_LIB_VERSION == _IEEE_)
    return z;

  if ((! finitel (z)) && finitel (x) && finitel (y))
    {
      /* hypot (finite, finite) overflow.  */
      struct exception exc;

      exc.type = OVERFLOW;
      exc.name = "hypotl";
      exc.err = 0;
      exc.arg1 = x;
      exc.arg2 = y;

      if (_LIB_VERSION == _SVID_)
	exc.retval = HUGE;
      else
	{
#ifndef HUGE_VAL 
#define HUGE_VAL inf
	  double inf = 0.0;

	  SET_HIGH_WORD (inf, 0x7ff00000);	/* Set inf to infinite.  */
#endif
	  exc.retval = HUGE_VAL;
	}

      if (_LIB_VERSION == _POSIX_)
	errno = ERANGE;
      else if (! matherr (& exc))
	errno = ERANGE;

      if (exc.err != 0)
	errno = exc.err;

      return (long double) exc.retval; 
    }

  return z;
#endif /* ! _LDBL_EQ_DBL */
}
