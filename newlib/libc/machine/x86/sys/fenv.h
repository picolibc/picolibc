/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2010-2019 Red Hat, Inc.
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

#ifndef _SYS_FENV_H
#define _SYS_FENV_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _SOFT_FLOAT
typedef int fenv_t;
typedef int fexcept_t;
#else

/* Primary sources:

     The Open Group Base Specifications Issue 6:
   http://www.opengroup.org/onlinepubs/000095399/basedefs/fenv.h.html

     C99 Language spec (draft n1256):
   <url unknown>

     Intel(R) 64 and IA-32 Architectures Software Developer's Manuals:
   http://www.intel.com/products/processor/manuals/

     GNU C library manual pages:
   http://www.gnu.org/software/libc/manual/html_node/Control-Functions.html
   http://www.gnu.org/software/libc/manual/html_node/Rounding.html
   http://www.gnu.org/software/libc/manual/html_node/FP-Exceptions.html
   http://www.gnu.org/software/libc/manual/html_node/Status-bit-operations.html

     Linux online man page(s):
   http://linux.die.net/man/3/fegetexcept

    The documentation quotes these sources for reference.  All definitions and
   code have been developed solely based on the information from these specs.

*/

/*  Represents the entire floating-point environment. The floating-point
   environment refers collectively to any floating-point status flags and
   control modes supported by the implementation.
    In this implementation, the struct contains the state information from
   the fstenv/fnstenv instructions and a copy of the SSE MXCSR, since GCC
   uses SSE for a lot of floating-point operations.  (Cygwin assumes i686
   or above these days, as does the compiler.)  */

typedef struct _fenv_t
{
  struct _fpu_env_info {
    unsigned int _fpu_cw;	/* low 16 bits only. */
    unsigned int _fpu_sw;	/* low 16 bits only. */
    unsigned int _fpu_tagw;	/* low 16 bits only. */
    unsigned int _fpu_ipoff;
    unsigned int _fpu_ipsel;
    unsigned int _fpu_opoff;
    unsigned int _fpu_opsel;	/* low 16 bits only. */
  } _fpu;
  unsigned int _sse_mxcsr;
} fenv_t;

/*  Represents the floating-point status flags collectively, including
   any status the implementation associates with the flags. A floating-point
   status flag is a system variable whose value is set (but never cleared)
   when a floating-point exception is raised, which occurs as a side effect
   of exceptional floating-point arithmetic to provide auxiliary information.
    A floating-point control mode is a system variable whose value may be
   set by the user to affect the subsequent behavior of floating-point
   arithmetic. */

typedef __uint32_t fexcept_t;

/*  The <fenv.h> header shall define the following constants if and only
   if the implementation supports the floating-point exception by means
   of the floating-point functions feclearexcept(), fegetexceptflag(),
   feraiseexcept(), fesetexceptflag(), and fetestexcept(). Each expands to
   an integer constant expression with values such that bitwise-inclusive
   ORs of all combinations of the constants result in distinct values.  */

#define FE_DIVBYZERO	(1 << 2)
#define FE_INEXACT	(1 << 5)
#define FE_INVALID	(1 << 0)
#define FE_OVERFLOW	(1 << 3)
#define FE_UNDERFLOW	(1 << 4)

/*  The <fenv.h> header shall define the following constant, which is
   simply the bitwise-inclusive OR of all floating-point exception
   constants defined above:  */

/* in agreement w/ Linux the subnormal exception will always be masked */
#define FE_ALL_EXCEPT \
  (FE_INEXACT | FE_UNDERFLOW | FE_OVERFLOW | FE_DIVBYZERO | FE_INVALID)

/*  The <fenv.h> header shall define the following constants if and only
   if the implementation supports getting and setting the represented
   rounding direction by means of the fegetround() and fesetround()
   functions. Each expands to an integer constant expression whose values
   are distinct non-negative vales.  */

#define FE_DOWNWARD	(1)
#define FE_TONEAREST	(0)
#define FE_TOWARDZERO	(3)
#define FE_UPWARD	(2)

/* Only Solaris and QNX implement fegetprec/fesetprec.  As Solaris, use the
   values defined by http://www.open-std.org/jtc1/sc22//WG14/www/docs/n752.htm
   QNX defines different values. */
#if __MISC_VISIBLE
#define FE_FLTPREC	(0)
#define FE_DBLPREC	(2)
#define FE_LDBLPREC	(3)
#endif
/*  Additional implementation-defined environments, with macro
   definitions beginning with FE_ and an uppercase letter,and having
   type "pointer to const-qualified fenv_t",may also be specified by
   the implementation.  */

#if __GNU_VISIBLE
/*  If possible, the GNU C Library defines a macro FE_NOMASK_ENV which
   represents an environment where every exception raised causes a trap
   to occur. You can test for this macro using #ifdef. It is only defined
   if _GNU_SOURCE is defined.  */
extern const fenv_t *_fe_nomask_env;
#define FE_NOMASK_ENV (_fe_nomask_env)
#endif

#ifdef __CYGWIN__

#if __MISC_VISIBLE
int fegetprec (void);
int fesetprec (int __prec);
#endif

#endif /* __CYGWIN__ */

#endif /* !_SOFT_FLOAT */

#ifdef _SOFT_FLOAT

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define	__declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#include <machine/fenv-softfloat.h>
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* _FENV_H */
