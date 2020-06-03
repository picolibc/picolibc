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
#ifndef RYU_F2S_INTRINSICS_H
#define RYU_F2S_INTRINSICS_H

// Defines RYU_32_BIT_PLATFORM if applicable.
#include "ryu/common.h"

#define FLOAT_POW5_INV_BITCOUNT (DOUBLE_POW5_INV_BITCOUNT - 64)
#define FLOAT_POW5_BITCOUNT (DOUBLE_POW5_BITCOUNT - 64)

static inline uint32_t pow5factor_32(uint32_t value) {
  uint32_t count = 0;
  for (;;) {
    assert(value != 0);
    const uint32_t q = value / 5;
    const uint32_t r = value % 5;
    if (r != 0) {
      break;
    }
    value = q;
    ++count;
  }
  return count;
}

// Returns true if value is divisible by 5^p.
static inline bool multipleOfPowerOf5_32(const uint32_t value, const uint32_t p) {
  return pow5factor_32(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline bool multipleOfPowerOf2_32(const uint32_t value, const uint32_t p) {
  // __builtin_ctz doesn't appear to be faster here.
  return (value & ((1u << p) - 1)) == 0;
}

uint32_t __mulPow5InvDivPow2(const uint32_t m, const uint32_t q, const int32_t j);
#define mulPow5InvDivPow2(m,q,j) __mulPow5InvDivPow2(m,q,j)

uint32_t __mulPow5divPow2(const uint32_t m, const uint32_t i, const int32_t j);
#define mulPow5divPow2(m,i,j) __mulPow5divPow2(m,i,j)

#endif // RYU_F2S_INTRINSICS_H
