/* fhandler_timerfd.cc: fhandler for timerfd

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
#include "timer.h"
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

#define cnew(name, ...) \
  ({ \
    void* ptr = (void*) ccalloc (HEAP_FHANDLER, 1, sizeof (name)); \
    ptr ? new (ptr) name (__VA_ARGS__) : NULL; \
  })

int
fhandler_timerfd::timerfd (clockid_t clock_id, int flags)
{
  timerid = (timer_t) cnew (timer_tracker, clock_id, NULL, true);
  if (flags & TFD_NONBLOCK)
    set_nonblocking (true);
  if (flags & TFD_CLOEXEC)
    set_close_on_exec (true);
  if (get_unique_id () == 0)
    {
      nohandle (true);
      set_unique_id ();
      set_ino (get_unique_id ());
    }
  return 0;
}

int
fhandler_timerfd::settime (int flags, const itimerspec *value,
			   itimerspec *ovalue)
{
  int ret = -1;

  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      ret = tt->settime (flags, value, ovalue);
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

int
fhandler_timerfd::gettime (itimerspec *ovalue)
{
  int ret = -1;

  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      tt->gettime (ovalue);
      ret = 0;
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
      timer_tracker *tt = (timer_tracker *) timerid;
      LONG64 ret = tt->wait (is_nonblocking ());
      if (ret == -1)
	__leave;
      PLONG64 pl64 = (PLONG64) ptr;
      *pl64 = ret + 1;
      len = sizeof (LONG64);
      return;
    }
  __except (EFAULT) {}
  __endtry
  len = (size_t) -1;
  return;
}

HANDLE
fhandler_timerfd::get_timerfd_handle ()
{
  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      return tt->get_timerfd_handle ();
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
	  timer_tracker *tt = (timer_tracker *) fhc->timerid;
	  tt->increment_instances ();
	  ret = 0;
	}
      __except (EFAULT) {}
      __endtry
    }
  return ret;
}

void
fhandler_timerfd::fixup_after_exec ()
{
  if (close_on_exec ())
    return;
  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      tt->fixup_after_exec ();
    }
  __except (EFAULT) {}
  __endtry
}

int
fhandler_timerfd::ioctl (unsigned int cmd, void *p)
{
  int ret = -1;

  switch (cmd)
    {
    case TFD_IOC_SET_TICKS:
      /* TODO */
      ret = 0;
      break;
    default:
      set_errno (EINVAL);
      break;
    }
  syscall_printf ("%d = ioctl_timerfd(%x, %p)", ret, cmd, p);
  return ret;
}

int
fhandler_timerfd::close ()
{
  int ret = -1;

  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      timer_tracker::close (tt);
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}
