/* fhandler_floppy.cc.  See fhandler.h for a description of the
   fhandler classes.

   Copyright 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "fhandler.h"
#include "cygerrno.h"

/**********************************************************************/
/* fhandler_dev_floppy */

int
fhandler_dev_floppy::is_eom (int win_error)
{
  int ret = (win_error == ERROR_INVALID_PARAMETER);
  if (ret)
    debug_printf ("end of medium");
  return ret;
}

int
fhandler_dev_floppy::is_eof (int)
{
  int ret = 0;
  if (ret)
    debug_printf ("end of file");
  return ret;
}

fhandler_dev_floppy::fhandler_dev_floppy (const char *name, int unit) : fhandler_dev_raw (FH_FLOPPY, name, unit)
{
  set_cb (sizeof *this);
}

int
fhandler_dev_floppy::open (const char *path, int flags, mode_t)
{
  /* The correct size of the buffer would be 512 bytes,
   * which is the atomic size, supported by WinNT.
   * Unfortunately, the performance is worse than
   * access to file system on same device!
   * Setting buffer size to a relatively big value
   * increases performance by means.
   * The new ioctl call with 'rdevio.h' header file
   * supports changing this value.
   *
   * Let's be smart: Let's take a multiplier of typical tar
   * and cpio buffer sizes by default!
  */
  devbufsiz = 61440L; /* 512L; */
  return fhandler_dev_raw::open (path, flags);
}

int
fhandler_dev_floppy::close (void)
{
  int ret;

  ret = writebuf ();
  if (ret)
    {
      fhandler_dev_raw::close ();
      return ret;
    }
  return fhandler_dev_raw::close ();
}

off_t
fhandler_dev_floppy::lseek (off_t offset, int whence)
{
  int ret;
  DWORD off;
  char buf[512];

  if (whence == SEEK_SET)
    {
      if (offset < 0)
        {
	  set_errno (EINVAL);
	  return -1;
	}

      /* Invalidate buffer. */
      ret = writebuf ();
      if (ret)
	return ret;
      devbufstart = devbufend = 0;

      off = (offset / 512) * 512;

      if (SetFilePointer (get_handle (), off, NULL, FILE_BEGIN)
          == INVALID_SET_FILE_POINTER)
        {
	  __seterrno ();
	  return -1;
	}
      return raw_read (buf, offset - off);
    }
  else if (whence == SEEK_CUR)
    {
      DWORD low;
      LONG high = 0;

      low = SetFilePointer (get_handle (), 0, &high, FILE_CURRENT);
      if (low == INVALID_SET_FILE_POINTER && GetLastError ())
	{
	  __seterrno ();
	  return -1;
	}
      long long cur = (long long) high << 32 + low;
      if (is_writing)
	cur += devbufend - devbufstart;
      else
	cur -= devbufend - devbufstart;
      
      /* Invalidate buffer. */
      ret = writebuf ();
      if (ret)
	return ret;
      devbufstart = devbufend = 0;

      cur += offset;
      if (cur < 0)
	{
	  set_errno (EINVAL);
	  return -1;
	}

      long long set = (cur / 512) * 512;
      high = set >> 32;
      low = set & 0xffffffff;

      off = cur - set;

      low = SetFilePointer (get_handle (), low, &high, FILE_BEGIN);
      if (low == INVALID_SET_FILE_POINTER && GetLastError ())
	{
	  __seterrno ();
	  return -1;
	}
      return raw_read (buf, off);
    }
  /* SEEK_END is not supported on raw disk devices. */
  set_errno (EINVAL);
  return -1;
}

int
fhandler_dev_floppy::ioctl (unsigned int cmd, void *buf)
{
  return fhandler_dev_raw::ioctl (cmd, buf);
}

