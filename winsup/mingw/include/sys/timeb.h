/*
 * timeb.h
 *
 * Support for the UNIX System V ftime system call.
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

#ifndef	_TIMEB_H_
#define	_TIMEB_H_

/* All the headers include this file. */
#include <_mingw.h>

#ifndef	RC_INVOKED

/*
 * TODO: Structure not tested.
 */
struct _timeb
{
	long	time;
	short	millitm;
	short	timezone;
	short	dstflag;
};

#ifndef	_NO_OLDNAMES
/*
 * TODO: Structure not tested.
 */
struct timeb
{
	long	time;
	short	millitm;
	short	timezone;
	short	dstflag;
};
#endif


#ifdef	__cplusplus
extern "C" {
#endif

/* TODO: Not tested. */
_CRTIMP void __cdecl	_ftime (struct _timeb*);

#ifndef	_NO_OLDNAMES
_CRTIMP void __cdecl	ftime (struct timeb*);
#endif	/* Not _NO_OLDNAMES */

#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _TIMEB_H_ */
