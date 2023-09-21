/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 1998 by Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

****************************************************************/

/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").	*/

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <string.h>
#include <locale.h>
#include "mprec.h"
#include "gdtoa.h"

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) && !defined(_SMALL_HEXDIG)
const unsigned char __hexdig[256]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,
	0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
#else /* !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) && !defined(_SMALL_HEXDIG) */
unsigned char
__hexdig_fun (unsigned char c)
{
	if(c>='0' && c<='9') return c-'0'+0x10;
	else if(c>='a' && c<='f') return c-'a'+0x10+10;
	else if(c>='A' && c<='F') return c-'A'+0x10+10;
	else return 0;
}
#endif /* !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) && !defined(_SMALL_HEXDIG) */

static void
rshift (_Bigint *b,
	int k)
{
	__ULong *x, *x1, *xe, y;
	int n;

	x = x1 = b->_x;
	n = k >> kshift;
	if (n < b->_wds) {
		xe = x + b->_wds;
		x += n;
		if (k &= kmask) {
			n = ULbits - k;
			y = *x++ >> k;
			while(x < xe) {
				*x1++ = (y | (*x << n)) & ALL_ON;
				y = *x++ >> k;
				}
			if ((*x1 = y) !=0)
				x1++;
			}
		else
			while(x < xe)
				*x1++ = *x++;
		}
	if ((b->_wds = x1 - b->_x) == 0)
		b->_x[0] = 0;
}

static _Bigint *
increment (
	_Bigint *b)
{
	__ULong *x, *xe;
	_Bigint *b1;
#ifdef Pack_16
	__ULong carry = 1, y;
#endif

	if (!b)
		return NULL;

	x = b->_x;
	xe = x + b->_wds;
#ifdef Pack_32
	do {
		if (*x < (__ULong)0xffffffffL) {
			++*x;
			return b;
			}
		*x++ = 0;
		} while(x < xe);
#else
	do {
		y = *x + carry;
		carry = y >> 16;
		*x++ = y & 0xffff;
		if (!carry)
			return b;
		} while(x < xe);
	if (carry)
#endif
	{
		if (b->_wds >= b->_maxwds) {
			b1 = Balloc(b->_k+1);
			if (!b1) {
				Bfree(b);
				return NULL;
			}
			Bcopy(b1, b);
			Bfree(b);
			b = b1;
			}
		b->_x[b->_wds++] = 1;
		}
	return b;
}


int
gethex (const char **sp, const FPI *fpi,
	Long *exp, _Bigint **bp, int sign, locale_t loc)
{
	_Bigint *b;
	const unsigned char *decpt, *s0, *s, *s1;
	int esign, havedig, irv, k, n, nbits, up, zret;
	__ULong L, lostbits, *x;
	Long e, e1;
#ifdef __HAVE_LOCALE_INFO__
	const unsigned char *decimalpoint = (const unsigned char *)
				      __get_numeric_locale(loc)->decimal_point;
	const size_t decp_len = strlen ((const char *) decimalpoint);
	const unsigned char decp_end = decimalpoint[decp_len - 1];
#else
	const unsigned char *decimalpoint = (const unsigned char *) ".";
	const size_t decp_len = 1;
	const unsigned char decp_end = (unsigned char) '.';
        (void) loc;
#endif

	havedig = 0;
	s0 = *(const unsigned char **)sp + 2;
	while(s0[havedig] == '0')
		havedig++;
	s0 += havedig;
	s = s0;
	decpt = 0;
	zret = 0;
	e = 0;
	if (!__get_hexdig(*s)) {
		zret = 1;
		if (strncmp ((const char *) s, (const char *) decimalpoint,
			     decp_len) != 0)
			goto pcheck;
		decpt = (s += decp_len);
		if (!__get_hexdig(*s))
			goto pcheck;
		while(*s == '0')
			s++;
		if (__get_hexdig(*s))
			zret = 0;
		havedig = 1;
		s0 = s;
		}
	while(__get_hexdig(*s))
		s++;
	if (strncmp ((const char *) s, (const char *) decimalpoint,
		     decp_len) == 0
	    && !decpt) {
		decpt = (s += decp_len);
		while(__get_hexdig(*s))
			s++;
		}
	if (decpt)
		e = -(((Long)(s-decpt)) << 2);
 pcheck:
	s1 = s;
	switch(*s) {
	  case 'p':
	  case 'P':
		esign = 0;
		switch(*++s) {
		  case '-':
			esign = 1;
			__PICOLIBC_FALLTHROUGH;
		  case '+':
			s++;
		  }
		if ((n = __get_hexdig(*s)) == 0 || n > 0x19) {
			s = s1;
			break;
			}
		e1 = n - 0x10;
		while((n = __get_hexdig(*++s)) !=0 && n <= 0x19)
			e1 = 10*e1 + n - 0x10;
		if (esign)
			e1 = -e1;
		e += e1;
	  }
	*sp = (char*)s;
	if (zret)
		return havedig ? STRTOG_Zero : STRTOG_NoNumber;
	n = s1 - s0 - 1;
	for(k = 0; n > 7; n >>= 1)
		k++;
	b = Balloc(k);
	if (!b)
		return STRTOG_NoNumber;
	x = b->_x;
	n = 0;
	L = 0;
	while(s1 > s0) {
		if (*--s1 == decp_end && s1 - decp_len + 1 >= s0
		    && strncmp ((const char *) s1 - decp_len + 1,
				(const char *) decimalpoint, decp_len) == 0) {
			s1 -= decp_len - 1; /* Note the --s1 above! */
			continue;
		}
		if (n == 32) {
			*x++ = L;
			L = 0;
			n = 0;
			}
		L |= (__ULong) (__get_hexdig(*s1) & 0x0f) << n;
		n += 4;
		}
	*x++ = L;
	b->_wds = n = x - b->_x;
	n = 32*n - hi0bits(L);
	nbits = fpi->nbits;
	lostbits = 0;
	x = b->_x;
	if (n > nbits) {
		n -= nbits;
		if (any_on(b,n)) {
			lostbits = 1;
			k = n - 1;
			if (x[k>>kshift] & (__ULong) 1 << (k & kmask)) {
				lostbits = 2;
				if (k > 1 && any_on(b,k-1))
					lostbits = 3;
				}
			}
		rshift(b, n);
		e += n;
		}
	else if (n < nbits) {
		n = nbits - n;
		b = lshift(b, n);
		e -= n;
		x = b->_x;
		}
	if (e > fpi->emax) {
 ovfl:
		Bfree(b);
		*bp = 0;
		return STRTOG_Infinite | STRTOG_Overflow | STRTOG_Inexhi;
		}
	irv = STRTOG_Normal;
	if (e < fpi->emin) {
		irv = STRTOG_Denormal;
		n = fpi->emin - e;
		if (n >= nbits) {
			switch (fpi->rounding) {
			  case FPI_Round_near:
				if (n == nbits && (n < 2 || any_on(b,n-1)))
					goto one_bit;
				break;
			  case FPI_Round_up:
				if (!sign)
					goto one_bit;
				break;
			  case FPI_Round_down:
				if (sign) {
 one_bit:
					*exp = fpi->emin;
					x[0] = b->_wds = 1;
					*bp = b;
					return STRTOG_Denormal | STRTOG_Inexhi
						| STRTOG_Underflow;
					}
			  }
			Bfree(b);
			*bp = 0;
			return STRTOG_Zero | STRTOG_Inexlo | STRTOG_Underflow;
			}
		k = n - 1;
		if (lostbits)
			lostbits = 1;
		else if (k > 0)
			lostbits = any_on(b,k);
		if (x[k>>kshift] & (__ULong) 1 << (k & kmask))
			lostbits |= 2;
		nbits -= n;
		rshift(b,n);
		e = fpi->emin;
		}
	if (lostbits) {
		up = 0;
		switch(fpi->rounding) {
		  case FPI_Round_zero:
			break;
		  case FPI_Round_near:
		    if ((lostbits & 2)
			    && ((lostbits & 1) | (x[0] & 1)))
				up = 1;
			break;
		  case FPI_Round_up:
			up = 1 - sign;
			break;
		  case FPI_Round_down:
			up = sign;
		  }
		if (up) {
			k = b->_wds;
			b = increment(b);
			if (!b)
				return STRTOG_NoNumber;
			x = b->_x;
			if (irv == STRTOG_Denormal) {
				if (nbits == fpi->nbits - 1
                                    && x[nbits >> kshift] & (__ULong) 1 << (nbits & kmask))
					irv =  STRTOG_Normal;
				}
			else if ((b->_wds > k)
			 || ((n = nbits & kmask) !=0
			     && (hi0bits(x[k-1]) < 32-n))) {
				rshift(b,1);
				if (++e > fpi->emax)
					goto ovfl;
				}
			irv |= STRTOG_Inexhi;
			}
		else
			irv |= STRTOG_Inexlo;
		}
	*bp = b;
	*exp = e;
	return irv;
}

