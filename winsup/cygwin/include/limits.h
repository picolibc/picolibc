/* limits.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _LIMITS_H___

#include <features.h>
#include <bits/wordsize.h>

#ifndef _MACH_MACHLIMITS_H_

/* _MACH_MACHLIMITS_H_ is used on OSF/1.  */
#define _LIMITS_H___
#define _MACH_MACHLIMITS_H_


/* Numerical limits */

/* Number of bits in a `char'.  */
#undef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__

#if __XSI_VISIBLE || __POSIX_VISIBLE >= 200809
/* Number of bits in a `long'.  */
#undef LONG_BIT
#define LONG_BIT (__SIZEOF_LONG__ * __CHAR_BIT__)

/* Number of bits in a `int'.  */
#undef WORD_BIT
#define WORD_BIT (__SIZEOF_INT__ * __CHAR_BIT__)
#endif /* __XSI_VISIBLE || __POSIX_VISIBLE >= 200809 */

/* Maximum length of a multibyte character.  */
#ifndef MB_LEN_MAX
/* Use value from newlib. */
#define MB_LEN_MAX 8
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
#if __WORDSIZE == 64
#define __LONG_MAX__ 9223372036854775807L
#else
#define __LONG_MAX__ 2147483647L
# endif /* __alpha__ */
#endif
#undef LONG_MIN
#define LONG_MIN (-LONG_MAX-1L)
#undef LONG_MAX
#define LONG_MAX __LONG_MAX__

/* Maximum value an `unsigned long int' can hold.  (Minimum is 0).  */
#undef ULONG_MAX
#define ULONG_MAX (LONG_MAX * 2UL + 1)

/* Minimum and maximum values a `signed long long int' can hold.  */
#ifndef __LONG_LONG_MAX__
#define __LONG_LONG_MAX__ 9223372036854775807LL
#endif

#if __GNU_VISIBLE
#undef LONG_LONG_MIN
#define LONG_LONG_MIN (-LONG_LONG_MAX-1)
#undef LONG_LONG_MAX
#define LONG_LONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
#undef ULONG_LONG_MAX
#define ULONG_LONG_MAX (LONG_LONG_MAX * 2ULL + 1)
#endif

#if __ISO_C_VISIBLE >= 1999
/* Minimum and maximum values a `signed long long int' can hold.  */
#undef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)
#undef LLONG_MAX
#define LLONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
#undef ULLONG_MAX
#define ULLONG_MAX (LLONG_MAX * 2ULL + 1)
#endif /* __ISO_C_VISIBLE >= 1999 */

/* Maximum size of ssize_t. Sadly, gcc doesn't give us __SSIZE_MAX__
   the way it does for __SIZE_MAX__.  On the other hand, we happen to
   know that for Cygwin, ssize_t is 'int' on 32-bit and 'long' on
   64-bit, and this particular header is specific to Cygwin, so we
   don't have to jump through hoops. */
#undef SSIZE_MAX
#if __WORDSIZE == 64
#define SSIZE_MAX (__LONG_MAX__)
#else
#define SSIZE_MAX (__INT_MAX__)
#endif


/* Runtime Invariant Values */

/* Please note that symbolic names shall be ommited, on specific
   implementations where the corresponding value is equal to or greater
   than the stated minimum, but is unspecified.  This indetermination
   might depend on the amount of available memory space on a specific
   instance of a specific implementation. The actual value supported by
   a specific instance shall be provided by the sysconf() function. */

/* Maximum number of I/O operations in a single list I/O call supported by
   the implementation.  Not yet implemented. */
#undef AIO_LISTIO_MAX
/* #define AIO_LISTIO_MAX >= _POSIX_AIO_LISTIO_MAX */

/* Maximum number of outstanding asynchronous I/O operations supported by
   the implementation.  Not yet implemented. */
#undef AIO_MAX
/*  #define AIO_MAX >= _POSIX_AIO_MAX */

/* The maximum amount by which a process can decrease its asynchronous I/O
   priority level from its own scheduling priority. */
#undef AIO_PRIO_DELTA_MAX
/* #define AIO_PRIO_DELTA_MAX >= 0 */

/* Maximum number of bytes in arguments and environment passed in an exec
   call.  32000 is the safe value used for Windows processes when called
   from Cygwin processes. */
#undef ARG_MAX
#define ARG_MAX 32000

#if __XSI_VISIBLE || __POSIX_VISIBLE >= 200809
/* Maximum number of functions that may be registered with atexit(). */
#undef ATEXIT_MAX
#define ATEXIT_MAX 32
#endif

/* Maximum number of simultaneous processes per real user ID. */
#undef CHILD_MAX
#define CHILD_MAX 256

/* Maximum number of timer expiration overruns.  Not yet implemented. */
#undef DELAYTIMER_MAX
/* #define DELAYTIMER_MAX >= _POSIX_DELAYTIMER_MAX */

/* Maximum length of a host name. */
#undef HOST_NAME_MAX
#define HOST_NAME_MAX 255

#if __XSI_VISIBLE
/* Maximum number of iovcnt in a writev (an arbitrary number) */
#undef IOV_MAX
#define IOV_MAX 1024
#endif

/* Maximum number of characters in a login name. */
#undef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 256	/* equal to UNLEN defined in w32api/lmcons.h */

/* The maximum number of open message queue descriptors a process may hold. */
#undef MQ_OPEN_MAX
#define MQ_OPEN_MAX OPEN_MAX

/* The maximum number of message priorities supported by the implementation. */
#undef MQ_PRIO_MAX
#define MQ_PRIO_MAX INT_MAX

/* # of open files per process. Actually it can be more since Cygwin
   grows the dtable as necessary. We define a reasonable limit here
   which is returned by getdtablesize(), sysconf(_SC_OPEN_MAX) and
   getrlimit(RLIMIT_NOFILE). */
#undef OPEN_MAX
#define OPEN_MAX 256

/* Size in bytes of a page. */
#undef PAGESIZE
#define PAGESIZE 65536
#if __XSI_VISIBLE
#undef PAGE_SIZE
#define PAGE_SIZE PAGESIZE
#endif

/* Maximum number of attempts made to destroy a thread's thread-specific
   data values on thread exit. */
/* FIXME: I really don't understand this value.  Why should multiple
   attempts be necessary to destroy thread-specific data?!?  Anyway, the
   current value here is 1, taken originally from our pthread.h file,
   where it was mistakenly defined first.  Unfortunately this value is
   lower than the POSIX defined minimum value, which is 4. */
#undef PTHREAD_DESTRUCTOR_ITERATIONS
#define PTHREAD_DESTRUCTOR_ITERATIONS 1

/* Maximum number of data keys that can be created by a process. */
/* Tls has 1088 items - and we don't want to use them all :] */
#undef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX 1024

/* Minimum size in bytes of thread stack storage. */
/* Actually the minimum stack size is somewhat of a split personality.
   The size parameter in a CreateThread call is the size of the initially
   commited stack size, which can be specified as low as 4K.  However, the
   default *reserved* stack size is 1 Meg, unless the .def file specifies
   another STACKSIZE value.  And even if you specify a stack size below 64K,
   the allocation granularity is in the way.  You can never squeeze multiple
   threads in the same allocation granularity slot.  Oh well. */
#undef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 65536

/* Maximum number of threads that can be created per process. */
/* Windows allows any arbitrary number of threads per process. */
#undef PTHREAD_THREADS_MAX
/* #define PTHREAD_THREADS_MAX unspecified */

/* Maximum number of realtime signals reserved for application use. */
/* FIXME: We only support one realtime signal in 32 bit mode, but
	 _POSIX_RTSIG_MAX is 8. */
#undef RTSIG_MAX
#if __WORDSIZE == 64
#define RTSIG_MAX 33
#else
#define RTSIG_MAX 1
#endif

/* Maximum number of semaphores that a process may have. */
/* Windows allows any arbitrary number of semaphores per process. */
#undef SEM_NSEMS_MAX
/* #define SEM_NSEMS_MAX unspecified */

/* The maximum value a semaphore may have. */
#undef SEM_VALUE_MAX
#define SEM_VALUE_MAX 1147483648

/* Maximum number of queued signals that a process may send and have pending
   at the receiver(s) at any time. */
#undef SIGQUEUE_MAX
#define SIGQUEUE_MAX 32

/* The maximum number of replenishment operations that may be simultaneously
   pending for a particular sporadic server scheduler.  Not implemented. */
#undef SS_REPL_MAX
/* #define SS_REPL_MAX >= _POSIX_SS_REPL_MAX */

/* Number of streams that one process can have open at one time. */
#undef STREAM_MAX
#define STREAM_MAX 20

/* Maximum number of nested symbolic links. */
#undef SYMLOOP_MAX
#define SYMLOOP_MAX 10

/* Maximum number of timer expiration overruns. */
#undef TIMER_MAX
#define TIMER_MAX 32

/* Maximum length of the trace event name.  Not implemented. */
#undef TRACE_EVENT_NAME_MAX
/* #define TRACE_EVENT_NAME_MAX >= _POSIX_TRACE_EVENT_NAME_MAX */

/* Maximum length of the trace generation version string or of the trace
   stream name.  Not implemented. */
#undef TRACE_NAME_MAX
/* #define TRACE_NAME_MAX >= _POSIX_TRACE_NAME_MAX */

/* Maximum number of trace streams that may simultaneously exist in the
   system.  Not implemented. */
#undef TRACE_SYS_MAX
/* #define TRACE_SYS_MAX >= _POSIX_TRACE_SYS_MAX */

/* Maximum number of user trace event type identifiers that may simultaneously
   exist in a traced process, including the predefined user trace event
   POSIX_TRACE_UNNAMED_USER_EVENT.  Not implemented. */
#undef TRACE_USER_EVENT_MAX
/* #define TRACE_USER_EVENT_MAX >= _POSIX_TRACE_USER_EVENT_MAX */

/* Maximum number of characters in a tty name. */
#undef TTY_NAME_MAX
#define TTY_NAME_MAX 32

/* Maximum number of bytes supported for the name of a timezone (not of the TZ variable).  Not implemented. */
#undef TZNAME_MAX
/* #define TZNAME_MAX >= _POSIX_TZNAME_MAX */


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

/* Maximum length of a path given to API functions including trailing NUL.
   Deliberately set to the same default value as on Linux.  Internal paths
   may be longer. */
/* Keep in sync with __PATHNAME_MAX__ in cygwin/config.h */
#undef PATH_MAX
#define PATH_MAX 4096

/* # of bytes in a pipe buf. This is the max # of bytes which can be
   written to a pipe in one atomic operation. */
#undef PIPE_BUF
#define PIPE_BUF 4096

/* Minimum number of bytes of storage actually allocated for any portion
   of a file.  Not implemented. */
#undef POSIX_ALLOC_SIZE_MIN
/* #define POSIX_ALLOC_SIZE_MIN unspecifed */

/* Recommended increment for file transfer sizes between the
   {POSIX_REC_MIN_XFER_SIZE} and {POSIX_REC_MAX_XFER_SIZE} values.
   Not implemented. */
#undef POSIX_REC_INCR_XFER_SIZE
/* #define POSIX_REC_INCR_XFER_SIZE unspecifed */

/* Maximum recommended file transfer size.  Not implemented. */
#undef POSIX_REC_MAX_XFER_SIZE
/* #define POSIX_REC_MAX_XFER_SIZE unspecifed */

/* Minimum recommended file transfer size.  Not implemented. */
#undef POSIX_REC_MIN_XFER_SIZE
/* #define POSIX_REC_MIN_XFER_SIZE unspecifed */

/* Recommended file transfer buffer alignment.  Not implemented. */
#undef POSIX_REC_XFER_ALIGN
/* #define POSIX_REC_XFER_ALIGN unspecifed */

/* Maximum number of bytes in a symbolic link. */
#undef SYMLINK_MAX
#define SYMLINK_MAX (PATH_MAX - 1)


/* Runtime Increasable Values */

#if __POSIX_VISIBLE >= 2
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

/* Maximum number of bytes in a character class name.  Not implemented. */
#undef CHARCLASS_NAME_MAX
/* #define CHARCLASS_NAME_MAX >= _POSIX2_CHARCLASS_NAME_MAX */

/* Maximum number of weights that can be assigned to an entry of the
   LC_COLLATE order keyword in the locale definition file. */
/* FIXME: We don't support this at all right now, so this value is
   misleading at best.  It's also lower than _POSIX2_COLL_WEIGHTS_MAX
   which is not good.  So, for now we deliberately not define it even
   though it was defined in the former syslimits.h file. */
#undef COLL_WEIGHTS_MAX
/* #define COLL_WEIGHTS_MAX >= _POSIX2_COLL_WEIGHTS_MAX */

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
#define NGROUPS_MAX 1024

/* Maximum number of repeated occurrences of a regular expression when
   using the interval notation \{m,n\} */
#undef RE_DUP_MAX
#define RE_DUP_MAX 255
#endif /* __POSIX_VISIBLE >= 2 */


/* POSIX values */
/* These should never vary from one system type to another */
/* They represent the minimum values that POSIX systems must support.
   POSIX-conforming apps must not require larger values. */

#if __POSIX_VISIBLE
/* Maximum Values */

#define _POSIX_CLOCKRES_MIN                 20000000

/* Minimum Values */

#define _POSIX_AIO_LISTIO_MAX                      2
#define _POSIX_AIO_MAX                             1
#define	_POSIX_ARG_MAX		                4096
#define _POSIX_CHILD_MAX	                  25
#define _POSIX_DELAYTIMER_MAX                     32
#define _POSIX_HOST_NAME_MAX	                 255
#define _POSIX_LINK_MAX		                   8
#define _POSIX_LOGIN_NAME_MAX	                   9
#define _POSIX_MAX_CANON	                 255
#define _POSIX_MAX_INPUT	                 255
#define _POSIX_MQ_OPEN_MAX                         8
#define _POSIX_MQ_PRIO_MAX                        32
#define _POSIX_NAME_MAX		                  14
#define _POSIX_NGROUPS_MAX	                   8
#define _POSIX_OPEN_MAX		                  20
#define _POSIX_PATH_MAX		                 256
#define _POSIX_PIPE_BUF		                 512
#define _POSIX_RE_DUP_MAX	                 255
#define _POSIX_RTSIG_MAX	                   8
#define _POSIX_SEM_NSEMS_MAX                     256
#define _POSIX_SEM_VALUE_MAX                   32767
#define _POSIX_SIGQUEUE_MAX                       32
#define _POSIX_SSIZE_MAX	               32767
#define _POSIX_STREAM_MAX	                   8
#define _POSIX_SS_REPL_MAX                         4
#define _POSIX_SYMLINK_MAX	                 255
#define _POSIX_SYMLOOP_MAX	                   8
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS        4
#define _POSIX_THREAD_KEYS_MAX                   128
#define _POSIX_THREAD_THREADS_MAX                 64
#define _POSIX_TIMER_MAX	                  32
#define _POSIX_TRACE_EVENT_NAME_MAX               30
#define _POSIX_TRACE_NAME_MAX                      8
#define _POSIX_TRACE_SYS_MAX                       8
#define _POSIX_TRACE_USER_EVENT_MAX               32
#define _POSIX_TTY_NAME_MAX	                   9
#define _POSIX_TZNAME_MAX                          6
#endif /* __POSIX_VISIBLE */

#if __POSIX_VISIBLE >= 2
#define _POSIX2_BC_BASE_MAX	                  99
#define _POSIX2_BC_DIM_MAX	                2048
#define _POSIX2_BC_SCALE_MAX	                  99
#define _POSIX2_BC_STRING_MAX	                1000
#define _POSIX2_COLL_WEIGHTS_MAX                   2
#define _POSIX2_EXPR_NEST_MAX	                  32
#define _POSIX2_LINE_MAX	                2048
#define _POSIX2_RE_DUP_MAX	                 255
#endif /* __POSIX_VISIBLE >= 2 */

#if __XSI_VISIBLE
#define _XOPEN_IOV_MAX                            16
#define _XOPEN_NAME_MAX                          255
#define _XOPEN_PATH_MAX                         1024
#endif

/* Other Invariant Values */

#define NL_ARGMAX                                  9
#if __XSI_VISIBLE
#define NL_LANGMAX                                14
#endif
#if __XSI_VISIBLE || __POSIX_VISIBLE >= 200809
#define NL_MSGMAX                              32767
#define NL_SETMAX                                255
#define NL_TEXTMAX                  _POSIX2_LINE_MAX
#endif
#if __POSIX_VISIBLE < 200809
#define NL_NMAX                              INT_MAX
#endif

#if __XSI_VISIBLE
/* Default process priority. */
#undef NZERO
#define NZERO			                  20
#endif

#endif /* _MACH_MACHLIMITS_H_ */
#endif /* _LIMITS_H___ */
