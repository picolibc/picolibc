/*
 * C library support files for the Blackfin processor
 *
 * Copyright (C) 2006 Analog Devices, Inc.
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

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "syscall.h"
#include <errno.h>
#include <reent.h>
#include <unistd.h>

register char *stack_ptr asm ("SP");

static inline int
do_syscall (int reason, void *arg)
{
  int result, result2, errcode;
  asm volatile ("excpt 0;"
		: "=q0" (result),
		  "=q1" (result2),
		  "=q2" (errcode)
		: "qA" (reason),
		  "q0" (arg)
		: "memory", "CC");
  errno = errcode;
  return result;
}

int
_read (int file, char *ptr, int len)
{
  int block[3];

  block[0] = file;
  block[1] = (int) ptr;
  block[2] = len;

  return do_syscall (SYS_read, block);
}

int
_lseek (int file, int ptr, int whence)
{
  int block[3];

  block[0] = file;
  block[1] = ptr;
  block[2] = whence;

  return do_syscall (SYS_lseek, block);
}

int
_write (int file, char *ptr, int len)
{
  int block[3];

  block[0] = file;
  block[1] = (int) ptr;
  block[2] = len;

  return do_syscall (SYS_write, block);
}

int
_open (const char *path, int flags)
{
  int block[2];

  block[0] = (int) path;
  block[1] = flags;

  return do_syscall (SYS_open, block);
}

int
_close (int file)
{
  return do_syscall (SYS_close, &file);
}

void
_exit (int n)
{
  do_syscall (SYS_exit, &n);
  __builtin_unreachable ();
}

int
_kill (int n, int m)
{
  int block[2];

  block[0] = n;
  block[1] = m;

  return do_syscall (SYS_kill, block);
}

int
_getpid (int n)
{
  return do_syscall (SYS_getpid, &n);
}

caddr_t
_sbrk (int incr)
{
  extern char end;		/* Defined by the linker.  */
  static char *heap_end;
  char *prev_heap_end;

  if (heap_end == NULL)
    heap_end = &end;

  prev_heap_end = heap_end;

  if (heap_end + incr > stack_ptr)
    {
      /* Some of the libstdc++-v3 tests rely upon detecting
	 out of memory errors, so do not abort here.  */
#if 0
      extern void abort (void);

      _write (1, "_sbrk: Heap and stack collision\n", 32);

      abort ();
#else
      errno = ENOMEM;
      return (caddr_t) -1;
#endif
    }

  heap_end += incr;

  return (caddr_t) prev_heap_end;
}

extern void *memset (void *, int, unsigned int);

int
_fstat (int file, struct stat *st)
{
  int block[2];

  block[0] = file;
  block[1] = (int) st;

  return do_syscall (SYS_fstat, block);
}

int _stat (const char *fname, struct stat *st)
{
  int block[2];

  block[0] = (int) fname;
  block[1] = (int) st;

  return do_syscall (SYS_stat, block);
}

int
_link (const char *existing, const char *new)
{
  int block[2];

  block[0] = (int) existing;
  block[1] = (int) new;

  return do_syscall (SYS_link, block);
}

int
_unlink (const char *path)
{
  return do_syscall (SYS_unlink, (char *) path);
}

void
_raise (void)
{
  return;
}

int
_gettimeofday (struct timeval *tv, void *tz)
{
  tv->tv_usec = 0;
  tv->tv_sec = do_syscall (SYS_time, 0);
  return 0;
}

/* Return a clock that ticks at 100Hz.  */
clock_t
_times (struct tms * tp)
{
  return -1;
}

int
_isatty (int fd)
{
  return 1;
}

int
_system (const char *s)
{
  if (s == NULL)
    return 0;
  errno = ENOSYS;
  return -1;
}

int
_rename (const char * oldpath, const char * newpath)
{
  errno = ENOSYS;
  return -1;
}

static inline int
__setup_argv_for_main (int argc)
{
  int block[2];
  char **argv;
  int i = argc;

  argv = __builtin_alloca ((1 + argc) * sizeof (*argv));

  argv[i] = NULL;
  while (i--) {
    block[0] = i;
    argv[i] = __builtin_alloca (1 + do_syscall (SYS_argnlen, (void *)block));
    block[1] = (int) argv[i];
    do_syscall (SYS_argn, (void *)block);
  }

  return main (argc, argv);
}

int
__setup_argv_and_call_main ()
{
  int argc = do_syscall (SYS_argc, 0);

  if (argc <= 0)
    return main (argc, NULL);
  else
    return __setup_argv_for_main (argc);
}
