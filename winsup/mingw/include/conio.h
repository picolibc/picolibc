/*
 * conio.h
 *
 * Low level console I/O functions. Pretty please try to use the ANSI
 * standard ones if you are writing new code.
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
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision$
 * $Author$
 * $Date$
 *
 */

#ifndef	_CONIO_H_
#define	_CONIO_H_

/* All the headers include this file. */
#include <_mingw.h>

#ifndef RC_INVOKED

#ifdef	__cplusplus
extern "C" {
#endif

_CRTIMP char* __cdecl	_cgets (char*);
_CRTIMP int __cdecl	_cprintf (const char*, ...);
_CRTIMP int __cdecl	_cputs (const char*);
_CRTIMP int __cdecl	_cscanf (char*, ...);

_CRTIMP int __cdecl	_getch (void);
_CRTIMP int __cdecl	_getche (void);
_CRTIMP int __cdecl	_kbhit (void);
_CRTIMP int __cdecl	_putch (int);
_CRTIMP int __cdecl	_ungetch (int);


#ifndef	_NO_OLDNAMES

_CRTIMP int __cdecl	getch (void);
_CRTIMP int __cdecl	getche (void);
_CRTIMP int __cdecl	kbhit (void);
_CRTIMP int __cdecl	putch (int);
_CRTIMP int __cdecl	ungetch (int);

#endif	/* Not _NO_OLDNAMES */


#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _CONIO_H_ */
