/* fhandler_nodevice.cc.  "No such device" handler.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"

int
fhandler_nodevice::open (int flags, mode_t)
{
  if (!pc.error)
    set_errno (ENXIO);
  /* Fixup EROFS error returned from path_conv if /dev is not backed by real
     directory on disk and the file doesn't exist. */
  else if (pc.error == EROFS && (flags & O_ACCMODE) == O_RDONLY)
    set_errno (ENOENT);
  return 0;
}

fhandler_nodevice::fhandler_nodevice ()
{
}
