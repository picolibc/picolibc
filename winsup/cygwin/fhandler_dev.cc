/* fhandler_dev.cc, Implement /dev.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/statvfs.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "devices.h"

#define _LIBC
#include <dirent.h>

#define dev_prefix_len (sizeof ("/dev"))
#define dev_storage_scan_start (dev_storage + 1)
#define dev_storage_size (dev_storage_end - dev_storage_scan_start)

static int
device_cmp (const void *a, const void *b)
{
  return strcmp (((const device *) a)->name (),
		 ((const device *) b)->name () + dev_prefix_len);
}

fhandler_dev::fhandler_dev () :
  fhandler_disk_file (), devidx (NULL), dir_exists (true)
{
}

int
fhandler_dev::open (int flags, mode_t mode)
{
  if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    {
      set_errno (EEXIST);
      return 0;
    }
  if (flags & O_WRONLY)
    {
      set_errno (EISDIR);
      return 0;
    }
  /* Filter O_CREAT flag to avoid creating a file called /dev accidentally. */
  int ret = fhandler_disk_file::open (flags & ~O_CREAT, mode);
  if (!ret)
    {
      /* Open a fake handle to \\Device\\Null */
      ret = open_null (flags);
      dir_exists = false;
    }
  return ret;
}

int
fhandler_dev::close ()
{
  return fhandler_disk_file::close ();
}

int __reg2
fhandler_dev::fstat (struct stat *st)
{
  /* If /dev really exists on disk, return correct disk information. */
  if (pc.fs_got_fs ())
    return fhandler_disk_file::fstat (st);
  /* Otherwise fake virtual filesystem. */
  fhandler_base::fstat (st);
  st->st_ino = 2;
  st->st_mode = S_IFDIR | STD_RBITS | STD_XBITS;
  st->st_nlink = 1;
  return 0;
}

int __reg2
fhandler_dev::fstatvfs (struct statvfs *sfs)
{
  int ret = -1, opened = 0;
  HANDLE fh = get_handle ();

  if (!fh)
    {
      if (!open (O_RDONLY, 0))
	return -1;
      opened = 1;
    }
  if (pc.fs_got_fs ())
    ret = fhandler_disk_file::fstatvfs (sfs);
  else
    {
      /* Virtual file system.  Just return an empty buffer with a few values
	 set to something useful similar to Linux. */
      memset (sfs, 0, sizeof (*sfs));
      sfs->f_bsize = sfs->f_frsize = 4096;
      sfs->f_flag = ST_RDONLY;
      sfs->f_namemax = NAME_MAX;
      ret = 0;
    }
  if (opened)
    close ();
  return ret;
}

int
fhandler_dev::rmdir ()
{
  set_errno (ENOTEMPTY);
  return -1;
}

DIR *
fhandler_dev::opendir (int fd)
{
  DIR *dir = fhandler_disk_file::opendir (fd);
  if (dir)
    dir_exists = true;
  else if ((dir = (DIR *) malloc (sizeof (DIR))) == NULL)
    set_errno (ENOMEM);
  else if ((dir->__d_dirent =
	    (struct dirent *) malloc (sizeof (struct dirent))) == NULL)
    {
      set_errno (ENOMEM);
      goto free_dir;
    }
  else
    {
      cygheap_fdnew cfd;
      if (cfd < 0 && fd < 0)
	goto free_dirent;

      dir->__d_dirname = NULL;
      dir->__d_dirent->__d_version = __DIRENT_VERSION;
      dir->__d_cookie = __DIRENT_COOKIE;
      dir->__handle = INVALID_HANDLE_VALUE;
      dir->__d_position = 0;
      dir->__flags = 0;
      dir->__d_internal = 0;

      if (fd >= 0)
	dir->__d_fd = fd;
      else if (!open (O_RDONLY, 0))
	goto free_dirent;
      else
	{
	  cfd = this;
	  dir->__d_fd = cfd;
	}
      set_close_on_exec (true);
      dir->__fh = this;
      dir_exists = false;
      drive = part = 0;
    }

  devidx = dir_exists ? NULL : dev_storage_scan_start;

  syscall_printf ("%p = opendir (%s)", dir, get_name ());
  return dir;

free_dirent:
  free (dir->__d_dirent);
free_dir:
  free (dir);
  return NULL;
}

static const WCHAR *hd_pattern = L"\\Device\\Harddisk%u\\Partition%u";

int
fhandler_dev::readdir (DIR *dir, dirent *de)
{
  int ret;
  const _device *curdev;
  device dev;

  if (!devidx)
    {
      while ((ret = fhandler_disk_file::readdir (dir, de)) == 0)
	{
	  /* Avoid to print devices for which users have created files under
	     /dev already, for instance by using the old script from Igor
	     Peshansky. */
	  dev.name (de->d_name);
	  if (!bsearch (&dev, dev_storage_scan_start, dev_storage_size,
			sizeof dev, device_cmp))
	    break;
	}
      if (ret != ENMFILE)
	goto out;
      devidx = dev_storage_scan_start;
    }

  /* Now start processing our internal dev table. */
  ret = ENMFILE;
  while ((curdev = devidx++) < dev_storage_end)
    {
      /* If exists returns < 0 it means that the device can be used by a
	 program but its use is deprecated and, so, it is not returned
	 by readdir(().  */
      device *cdev = (device *) curdev;
      if (cdev->exists () <= 0)
	continue;
      ++dir->__d_position;
      strcpy (de->d_name, cdev->name () + dev_prefix_len);
      if (cdev->get_major () == DEV_TTY_MAJOR
	  && (cdev->is_device (FH_CONIN)
	      || cdev->is_device (FH_CONOUT)
	      || cdev->is_device (FH_CONSOLE)))
	{
	  /* Make sure conin, conout, and console have the same inode number
	     as the current consX. */
	  de->d_ino = myself->ctty;
	}
      else
	de->d_ino = cdev->get_device ();
      de->d_type = cdev->type ();
      ret = 0;
      break;
    }
  /* Last but not least, scan for existing disks/partitions. */
  if (ret)
    {
      UNICODE_STRING upath;
      WCHAR buf[(sizeof *hd_pattern + 32) / sizeof (wchar_t)];
      OBJECT_ATTRIBUTES attr;
      FILE_BASIC_INFORMATION fbi;
      NTSTATUS status;

      InitializeObjectAttributes (&attr, &upath, 0, NULL, NULL);
      while (drive < 128)
	{
	  while (part < 64)
	    {
	      USHORT len = __small_swprintf (buf, hd_pattern, drive, part);
	      RtlInitCountedUnicodeString (&upath, buf, len * sizeof (WCHAR));
	      status = NtQueryAttributesFile (&attr, &fbi);
	      debug_printf ("%S %y", &upath, status);
	      if (status != STATUS_OBJECT_NAME_NOT_FOUND
		  && status != STATUS_OBJECT_PATH_NOT_FOUND)
		{
		  device dev (drive, part);
		  strcpy (de->d_name, dev.name () + 5);
		  de->d_ino = dev.get_device ();
		  de->d_type = DT_BLK;
		  ++part;
		  ret = 0;
		  goto out;
		}
	      if (part == 0)
		break;
	      ++part;
	    }
	  part = 0;
	  ++drive;
	}
    }

out:
  debug_printf ("returning %d", ret);
  return ret;
}

void
fhandler_dev::rewinddir (DIR *dir)
{
  devidx = dir_exists ? NULL : dev_storage_scan_start;
  dir->__d_position = 0;
  if (dir_exists)
    fhandler_disk_file::rewinddir (dir);
}
