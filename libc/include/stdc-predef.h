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

#ifndef _STDC_PREDEF_H_
#define _STDC_PREDEF_H_

/*
 * Check compiler float support; picolibc is expected to support
 * whatever the hardware is capable of with compiler help
 */

#undef __STDC_IEC_559__
#undef __STDC_IEC_60559_BFP__

#if __GCC_IEC_559 > 0
#define __STDC_IEC_559__       1
#define __STDC_IEC_60559_BFP__ 202311L
#endif

#undef __STDC_IEC_559_COMPLEX__
#undef __STDC_IEC_60559_COMPLEX__

#if __GCC_IEC_559_COMPLEX > 0
#define __STDC_IEC_559_COMPLEX__   1
#define __STDC_IEC_60559_COMPLEX__ 202311L
#endif

#undef __STDC_ISO_10646__

#if __WCHAR_WIDTH__ >= 21
/*
 * The current ctype tables are based on Unicode 17.0.0.
 *
 * This value matches the release date of ISO/IEC 10646:2020/Amd
 * 2:2025, which is the version of that standard based on Unicode
 * 16.0.0. If a ISO/IEC 10646 version is released that is based on
 * Unicode 17.0.0, this value should be updated.
 */
#define __STDC_ISO_10646__ 202506L
#endif

#endif /* _STDC_PREDEF_H_ */
