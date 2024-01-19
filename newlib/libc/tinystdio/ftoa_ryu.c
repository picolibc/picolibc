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

#define _NEED_IO_FLOAT32

#include "dtoa.h"
#include "ryu/ryu.h"

#include "ryu/common.h"
#include "ryu/f2s_intrinsics.h"
#include "ryu/digit_table.h"

#define FLOAT_MANTISSA_BITS 23
#define FLOAT_EXPONENT_BITS 8
#define FLOAT_BIAS 127

// Returns the number of decimal digits in v, which must not contain more than 9 digits.
static int decimalLength9(const uint32_t v) {
	int len = 1;
	uint32_t c = 10;
	while (c <= v) {
		len++;
		c = (c << 3) + (c << 1);
	}
	return len;
}

// A floating decimal representing m * 10^e.
typedef struct floating_decimal_32 {
	uint32_t mantissa;
	// Decimal exponent's range is -45 to 38
	// inclusive, and can fit in a short if needed.
	int16_t exponent;
	int16_t olength;
} floating_decimal_32;

static inline floating_decimal_32
f2d(const uint32_t ieeeMantissa, const uint32_t ieeeExponent, int max_digits, bool fmode, int max_decimals)
{
	int32_t e2;
	uint32_t m2;
	if (ieeeExponent == 0) {
		// We subtract 2 so that the bounds computation has 2 additional bits.
		e2 = 1 - FLOAT_BIAS - FLOAT_MANTISSA_BITS - 2;
		m2 = ieeeMantissa;
	} else {
		e2 = (int32_t) ieeeExponent - FLOAT_BIAS - FLOAT_MANTISSA_BITS - 2;
		m2 = ((uint32_t)1u << FLOAT_MANTISSA_BITS) | ieeeMantissa;
	}
	const bool even = (m2 & 1) == 0;
	const bool acceptBounds = even;
	bool truncate_max = false;

#ifdef RYU_DEBUG
	printf("-> %u * 2^%d\n", m2, e2 + 2);
#endif

	// Step 2: Determine the interval of valid decimal representations.
	const uint32_t mv = 4 * m2;
	const uint32_t mp = 4 * m2 + 2;
	// Implicit bool -> int conversion. True is 1, false is 0.
	const uint32_t mmShift = ieeeMantissa != 0 || ieeeExponent <= 1;
	const uint32_t mm = 4 * m2 - 1 - mmShift;

	// Step 3: Convert to a decimal power base using 64-bit arithmetic.
	uint32_t vr, vp, vm;
	int32_t e10;
	bool vmIsTrailingZeros = false;
	bool vrIsTrailingZeros = false;
	uint8_t lastRemovedDigit = 0;
	if (e2 >= 0) {
		const uint32_t q = log10Pow2(e2);
		e10 = (int32_t) q;
		const int32_t k = FLOAT_POW5_INV_BITCOUNT + pow5bits((int32_t) q) - 1;
		const int32_t i = -e2 + (int32_t) q + k;
		vr = mulPow5InvDivPow2(mv, q, i);
		vp = mulPow5InvDivPow2(mp, q, i);
		vm = mulPow5InvDivPow2(mm, q, i);
#ifdef RYU_DEBUG
		printf("%u * 2^%d / 10^%u\n", mv, e2, q);
		printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
#endif
		if (q != 0 && (vp - 1) / 10 <= vm / 10) {
			// We need to know one removed digit even if we are not going to loop below. We could use
			// q = X - 1 above, except that would require 33 bits for the result, and we've found that
			// 32-bit arithmetic is faster even on 64-bit machines.
			const int32_t l = FLOAT_POW5_INV_BITCOUNT + pow5bits((int32_t) (q - 1)) - 1;
			lastRemovedDigit = (uint8_t) (mulPow5InvDivPow2(mv, q - 1, -e2 + (int32_t) q - 1 + l) % 10);
		}
		if (q <= 9) {
			// The largest power of 5 that fits in 24 bits is 5^10, but q <= 9 seems to be safe as well.
			// Only one of mp, mv, and mm can be a multiple of 5, if any.
			if (mv % 5 == 0) {
				vrIsTrailingZeros = multipleOfPowerOf5_32(mv, q);
			} else if (acceptBounds) {
				vmIsTrailingZeros = multipleOfPowerOf5_32(mm, q);
			} else {
				vp -= multipleOfPowerOf5_32(mp, q);
			}
		}
	} else {
		const uint32_t q = log10Pow5(-e2);
		e10 = (int32_t) q + e2;
		const int32_t i = -e2 - (int32_t) q;
		const int32_t k = pow5bits(i) - FLOAT_POW5_BITCOUNT;
		int32_t j = (int32_t) q - k;
		vr = mulPow5divPow2(mv, (uint32_t) i, j);
		vp = mulPow5divPow2(mp, (uint32_t) i, j);
		vm = mulPow5divPow2(mm, (uint32_t) i, j);
#ifdef RYU_DEBUG
		printf("%u * 5^%d / 10^%u\n", mv, -e2, q);
		printf("%u %d %d %d\n", q, i, k, j);
		printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
#endif
		if (q != 0 && (vp - 1) / 10 <= vm / 10) {
			j = (int32_t) q - 1 - (pow5bits(i + 1) - FLOAT_POW5_BITCOUNT);
			lastRemovedDigit = (uint8_t) (mulPow5divPow2(mv, (uint32_t) (i + 1), j) % 10);
		}
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
		} else if (q < 31) { // TODO(ulfjack): Use a tighter bound here.
			vrIsTrailingZeros = multipleOfPowerOf2_32(mv, q - 1);
#ifdef RYU_DEBUG
			printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
		}
	}
#ifdef RYU_DEBUG
	printf("e10=%d\n", e10);
	printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
	printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
	printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

	// Step 4: Find the shortest decimal representation in the interval of valid representations.
	int32_t removed = 0;
	uint32_t output;

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
		int exp = e10 + decimalLength9(vr) - 1;
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
		if (vp / 10 <= vm / 10) {
			if (decimalLength9(vr) <= max_digits || (max_digits == 0 && vr == 0))
				break;
			else
				truncate_max = true;
		}
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=23106
		// The compiler does not realize that vm % 10 can be computed from vm / 10
		// as vm - (vm / 10) * 10.
		vmIsTrailingZeros &= vm - (vm / 10) * 10 == 0;
#else
		vmIsTrailingZeros &= vm % 10 == 0;
#endif
		vrIsTrailingZeros &= lastRemovedDigit == 0;
		lastRemovedDigit = (uint8_t) (vr % 10);
		vr /= 10;
		vp /= 10;
		vm /= 10;
		++removed;
	}
#ifdef RYU_DEBUG
	printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
	printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
	if (vmIsTrailingZeros) {
		while (vm % 10 == 0) {
			vrIsTrailingZeros &= lastRemovedDigit == 0;
			lastRemovedDigit = (uint8_t) (vr % 10);
			vr /= 10;
			vp /= 10;
			vm /= 10;
			++removed;
		}
	}
#ifdef RYU_DEBUG
	printf("%u %d\n", vr, lastRemovedDigit);
	printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
	if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
		// Round even if the exact number is .....50..0.
		lastRemovedDigit = 4;
	}
	// We need to take vr + 1 if vr is outside bounds or we need to round up.
	output = vr;
	e10 += removed;

	uint8_t carry = ((!truncate_max && vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
	output += carry;

	int len = decimalLength9(output);

	if (carry) {
		/* This can only happen if output has carried out of the top digit */
		if (len > max_digits) {

			/* Recompute max digits in this case */
                        if(fmode) {
				int exp = e10 + len - 1;
				max_digits = min_int(save_max_digits, max_int(1, max_decimals + exp + 1));
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
	printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
	printf("O=%u\n", output);
	printf("EXP=%d\n", exp);
#endif

	floating_decimal_32 fd;
	fd.exponent = e10;
	fd.olength = len;
	fd.mantissa = output;
	return fd;
}

int
__ftoa_engine(float x, struct dtoa *dtoa, int max_digits, bool fmode, int max_decimals)
{
	// Step 1: Decode the floating-point number, and unify normalized and subnormal cases.
	const uint32_t bits = float_to_bits(x);

	// Decode bits into sign, mantissa, and exponent.
	const bool ieeeSign = ((bits >> (FLOAT_MANTISSA_BITS + FLOAT_EXPONENT_BITS)) & 1) != 0;
	const uint64_t ieeeMantissa = bits & ((1ull << FLOAT_MANTISSA_BITS) - 1);
	const uint32_t ieeeExponent = (uint32_t) ((bits >> FLOAT_MANTISSA_BITS) & ((1u << FLOAT_EXPONENT_BITS) - 1));

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
	if (ieeeExponent == ((1u << FLOAT_EXPONENT_BITS) - 1u)) {
		if (ieeeMantissa) {
			flags |= DTOA_NAN;
		} else {
			flags |= DTOA_INF;
		}
		dtoa->flags = flags;
		return 0;
	}

	floating_decimal_32 v;

	v = f2d(ieeeMantissa, ieeeExponent, max_digits, fmode, max_decimals);

	uint32_t mant = v.mantissa;
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
