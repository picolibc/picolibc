/* Copyright (c) 2011 Yaakov Selkowitz <yselkowi@redhat.com> */
/*
 * stdio_ext.h
 *
 * Definitions for I/O internal operations, originally from Solaris.
 */

#ifndef _STDIO_EXT_H_
#define _STDIO_EXT_H_

#include <stdio.h>

#define	FSETLOCKING_QUERY	0
#define	FSETLOCKING_INTERNAL	1
#define	FSETLOCKING_BYCALLER	2

_BEGIN_STD_C

void	 __fpurge (FILE *);
int	 __fsetlocking (FILE *, int);

/* TODO:

   void _flushlbf (void);
*/

#ifdef __declare_extern_inline

__declare_extern_inline(size_t)
__fbufsize (FILE *__fp) { return (size_t) __fp->_bf._size; }

__declare_extern_inline(int)
__freading (FILE *__fp) { return (__fp->_flags & __SRD) != 0; }

__declare_extern_inline(int)
__fwriting (FILE *__fp) { return (__fp->_flags & __SWR) != 0; }

__declare_extern_inline(int)
__freadable (FILE *__fp) { return (__fp->_flags & (__SRD | __SRW)) != 0; }

__declare_extern_inline(int)
__fwritable (FILE *__fp) { return (__fp->_flags & (__SWR | __SRW)) != 0; }

__declare_extern_inline(int)
__flbf (FILE *__fp) { return (__fp->_flags & __SLBF) != 0; }

__declare_extern_inline(size_t)
__fpending (FILE *__fp) { return __fp->_p - __fp->_bf._base; }

#endif

size_t	 __fbufsize (FILE *);
int	 __freading (FILE *);
int	 __fwriting (FILE *);
int	 __freadable (FILE *);
int	 __fwritable (FILE *);
int	 __flbf (FILE *);
size_t	 __fpending (FILE *);

_END_STD_C

#endif /* _STDIO_EXT_H_ */
