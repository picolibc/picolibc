/* fhandler_process_fd.cc: fhandler for the /dev/{fd,std{in,out,err}} symlinks

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "path.h"
#include "fhandler.h"

fhandler_dev_fd::fhandler_dev_fd ():
  fhandler_virtual ()
{
}

virtual_ftype_t
fhandler_dev_fd::exists ()
{
  return virt_symlink;
}

int
fhandler_dev_fd::fstat (struct stat *buf)
{
  const char *path = get_name ();
  debug_printf ("fstat (%s)", path);

  fhandler_base::fstat (buf);

  buf->st_mode = S_IFLNK | STD_RBITS | S_IWUSR | S_IWGRP | S_IWOTH | STD_XBITS;
  buf->st_ino = get_ino ();

  return 0;
}

bool
fhandler_dev_fd::fill_filebuf ()
{
  const char *path = get_name ();
  debug_printf ("fill_filebuf (%s)", path);

  const char *native = get_native_name ();
  if (!native)
    {
      return false;
    }

  free(filebuf);
  filebuf = cstrdup (native);
  return true;
}
