#ifndef _NEWLIB_STDIO_H
#define _NEWLIB_STDIO_H

#include <sys/lock.h>
#include <sys/reent.h>

/* Internal locking macros, used to protect stdio functions.  In the
   general case, expand to nothing. */
#if !defined(_flockfile)
#ifndef __SINGLE_THREAD__
#  define _flockfile(fp) __lock_acquire_recursive(fp->_lock)
#else
#  define _flockfile(fp)
#endif
#endif

#if !defined(_funlockfile)
#ifndef __SINGLE_THREAD__
#  define _funlockfile(fp) __lock_release_recursive(fp->_lock)
#else
#  define _funlockfile(fp)
#endif
#endif

#endif /* _NEWLIB_STDIO_H */
