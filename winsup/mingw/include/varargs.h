/*
 * varargs.h
 *
 * Old, non-ANSI facilities for stepping through a list of function
 * arguments of an unknown number and type.
 * TODO: Has not been tested. Essentially it copies the GCC version.
 *
 * NOTE: I believe GCC supplies a version of this header as well (in
 *       addition to stdarg.h and others). The GCC version is more
 *       complex, to deal with many alternate systems, but it is
 *       probably more trustworthy overall. It would probably be
 *       better to use the GCC version.
 *
 * NOTE: These are incompatible with the versions in stdarg.h and should
 *       NOT be mixed! All new code should use the ANSI compatible versions.
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Colin Peters <colin@bird.fu.is.saga-u.ac.jp>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision$
 * $Author$
 * $Date$
 *
 */

#ifndef	__STRICT_ANSI__

#ifndef _VARARGS_H_
#define _VARARGS_H_

/* All the headers include this file. */
#include <_mingw.h>

/* 
 * I was told that Win NT likes this.
 */
#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
#endif

#ifndef RC_INVOKED

#ifndef _VA_LIST
#define	_VA_LIST
typedef char* va_list;
#endif

/*
 * Amount of space required in an argument list (ie. the stack) for an
 * argument of type t.
 */
#define __va_argsiz(t)	\
	(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define	va_alist	__builtin_va_alist

/*
 * Used in old style argument lists IIRC. The ellipsis forces the compiler
 * to realize this is a vararg function.
 */
#define va_dcl		int __builtin_va_alist; ...

#define va_start(ap)	\
	((ap) = ((va_list) &__builtin_va_alist))
#define va_end(ap)	((void)0)


/*
 * Increment ap to the next argument in the list while returing a
 * pointer to what ap pointed to first, which is of type t.
 *
 * We cast to void* and then to t* because this avoids a warning about
 * increasing the alignment requirement.
 */

#define va_arg(ap, t)					\
	 (((ap) = (ap) + __va_argsiz(t)),		\
	  *((t*) (void*) ((ap) - __va_argsiz(t))))


#endif	/* Not RC_INVOKED */

#endif	/* Not _VARARGS_H_ */

#endif	/* Not __STRICT_ANSI__ */

