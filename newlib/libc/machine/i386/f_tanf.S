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
Fast version of tanf using Intel float instructions.

   float _f_tanf (float x);

Function calculates the tangent of x.
There is no error checking or setting of errno.
*/

	#include "i386mach.h"

	.global SYM (_f_tanf)
       SOTYPE_FUNCTION(_f_tanf)

SYM (_f_tanf):
	pushl ebp
	movl esp,ebp
	flds 8(ebp)
	fptan
	fincstp

	leave
	ret
