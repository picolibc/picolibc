/* sys/signalfd.h: define signalfd(2) and struct signalfd_siginfo

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef	_SYS_SIGNALFD_H
#define	_SYS_SIGNALFD_H

#include <stdint.h>
#include <sys/_default_fcntl.h>

enum
{
  SFD_CLOEXEC = O_CLOEXEC,
  SFD_NONBLOCK = O_NONBLOCK
};
#define SFD_CLOEXEC SFD_CLOEXEC
#define SFD_NONBLOCK SFD_NONBLOCK

struct signalfd_siginfo
{
  uint32_t	ssi_signo;
  int32_t	ssi_errno;
  int32_t	ssi_code;
  uint32_t	ssi_pid;
  uint32_t	ssi_uid;
  int32_t	ssi_fd;
  uint32_t	ssi_tid;
  uint32_t	ssi_band;
  uint32_t	ssi_overrun;
  uint32_t	ssi_trapno;
  int32_t	ssi_status;
  int32_t	ssi_int;
  uint64_t	ssi_ptr;
  uint64_t	ssi_utime;
  uint64_t	ssi_stime;
  uint64_t	ssi_addr;
  uint8_t	pad[48];
};

#ifdef __cplusplus
extern "C" {
#endif

extern int signalfd (int, const sigset_t *, int);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SIGNALFD_H */
