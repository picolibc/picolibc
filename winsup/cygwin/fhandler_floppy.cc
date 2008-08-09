/* fhandler_floppy.cc.  See fhandler.h for a description of the
   fhandler classes.

   Copyright 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <winioctl.h>
#include <cygwin/rdevio.h>
#include <cygwin/hdreg.h>
#include <cygwin/fs.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "ntdll.h"

#define IS_EOM(err)	((err) == ERROR_INVALID_PARAMETER \
			 || (err) == ERROR_SEEK \
			 || (err) == ERROR_SECTOR_NOT_FOUND)

/**********************************************************************/
/* fhandler_dev_floppy */

fhandler_dev_floppy::fhandler_dev_floppy ()
  : fhandler_dev_raw (), status ()
{
}

int
fhandler_dev_floppy::get_drive_info (struct hd_geometry *geo)
{
  char dbuf[256];
  char pbuf[256];

  DISK_GEOMETRY *di = NULL;
  PARTITION_INFORMATION_EX *pix = NULL;
  PARTITION_INFORMATION *pi = NULL;
  DWORD bytes_read = 0;

  /* Always try using the new EX ioctls first (>= XP).  If not available,
     fall back to trying the old non-EX ioctls.
     Unfortunately the EX ioctls are not implemented in the floppy driver. */
  if (wincap.has_disk_ex_ioctls () && get_major () != DEV_FLOPPY_MAJOR)
    {
      if (!DeviceIoControl (get_handle (),
			    IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0,
			    dbuf, 256, &bytes_read, NULL))
	__seterrno ();
      else
	{
	  di = &((DISK_GEOMETRY_EX *) dbuf)->Geometry;
	  if (!DeviceIoControl (get_handle (),
				IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0,
				pbuf, 256, &bytes_read, NULL))
	    __seterrno ();
	  else
	    pix = (PARTITION_INFORMATION_EX *) pbuf;
	}
    }
  if (!di)
    {
      if (!DeviceIoControl (get_handle (),
			    IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
			    dbuf, 256, &bytes_read, NULL))
	__seterrno ();
      else
	{
	  di = (DISK_GEOMETRY *) dbuf;
	  if (!DeviceIoControl (get_handle (),
				IOCTL_DISK_GET_PARTITION_INFO, NULL, 0,
				pbuf, 256, &bytes_read, NULL))
	    __seterrno ();
	  else
	    pi = (PARTITION_INFORMATION *) pbuf;
	}
    }
  if (!di)
    {
      /* Up to Win2K, even IOCTL_DISK_GET_DRIVE_GEOMETRY fails when trying
	 it on CD or DVD drives.  In that case fall back to requesting
	 simple file system information. */
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      FILE_FS_SIZE_INFORMATION ffsi;

      status = NtQueryVolumeInformationFile (get_handle (), &io, &ffsi,
					     sizeof ffsi,
					     FileFsSizeInformation);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return -1;
	}
      debug_printf ("fsys geometry: (%D units)*(%u sec)*(%u bps)",
		    ffsi.TotalAllocationUnits.QuadPart,
		    ffsi.SectorsPerAllocationUnit,
		    ffsi.BytesPerSector);
      bytes_per_sector = ffsi.BytesPerSector;
      drive_size = ffsi.TotalAllocationUnits.QuadPart
		   * ffsi.SectorsPerAllocationUnit
		   * ffsi.BytesPerSector;
      if (geo)
	{
	  geo->heads = 1;
	  geo->sectors = ffsi.SectorsPerAllocationUnit;
	  geo->cylinders = ffsi.TotalAllocationUnits.LowPart;
	  geo->start = 0;
	}
    }
  else
    {
      debug_printf ("disk geometry: (%D cyl)*(%u trk)*(%u sec)*(%u bps)",
		     di->Cylinders.QuadPart,
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
	     looks wrong, but this is a historical necessity.  NT4 didn't
	     maintain partition information for the whole drive (aka
	     "partition 0"), but returned ERROR_INVALID_HANDLE instead.  That
	     got fixed in W2K. */
	  drive_size = di->Cylinders.QuadPart * di->TracksPerCylinder
		       * di->SectorsPerTrack * di->BytesPerSector;
	}
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
    }
  debug_printf ("drive size: %D", drive_size);

  return 0;
}

/* Wrapper functions for ReadFile and WriteFile to simplify error handling. */
BOOL
fhandler_dev_floppy::read_file (void *buf, DWORD to_read, DWORD *read, int *err)
{
  BOOL ret;

  *err = 0;
  if (!(ret = ReadFile (get_handle (), buf, to_read, read, 0)))
    *err = GetLastError ();
  syscall_printf ("%d (err %d) = ReadFile (%d, %d, to_read %d, read %d, 0)",
		  ret, *err, get_handle (), buf, to_read, *read);
  return ret;
}

BOOL
fhandler_dev_floppy::write_file (const void *buf, DWORD to_write,
				 DWORD *written, int *err)
{
  BOOL ret;

  *err = 0;
  if (!(ret = WriteFile (get_handle (), buf, to_write, written, 0)))
    *err = GetLastError ();
  syscall_printf ("%d (err %d) = WriteFile (%d, %d, write %d, written %d, 0)",
		  ret, *err, get_handle (), buf, to_write, *written);
  return ret;
}

int
fhandler_dev_floppy::open (int flags, mode_t)
{
  /* The correct size of the buffer would be 512 bytes, which is the atomic
     size, supported by WinNT.  Unfortunately, the performance is worse than
     access to file system on same device!  Setting buffer size to a
     relatively big value increases performance by means.  The new ioctl call
     with 'rdevio.h' header file supports changing this value.

     As default buffer size, we're using some value which is a multiple of
     the typical tar and cpio buffer sizes, Except O_DIRECT is set, in which
     case we're not buffering at all. */
  devbufsiz = (flags & O_DIRECT) ? 0L : 61440L;
  int ret =  fhandler_dev_raw::open (flags);

  if (ret && get_drive_info (NULL))
    {
      close ();
      return 0;
    }

  return ret;
}

int
fhandler_dev_floppy::dup (fhandler_base *child)
{
  int ret = fhandler_dev_raw::dup (child);

  if (!ret)
    {
      fhandler_dev_floppy *fhc = (fhandler_dev_floppy *) child;

      fhc->drive_size = drive_size;
      fhc->bytes_per_sector = bytes_per_sector;
      fhc->eom_detected (eom_detected ());
    }
  return ret;
}

inline _off64_t
fhandler_dev_floppy::get_current_position ()
{
  LARGE_INTEGER off = { QuadPart: 0LL };
  off.LowPart = SetFilePointer (get_handle (), 0, &off.HighPart, FILE_CURRENT);
  return off.QuadPart;
}

void
fhandler_dev_floppy::raw_read (void *ptr, size_t& ulen)
{
  DWORD bytes_read = 0;
  DWORD read2;
  DWORD bytes_to_read;
  int ret;
  size_t len = ulen;
  char *tgt;
  char *p = (char *) ptr;

  /* Checking a previous end of media */
  if (eom_detected () && !lastblk_to_read ())
    {
      set_errno (ENOSPC);
      goto err;
    }

  if (devbuf)
    {
      while (len > 0)
	{
	  if (devbufstart < devbufend)
	    {
	      bytes_to_read = min (len, devbufend - devbufstart);
	      debug_printf ("read %d bytes from buffer (rest %d)",
			    bytes_to_read,
			    devbufend - devbufstart - bytes_to_read);
	      memcpy (p, devbuf + devbufstart, bytes_to_read);
	      len -= bytes_to_read;
	      p += bytes_to_read;
	      bytes_read += bytes_to_read;
	      devbufstart += bytes_to_read;

	      if (lastblk_to_read ())
		{
		  lastblk_to_read (false);
		  break;
		}
	    }
	  if (len > 0)
	    {
	      if (len >= devbufsiz)
		{
		  bytes_to_read = (len / bytes_per_sector) * bytes_per_sector;
		  tgt = p;
		}
	      else
		{
		  tgt = devbuf;
		  bytes_to_read = devbufsiz;
		}
	      _off64_t current_position = get_current_position ();
	      if (current_position + bytes_to_read >= drive_size)
		bytes_to_read = drive_size - current_position;
	      if (!bytes_to_read)
		break;

	      debug_printf ("read %d bytes %s", bytes_to_read,
			    len < devbufsiz ? "into buffer" : "directly");
	      if (!read_file (tgt, bytes_to_read, &read2, &ret))
		{
		  if (!IS_EOM (ret))
		    {
		      __seterrno ();
		      goto err;
		    }

		  eom_detected (true);

		  if (!read2)
		    {
		      if (!bytes_read)
			{
			  debug_printf ("return -1, set errno to ENOSPC");
			  set_errno (ENOSPC);
			  goto err;
			}
		      break;
		    }
		  lastblk_to_read (true);
		}
	      if (!read2)
	       break;
	      if (tgt == devbuf)
		{
		  devbufstart = 0;
		  devbufend = read2;
		}
	      else
		{
		  len -= read2;
		  p += read2;
		  bytes_read += read2;
		}
	    }
	}
    }
  else if (!read_file (p, len, &bytes_read, &ret))
    {
      if (!IS_EOM (ret))
	{
	  __seterrno ();
	  goto err;
	}
      if (bytes_read)
	eom_detected (true);
      else
	{
	  debug_printf ("return -1, set errno to ENOSPC");
	  set_errno (ENOSPC);
	  goto err;
	}
    }

  ulen = (size_t) bytes_read;
  return;

err:
  ulen = (size_t) -1;
}

int
fhandler_dev_floppy::raw_write (const void *ptr, size_t len)
{
  DWORD bytes_written = 0;
  char *p = (char *) ptr;
  int ret;

  /* Checking a previous end of media on tape */
  if (eom_detected ())
    {
      set_errno (ENOSPC);
      return -1;
    }

  /* Invalidate buffer. */
  devbufstart = devbufend = 0;

  if (len > 0)
    {
      if (!write_file (p, len, &bytes_written, &ret))
	{
	  if (!IS_EOM (ret))
	    {
	      __seterrno ();
	      return -1;
	    }
	  eom_detected (true);
	  if (!bytes_written)
	    {
	      set_errno (ENOSPC);
	      return -1;
	    }
	}
    }
  return bytes_written;
}

_off64_t
fhandler_dev_floppy::lseek (_off64_t offset, int whence)
{
  char buf[bytes_per_sector];
  _off64_t lloffset = offset;
  _off64_t current_pos = (_off64_t) -1;
  LARGE_INTEGER sector_aligned_offset;
  size_t bytes_left;

  if (whence == SEEK_END)
    {
      lloffset += drive_size;
      whence = SEEK_SET;
    }
  else if (whence == SEEK_CUR)
    {
      current_pos = get_current_position ();
      lloffset += current_pos - (devbufend - devbufstart);
      whence = SEEK_SET;
    }

  if (whence != SEEK_SET || lloffset < 0 || lloffset > drive_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* If new position is in buffered range, adjust buffer and return */
  if (devbufstart < devbufend)
    {
      if (current_pos == (_off64_t) -1)
	current_pos = get_current_position ();
      if (current_pos - devbufend <= lloffset && lloffset <= current_pos)
	{
	  devbufstart = devbufend - (current_pos - lloffset);
	  return lloffset;
	}
    }

  sector_aligned_offset.QuadPart = (lloffset / bytes_per_sector)
				   * bytes_per_sector;
  bytes_left = lloffset - sector_aligned_offset.QuadPart;

  /* Invalidate buffer. */
  devbufstart = devbufend = 0;

  sector_aligned_offset.LowPart =
			SetFilePointer (get_handle (),
					sector_aligned_offset.LowPart,
					&sector_aligned_offset.HighPart,
					FILE_BEGIN);
  if (sector_aligned_offset.LowPart == INVALID_SET_FILE_POINTER
      && GetLastError ())
    {
      __seterrno ();
      return -1;
    }

  eom_detected (false);

  if (bytes_left)
    {
      raw_read (buf, bytes_left);
      if (bytes_left == (size_t) -1)
	return -1;
    }

  return sector_aligned_offset.QuadPart + bytes_left;
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
    case RDSETBLK:
      /* Just check the restriction that blocksize must be a multiple
	 of the sector size of the underlying volume sector size,
	 then fall through to fhandler_dev_raw::ioctl. */
      if (((struct rdop *) buf)->rd_parm % bytes_per_sector)
	{
	  SetLastError (ERROR_INVALID_PARAMETER);
	  __seterrno ();
	  return -1;
	}
      /*FALLTHRU*/
    default:
      return fhandler_dev_raw::ioctl (cmd, buf);
    }
}

