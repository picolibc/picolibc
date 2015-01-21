/* I/O stub functions for the Visium processor.

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
#include <sys/times.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <errno.h>

struct DIR;

int
chdir (const char *path)
{
  errno = ENOSYS;
  return -1;
}

int
chmod (const char *path, mode_t mode)
{
  errno = ENOSYS;
  return -1;
}

int
closedir (struct DIR *dirp)
{
  errno = ENOSYS;
  return -1;
}

int
execv (const char *path, char *const argv[])
{
  errno = ENOSYS;
  return -1;
}

int
fcntl (int fd, int cmd, ...)
{
  errno = ENOSYS;
  return -1;
}

int
fork (void)
{
  errno = EAGAIN;
  return -1;
}

char *
getcwd (char *buf, size_t size)
{
  buf[0] = 0;
  return buf;
}

pid_t
getppid (void)
{
  errno = ENOSYS;
  return -1;
}

int
link (const char *old, const char *new)
{
  errno = EMLINK;
  return -1;
}

int
nanosleep (const struct timespec *requested_time,
           struct timespec *remaining)
{
  remaining->tv_sec = 0;
  remaining->tv_nsec = 0;
  return 0;
}

struct DIR *
opendir (const char *dirname)
{
  errno = ENOSYS;
  return NULL;
}

struct dirent *
readdir (struct DIR *dirp)
{
  errno = ENOSYS;
  return NULL;
}

ssize_t
readlink (const char *__path, char *__buf, size_t __buflen)
{
  errno = ENOSYS;
  return -1;
}

int
rmdir (const char *path)
{
  errno = ENOSYS;
  return -1;
}

int
symlink (const char *name1, const char *name2)
{
  errno = ENOSYS;
  return -1;
}

clock_t
times (struct tms *buf)
{
  errno = ENOSYS;
  return -1;
}

char *
ttyname (int fildes)
{
  return "";
}

int
utime (const char *path, const struct utimbuf *times)
{
  errno = ENOSYS;
  return -1;
}

int
wait (int *status)
{
  errno = ENOSYS;
  return -1;
}

pid_t
waitpid (pid_t pid, int *stat_loc, int options)
{
  errno = ENOSYS;
  return -1;
}
