/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

static const long double
  zero = 0.0L,
  half = 0.5L,
  one = 1.0L,
  pi = 3.14159265358979323846264L,
  two63 = 9.223372036854775808e18L;

long double
sinpil(long double x)
{
  long double y, z;
  int n, ix;
  u_int32_t se, i0, i1;

  GET_LDOUBLE_WORDS (se, i0, i1, x);
  ix = se & 0x7fff;
  ix = (ix << 16) | (i0 >> 16);
  if (ix < 0x3ffd8000) /* 0.25 */
    return sinl (pi * x);
  y = -x;			/* x is assume negative */

  /*
   * argument reduction, make sure inexact flag not raised if input
   * is an integer
   */
  z = floorl (y);
  if (z != y)
    {				/* inexact anyway */
      y  *= 0.5L;
      y = 2.0L*(y - floorl(y));		/* y = |x| mod 2.0 */
      n = (int) (y*4.0L);
    }
  else
    {
      if (ix >= 0x403f8000)  /* 2^64 */
	{
	  y = zero; n = 0;		/* y must be even */
	}
      else
	{
	if (ix < 0x403e8000)  /* 2^63 */
	  z = y + two63;	/* exact */
	GET_LDOUBLE_WORDS (se, i0, i1, z);
	n = i1 & 1;
	y  = n;
	n <<= 2;
      }
    }

  switch (n)
    {
    case 0:
      y = sinl (pi * y);
      break;
    case 1:
    case 2:
      y = cosl (pi * (half - y));
      break;
    case 3:
    case 4:
      y = sinl (pi * (one - y));
      break;
    case 5:
    case 6:
      y = -cosl (pi * (y - 1.5L));
      break;
    default:
      y = sinl (pi * (y - 2.0L));
      break;
    }
  return -y;
}
