/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

 THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>
#include "svc.h"

/* Safe casting in both LP64 and ILP32.  */
#define POINTER_TO_PARAM_BLOCK_T(PTR)		\
  (param_block_t)(unsigned long) (PTR)

/* Forward prototypes.  */
int _system _PARAMS ((const char *));
int _rename _PARAMS ((const char *, const char *));
int _isatty _PARAMS ((int));
clock_t _times _PARAMS ((struct tms *));
int _gettimeofday _PARAMS ((struct timeval *, void *));
int _unlink _PARAMS ((const char *));
int _link _PARAMS ((void));
int _stat _PARAMS ((const char *, struct stat *));
int _fstat _PARAMS ((int, struct stat *));
int _swistat _PARAMS ((int fd, struct stat * st));
caddr_t _sbrk _PARAMS ((int));
int _getpid _PARAMS ((int));
int _close _PARAMS ((int));
clock_t _clock _PARAMS ((void));
int _swiclose _PARAMS ((int));
int _open _PARAMS ((const char *, int, ...));
int _swiopen _PARAMS ((const char *, int));
int _write _PARAMS ((int, char *, int));
int _swiwrite _PARAMS ((int, char *, int));
int _lseek _PARAMS ((int, int, int));
int _swilseek _PARAMS ((int, int, int));
int _read _PARAMS ((int, char *, int));
int _swiread _PARAMS ((int, char *, int));
void initialise_monitor_handles _PARAMS ((void));

static int checkerror _PARAMS ((int));
static int error _PARAMS ((int));
static int get_errno _PARAMS ((void));

/* Struct used to keep track of the file position, just so we
   can implement fseek(fh,x,SEEK_CUR).  */
struct fdent
{
  int handle;
  int flags;
  ino_t ino;
  int pos;
};

#define MAX_OPEN_FILES 20

/* User file descriptors (fd) are integer indexes into
   the openfiles[] array. Error checking is done by using
   findslot().

   This openfiles array is manipulated directly by only
   these 5 functions:

	findslot() - Translate entry.
	newslot() - Find empty entry.
	initilise_monitor_handles() - Initialize entries.
	_swiopen() - Initialize entry.
	_close() - Handle stdout == stderr case.

   Every other function must use findslot().  */

static struct fdent openfiles[MAX_OPEN_FILES];

static struct fdent *findslot _PARAMS ((int));
static int newslot _PARAMS ((void));

/* Register name faking - works in collusion with the linker.  */
#ifdef __ILP32__
register char * stack_ptr asm ("wsp");
#else
register char * stack_ptr asm ("sp");
#endif


/* following is copied from libc/stdio/local.h to check std streams */
extern void _EXFUN (__sinit, (struct _reent *));
#define CHECK_INIT(ptr) \
  do						\
    {						\
      if ((ptr) && !(ptr)->__sdidinit)		\
	__sinit (ptr);				\
    }						\
  while (0)

static int monitor_stdin;
static int monitor_stdout;
static int monitor_stderr;

/* Return a pointer to the structure associated with
   the user file descriptor fd. */
static struct fdent *
findslot (int fd)
{
  CHECK_INIT (_REENT);

  /* User file descriptor is out of range. */
  if ((unsigned int) fd >= MAX_OPEN_FILES)
    return NULL;

  /* User file descriptor is open? */
  if (openfiles[fd].handle == -1)
    return NULL;

  /* Valid. */
  return &openfiles[fd];
}

/* Return the next lowest numbered free file
   structure, or -1 if we can't find one. */
static int
newslot (void)
{
  int i;

  for (i = 0; i < MAX_OPEN_FILES; i++)
    if (openfiles[i].handle == -1)
      break;

  if (i == MAX_OPEN_FILES)
    return -1;

  return i;
}

void
initialise_monitor_handles (void)
{
  int i;

  /* Open the standard file descriptors by opening the special
   * teletype device, ":tt", read-only to obtain a descritpor for
   * standard input and write-only to obtain a descriptor for standard
   * output. Finally, open ":tt" in append mode to obtain a descriptor
   * for standard error. Since this is a write mode, most kernels will
   * probably return the same value as for standard output, but the
   * kernel can differentiate the two using the mode flag and return a
   * different descriptor for standard error.
   */

  param_block_t block[3];

  block[0] = POINTER_TO_PARAM_BLOCK_T (":tt");
  block[2] = 3;			/* length of filename */
  block[1] = 0;			/* mode "r" */
  monitor_stdin = do_AngelSVC (AngelSVC_Reason_Open, block);

  block[0] = POINTER_TO_PARAM_BLOCK_T (":tt");
  block[2] = 3;			/* length of filename */
  block[1] = 4;			/* mode "w" */
  monitor_stdout = do_AngelSVC (AngelSVC_Reason_Open, block);

  block[0] = POINTER_TO_PARAM_BLOCK_T (":tt");
  block[2] = 3;			/* length of filename */
  block[1] = 8;			/* mode "a" */
  monitor_stderr = do_AngelSVC (AngelSVC_Reason_Open, block);

  /* If we failed to open stderr, redirect to stdout. */
  if (monitor_stderr == -1)
    monitor_stderr = monitor_stdout;

  for (i = 0; i < MAX_OPEN_FILES; i++)
    openfiles[i].handle = -1;

  openfiles[0].handle = monitor_stdin;
  openfiles[0].flags = _FREAD;
  openfiles[0].pos = 0;
  openfiles[1].handle = monitor_stdout;
  openfiles[0].flags = _FWRITE;
  openfiles[1].pos = 0;
  openfiles[2].handle = monitor_stderr;
  openfiles[0].flags = _FWRITE;
  openfiles[2].pos = 0;
}

static int
get_errno (void)
{
  return do_AngelSVC (AngelSVC_Reason_Errno, NULL);
}

/* Set errno and return result. */
static int
error (int result)
{
  errno = get_errno ();
  return result;
}

/* Check the return and set errno appropriately. */
static int
checkerror (int result)
{
  if (result == -1)
    return error (-1);
  return result;
}

/* fh, is a valid internal file handle.
   ptr, is a null terminated string.
   len, is the length in bytes to read.
   Returns the number of bytes *not* written. */
int
_swiread (int fh, char *ptr, int len)
{
  param_block_t block[3];

  block[0] = fh;
  block[1] = POINTER_TO_PARAM_BLOCK_T (ptr);
  block[2] = len;

  return checkerror (do_AngelSVC (AngelSVC_Reason_Read, block));
}

/* fd, is a valid user file handle.
   Translates the return of _swiread into
   bytes read. */
int
_read (int fd, char *ptr, int len)
{
  int res;
  struct fdent *pfd;

  pfd = findslot (fd);
  if (pfd == NULL)
    {
      errno = EBADF;
      return -1;
    }

  res = _swiread (pfd->handle, ptr, len);

  if (res == -1)
    return res;

  pfd->pos += len - res;

  /* res == len is not an error,
     at least if we want feof() to work.  */
  return len - res;
}

/* fd, is a user file descriptor. */
int
_swilseek (int fd, int ptr, int dir)
{
  int res;
  struct fdent *pfd;

  /* Valid file descriptor? */
  pfd = findslot (fd);
  if (pfd == NULL)
    {
      errno = EBADF;
      return -1;
    }

  /* Valid whence? */
  if ((dir != SEEK_CUR) && (dir != SEEK_SET) && (dir != SEEK_END))
    {
      errno = EINVAL;
      return -1;
    }

  /* Convert SEEK_CUR to SEEK_SET */
  if (dir == SEEK_CUR)
    {
      ptr = pfd->pos + ptr;
      /* The resulting file offset would be negative. */
      if (ptr < 0)
	{
	  errno = EINVAL;
	  if ((pfd->pos > 0) && (ptr > 0))
	    errno = EOVERFLOW;
	  return -1;
	}
      dir = SEEK_SET;
    }

  param_block_t block[2];
  if (dir == SEEK_END)
    {
      block[0] = pfd->handle;
      res = checkerror (do_AngelSVC (AngelSVC_Reason_FLen, block));
      if (res == -1)
	return -1;
      ptr += res;
    }

  /* This code only does absolute seeks.  */
  block[0] = pfd->handle;
  block[1] = ptr;
  res = checkerror (do_AngelSVC (AngelSVC_Reason_Seek, block));
  /* At this point ptr is the current file position. */
  if (res >= 0)
    {
      pfd->pos = ptr;
      return ptr;
    }
  else
    return -1;
}

_lseek (int fd, int ptr, int dir)
{
  return _swilseek (fd, ptr, dir);
}

/* fh, is a valid internal file handle.
   Returns the number of bytes *not* written. */
int
_swiwrite (int fh, char *ptr, int len)
{
  param_block_t block[3];

  block[0] = fh;
  block[1] = POINTER_TO_PARAM_BLOCK_T (ptr);
  block[2] = len;

  return checkerror (do_AngelSVC (AngelSVC_Reason_Write, block));
}

/* fd, is a user file descriptor. */
int
_write (int fd, char *ptr, int len)
{
  int res;
  struct fdent *pfd;

  pfd = findslot (fd);
  if (pfd == NULL)
    {
      errno = EBADF;
      return -1;
    }

  res = _swiwrite (pfd->handle, ptr, len);

  /* Clearly an error. */
  if (res < 0)
    return -1;

  pfd->pos += len - res;

  /* We wrote 0 bytes?
     Retrieve errno just in case. */
  if ((len - res) == 0)
    return error (0);

  return (len - res);
}

int
_swiopen (const char *path, int flags)
{
  int aflags = 0, fh;
  param_block_t block[3];
  static ino_t ino = 1;
  int fd;

  if (path == NULL)
    {
      errno = ENOENT;
      return -1;
    }

  fd = newslot ();

  if (fd == -1)
    {
      errno = EMFILE;
      return -1;
    }

  /* It is an error to open a file that already exists. */
  if ((flags & O_CREAT) && (flags & O_EXCL))
    {
      struct stat st;
      int res;
      res = _stat (path, &st);
      if (res != -1)
	{
	  errno = EEXIST;
	  return -1;
	}
    }

  /* The flags are Unix-style, so we need to convert them. */
#ifdef O_BINARY
  if (flags & O_BINARY)
    aflags |= 1;
#endif

  /* In O_RDONLY we expect aflags == 0. */

  if (flags & O_RDWR)
    aflags |= 2;

  if ((flags & O_CREAT) || (flags & O_TRUNC) || (flags & O_WRONLY))
    aflags |= 4;

  if (flags & O_APPEND)
    {
      /* Can't ask for w AND a; means just 'a'.  */
      aflags &= ~4;
      aflags |= 8;
    }

  block[0] = POINTER_TO_PARAM_BLOCK_T (path);
  block[2] = strlen (path);
  block[1] = aflags;

  fh = do_AngelSVC (AngelSVC_Reason_Open, block);

  /* Return a user file descriptor or an error. */
  if (fh >= 0)
    {
      openfiles[fd].handle = fh;
      openfiles[fd].flags = flags + 1;
      openfiles[fd].ino = ino++;
      openfiles[fd].pos = 0;
      return fd;
    }
  else
    return error (fh);
}

int
_open (const char *path, int flags, ...)
{
  return _swiopen (path, flags);
}

/* fh, is a valid internal file handle. */
int
_swiclose (int fh)
{
  param_block_t param[1];
  param[0] = fh;
  return checkerror (do_AngelSVC (AngelSVC_Reason_Close, param));
}

/* fd, is a user file descriptor. */
int
_close (int fd)
{
  int res;
  struct fdent *pfd;

  pfd = findslot (fd);
  if (pfd == NULL)
    {
      errno = EBADF;
      return -1;
    }

  /* Handle stderr == stdout. */
  if ((fd == 1 || fd == 2) && (openfiles[1].handle == openfiles[2].handle))
    {
      pfd->handle = -1;
      return 0;
    }

  /* Attempt to close the handle. */
  res = _swiclose (pfd->handle);

  /* Reclaim handle? */
  if (res == 0)
    pfd->handle = -1;

  return res;
}

int __attribute__((weak))
_getpid (int n __attribute__ ((unused)))
{
  return 1;
}

caddr_t
_sbrk (int incr)
{
  extern char end asm ("end");	/* Defined by the linker.  */
  static char *heap_end;
  char *prev_heap_end;

  if (heap_end == NULL)
    heap_end = &end;

  prev_heap_end = heap_end;

  if (heap_end + incr > stack_ptr)
    {
      /* Some of the libstdc++-v3 tests rely upon detecting
         out of memory errors, so do not abort here.  */
      errno = ENOMEM;
      return (caddr_t) - 1;
    }

  heap_end += incr;

  return (caddr_t) prev_heap_end;
}

int
_swistat (int fd, struct stat *st)
{
  struct fdent *pfd;
  param_block_t param[1];
  int res;

  pfd = findslot (fd);
  if (pfd == NULL)
    {
      errno = EBADF;
      return -1;
    }

  param[0] = pfd->handle;
  res = do_AngelSVC (AngelSVC_Reason_IsTTY, param);
  if (res != 0 && res != 1)
    return error (-1);

  memset (st, 0, sizeof (*st));

  if (res)
    {
      /* This is a tty. */
      st->st_mode |= S_IFCHR;
    }
  else
    {
      /* This is a file, return the file length.  */
      st->st_mode |= S_IFREG;
      res = checkerror (do_AngelSVC (AngelSVC_Reason_FLen, param));
      if (res == -1)
	return -1;
      st->st_size = res;
      st->st_blksize = 1024;
      st->st_blocks = (res + 1023) / 1024;
    }

  /* Deduce permissions based on mode in which file opened.  */
  st->st_mode |= S_IRUSR | S_IRGRP | S_IROTH;
  if (pfd->flags & _FWRITE)
    st->st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;

  st->st_ino = pfd->ino;
  st->st_nlink = 1;
  return 0;
}

int __attribute__((weak))
_fstat (int fd, struct stat * st)
{
  return _swistat (fd, st);
}

int __attribute__((weak))
_stat (const char *fname, struct stat *st)
{
  int fd, res;
  /* The best we can do is try to open the file readonly.
     If it exists, then we can guess a few things about it. */
  if ((fd = _open (fname, O_RDONLY)) == -1)
    return -1;
  res = _swistat (fd, st);
  /* Not interested in the error. */
  _close (fd);
  return res;
}

int __attribute__((weak))
_link (void)
{
  errno = ENOSYS;
  return -1;
}

int
_unlink (const char *path)
{
  int res;
  param_block_t block[2];
  block[0] = POINTER_TO_PARAM_BLOCK_T (path);
  block[1] = strlen (path);
  res = do_AngelSVC (AngelSVC_Reason_Remove, block);
  if (res == -1)
    return error (res);
  return 0;
}

int
_gettimeofday (struct timeval *tp, void *tzvp)
{
  struct timezone *tzp = tzvp;
  if (tp)
    {
      /* Ask the host for the seconds since the Unix epoch.  */
      tp->tv_sec = do_AngelSVC (AngelSVC_Reason_Time, NULL);
      tp->tv_usec = 0;
    }

  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}

/* Return a clock that ticks at 100Hz.  */
clock_t
_clock (void)
{
  clock_t timeval;

  timeval = do_AngelSVC (AngelSVC_Reason_Clock, NULL);
  return timeval;
}

/* Return a clock that ticks at 100Hz.  */
clock_t
_times (struct tms * tp)
{
  clock_t timeval = _clock ();

  if (tp)
    {
      tp->tms_utime = timeval;	/* user time */
      tp->tms_stime = 0;	/* system time */
      tp->tms_cutime = 0;	/* user time, children */
      tp->tms_cstime = 0;	/* system time, children */
    }

  return timeval;
};


int
_isatty (int fd)
{
  struct fdent *pfd;
  param_block_t param[1];
  int res;

  /* Return 1 if fd is an open file descriptor referring to a terminal;
     otherwise 0 is returned, and errno is set to indicate the error.  */

  pfd = findslot (fd);
  if (pfd == NULL)
    {
      errno = EBADF;
      return 0;
    }

  param[0] = pfd->handle;
  res = do_AngelSVC (AngelSVC_Reason_IsTTY, param);

  if (res != 1)
    return error (0);
  return res;
}

int
_system (const char *s)
{
  param_block_t block[2];
  int e;

  /* Hmmm.  The ARM debug interface specification doesn't say whether
     SYS_SYSTEM does the right thing with a null argument, or assign any
     meaning to its return value.  Try to do something reasonable....  */
  if (!s)
    return 1;			/* maybe there is a shell available? we can hope. :-P */
  block[0] = POINTER_TO_PARAM_BLOCK_T (s);
  block[1] = strlen (s);
  e = checkerror (do_AngelSVC (AngelSVC_Reason_System, block));
  if ((e >= 0) && (e < 256))
    {
      /* We have to convert e, an exit status to the encoded status of
         the command.  To avoid hard coding the exit status, we simply
         loop until we find the right position.  */
      int exit_code;

      for (exit_code = e; e && WEXITSTATUS (e) != exit_code; e <<= 1)
	continue;
    }
  return e;
}

int
_rename (const char *oldpath, const char *newpath)
{
  param_block_t block[4];
  block[0] = POINTER_TO_PARAM_BLOCK_T (oldpath);
  block[1] = strlen (oldpath);
  block[2] = POINTER_TO_PARAM_BLOCK_T (newpath);
  block[3] = strlen (newpath);
  return checkerror (do_AngelSVC (AngelSVC_Reason_Rename, block)) ? -1 : 0;
}

/* Returns the number of elapsed target ticks since the support code
   started execution. Returns -1 and sets errno on error.  */
long
__aarch64_angel_elapsed (void)
{
  int result;
  param_block_t block[2];
  result = checkerror (do_AngelSVC (AngelSVC_Reason_Elapsed, block));
  if (result == -1)
    return result;
  return block[0];
}
