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
#include "devices.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

#define dev_prefix_len (sizeof ("/dev"))

static int
device_cmp (const void *a, const void *b)
{
  return strcmp (((const device *) a)->name,
		 ((const device *) b)->name + dev_prefix_len);
}

int
fhandler_dev::readdir (DIR *dir, dirent *de)
{
  int ret;

  if (dir_exists && !lastrealpos)
    {
      while ((ret = fhandler_disk_file::readdir (dir, de)) == 0)
	{
	  /* Avoid to print devices for which users have created files under
	     /dev already, for instance by using the old script from Igor
	     Peshansky. */
	  device dev;
	  dev.name = de->d_name;
	  if (!bsearch (&dev, ext_dev_storage, dev_storage_size, sizeof dev,
		       device_cmp))
	    break;
	}
      if (ret == ENMFILE)
	lastrealpos = dir->__d_position;
    }
  if (!dir_exists || lastrealpos)
    {
      ret = ENMFILE;
      for (size_t idx = dir->__d_position - lastrealpos + 1;
	   idx < dev_storage_size;
	   ++idx)
	{
	  struct __stat64 st;

	  ++dir->__d_position;
	  /* Exclude devices which are only available for internal purposes
	     and devices which are not really existing at this time. */
	  switch (ext_dev_storage[idx].d.major)
	    {
	    case 0:
	      if (ext_dev_storage[idx].d.minor == FH_FIFO
		  || ext_dev_storage[idx].d.minor == FH_PIPE)
		continue;
	    case DEV_PTYM_MAJOR:
	      if (ext_dev_storage[idx].d.minor
		  || !strcmp (ext_dev_storage[idx].name, "/dev/ptm0"))
		continue;
	      break;
	    case DEV_PTYS_MAJOR:
	      if (cygwin_shared->tty.connect (ext_dev_storage[idx].d.minor)
		  == -1)
		continue;
	      break;
	    case DEV_CONS_MAJOR:
	      if (!iscons_dev (myself->ctty)
		  || myself->ctty != ext_dev_storage[idx].d.devn_int)
		continue;
	      break;
	    case DEV_FLOPPY_MAJOR:
	    case DEV_TAPE_MAJOR:
	    case DEV_CDROM_MAJOR:
	    case DEV_SERIAL_MAJOR:
	    case DEV_SD_MAJOR:
	    case DEV_SD1_MAJOR:
	    case DEV_SD2_MAJOR:
	    case DEV_SD3_MAJOR:
	    case DEV_SD4_MAJOR:
	    case DEV_SD5_MAJOR:
	    case DEV_SD6_MAJOR:
	    case DEV_SD7_MAJOR:
	      {
		WCHAR wpath[MAX_PATH];
		UNICODE_STRING upath;
		OBJECT_ATTRIBUTES attr;
		HANDLE h;
		NTSTATUS status;

		sys_mbstowcs (wpath, MAX_PATH, ext_dev_storage[idx].native);
		RtlInitUnicodeString (&upath, wpath);
		InitializeObjectAttributes (&attr, &upath,
					    OBJ_CASE_INSENSITIVE, NULL, NULL);
		/* The native paths are devices, not symlinks, so we expect
		   a matching error message. */
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
	  if (!lstat64 (ext_dev_storage[idx].name, &st))
	    {
	      strcpy (de->d_name, ext_dev_storage[idx].name + dev_prefix_len);
	      de->d_ino = st.st_ino;
	      de->d_type = S_ISBLK (st.st_mode) ? DT_BLK : DT_CHR;
	      ret = 0;
	      break;
	    }
	}
    }
  return ret;
}

void
fhandler_dev::rewinddir (DIR *dir)
{
  lastrealpos = 0;
  fhandler_disk_file::rewinddir (dir);
}

