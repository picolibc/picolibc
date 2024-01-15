// Copyright 2019 Ulf Adams
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

#include "stdio_private.h"

#ifdef RYU_DEBUG
#include <inttypes.h>
#endif

#include "ryu/common.h"
#include "ryu/f2s_intrinsics.h"

#define FLOAT_MANTISSA_BITS 23
#define FLOAT_EXPONENT_BITS 8
#define FLOAT_EXPONENT_BIAS 127

#if defined(_MSC_VER)
#include <intrin.h>

static inline uint32_t floor_log2(const uint32_t value) {
  uint32_t index;
  return _BitScanReverse(&index, value) ? index : 32;
}

#else

static inline uint32_t floor_log2(const uint32_t value) {
#if __SIZEOF_INT__ >= 4
  return 31 - __builtin_clz(value);
#elif __SIZEOF_INT__ < 4 && __SIZEOF_LONG__ >= 4
  return 31 - __builtin_clzl(value);
#else
#error no usable clz
#endif
}

#endif

// The max function is already defined on Windows.
static inline int32_t max32(int32_t a, int32_t b) {
  return a < b ? b : a;
}

static inline float int32Bits2Float(uint32_t bits) {
  float f;
  memcpy(&f, &bits, sizeof(float));
  return f;
}

float
__atof_engine(uint32_t m10, int e10)
{
#ifdef RYU_DEBUG
	printf("m10digits = %ld\n", m10);
	printf("e10digits = %d\n", e10);
	printf("m10 * 10^e10 = %lu * 10^%d\n", m10, e10);
#endif

	// Convert to binary float m2 * 2^e2, while retaining information about whether the conversion
	// was exact (trailingZeros).
	int32_t e2;
	uint32_t m2;
	bool trailingZeros;
	if (e10 >= 0) {
		// The length of m * 10^e in bits is:
		//   log2(m10 * 10^e10) = log2(m10) + e10 log2(10) = log2(m10) + e10 + e10 * log2(5)
		//
		// We want to compute the FLOAT_MANTISSA_BITS + 1 top-most bits (+1 for the implicit leading
		// one in IEEE format). We therefore choose a binary output exponent of
		//   log2(m10 * 10^e10) - (FLOAT_MANTISSA_BITS + 1).
		//
		// We use floor(log2(5^e10)) so that we get at least this many bits; better to
		// have an additional bit than to not have enough bits.
		e2 = floor_log2(m10) + e10 + log2pow5(e10) - (FLOAT_MANTISSA_BITS + 1);

		// We now compute [m10 * 10^e10 / 2^e2] = [m10 * 5^e10 / 2^(e2-e10)].
		// To that end, we use the FLOAT_POW5_SPLIT table.
		int j = e2 - e10 - ceil_log2pow5(e10) + FLOAT_POW5_BITCOUNT;
		assert(j >= 0);
		m2 = mulPow5divPow2(m10, e10, j);

		// We also compute if the result is exact, i.e.,
		//   [m10 * 10^e10 / 2^e2] == m10 * 10^e10 / 2^e2.
		// This can only be the case if 2^e2 divides m10 * 10^e10, which in turn requires that the
		// largest power of 2 that divides m10 + e10 is greater than e2. If e2 is less than e10, then
		// the result must be exact. Otherwise we use the existing multipleOfPowerOf2 function.
		trailingZeros = e2 < e10 || (e2 - e10 < 32 && multipleOfPowerOf2_32(m10, e2 - e10));
	} else {
		e2 = floor_log2(m10) + e10 - ceil_log2pow5(-e10) - (FLOAT_MANTISSA_BITS + 1);

		// We now compute [m10 * 10^e10 / 2^e2] = [m10 / (5^(-e10) 2^(e2-e10))].
		int j = e2 - e10 + ceil_log2pow5(-e10) - 1 + FLOAT_POW5_INV_BITCOUNT;
		m2 = mulPow5InvDivPow2(m10, -e10, j);

		// We also compute if the result is exact, i.e.,
		//   [m10 / (5^(-e10) 2^(e2-e10))] == m10 / (5^(-e10) 2^(e2-e10))
		//
		// If e2-e10 >= 0, we need to check whether (5^(-e10) 2^(e2-e10)) divides m10, which is the
		// case iff pow5(m10) >= -e10 AND pow2(m10) >= e2-e10.
		//
		// If e2-e10 < 0, we have actually computed [m10 * 2^(e10 e2) / 5^(-e10)] above,
		// and we need to check whether 5^(-e10) divides (m10 * 2^(e10-e2)), which is the case iff
		// pow5(m10 * 2^(e10-e2)) = pow5(m10) >= -e10.
		trailingZeros = (e2 < e10 || (e2 - e10 < 32 && multipleOfPowerOf2_32(m10, e2 - e10)))
			&& multipleOfPowerOf5_32(m10, -e10);
	}

#ifdef RYU_DEBUG
	printf("m2 * 2^e2 = %lu * 2^%ld\n", m2, e2);
#endif

	// Compute the final IEEE exponent.
	uint32_t ieee_e2 = (uint32_t) max32(0, e2 + FLOAT_EXPONENT_BIAS + floor_log2(m2));

	if (ieee_e2 > 0xfe) {
		// Final IEEE exponent is larger than the maximum representable; return Infinity.
		uint32_t ieee = ((uint32_t)0xffu << FLOAT_MANTISSA_BITS);
		return int32Bits2Float(ieee);
	}

	// We need to figure out how much we need to shift m2. The tricky part is that we need to take
	// the final IEEE exponent into account, so we need to reverse the bias and also special-case
	// the value 0.
	int32_t shift = (ieee_e2 == 0 ? 1 : ieee_e2) - e2 - FLOAT_EXPONENT_BIAS - FLOAT_MANTISSA_BITS;
	assert(shift >= 0);
#ifdef RYU_DEBUG
	printf("ieee_e2 = %ld\n", ieee_e2);
	printf("shift = %ld\n", shift);
#endif

	// We need to round up if the exact value is more than 0.5 above the value we computed. That's
	// equivalent to checking if the last removed bit was 1 and either the value was not just
	// trailing zeros or the result would otherwise be odd.
	//
	// We need to update trailingZeros given that we have the exact output exponent ieee_e2 now.
	trailingZeros &= (m2 & ((1u << (shift - 1)) - 1)) == 0;
	uint32_t lastRemovedBit = (m2 >> (shift - 1)) & 1;
	bool roundUp = (lastRemovedBit != 0) && (!trailingZeros || (((m2 >> shift) & 1) != 0));

#ifdef RYU_DEBUG
	printf("roundUp = %d\n", roundUp);
	printf("ieee_m2 = %lu\n", (m2 >> shift) + roundUp);
#endif
	uint32_t ieee_m2 = (m2 >> shift) + roundUp;
        assert(ieee_m2 <= (1u << (FLOAT_MANTISSA_BITS + 1)));
        ieee_m2 &= ((uint32_t)1u << FLOAT_MANTISSA_BITS) - 1;
        if (ieee_m2 == 0 && roundUp) {
            // Rounding up may overflow the mantissa.
            // In this case we move a trailing zero of the mantissa into the exponent.
		// Due to how the IEEE represents +/-Infinity, we don't need to check for overflow here.
		ieee_e2++;
	}
	uint32_t ieee = (((uint32_t)ieee_e2) << FLOAT_MANTISSA_BITS) | ieee_m2;
	return int32Bits2Float(ieee);
}
