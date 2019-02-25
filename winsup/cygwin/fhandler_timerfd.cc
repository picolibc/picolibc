/* fhandler_timerfd.cc: fhandler for timerfd, public timerfd API

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
#include "timerfd.h"
#include <sys/timerfd.h>
#include <cygwin/signal.h>

fhandler_timerfd::fhandler_timerfd () :
  fhandler_base ()
{
}

char *
fhandler_timerfd::get_proc_fd_name (char *buf)
{
  return strcpy (buf, "anon_inode:[timerfd]");
}

/* The timers connected to a descriptor are stored on the cygheap
   together with their fhandler. */

int
fhandler_timerfd::timerfd (clockid_t clock_id, int flags)
{
  timerfd_tracker *tfd = (timerfd_tracker *)
			 ccalloc (HEAP_FHANDLER, 1, sizeof (timerfd_tracker));
  if (!tfd)
    {
      set_errno (ENOMEM);
      return -1;
    }
  new (tfd) timerfd_tracker ();
  int ret = tfd->create (clock_id);
  if (ret < 0)
    {
      cfree (tfd);
      set_errno (-ret);
      return -1;
    }
  if (flags & TFD_NONBLOCK)
    set_nonblocking (true);
  if (flags & TFD_CLOEXEC)
    set_close_on_exec (true);
  nohandle (true);
  set_unique_id ();
  set_ino (get_unique_id ());
  set_flags (O_RDWR | O_BINARY);
  timerid = (timer_t) tfd;
  return 0;
}

int
fhandler_timerfd::settime (int flags, const struct itimerspec *new_value,
			   struct itimerspec *old_value)
{
  int ret = -1;

  __try
    {
      timerfd_tracker *tfd = (timerfd_tracker *) timerid;
      ret = tfd->settime (flags, new_value, old_value);
      if (ret < 0)
	{
	  set_errno (-ret);
	  ret = -1;
	}
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

int
fhandler_timerfd::gettime (struct itimerspec *ovalue)
{
  int ret = -1;

  __try
    {
      timerfd_tracker *tfd = (timerfd_tracker *) timerid;
      ret = tfd->gettime (ovalue);
      if (ret < 0)
	{
	  set_errno (-ret);
	  ret = -1;
	}
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

int __reg2
fhandler_timerfd::fstat (struct stat *buf)
{
  int ret = fhandler_base::fstat (buf);
  if (!ret)
    {
      buf->st_mode = S_IRUSR | S_IWUSR;
      buf->st_dev = FH_TIMERFD;
      buf->st_ino = get_unique_id ();
    }
  return ret;
}

void __reg3
fhandler_timerfd::read (void *ptr, size_t& len)
{
  if (len < sizeof (LONG64))
    {
      set_errno (EINVAL);
      len = (size_t) -1;
      return;
    }

  __try
    {
      timerfd_tracker *tfd = (timerfd_tracker *) timerid;
      LONG64 ret = tfd->wait (is_nonblocking ());
      if (ret < 0)
	{
	  set_errno (-ret);
	  __leave;
	}
      *(PLONG64) ptr = ret;
      len = sizeof (LONG64);
      return;
    }
  __except (EFAULT) {}
  __endtry
  len = (size_t) -1;
  return;
}

ssize_t __stdcall
fhandler_timerfd::write (const void *, size_t)
{
  set_errno (EINVAL);
  return -1;
}

HANDLE
fhandler_timerfd::get_timerfd_handle ()
{
  __try
    {
      timerfd_tracker *tfd = (timerfd_tracker *) timerid;
      return tfd->get_timerfd_handle ();
    }
  __except (EFAULT) {}
  __endtry
  return NULL;
}

int
fhandler_timerfd::dup (fhandler_base *child, int flags)
{
  int ret = fhandler_base::dup (child, flags);

  if (!ret)
    {
      fhandler_timerfd *fhc = (fhandler_timerfd *) child;
      __try
	{
	  timerfd_tracker *tfd = (timerfd_tracker *) fhc->timerid;
	  tfd->dup ();
	  ret = 0;
	}
      __except (EFAULT) {}
      __endtry
    }
  return ret;
}

int
fhandler_timerfd::ioctl (unsigned int cmd, void *p)
{
  int ret = -1;
  uint64_t exp_cnt;

  switch (cmd)
    {
    case TFD_IOC_SET_TICKS:
      __try
	{
	  timerfd_tracker *tfd = (timerfd_tracker *) timerid;

	  exp_cnt = *(uint64_t *) p;
	  ret = tfd->ioctl_set_ticks (exp_cnt);
	  if (ret < 0)
	    set_errno (-ret);
	}
      __except (EFAULT) {}
      __endtry
      break;
    default:
      ret = fhandler_base::ioctl (cmd, p);
      break;
    }
  syscall_printf ("%d = ioctl_timerfd(%x, %p)", ret, cmd, p);
  return ret;
}

void
fhandler_timerfd::fixup_after_fork (HANDLE)
{
  __try
    {
      timerfd_tracker *tfd = (timerfd_tracker *) timerid;
      tfd->fixup_after_fork ();
    }
  __except (EFAULT) {}
  __endtry
}

void
fhandler_timerfd::fixup_after_exec ()
{
  __try
    {
      timerfd_tracker *tfd = (timerfd_tracker *) timerid;
      tfd->init_fixup_after_fork_exec ();
      if (close_on_exec ())
	timerfd_tracker::dtor (tfd);
      else
	tfd->fixup_after_exec ();
    }
  __except (EFAULT) {}
  __endtry
}

int
fhandler_timerfd::close ()
{
  int ret = -1;

  __try
    {
      timerfd_tracker *tfd = (timerfd_tracker *) timerid;
      timerfd_tracker::dtor (tfd);
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

extern "C" int
timerfd_create (clockid_t clock_id, int flags)
{
  int ret = -1;
  fhandler_timerfd *fh;

  debug_printf ("timerfd_create (%lu, %y)", clock_id, flags);

  if (clock_id != CLOCK_REALTIME
      && clock_id != CLOCK_MONOTONIC
      && clock_id != CLOCK_BOOTTIME)
    {
      set_errno (EINVAL);
      goto done;
    }
  if ((flags & ~(TFD_NONBLOCK | TFD_CLOEXEC)) != 0)
    {
      set_errno (EINVAL);
      goto done;
    }

    {
      /* Create new timerfd descriptor. */
      cygheap_fdnew fd;

      if (fd < 0)
        goto done;
      fh = (fhandler_timerfd *) build_fh_dev (*timerfd_dev);
      if (fh && fh->timerfd (clock_id, flags) == 0)
        {
          fd = fh;
          if (fd <= 2)
            set_std_handle (fd);
          ret = fd;
        }
      else
        delete fh;
    }

done:
  syscall_printf ("%R = timerfd_create (%lu, %y)", ret, clock_id, flags);
  return ret;
}

extern "C" int
timerfd_settime (int fd_in, int flags, const struct itimerspec *value,
		 struct itimerspec *ovalue)
{
  if ((flags & ~(TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET)) != 0)
    {
      set_errno (EINVAL);
      return -1;
    }

  cygheap_fdget fd (fd_in);
  if (fd < 0)
    return -1;
  fhandler_timerfd *fh = fd->is_timerfd ();
  if (!fh)
    {
      set_errno (EINVAL);
      return -1;
    }
  return fh->settime (flags, value, ovalue);
}

extern "C" int
timerfd_gettime (int fd_in, struct itimerspec *ovalue)
{
  cygheap_fdget fd (fd_in);
  if (fd < 0)
    return -1;
  fhandler_timerfd *fh = fd->is_timerfd ();
  if (!fh)
    {
      set_errno (EINVAL);
      return -1;
    }
  return fh->gettime (ovalue);
}
