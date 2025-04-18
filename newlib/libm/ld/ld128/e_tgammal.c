/*	$OpenBSD: e_tgammal.c,v 1.1 2011/07/06 00:02:42 martynas Exp $	*/

/*
 * Copyright (c) 2011 Martynas Venckus <martynas@openbsd.org>
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



long double
tgammal(long double x)
{
	int64_t i0,i1;
        int sign;
        long double y, z;

	GET_LDOUBLE_WORDS64(i0,i1,x);
	if (((i0&0x7fffffffffffffffLL)|i1) == 0)
                return __math_divzerol(i0 < 0);

	if (i0<0 && (u_int64_t)i0<0xffff000000000000ULL && rintl(x)==x)
		return __math_invalidl(x);

	if ((uint64_t) i0==0xffff000000000000ULL && i1==0)
		return __math_invalidl(x);

        y = lgammal_r(x, &sign);
        z = expl(y);
        if (sign < 0)
            z = -z;
        return z;
}
