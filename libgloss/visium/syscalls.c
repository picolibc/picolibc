/* system calls for the Visium processor.

   Copyright (c) 2015 Rolls-Royce Controls and Data Services Limited.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of Rolls-Royce Controls and Data Services Limited nor
       the names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
   THE POSSIBILITY OF SUCH DAMAGE.  */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include "io.h"
#include "syscall.h"

#ifdef TARGET_SIM
struct file_register2
{
  unsigned action;
  unsigned p1, p2, p3, p4;
  unsigned error;
  unsigned retval;
};

extern struct file_register2 _sim_fileio_register;

static volatile struct file_register2 *const fileio = &_sim_fileio_register;

static int
do_syscall (unsigned action, unsigned p1, unsigned p2,
            unsigned p3, unsigned p4, int *error)
{
  fileio->p1 = p1;
  fileio->p2 = p2;
  fileio->p3 = p3;
  fileio->p4 = p4;
  fileio->action = action;

  *error = (int) fileio->error;
  return (int) fileio->retval;
}
#else
static int
do_syscall (unsigned action, unsigned p1, unsigned p2,
            unsigned p3, unsigned p4, int *error)
{
  int ret;
  int err;

  /* There is a two instruction delay after the software interrupt is
     initiated, to allow it to take effect. */

  asm volatile ("\n\
	move.l	r1,%3\n\
	move.l	r2,%4\n\
	move.l	r3,%5\n\
	moviu	r5,%%u 0x20002208\n\
	movil	r5,%%l 0x20002208\n\
	move.l	r4,%6\n\
	write.l	(r5),%2\n\
	nop\n\
	nop\n\
	move.l	%0,r1\n\
	move.l	%1,r2"
	: "=r" (ret), "=r" (err)
	: "r" (action), "r" (p1), "r" (p2), "r" (p3), "r" (p4)
	: "r1", "r2", "r3", "r4", "r5");

  *error = err;
  return ret;
}
#endif

int
close (int fildes)
{
  int status;
  int error;

  status = do_syscall (SYS_close, fildes, 0, 0, 0, &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);

  return status;
}

void _exit (int) __attribute ((__noreturn__));

void
_exit (int code)
{
#ifdef TARGET_SIM
  asm volatile ("stop    0,%0" : : "r" (code & 0xff));
#else
  int error;
  do_syscall (SYS_exit, code, 0, 0, 0, &error);
#endif

  /* Should never reach this point.  Since this function is not supposed to
     return, pretend to get stuck in a loop. */
  while (1)
    ;
}

#ifdef TARGET_SIM
extern long long _sim_cmdline_header;

long long
_get_cmdline (void)
{
  return _sim_cmdline_header;
}
#endif

int
fstat (int fildes, struct stat *st)
{
  struct gdb_stat gst;
  int status;
  int error;

  status = do_syscall (SYS_fstat, fildes, (unsigned) &gst, 0, 0, &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);
  else
    __hosted_from_gdb_stat (&gst, st);

  return status;
}

int
gettimeofday (struct timeval *__p, void *__tz)
{
  struct timeval *tv = __p;
  struct timezone *tz = __tz;
  struct gdb_timeval gtv;
  int status;
  int error;

  status = do_syscall (SYS_gettimeofday, (unsigned) &gtv, 0, 0, 0, &error);

  /* The timezone argument is not really supported so:
      Local time is GMT, no daylight saving */
  if (tz)
    {
      tz->tz_minuteswest = 0;
      tz->tz_dsttime = 0;
    }

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);
  else
    __hosted_from_gdb_timeval (&gtv, tv);

  return status;
}

int
isatty (int fildes)
{
  int status;
  int error;

  status = do_syscall (SYS_isatty, fildes, 0, 0, 0, &error);

  if (status == 0)
    errno = __hosted_from_gdb_errno (error);

  return status;
}

off_t
lseek (int fildes, off_t offset, int whence)
{
  off_t ret;
  int error;

  ret = do_syscall (SYS_lseek, fildes, offset,
		    __hosted_to_gdb_lseek_flags (whence), 0, &error);

  if (ret == (off_t)-1)
    errno = __hosted_from_gdb_errno (error);

  return ret;
}

int
open (const char *path, int oflag, ...)
{
  mode_t mode = 0;
  int status;
  int error;
  int len = strlen (path) + 1;

  if (oflag & O_CREAT)
    {
      va_list ap;
      va_start (ap, oflag);
      mode = va_arg (ap, mode_t);
      va_end (ap);
    }

  status = do_syscall (SYS_open, (unsigned) path, len,
		       __hosted_to_gdb_open_flags (oflag),
		       __hosted_to_gdb_mode_t (mode), &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);

  return status;
}

int
read (int fildes, void *buf, size_t nbyte)
{
  int status;
  int error;

  status = do_syscall (SYS_read, fildes, (unsigned) buf, nbyte, 0, &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);

  return status;
}

int
rename (const char *old, const char *new)
{
  int status;
  int error;
  int oldlen = strlen (old) + 1;
  int newlen = strlen (new) + 1;

  status = do_syscall (SYS_rename, (unsigned) old, oldlen, (unsigned) new,
		       newlen, &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);

  return status;
}

int
stat (const char *path, struct stat *st)
{
  struct gdb_stat gst;
  int status;
  int error;
  int len = strlen (path) + 1;

  status = do_syscall (SYS_stat, (unsigned) path, len, (unsigned) &gst, 0,
		       &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);
  else
    __hosted_from_gdb_stat (&gst, st);

  return status;
}

int
system (const char *string)
{
  int status;
  int error;
  int len = strlen (string) + 1;

  status = do_syscall (SYS_system, (unsigned) string, len, 0, 0, &error);

  return status;
}

int
unlink (const char *path)
{
  int status;
  int error;
  int len = strlen (path) + 1;

  status = do_syscall (SYS_unlink, (unsigned) path, len, 0, 0, &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);

  return status;
}

int
write (int fildes, const void *buf, size_t nbyte)
{
  int status;
  int error;

  status = do_syscall (SYS_write, fildes, (unsigned) buf, nbyte, 0, &error);

  if (status < 0)
    errno = __hosted_from_gdb_errno (error);

  return status;
}

extern clock_t _sim_clock;

clock_t
clock (void)
{
#ifdef TARGET_SIM
  return _sim_clock;
#else
  return (clock_t) -1;
#endif
}
