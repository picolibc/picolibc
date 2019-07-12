/* fhandler_signalfd.cc: fhandler for signalfd

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "path.h"
#include "fhandler.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include <cygwin/signal.h>
#include <sys/signalfd.h>

fhandler_signalfd::fhandler_signalfd () :
  fhandler_base (),
  sigset (0)
{
}

char *
fhandler_signalfd::get_proc_fd_name (char *buf)
{
  return strcpy (buf, "anon_inode:[signalfd]");
}

int
fhandler_signalfd::signalfd (const sigset_t *mask, int flags)
{
  __try
    {
      sigset = *mask & ~(SIGKILL | SIGSTOP);
    }
  __except (EINVAL)
    {
      return -1;
    }
  __endtry
  if (flags & SFD_NONBLOCK)
    set_nonblocking (true);
  if (flags & SFD_CLOEXEC)
    set_close_on_exec (true);
  if (get_unique_id () == 0)
    {
      nohandle (true);
      set_unique_id ();
      set_ino (get_unique_id ());
      set_flags (O_RDWR | O_BINARY);
    }
  return 0;
}

int __reg2
fhandler_signalfd::fstat (struct stat *buf)
{
  int ret = fhandler_base::fstat (buf);
  if (!ret)
    {
      buf->st_mode = S_IRUSR | S_IWUSR;
      buf->st_dev = FH_SIGNALFD;
      buf->st_ino = get_unique_id ();
    }
  return ret;
}

static inline void
copy_siginfo_to_signalfd (struct signalfd_siginfo *sfd,
			  const siginfo_t * const si)
{
  sfd->ssi_signo = si->si_signo;
  sfd->ssi_errno = si->si_errno;
  sfd->ssi_code = si->si_code;
  sfd->ssi_pid = si->si_pid;
  sfd->ssi_uid = si->si_uid;
  sfd->ssi_fd = -1;
  sfd->ssi_tid = si->si_tid;
  sfd->ssi_band = 0;
  sfd->ssi_overrun = si->si_overrun;
  sfd->ssi_trapno = 0;
  sfd->ssi_status = si->si_status;
  sfd->ssi_int = si->si_value.sival_int;
  sfd->ssi_ptr = (uint64_t) si->si_value.sival_ptr;
  sfd->ssi_utime = si->si_utime;
  sfd->ssi_stime = si->si_stime;
  sfd->ssi_addr = (uint64_t) si->si_addr;
}

void __reg3
fhandler_signalfd::read (void *ptr, size_t& len)
{
  const LARGE_INTEGER poll = { QuadPart : 0 };
  siginfo_t si;
  int ret, old_errno;
  size_t curlen = 0;
  signalfd_siginfo *sfd_ptr = (signalfd_siginfo *) ptr;

  if (len < sizeof (struct signalfd_siginfo))
    {
      set_errno (EINVAL);
      len = (size_t) -1;
      return;
    }
  old_errno = get_errno ();
  do
    {
      /* Even when read is blocking, only one pending signal is actually
	 required to return.  Subsequently use sigtimedwait to just poll
	 if some more signal is available. */
      ret = sigwait_common (&sigset, &si, (is_nonblocking () || curlen)
					  ? (PLARGE_INTEGER) &poll : NULL);
      if (ret == -1)
	{
	  /* If we already read a signal so the buffer isn't empty, just
	     return success. */
	  if (curlen > 0)
	    break;
	  len = -1;
	  return;
	}
      __try
	{
	  copy_siginfo_to_signalfd (sfd_ptr, &si);
	}
      __except (EFAULT)
	{
	  len = (size_t) -1;
	  return;
	}
      __endtry
      sfd_ptr++;
      curlen += sizeof (*sfd_ptr);
    }
  while ((len - curlen >= sizeof (struct signalfd_siginfo)));
  set_errno (old_errno);
  len = curlen;
  return;
}

ssize_t __stdcall
fhandler_signalfd::write (const void *, size_t)
{
  set_errno (EINVAL);
  return -1;
}

/* Called from select */
int
fhandler_signalfd::poll ()
{
  sigset_t outset = sig_send (myself, __SIGPENDING, &_my_tls);
  if (outset == SIG_BAD_MASK)
    return -1;
  if ((outset & sigset) != 0)
    return 0;
  return -1;
}
