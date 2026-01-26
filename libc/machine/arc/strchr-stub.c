/*
   Copyright (c) 2015-2024, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#include <picolibc.h>

/* strchr.S */
#if !defined(__OPTIMIZE_SIZE__) && !defined(__PREFER_SIZE_OVER_SPEED)
#if defined(__ARC601__) || !defined(__ARC_BARREL_SHIFTER__)
#define STRCHR_ASM
#endif
#endif

/* strchr-bs.S */
#if !defined(__OPTIMIZE_SIZE__) && !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__ARC_RF16__)
#if defined(__ARC_BARREL_SHIFTER__)                                              \
    && (defined(__ARC600__) || (!defined(__ARC_NORM__) && !defined(__ARC601__)))
#define STRCHR_ASM
#endif
#endif

/* strchr-bs-norm.S */
#if !defined(__OPTIMIZE_SIZE__) && !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__ARC_RF16__)
#if (defined(__ARC700__) || defined(__ARCEM__) || defined(__ARCHS__)) && defined(__ARC_NORM__) \
    && defined(__ARC_BARREL_SHIFTER__)
#define STRCHR_ASM
#endif
#endif

#ifndef STRCHR_ASM
#include "../../string/strchr.c"
#endif
