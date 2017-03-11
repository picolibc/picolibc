/* getentropy.cc: getentropy/getrandmom functions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <ntsecapi.h>
#include <sys/param.h>
#include <sys/random.h>
#include "cygtls.h"
#include "ntdll.h"

extern "C" int
getentropy (void *ptr, size_t len)
{
  /* Per BSD man page: The maximum buffer size permitted is 256 bytes.
     If buflen exceeds this, an error of EIO will be indicated. */
  if (len > 256)
    {
      debug_printf ("len (%U) > 256", len);
      set_errno (EIO);
      return -1;
    }
  __try
    {
      if (!RtlGenRandom (ptr, len))
	{
	  debug_printf ("RtlGenRandom() = FALSE");
	  set_errno (EIO);
	  return -1;
	}
    }
  __except (EFAULT)
    {
      return -1;
    }
  __endtry
  return 0;
}

extern "C" ssize_t
getrandom (void *ptr, size_t len, unsigned int flags)
{
  if (flags & ~(GRND_NONBLOCK | GRND_RANDOM))
    {
      debug_printf ("invalid flags: %y", flags);
      set_errno (EINVAL);
      return -1;
    }
  /* Max. bytes returned by Linux call. */
  len = MIN (len, (flags & GRND_RANDOM) ? 512 : 33554431);
  __try
    {
      if (!RtlGenRandom (ptr, len))
	{
	  debug_printf ("RtlGenRandom() = FALSE");
	  set_errno (EIO);
	  return -1;
	}
    }
  __except (EFAULT)
    {
      return -1;
    }
  __endtry
  return len;
}
