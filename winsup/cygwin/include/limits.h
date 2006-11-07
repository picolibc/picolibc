/* limits.h

   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _LIMITS_H___

#include <features.h>

#ifndef _MACH_MACHLIMITS_H_

/* _MACH_MACHLIMITS_H_ is used on OSF/1.  */
#define _LIMITS_H___
#define _MACH_MACHLIMITS_H_


/* Numerical limits */

/* Number of bits in a `char'.  */
#undef CHAR_BIT
#define CHAR_BIT 8

/* Maximum length of a multibyte character.  */
#ifndef MB_LEN_MAX
#define MB_LEN_MAX 1
#endif

/* Minimum and maximum values a `signed char' can hold.  */
#undef SCHAR_MIN
#define SCHAR_MIN (-128)
#undef SCHAR_MAX
#define SCHAR_MAX 127

/* Maximum value an `unsigned char' can hold.  (Minimum is 0).  */
#undef UCHAR_MAX
#define UCHAR_MAX 255

/* Minimum and maximum values a `char' can hold.  */
#ifdef __CHAR_UNSIGNED__
#undef CHAR_MIN
#define CHAR_MIN 0
#undef CHAR_MAX
#define CHAR_MAX 255
#else
#undef CHAR_MIN
#define CHAR_MIN (-128)
#undef CHAR_MAX
#define CHAR_MAX 127
#endif

/* Minimum and maximum values a `signed short int' can hold.  */
#undef SHRT_MIN
#define SHRT_MIN (-32768)
#undef SHRT_MAX
#define SHRT_MAX 32767

/* Maximum value an `unsigned short int' can hold.  (Minimum is 0).  */
#undef USHRT_MAX
#define USHRT_MAX 65535

/* Minimum and maximum values a `signed int' can hold.  */
#ifndef __INT_MAX__
#define __INT_MAX__ 2147483647
#endif
#undef INT_MIN
#define INT_MIN (-INT_MAX-1)
#undef INT_MAX
#define INT_MAX __INT_MAX__

/* Maximum value an `unsigned int' can hold.  (Minimum is 0).  */
#undef UINT_MAX
#define UINT_MAX (INT_MAX * 2U + 1)

/* Minimum and maximum values a `signed long int' can hold.
   (Same as `int').  */
#ifndef __LONG_MAX__
#ifndef __alpha__
#define __LONG_MAX__ 2147483647L
#else
#define __LONG_MAX__ 9223372036854775807L
# endif /* __alpha__ */
#endif
#undef LONG_MIN
#define LONG_MIN (-LONG_MAX-1)
#undef LONG_MAX
#define LONG_MAX __LONG_MAX__

/* Maximum value an `unsigned long int' can hold.  (Minimum is 0).  */
#undef ULONG_MAX
#define ULONG_MAX (LONG_MAX * 2UL + 1)

/* Minimum and maximum values a `signed long long int' can hold.  */
#ifndef __LONG_LONG_MAX__
#define __LONG_LONG_MAX__ 9223372036854775807LL
#endif

#if defined (__GNU_LIBRARY__) ? defined (__USE_GNU) : !defined (__STRICT_ANSI__)
#undef LONG_LONG_MIN
#define LONG_LONG_MIN (-LONG_LONG_MAX-1)
#undef LONG_LONG_MAX
#define LONG_LONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
#undef ULONG_LONG_MAX
#define ULONG_LONG_MAX (LONG_LONG_MAX * 2ULL + 1)
#endif

/* Minimum and maximum values a `signed long long int' can hold.  */
#undef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)
#undef LLONG_MAX
#define LLONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
#undef ULLONG_MAX
#define ULLONG_MAX (LLONG_MAX * 2ULL + 1)

/* Maximum size of ssize_t */
#undef SSIZE_MAX
#define SSIZE_MAX (__LONG_MAX__)


/* Runtime Invariant Values */

/* Maximum number of bytes in arguments and environment passed in an exec
   call.  32000 is the safe value used for Windows processes when called
   from Cygwin processes. */
#undef ARG_MAX
#define ARG_MAX 32000

/* Maximum number of simultaneous processes per real user ID. */
#undef CHILD_MAX
#define CHILD_MAX 256

/* Maximum length of a host name. */
#undef HOST_NAME_MAX
#define HOST_NAME_MAX 255

/* Maximum number of iovcnt in a writev (an arbitrary number) */
#undef IOV_MAX
#define IOV_MAX 1024

/* Maximum number of characters in a login name. */
#undef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 256	/* equal to UNLEN defined in w32api/lmcons.h */

/* # of open files per process. Actually it can be more since Cygwin
   grows the dtable as necessary. We define a reasonable limit here
   which is returned by getdtablesize(), sysconf(_SC_OPEN_MAX) and
   getrlimit(RLIMIT_NOFILE). */
#undef OPEN_MAX
#define OPEN_MAX 256

/* Size in bytes of a page. */
#undef PAGESIZE
#undef PAGE_SIZE
#define PAGESIZE 65536
#define PAGE_SIZE PAGESIZE

/* Maximum number of realtime signals reserved for application use. */
/* FIXME: We only support one realtime signal but _POSIX_RTSIG_MAX is 8. */
#undef RTSIG_MAX
#define RTSIG_MAX 1

/* Number of streams that one process can have open at one time. */
#undef STREAM_MAX
#define STREAM_MAX 20

/* Maximum number of nested symbolic links. */
#undef SYMLOOP_MAX
#define SYMLOOP_MAX 10

/* Maximum number of timer expiration overruns. */
#undef TIMER_MAX
#define TIMER_MAX 32

/* Maximum number of characters in a tty name. */
#undef TTY_NAME_MAX
#define TTY_NAME_MAX 12


/* Pathname Variable Values */

/* Minimum bits needed to represent the maximum size of a regular file. */
#undef FILESIZEBITS
#define FILESIZEBITS 64

/* Maximum number of hardlinks to a file. */
#undef LINK_MAX
#define LINK_MAX 1024

/* Maximum number of bytes in a terminal canonical input line. */
#undef MAX_CANON
#define MAX_CANON 255

/* Minimum number of bytes available in a terminal input queue. */
#undef MAX_INPUT
#define MAX_INPUT 255

/* Maximum length of a path component. */
#undef NAME_MAX
#define NAME_MAX 255

/* Maximum length of a path */
#undef PATH_MAX
#define PATH_MAX 260

/* # of bytes in a pipe buf. This is the max # of bytes which can be
   written to a pipe in one atomic operation. */
#undef PIPE_BUF
#define PIPE_BUF 4096


/* Runtime Increasable Values */

/* Maximum obase values allowed by the bc utility. */
#undef BC_BASE_MAX
#define BC_BASE_MAX 99

/* Maximum number of elements permitted in an array by the bc utility. */
#undef BC_DIM_MAX
#define BC_DIM_MAX 2048

/* Maximum scale value allowed by the bc utility. */
#undef BC_SCALE_MAX
#define BC_SCALE_MAX 99

/* Maximum length of a string constant accepted by the bc utility. */
#undef BC_STRING_MAX
#define BC_STRING_MAX 1000

/* Maximum number of weights that can be assigned to an entry of the
   LC_COLLATE order keyword in the locale definition file. */
/* FIXME: We don't support this at all right now, so this value is
   misleading at best.  It's also lower than _POSIX2_COLL_WEIGHTS_MAX
   which is not good.  So, for now we deliberately not define it even
   though it was defined in the former syslimits.h file. */
#if 0
#undef COLL_WEIGHTS_MAX
#define COLL_WEIGHTS_MAX 0
#endif

/* Maximum number of expressions that can be nested within parentheses
   by the expr utility. */
#undef EXPR_NEST_MAX
#define EXPR_NEST_MAX 32

/* Maximum bytes of a text utility's input line */
#undef LINE_MAX
#define LINE_MAX 2048

/* Max num groups for a user, value taken from NT documentation */
/* Must match <sys/param.h> NGROUPS */
#undef NGROUPS_MAX
#define NGROUPS_MAX 16

/* Maximum number of repeated occurrences of a regular expression when
   using the interval notation \{m,n\} */
#undef RE_DUP_MAX
#define RE_DUP_MAX 255


/* Minimum Values */

/* POSIX values */
/* These should never vary from one system type to another */
/* They represent the minimum values that POSIX systems must support.
   POSIX-conforming apps must not require larger values. */
#define	_POSIX_ARG_MAX		4096
#define _POSIX_CHILD_MAX	6
#define _POSIX_HOST_NAME_MAX	255
#define _POSIX_LINK_MAX		8
#define _POSIX_LOGIN_NAME_MAX	9
#define _POSIX_MAX_CANON	255
#define _POSIX_MAX_INPUT	255
#define _POSIX_NAME_MAX		14
#define _POSIX_NGROUPS_MAX	0
#define _POSIX_OPEN_MAX		16
#define _POSIX_PATH_MAX		255
#define _POSIX_PIPE_BUF		512
#define _POSIX_RE_DUP_MAX	255
#define _POSIX_RTSIG_MAX	8
#define _POSIX_SSIZE_MAX	32767
#define _POSIX_STREAM_MAX	8
#define _POSIX_SYMLINK_MAX	255
#define _POSIX_SYMLOOP_MAX	8
#define _POSIX_TIMER_MAX	32
#define _POSIX_TTY_NAME_MAX	9
#define _POSIX_TZNAME_MAX       3

#define _POSIX2_BC_BASE_MAX	99
#define _POSIX2_BC_DIM_MAX	2048
#define _POSIX2_BC_SCALE_MAX	99
#define _POSIX2_BC_STRING_MAX	1000
#if 0	/* See comment about COLL_WEIGHTS_MAX above. */
#define _POSIX2_COLL_WEIGHTS_MAX	2
#endif
#define _POSIX2_EXPR_NEST_MAX	32
#define _POSIX2_LINE_MAX	2048
#define _POSIX2_RE_DUP_MAX	255


/* Other Invariant Values */

/* Default process priority. */
#undef NZERO
#define NZERO			20

#endif /* _MACH_MACHLIMITS_H_ */
#endif /* _LIMITS_H___ */
