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

#ifndef _MACHINE_FENV_H
#define _MACHINE_FENV_H

/* Exception flags */
#define FE_INVALID    0x00000002u /* bit 1 */
#define FE_DIVBYZERO  0x00000004u /* bit 2 */
#define FE_OVERFLOW   0x00000008u /* bit 3 */
#define FE_UNDERFLOW  0x00000010u /* bit 4 */
#define FE_INEXACT    0x00000020u /* bit 5 */

#define FE_ALL_EXCEPT (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)

/* Rounding modes */
#define FE_TONEAREST     0
#define FE_TOWARDZERO    1
#define FE_DOWNWARD      2
#define FE_UPWARD        3

#define _FE_RND_OFF      22u
#define _FE_RND_MASK     0x3u

#define _FE_ENABLE_SHIFT 24u

/*
 * Mask of all FP-relevant USR bits.  fegetenv/fesetenv only touch
 * these bits so that non-FP USR fields (loop counters, etc.) are
 * never disturbed.
 */
#define _FE_FP_MASK                                                                       \
    (FE_ALL_EXCEPT | (_FE_RND_MASK << _FE_RND_OFF) | (FE_ALL_EXCEPT << _FE_ENABLE_SHIFT))

typedef unsigned long fenv_t;
typedef unsigned long fexcept_t;

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define __declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#include <machine/fenv-fp.h>
#endif

#endif /* _MACHINE_FENV_H */
