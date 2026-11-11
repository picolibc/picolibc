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

#ifndef _SYS__FASTINT_H_
#define _SYS__FASTINT_H_

/*
 * Fastest minimum-width integer types
 *
 * Assume int to be the fastest type for all types with a width
 * less than __INT_MAX__ rsp. INT_MAX
 */
#ifdef __INT_FAST8_TYPE__
typedef __INT_FAST8_TYPE__  int_fast8_t;
typedef __UINT_FAST8_TYPE__ uint_fast8_t;
#define __int_fast8_t_defined 1
#elif __STDINT_EXP(INT_MAX) >= 0x7f
typedef signed int   int_fast8_t;
typedef unsigned int uint_fast8_t;
#define __int_fast8_t_defined 1
#endif

#ifdef __INT_FAST16_TYPE__
typedef __INT_FAST16_TYPE__  int_fast16_t;
typedef __UINT_FAST16_TYPE__ uint_fast16_t;
#define __int_fast16_t_defined 1
#elif __STDINT_EXP(INT_MAX) >= 0x7fff
typedef signed int   int_fast16_t;
typedef unsigned int uint_fast16_t;
#define __int_fast16_t_defined 1
#endif

#ifdef __INT_FAST32_TYPE__
typedef __INT_FAST32_TYPE__  int_fast32_t;
typedef __UINT_FAST32_TYPE__ uint_fast32_t;
#define __int_fast32_t_defined 1
#elif __STDINT_EXP(INT_MAX) >= 0x7fffffff
typedef signed int   int_fast32_t;
typedef unsigned int uint_fast32_t;
#define __int_fast32_t_defined 1
#endif

#ifdef __INT_FAST64_TYPE__
typedef __INT_FAST64_TYPE__  int_fast64_t;
typedef __UINT_FAST64_TYPE__ uint_fast64_t;
#define __int_fast64_t_defined 1
#elif __STDINT_EXP(INT_MAX) > 0x7fffffff
typedef signed int   int_fast64_t;
typedef unsigned int uint_fast64_t;
#define __int_fast64_t_defined 1
#endif

/*
 * Fall back to [u]int_least<N>_t for [u]int_fast<N>_t types
 * not having been defined, yet.
 * Leave undefined, if [u]int_least<N>_t should not be available.
 */
#if !__int_fast8_t_defined
#if __int_least8_t_defined
typedef int_least8_t  int_fast8_t;
typedef uint_least8_t uint_fast8_t;
#define __int_fast8_t_defined 1
#endif
#endif

#if !__int_fast16_t_defined
#if __int_least16_t_defined
typedef int_least16_t  int_fast16_t;
typedef uint_least16_t uint_fast16_t;
#define __int_fast16_t_defined 1
#endif
#endif

#if !__int_fast32_t_defined
#if __int_least32_t_defined
typedef int_least32_t  int_fast32_t;
typedef uint_least32_t uint_fast32_t;
#define __int_fast32_t_defined 1
#endif
#endif

#if !__int_fast64_t_defined
#if __int_least64_t_defined
typedef int_least64_t  int_fast64_t;
typedef uint_least64_t uint_fast64_t;
#define __int_fast64_t_defined 1
#endif
#endif

#endif /* _SYS__FASTINT_H_ */
