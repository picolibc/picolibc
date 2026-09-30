/* Copyright © 2017 Keith Packard
   Copyright © 2026 Artem Kulyk
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

/*
 * Divide-by-ten (and five) with remainder for the printf decimal digit
 * loops, shared by ultoa_invert.c and the floating-point conversions.
 *
 * udivmod10_64/32 approximate n/10 with a chain of shift/add steps and
 * correct the (at most one) low estimate; they need no multiplier and
 * no division.  They were originally written for ultoa_invert.c
 * (Keith Packard).
 *
 * The udivmod*m_64/div*m_64 forms serve the digit loops which divide
 * 64-bit quantities by ten (or five).  Where 64-bit division compiles
 * to a call to wide software division (__aeabi_uldivmod and friends)
 * -- even for a constant divisor when optimizing for size -- the magic
 * multiply (64x64->128 high half) or the shift/add form above
 * replaces it.  Elsewhere plain division is kept.
 */
#ifndef _UDIVMOD10_H_
#define _UDIVMOD10_H_

#include <stdint.h>

/*
 * Which form is cheapest is a property of the instruction set
 * architecture, not the ABI: the ILP32 ABIs of 64-bit ISAs (x86_64
 * x32, AArch64 ILP32, MIPS n32) have 32-bit pointers and longs but
 * 64-bit registers with divide and multiply instructions, so plain
 * 64-bit division is smallest there.  On smaller-register targets
 * with wide software division, the divide-free forms avoid libcalls.
 *
 * The predicates below classify the ISA from the compiler's macros:
 * a 64-bit ISA shows up as a 64-bit long or pointer, __int128
 * support, or a known 64-bit ISA macro.  _UDIVMOD10_WIDE_SOFTDIV is
 * the negation; targets lacking a usable wide multiply (ARMv6-M,
 * ARMv8-M Baseline, and other small cores) additionally cannot have
 * constant division strength-reduced, so the multiply-free shift/add
 * form is used there at every optimization level.
 */
#if !defined(_UDIVMOD10_NATIVE64) && !defined(_UDIVMOD10_WIDE_SOFTDIV)
#if __SIZEOF_LONG__ == 8 || __SIZEOF_POINTER__ == 8 || defined(__SIZEOF_INT128__)                 \
    || defined(__x86_64__) || defined(__aarch64__) || defined(__mips64) || defined(__loongarch64) \
    || defined(__powerpc64__) || defined(__s390x__) || defined(__sparcv9)                         \
    || (defined(__riscv) && __riscv_xlen == 64)
#define _UDIVMOD10_NATIVE64
#else
#define _UDIVMOD10_WIDE_SOFTDIV
#endif
#endif

#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_8M_BASE__) || defined(__AVR__) \
    || defined(__MSP430__)
#define _UDIVMOD10_NO_WIDE_MUL
#endif

/*
 * Use the divide-free conversions when the target has wide software
 * division and either printf-small-ultoa asks to avoid it (the
 * default) or this build optimizes while not optimizing for size
 * (__OPTIMIZE__ and not __OPTIMIZE_SIZE__: -Og, -O1, -O2, -O3; the
 * compiler macros do not distinguish the levels).  -O0 and
 * size-optimized builds honour a request to share the soft division
 * helper.  ultoa_invert.c uses this predicate as well; keep the
 * printf-small-ultoa description in meson_options.txt in sync.
 */
#if defined(_UDIVMOD10_WIDE_SOFTDIV)                                                         \
    && (defined(__IO_SMALL_ULTOA) || (defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)))
#define _UDIVMOD10_DIVIDE_FREE
#endif

static inline uint64_t
udivmod10_64(uint64_t n, char *rp)
{
    uint64_t q;
    char     r;

    /* single-digit values need no correction step */
    if (n < 10) {
        *rp = (char)n;
        return 0;
    }

    /* Compute n * 0x1999999999999999 / (2^64) ≃ n / 10 */

    /* q = n * 0xc >> 4 */
    q = (n >> 1) + (n >> 2);

    /* q = q * 0x11 >> 4 ≃ n * 0xcc >> 8 */
    q = q + (q >> 4);

    /* q = q * 0x101 >> 8 ≃ n * 0xcccc >> 16 */
    q = q + (q >> 8);

    /* q = q * 0x10001 >> 16 ≃ n * 0xcccccccc >> 32 */
    q = q + (q >> 16);

    /* q = q * 0x100000001 >> 32 ≃ n * 0xcccccccccccccccc >> 64 */
    q = q + (q >> 32);

    /* q ≃ n * 0x1999999999999999 >> 64 */
    q = q >> 3;

    /* r = n - q * 10 */
    r = (char)(n - (((q << 2) + q) << 1));

    /*
     * Because of the approximations above, q will
     * be +0/-1 of the real result. Check and adjust
     */
    if (r > 9) {
        q++;
        r -= 10;
    }
    *rp = r;
    return q;
}

static inline uint32_t
udivmod10_32(uint32_t n, char *rp)
{
    uint32_t q;
    char     r;

    if (n < 10) {
        *rp = (char)n;
        return 0;
    }

    q = (n >> 1) + (n >> 2);
    q = q + (q >> 4);
    q = q + (q >> 8);
    q = q + (q >> 16);
    q = q >> 3;
    r = (char)(n - (((q << 2) + q) << 1));
    if (r > 9) {
        q++;
        r -= 10;
    }
    *rp = r;
    return q;
}

#if defined(_UDIVMOD10_DIVIDE_FREE) && defined(_UDIVMOD10_NO_WIDE_MUL)

/*
 * Multiply-free and divide-free: reuse the shift/add udivmod10_64.
 * n = 10q + r gives n/5 = 2q + (r >= 5).
 */
#define udivmod10m_64 udivmod10_64

static inline uint64_t
div10m_64(uint64_t n)
{
    char d;

    return udivmod10_64(n, &d);
}

static inline uint64_t
div5m_64(uint64_t n)
{
    char     r;
    uint64_t q = udivmod10_64(n, &r);

    return (q << 1) + (uint64_t)(r >= 5);
}

#elif defined(_UDIVMOD10_DIVIDE_FREE) && defined(__OPTIMIZE_SIZE__)

/*
 * High half of a 64x64->128 product, built from 32-bit multiplies so
 * it needs no compiler-specific 128-bit type.  Chosen when optimizing
 * for size, where it replaces the wide soft-division call that even
 * constant divisors can otherwise lower to.
 */
static inline uint64_t
umulh64(uint64_t a, uint64_t b)
{
    uint64_t a_lo = (uint32_t)a, a_hi = a >> 32;
    uint64_t b_lo = (uint32_t)b, b_hi = b >> 32;
    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;
    uint64_t mid = (p0 >> 32) + (uint32_t)p1 + (uint32_t)p2;

    return p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
}

/*
 * umulh(x, 0xCC..CD) >> 3 is exactly floor(x/10) and
 * umulh(x, 0xCC..CD) >> 2 exactly floor(x/5) for all 64-bit x; the
 * remainder follows from one multiply-subtract.  The udivmod10m_64 /
 * div10m_64 / div5m_64 names select whichever of the three
 * implementations above applies to the current target and build.
 */
static inline uint64_t
udivmod10m_64(uint64_t n, char *rp)
{
    uint64_t q = umulh64(n, 0xCCCCCCCCCCCCCCCDull) >> 3;

    *rp = (char)(n - (((q << 2) + q) << 1));
    return q;
}

static inline uint64_t
div10m_64(uint64_t n)
{
    return umulh64(n, 0xCCCCCCCCCCCCCCCDull) >> 3;
}

static inline uint64_t
div5m_64(uint64_t n)
{
    return umulh64(n, 0xCCCCCCCCCCCCCCCDull) >> 2;
}

#else /* plain division */

static inline uint64_t
udivmod10m_64(uint64_t n, char *rp)
{
    uint64_t q = n / 10u;

    *rp = (char)(n % 10u);
    return q;
}

static inline uint64_t
div10m_64(uint64_t n)
{
    return n / 10u;
}

static inline uint64_t
div5m_64(uint64_t n)
{
    return n / 5u;
}

#endif /* implementation selection */

#endif /* _UDIVMOD10_H_ */
