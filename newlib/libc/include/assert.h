/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
c) UNIX System Laboratories, Inc.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/*
	assert.h
*/

#include <sys/cdefs.h>

_BEGIN_STD_C

#undef assert

#ifdef NDEBUG           /* required by ANSI standard */
# define assert(__e) ((void)0)
#else
# ifndef _ASSERT_VERBOSE
# define assert(__e) ((__e) ? (void)0 : __assert_no_args ())
#else
# ifndef __ASSERT_FUNC
  /* Use g++'s demangled names in C++.  */
#  if defined __cplusplus && defined __GNUC__
#   define __ASSERT_FUNC __PRETTY_FUNCTION__

  /* C99 requires the use of __func__.  */
#  elif __STDC_VERSION__ >= 199901L
#   define __ASSERT_FUNC __func__

  /* Older versions of gcc don't have __func__ but can use __FUNCTION__.  */
#  elif __GNUC__ >= 2
#   define __ASSERT_FUNC __FUNCTION__

  /* failed to detect __func__ support.  */
#  else
#   define __ASSERT_FUNC ((char *) 0)
#  endif
# endif /* !__ASSERT_FUNC */

# define assert(__e) ((__e) ? (void)0 : __assert_func (__FILE__, __LINE__, \
						       __ASSERT_FUNC, #__e))
#endif

#endif /* !NDEBUG */

_Noreturn void __assert (const char *, const char *, int);

_Noreturn void __assert_func (const char *, int, const char *, const char *);

_Noreturn void __assert_no_args (void);

#if __STDC_VERSION__ >= 201112L && !defined __cplusplus
# define static_assert _Static_assert
#endif

_END_STD_C
