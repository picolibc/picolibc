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
Fast version of log using Intel float instructions.

   double _f_log (double x);

Function calculates the log base e of x.
There is no error checking or setting of errno. 
*/

	#include "i386mach.h"

	.global SYM (_f_log)
       SOTYPE_FUNCTION(_f_log)

SYM (_f_log):
	pushl ebp
	movl esp,ebp

	fld1
	fldl2e
	fdivrp
	fldl 8(ebp)
	fyl2x

	leave
	ret
