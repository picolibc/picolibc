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

#include "ryu/f2s_intrinsics.h"

// It seems to be slightly faster to avoid uint128_t here, although the
// generated code for uint128_t looks slightly nicer.
static inline uint32_t mulShift32(const uint32_t m, const uint64_t factor, const int32_t shift) {
  assert(shift > 32);

  // The casts here help MSVC to avoid calls to the __allmul library
  // function.
  const uint32_t factorLo = (uint32_t)(factor);
  const uint32_t factorHi = (uint32_t)(factor >> 32);
  const uint64_t bits0 = (uint64_t)m * factorLo;
  const uint64_t bits1 = (uint64_t)m * factorHi;

  /*
   * This function *does* get called with shift < 32 when converting
   * strings to floats, so disable this optimization
   */
#if defined(RYU_32_BIT_PLATFORM)
  // On 32-bit platforms we can avoid a 64-bit shift-right since we only
  // need the upper 32 bits of the result and the shift value is > 32.
  const uint32_t bits0Hi = (uint32_t)(bits0 >> 32);
  uint32_t bits1Lo = (uint32_t)(bits1);
  uint32_t bits1Hi = (uint32_t)(bits1 >> 32);
  bits1Lo += bits0Hi;
  bits1Hi += (bits1Lo < bits0Hi);
  if (shift >= 64) {
    // s2f can call this with a shift value >= 64, which we have to handle.
    // This could now be slower than the !defined(RYU_32_BIT_PLATFORM) case.
    return (uint32_t)(bits1Hi >> (shift - 64));
  } else {
    const int32_t s = shift - 32;
    return (bits1Hi << (32 - s)) | (bits1Lo >> s);
  }
#else // RYU_32_BIT_PLATFORM
  const uint64_t sum = (bits0 >> 32) + bits1;
  const uint64_t shiftedSum = sum >> (shift - 32);
  assert(shiftedSum <= UINT32_MAX);
  return (uint32_t) shiftedSum;
#endif // RYU_32_BIT_PLATFORM
}

uint32_t __mulPow5InvDivPow2(const uint32_t m, const uint32_t q, const int32_t j) {
  // The inverse multipliers are defined as [2^x / 5^y] + 1; the upper 64 bits from the double lookup
  // table are the correct bits for [2^x / 5^y], so we have to add 1 here. Note that we rely on the
  // fact that the added 1 that's already stored in the table never overflows into the upper 64 bits.
  uint64_t pow5[2];
  __double_computeInvPow5(q, pow5);
  return mulShift32(m, pow5[1] + 1, j);
}

uint32_t __mulPow5divPow2(const uint32_t m, const uint32_t i, const int32_t j) {
  uint64_t pow5[2];
  __double_computePow5(i, pow5);
  return mulShift32(m, pow5[1], j);
}

