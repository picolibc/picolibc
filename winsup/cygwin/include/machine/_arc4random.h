/* machine/_arc4random.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MACHINE_ARC4RANDOM_H
#define _MACHINE_ARC4RANDOM_H

extern int __isthreaded;

#define _ARC4_LOCK_INIT	__LOCK_INIT(static, _arc4random_mutex);

#define _ARC4_LOCK()				\
        do {					\
	  if (__isthreaded)			\
	    __lock_acquire (_arc4random_mutex);	\
        } while (0)

#define _ARC4_UNLOCK()				\
        do {					\
	  if (__isthreaded)			\
	    __lock_release (_arc4random_mutex);	\
        } while (0)

#endif /* _MACHINE_ARC4RANDOM_H */ 
