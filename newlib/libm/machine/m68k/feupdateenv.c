/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#include <fenv.h>

#ifdef __HAVE_68881__

int feupdateenv(const fenv_t *envp)
{
    fenv_t env = *envp;

    /* Get current fpcr and fpsr */

    fenv_t fpcr;
    fexcept_t fpsr;

    __asm__ volatile("fmove.l %%fpcr, %0" : "=d"(fpcr));
    __asm__ volatile("fmove.l %%fpsr, %0" : "=d"(fpsr));

    /* Set the rounding mode */

    fpcr = (fpcr & ~(0x3 << 4)) | (env & 3);

    /* Set the exception enables */

    fpcr = (fpcr & 0xff) | (env & 0xff00);

    /* Merge in exceptions */

    fpsr |= (env & 0xf8);

    /* Save to registers */

    __asm__ volatile("fmove.l %0, %%fpcr" : : "d"(fpcr));
    __asm__ volatile("fmove.l %0, %%fpsr" : : "d"(fpsr));

    return 0;

}
#else
#include "../../fenv/feupdateenv.c"
#endif
