/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

/* Please send bug reports to
	David M. Gay
	AT&T Bell Laboratories, Room 2C-463
	600 Mountain Avenue
	Murray Hill, NJ 07974-2070
	U.S.A.
	dmg@research.att.com or research!dmg
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *	1. We only require IEEE, IBM, or VAX double-precision
 *		arithmetic (not IEEE double-extended).
 *	2. We get by with floating-point arithmetic in a case that
 *		Clinger missed -- when we're computing d * 10^n
 *		for a small integer d and the integer n is not too
 *		much larger than 22 (the maximum integer k for which
 *		we can represent 10^k exactly), we may be able to
 *		compute (d*10^k) * 10^(e-k) with just one roundoff.
 *	3. Rather than a bit-at-a-time adjustment of the binary
 *		result in the hard case, we use floating-point
 *		arithmetic to determine the adjustment to within
 *		one bit; only in really hard cases do we need to
 *		compute a second residual.
 *	4. Because of 3., we don't need a large table of powers of 10
 *		for ten-to-e (just some small tables, e.g. of 10^k
 *		for 0 <= k <= 22).
 */

/*
 * #define IEEE_8087 for IEEE-arithmetic machines where the least
 *	significant byte has the lowest address.
 * #define IEEE_MC68k for IEEE-arithmetic machines where the most
 *	significant byte has the lowest address.
 * #define Sudden_Underflow for IEEE-format machines without gradual
 *	underflow (i.e., that flush to zero on underflow).
 * #define IBM for IBM mainframe-style floating-point arithmetic.
 * #define VAX for VAX-style floating-point arithmetic.
 * #define Unsigned_Shifts if >> does treats its left operand as unsigned.
 * #define No_leftright to omit left-right logic in fast floating-point
 *	computation of dtoa.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3.
 * #define RND_PRODQUOT to use rnd_prod and rnd_quot (assembly routines
 *	that use extended-precision instructions to compute rounded
 *	products and quotients) with IBM.
 * #define ROUND_BIASED for IEEE-format with biased rounding.
 * #define Inaccurate_Divide for IEEE-format with correctly rounded
 *	products but inaccurate quotients, e.g., for Intel i860.
 * #define Just_16 to store 16 bits per 32-bit long when doing high-precision
 *	integer arithmetic.  Whether this speeds things up or slows things
 *	down depends on the machine and the number being converted.
 */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdlib.h>
#include <string.h>
#include "mprec.h"
#include "atexit.h"

/* This is defined in sys/reent.h as (sizeof (size_t) << 3) now, as in NetBSD.
   The old value of 15 was wrong and made newlib vulnerable against buffer
   overrun attacks (CVE-2009-0689), same as other implementations of gdtoa
   based on BSD code.
#define _Kmax 15
*/
/* This value is used in stdlib/misc.c.  reent/reent.c has to know it
   as well to make sure the freelist is correctly free'd.  Therefore
   we define it here, rather than in stdlib/misc.c, as before. */
#define _Kmax (sizeof (size_t) << 3)

static NEWLIB_THREAD_LOCAL char *__dtoa_result;
static NEWLIB_THREAD_LOCAL int __dtoa_result_len;

char *__alloc_dtoa_result(int len)
{
  if (len > __dtoa_result_len) {
    if (__mprec_register_exit() != 0)
      return NULL;
    if (__dtoa_result)
      free(__dtoa_result);
    __dtoa_result = malloc(len);
    if (__dtoa_result)
      __dtoa_result_len = len;
  }
  return __dtoa_result;
}

static NEWLIB_THREAD_LOCAL int _mprec_exit_registered;

static void
__mprec_exit(void)
{
  if (__dtoa_result) {
    free(__dtoa_result);
    __dtoa_result = NULL;
    __dtoa_result_len = 0;
  }
}

int
__mprec_register_exit(void)
{
  if (!_mprec_exit_registered) {
    _mprec_exit_registered = 1;
    return atexit(__mprec_exit);
  }
  return 0;
}

_Bigint *
Balloc (int k)
{
  int x;
  _Bigint *rv ;

  x = 1 << k;
  /* Allocate an mprec Bigint */
  rv = (_Bigint *) calloc(1,
			  sizeof (_Bigint) +
			  (x-1) * sizeof(rv->_x));
  if (rv == NULL) return NULL;
  rv->_k = k;
  rv->_maxwds = x;
  rv->_sign = rv->_wds = 0;
  return rv;
}

void
Bfree (_Bigint * v)
{
  free(v);
}

_Bigint *
multadd (
	_Bigint * b,
	int m,
	int a)
{
  int i, wds;
  __ULong *x, y;
#ifdef Pack_32
  __ULong xi, z;
#endif
  _Bigint *b1;

  if (!b)
    return NULL;

  wds = b->_wds;
  x = b->_x;
  i = 0;
  do
    {
#ifdef Pack_32
      xi = *x;
      y = (xi & 0xffff) * m + a;
      z = (xi >> 16) * m + (y >> 16);
      a = (int) (z >> 16);
      *x++ = (z << 16) + (y & 0xffff);
#else
      y = *x * m + a;
      a = (int) (y >> 16);
      *x++ = y & 0xffff;
#endif
    }
  while (++i < wds);
  if (a)
    {
      if (wds >= b->_maxwds)
	{
	  b1 = Balloc (b->_k + 1);
	  if (!b1) {
	    Bfree(b);
	    return NULL;
	  }
	  Bcopy (b1, b);
	  Bfree (b);
	  b = b1;
	}
      b->_x[wds++] = a;
      b->_wds = wds;
    }
  return b;
}

_Bigint *
s2b (
	const char *s,
	int nd0,
	int nd,
	__ULong y9)
{
  _Bigint *b;
  int i, k;
  __Long x, y;

  x = (nd + 8) / 9;
  for (k = 0, y = 1; x > y; y <<= 1, k++);
#ifdef Pack_32
  b = Balloc (k);
  if (!b)
    return NULL;
  b->_x[0] = y9;
  b->_wds = 1;
#else
  b = Balloc (k + 1);
  if (!b)
    return NULL;
  b->_x[0] = y9 & 0xffff;
  b->_wds = (b->_x[1] = y9 >> 16) ? 2 : 1;
#endif

  i = 9;
  if (9 < nd0)
    {
      s += 9;
      do
	b = multadd (b, 10, *s++ - '0');
      while (++i < nd0);
      s++;
    }
  else
    s += 10;
  for (; i < nd; i++)
    b = multadd (b, 10, *s++ - '0');
  return b;
}

int
hi0bits (register __ULong x)
{
  register int k = 0;

  if (!(x & 0xffff0000))
    {
      k = 16;
      x <<= 16;
    }
  if (!(x & 0xff000000))
    {
      k += 8;
      x <<= 8;
    }
  if (!(x & 0xf0000000))
    {
      k += 4;
      x <<= 4;
    }
  if (!(x & 0xc0000000))
    {
      k += 2;
      x <<= 2;
    }
  if (!(x & 0x80000000))
    {
      k++;
      if (!(x & 0x40000000))
	return 32;
    }
  return k;
}

int
lo0bits (__ULong *y)
{
  register int k;
  register __ULong x = *y;

  if (x & 7)
    {
      if (x & 1)
	return 0;
      if (x & 2)
	{
	  *y = x >> 1;
	  return 1;
	}
      *y = x >> 2;
      return 2;
    }
  k = 0;
  if (!(x & 0xffff))
    {
      k = 16;
      x >>= 16;
    }
  if (!(x & 0xff))
    {
      k += 8;
      x >>= 8;
    }
  if (!(x & 0xf))
    {
      k += 4;
      x >>= 4;
    }
  if (!(x & 0x3))
    {
      k += 2;
      x >>= 2;
    }
  if (!(x & 1))
    {
      k++;
      x >>= 1;
      if (!(x & 1))
	return 32;
    }
  *y = x;
  return k;
}

_Bigint *
i2b (int i)
{
  _Bigint *b;

  b = Balloc (1);
  if (b) {
    b->_x[0] = i;
    b->_wds = 1;
  }
  return b;
}

_Bigint *
mult (_Bigint * a, _Bigint * b)
{
  _Bigint *c;
  int k, wa, wb, wc;
  __ULong carry, y, z;
  __ULong *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
#ifdef Pack_32
  __ULong z2;
#endif

  if (!a || !b)
    return NULL;

  if (a->_wds < b->_wds)
    {
      c = a;
      a = b;
      b = c;
    }
  k = a->_k;
  wa = a->_wds;
  wb = b->_wds;
  wc = wa + wb;
  if (wc > a->_maxwds)
    k++;
  c = Balloc (k);
  if (!c)
    return NULL;
  for (x = c->_x, xa = x + wc; x < xa; x++)
    *x = 0;
  xa = a->_x;
  xae = xa + wa;
  xb = b->_x;
  xbe = xb + wb;
  xc0 = c->_x;
#ifdef Pack_32
  for (; xb < xbe; xb++, xc0++)
    {
      if ((y = *xb & 0xffff) != 0)
	{
	  x = xa;
	  xc = xc0;
	  carry = 0;
	  do
	    {
	      z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
	      carry = z >> 16;
	      z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
	      carry = z2 >> 16;
	      Storeinc (xc, z2, z);
	    }
	  while (x < xae);
	  *xc = carry;
	}
      if ((y = *xb >> 16) != 0)
	{
	  x = xa;
	  xc = xc0;
	  carry = 0;
	  z2 = *xc;
	  do
	    {
	      z = (*x & 0xffff) * y + (*xc >> 16) + carry;
	      carry = z >> 16;
	      Storeinc (xc, z, z2);
	      z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
	      carry = z2 >> 16;
	    }
	  while (x < xae);
	  *xc = z2;
	}
    }
#else
  for (; xb < xbe; xc0++)
    {
      if (y = *xb++)
	{
	  x = xa;
	  xc = xc0;
	  carry = 0;
	  do
	    {
	      z = *x++ * y + *xc + carry;
	      carry = z >> 16;
	      *xc++ = z & 0xffff;
	    }
	  while (x < xae);
	  *xc = carry;
	}
    }
#endif
  for (xc0 = c->_x, xc = xc0 + wc; wc > 0 && !*--xc; --wc);
  c->_wds = wc;
  return c;
}

_Bigint *
pow5mult (_Bigint * b, int k)
{
  _Bigint *b1, *p5, *p51;
  int i;
  static const int p05[3] = {5, 25, 125};

  if ((i = k & 3) != 0)
    b = multadd (b, p05[i - 1], 0);

  if (!(k >>= 2))
    return b;
  p5 = i2b (625);
  for (;;)
    {
      if (k & 1)
	{
	  b1 = mult (b, p5);
	  Bfree (b);
	  b = b1;
	}
      if (!(k >>= 1))
	break;
      p51 = mult (p5, p5);
      Bfree(p5);
      p5 = p51;
    }
  Bfree(p5);
  return b;
}

_Bigint *
lshift (_Bigint * b, int k)
{
  int i, k1, n, n1;
  _Bigint *b1;
  __ULong *x, *x1, *xe, z;

  if (!b)
    return NULL;

#ifdef Pack_32
  n = k >> 5;
#else
  n = k >> 4;
#endif
  k1 = b->_k;
  n1 = n + b->_wds + 1;
  for (i = b->_maxwds; n1 > i; i <<= 1)
    k1++;
  b1 = Balloc (k1);
  if (!b1)
    goto bail;

  x1 = b1->_x;
  for (i = 0; i < n; i++)
    *x1++ = 0;
  x = b->_x;
  xe = x + b->_wds;
#ifdef Pack_32
  if (k &= 0x1f)
    {
      k1 = 32 - k;
      z = 0;
      do
	{
	  *x1++ = *x << k | z;
	  z = *x++ >> k1;
	}
      while (x < xe);
      if ((*x1 = z) != 0)
	++n1;
    }
#else
  if (k &= 0xf)
    {
      k1 = 16 - k;
      z = 0;
      do
	{
	  *x1++ = *x << k & 0xffff | z;
	  z = *x++ >> k1;
	}
      while (x < xe);
      if (*x1 = z)
	++n1;
    }
#endif
  else
    do
      *x1++ = *x++;
    while (x < xe);
  b1->_wds = n1 - 1;
bail:
  Bfree (b);
  return b1;
}

int
cmp (_Bigint * a, _Bigint * b)
{
  __ULong *xa, *xa0, *xb, *xb0;
  int i, j;

  if (!a || !b)
    return 0;

  i = a->_wds;
  j = b->_wds;
#ifdef DEBUG
  if (i > 1 && !a->_x[i - 1])
    Bug ("cmp called with a->_x[a->_wds-1] == 0");
  if (j > 1 && !b->_x[j - 1])
    Bug ("cmp called with b->_x[b->_wds-1] == 0");
#endif
  if (i -= j)
    return i;
  xa0 = a->_x;
  xa = xa0 + j;
  xb0 = b->_x;
  xb = xb0 + j;
  for (;;)
    {
      if (*--xa != *--xb)
	return *xa < *xb ? -1 : 1;
      if (xa <= xa0)
	break;
    }
  return 0;
}

_Bigint *
diff (
	_Bigint * a, _Bigint * b)
{
  _Bigint *c;
  int i, wa, wb;
  __Long borrow, y;		/* We need signed shifts here. */
  __ULong *xa, *xae, *xb, *xbe, *xc;
#ifdef Pack_32
  __Long z;
#endif

  i = cmp (a, b);
  if (!i)
    {
      c = Balloc (0);
      if (!c)
	return NULL;
      c->_wds = 1;
      c->_x[0] = 0;
      return c;
    }
  if (i < 0)
    {
      c = a;
      a = b;
      b = c;
      i = 1;
    }
  else
    i = 0;
  c = Balloc (a->_k);
  if (!c)
    return NULL;
  c->_sign = i;
  wa = a->_wds;
  xa = a->_x;
  xae = xa + wa;
  wb = b->_wds;
  xb = b->_x;
  xbe = xb + wb;
  xc = c->_x;
  borrow = 0;
#ifdef Pack_32
  do
    {
      y = (*xa & 0xffff) - (*xb & 0xffff) + borrow;
      borrow = y >> 16;
      Sign_Extend (borrow, y);
      z = (*xa++ >> 16) - (*xb++ >> 16) + borrow;
      borrow = z >> 16;
      Sign_Extend (borrow, z);
      Storeinc (xc, z, y);
    }
  while (xb < xbe);
  while (xa < xae)
    {
      y = (*xa & 0xffff) + borrow;
      borrow = y >> 16;
      Sign_Extend (borrow, y);
      z = (*xa++ >> 16) + borrow;
      borrow = z >> 16;
      Sign_Extend (borrow, z);
      Storeinc (xc, z, y);
    }
#else
  do
    {
      y = *xa++ - *xb++ + borrow;
      borrow = y >> 16;
      Sign_Extend (borrow, y);
      *xc++ = y & 0xffff;
    }
  while (xb < xbe);
  while (xa < xae)
    {
      y = *xa++ + borrow;
      borrow = y >> 16;
      Sign_Extend (borrow, y);
      *xc++ = y & 0xffff;
    }
#endif
  while (!*--xc)
    wa--;
  c->_wds = wa;
  return c;
}

double
ulp (double _x)
{
  union double_union x, a;
  register __Long L;

  x.d = _x;

  L = (word0 (x) & Exp_mask) - (P - 1) * Exp_msk1;
#ifndef Sudden_Underflow
  if (L > 0)
    {
#endif
#ifdef IBM
      L |= Exp_msk1 >> 4;
#endif
      word0 (a) = L;
#ifndef _DOUBLE_IS_32BITS
      word1 (a) = 0;
#endif

#ifndef Sudden_Underflow
    }
  else
    {
      L = -L >> Exp_shift;
      if (L < Exp_shift)
	{
	  word0 (a) = 0x80000 >> L;
#ifndef _DOUBLE_IS_32BITS
	  word1 (a) = 0;
#endif
	}
      else
	{
	  word0 (a) = 0;
	  L -= Exp_shift;
#ifndef _DOUBLE_IS_32BITS
         word1 (a) = L >= 31 ? 1 : 1 << (31 - L);
#endif
	}
    }
#endif
  return a.d;
}

double
b2d (_Bigint * a, int *e)
{
  __ULong *xa, *xa0, y, z;
#if !defined(Pack_32) || !defined(_DOUBLE_IS_32BITS)
  __ULong w;
#endif
  int k;
  union double_union d;
#ifdef VAX
  __ULong d0, d1;
#else
#define d0 word0(d)
#define d1 word1(d)
#endif

  xa0 = a->_x;
  xa = xa0 + a->_wds;
  y = *--xa;
#ifdef DEBUG
  if (!y)
    Bug ("zero y in b2d");
#endif
  k = hi0bits (y);
  *e = 32 - k;
#ifdef Pack_32
  if (k < Ebits)
    {
      d0 = Exp_1 | y >> (Ebits - k);
#ifndef _DOUBLE_IS_32BITS
      w = xa > xa0 ? *--xa : 0;
      d1 = y << ((32 - Ebits) + k) | w >> (Ebits - k);
#endif
      goto ret_d;
    }
  z = xa > xa0 ? *--xa : 0;
  if (k -= Ebits)
    {
      d0 = Exp_1 | y << k | z >> (32 - k);
      y = xa > xa0 ? *--xa : 0;
#ifndef _DOUBLE_IS_32BITS
      d1 = z << k | y >> (32 - k);
#endif
    }
  else
    {
      d0 = Exp_1 | y;
#ifndef _DOUBLE_IS_32BITS
      d1 = z;
#endif
    }
#else
  if (k < Ebits + 16)
    {
      z = xa > xa0 ? *--xa : 0;
      d0 = Exp_1 | y << k - Ebits | z >> Ebits + 16 - k;
      w = xa > xa0 ? *--xa : 0;
      y = xa > xa0 ? *--xa : 0;
      d1 = z << k + 16 - Ebits | w << k - Ebits | y >> 16 + Ebits - k;
      goto ret_d;
    }
  z = xa > xa0 ? *--xa : 0;
  w = xa > xa0 ? *--xa : 0;
  k -= Ebits + 16;
  d0 = Exp_1 | y << k + 16 | z << k | w >> 16 - k;
  y = xa > xa0 ? *--xa : 0;
  d1 = w << k + 16 | y << k;
#endif
ret_d:
#ifdef VAX
  word0 (d) = d0 >> 16 | d0 << 16;
  word1 (d) = d1 >> 16 | d1 << 16;
#else
#undef d0
#undef d1
#endif
  return d.d;
}

_Bigint *
d2b (
	double _d,
	int *e,
	int *bits)

{
  union double_union d;
  _Bigint *b;
  int de, i, k;
  __ULong *x, z;
#if !defined(Pack_32) || !defined(_DOUBLE_IS_32BITS)
  __ULong y;
#endif
#ifdef VAX
  __ULong d0, d1;
#endif
  d.d = _d;
#ifdef VAX
  d0 = word0 (d) >> 16 | word0 (d) << 16;
  d1 = word1 (d) >> 16 | word1 (d) << 16;
#else
#define d0 word0(d)
#define d1 word1(d)
  d.d = _d;
#endif

#ifdef Pack_32
  b = Balloc (1);
#else
  b = Balloc (2);
#endif
  if (!b)
    return NULL;
  x = b->_x;

  z = d0 & Frac_mask;
  d0 &= 0x7fffffff;		/* clear sign bit, which we ignore */
#ifdef Sudden_Underflow
  de = (int) (d0 >> Exp_shift);
#ifndef IBM
  z |= Exp_msk11;
#endif
#else
  if ((de = (int) (d0 >> Exp_shift)) != 0)
    z |= Exp_msk1;
#endif
#ifdef Pack_32
#ifndef _DOUBLE_IS_32BITS
  if (d1)
    {
      y = d1;
      k = lo0bits (&y);
      if (k)
	{
         x[0] = y | z << (32 - k);
	  z >>= k;
	}
      else
	x[0] = y;
      i = b->_wds = (x[1] = z) ? 2 : 1;
    }
  else
#endif
    {
#ifdef DEBUG
      if (!z)
	Bug ("Zero passed to d2b");
#endif
      k = lo0bits (&z);
      x[0] = z;
      i = b->_wds = 1;
#ifndef _DOUBLE_IS_32BITS
      k += 32;
#endif
    }
#else
  if (d1)
    {
      y = d1;
      k = lo0bits (&y);
      if (k)
	if (k >= 16)
	  {
	    x[0] = y | z << 32 - k & 0xffff;
	    x[1] = z >> k - 16 & 0xffff;
	    x[2] = z >> k;
	    i = 2;
	  }
	else
	  {
	    x[0] = y & 0xffff;
	    x[1] = y >> 16 | z << 16 - k & 0xffff;
	    x[2] = z >> k & 0xffff;
	    x[3] = z >> k + 16;
	    i = 3;
	  }
      else
	{
	  x[0] = y & 0xffff;
	  x[1] = y >> 16;
	  x[2] = z & 0xffff;
	  x[3] = z >> 16;
	  i = 3;
	}
    }
  else
    {
#ifdef DEBUG
      if (!z)
	Bug ("Zero passed to d2b");
#endif
      k = lo0bits (&z);
      if (k >= 16)
	{
	  x[0] = z;
	  i = 0;
	}
      else
	{
	  x[0] = z & 0xffff;
	  x[1] = z >> 16;
	  i = 1;
	}
      k += 32;
    }
  while (!x[i])
    --i;
  b->_wds = i + 1;
#endif
#ifndef Sudden_Underflow
  if (de)
    {
#endif
#ifdef IBM
      *e = (de - Bias - (P - 1) << 2) + k;
      *bits = 4 * P + 8 - k - hi0bits (word0 (d) & Frac_mask);
#else
      *e = de - Bias - (P - 1) + k;
      *bits = P - k;
#endif
#ifndef Sudden_Underflow
    }
  else
    {
      *e = de - Bias - (P - 1) + 1 + k;
#ifdef Pack_32
      *bits = 32 * i - hi0bits (x[i - 1]);
#else
      *bits = (i + 2) * 16 - hi0bits (x[i]);
#endif
    }
#endif
  return b;
}
#undef d0
#undef d1

double
ratio (_Bigint * a, _Bigint * b)

{
  union double_union da, db;
  int k, ka, kb;

  da.d = b2d (a, &ka);
  db.d = b2d (b, &kb);
#ifdef Pack_32
  k = ka - kb + 32 * (a->_wds - b->_wds);
#else
  k = ka - kb + 16 * (a->_wds - b->_wds);
#endif
#ifdef IBM
  if (k > 0)
    {
      word0 (da) += (k >> 2) * Exp_msk1;
      if (k &= 3)
	da.d *= 1 << k;
    }
  else
    {
      k = -k;
      word0 (db) += (k >> 2) * Exp_msk1;
      if (k &= 3)
	db.d *= 1 << k;
    }
#else
  if (k > 0)
    word0 (da) += k * Exp_msk1;
  else
    {
      k = -k;
      word0 (db) += k * Exp_msk1;
    }
#endif
  return da.d / db.d;
}


const double
  tens[] =
{
  1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
  1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
  1e20, 1e21, 1e22, 1e23, 1e24

};

#if !defined(_DOUBLE_IS_32BITS) && !defined(__v800)
const double bigtens[] =
{1e16, 1e32, 1e64, 1e128, 1e256};

const double tinytens[] =
{1e-16, 1e-32, 1e-64, 1e-128, 1e-256};
#else
const double bigtens[] =
{1e16, 1e32};

const double tinytens[] =
{1e-16, 1e-32};
#endif


double
_mprec_log10 (int dig)
{
  double v = 1.0;
  if (dig < 24)
    return tens[dig];
  while (dig > 0)
    {
      v *= 10;
      dig--;
    }
  return v;
}

void
copybits (__ULong *c,
	int n,
	_Bigint *b)
{
	__ULong *ce, *x, *xe;
#ifdef Pack_16
	int nw, nw1;
#endif

	ce = c + ((n-1) >> kshift) + 1;
	x = b->_x;
#ifdef Pack_32
	xe = x + b->_wds;
	while(x < xe)
		*c++ = *x++;
#else
	nw = b->_wds;
	nw1 = nw & 1;
	for(xe = x + (nw - nw1); x < xe; x += 2)
		Storeinc(c, x[1], x[0]);
	if (nw1)
		*c++ = *x;
#endif
	while(c < ce)
		*c++ = 0;
}

__ULong
any_on (_Bigint *b,
	int k)
{
	int n, nwds;
	__ULong *x, *x0, x1, x2;

	x = b->_x;
	nwds = b->_wds;
	n = k >> kshift;
	if (n > nwds)
		n = nwds;
	else if (n < nwds && (k &= kmask)) {
		x1 = x2 = x[n];
		x1 >>= k;
		x1 <<= k;
		if (x1 != x2)
			return 1;
		}
	x0 = x;
	x += n;
	while(x > x0)
		if (*--x)
			return 1;
	return 0;
}

