/*
 * xtensa/config/core-isa.h -- minimum required HAL definitions that are
 *				dependent on Xtensa processor CORE configuration
 *
 *  See <xtensa/config/core.h>, which includes this file, for more details.
 */

/* Xtensa processor core configuration information.

   Copyright (c) 1999-2023 Tensilica Inc.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#ifndef _XTENSA_CORE_CONFIGURATION_H
#define _XTENSA_CORE_CONFIGURATION_H

#if defined(_LIBC) || defined(_LIBM) || defined(_LIBGLOSS)

/* Macros used to build newlib and libgloss */

#undef XCHAL_HAVE_BE
#ifdef __XCHAL_HAVE_BE
#define XCHAL_HAVE_BE		__XCHAL_HAVE_BE
#else
#define XCHAL_HAVE_BE		0	/* big-endian byte ordering */
#endif

#undef XCHAL_HAVE_WINDOWED
#ifdef __XCHAL_HAVE_WINDOWED
#define XCHAL_HAVE_WINDOWED		__XCHAL_HAVE_WINDOWED
#else
#define XCHAL_HAVE_WINDOWED		1	/* windowed registers option */
#endif

#undef XCHAL_NUM_AREGS
#ifdef __XCHAL_NUM_AREGS
#define XCHAL_NUM_AREGS		__XCHAL_NUM_AREGS
#else
#define XCHAL_NUM_AREGS		64	/* num of physical addr regs */
#endif

#undef XCHAL_HAVE_DENSITY
#ifdef __XCHAL_HAVE_DENSITY
#define XCHAL_HAVE_DENSITY		__XCHAL_HAVE_DENSITY
#else
#define XCHAL_HAVE_DENSITY		1	/* 16-bit instructions */
#endif

#undef XCHAL_HAVE_LOOPS
#ifdef __XCHAL_HAVE_LOOPS
#define XCHAL_HAVE_LOOPS		__XCHAL_HAVE_LOOPS
#else
#define XCHAL_HAVE_LOOPS		1	/* zero-overhead loops */
#endif

#undef XCHAL_HAVE_L32R
#ifdef __XCHAL_HAVE_L32R
#define XCHAL_HAVE_L32R		__XCHAL_HAVE_L32R
#else
#define XCHAL_HAVE_L32R		1	/* L32R instruction */
#endif

#undef XCHAL_HAVE_FP
#ifdef __XCHAL_HAVE_FP
#define XCHAL_HAVE_FP		__XCHAL_HAVE_FP
#else
#define XCHAL_HAVE_FP		1	/* single prec floating point */
#endif

#undef XCHAL_HAVE_FP_SQRT
#ifdef __XCHAL_HAVE_FP_SQRT
#define XCHAL_HAVE_FP_SQRT		__XCHAL_HAVE_FP_SQRT
#else
#define XCHAL_HAVE_FP_SQRT		1	/* FP with SQRT instructions */
#endif

#undef XCHAL_HAVE_DFP
#ifdef __XCHAL_HAVE_DFP
#define XCHAL_HAVE_DFP		__XCHAL_HAVE_DFP
#else
#define XCHAL_HAVE_DFP		0	/* double precision FP pkg */
#endif

#undef XCHAL_INST_FETCH_WIDTH
#ifdef __XCHAL_INST_FETCH_WIDTH
#define XCHAL_INST_FETCH_WIDTH		__XCHAL_INST_FETCH_WIDTH
#else
#define XCHAL_INST_FETCH_WIDTH		4	/* instr-fetch width in bytes */
#endif

#else /* defined(_LIBC) || defined(_LIBM) || defined(_LIBGLOSS) */

/* Expect that core-isa.h exists in OS/baremetal port */
#include_next <xtensa/config/core-isa.h>

#endif /* defined(_LIBC) || defined(_LIBM) || defined(_LIBGLOSS) */

#endif /* _XTENSA_CORE_CONFIGURATION_H */
