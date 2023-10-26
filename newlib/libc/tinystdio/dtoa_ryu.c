// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

// Runtime compiler options:
// -DRYU_DEBUG Generate verbose debugging output to stdout.
//
// -DRYU_ONLY_64_BIT_OPS Avoid using uint128_t or 64-bit intrinsics. Slower,
//     depending on your compiler.
//

#include "stdio_private.h"

#ifdef FLOAT64

#define _NEED_IO_FLOAT64

#include "dtoa.h"

#include "ryu/ryu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef RYU_DEBUG
#include <inttypes.h>
#include <stdio.h>
#endif

#include "ryu/common.h"
#include "ryu/d2s_intrinsics.h"

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_BIAS 1023

static int decimalLength17(const uint64_t v) {
	int len = 1;
	uint64_t c = 10;
	while (c <= v) {
		len++;
		c = (c << 3) + (c << 1);
	}
	return len;
}

// A floating decimal representing m * 10^e.
typedef struct floating_decimal_64 {
	uint64_t mantissa;
	// Decimal exponent's range is -324 to 308
	// inclusive, and can fit in a short if needed.
	int16_t exponent;
	int16_t olength;
} floating_decimal_64;

static inline floating_decimal_64
d2d(const uint64_t ieeeMantissa, const uint32_t ieeeExponent, int max_digits, bool fmode, int max_decimals)
{
	int32_t e2;
	uint64_t m2;
	if (ieeeExponent == 0) {
		// We subtract 2 so that the bounds computation has 2 additional bits.
		e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
		m2 = ieeeMantissa;
	} else {
		e2 = (int32_t) ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
		m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
	}
	const bool even = (m2 & 1) == 0;
	const bool acceptBounds = even;
	/* Set if we're truncating more digits to meet format request */
	bool truncate_max = false;

#ifdef RYU_DEBUG
	printf("-> %" PRIu64 " * 2^%d\n", m2, e2 + 2);
#endif

	// Step 2: Determine the interval of valid decimal representations.
	const uint64_t mv = 4 * m2;
	// Implicit bool -> int conversion. True is 1, false is 0.
	const uint32_t mmShift = ieeeMantissa != 0 || ieeeExponent <= 1;
	// We would compute mp and mm like this:
	// uint64_t mp = 4 * m2 + 2;
	// uint64_t mm = mv - 1 - mmShift;

	// Step 3: Convert to a decimal power base using 128-bit arithmetic.
	uint64_t vr, vp, vm;
	int32_t e10;
	bool vmIsTrailingZeros = false;
	bool vrIsTrailingZeros = false;
	if (e2 >= 0) {
		// I tried special-casing q == 0, but there was no effect on performance.
		// This expression is slightly faster than max_int(0, log10Pow2(e2) - 1).
		const uint32_t q = log10Pow2(e2) - (e2 > 3);
		e10 = (int32_t) q;
		const int32_t k = DOUBLE_POW5_INV_BITCOUNT + pow5bits((int32_t) q) - 1;
		const int32_t i = -e2 + (int32_t) q + k;
		uint64_t pow5[2];
		__double_computeInvPow5(q, pow5);
		vr = mulShiftAll64(m2, pow5, i, &vp, &vm, mmShift);
#ifdef RYU_DEBUG
		printf("%" PRIu64 " * 2^%d / 10^%u\n", mv, e2, q);
		printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
		if (q <= 21) {
			// This should use q <= 22, but I think 21 is also safe. Smaller values
			// may still be safe, but it's more difficult to reason about them.
			// Only one of mp, mv, and mm can be a multiple of 5, if any.
			const uint32_t mvMod5 = ((uint32_t) mv) - 5 * ((uint32_t) div5(mv));
			if (mvMod5 == 0) {
				vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
			} else if (acceptBounds) {
				// Same as min_int(e2 + (~mm & 1), pow5Factor(mm)) >= q
				// <=> e2 + (~mm & 1) >= q && pow5Factor(mm) >= q
				// <=> true && pow5Factor(mm) >= q, since e2 >= q.
				vmIsTrailingZeros = multipleOfPowerOf5(mv - 1 - mmShift, q);
			} else {
				// Same as min_int(e2 + 1, pow5Factor(mp)) >= q.
				vp -= multipleOfPowerOf5(mv + 2, q);
			}
		}
	} else {
		// This expression is slightly faster than max_int(0, log10Pow5(-e2) - 1).
		const uint32_t q = log10Pow5(-e2) - (-e2 > 1);
		e10 = (int32_t) q + e2;
		const int32_t i = -e2 - (int32_t) q;
		const int32_t k = pow5bits(i) - DOUBLE_POW5_BITCOUNT;
		const int32_t j = (int32_t) q - k;
		uint64_t pow5[2];
		__double_computePow5(i, pow5);
		vr = mulShiftAll64(m2, pow5, j, &vp, &vm, mmShift);

#ifdef RYU_DEBUG
		printf("%" PRIu64 " * 5^%d / 10^%u\n", mv, -e2, q);
		printf("%u %d %d %d\n", q, i, k, j);
		printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
		if (q <= 1) {
			// {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
			// mv = 4 * m2, so it always has at least two trailing 0 bits.
			vrIsTrailingZeros = true;
			if (acceptBounds) {
				// mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
				vmIsTrailingZeros = mmShift == 1;
			} else {
				// mp = mv + 2, so it always has at least one trailing 0 bit.
				--vp;
			}
		} else if (q < 63) { // TODO(ulfjack): Use a tighter bound here.
			// We want to know if the full product has at least q trailing zeros.
			// We need to compute min_int(p2(mv), p5(mv) - e2) >= q
			// <=> p2(mv) >= q && p5(mv) - e2 >= q
			// <=> p2(mv) >= q (because -e2 >= q)
			vrIsTrailingZeros = multipleOfPowerOf2(mv, q);
#ifdef RYU_DEBUG
			printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
		}
	}
#ifdef RYU_DEBUG
	printf("e10=%d\n", e10);
	printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
	printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
	printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

	// Step 4: Find the shortest decimal representation in the interval of valid representations.
	int32_t removed = 0;
	uint8_t lastRemovedDigit = 0;
	uint64_t output;
	// On average, we remove ~2 digits.
	// General case, which happens rarely (~0.7%).

	/* If limiting decimals, then limit the max digits
	 * to no more than the number of digits left of the decimal
	 * plus the number of digits right of the decimal
	 *
	 * exp:          exponent value. If negative, there are
	 *		 -exp - 1 zeros left of the first non-zero
	 *               digit in 'f' format. If non-negative,
	 *               there are exp digits to the left of
	 *               the decimal point
	 *
	 * max_decimals: Only used in 'f' format. Round to this many
	 *               digits to the right of the decimal point
	 *               (left if negative)
	 *
	 * max_digits:	 We can't convert more than this number of digits given
	 *               the limits of the buffer
	 */

	int save_max_digits = max_digits;
	if(fmode) {
		int exp = e10 + decimalLength17(vr) - 1;
		/*
		 * This covers two cases:
		 *
		 * When exp is < 0, there are -exp-1 zeros taking up
		 * space before we can display any of the real digits,
		 * so we have to subtract those off max_decimals before
		 * we round that (max_decimals - (-exp - 1)). This
		 * may end up less than zero, in which case we have
		 * no digits to display.
		 *
		 * When exp >= 0, there are exp + 1 digits left of the
		 * decimal point *plus* max_decimals right of the
		 * decimal point that need to be generated
		 *
		 * A single expression gives the right answer in both
		 * cases, which is kinda cool
		 */
		max_digits = min_int(max_digits, max_int(1, max_decimals + exp + 1));
	}

	for (;;) {
		const uint64_t vpDiv10 = div10(vp);
		const uint64_t vmDiv10 = div10(vm);
		if (vpDiv10 <= vmDiv10) {
			if (decimalLength17(vr) <= max_digits || (max_digits == 0 && vr == 0))
				break;
			else
				truncate_max = true;
		}
		const uint32_t vmMod10 = ((uint32_t) vm) - 10 * ((uint32_t) vmDiv10);
		const uint64_t vrDiv10 = div10(vr);
		const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
		vmIsTrailingZeros &= vmMod10 == 0;
		vrIsTrailingZeros &= lastRemovedDigit == 0;
		lastRemovedDigit = (uint8_t) vrMod10;
		vr = vrDiv10;
		vp = vpDiv10;
		vm = vmDiv10;
		++removed;
	}
#ifdef RYU_DEBUG
	printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
	printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
	if (vmIsTrailingZeros) {
		for (;;) {
			const uint64_t vmDiv10 = div10(vm);
			const uint32_t vmMod10 = ((uint32_t) vm) - 10 * ((uint32_t) vmDiv10);
			if (vmMod10 != 0) {
				break;
			}
			const uint64_t vpDiv10 = div10(vp);
			const uint64_t vrDiv10 = div10(vr);
			const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
			vrIsTrailingZeros &= lastRemovedDigit == 0;
			lastRemovedDigit = (uint8_t) vrMod10;
			vr = vrDiv10;
			vp = vpDiv10;
			vm = vmDiv10;
			++removed;
		}
	}
#ifdef RYU_DEBUG
	printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
	printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
	if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
		// Round even if the exact number is .....50..0.
		lastRemovedDigit = 4;
	}

	output = vr;
	e10 += removed;

	// We need to take vr + 1 if vr is outside bounds or we need to round up.
	// I don't know if the 'truncate_max' case is entirely correct; need some tests
	uint8_t carry = ((!truncate_max && vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
	output += carry;

	int len = decimalLength17(output);

	if (carry) {
		/* This can only happen if output has carried out of the top digit */
		if (len > max_digits) {

			/* Recompute max digits in this case */
                        if(fmode) {
				int exp = e10 + len - 1;
				/* max_decimals comes in biased by 1 to flag the 'f' case */
				max_digits = min_int(save_max_digits, max_int(0, max_decimals + exp + 1));
			}

			if (len > max_digits) {
				output += 5;
				output /= 10;
				e10++;
				len--;
			}
		}
	}
	if (len > max_digits)
		len = max_digits;

#ifdef RYU_DEBUG
	printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
	printf("O=%" PRIu64 "\n", output);
	printf("EXP=%d\n", exp);
#endif

	floating_decimal_64 fd;
	fd.exponent = e10;
	fd.olength = len;
	fd.mantissa = output;
	return fd;
}

static inline bool d2d_small_int(const uint64_t ieeeMantissa, const uint32_t ieeeExponent,
				 floating_decimal_64* const v) {
	const uint64_t m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
	const int32_t e2 = (int32_t) ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;

	if (e2 > 0) {
		// f = m2 * 2^e2 >= 2^53 is an integer.
		// Ignore this case for now.
		return false;
	}

	if (e2 < -52) {
		// f < 1.
		return false;
	}

	// Since 2^52 <= m2 < 2^53 and 0 <= -e2 <= 52: 1 <= f = m2 / 2^-e2 < 2^53.
	// Test if the lower -e2 bits of the significand are 0, i.e. whether the fraction is 0.
	const uint64_t mask = (1ull << -e2) - 1;
	const uint64_t fraction = m2 & mask;
	if (fraction != 0) {
		return false;
	}

	// f is an integer in the range [1, 2^53).
	// Note: mantissa might contain trailing (decimal) 0's.
	// Note: since 2^53 < 10^16, there is no need to adjust decimalLength17().
	v->mantissa = m2 >> -e2;
	v->exponent = 0;
	return true;
}

int
__dtoa_engine(FLOAT64 x, struct dtoa *dtoa, int max_digits, bool fmode, int max_decimals)
{
	// Step 1: Decode the floating-point number, and unify normalized and subnormal cases.
	const uint64_t bits = ryu64_to_bits(x);

#ifdef RYU_DEBUG
	printf("IN=");
	for (int32_t bit = 63; bit >= 0; --bit) {
		printf("%d", (int) ((bits >> bit) & 1));
	}
	printf("\n");
#endif

	// Decode bits into sign, mantissa, and exponent.
	const bool ieeeSign = ((bits >> (DOUBLE_MANTISSA_BITS + DOUBLE_EXPONENT_BITS)) & 1) != 0;
	const uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
	const uint32_t ieeeExponent = (uint32_t) ((bits >> DOUBLE_MANTISSA_BITS) & ((1u << DOUBLE_EXPONENT_BITS) - 1));

	uint8_t	flags = 0;

	if (ieeeSign)
		flags |= DTOA_MINUS;
	if (ieeeExponent == 0 && ieeeMantissa == 0) {
		flags |= DTOA_ZERO;
		dtoa->digits[0] = '0';
		dtoa->flags = flags;
		dtoa->exp = 0;
		return 1;
	}
	if (ieeeExponent == ((1u << DOUBLE_EXPONENT_BITS) - 1u)) {
		if (ieeeMantissa) {
			flags |= DTOA_NAN;
		} else {
			flags |= DTOA_INF;
		}
		dtoa->flags = flags;
		return 0;
	}

	floating_decimal_64 v;
	const bool isSmallInt = d2d_small_int(ieeeMantissa, ieeeExponent, &v);
	if (isSmallInt && 0) {
		// For small integers in the range [1, 2^53), v.mantissa might contain trailing (decimal) zeros.
		// For scientific notation we need to move these zeros into the exponent.
		// (This is not needed for fixed-point notation, so it might be beneficial to trim
		// trailing zeros in to_chars only if needed - once fixed-point notation output is implemented.)
		for (;;) {
			const uint64_t q = div10(v.mantissa);
			const uint32_t r = ((uint32_t) v.mantissa) - 10 * ((uint32_t) q);
			if (r != 0) {
				break;
			}
			v.mantissa = q;
			++v.exponent;
		}
	} else {
		v = d2d(ieeeMantissa, ieeeExponent, max_digits, fmode, max_decimals);
	}

	uint64_t mant = v.mantissa;
	int32_t olength = v.olength;
	int32_t exp = v.exponent + olength - 1;

	int i;

	for (i = 0; i < olength; i++) {
		dtoa->digits[olength - i - 1] = (mant % 10) + '0';
		mant /= 10;
	}

	dtoa->exp = exp;
	dtoa->flags = flags;
	return olength;
}

#endif /* FLOAT64 */
