#if _IEEE_754_2008_SNAN
{ .x = REAL(FN32(0x0p-2), FN64(0x0p-2), FN80(0x0p-2), FN80M(0x0p-2), FN128(0x0p-2)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(0.0)),  },
#else
{ .x = REAL(FN32(0x0p-2), FN64(0x0p-2), FN80(0x0p-2), FN80M(0x0p-2), FN128(0x0p-2)),  .y = REAL(FN32(__builtin_nans)("0"), FN64(__builtin_nans)("0"), FN80(__builtin_nans)("0"), FN80M(__builtin_nans)("0"), FN128(__builtin_nans)("0")),  },
#endif
{ .x = REAL(FN32(0x1p0), FN64(0x1p0), FN80(0x1p0), FN80M(0x1p0), FN128(0x1p0)),  .y = REAL(FN32(__builtin_nans)("1"), FN64(__builtin_nans)("1"), FN80(__builtin_nans)("1"), FN80M(__builtin_nans)("1"), FN128(__builtin_nans)("1")),  },
{ .x = REAL(FN32(0x1.9p6), FN64(0x1.9p6), FN80(0x1.9p6), FN80M(0x1.9p6), FN128(0x1.9p6)),  .y = REAL(FN32(__builtin_nans)("100"), FN64(__builtin_nans)("100"), FN80(__builtin_nans)("100"), FN80M(__builtin_nans)("100"), FN128(__builtin_nans)("100")),  },
{ .x = REAL(FN32(-0x1p0), FN64(-0x1p0), FN80(-0x1p0), FN80M(-0x1p0), FN128(-0x1p0)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(0.0)),  },
{ .x = REAL(FN32(0x1.fffffcp21), FN64(0x1.fffffcp21), FN80(0x1.fffffcp21), FN80M(0x1.fffffcp21), FN128(0x1.fffffcp21)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(0.0)),  },
{ .x = REAL(FN32(0x1.fffff8p21), FN64(0x1.fffff8p21), FN80(0x1.fffff8p21), FN80M(0x1.fffff8p21), FN128(0x1.fffff8p21)),  .y = REAL(FN32(__builtin_nans)("4194303"), FN64(__builtin_nans)("4194303"), FN80(__builtin_nans)("4194303"), FN80M(__builtin_nans)("4194303"), FN128(__builtin_nans)("4194303")),  },
{ .x = REAL(FN32(0x1.fffffcp22), FN64(0x1.fffffcp22), FN80(0x1.fffffcp22), FN80M(0x1.fffffcp22), FN128(0x1.fffffcp22)),  .y = REAL(FN32(0.0), FN64(__builtin_nans)("8388607"), FN80(__builtin_nans)("8388607"), FN80M(__builtin_nans)("8388607"), FN128(__builtin_nans)("8388607")),  },
{ .x = REAL(FN32(0x1.ffffffffffffep50), FN64(0x1.ffffffffffffep50), FN80(0x1.ffffffffffffep50), FN80M(0x1.ffffffffffffep50), FN128(0x1.ffffffffffffep50)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(0.0)),  },
{ .x = REAL(FN32(0x1.ffffffffffffcp50), FN64(0x1.ffffffffffffcp50), FN80(0x1.ffffffffffffcp50), FN80M(0x1.ffffffffffffcp50), FN128(0x1.ffffffffffffcp50)),  .y = REAL(FN32(0.0), FN64(__builtin_nans)("2251799813685247"), FN80(__builtin_nans)("2251799813685247"), FN80M(__builtin_nans)("2251799813685247"), FN128(__builtin_nans)("2251799813685247")),  },
{ .x = REAL(FN32(0x1.ffffffffffffep51), FN64(0x1.ffffffffffffep51), FN80(0x1.ffffffffffffep51), FN80M(0x1.ffffffffffffep51), FN128(0x1.ffffffffffffep51)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(__builtin_nans)("4503599627370495"), FN80M(__builtin_nans)("4503599627370495"), FN128(__builtin_nans)("4503599627370495")),  },
{ .x = REAL(FN32(0x1.fffffffffffffffcp61), FN64(0x1.fffffffffffffffcp61), FN80(0x1.fffffffffffffffcp61), FN80M(0x1.fffffffffffffffcp61), FN128(0x1.fffffffffffffffcp61)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(0.0)),  },
{ .x = REAL(FN32(0x1.fffffffffffffff8p61), FN64(0x1.fffffffffffffff8p61), FN80(0x1.fffffffffffffff8p61), FN80M(0x1.fffffffffffffff8p61), FN128(0x1.fffffffffffffff8p61)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(__builtin_nans)("4611686018427387903"), FN80M(__builtin_nans)("4611686018427387903"), FN128(__builtin_nans)("4611686018427387903")),  },
{ .x = REAL(FN32(0x1.fffffffffffffffcp62), FN64(0x1.fffffffffffffffcp62), FN80(0x1.fffffffffffffffcp62), FN80M(0x1.fffffffffffffffcp62), FN128(0x1.fffffffffffffffcp62)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(__builtin_nans)("9223372036854775807")),  },
{ .x = REAL(FN32(0x1.fffffffffffffffffffffffffffep110), FN64(0x1.fffffffffffffffffffffffffffep110), FN80(0x1.fffffffffffffffffffffffffffep110), FN80M(0x1.fffffffffffffffffffffffffffep110), FN128(0x1.fffffffffffffffffffffffffffep110)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(0.0)),  },
{ .x = REAL(FN32(0x1.fffffffffffffffffffffffffffcp110), FN64(0x1.fffffffffffffffffffffffffffcp110), FN80(0x1.fffffffffffffffffffffffffffcp110), FN80M(0x1.fffffffffffffffffffffffffffcp110), FN128(0x1.fffffffffffffffffffffffffffcp110)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(__builtin_nans)("2596148429267413814265248164610047")),  },
{ .x = REAL(FN32(0x1.fffffffffffffffffffffffffffep111), FN64(0x1.fffffffffffffffffffffffffffep111), FN80(0x1.fffffffffffffffffffffffffffep111), FN80M(0x1.fffffffffffffffffffffffffffep111), FN128(0x1.fffffffffffffffffffffffffffep111)),  .y = REAL(FN32(0.0), FN64(0.0), FN80(0.0), FN80M(0.0), FN128(0.0)),  },
/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
