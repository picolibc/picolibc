/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#ifndef _FENV_H
#define _FENV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/fenv.h>

#ifndef FE_ALL_EXCEPT
#define FE_ALL_EXCEPT	0
#endif

/* Exception */
int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int feraiseexcept(int excepts);
int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fetestexcept(int excepts);

/* Rounding mode */
int fegetround(void);
int fesetround(int rounding_mode);

/* Float environment */
int fegetenv(fenv_t *envp);
int feholdexcept(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feupdateenv(const fenv_t *envp);

int feenableexcept(int);
int fedisableexcept(int);
int fegetexcept(void);

/*
 * Lastly, a FE_DFL_ENV macro must be defined, representing a pointer
 * to const fenv_t that contains the value of the default floating point
 * environment.
 *
 * NOTE: The extern'ed variable fe_default_env_p is an implementation
 *       detail of this stub.  FE_DFL_ENV must point to an instance of
 *       fenv_t with the default fenv_t. The format of fenv_t and where
 *       FE_DFL_ENV is are implementation specific.
 */
extern fenv_t _fe_dfl_env;
#define FE_DFL_ENV ((const fenv_t *) &_fe_dfl_env)

#ifdef __STDC_WANT_IEC_60559_BFP_EXT__

#ifndef FE_DFL_MODE
typedef struct { int round, except; } femode_t;
#define FE_DFL_MODE     ((femode_t *) 0)
#endif

int fegetmode(femode_t *modep);
int fesetmode(femode_t *modep);
int fesetexcept(int excepts);

#endif

#ifdef __cplusplus
}
#endif

#endif
