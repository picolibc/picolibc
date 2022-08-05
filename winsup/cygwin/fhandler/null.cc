/* null.cc.  /dev/null specifics.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "path.h"
#include "fhandler.h"

fhandler_dev_null::fhandler_dev_null () :
	fhandler_base ()
{
}

ssize_t
fhandler_dev_null::write (const void *ptr, size_t len)
{
  /* Shortcut.  This also fixes a problem with the NUL device on x86_64:
     If you write > 4 GB in a single attempt, the bytes written returned
     from by is numBytes & 0xffffffff. */
  return len;
}

