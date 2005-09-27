/* fhandler_floppy.cc.  See fhandler.h for a description of the
   fhandler classes.

   Copyright 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/termios.h>
#include <unistd.h>
#include <winioctl.h>
#include <asm/socket.h>
#include <cygwin/hdreg.h>
#include <cygwin/fs.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"

/**********************************************************************/
/* fhandler_dev_floppy */

int
fhandler_dev_floppy::is_eom (int win_error)
{
  int ret = (win_error == ERROR_INVALID_PARAMETER
	     || win_error == ERROR_SEEK
	     || win_error == ERROR_SECTOR_NOT_FOUND);
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

fhandler_dev_floppy::fhandler_dev_floppy ()
  : fhandler_dev_raw ()
{
}

int
fhandler_dev_floppy::get_drive_info (struct hd_geometry *geo)
{
  char dbuf[256];
  char pbuf[256];

  DISK_GEOMETRY *di;
  PARTITION_INFORMATION_EX *pix = NULL;
  PARTITION_INFORMATION *pi = NULL;
  DWORD bytes_read = 0;

  /* Always try using the new EX ioctls first (>= XP).  If not available,
     fall back to trying the old non-EX ioctls. */
  if (!DeviceIoControl (get_handle (),
			IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0,
			dbuf, 256, &bytes_read, NULL))
    {
      if (!DeviceIoControl (get_handle (),
			    IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
			    dbuf, 256, &bytes_read, NULL))
	{
	  __seterrno ();
	  return -1;
	}
      di = (DISK_GEOMETRY *) dbuf;
    }
  else
    di = &((DISK_GEOMETRY_EX *) dbuf)->Geometry;
    
  if (DeviceIoControl (get_handle (),
		       IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0,
		       pbuf, 256, &bytes_read, NULL))
    pix = (PARTITION_INFORMATION_EX *) pbuf;
  else if (DeviceIoControl (get_handle (),
			    IOCTL_DISK_GET_PARTITION_INFO, NULL, 0,
			    pbuf, 256, &bytes_read, NULL))
    pi = (PARTITION_INFORMATION *) pbuf;

  debug_printf ("disk geometry: (%ld cyl)*(%ld trk)*(%ld sec)*(%ld bps)",
		 di->Cylinders.LowPart,
		 di->TracksPerCylinder,
		 di->SectorsPerTrack,
		 di->BytesPerSector);
  bytes_per_sector = di->BytesPerSector;
  if (pix)
    {
      debug_printf ("partition info: offset %D  length %D",
		    pix->StartingOffset.QuadPart,
		    pix->PartitionLength.QuadPart);
      drive_size = pix->PartitionLength.QuadPart;
    }
  else if (pi)
    {
      debug_printf ("partition info: offset %D  length %D",
		    pi->StartingOffset.QuadPart,
		    pi->PartitionLength.QuadPart);
      drive_size = pi->PartitionLength.QuadPart;
    }
  else
    {
      /* Getting the partition size by using the drive geometry information
         looks wrong, but this is a historical necessity.  NT4 didn't maintain
	 partition information for the whole drive (aka "partition 0"), but
	 returned ERROR_INVALID_HANDLE instead.  That got fixed in W2K. */
      drive_size = di->Cylinders.QuadPart * di->TracksPerCylinder *
		   di->SectorsPerTrack * di->BytesPerSector;
    }
  debug_printf ("drive size: %D", drive_size);
  if (geo)
    {
      geo->heads = di->TracksPerCylinder;
      geo->sectors = di->SectorsPerTrack;
      geo->cylinders = di->Cylinders.LowPart;
      if (pix)
	geo->start = pix->StartingOffset.QuadPart >> 9ULL;
      else if (pi)
	geo->start = pi->StartingOffset.QuadPart >> 9ULL;
      else
        geo->start = 0;
    }
  return 0;
}

int
fhandler_dev_floppy::open (int flags, mode_t)
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
  int ret =  fhandler_dev_raw::open (flags);

  if (ret && get_drive_info (NULL))
    {
      close ();
      return 0;
    }

  return ret;
}

_off64_t
fhandler_dev_floppy::lseek (_off64_t offset, int whence)
{
  char buf[512];
  _off64_t lloffset = offset;
  _off64_t sector_aligned_offset;
  _off64_t bytes_left;
  DWORD low;
  LONG high = 0;

  if (whence == SEEK_END && drive_size > 0)
    {
      lloffset = offset + drive_size;
      whence = SEEK_SET;
    }

  if (whence == SEEK_CUR)
    {
      lloffset += current_position - (devbufend - devbufstart);
      whence = SEEK_SET;
    }

  if (whence != SEEK_SET
      || lloffset < 0
      || drive_size > 0 && lloffset >= drive_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  sector_aligned_offset = (lloffset / bytes_per_sector) * bytes_per_sector;
  bytes_left = lloffset - sector_aligned_offset;

  /* Invalidate buffer. */
  devbufstart = devbufend = 0;

  low = sector_aligned_offset & UINT32_MAX;
  high = sector_aligned_offset >> 32;
  if (SetFilePointer (get_handle (), low, &high, FILE_BEGIN)
      == INVALID_SET_FILE_POINTER && GetLastError ())
    {
      __seterrno ();
      return -1;
    }

  eom_detected (false);
  current_position = sector_aligned_offset;
  if (bytes_left)
    {
      size_t len = bytes_left;
      raw_read (buf, len);
    }
  return current_position + bytes_left;
}

int
fhandler_dev_floppy::ioctl (unsigned int cmd, void *buf)
{
  DISK_GEOMETRY di;
  DWORD bytes_read;
  switch (cmd)
    {
    case HDIO_GETGEO:
      {
	debug_printf ("HDIO_GETGEO");
	return get_drive_info ((struct hd_geometry *) buf);
      }
    case BLKGETSIZE:
    case BLKGETSIZE64:
      {
	debug_printf ("BLKGETSIZE");
	if (cmd == BLKGETSIZE)
	  *(long *)buf = drive_size >> 9UL;
	else
	  *(_off64_t *)buf = drive_size;
	return 0;
      }
    case BLKRRPART:
      {
	debug_printf ("BLKRRPART");
	if (!DeviceIoControl (get_handle (),
			      IOCTL_DISK_UPDATE_DRIVE_SIZE,
			      NULL, 0,
			      &di, sizeof (di),
			      &bytes_read, NULL))
	  {
	    __seterrno ();
	    return -1;
	  }
	get_drive_info (NULL);
	return 0;
      }
    case BLKSSZGET:
      {
	debug_printf ("BLKSSZGET");
	*(int *)buf = bytes_per_sector;
	return 0;
      }
    default:
      return fhandler_dev_raw::ioctl (cmd, buf);
    }
}

