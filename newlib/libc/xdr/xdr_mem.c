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
 * xdr_mem.h, XDR implementation using memory buffers.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * If you have some data to be interpreted as external data representation
 * or to be converted to external data representation in a memory buffer,
 * then this is the package for you.
 *
 */

#include <sys/types.h>
#include <string.h>
#include <limits.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include "xdr_private.h"

#ifndef ntohl
# define ntohl(x) xdr_ntohl(x)
#endif
#ifndef htonl
# define htonl(x) xdr_htonl(x)
#endif

static void _EXFUN (xdrmem_destroy, (XDR *));
static bool_t _EXFUN (xdrmem_getlong_aligned, (XDR *, long *));
static bool_t _EXFUN (xdrmem_putlong_aligned, (XDR *, _CONST long *));
static bool_t _EXFUN (xdrmem_getlong_unaligned, (XDR *, long *));
static bool_t _EXFUN (xdrmem_putlong_unaligned, (XDR *, _CONST long *));
static bool_t _EXFUN (xdrmem_getbytes, (XDR *, char *, u_int));
static bool_t _EXFUN (xdrmem_putbytes, (XDR *, _CONST char *, u_int));
/* XXX: w/64-bit pointers, u_int not enough! */
static u_int _EXFUN (xdrmem_getpos, (XDR *));
static bool_t _EXFUN (xdrmem_setpos, (XDR *, u_int));
static int32_t * _EXFUN (xdrmem_inline_aligned, (XDR *, u_int));
static int32_t * _EXFUN (xdrmem_inline_unaligned, (XDR *, u_int));
static bool_t  _EXFUN (xdrmem_getint32_aligned, (XDR *, int32_t *));
static bool_t  _EXFUN (xdrmem_putint32_aligned, (XDR *, _CONST int32_t *));
static bool_t  _EXFUN (xdrmem_getint32_unaligned, (XDR *, int32_t *));
static bool_t  _EXFUN (xdrmem_putint32_unaligned, (XDR *, _CONST int32_t *));

static _CONST struct xdr_ops xdrmem_ops_aligned = {
  xdrmem_getlong_aligned,
  xdrmem_putlong_aligned,
  xdrmem_getbytes,
  xdrmem_putbytes,
  xdrmem_getpos,
  xdrmem_setpos,
  xdrmem_inline_aligned,
  xdrmem_destroy,
  xdrmem_getint32_aligned,
  xdrmem_putint32_aligned
};

static _CONST struct xdr_ops xdrmem_ops_unaligned = {
  xdrmem_getlong_unaligned,
  xdrmem_putlong_unaligned,
  xdrmem_getbytes,
  xdrmem_putbytes,
  xdrmem_getpos,
  xdrmem_setpos,
  xdrmem_inline_unaligned,
  xdrmem_destroy,
  xdrmem_getint32_unaligned,
  xdrmem_putint32_unaligned
};

/*
 * The procedure xdrmem_create initializes a stream descriptor for a
 * memory buffer.
 */
void
_DEFUN (xdrmem_create, (xdrs, addr, size, op),
        XDR * xdrs _AND
	caddr_t addr _AND
	u_int size _AND
	enum xdr_op op)
{
  xdrs->x_op = op;
  xdrs->x_ops = ((unsigned long)addr & (sizeof (int32_t) - 1))
    ? (struct xdr_ops *)&xdrmem_ops_unaligned
    : (struct xdr_ops *)&xdrmem_ops_aligned;
  xdrs->x_private = xdrs->x_base = addr;
  xdrs->x_handy = size;
}

static void
_DEFUN (xdrmem_destroy, (xdrs),
       XDR * xdrs)
{
}

static bool_t
_DEFUN (xdrmem_getlong_aligned, (xdrs, lp),
        XDR * xdrs _AND
	long *lp)
{
  if (xdrs->x_handy < sizeof (int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof (int32_t);
  *lp = (int32_t) ntohl (*(u_int32_t *) xdrs->x_private);
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_putlong_aligned, (xdrs, lp),
        XDR * xdrs _AND
	_CONST long *lp)
{
  if (xdrs->x_handy < sizeof (int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof (int32_t);
  *(u_int32_t *) xdrs->x_private = htonl ((u_int32_t) * lp);
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_getlong_unaligned, (xdrs, lp),
        XDR * xdrs _AND
	long *lp)
{
  u_int32_t l;

  if (xdrs->x_handy < sizeof (int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof (int32_t);
  memmove (&l, xdrs->x_private, sizeof (int32_t));
  *lp = ntohl (l);
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_putlong_unaligned, (xdrs, lp),
        XDR * xdrs _AND
	_CONST long *lp)
{
  u_int32_t l;

  if (xdrs->x_handy < sizeof (int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof (int32_t);
  l = htonl ((u_int32_t) * lp);
  memmove (xdrs->x_private, &l, sizeof (int32_t));
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_getbytes, (xdrs, addr, len),
        XDR * xdrs _AND
	char *addr _AND
	u_int len)
{
  if (xdrs->x_handy < len)
    return FALSE;
  xdrs->x_handy -= len;
  memmove (addr, xdrs->x_private, len);
  xdrs->x_private = (char *) xdrs->x_private + len;
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_putbytes, (xdrs, addr, len),
        XDR * xdrs _AND
	_CONST char *addr _AND
	u_int len)
{
  if (xdrs->x_handy < len)
    return FALSE;
  xdrs->x_handy -= len;
  memmove (xdrs->x_private, addr, len);
  xdrs->x_private = (char *) xdrs->x_private + len;
  return TRUE;
}

static u_int
_DEFUN (xdrmem_getpos, (xdrs),
        XDR * xdrs)
{
  /* XXX w/64-bit pointers, u_int not enough! */
  return (u_int) ((u_long) xdrs->x_private - (u_long) xdrs->x_base);
}

static bool_t
_DEFUN (xdrmem_setpos, (xdrs, pos),
        XDR * xdrs _AND
	u_int pos)
{
  caddr_t newaddr = xdrs->x_base + pos;
  caddr_t lastaddr = (caddr_t) xdrs->x_private + xdrs->x_handy;
  size_t handy = lastaddr - newaddr;

  if (newaddr > lastaddr
      || newaddr < xdrs->x_base
      || handy != (u_int) handy)
    return FALSE;

  xdrs->x_private = newaddr;
  xdrs->x_handy = (u_int) handy;
  /* XXX sizeof(u_int) <? sizeof(ptrdiff_t) */
  return TRUE;
}

static int32_t *
_DEFUN (xdrmem_inline_aligned, (xdrs, len),
        XDR * xdrs _AND
	u_int len)
{
  int32_t *buf = 0;

  if (xdrs->x_handy >= len)
    {
      xdrs->x_handy -= len;
      buf = (int32_t *) xdrs->x_private;
      xdrs->x_private = (char *) xdrs->x_private + len;
    }
  return (buf);
}

static int32_t *
_DEFUN (xdrmem_inline_unaligned, (xdrs, len),
        XDR * xdrs _AND
	u_int len)
{
  return (0);
}

static bool_t
_DEFUN (xdrmem_getint32_aligned, (xdrs, ip),
	XDR *xdrs _AND
	int32_t *ip)
{
  if (xdrs->x_handy < sizeof(int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof(int32_t);
  *ip = (int32_t) ntohl (*(u_int32_t *) xdrs->x_private);
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_putint32_aligned, (xdrs, ip),
        XDR *xdrs _AND
	_CONST int32_t *ip)
{
  if (xdrs->x_handy < sizeof(int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof(int32_t);
  *(u_int32_t *) xdrs->x_private = htonl ((u_int32_t) * ip);
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_getint32_unaligned, (xdrs, ip),
	XDR *xdrs _AND
	int32_t *ip)
{
  u_int32_t l;

  if (xdrs->x_handy < sizeof(int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof(int32_t);
  memmove (&l, xdrs->x_private, sizeof (int32_t));
  *ip = (int32_t) ntohl (l);
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

static bool_t
_DEFUN (xdrmem_putint32_unaligned, (xdrs, ip),
        XDR *xdrs _AND
	_CONST int32_t *ip)
{
  u_int32_t l;

  if (xdrs->x_handy < sizeof(int32_t))
    return FALSE;
  xdrs->x_handy -= sizeof(int32_t);
  l = htonl ((u_int32_t) * ip);
  memmove (xdrs->x_private, &l, sizeof (int32_t));
  xdrs->x_private = (char *) xdrs->x_private + sizeof (int32_t);
  return TRUE;
}

