/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#ifndef _STDBIT_H_
#define _STDBIT_H_

#define __STDC_VERSION_STDBIT_H__ 202311L

#include <sys/cdefs.h>
#define __need_size_t
#include <stddef.h>
#include <sys/_exactint.h>
#include <sys/_leastint.h>

#define __STDC_ENDIAN_LITTLE__ __ORDER_LITTLE_ENDIAN__
#define __STDC_ENDIAN_BIG__    __ORDER_BIG_ENDIAN__
#define __STDC_ENDIAN_NATIVE__ __BYTE_ORDER__

_BEGIN_STD_C

#define ___stdbit_func(a, b)  a##b
#define __stdbit_func(a, b)   ___stdbit_func(a, b)
#define __stdbit_func_uc(a)   __stdbit_func(a, _uc)
#define __stdbit_func_us(a)   __stdbit_func(a, _us)
#define __stdbit_func_ui(a)   __stdbit_func(a, _ui)
#define __stdbit_func_ul(a)   __stdbit_func(a, _ul)
#define __stdbit_func_ull(a)  __stdbit_func(a, _ull)
#define __stdbit_func_u128(a) __stdbit_func(a, _u128)

#if __STDC_VERSION__ >= 201112L

#ifdef __SIZEOF_INT128__
#define __stdbit_int128_case(func) , unsigned __int128 : func
#else
#define __stdbit_int128_case(func)
#endif
#define __stdbit_generic_ui(func, value)                             \
    (_Generic(value,                                                 \
         unsigned char: __stdbit_func_uc(func),                      \
         unsigned short: __stdbit_func_us(func),                     \
         unsigned int: __stdbit_func_ui(func),                       \
         unsigned long: __stdbit_func_ul(func),                      \
         unsigned long long: __stdbit_func_ull(func)                 \
             __stdbit_int128_case(__stdbit_func_u128(func)))(value))

#define __stdbit_generic_ub(func, value) __stdbit_generic_ui(func, value)

#else /* __STDC_VERSION__ >= 201112L */

#ifdef __SIZEOF_INT128__
#define __stdbit_int128_case(func, value)                                   \
    sizeof(value) == sizeof(__int128) ? __stdbit_func_u128(func)(value) : 0
#else
#define __stdbit_int128_case(func, value) 0
#endif

#define __stdbit_generic_ui(func, value)                                           \
    (sizeof(value) == sizeof(char)            ? __stdbit_func_uc(func)(value)      \
         : sizeof(value) == sizeof(short)     ? __stdbit_func_us(func)(value)      \
         : sizeof(value) == sizeof(int)       ? __stdbit_func_ui(func)(value)      \
         : sizeof(value) == sizeof(long)      ? __stdbit_func_ul(func)(value)      \
         : sizeof(value) == sizeof(long long) ? __stdbit_func_ull(func)(value)     \
                                              : __stdbit_int128_case(func, value))

#define __stdbit_generic_ub(func, value) ((__typeof(value))__stdbit_generic_ui(func, value))

#endif /* else __STDC_VERSION__ >= 201112L */

/* Count leading ones */

unsigned int stdc_leading_ones_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_leading_ones_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_leading_ones_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_leading_ones_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_leading_ones_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_leading_ones_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_leading_ones)
#define stdc_leading_ones(value) __builtin_stdc_leading_ones(value)
#elif __has_builtin(__builtin_clzg)
#define stdc_leading_ones(value)                                                                \
    ((unsigned int)__builtin_clzg((__typeof(value))~(value), (int)sizeof(__typeof(value)) * 8))
#else
#define stdc_leading_ones(value) __stdbit_generic_ui(stdc_leading_ones, value)
#endif

/* Count leading zeros */

unsigned int stdc_leading_zeros_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_leading_zeros_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_leading_zeros_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_leading_zeros_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_leading_zeros_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_leading_zeros_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_leading_zeros)
#define stdc_leading_zeros(value) __builtin_stdc_leading_zeros(value)
#elif __has_builtin(__builtin_clzg)
#define stdc_leading_zeros(value)                                           \
    ((unsigned int)__builtin_clzg(value, (int)sizeof(__typeof(value)) * 8))
#else
#define stdc_leading_zeros(value) __stdbit_generic_ui(stdc_leading_zeros, value)
#endif

/* Count trailing ones */

unsigned int stdc_trailing_ones_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_trailing_ones_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_trailing_ones_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_trailing_ones_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_trailing_ones_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_trailing_ones_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_trailing_ones)
#define stdc_trailing_ones(value) __builtin_stdc_trailing_ones(value)
#elif __has_builtin(__builtin_ctzg)
#define stdc_trailing_ones(value)                                                               \
    ((unsigned int)__builtin_ctzg((__typeof(value))~(value), (int)sizeof(__typeof(value)) * 8))
#else
#define stdc_trailing_ones(value) __stdbit_generic_ui(stdc_trailing_ones, value)
#endif

/* Count trailing zeros */

unsigned int stdc_trailing_zeros_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_trailing_zeros_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_trailing_zeros_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_trailing_zeros_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_trailing_zeros_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_trailing_zeros_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_trailing_zeros)
#define stdc_trailing_zeros(value) __builtin_stdc_trailing_zeros(value)
#elif __has_builtin(__builtin_ctzg)
#define stdc_trailing_zeros(value)                                          \
    ((unsigned int)__builtin_ctzg(value, (int)sizeof(__typeof(value)) * 8))
#else
#define stdc_trailing_zeros(value) __stdbit_generic_ui(stdc_trailing_zeros, value)
#endif

/* First leading ones */

unsigned int stdc_first_leading_one_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_first_leading_one_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_first_leading_one_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_first_leading_one_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_first_leading_one_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_first_leading_one_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_first_leading_one)
#define stdc_first_leading_one(value) __builtin_stdc_first_leading_one(value)
#elif __has_builtin(__builtin_clzg)
#define stdc_first_leading_one(value) ((unsigned int)__builtin_clzg(value, -1) + 1U)
#else
#define stdc_first_leading_one(value) __stdbit_generic_ui(stdc_first_leading_one, value)
#endif

/* First leading zeros */

unsigned int stdc_first_leading_zero_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_first_leading_zero_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_first_leading_zero_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_first_leading_zero_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_first_leading_zero_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_first_leading_zero_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_first_leading_zero)
#define stdc_first_leading_zero(value) __builtin_stdc_first_leading_zero(value)
#elif __has_builtin(__builtin_clzg)
#define stdc_first_leading_zero(value)                                 \
    ((unsigned int)__builtin_clzg((__typeof(value))~(value), -1) + 1U)
#else
#define stdc_first_leading_zero(value) __stdbit_generic_ui(stdc_first_leading_zero, value)
#endif

/* First trailing ones */

unsigned int stdc_first_trailing_one_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_first_trailing_one_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_first_trailing_one_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_first_trailing_one_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_first_trailing_one_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_first_trailing_one_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_first_trailing_one)
#define stdc_first_trailing_one(value) __builtin_stdc_first_trailing_one(value)
#elif __has_builtin(__builtin_ctzg)
#define stdc_first_trailing_one(value) ((unsigned int)__builtin_ctzg(value, -1) + 1U)
#else
#define stdc_first_trailing_one(value) __stdbit_generic_ui(stdc_first_trailing_one, value)
#endif

/* First trailing zeros */

unsigned int stdc_first_trailing_zero_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_first_trailing_zero_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_first_trailing_zero_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_first_trailing_zero_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_first_trailing_zero_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_first_trailing_zero_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_first_trailing_zero)
#define stdc_first_trailing_zero(value) __builtin_stdc_first_trailing_zero(value)
#elif __has_builtin(__builtin_ctzg)
#define stdc_first_trailing_zero(value)                                \
    ((unsigned int)__builtin_ctzg((__typeof(value))~(value), -1) + 1U)
#else
#define stdc_first_trailing_zero(value) __stdbit_generic_ui(stdc_first_trailing_zero, value)
#endif

/* Count ones */

unsigned int stdc_count_ones_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_count_ones_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_count_ones_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_count_ones_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_count_ones_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_count_ones_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_count_ones)
#define stdc_count_ones(value) __builtin_stdc_count_ones(value)
#elif __has_builtin(__builtin_popcountg)
#define stdc_count_ones(value) __builtin_popcountg(value)
#else
#define stdc_count_ones(value) __stdbit_generic_ui(stdc_count_ones, value)
#endif

/* Count zeros */

unsigned int stdc_count_zeros_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_count_zeros_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_count_zeros_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_count_zeros_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_count_zeros_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_count_zeros_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_count_zeros)
#define stdc_count_zeros(value) __builtin_stdc_count_zeros(value)
#elif __has_builtin(__builtin_popcountg)
#define stdc_count_zeros(value) __builtin_popcountg((__typeof(value))~(value))
#else
#define stdc_count_zeros(value) __stdbit_generic_ui(stdc_count_zeros, value)
#endif

/* Check for values with only one bit set */

_Bool stdc_has_single_bit_uc(unsigned char value) __pure __picolibc_export;
_Bool stdc_has_single_bit_us(unsigned short value) __pure __picolibc_export;
_Bool stdc_has_single_bit_ui(unsigned int value) __pure __picolibc_export;
_Bool stdc_has_single_bit_ul(unsigned long value) __pure __picolibc_export;
_Bool stdc_has_single_bit_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
_Bool stdc_has_single_bit_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_has_single_bit)
#define stdc_has_single_bit(value) __builtin_stdc_has_single_bit(value)
#elif __has_builtin(__builtin_popcountg)
#define stdc_has_single_bit(value) ((_Bool)(__builtin_popcountg(value) == 1))
#else
#define stdc_has_single_bit(value) __stdbit_generic_ui(stdc_has_single_bit, value)
#endif

/* Compute number bits needed to hold a value */

unsigned int stdc_bit_width_uc(unsigned char value) __pure __picolibc_export;
unsigned int stdc_bit_width_us(unsigned short value) __pure __picolibc_export;
unsigned int stdc_bit_width_ui(unsigned int value) __pure __picolibc_export;
unsigned int stdc_bit_width_ul(unsigned long value) __pure __picolibc_export;
unsigned int stdc_bit_width_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned int stdc_bit_width_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if __has_builtin(__builtin_stdc_bit_width)
#define stdc_bit_width(value) __builtin_stdc_bit_width(value)
#elif __has_builtin(__builtin_clzg)
#define stdc_bit_width(value)                                   \
    ((unsigned int)(sizeof(__typeof(value)) * 8)                \
     - __builtin_clzg(value, (int)sizeof(__typeof(value)) * 8))
#else
#warning using generic bit width
#define stdc_bit_width(value) __stdbit_generic_ui(stdc_bit_width, value)
#endif

/* Largest power of two <= value */

unsigned char      stdc_bit_floor_uc(unsigned char value) __pure __picolibc_export;
unsigned short     stdc_bit_floor_us(unsigned short value) __pure __picolibc_export;
unsigned int       stdc_bit_floor_ui(unsigned int value) __pure __picolibc_export;
unsigned long      stdc_bit_floor_ul(unsigned long value) __pure __picolibc_export;
unsigned long long stdc_bit_floor_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned __int128 stdc_bit_floor_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if defined(__declare_extern_inline) && __has_builtin(__builtin_clzg)
#define __stdc_bit_floor(type, ext)                                              \
    __declare_extern_inline(type) __stdbit_func(stdc_bit_floor, ext)(type value) \
    {                                                                            \
        if (value == 0)                                                          \
            return 0;                                                            \
        return (type)1 << (sizeof(type) * 8 - 1 - __builtin_clzg(value));        \
    }
__stdc_bit_floor(unsigned char, _uc);
__stdc_bit_floor(unsigned short, _us);
__stdc_bit_floor(unsigned int, _ui);
__stdc_bit_floor(unsigned long, _ul);
__stdc_bit_floor(unsigned long long, _ull);
#ifdef __SIZEOF_INT128__
__stdc_bit_floor(unsigned __int128, _u128);
#endif
#endif

#if __has_builtin(__builtin_stdc_bit_floor)
#define stdc_bit_floor(value) __builtin_stdc_bit_floor(value)
#else
#define stdc_bit_floor(value) __stdbit_generic_ub(stdc_bit_floor, value)
#endif

/* Smallest power of two >= value */

unsigned char      stdc_bit_ceil_uc(unsigned char value) __pure __picolibc_export;
unsigned short     stdc_bit_ceil_us(unsigned short value) __pure __picolibc_export;
unsigned int       stdc_bit_ceil_ui(unsigned int value) __pure __picolibc_export;
unsigned long      stdc_bit_ceil_ul(unsigned long value) __pure __picolibc_export;
unsigned long long stdc_bit_ceil_ull(unsigned long long value) __pure __picolibc_export;
#ifdef __SIZEOF_INT128__
unsigned __int128 stdc_bit_ceil_u128(unsigned __int128 value) __pure __picolibc_export;
#endif

#if defined(__declare_extern_inline) && __has_builtin(__builtin_clzg)
#define __stdc_bit_ceil(type, ext)                                                    \
    __declare_extern_inline(type) __stdbit_func(stdc_bit_ceil, ext)(type value)       \
    {                                                                                 \
        if (value <= 1)                                                               \
            return 1;                                                                 \
        return (type)2 << (sizeof(type) * 8 - 1 - __builtin_clzg((type)(value - 1))); \
    }
__stdc_bit_ceil(unsigned char, _uc);
__stdc_bit_ceil(unsigned short, _us);
__stdc_bit_ceil(unsigned int, _ui);
__stdc_bit_ceil(unsigned long, _ul);
__stdc_bit_ceil(unsigned long long, _ull);
#ifdef __SIZEOF_INT128__
__stdc_bit_ceil(unsigned __int128, _u128);
#endif
#endif

#if __has_builtin(__builtin_stdc_bit_ceil)
#define stdc_bit_ceil(value) __builtin_stdc_bit_ceil(value)
#else
#define stdc_bit_ceil(value) __stdbit_generic_ub(stdc_bit_ceil, value)
#endif

_END_STD_C

#endif /* _STDBIT_H_ */
