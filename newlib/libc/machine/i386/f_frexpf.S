/*
 * ====================================================
 * Copyright (C) 1998 by Cygnus Solutions. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
Fast version of frexpf using Intel float instructions.

   float _f_frexpf (float x, int *exp);

Function splits x into y * 2 ** z.  It then
returns the value of y and updates *exp with z.
There is no error checking or setting of errno.
*/

	#include "i386mach.h"

	.global SYM (_f_frexpf)
       SOTYPE_FUNCTION(_f_frexpf)

SYM (_f_frexpf):
	pushl ebp
	movl esp,ebp
	flds 8(ebp)
	movl 12(ebp),eax

	fxtract
	fld1
	fchs
	fxch
	fscale
	fstp st1
	fxch
	fld1
	faddp
	fistpl 0(eax)

	leave
	ret
