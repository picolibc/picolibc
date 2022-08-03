/* cygwin/limits.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_LIMITS_H__
#define _CYGWIN_LIMITS_H__

#define __AIO_LISTIO_MAX 32
#define __AIO_MAX 8
#define __AIO_PRIO_DELTA_MAX 0

/* 32000 is the safe value used for Windows processes when called from
   Cygwin processes. */
#define __ARG_MAX 32000
#define __ATEXIT_MAX 32
#define __CHILD_MAX 256
#define __DELAYTIMER_MAX __INT_MAX__
#define __HOST_NAME_MAX 255
#define __IOV_MAX 1024
#define __LOGIN_NAME_MAX 256	/* equal to UNLEN defined in w32api/lmcons.h */
#define __MQ_OPEN_MAX 256
#define __MQ_PRIO_MAX INT_MAX
#define __OPEN_MAX 3200		/* value of the old OPEN_MAX_MAX */
#define __PAGESIZE 65536
#define __PTHREAD_DESTRUCTOR_ITERATIONS 4

/* Tls has 1088 items - and we don't want to use them all :] */
#define __PTHREAD_KEYS_MAX 1024
/* Actually the minimum stack size is somewhat of a split personality.
   The size parameter in a CreateThread call is the size of the initially
   commited stack size, which can be specified as low as 4K.  However, the
   default *reserved* stack size is 1 Meg, unless the .def file specifies
   another STACKSIZE value.  And even if you specify a stack size below 64K,
   the allocation granularity is in the way.  You can never squeeze multiple
   threads in the same allocation granularity slot.  Oh well. */
#define __PTHREAD_STACK_MIN 65536

#define __RTSIG_MAX 33
#define __SEM_VALUE_MAX 1147483648
#define __SIGQUEUE_MAX 32
#define __STREAM_MAX 20
#define __SYMLOOP_MAX 10
#define __TIMER_MAX 32
#define __TTY_NAME_MAX 32
#define __FILESIZEBITS 64
#define __LINK_MAX 1024
#define __MAX_CANON 255
#define __MAX_INPUT 255
#define __NAME_MAX 255

/* Keep in sync with __PATHNAME_MAX__ in cygwin/config.h */
#define __PATH_MAX 4096
#define __PIPE_BUF 4096

#endif /* _CYGWIN_LIMITS_H__ */
