#include_next "winbase.h"

#ifndef _WINBASE2_H
#define _WINBASE2_H

extern __inline__ long
ilockincr (long *m)
{
  register int __res;
  __asm__ __volatile__ ("\n\
	movl	$1,%0\n\
	lock	xadd %0,(%1)\n\
	inc	%0\n\
	": "=a" (__res), "=q" (m): "1" (m));
  return __res;
}

extern __inline__ long
ilockdecr (long *m)
{
  register int __res;
  __asm__ __volatile__ ("\n\
	movl	$0xffffffff,%0\n\
	lock	xadd %0,(%1)\n\
	dec	%0\n\
	": "=a" (__res), "=q" (m): "1" (m));
  return __res;
}

extern __inline__ long
ilockexch (long *t, long v)
{
  register int __res;
  __asm__ __volatile__ ("\n\
1:	lock	cmpxchgl %3,(%1)\n\
	jne 1b\n\
 	": "=a" (__res), "=q" (t): "1" (t), "q" (v), "0" (*t));
  return __res;
}

extern __inline__ long
ilockcmpexch (long *t, long v, long c)
{
  register int __res;
  __asm__ __volatile__ ("\n\
	lock cmpxchgl %3,(%1)\n\
	": "=a" (__res), "=q" (t) : "1" (t), "q" (v), "0" (c));
  return __res;
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
