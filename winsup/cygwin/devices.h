/* devices.h

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <sys/ioctl.h>
#include <fcntl.h>

enum
{
  FH_RBINARY	= 0x00001000,	/* binary read mode */
  FH_WBINARY	= 0x00002000,	/* binary write mode */
  FH_CLOEXEC	= 0x00004000,	/* close-on-exec */
  FH_RBINSET	= 0x00008000,	/* binary read mode has been explicitly set */
  FH_WBINSET	= 0x00010000,	/* binary write mode has been explicitly set */
  FH_APPEND	= 0x00020000,	/* always append */
  FH_ASYNC	= 0x00040000,	/* async I/O */
  FH_SIGCLOSE	= 0x00080000,	/* signal handler should close fd on interrupt */

  FH_SYMLINK	= 0x00100000,	/* is a symlink */
  FH_EXECABL	= 0x00200000,	/* file looked like it would run:
				 * ends in .exe or .bat or begins with #! */
  FH_W95LSBUG	= 0x00400000,	/* set when lseek is called as a flag that
				 * _write should check if we've moved beyond
				 * EOF, zero filling if so. */
  FH_NOHANDLE	= 0x00800000,	/* No handle associated with fhandler. */
  FH_NOEINTR	= 0x01000000,	/* Set if I/O should be uninterruptible. */
  FH_FFIXUP	= 0x02000000,	/* Set if need to fixup after fork. */
  FH_LOCAL	= 0x04000000,	/* File is unix domain socket */
  FH_SHUTRD	= 0x08000000,	/* Socket saw a SHUT_RD */
  FH_SHUTWR	= 0x10000000,	/* Socket saw a SHUT_WR */
  FH_ISREMOTE	= 0x10000000,	/* File is on a remote drive */
  FH_DCEXEC	= 0x20000000,	/* Don't care if this is executable */
  FH_HASACLS	= 0x40000000,	/* True if fs of file has ACLS */
  FH_QUERYOPEN	= 0x80000000,	/* open file without requesting either read
				   or write access */

  /* Device flags */

  /* Slow devices */
  FH_CONSOLE = 0x00000001,	/* is a console */
  FH_CONIN   = 0x00000002,	/* console input */
  FH_CONOUT  = 0x00000003,	/* console output */
  FH_TTYM    = 0x00000004,	/* is a tty master */
  FH_TTYS    = 0x00000005,	/* is a tty slave */
  FH_PTYM    = 0x00000006,	/* is a pty master */
  FH_SERIAL  = 0x00000007,	/* is a serial port */
  FH_PIPE    = 0x00000008,	/* is a pipe */
  FH_PIPER   = 0x00000009,	/* read end of a pipe */
  FH_PIPEW   = 0x0000000a,	/* write end of a pipe */
  FH_SOCKET  = 0x0000000b,	/* is a socket */
  FH_WINDOWS = 0x0000000c,	/* is a window */
  FH_SLOW    = 0x00000010,	/* "slow" device if below this */

  /* Fast devices */
  FH_DISK    = 0x00000010,	/* is a disk */
  FH_FLOPPY  = 0x00000011,	/* is a floppy */
  FH_TAPE    = 0x00000012,	/* is a tape */
  FH_NULL    = 0x00000013,	/* is the null device */
  FH_ZERO    = 0x00000014,	/* is the zero device */
  FH_RANDOM  = 0x00000015,	/* is a random device */
  FH_MEM     = 0x00000016,	/* is a mem device */
  FH_CLIPBOARD = 0x00000017,	/* is a clipboard device */
  FH_OSS_DSP = 0x00000018,	/* is a dsp audio device */
  FH_CYGDRIVE= 0x00000019,	/* /cygdrive/x */
  FH_PROC    = 0x0000001a,      /* /proc */
  FH_REGISTRY =0x0000001b,      /* /proc/registry */
  FH_PROCESS = 0x0000001c,      /* /proc/<n> */

  FH_NDEV    = 0x0000001d,      /* Maximum number of devices */
  FH_DEVMASK = 0x00000fff,	/* devices live here */
  FH_BAD     = 0xffffffff
};

#define FHDEVN(n)	((n) & FH_DEVMASK)
#define FHISSETF(x)	__ISSETF (this, x, FH)
#define FHSETF(x)	__SETF (this, x, FH)
#define FHCLEARF(x)	__CLEARF (this, x, FH)
#define FHCONDSETF(n, x) __CONDSETF(n, this, x, FH)

#define FHSTATOFF	0
#endif
