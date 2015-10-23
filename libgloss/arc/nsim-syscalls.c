/*
   Copyright (c) 2015, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

#include "glue.h"
#include "nsim-syscall.h"

/* Those system calls are implemented in both nSIM and CGEN.  */
_syscall3 (_ssize_t, read, int, fd, void *, buf, size_t, count)
_syscall3 (_ssize_t, write, int, fd, const void *, buf, size_t, count)
_syscall1 (int, unlink, const char *, pathname)
_syscall3 (off_t, lseek, int, fd, off_t, offset, int, whence)
_syscall2 (int, gettimeofday, struct timeval *, tv, void *, tz)
_syscall1 (_CLOCK_T_, time, _CLOCK_T_ *, t)
_syscall1 (int, close, int, fd)
_syscall1 (_CLOCK_T_, times, struct tms *, buf)
/* stat requires custom implementation.  */
/* _syscall2 (int, stat, const char *, path, struct stat *, st)
   _syscall2 (int, fstat, int, file, struct stat *, st) */
/* nSIM implements brk and sbrk, but instead sbrk.c is used.  */
/* _syscall1 (int, brk, void *, addr) */
/* open requires custom implementation.  */
/* _syscall3 (int, open, const char *, pathname, int, flags, int, mode) */

/* Those syscalls are not available in CGEN simulator.  */
_syscall1 (int, rmdir, const char *, pathname)
_syscall2 (char *, getcwd, char *, buf, size_t, size)
/* stat requires custom implementation.  */
/* _syscall2 (int, lstat, const char *, path, struct stat *, st) */

/* Historically "open" flags defined by default in newlib and in Linux for ARC
   are different - this is true for some other architectures as well, e.g.
   ARM.  To provide compatibility ARC port of newlib had a custom fcntl.h file
   that has "open" flags identical to Linux ones.  Some other architectures
   (spart64, cris) override default fcntl.h as well, but I'm not sure this is
   really a good idea.  Unlike system call numbers that can be unique to each
   BSP in libgloss, "open" flags are not abstracted from the application code
   itself, hence it is not possible to have fcntl.h in the libgloss.  To make
   matters worse, existing simulators already has been built for the Linux-like
   "open" flags.  To preserve compatibility with existing hostlink
   implementations in simulators, but avoid custom fcntl.h in the future,
   simulator BSP has to do dynamic rewriting of "open" flags from the newlib
   default into old ARC Linux flags.  Simulators support only most basic flags,
   therefore only those are translated in this implementation.  */
int
_open (const char * pathname, int flags, int mode)
{
  int nsim_flags = 0;

  /* RDONLY, WRONLY, RDWR are same as newlib default.  */
  nsim_flags |= flags & O_RDONLY;
  nsim_flags |= flags & O_WRONLY;
  nsim_flags |= flags & O_RDWR;
  nsim_flags |= (flags & O_CREAT) ? ARC_LINUX_CREAT : 0;
  nsim_flags |= (flags & O_APPEND) ? ARC_LINUX_APPEND : 0;
  nsim_flags |= (flags & O_TRUNC) ? ARC_LINUX_TRUNC : 0;
  nsim_flags |= (flags & O_EXCL) ? ARC_LINUX_EXCL : 0;
  /* There are other fcntl flags that are different between newlib and ARC
     uClibc, however they are not supported by nSIM hostlink, therefore there
     is no need to translate them.  */

  long __res;
  _naked_syscall3 (__res, open, pathname, nsim_flags, mode)
  return __res;
}

/* Should be provided by crt0.S.  */
extern void __attribute__((noreturn)) _exit_halt ();

void
__attribute__((noreturn))
_exit (int ret)
{
  /* Doing an "exit" system call would work on nSIM with hostlink, but call to
     _exit_halt, which will do a CPU halt is more universal and will work in
     many other cases as well, including an FPGA/SoC.  */
  _exit_halt ();
}

/* This is a copy of newlib/libc/posix/_isatty.c.  It is needed because nSIM
   hostlink doesn't implement isatty system call.  Hardware boards on the other
   hand would want isatty implementation that always returns 1, since they are
   connected to console and doesn't have file IO.  */
int
_isatty (int fd)
{
  struct stat buf;

  if (fstat (fd, &buf) < 0)
    {
      errno = EBADF;
      return 0;
    }
  if (S_ISCHR (buf.st_mode))
    {
      return 1;
    }
  errno = ENOTTY;
  return 0;
}

/* System call "getpid" is implemented in nSIM hostlink, but it is better not
   to expose it in libgloss.  */
int
_getpid (void)
{
  return __MYPID;
}

/* System call "kill" is implemented in nSIM hostlink on Linux hosts, but it
   seems dangerous to expose it.  Instead, like most of the other "_kill"
   implementations in libgloss, this will kill only self.  */
int
_kill (int pid, int sig)
{
  if (pid == __MYPID)
    {
      _exit (sig);
    }
  errno = ENOSYS;
  return -1;
}

static void
translate_stat (struct nsim_stat *nsim, struct stat *buf)
{
  #define TR(field, type) buf->st_ ## field = (type) nsim->field
  TR (dev, dev_t);
  TR (ino, ino_t);
  TR (mode, mode_t);
  TR (nlink, nlink_t);
  TR (uid, uid_t);
  TR (gid, gid_t);
  TR (rdev, dev_t);
  TR (size, off_t);
  TR (atime, time_t);
  TR (mtime, time_t);
  TR (ctime, time_t);
  TR (blksize, long);
  TR (blocks, long);
  #undef TR
}

/* stat/fstat implementation.  Situation is similiar to open and its flags -
   structure is defined in libc, hence cannot be customized in libgloss, yet we
   have a case where nSIM uses some definition which is not compatible with
   neither old ARC-custom definition of "struct stat" in newlib, nor with
   generic newlib implementation.  */
int
_stat (const char * path, struct stat *buf)
{
  struct nsim_stat nsim_stat;
  long __res;
  _naked_syscall2 (__res, stat, path, &nsim_stat)
  translate_stat (&nsim_stat, buf);
  return __res;
}

int
_lstat (const char * path, struct stat *buf)
{
  struct nsim_stat nsim_stat;
  long __res;
  _naked_syscall2 (__res, stat, path, &nsim_stat)
  translate_stat (&nsim_stat, buf);
  return __res;
}

int
_fstat (int fd, struct stat *buf)
{
  struct nsim_stat nsim_stat;
  long __res;
  _naked_syscall2 (__res, stat, fd, &nsim_stat)
  translate_stat (&nsim_stat, buf);
  return __res;
}

/* Some system calls are implemented in nSIM hostlink, but are available only
   on Linux hosts.  To minimize potential compatibility issues they are by
   default disabled in libgloss build.  */
#ifdef ARC_NSIM_WIN32_HOST
_syscall3 (int, ioctl, int, fd, int, request, char *, argp)
_syscall3 (_ssize_t, readv, int, fd, const struct iovec *, iov, int, iovcnt)
_syscall3 (_ssize_t, writev, int, fd, const struct iovec *, iov, int, iovcnt)
_syscall5 (off_t, llseek, int, fd, unsigned long, offset_high,
	   unsigned long, offset_low, loff_t *, result,
	   unsigned int, whence)
_syscall2 (int, getrusage, int, who, struct rusage *, usage)
_syscall2 (int, setrlimit, int, resource, const struct rlimit *, rlim)
_syscall2 (int, getrlimit, int, resource, struct rlimit *, rlim)
_syscall3 (int, sigaction, int signum, const struct sigaction *, act,
	   struct sigaction *, oldact)
_syscall0 (uid_t, getuid)
_syscall0 (gid_t, getgid)
_syscall0 (uid_t, geteuid)
_syscall0 (gid_t, getegid)
_syscall2 (int, kill, pid_t, pid, int, sig)
_syscall3 (_ssize_t, readlink, const char *, path, char *, buf, size_t, bufsize)
#endif
