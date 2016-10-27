/*-
 * Copyright (c) 2016 embedded brains GmbH
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SYS_TYPES_H
#error "must be included via <sys/types.h>"
#endif /* !_SYS_TYPES_H */

#if __BSD_VISIBLE

#ifndef _ACCMODE_T_DECLARED
typedef	__accmode_t	accmode_t;	/* access permissions */
#define	_ACCMODE_T_DECLARED
#endif

#ifndef _CAP_IOCTL_T_DECLARED
#define	_CAP_IOCTL_T_DECLARED
typedef	unsigned long	cap_ioctl_t;
#endif

#ifndef _CAP_RIGHTS_T_DECLARED
#define	_CAP_RIGHTS_T_DECLARED
struct cap_rights;

typedef	struct cap_rights	cap_rights_t;
#endif

typedef	const char *	c_caddr_t;	/* core address, pointer to const */

typedef	int		cpulevel_t;
typedef	int		cpusetid_t;
typedef	int		cpuwhich_t;

typedef	__fixpt_t	fixpt_t;	/* fixed point number */

#ifndef _LWPID_T_DECLARED
typedef	__lwpid_t	lwpid_t;	/* Thread ID (a.k.a. LWP) */
#define	_LWPID_T_DECLARED
#endif

#ifndef _RLIM_T_DECLARED
typedef	__rlim_t	rlim_t;		/* resource limit */
#define	_RLIM_T_DECLARED
#endif

typedef	__uintptr_t	segsz_t;	/* segment size (in pages) */

typedef	__uintptr_t	uintfptr_t;

typedef	__intptr_t	vm_ooffset_t;
typedef	__uintptr_t	vm_offset_t;
typedef	__uintptr_t	vm_paddr_t;
typedef	__uintptr_t	vm_pindex_t;
typedef	__uintptr_t	vm_size_t;

typedef	__uintmax_t	rman_res_t;

#ifdef _KERNEL
/* Header file provided outside of Newlib */
#include <machine/_kernel_types.h>
#endif

#endif /* __BSD_VISIBLE */
