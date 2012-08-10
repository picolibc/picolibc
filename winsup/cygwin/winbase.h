/* winbase.h

   Copyright 2002, 2003, 2004, 2008 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include_next "winbase.h"

#ifndef _WINBASE2_H
#define _WINBASE2_H

extern __inline__ long
ilockincr (volatile long *m)
{
  register int __res;
  __asm__ __volatile__ ("\n\
	movl	$1,%0\n\
	lock	xadd %0,%1\n\
	inc	%0\n\
	": "=&r" (__res), "=m" (*m): "m" (*m): "cc");
  return __res;
}

extern __inline__ long
ilockdecr (volatile long *m)
{
  register int __res;
  __asm__ __volatile__ ("\n\
	movl	$0xffffffff,%0\n\
	lock	xadd %0,%1\n\
	dec	%0\n\
	": "=&r" (__res), "=m" (*m): "m" (*m): "cc");
  return __res;
}

extern __inline__ long
ilockexch (volatile long *t, long v)
{
  return
  ({
    register long ret __asm ("%eax");
    __asm __volatile ("\n"
	"1:	lock cmpxchgl %2, %1\n"
	"	jne  1b\n"
	: "=a" (ret), "=m" (*t)
	: "r" (v), "m" (*t), "0" (*t)
	: "memory");
    ret;
  });
}

extern __inline__ long
ilockcmpexch (volatile long *t, long v, long c)
{
  return
  ({
    register long ret __asm ("%eax");
    __asm __volatile ("lock cmpxchgl %2, %1"
	: "=a" (ret), "=m" (*t)
	: "r" (v), "m" (*t), "0" (c)
	: "memory");
    ret;
  });
}

#undef InterlockedIncrement
#define InterlockedIncrement ilockincr
#undef InterlockedDecrement
#define InterlockedDecrement ilockdecr
#undef InterlockedExchange
#define InterlockedExchange ilockexch
#undef InterlockedCompareExchange
#define InterlockedCompareExchange ilockcmpexch
#endif /*_WINBASE2_H*/
