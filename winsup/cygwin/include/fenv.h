/* fenv.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _FENV_H
#define _FENV_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Primary sources:

     The Open Group Base Specifications Issue 6:
   http://www.opengroup.org/onlinepubs/000095399/basedefs/fenv.h.html

     C99 Language spec (draft n1256):
   <url unknown>

     Intel® 64 and IA-32 Architectures Software Developer’s Manuals:
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

/*  This is not defined by Posix, but since x87 supports it we provide
   a definition according to the same naming scheme used above.  */
#define FE_DENORMAL	(1 << 1)

/*  The <fenv.h> header shall define the following constant, which is
   simply the bitwise-inclusive OR of all floating-point exception
   constants defined above:  */

#define FE_ALL_EXCEPT (FE_DIVBYZERO | FE_INEXACT | FE_INVALID \
			| FE_OVERFLOW | FE_UNDERFLOW | FE_DENORMAL)

/*  The <fenv.h> header shall define the following constants if and only
   if the implementation supports getting and setting the represented
   rounding direction by means of the fegetround() and fesetround()
   functions. Each expands to an integer constant expression whose values
   are distinct non-negative vales.  */

#define FE_DOWNWARD	(1)
#define FE_TONEAREST	(0)
#define FE_TOWARDZERO	(3)
#define FE_UPWARD	(2)

/*  Precision bit values.  Not defined by Posix, but follow logically.  */
#define FE_SINGLEPREC	(0)
#define FE_RESERVEDPREC	(1)
#define FE_DOUBLEPREC	(2)
#define FE_EXTENDEDPREC	(3)

/*  The <fenv.h> header shall define the following constant, which
   represents the default floating-point environment (that is, the one
   installed at program startup) and has type pointer to const-qualified
   fenv_t. It can be used as an argument to the functions within the
   <fenv.h> header that manage the floating-point environment.  */

extern const fenv_t *_fe_dfl_env;
#define FE_DFL_ENV (_fe_dfl_env)

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
#endif /* __GNU_VISIBLE */


/*  The following shall be declared as functions and may also be
   defined as macros. Function prototypes shall be provided.  */
extern int feclearexcept (int excepts);
extern int fegetexceptflag (fexcept_t *flagp, int excepts);
extern int feraiseexcept (int excepts);
extern int fesetexceptflag (const fexcept_t *flagp, int excepts);
extern int fetestexcept (int excepts);
extern int fegetround (void);
extern int fesetround (int round);
extern int fegetenv (fenv_t *envp);
extern int feholdexcept (fenv_t *envp);
extern int fesetenv (const fenv_t *envp);
extern int feupdateenv (const fenv_t *envp);

/* These are not defined in Posix, but make sense by obvious extension.  */
extern int fegetprec (void);
extern int fesetprec (int prec);

/* This is Cygwin-custom, not from the standard, for use in the Cygwin CRT.  */
extern void _feinitialise (void);

/* These are GNU extensions defined in glibc.  */
extern int feenableexcept (int excepts);
extern int fedisableexcept (int excepts);
extern int fegetexcept (void);


#ifdef __cplusplus
}
#endif

#endif /* _FENV_H */
