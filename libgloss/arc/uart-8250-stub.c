/*
 * uart-8250-stub.c -- various stubs for 8250 UART libgloss.
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
 */
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "glue.h"


/* Always return character device with 1024 bytes block.  */
int
_fstat (int fd __attribute__ ((unused)), struct stat *buf)
{
  memset (buf, 0, sizeof (*buf));
  buf->st_mode = S_IFCHR;
  buf->st_blksize = 1024;

  return 0;
}

/* ARC UART is actually console so just return 1.  */
int
_isatty (int fildes __attribute__ ((unused)))
{
  return 1;
}

/* Should be provided by crt0.S.  */
extern void __attribute__((noreturn)) _exit_halt (int ret);

void
__attribute__((noreturn))
_exit (int ret)
{
  _exit_halt (ret);
}

/* Not implemented.  */
off_t
_lseek (int fd __attribute__ ((unused)),
	off_t offset __attribute__ ((unused)),
	int whence __attribute__ ((unused)))
{
  errno = ENOSYS;
  return -1;
}

/* Not implemented.  */
int
_close (int fd __attribute__ ((unused)))
{
  errno = ENOSYS;
  return -1;
}

/* Not implemented.  */
int
_open (const char *path __attribute__ ((unused)),
       int flags __attribute__ ((unused)), ...)
{
  errno = ENOSYS;
  return -1;
}

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

/* No arguments.  */
int
_argc (void)
{
  return 0;
}

/* No arguments.  */
uint32_t
_argvlen (int a __attribute__ ((unused)))
{
  return 0;
}

/* No arguments.  */
int
_argv (int a __attribute__ ((unused)), char *arg __attribute__ ((unused)))
{
  return -1;
}
