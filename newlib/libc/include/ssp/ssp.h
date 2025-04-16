/*	$NetBSD: ssp.h,v 1.13 2015/09/03 20:43:47 plunky Exp $	*/

/*-
 * Copyright (c) 2006, 2011 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _SSP_SSP_H_
#define _SSP_SSP_H_

#include <sys/cdefs.h>

/* __ssp_real is used by the implementation in libc */
#if __SSP_FORTIFY_LEVEL == 0
#define __ssp_real_(fun)	fun
#else
#define __ssp_real_(fun)	__ssp_real_ ## fun
#endif
#define __ssp_real(fun)		__ssp_real_(fun)

#if __SSP_FORTIFY_LEVEL > 2
#define __ssp_bos(ptr) __builtin_dynamic_object_size(ptr, 1)
#define __ssp_bos0(ptr) __builtin_dynamic_object_size(ptr, 0)
#define __ssp_bos_known(ptr) \
       (__builtin_object_size(ptr, 0) != (size_t)-1 \
       || !__builtin_constant_p(__ssp_bos(ptr)))
#else
#define __ssp_bos(ptr) __builtin_object_size(ptr, __SSP_FORTIFY_LEVEL > 1)
#define __ssp_bos0(ptr) __builtin_object_size(ptr, 0)
#define __ssp_bos_known(ptr) (__ssp_bos0(ptr) != (size_t)-1)
#endif

#define __ssp_check(buf, len, bos) \
	if (__ssp_bos_known(buf) && len > bos(buf)) \
		__chk_fail()
#define __ssp_decl(rtype, fun, args) \
rtype __ssp_real_(fun) args __asm__(__ASMNAME(#fun)); \
__declare_extern_inline(rtype) fun args
#define __ssp_redirect_raw(rtype, fun, args, call, cond, bos) \
__ssp_decl(rtype, fun, args) \
{ \
	if (cond) \
		__ssp_check(__buf, __len, bos); \
	return __ssp_real_(fun) call; \
}

#define __ssp_redirect(rtype, fun, args, call) \
    __ssp_redirect_raw(rtype, fun, args, call, 1, __ssp_bos)
#define __ssp_redirect0(rtype, fun, args, call) \
    __ssp_redirect_raw(rtype, fun, args, call, 1, __ssp_bos0)

#define __ssp_overlap(a, b, l) \
    (((a) <= (b) && (b) < (a) + (l)) || ((b) <= (a) && (a) < (b) + (l)))

_BEGIN_STD_C
void __stack_chk_fail(void) __noreturn;
void __chk_fail(void) __noreturn;
void set_fortify_handler (void (*handler) (int sig));
_END_STD_C

#endif /* _SSP_SSP_H_ */
