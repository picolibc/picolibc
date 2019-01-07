/* fhandler_cygdrive.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <lm.h>
#include <sys/statvfs.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

fhandler_cygdrive::fhandler_cygdrive () :
  fhandler_disk_file ()
{
}

int
fhandler_cygdrive::open (int flags, mode_t mode)
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
  /* Open a fake handle to \\Device\\Null */
  return open_null (flags);
}

int
fhandler_cygdrive::fstat (struct stat *buf)
{
  fhandler_base::fstat (buf);
  buf->st_ino = 2;
  buf->st_mode = S_IFDIR | STD_RBITS | STD_XBITS;
  buf->st_nlink = 1;
  return 0;
}

int __reg2
fhandler_cygdrive::fstatvfs (struct statvfs *sfs)
{
  /* Virtual file system.  Just return an empty buffer with a few values
     set to something useful.  Just as on Linux. */
  memset (sfs, 0, sizeof (*sfs));
  sfs->f_bsize = sfs->f_frsize = 4096;
  sfs->f_flag = ST_RDONLY;
  sfs->f_namemax = NAME_MAX;
  return 0;
}

#define MAX_DRIVE_BUF_LEN	(sizeof ("x:\\") * 26 + 2)

struct __DIR_drives
{
  char *pdrive;
  char  pbuf[MAX_DRIVE_BUF_LEN];
};

#define d_drives(d)	((__DIR_drives *) (d)->__d_internal)

DIR *
fhandler_cygdrive::opendir (int fd)
{
  DIR *dir;

  dir = fhandler_disk_file::opendir (fd);
  if (dir)
    {
      dir->__d_internal = (uintptr_t) new __DIR_drives;
      GetLogicalDriveStrings (MAX_DRIVE_BUF_LEN, d_drives(dir)->pbuf);
      d_drives(dir)->pdrive = d_drives(dir)->pbuf;
    }

  return dir;
}

int
fhandler_cygdrive::readdir (DIR *dir, dirent *de)
{
  WCHAR drive[] = L"X:";

  while (true)
    {
      if (!d_drives(dir)->pdrive || !*d_drives(dir)->pdrive)
	{
	  if (!(dir->__flags & dirent_saw_dot))
	    {
	      de->d_name[0] = '.';
	      de->d_name[1] = '\0';
	      de->d_ino = 2;
	    }
	  return ENMFILE;
	}
      disk_type dt = get_disk_type ((drive[0] = *d_drives(dir)->pdrive, drive));
      if (dt == DT_SHARE_SMB)
	{
	  /* Calling NetUseGetInfo on SMB drives allows to fetch the
	     current state of the drive without trying to open a file
	     descriptor on the share (GetFileAttributes).  This avoids
	     waiting for SMB timeouts.  Of course, there's a downside:
	     If a drive becomes availabe again, it can take a couple of
	     minutes to recognize it. As long as this didn't happen,
	     the drive will not show up in the cygdrive dir. */
	  PUSE_INFO_1 pui1;
	  DWORD status;

	  if (NetUseGetInfo (NULL, drive, 1, (PBYTE *) &pui1) == NERR_Success)
	    {
	      status = pui1->ui1_status;
	      NetApiBufferFree (pui1);
	      if (status == USE_OK)
		break;
	    }
	}
      else if (dt != DT_FLOPPY
	       && GetFileAttributes (d_drives(dir)->pdrive) != INVALID_FILE_ATTRIBUTES)
	break;
      d_drives(dir)->pdrive = strchr (d_drives(dir)->pdrive, '\0') + 1;
    }
  *de->d_name = cyg_tolower (*d_drives(dir)->pdrive);
  de->d_name[1] = '\0';
  user_shared->warned_msdos = true;
  de->d_ino = readdir_get_ino (d_drives(dir)->pdrive, false);
  dir->__d_position++;
  d_drives(dir)->pdrive = strchr (d_drives(dir)->pdrive, '\0') + 1;
  syscall_printf ("%p = readdir (%p) (%s)", &de, dir, de->d_name);
  return 0;
}

void
fhandler_cygdrive::rewinddir (DIR *dir)
{
  d_drives(dir)->pdrive = d_drives(dir)->pbuf;
  dir->__d_position = 0;
}

int
fhandler_cygdrive::closedir (DIR *dir)
{
  delete d_drives(dir);
  return 0;
}
