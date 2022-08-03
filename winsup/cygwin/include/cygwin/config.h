/* cygwin/config.h header file for Cygwin.

   This wraps Cygwin configuration setting which were in newlib's
   sys/config.h before.  This way we can manaage our configuration
   setting without bothering newlib.
   Written by C. Vinschen.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif
#define _CYGWIN_CONFIG_H

#define __DYNAMIC_REENT__

/* The following provides an inline version of __getreent() for newlib,
   which will be used throughout the library whereever there is a _r
   version of a function that takes _REENT.  This saves the overhead
   of a function call for what amounts to a simple computation.

   This is the allocation size of the TLS area on the stack.  Parts of
   the stack are in use by the OS, so we need to go a bit higher than
   what's actually required by the cygtls struct.  The _reent struct is
   right at the beginning of struct cygtls and always has to be. */
#define __CYGTLS_PADSIZE__ 12800	/* Must be 16-byte aligned */

#if defined (_LIBC) || defined (__INSIDE_CYGWIN__)

__attribute__((__gnu_inline__))
extern inline struct _reent *__getreent (void)
{
  register char *ret;
#ifdef __x86_64__
  __asm __volatile__ ("movq %%gs:8,%0" : "=r" (ret));
#else
#error unimplemented for this target
#endif
  return (struct _reent *) (ret - __CYGTLS_PADSIZE__);
}
#endif /* _LIBC || __INSIDE_CYGWIN__ */

#define _SYMSTR(x)	#x

#define __FILENAME_MAX__ 4096	/* Keep in sync with PATH_MAX in limits.h. */

/* The following block of macros is required to build newlib correctly for
   Cygwin.  Changing them in applications has no or not the desired effect.
   Just leave them alone. */
#define _READ_WRITE_RETURN_TYPE _ssize_t
#define _READ_WRITE_BUFSIZE_TYPE size_t
#define __LINUX_ERRNO_EXTENSIONS__ 1
#define _MB_EXTENDED_CHARSETS_ALL 1
#define __HAVE_LOCALE_INFO__ 1
#define __HAVE_LOCALE_INFO_EXTENDED__ 1
#define _WANT_C99_TIME_FORMATS 1
#define _GLIBC_EXTENSION 1
#define _STDIO_BSD_SEMANTICS 1
#define __TM_GMTOFF tm_gmtoff
#define __TM_ZONE   tm_zone
#define _USE_LONG_TIME_T 1
#define _REENT_BACKWARD_BINARY_COMPAT 1

#if defined(__INSIDE_CYGWIN__) || defined(_LIBC)
#define __EXPORT __declspec(dllexport)
#define __IMPORT
#else
#define __EXPORT
#define __IMPORT __declspec(dllimport)
#endif

#ifndef __WCHAR_MAX__
#define __WCHAR_MAX__ 0xffffu
#endif

#define DEFAULT_LOCALE "C.UTF-8"

#ifdef __cplusplus
}
#endif
#endif /* _CYGWIN_CONFIG_H */
