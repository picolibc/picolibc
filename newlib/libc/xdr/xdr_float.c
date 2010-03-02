
/*
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * xdr_float.c, Generic XDR routines implementation.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * These are the "floating point" xdr routines used to (de)serialize
 * most common data items.  See xdr.h for more info on the interface to
 * xdr.
 */

#include <sys/types.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include "xdr_private.h"

/*
 * NB: Not portable.
 * This routine works on machines with IEEE754 FP and Vaxen.
 * Assume that xdr_private.h arranges things so that one of
 *   1) __IEEE_LITTLE_ENDIAN
 *   2) __IEEE_BIG_ENDIAN
 *   3) __vax__
 * is #defined.  Otherwise, expect errors.
 */
#ifndef XDR_FLOAT_C
#define XDR_FLOAT_C
#endif

#if defined(__IEEE_LITTLE_ENDIAN) || defined(__IEEE_BIG_ENDIAN)

bool_t
_DEFUN (xdr_float, (xdrs, fp),
       XDR * xdrs _AND
       float *fp)
{
  switch (xdrs->x_op)
    {

    case XDR_ENCODE:
      return (XDR_PUTINT32 (xdrs, (int32_t *) fp));

    case XDR_DECODE:
      return (XDR_GETINT32 (xdrs, (int32_t *) fp));

    case XDR_FREE:
      return TRUE;
    }
  return FALSE;
}

#if !defined(_DOUBLE_IS_32BITS)
bool_t
_DEFUN (xdr_double, (xdrs, dp),
        XDR * xdrs _AND
	double *dp)
{
  int32_t *i32p;
  bool_t rv;

  switch (xdrs->x_op)
    {

    case XDR_ENCODE:
      i32p = (int32_t *) (void *) dp;
#if defined(__IEEE_BIG_ENDIAN)
      rv = XDR_PUTINT32 (xdrs, i32p);
      if (!rv)
        return (rv);
      rv = XDR_PUTINT32 (xdrs, i32p + 1);
#else /* must be __IEEE_LITTLE_ENDIAN */
      rv = XDR_PUTINT32 (xdrs, i32p + 1);
      if (!rv)
        return (rv);
      rv = XDR_PUTINT32 (xdrs, i32p);
#endif /* __IEEE_LITTLE_ENDIAN */
      return (rv);

    case XDR_DECODE:
      i32p = (int32_t *) (void *) dp;
#if defined(__IEEE_BIG_ENDIAN)
      rv = XDR_GETINT32 (xdrs, i32p);
      if (!rv)
        return (rv);
      rv = XDR_GETINT32 (xdrs, i32p + 1);
#else /* must be __IEEE_LITTLE_ENDIAN */
      rv = XDR_GETINT32 (xdrs, i32p + 1);
      if (!rv)
        return (rv);
      rv = XDR_GETINT32 (xdrs, i32p);
#endif /* __IEEE_LITTLE_ENDIAN */
      return (rv);

    case XDR_FREE:
      return TRUE;
    }
  return FALSE;
}
#endif /* !_DOUBLE_IS_32BITS */

#elif defined(__vax__)
#include "xdr_float_vax.c"
#endif

