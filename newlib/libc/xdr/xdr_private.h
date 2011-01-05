/* xdr_private.h - declarations of utility functions for porting xdr
 *
 * Copyright (c) 2009 Charles S. Wilson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _XDR_PRIVATE_H
#define _XDR_PRIVATE_H

#include <_ansi.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/param.h>

/* avoid including stdio header here */
#ifndef __VALIST
#ifdef __GNUC__
#define __VALIST __gnuc_va_list
#else
#define __VALIST char*
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void _EXFNPTR (xdr_vprintf_t, (const char *, va_list));

xdr_vprintf_t _EXFUN (xdr_set_vprintf, (xdr_vprintf_t));

void _EXFUN (xdr_vwarnx, (const char *, __VALIST)
              _ATTRIBUTE ((__format__ (__printf__, 1, 0))));

void _EXFUN (xdr_warnx, (const char *, ...)
              _ATTRIBUTE ((__format__ (__printf__, 1, 2))));

/* endian issues */
#include <machine/endian.h>

/* byteswap and ntohl stuff; platform may provide optimzed version
 * of this, but we don't have access to that here.*/
_ELIDABLE_INLINE uint32_t xdr_ntohl (uint32_t x)
{
#if BYTE_ORDER == BIG_ENDIAN
  return x;
#elif BYTE_ORDER == LITTLE_ENDIAN
  u_char *s = (u_char *)&x;
  return (uint32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
#else
# error Unsupported endian type
#endif
}
#define xdr_htonl(x) xdr_ntohl(x)

#ifdef __cplusplus
}
#endif

#endif /* _XDR_PRIVATE_H */

