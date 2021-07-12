/* Copyright © 2017 Keith Packard
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

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
  POSSIBILITY OF SUCH DAMAGE. */

#include "xtoa_fast.h"

typedef struct {
	uint64_t	div;
	uint8_t		mod;
} divmod_t;

/*
 * Use the krufty divmod10 code when doing 64-bit
 * ultoa on targets where long is smaller than 64 bits,
 * which we're guessing happens when the target doesn't have
 * native 64-bit divides
 */
#if SIZEOF_ULTOA == 8 && __SIZEOF_LONG__ < 8

typedef struct {
	uint64_t	lo;
	uint8_t		hi;
} uint72_t;

/* Compute (x +1 ) * 51 */
static uint72_t mul51(uint64_t x)
{
	uint32_t	xlo = x;
	uint64_t	xhi = x >> 32;

	if (++xlo == 0)
		xhi++;

	uint64_t	mlo = (uint64_t) xlo * 51;
	uint64_t	mhi = (uint64_t) xhi * 51 + (mlo >> 32);

	return (uint72_t) {
		.hi = mhi >> 32,
		.lo = ((uint32_t) mlo) + (mhi << 32)
	};
}

/* Add two 72-bit numbers */
static uint72_t plus72(uint72_t a, uint64_t b)
{
	uint64_t lo = a.lo + b;
	uint8_t hi = a.hi;
	if (lo < b)
		hi++;
	return (uint72_t) { .lo = lo, .hi = hi };
}

/* Shift a 72-bit number by more than 8 */
static uint64_t shift72to64(uint72_t a, int amt)
{
	return (((uint64_t) a.hi) << (64 - amt)) | (amt == 64 ? 0 : (a.lo >> amt));
}

/* Shift a 72-bit number an arbitrary amount */
static uint72_t shift72(uint72_t a, int amt)
{
	return (uint72_t) {
		.hi = a.hi >> amt,
		.lo = shift72to64(a, amt)
			};
}

/* Compute t/10 and t % 10 simultaneously */
static divmod_t divmod10(uint64_t t)
{
	/*
	 * We're computing (t + 1) * 256 / 10 by doing:
	 *
	 * q = (t + 1) * 51 * 256/255 * 1/2
	 *   = (t + 1) * 256 * 51/255 * 1/2
	 *   = (t + 1) * 256 * 2/10 * 1/2
	 *   = (t + 1) * 256 * 1/10
	 *   = (t + 1) * 256/10
	 *
	 * This leaves q/10 in the upper 64 bits and
	 * (q % 10) * 256 / 10 in the lower 8 bits
	 */

	/* (t+1) * 51 */
	uint72_t	q = mul51(t);

	/*
	 * Now recognize that 256/255 is a repeating binary
	 * fraction:
	 * 256 / 255 = 0x1.010101010101...
	 *
	 * q = q * 256 / 255
	 *   = q + (q >> 8) + (q >> 16) + (q >> 24) + (q >> 32) + (q >> 40)
	 *     + (q >> 48) + (q >> 56) + (q >> 64)
	 *
	 * We can shorten that a bit by stirring things around
	 *
	 *	q1 = q0 * (1 + 2**-8)
	 *
	 *	q2 = q1 * (1 + 2**-16)
	 *         = q0 * (1 + 2**-8) * (1 + 2**-16)
	 *	   = q0 * (1 + 2**-8 + 2**-16 + 2**-24)
	 *
	 *	q3 = q2 * (1 + 2**-32)
	 *	   = q0 * (1 + 2**-8 + 2**-16 + 2**-24) * (1 + 2**-32)
	 *	   = q0 * (1 + 2**-8 + 2**-16 + 2**-24 + 2**-32 + 2**-40 + 2**-48 + 2**-56)
	 *
	 *	q4 = q3 * (1 + 2**-64)
	 *	   = q0 * (1 + 2**-8 + 2**-16 + 2**-24 + 2**-32 + 2**-40 + 2**-48 + 2**-56) * (1 + 2**-64)
	 *	   = q0 * (1 + 2**-8 + 2**-16 + 2**-24 + 2**-32 + 2**-40 + 2**-48 + 2**-56 + 2**-64 + (terms that will be zero))
	 */
	q = plus72(q, shift72to64(q, 8));
	q = plus72(q, shift72to64(q, 16));
	q = plus72(q, shift72to64(q, 32));
	q = plus72(q, shift72to64(q, 64));

	/* q = q * 1/2 */
	q = shift72(q, 1);

	divmod_t	a = {
		.div = shift72to64(q, 8),
		.mod = (((uint16_t) ((uint8_t) q.lo)) * 10) >> 8,
	};
	return a;
}

static divmod_t divmodbase(uint64_t val, int base)
{
	switch (base) {
	default:
		return divmod10(val);
	case 8:
		return (divmod_t) { .mod = val & 7, .div = val >> 3 };
	case 16:
		return (divmod_t) { .mod = val & 0xf, .div = val >> 4 };
	}
}

#else
static divmod_t divmodbase(ultoa_unsigned_t val, int base)
{
	return (divmod_t) { .mod = val % base, .div = val / base };
}
#endif

static __noinline char *
__ultoa_invert(ultoa_unsigned_t val, char *str, int base)
{
	char hex = 'a' - '0' - 10;

	if (base & XTOA_UPPER) {
		hex = 'A' - '0' - 10;
		base &= ~XTOA_UPPER;
	}
	do {
		char	v;

		divmod_t d = divmodbase(val, base);
		v = d.mod;
		val = d.div;

		if (v > 9)
                        v += hex;
                v += '0';
		*str++ = v;
	} while (val);
	return str;
}
