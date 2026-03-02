/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
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
        setjmp.h
        stubs for future use.
*/

#ifndef _SETJMP_H_
#define _SETJMP_H_

#include <sys/cdefs.h>
#include <sys/_sigset.h>
#include <machine/setjmp.h>

_BEGIN_STD_C

typedef struct {
    int        savesigs;
    __sigset_t sigs;
    jmp_buf    jmpb;
} sigjmp_buf[1];

__noreturn void     longjmp(jmp_buf __jmpb, int __retval);
__returns_twice int setjmp(jmp_buf __jmpb);
void                __sigjmp_getsigs(sigjmp_buf jmpb, int savesigs);
void                __sigjmp_setsigs(sigjmp_buf jmpb);

#define sigsetjmp(__e, __s)  (__sigjmp_getsigs((__e), (__s)), setjmp((__e)[0].jmpb))
#define siglongjmp(__e, __v) (__sigjmp_setsigs((__e)), longjmp((__e)[0].jmpb, (__v)))

_END_STD_C

#endif /* _SETJMP_H_ */
