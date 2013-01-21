/* sys/param.h

   Copyright 1996, 1997, 1998, 2002, 2003, 2005, 2007, 2009, 2011, 2012 Red
   Hat, Inc.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <sys/types.h>
/* Linux includes limits.h, but this is not universally done. */
#include <limits.h>

#define __need_NULL
#include <stddef.h>

/* Max number of open files.  The Posix version is OPEN_MAX.  */
/* Number of fds is virtually unlimited in cygwin, but we must provide
   some reasonable value for Posix conformance */
#define NOFILE		8192

/* Max number of groups; must keep in sync with NGROUPS_MAX in limits.h */
#define NGROUPS		NGROUPS_MAX

/* Ticks/second for system calls such as times() */
/* FIXME: is this the appropriate value? */
#define HZ		1000

/* Max hostname size that can be dealt with (== Win32 MAX_HOSTNAME_LEN) */
#define MAXHOSTNAMELEN	128

/* Maximum path length including trailing NUL; the Posix version is PATH_MAX.
   MAXPATHLEN is the BSD variant. */
#define MAXPATHLEN      PATH_MAX

/* Maximum number of nested symlinks; the Posix version is SYMLOOP_MAX.
   MAXSYMLINKS is the BSD variant. */
#define MAXSYMLINKS	SYMLOOP_MAX

/* This is the number of bytes per block given in the st_blocks stat member.
   It should be in sync with S_BLKSIZE in sys/stat.h.  S_BLKSIZE is the
   BSD variant of this constant. */
#define DEV_BSIZE	1024

#ifndef NBBY
#define    NBBY 8
#endif

/* Bit map related macros. */
#define    setbit(a,i) ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define    clrbit(a,i) ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define    isset(a,i)  ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define    isclr(a,i)  (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

/* Macros for counting and rounding. */
#ifndef howmany
#define    howmany(x, y)   (((x)+((y)-1))/(y))
#endif
#define    rounddown(x, y) (((x)/(y))*(y))
#define    roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */
#define    roundup2(x, y)  (((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#define powerof2(x)    ((((x)-1)&(x))==0)

/* Macros for min/max. */
#define    MIN(a,b)    (((a)<(b))?(a):(b))
#define    MAX(a,b)    (((a)>(b))?(a):(b))

#endif
