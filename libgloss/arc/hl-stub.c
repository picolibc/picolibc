/*
 * hl-stub.c -- provide _kill() and _getpid().
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "glue.h"


/* If PID is equal to __MYPID, exit with sig as retcode.  */
int
_kill (int pid, int sig)
{
  if (pid == __MYPID)
    _exit (sig);

  errno = ENOSYS;
  return -1;
}


/* Return __MYPID.  */
int
_getpid (void)
{
  return __MYPID;
}

/* We do not have 64-bit compatible hostlink fstat.  */
#if defined (__ARC64__)
int
_fstat (int fd, struct stat *st)
{
  memset (st, 0, sizeof (*st));
  st->st_mode = S_IFCHR;
  st->st_blksize = 1024;

  return 0;
}
#endif


/* hostlink backend has only fstat(), so use fstat() in stat().  */
int
_stat (const char *pathname, struct stat *buf)
{
  int fd;
  int ret;
  int saved_errno;

  fd = open (pathname, O_RDONLY);
  if (fd < 0)
    {
      /* errno is set by open().  */
      return -1;
    }

  ret = fstat (fd, buf);
  saved_errno = errno;

  close (fd);

  errno = saved_errno;

  return ret;
}


/* No Metaware hostlink backend for this call.  */
int
_link (const char *oldpath __attribute__ ((unused)),
       const char *newpath __attribute__ ((unused)))
{
  errno = ENOSYS;
  return -1;
}
