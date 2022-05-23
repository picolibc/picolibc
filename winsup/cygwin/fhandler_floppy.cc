/* fhandler_floppy.cc.  See fhandler.h for a description of the
   fhandler classes.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <alloca.h>
#include <unistd.h>
#include <sys/param.h>
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

#define bytes_per_sector devbufalign

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

  DISK_GEOMETRY_EX *dix = NULL;
  DISK_GEOMETRY *di = NULL;
  PARTITION_INFORMATION_EX *pix = NULL;
  PARTITION_INFORMATION *pi = NULL;
  DWORD bytes_read = 0;

  /* Always try using the new EX ioctls first.  If not available, fall back
     to trying the non-EX ioctls which are unfortunately not implemented in
     the floppy driver. */
  if (get_major () != DEV_FLOPPY_MAJOR)
    {
      if (!DeviceIoControl (get_handle (),
			    IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0,
			    dbuf, 256, &bytes_read, NULL))
	__seterrno ();
      else
	{
	  dix = (DISK_GEOMETRY_EX *) dbuf;
	  di = &dix->Geometry;
	}
    }
  if (!di)
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
  if (dix) /* Don't try IOCTL_DISK_GET_PARTITION_INFO_EX if
	      IOCTL_DISK_GET_DRIVE_GEOMETRY_EX didn't work.
	      Probably a floppy.*/
    {
      if (!DeviceIoControl (get_handle (),
			    IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0,
			    pbuf, 256, &bytes_read, NULL))
	__seterrno ();
      else
	pix = (PARTITION_INFORMATION_EX *) pbuf;
    }
  if (!pix && get_major () != DEV_FLOPPY_MAJOR)
    {
      /* It's unlikely that this code path will be used at all.  Either the
	 _EX call already worked, or it's a floppy.  But it doesn't hurt to
	 keep the code in. */
      if (!DeviceIoControl (get_handle (),
			    IOCTL_DISK_GET_PARTITION_INFO, NULL, 0,
			    pbuf, 256, &bytes_read, NULL))
	__seterrno ();
      else
	pi = (PARTITION_INFORMATION *) pbuf;
    }
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
  else	/* Floppy drive. */
    drive_size = di->Cylinders.QuadPart * di->TracksPerCylinder
		 * di->SectorsPerTrack * di->BytesPerSector;
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
  syscall_printf ("%d (err %d) = ReadFile (%p, %p, to_read %u, read %u, 0)",
		  ret, *err, get_handle (), buf, to_read, *read);
  return ret;
}

/* See comment in write_file below. */
BOOL
fhandler_dev_floppy::lock_partition (DWORD to_write)
{
  DWORD bytes_read;

  /* The simple case.  We have only a single partition open anyway.
     Try to lock the partition so that a subsequent write succeeds.
     If there's some file handle open on one of the affected partitions,
     this fails, but that's how it works...
     The high partition major numbers don't have a partition 0. */
  if (get_major () == DEV_FLOPPY_MAJOR
      || get_major () >= DEV_SD_HIGHPART_START || get_minor () % 16 != 0)
    {
      if (!DeviceIoControl (get_handle (), FSCTL_LOCK_VOLUME,
			   NULL, 0, NULL, 0, &bytes_read, NULL))
	{
	  debug_printf ("DeviceIoControl (FSCTL_LOCK_VOLUME) failed, %E");
	  return FALSE;
	}
      return TRUE;
    }

  /* The tricky case.  We're writing to the entire disk.  What this code
     basically does is to find out if the current write operation affects
     one or more partitions on the disk.  If so, it tries to lock all these
     partitions and stores the handles for a subsequent close(). */
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_POSITION_INFORMATION fpi;
  /* Allocate space for 4 times the maximum partition count we can handle.
     The reason is that for *every* single logical drive in an extended
     partition on an MBR drive, 3 filler entries with partition number set
     to 0 are added into the partition table returned by
     IOCTL_DISK_GET_DRIVE_LAYOUT_EX.  The first of them reproduces the data
     of the next partition entry, if any, except for the partiton number.
     Then two entries with everything set to 0 follow.  Well, the
     documentation states that for MBR drives the number of partition entries
     in the PARTITION_INFORMATION_EX array is always a multiple of 4, but,
     nevertheless, how crappy is that layout? */
  const DWORD size = sizeof (DRIVE_LAYOUT_INFORMATION_EX)
		     + 4 * MAX_PARTITIONS * sizeof (PARTITION_INFORMATION_EX);
  PDRIVE_LAYOUT_INFORMATION_EX pdlix = (PDRIVE_LAYOUT_INFORMATION_EX)
				       alloca (size);
  BOOL found = FALSE;

  /* Fetch current file pointer position on disk. */
  status = NtQueryInformationFile (get_handle (), &io, &fpi, sizeof fpi,
				   FilePositionInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryInformationFile(FilePositionInformation): %y",
		    status);
      return FALSE;
    }
  /* Fetch drive layout to get start and end positions of partitions on disk. */
  if (!DeviceIoControl (get_handle (), IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0,
			pdlix, size, &bytes_read, NULL))
    {
      debug_printf ("DeviceIoControl(IOCTL_DISK_GET_DRIVE_LAYOUT_EX): %E");
      return FALSE;
    }
  /* Scan through partition info to find the partition(s) into which we're
     currently trying to write. */
  PARTITION_INFORMATION_EX *ppie = pdlix->PartitionEntry;
  for (DWORD i = 0; i < pdlix->PartitionCount; ++i, ++ppie)
    {
      /* A partition number of 0 denotes an extended partition or one of the
	 aforementioned filler entries.  Just skip. */
      if (ppie->PartitionNumber == 0)
	continue;
      /* Check if our writing range affects this partition. */
      if (fpi.CurrentByteOffset.QuadPart   < ppie->StartingOffset.QuadPart
					     + ppie->PartitionLength.QuadPart
	  && ppie->StartingOffset.QuadPart < fpi.CurrentByteOffset.QuadPart
					     + to_write)
	{
	  /* Yes.  Now check if we can handle it.  We can only handle
	     up to MAX_PARTITIONS partitions.  The partition numbering is
	     one-based, so we decrement the partition number by 1 when using
	     as index into the partition array. */
	  DWORD &part_no = ppie->PartitionNumber;
	  if (part_no >= MAX_PARTITIONS)
	    return FALSE;
	  found = TRUE;
	  debug_printf ("%u %D->%D : %D->%D", part_no,
			ppie->StartingOffset.QuadPart,
			ppie->StartingOffset.QuadPart
			+ ppie->PartitionLength.QuadPart,
			fpi.CurrentByteOffset.QuadPart,
			fpi.CurrentByteOffset.QuadPart + to_write);
	  /* Do we already have partitions?  If not, create it. */
	  if (!partitions)
	    {
	      partitions = (part_t *) ccalloc_abort (HEAP_FHANDLER, 1,
						     sizeof (part_t));
	      partitions->refcnt = 1;
	    }
	  /* Next, check if the partition is already open.  If so, skip it. */
	  if (partitions->hdl[part_no - 1])
	    continue;
	  /* Now open the partition and lock it. */
	  WCHAR part[MAX_PATH], *p;
	  NTSTATUS status;
	  UNICODE_STRING upart;
	  OBJECT_ATTRIBUTES attr;
	  IO_STATUS_BLOCK io;

	  sys_mbstowcs (part, MAX_PATH, get_win32_name ());
	  p = wcschr (part, L'\0') - 1;
	  __small_swprintf (p, L"%d", part_no);
	  RtlInitUnicodeString (&upart, part);
	  InitializeObjectAttributes (&attr, &upart,
				      OBJ_CASE_INSENSITIVE
				      | ((get_flags () & O_CLOEXEC)
					 ? 0 : OBJ_INHERIT),
				      NULL, NULL);
	  status = NtOpenFile (&partitions->hdl[part_no - 1],
			       GENERIC_READ | GENERIC_WRITE, &attr,
			       &io, FILE_SHARE_READ | FILE_SHARE_WRITE, 0);
	  if (!NT_SUCCESS (status))
	    {
	      debug_printf ("NtCreateFile(%W): %y", part, status);
	      return FALSE;
	    }
	  if (!DeviceIoControl (partitions->hdl[part_no - 1], FSCTL_LOCK_VOLUME,
				NULL, 0, NULL, 0, &bytes_read, NULL))
	    {
	      debug_printf ("DeviceIoControl (%W, FSCTL_LOCK_VOLUME) "
			    "failed, %E", part);
	      return FALSE;
	    }
	}
    }
  /* If we didn't find a single matching partition, the "Access denied"
     had another reason, so return FALSE in that case. */
  return found;
}

BOOL
fhandler_dev_floppy::write_file (const void *buf, DWORD to_write,
				 DWORD *written, int *err)
{
  BOOL ret;

  *err = 0;
  if (!(ret = WriteFile (get_handle (), buf, to_write, written, 0)))
    *err = GetLastError ();
  /* When writing to a disk or partition an "Access denied" error may
     occur due to the raw disk write restriction.
     See http://support.microsoft.com/kb/942448 for details.
     What we do here is to lock the affected partition(s) and retry. */
  if (*err == ERROR_ACCESS_DENIED
      && get_major () != DEV_CDROM_MAJOR
      && (get_flags () & O_ACCMODE) != O_RDONLY
      && lock_partition (to_write))
    {
      *err = 0;
      if (!(ret = WriteFile (get_handle (), buf, to_write, written, 0)))
	*err = GetLastError ();
    }
  syscall_printf ("%d (err %d) = WriteFile (%p, %p, write %u, written %u, 0)",
		  ret, *err, get_handle (), buf, to_write, *written);
  return ret;
}

int
fhandler_dev_floppy::open (int flags, mode_t)
{
  int ret = fhandler_dev_raw::open (flags);

  if (ret)
    {
      DWORD bytes_read;

      if (get_drive_info (NULL))
	{
	  close ();
	  return 0;
	}
      if (!(flags & O_DIRECT))
	{
	  /* Create sector-aligned buffer.  As default buffer size, we're using
	     some big, sector-aligned value.  Since direct blockdev IO is
	     usually non-buffered and non-cached, the performance without
	     buffering is worse than access to a file system on same device.
	     Whoever uses O_DIRECT has my condolences. */
	  devbufsiz = MAX (16 * bytes_per_sector, 65536);
	  devbufalloc = new char [devbufsiz + devbufalign];
	  devbuf = (char *) roundup2 ((uintptr_t) devbufalloc,
				      (uintptr_t) devbufalign);
	}

      /* If we're not trying to access a floppy disk, make sure we're actually
         allowed to read *all* of the device or volume.  This is actually
	 documented in the MSDN CreateFile man page. */
      if (get_major () != DEV_FLOPPY_MAJOR
	  && !DeviceIoControl (get_handle (), FSCTL_ALLOW_EXTENDED_DASD_IO,
			       NULL, 0, NULL, 0, &bytes_read, NULL))
	debug_printf ("DeviceIoControl (FSCTL_ALLOW_EXTENDED_DASD_IO) "
		      "failed, %E");
    }

  return ret;
}

int
fhandler_dev_floppy::close ()
{
  int ret = fhandler_dev_raw::close ();

  if (partitions && InterlockedDecrement (&partitions->refcnt) == 0)
    {
      for (int i = 0; i < MAX_PARTITIONS; ++i)
	if (partitions->hdl[i])
	  NtClose (partitions->hdl[i]);
      cfree (partitions);
    }
  return ret;
}

int
fhandler_dev_floppy::dup (fhandler_base *child, int flags)
{
  int ret = fhandler_dev_raw::dup (child, flags);

  if (!ret && partitions)
    InterlockedIncrement (&partitions->refcnt);
  return ret;
}

inline off_t
fhandler_dev_floppy::get_current_position ()
{
  IO_STATUS_BLOCK io;
  FILE_POSITION_INFORMATION fpi = { { QuadPart : 0LL } };

  NtQueryInformationFile (get_handle (), &io, &fpi, sizeof fpi,
			  FilePositionInformation);
  return fpi.CurrentByteOffset.QuadPart;
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
	      bytes_to_read = MIN (len, devbufend - devbufstart);
	      debug_printf ("read %u bytes from buffer (rest %u)",
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
	      off_t current_position = get_current_position ();
	      if (current_position + bytes_to_read >= drive_size)
		bytes_to_read = drive_size - current_position;
	      if (!bytes_to_read)
		break;

	      debug_printf ("read %u bytes from pos %U %s", bytes_to_read,
			    current_position,
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
  else
    {
      off_t current_position = get_current_position ();
      bytes_to_read = len;
      if (current_position + bytes_to_read >= drive_size)
	bytes_to_read = drive_size - current_position;
      debug_printf ("read %u bytes from pos %U directly", bytes_to_read,
		    current_position);
      if (bytes_to_read && !read_file (p, bytes_to_read, &bytes_read, &ret))
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
    }

  ulen = (size_t) bytes_read;
  return;

err:
  ulen = (size_t) -1;
}

ssize_t
fhandler_dev_floppy::raw_write (const void *ptr, size_t len)
{
  DWORD bytes_written = 0;
  char *p = (char *) ptr;
  int ret;

  /* Checking a previous end of media */
  if (eom_detected ())
    {
      set_errno (ENOSPC);
      return -1;
    }

  if (!len)
    return 0;

  if (devbuf)
    {
      DWORD cplen, written;

      /* First check if we have an active read buffer.  If so, try to fit in
	 the start of the input buffer and write out the entire result.
	 This also covers the situation after lseek since lseek fills the read
	 buffer in case we seek to an address which is not sector aligned. */
      if (devbufend && devbufstart < devbufend)
	{
	  off_t current_pos = get_current_position ();
	  cplen = MIN (len, devbufend - devbufstart);
	  memcpy (devbuf + devbufstart, p, cplen);
	  LARGE_INTEGER off = { QuadPart:current_pos - devbufend };
	  if (!SetFilePointerEx (get_handle (), off, NULL, FILE_BEGIN))
	    {
	      devbufstart = devbufend = 0;
	      __seterrno ();
	      return -1;
	    }
	  if (!write_file (devbuf, devbufend, &written, &ret))
	    {
	      devbufstart = devbufend = 0;
	      goto err;
	    }
	  /* Align pointers, lengths, etc. */
	  cplen = MIN (cplen, written);
	  devbufstart += cplen;
	  if (devbufstart >= devbufend)
	    devbufstart = devbufend = 0;
	  p += cplen;
	  len -= cplen;
	  bytes_written += cplen;
	}
      /* As long as there's still something left in the input buffer ... */
      while (len)
	{
	  /* Compute the length to write.  The problem is that the underlying
	     driver may require sector aligned read/write.  So we copy the data
	     over to devbuf, which is guaranteed to be sector aligned. */
	  cplen = MIN (len, devbufsiz);
	  if (cplen >= bytes_per_sector)
	    /* If the remaining len is >= sector size, write out the maximum
	       possible multiple of the sector size which fits into devbuf. */
	    cplen = rounddown (cplen, bytes_per_sector);
	  else
	    {
	      /* If len < sector size, read in the next sector, seek back,
		 and just copy the new data over the old one before writing. */
	      LARGE_INTEGER off = { QuadPart:get_current_position () };
	      if (!read_file (devbuf, bytes_per_sector, &written, &ret))
		goto err;
	      if (!SetFilePointerEx (get_handle (), off, NULL, FILE_BEGIN))
		{
		  __seterrno ();
		  return -1;
		}
	    }
	  memcpy (devbuf, p, cplen);
	  if (!write_file (devbuf, MAX (cplen, bytes_per_sector), &written,
			   &ret))
	    {
	      bytes_written += MIN (cplen, written);
	      goto err;
	    }
	  cplen = MIN (cplen, written);
	  p += cplen;
	  len -= cplen;
	  bytes_written += cplen;
	  /* If we overwrote, revalidate devbuf.  It still contains the
	     content from the above read/modify/write.  Revalidating makes
	     sure lseek reports the correct position. */
	  if (cplen < bytes_per_sector)
	    {
	      devbufstart = cplen;
	      devbufend = bytes_per_sector;
	    }
	}
      return (ssize_t) bytes_written;
    }

  /* In O_DIRECT case, just write. */
  if (write_file (p, len, &bytes_written, &ret))
    return (ssize_t) bytes_written;

err:
  if (IS_EOM (ret))
    {
      eom_detected (true);
      if (!bytes_written)
	set_errno (ENOSPC);
    }
  else if (!bytes_written)
    __seterrno ();
  /* Cast is required, otherwise the error return value is (DWORD)-1. */
  return (ssize_t) bytes_written ?: -1;
}

off_t
fhandler_dev_floppy::lseek (off_t offset, int whence)
{
  char buf[bytes_per_sector];
  off_t current_pos = (off_t) -1;
  LARGE_INTEGER sector_aligned_offset;
  size_t bytes_left;

  if (whence == SEEK_END)
    {
      offset += drive_size;
      whence = SEEK_SET;
    }
  else if (whence == SEEK_CUR)
    {
      current_pos = get_current_position ();
      off_t exact_pos = current_pos - (devbufend - devbufstart);
      /* Shortcut when used to get current position. */
      if (offset == 0)
	return exact_pos;
      offset += exact_pos;
      whence = SEEK_SET;
    }

  if (whence != SEEK_SET || offset < 0 || offset > drive_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* If new position is in buffered range, adjust buffer and return */
  if (devbufstart < devbufend)
    {
      if (current_pos == (off_t) -1)
	current_pos = get_current_position ();
      if (current_pos - devbufend <= offset && offset <= current_pos)
	{
	  devbufstart = devbufend - (current_pos - offset);
	  return offset;
	}
    }

  sector_aligned_offset.QuadPart = rounddown (offset, bytes_per_sector);
  bytes_left = offset - sector_aligned_offset.QuadPart;

  /* Invalidate buffer. */
  devbufstart = devbufend = 0;

  if (!SetFilePointerEx (get_handle (), sector_aligned_offset, NULL,
			 FILE_BEGIN))
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
  int ret = 0;
  DWORD bytes_read;

  switch (cmd)
    {
    case HDIO_GETGEO:
      debug_printf ("HDIO_GETGEO");
      ret = get_drive_info ((struct hd_geometry *) buf);
      break;
    case BLKGETSIZE:
    case BLKGETSIZE64:
      debug_printf ("BLKGETSIZE");
      if (cmd == BLKGETSIZE)
	*(long *)buf = drive_size >> 9UL;
      else
	*(off_t *)buf = drive_size;
      break;
    case BLKRRPART:
      debug_printf ("BLKRRPART");
      if (!DeviceIoControl (get_handle (), IOCTL_DISK_UPDATE_PROPERTIES,
			    NULL, 0, NULL, 0, &bytes_read, NULL))
	{
	  __seterrno ();
	  ret = -1;
	}
      else
	get_drive_info (NULL);
      break;
    case BLKSSZGET:
      debug_printf ("BLKSSZGET");
      *(int *)buf = (int) bytes_per_sector;
      break;
    case BLKIOMIN:
      debug_printf ("BLKIOMIN");
      *(int *)buf = (int) bytes_per_sector;
      break;
    case BLKIOOPT:
      debug_printf ("BLKIOOPT");
      *(int *)buf = (int) bytes_per_sector;
      break;
    case BLKPBSZGET:
      debug_printf ("BLKPBSZGET");
      *(int *)buf = (int) bytes_per_sector;
      break;
    case BLKALIGNOFF:
      debug_printf ("BLKALIGNOFF");
      *(int *)buf = 0;
      break;
    default:
      ret = fhandler_dev_raw::ioctl (cmd, buf);
      break;
    }
  return ret;
}

