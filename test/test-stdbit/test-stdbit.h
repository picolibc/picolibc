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

#ifndef _TEST_STDBIT_H_
#define _TEST_STDBIT_H_

#include <stdlib.h>

#define __name(a, b)     a##b
#define _name(a, b)      __name(a, b)
#define __name3(a, b, c) a##b##c
#define _name3(a, b, c)  __name3(a, b, c)

#define randint(type, ext)                       \
    static inline type _name(randint, ext)(void) \
    {                                            \
        type   ret = 0;                          \
        size_t i;                                \
        for (i = 0; i < sizeof(type); i++) {     \
            if (sizeof(type) > 1)                \
                ret <<= 8;                       \
            ret |= (rand() & 0xff);              \
        }                                        \
        return ret;                              \
    }

randint(unsigned char, _uc);

randint(unsigned short, _us);

randint(unsigned int, _ui);

randint(unsigned long, _ul);

randint(unsigned long long, _ull);

#ifdef __SIZEOF_INT128__
randint(unsigned __int128, _u128);
#endif

#define COUNT 1000

#define test(func, type, ext, value)                                                 \
    do {                                                                             \
        type         _val = (value);                                                 \
        unsigned int got = func(_val);                                               \
        unsigned int want = _name3(check_, func, ext)(_val);                         \
        if (got != want) {                                                           \
            printf("0x%llx: got %u want %u\n", (unsigned long long)_val, got, want); \
            ret = 0;                                                                 \
        }                                                                            \
    } while (0)

#define loop(func, type, ext)                                \
    static inline int _name(loop, ext)(void)                 \
    {                                                        \
        int    ret = 1;                                      \
        size_t count;                                        \
        printf("Testing %s type %s\n", #func, #type);        \
        test(func, type, ext, 0);                            \
        test(func, type, ext, -1);                           \
        for (count = 0; count < 8 * sizeof(type); count++) { \
            type val = (type)1 << count;                     \
            test(func, type, ext, val);                      \
            test(func, type, ext, val - 1);                  \
            test(func, type, ext, ~val);                     \
            test(func, type, ext, ~(val - 1));               \
        }                                                    \
        for (count = 0; count < COUNT; count++) {            \
            type val;                                        \
            val = _name(randint, ext)();                     \
            test(func, type, ext, val);                      \
        }                                                    \
        return ret;                                          \
    }

#ifdef __SIZEOF_INT128__
#define loop128(func)  loop(func, unsigned __int128, _u128)
#define check128(func) check(func, unsigned __int128, _u128)
#define run128()          \
    do {                  \
        if (!loop_u128()) \
            ret = 1;      \
    } while (0)
#else
#define loop128(func)
#define check128(func)
#define run128()
#endif

#define loops(func)                       \
    loop(func, unsigned char, _uc);       \
    loop(func, unsigned short, _us);      \
    loop(func, unsigned int, _ui);        \
    loop(func, unsigned long, _ul);       \
    loop(func, unsigned long long, _ull); \
    loop128(func);

#define checks(func)                       \
    check(func, unsigned char, _uc);       \
    check(func, unsigned short, _us);      \
    check(func, unsigned int, _ui);        \
    check(func, unsigned long, _ul);       \
    check(func, unsigned long long, _ull); \
    check128(func);

#define run()            \
    do {                 \
        if (!loop_uc())  \
            ret = 1;     \
        if (!loop_us())  \
            ret = 1;     \
        if (!loop_ui())  \
            ret = 1;     \
        if (!loop_ul())  \
            ret = 1;     \
        if (!loop_ull()) \
            ret = 1;     \
        run128();        \
    } while (0)

#endif /* _TEST_STDBIT_H_ */
