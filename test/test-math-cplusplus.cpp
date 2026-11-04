/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Check that math.h can be used from C++. scripts/check-cplusplus-macros
 * compiles this with each C++ version, and also checks that math.h
 * doesn't define the macros listed in cplusplus-hidden-macros.txt.
 */

#include <math.h>

/*
 * Since C++11, <cmath> declares bool isinf(double) and bool
 * isnan(double), and the C++ library may bring them into the global
 * namespace, so math.h must not declare int versions of them
 */
#if __cplusplus >= 201103L
#undef isinf
#undef isnan
namespace cxx {
bool isinf(double);
bool isnan(double);
}
using cxx::isinf;
using cxx::isnan;
#endif

int
main(void)
{
    int ret = 0;
#if __MISC_VISIBLE
    /* Without the macros, these call the functions */
    ret += isinff(1.0f) + isnanf(1.0f);
#ifdef __HAVE_LONG_DOUBLE
    ret += isinfl(1.0L) + isnanl(1.0L);
#endif
#endif
    return ret;
}
