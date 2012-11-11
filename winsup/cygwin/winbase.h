/* winbase.h

   Copyright 2002, 2003, 2004, 2008, 2009, 2012 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include_next "winbase.h"

#ifndef _WINBASE2_H
#define _WINBASE2_H

#ifndef __x86_64__
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
#define InterlockedCompareExchangePointer(d,e,c) \
    (PVOID)InterlockedCompareExchange((LONG volatile *)(d),(LONG)(e),(LONG)(c))
#endif /* !__x86_64 */
#endif /*_WINBASE2_H*/
