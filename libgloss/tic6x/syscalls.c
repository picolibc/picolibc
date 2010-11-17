/* Copyright (c) 2010 CodeSourcery, Inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of CodeSourcery nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY CODESOURCERY, INC. ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CODESOURCERY BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
    DAMAGE.  */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

#define _DTOPEN 0xf0
#define _DTCLOSE 0xf1
#define _DTREAD 0xf2
#define _DTWRITE 0xf3
#define _DTLSEEK 0xf4
#define _DTUNLINK 0xf5
#define _DTGETENV 0xf6
#define _DTRENAME 0xf7
#define _DTGETTIME 0xf8
#define _DTGETCLK 0xf9
#define _DTSYNC 0xff

#define CIOBUFSIZ (BUFSIZ + 32)

struct __attribute__((packed)) cio_open_to_host
{
  /* Suggested file descriptor (little endian).  */
  short fd;
  /* Flags (little endian).  */
  short flags;
};

struct __attribute__((packed)) cio_open_from_host
{
  /* File descriptor (little endian).  */
  short fd;
};

struct __attribute__((packed)) cio_close_to_host
{
  /* File descriptor (little endian).  */
  short fd;
};

struct __attribute__((packed)) cio_close_from_host
{
  /* Result (little endian).  */
  short result;
};

struct __attribute__((packed)) cio_read_to_host
{
  /* File descriptor (little endian).  */
  short fd;
  /* Length (little endian).  */
  short length;
};

struct __attribute__((packed)) cio_read_from_host
{
  /* Result (little endian).  */
  short result;
};

struct __attribute__((packed)) cio_write_to_host
{
  /* File descriptor (little endian).  */
  short fd;
  /* Length (little endian).  */
  short length;
};

struct __attribute__((packed)) cio_write_from_host
{
  /* Result (little endian).  */
  short result;
};

struct __attribute__((packed)) cio_lseek_to_host
{
  /* File descriptor (little endian).  */
  short fd;
  /* Offset (little endian).  */
  int offset;
  /* Whence (little endian).  */
  short whence;
};

struct __attribute__((packed)) cio_lseek_from_host
{
  /* Result (little endian).  */
  int result;
};

struct __attribute__((packed)) cio_unlink_to_host
{
  /* Empty.  */
};

struct __attribute__((packed)) cio_unlink_from_host
{
  /* Result (little endian).  */
  short result;
};

struct __attribute__((packed)) cio_rename_to_host
{
  /* Empty.  */
};

struct __attribute__((packed)) cio_rename_from_host
{
  /* Result (little endian).  */
  short result;
};

struct __attribute__((packed)) cio_gettime_to_host
{
  /* Empty.  */
};

struct __attribute__((packed)) cio_gettime_from_host
{
  /* Time (little endian).  */
  int time;
};

struct __attribute__((packed)) cio_getclk_to_host
{
  /* Empty.  */
};

struct __attribute__((packed)) cio_getclk_from_host
{
  /* Clock cycles (little endian).  */
  int result;
};

struct __attribute__((packed)) cio_to_host
{
  /* Data length (target endian).  */
  unsigned int length;
  /* Command.  */
  unsigned char command;
  /* Parameters.  */
  union
  {
    unsigned char buf[8];
    struct cio_open_to_host open;
    struct cio_close_to_host close;
    struct cio_read_to_host read;
    struct cio_write_to_host write;
    struct cio_lseek_to_host lseek;
    struct cio_unlink_to_host unlink;
    struct cio_rename_to_host rename;
    struct cio_gettime_to_host gettime;
    struct cio_getclk_to_host getclk;
  } parms;
  /* Variable-length data.  */
  unsigned char data[];
};

struct __attribute__((packed)) cio_from_host
{
  /* Length (target endian).  */
  unsigned int length;
  /* Parameters.  */
  union
  {
    unsigned char buf[8];
    struct cio_open_from_host open;
    struct cio_close_from_host close;
    struct cio_read_from_host read;
    struct cio_write_from_host write;
    struct cio_lseek_from_host lseek;
    struct cio_unlink_from_host unlink;
    struct cio_rename_from_host rename;
    struct cio_gettime_from_host gettime;
    struct cio_getclk_from_host getclk;
  } parms;
  /* Data.  */
  unsigned char data[];
};

union
{
  unsigned char buf[CIOBUFSIZ];
  int align;
  union
  {
    struct cio_to_host to_host;
    struct cio_from_host from_host;
  } u;
} _CIOBUF_ __attribute__((section(".cio")));

#ifdef _BIG_ENDIAN
#define SWAPSHORT(s)	((short)((((s) & 0xff) << 8) | (((s) & 0xff00) >> 8)))
#define SWAPINT(i)	(__builtin_bswap32 (i))
#else
#define SWAPSHORT(s)	(s)
#define SWAPINT(i)	(i)
#endif

static void __attribute__((noinline))
do_semi_call (void)
{
  asm volatile (".globl C$$IO$$\nnop\nC$$IO$$:nop" : "+m" (_CIOBUF_));
}

static inline void
semi_call_wrapper (unsigned char command, const char *data,
		   unsigned int length)
{
  _CIOBUF_.u.to_host.length = length;
  _CIOBUF_.u.to_host.command = command;
  if (data != NULL)
    memcpy (_CIOBUF_.u.to_host.data, data, length);
  do_semi_call ();
}

static inline void
semi_call_wrapper2 (unsigned char command, const char *data1,
		    unsigned int length1, const char *data2,
		    unsigned int length2)
{
  _CIOBUF_.u.to_host.length = length1 + length2;
  _CIOBUF_.u.to_host.command = command;
  if (data1 != NULL)
    memcpy (_CIOBUF_.u.to_host.data, data1, length1);
  if (data2 != NULL)
    memcpy (_CIOBUF_.u.to_host.data + length1, data2, length2);
  do_semi_call ();
}

void
_exit (int status)
{
  /* The semihosting interface appears to provide no way to return an
     exit status.  */
  asm volatile (".globl C$$EXIT\nnop\nC$$EXIT:nop");
}

int
open (const char *path, int flags, ...)
{
  /* ??? It's not clear what the suggested fd is for.  */
  static short suggest_fd = 3;
  short ret_fd;
  ++suggest_fd;
  _CIOBUF_.u.to_host.parms.open.fd = SWAPSHORT (suggest_fd);
  _CIOBUF_.u.to_host.parms.open.flags = SWAPSHORT (flags);
  semi_call_wrapper (_DTOPEN, path, strlen (path) + 1);
  ret_fd = SWAPSHORT (_CIOBUF_.u.from_host.parms.open.fd);
  if (ret_fd == -1)
    return -1;
  return suggest_fd;
}

int
close (int fd)
{
  _CIOBUF_.u.to_host.parms.close.fd = SWAPSHORT (fd);
  semi_call_wrapper (_DTCLOSE, NULL, 0);
  return SWAPSHORT (_CIOBUF_.u.from_host.parms.close.result);
}

int
read (int fd, char *ptr, int len)
{
  if (len > BUFSIZ)
    len = BUFSIZ;
  _CIOBUF_.u.to_host.parms.read.fd = SWAPSHORT (fd);
  _CIOBUF_.u.to_host.parms.read.length = SWAPSHORT (len);
  semi_call_wrapper (_DTREAD, NULL, 0);
  memcpy (ptr, _CIOBUF_.u.from_host.data, _CIOBUF_.u.from_host.length);
  return SWAPSHORT (_CIOBUF_.u.from_host.parms.read.result);
}

int
write (int fd, char *ptr, int len)
{
  if (len > BUFSIZ)
    len = BUFSIZ;
  _CIOBUF_.u.to_host.parms.write.fd = SWAPSHORT (fd);
  _CIOBUF_.u.to_host.parms.write.length = SWAPSHORT (len);
  semi_call_wrapper (_DTWRITE, ptr, len);
  return SWAPSHORT (_CIOBUF_.u.from_host.parms.write.result);
}

int
lseek (int fd, int offset, int whence)
{
  _CIOBUF_.u.to_host.parms.lseek.fd = SWAPSHORT (fd);
  _CIOBUF_.u.to_host.parms.lseek.offset = SWAPINT (offset);
  _CIOBUF_.u.to_host.parms.lseek.whence = SWAPSHORT (whence);
  semi_call_wrapper (_DTLSEEK, NULL, 0);
  return SWAPINT (_CIOBUF_.u.from_host.parms.lseek.result);
}

int
unlink (const char *path)
{
  semi_call_wrapper (_DTUNLINK, path, strlen (path) + 1);
  return SWAPSHORT (_CIOBUF_.u.from_host.parms.unlink.result);
}

int
rename (const char *oldpath, const char *newpath)
{
  semi_call_wrapper2 (_DTRENAME, oldpath, strlen (oldpath) + 1,
		      newpath, strlen (newpath) + 1);
  return SWAPSHORT (_CIOBUF_.u.from_host.parms.rename.result);
}

int
gettimeofday (struct timeval *tp, void *tzvp)
{
  struct timezone *tzp = tzvp;

  if (tp)
    {
      semi_call_wrapper (_DTGETTIME, NULL, 0);
      tp->tv_sec = SWAPINT (_CIOBUF_.u.from_host.parms.gettime.time);
      tp->tv_usec = 0;
    }

  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}

clock_t
clock (void)
{
  semi_call_wrapper (_DTGETCLK, NULL, 0);
  return SWAPINT (_CIOBUF_.u.from_host.parms.getclk.result);
}


int
isatty (int file __attribute__((unused)))
{
  errno = ENOSYS;
  return 0;
}

int
fstat (int fd, struct stat *buf)
{
  buf->st_mode = S_IFCHR;	/* Always pretend to be a tty */
  buf->st_blksize = 0;

  return (0);
}
