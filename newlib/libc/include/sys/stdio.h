#ifndef _NEWLIB_STDIO_H
#define _NEWLIB_STDIO_H

/* Internal locking macros, used to protect stdio functions.  In the
   general case, expand to nothing. */
#if !defined(_flockfile)
#  define _flockfile(fp)
#endif

#if !defined(_funlockfile)
#  define _funlockfile(fp)
#endif

#endif /* _NEWLIB_STDIO_H */
