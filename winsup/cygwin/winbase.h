/* winbase.h

   Copyright 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2012 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include_next "winbase.h"

#ifndef _WINBASE2_H
#define _WINBASE2_H

extern __inline__ LONG
ilockcmpexch (volatile LONG *t, LONG v, LONG c)
{
  return
  ({
    register LONG ret __asm ("%eax");
    __asm __volatile ("lock cmpxchgl %2, %1"
	: "=a" (ret), "=m" (*t)
	: "r" (v), "m" (*t), "0" (c)
	: "memory");
    ret;
  });
}

#undef InterlockedCompareExchange
#define InterlockedCompareExchange ilockcmpexch
#undef InterlockedCompareExchangePointer

#ifdef __x86_64__
extern __inline__ LONGLONG
ilockcmpexch64 (volatile LONGLONG *t, LONGLONG v, LONGLONG c)
{
  return
  ({
    register LONGLONG ret __asm ("%rax");
    __asm __volatile ("lock cmpxchgq %2, %1"
	: "=a" (ret), "=m" (*t)
	: "r" (v), "m" (*t), "0" (c)
	: "memory");
    ret;
  });
}

#define InterlockedCompareExchange64 ilockcmpexch64
#define InterlockedCompareExchangePointer(d,e,c) \
    (PVOID)InterlockedCompareExchange64((LONGLONG volatile *)(d),(LONGLONG)(e),(LONGLONG)(c))

#else

#define InterlockedCompareExchangePointer(d,e,c) \
    (PVOID)InterlockedCompareExchange((LONG volatile *)(d),(LONG)(e),(LONG)(c))

#endif /* !__x86_64 */
#endif /*_WINBASE2_H*/
