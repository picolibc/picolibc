#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <_ansi.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <reent.h>

_syscall3 (_ssize_t,read, int,fd, void *,buf, size_t,nbytes)

_syscall3 (_ssize_t,write, int,fd, const void *,buf, size_t,nbytes)

/* FIXME: The prototype in <fcntl.h> for open() uses ...,
   but reent.h uses int.  */

_syscall3 (int,open, const char *,buf, int,flags, int,mode)
#if 0
  /* Could put this into a variant of _syscall3:  */
  int mode;
  va_list ap;

  va_start (ap, flags);
  mode = va_arg (ap, int);
  va_end (ap);
#endif

_syscall1 (int,close, int,fd)

_syscall3 (off_t,lseek, int,fd, off_t,offset, int,whence)

_syscall2 (int,fstat, int,file, struct stat *,st)

_syscall2 (int,gettimeofday, struct timeval *,tv, void *,tz)

_syscall1 (void,exit, int,ret)

time_t
_time (time_t *timer)
{
  return 0;
}

int
_creat_r (struct _reent *r, const char *path, int mode)
{
  return _open_r (r, path, O_CREAT | O_TRUNC, mode);
}

int
_getpid_r (struct _reent *r)
{
  return 42;
}

int
_kill_r (struct _reent *r, int pid, int sig)
{
  _exit_r (r, 0xdead00 | (sig & 0xff));
}
