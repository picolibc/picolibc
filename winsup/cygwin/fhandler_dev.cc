/* fhandler_dev.cc, Implement /dev.

   Copyright 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include "path.h"
#include "fhandler.h"
#include "shared_info.h"
#include "ntdll.h"
#include "dtable.h"
#include "cygheap.h"
#include "devices.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

#define dev_prefix_len (sizeof ("/dev"))
#define dev_storage_scan_start (dev_storage + 1)
#define dev_storage_size (dev_storage_end - dev_storage_scan_start)

static int
device_cmp (const void *a, const void *b)
{
  return strcmp (((const device *) a)->name,
		 ((const device *) b)->name + dev_prefix_len);
}

fhandler_dev::fhandler_dev () :
  fhandler_disk_file (), devidx (NULL), dir_exists (true)
{
}

DIR *
fhandler_dev::opendir (int fd)
{
  DIR *dir;
  DIR *res = NULL;

  dir = fhandler_disk_file::opendir (fd);
  if (dir)
    return dir;
  if ((dir = (DIR *) malloc (sizeof (DIR))) == NULL)
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
      else
	{
	  cfd = this;
	  dir->__d_fd = cfd;
	  cfd->nohandle (true);
	}
      set_close_on_exec (true);
      dir->__fh = this;
      devidx = dev_storage_scan_start;
      res = dir;
    }

  syscall_printf ("%p = opendir (%s)", res, get_name ());
  return res;

free_dirent:
  free (dir->__d_dirent);
free_dir:
  free (dir);
  return res;
}

int
fhandler_dev::readdir (DIR *dir, dirent *de)
{
  int ret;
  device dev;

  if (!devidx)
    {
      while ((ret = fhandler_disk_file::readdir (dir, de)) == 0)
	{
	  /* Avoid to print devices for which users have created files under
	     /dev already, for instance by using the old script from Igor
	     Peshansky. */
	  dev.name = de->d_name;
	  if (!bsearch (&dev, dev_storage_scan_start, dev_storage_size, sizeof dev,
			device_cmp))
	    break;
	}
      if (ret == ENMFILE)
	devidx = dev_storage_scan_start;
      else
	goto out;
    }

  /* Now start processing our internal dev table. */
  ret = ENMFILE;
  while (devidx < dev_storage_end)
    {
      const device& thisdev = *devidx++;
      if (!thisdev.expose ())
	continue;
      int devn = *const_cast<device *> (&thisdev);
      /* Exclude devices which are only available for internal purposes
	 and devices which are not really existing at this time. */
      switch (thisdev.get_major ())
	{
	case DEV_PTYS_MAJOR:
	  /* Show only existing slave ptys. */
	  if (cygwin_shared->tty.connect (thisdev.get_minor ()) == -1)
	    continue;
	  break;
	case DEV_CONS_MAJOR:
	  /* Show only the one console which is our controlling tty
	     right now. */
	  if (!iscons_dev (myself->ctty)
	      || myself->ctty != devn)
	    continue;
	  break;
	case DEV_TTY_MAJOR:
	  /* Show con{in,out,sole} only if we're running in a console. */
	  switch (devn)
	    {
	    case FH_CONIN:
	    case FH_CONOUT:
	    case FH_CONSOLE:
	      if (!iscons_dev (myself->ctty))
		continue;
	    }
	  break;
	case DEV_SERIAL_MAJOR:
	case DEV_FLOPPY_MAJOR:
	case DEV_TAPE_MAJOR:
	case DEV_CDROM_MAJOR:
	case DEV_SD_MAJOR:
	case DEV_SD1_MAJOR:
	case DEV_SD2_MAJOR:
	case DEV_SD3_MAJOR:
	case DEV_SD4_MAJOR:
	case DEV_SD5_MAJOR:
	case DEV_SD6_MAJOR:
	case DEV_SD7_MAJOR:
	  /* Check existence of POSIX devices backed by real NT devices. */
	  {
	    WCHAR wpath[MAX_PATH];
	    UNICODE_STRING upath;
	    OBJECT_ATTRIBUTES attr;
	    HANDLE h;
	    NTSTATUS status;

	    sys_mbstowcs (wpath, MAX_PATH, thisdev.native);
	    RtlInitUnicodeString (&upath, wpath);
	    InitializeObjectAttributes (&attr, &upath,
					OBJ_CASE_INSENSITIVE, NULL, NULL);
	    /* Except for the serial IO devices, the native paths are
	       direct device paths, not symlinks, so every status code
	       except for "NOT_FOUND" means the device exists. */
	    status = NtOpenSymbolicLinkObject (&h, SYMBOLIC_LINK_QUERY,
					       &attr);
	    switch (status)
	      {
	      case STATUS_OBJECT_NAME_NOT_FOUND:
	      case STATUS_OBJECT_PATH_NOT_FOUND:
		continue;
	      case STATUS_SUCCESS:
		NtClose (h);
		break;
	      default:
		break;
	      }
	  }
	  break;
	}
      ++dir->__d_position;
      strcpy (de->d_name, thisdev.name + dev_prefix_len);
      de->d_ino = hash_path_name (0, thisdev.native);
      switch (thisdev.get_major ())
	{
	case DEV_FLOPPY_MAJOR:
	case DEV_TAPE_MAJOR:
	case DEV_CDROM_MAJOR:
	case DEV_SD_MAJOR:
	case DEV_SD1_MAJOR:
	case DEV_SD2_MAJOR:
	case DEV_SD3_MAJOR:
	case DEV_SD4_MAJOR:
	case DEV_SD5_MAJOR:
	case DEV_SD6_MAJOR:
	case DEV_SD7_MAJOR:
	  de->d_type = DT_BLK;
	  break;
	case DEV_TTY_MAJOR:
	  switch (devn)
	    {
	    case FH_CONIN:
	    case FH_CONOUT:
	    case FH_CONSOLE:
	      dev.parse (myself->ctty);
	      de->d_ino = hash_path_name (0, dev.native);
	      break;
	    }
	  /*FALLTHRU*/
	default:
	  de->d_type = DT_CHR;
	  break;
	}
      ret = 0;
      break;
    }

out:
  debug_printf ("returning %d", ret);
  return ret;
}

void
fhandler_dev::rewinddir (DIR *dir)
{
  devidx = dir_exists ? NULL : dev_storage_scan_start;
  fhandler_disk_file::rewinddir (dir);
}
