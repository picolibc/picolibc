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
#include <winioctl.h>

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
  char buf[512];
  long long drive_size = 0;
  long long lloffset = offset;
  long long current_position;
  off_t sector_aligned_offset;
  off_t bytes_left;
  DWORD low;
  LONG high = 0;

  if (os_being_run == winNT)
    {
      DISK_GEOMETRY di;
      PARTITION_INFORMATION pi;
      DWORD bytes_read;

      if ( !DeviceIoControl ( get_handle(),
                              IOCTL_DISK_GET_DRIVE_GEOMETRY,
                              NULL, 0,
                              &di, sizeof (di),
                              &bytes_read, NULL) )
        {
          __seterrno ();
          return -1;
        }
      debug_printf ( "disk geometry: (%ld cyl)*(%ld trk)*(%ld sec)*(%ld bps)",
                     di.Cylinders.LowPart,
                     di.TracksPerCylinder,
                     di.SectorsPerTrack,
                     di.BytesPerSector );
      if ( DeviceIoControl ( get_handle (),
                             IOCTL_DISK_GET_PARTITION_INFO,
                             NULL, 0,
                             &pi, sizeof (pi),
                             &bytes_read, NULL ))
        {
          debug_printf ( "partition info: %ld (%ld)",
                          pi.StartingOffset.LowPart,
                          pi.PartitionLength.LowPart);
          drive_size = (long long) pi.PartitionLength.QuadPart;
        }
      else
        {
          drive_size = (long long) di.Cylinders.QuadPart * di.TracksPerCylinder *
                       di.SectorsPerTrack * di.BytesPerSector;
        }
      debug_printf ( "drive size: %ld", drive_size );
    }

  if (whence == SEEK_END && drive_size > 0)
    {
      lloffset = offset + drive_size;
      whence = SEEK_SET; 
    }

  if (whence == SEEK_CUR)
    {
      low = SetFilePointer (get_handle (), 0, &high, FILE_CURRENT);
      if (low == INVALID_SET_FILE_POINTER && GetLastError ())
	{
	  __seterrno ();
	  return -1;
	}
      current_position = (long long) low + ((long long) high << 32);
      if (is_writing)
	current_position += devbufend - devbufstart;
      else
	current_position -= devbufend - devbufstart;

      lloffset += current_position;
      whence = SEEK_SET;
    }

  if (lloffset < 0 ||
      drive_size > 0 && lloffset > drive_size)
    {
      set_errno (EINVAL);
      return -1;
    }
  high = lloffset >> 32;
  low = lloffset & 0xffffffff;
  if ( high || (off_t) low < 0 )
    {
      set_errno (EFBIG);
      return -1;
    }
  offset = (off_t) low;

  /* FIXME: sector can possibly be not 512 bytes long */
  sector_aligned_offset = (offset / 512) * 512;
  bytes_left = offset - sector_aligned_offset;

  if (whence == SEEK_SET)
    {
      /* Invalidate buffer. */
      ret = writebuf ();
      if (ret)
	return ret;
      devbufstart = devbufend = 0;

      if (SetFilePointer (get_handle (), sector_aligned_offset, NULL, FILE_BEGIN)
          == INVALID_SET_FILE_POINTER)
        {
	  __seterrno ();
	  return -1;
	}
      return sector_aligned_offset + raw_read (buf, bytes_left);
    }

  set_errno (EINVAL);
  return -1;
}

int
fhandler_dev_floppy::ioctl (unsigned int cmd, void *buf)
{
  return fhandler_dev_raw::ioctl (cmd, buf);
}

