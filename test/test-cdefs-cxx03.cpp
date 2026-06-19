/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *   * Neither the name of Qualcomm Innovation Center, Inc. nor the names
 *     of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Test that _Static_assert and _Alignof work with template types whose
 * argument lists contain commas, when compiled in C++03 mode.
 *
 * When Clang compiles in pre-C++11 mode, _Static_assert and _Alignof are
 * available as built-in keywords.  If they were instead defined as
 * function-like macros (the fallback for other compilers), the comma inside
 * a template argument such as pair<int,int> would be treated as a macro
 * argument separator, causing a compilation error.
 *
 * This test is skipped on non-Clang compilers because the fallback macro
 * definitions do not support comma-containing arguments.
 */

#ifdef __clang__

#include <sys/cdefs.h>
#include <stddef.h>

template <typename A, typename B>
struct pair {
    A first;
    B second;
};

/* These would fail if _Static_assert/_Alignof were function-like macros:
 * the comma in pair<int,int> would be seen as a macro argument separator. */
_Static_assert(sizeof(pair<int,int>) == 2 * sizeof(int), "pair size");
_Static_assert(_Alignof(pair<int,int>) == _Alignof(int), "pair align");

#endif /* __clang__ */

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wmain"
#endif

extern "C" {

int main(void);

int main(void) { return 0; }

}
