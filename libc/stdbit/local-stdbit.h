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

#ifndef _LOCAL_STDBIT_H_
#define _LOCAL_STDBIT_H_

#include <stdbit.h>

#define USE_TYPE_NONE           0
#define USE_TYPE_INT            1
#define USE_TYPE_CHAR           2
#define USE_TYPE_SHORT          3
#define USE_TYPE_LONG           4
#define USE_TYPE_LONG_LONG      5
#define USE_TYPE_INT128         6

#define CAN_USE_INT(size)       ((size) == __SIZEOF_INT__ * 8)
#define CAN_USE_CHAR(size)      ((size) == 8)
#define CAN_USE_SHORT(size)     ((size) == __SIZEOF_SHORT__ * 8)
#define CAN_USE_LONG(size)      ((size) == __SIZEOF_LONG__ * 8)
#define CAN_USE_LONG_LONG(size) ((size) == __SIZEOF_LONG_LONG__ * 8)
#ifdef __SIZEOF_INT128__
#define CAN_USE_INT128(size) ((size) == __SIZEOF_INT128__ * 8)
#else
#define CAN_USE_INT128(size) 0
#endif

#define SELECT_TYPE(size)                               \
    (CAN_USE_INT(size)             ? USE_TYPE_INT       \
         : CAN_USE_CHAR(size)      ? USE_TYPE_CHAR      \
         : CAN_USE_SHORT(size)     ? USE_TYPE_SHORT     \
         : CAN_USE_LONG(size)      ? USE_TYPE_LONG      \
         : CAN_USE_LONG_LONG(size) ? USE_TYPE_LONG_LONG \
         : CAN_USE_INT128(size)    ? USE_TYPE_INT128    \
                                   : USE_TYPE_NONE)

#define INT_SUFFIX       _ui
#define CHAR_SUFFIX      _uc
#define SHORT_SUFFIX     _us
#define LONG_SUFFIX      _ul
#define LONG_LONG_SUFFIX _ull
#define INT128_SUFFIX    _u128

#if SELECT_TYPE(WANT_SIZE) == USE_TYPE_INT
#define stdbit_t    unsigned int
#define NAME_SUFFIX INT_SUFFIX
#elif SELECT_TYPE(WANT_SIZE) == USE_TYPE_CHAR
#define stdbit_t    unsigned char
#define NAME_SUFFIX CHAR_SUFFIX
#elif SELECT_TYPE(WANT_SIZE) == USE_TYPE_SHORT
#define stdbit_t    unsigned short
#define NAME_SUFFIX SHORT_SUFFIX
#elif SELECT_TYPE(WANT_SIZE) == USE_TYPE_LONG
#define stdbit_t    unsigned long
#define NAME_SUFFIX LONG_SUFFIX
#elif SELECT_TYPE(WANT_SIZE) == USE_TYPE_LONG_LONG
#define stdbit_t    unsigned long long
#define NAME_SUFFIX LONG_LONG_SUFFIX
#elif SELECT_TYPE(WANT_SIZE) == USE_TYPE_INT128
#define stdbit_t    unsigned __int128
#define NAME_SUFFIX INT128_SUFFIX
#endif

#define __name(a, b) a##b
#define _name(a, b)  __name(a, b)
#define name(a)      _name(a, NAME_SUFFIX)
#define name_s(a, b) _name(a, b)

#define name_ui(a)   _name(a, INT_SUFFIX)
#define name_uc(a)   _name(a, CHAR_SUFFIX)
#define name_us(a)   _name(a, SHORT_SUFFIX)
#define name_ul(a)   _name(a, LONG_SUFFIX)
#define name_ull(a)  _name(a, LONG_LONG_SUFFIX)
#define name_u128(a) _name(a, INT128_SUFFIX)

#define NAME_SB      name(NAME)
#define NAME_UI      name_ui(NAME)
#define NAME_UC      name_uc(NAME)
#define NAME_US      name_us(NAME)
#define NAME_UL      name_ul(NAME)
#define NAME_ULL     name_ull(NAME)
#define NAME_U128    name_u128(NAME)

#if CAN_USE_CHAR(WANT_SIZE) && SELECT_TYPE(WANT_SIZE) != USE_TYPE_CHAR
#define ALIAS_CHAR __strong_reference_dup(NAME_SB, NAME_UC);
#else
#define ALIAS_CHAR
#endif

#if CAN_USE_SHORT(WANT_SIZE) && SELECT_TYPE(WANT_SIZE) != USE_TYPE_SHORT
#define ALIAS_SHORT __strong_reference_dup(NAME_SB, NAME_US);
#else
#define ALIAS_SHORT
#endif

#if CAN_USE_LONG(WANT_SIZE) && SELECT_TYPE(WANT_SIZE) != USE_TYPE_LONG
#define ALIAS_LONG __strong_reference_dup(NAME_SB, NAME_UL);
#else
#define ALIAS_LONG
#endif

#if CAN_USE_LONG_LONG(WANT_SIZE) && SELECT_TYPE(WANT_SIZE) != USE_TYPE_LONG_LONG
#define ALIAS_LONG_LONG __strong_reference_dup(NAME_SB, NAME_ULL);
#else
#define ALIAS_LONG_LONG
#endif

#if CAN_USE_INT128(WANT_SIZE) && SELECT_TYPE(WANT_SIZE) != USE_TYPE_INT128
#define ALIAS_INT128 __strong_reference_dup(NAME_SB, NAME_U128);
#else
#define ALIAS_INT128
#endif

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wattribute-alias="
#pragma GCC diagnostic ignored "-Wattribute-alias"
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif

#define ALIASES     \
    ALIAS_CHAR      \
    ALIAS_SHORT     \
    ALIAS_LONG      \
    ALIAS_LONG_LONG \
    ALIAS_INT128

#endif
