/* fcntl.cc: fcntl syscall

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"

extern "C" int
fcntl (int fd, int cmd, ...)
{
  int res = -1;
  intptr_t arg = 0;
  va_list args;

  pthread_testcancel ();

  __try
    {

      debug_printf ("fcntl(%d, %d, ...)", fd, cmd);

      /* Don't lock the fd table when performing locking calls. */
      cygheap_fdget cfd (fd, cmd < F_GETLK || cmd > F_SETLKW);
      if (cfd < 0)
	__leave;

      /* FIXME?  All numerical args to fcntl are defined as long on Linux.
	 This relies on a really dirty trick on x86_64:  A 32 bit mov to
	 a register (e.g. mov $1, %edx) always sets the high 32 bit to 0.
	 We're following the Linux lead here since the third arg to any
	 function is in a register anyway (%r8 in MS ABI).  That's the easy
	 case which is covered here by always reading the arg with
	 sizeof (intptr_t) == sizeof (long) == sizeof (void*) which matches
	 all targets.

	 However, the POSIX standard defines all numerical args as type int.
	 If we take that literally, we had a (small) problem on 64 bit, since
	 sizeof (void*) != sizeof (int).  If we would like to follow POSIX more
	 closely than Linux, we'd have to call va_arg on a per cmd basis. */

      va_start (args, cmd);
      arg = va_arg (args, intptr_t);
      va_end (args);

      switch (cmd)
	{
	case F_DUPFD:
	case F_DUPFD_CLOEXEC:
	  if (arg >= 0 && arg < OPEN_MAX)
	    {
	      int flags = cmd == F_DUPFD_CLOEXEC ? O_CLOEXEC : 0;
	      res = cygheap->fdtab.dup3 (fd, cygheap_fdnew ((arg) - 1), flags);
	    }
	  else
	    {
	      set_errno (EINVAL);
	      res = -1;
	    }
	  break;
	default:
	  res = cfd->fcntl (cmd, arg);
	  break;
	}
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = fcntl(%d, %d, %ly)", res, fd, cmd, arg);
  return res;
}

EXPORT_ALIAS (fcntl, _fcntl)
