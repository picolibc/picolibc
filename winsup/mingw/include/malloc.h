/*
 * malloc.h
 *
 * Support for programs which want to use malloc.h to get memory management
 * functions. Unless you absolutely need some of these functions and they are
 * not in the ANSI headers you should use the ANSI standard header files
 * instead.
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

#ifndef _MALLOC_H_
#define _MALLOC_H_

/* All the headers include this file. */
#include <_mingw.h>

#include <stdlib.h>

#ifndef RC_INVOKED

/*
 * The structure used to walk through the heap with _heapwalk.
 */
typedef	struct _heapinfo
{
	int*	_pentry;
	size_t	_size;
	int	_useflag;
} _HEAPINFO;

/* Values for _heapinfo.useflag */
#define _USEDENTRY 0
#define _FREEENTRY 1

#ifdef	__cplusplus
extern "C" {
#endif
/*
   The _heap* memory allocation functions are supported on NT
   but not W9x. On latter, they always set errno to ENOSYS.
*/
_CRTIMP int __cdecl _heapwalk (_HEAPINFO*);
#ifdef __GNUC__
#define _alloca(x) __builtin_alloca((x))
#endif

#ifndef	_NO_OLDNAMES
_CRTIMP int __cdecl heapwalk (_HEAPINFO*);
#ifdef __GNUC__
#define alloca(x) __builtin_alloca((x))
#endif
#endif	/* Not _NO_OLDNAMES */

_CRTIMP int __cdecl _heapchk (void);	/* Verify heap integrety. */
_CRTIMP int __cdecl _heapmin (void);	/* Return unused heap to the OS. */
_CRTIMP int __cdecl _heapset (unsigned int);

_CRTIMP size_t __cdecl _msize (void*);
_CRTIMP size_t __cdecl _get_sbh_threshold (void); 
_CRTIMP int __cdecl _set_sbh_threshold (size_t);
_CRTIMP void* __cdecl _expand (void*, size_t); 

#ifdef __cplusplus
}
#endif

#endif	/* RC_INVOKED */

#endif /* Not _MALLOC_H_ */
