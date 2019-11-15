/* SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * do_global_dtors - invoke global destructors
 *
 * Copyright (c) 2018-2019 Dimitar Dimitrov <dimitar@dinux.eu>
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <_ansi.h>
#include "syscall.h"

extern void *__fini_array_start;
extern void *__fini_array_end;

/*
 * _do_global_dtors
 */
void
_do_global_dtors (void)
{
    /* ABI dictates pointers in init/fini arrays are 16-bit.  */
    const uint16_t *end = (uint16_t *)&__fini_array_end;
    const uint16_t *begin = (uint16_t *)&__fini_array_start;
    const uint16_t *p;
    void (**dtor) (void);

    /* call destructors in reverse order */
    for (p = end; p > begin; ) {
	p--;
	dtor = (void *)p;
	(*dtor) ();
    }
}
