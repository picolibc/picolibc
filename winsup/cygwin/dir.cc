/* dir.cc: Posix directory-related routines

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <unistd.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"
#include "tls_pbuf.h"

extern "C" int
dirfd (DIR *dir)
{
  __try
    {
      if (dir->__d_cookie == __DIRENT_COOKIE)
	return dir->__d_fd;
      syscall_printf ("-1 = dirfd (%p)", dir);
      set_errno (EINVAL);
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

/* Symbol kept for backward compatibility.  Don't remove.  Don't declare
   in public header file. */
extern "C" DIR *
__opendir_with_d_ino (const char *name)
{
  return opendir (name);
}

/* opendir: POSIX 5.1.2.1 */
extern "C" DIR *
opendir (const char *name)
{
  fhandler_base *fh;
  DIR *res;

  fh = build_fh_name (name, PC_SYM_FOLLOW);
  if (!fh)
    res = NULL;
  else if (fh->error ())
    {
      set_errno (fh->error ());
      res = NULL;
    }
  else if (fh->exists ())
    res = fh->opendir (-1);
  else
    {
      set_errno (ENOENT);
      res = NULL;
    }

  if (!res && fh)
    delete fh;
  /* Applications calling flock(2) on dirfd(fd) need this... */
  if (res && !fh->nohandle ())
    fh->set_unique_id ();
  return res;
}

extern "C" DIR *
fdopendir (int fd)
{
  DIR *res = NULL;

  cygheap_fdget cfd (fd);
  if (cfd >= 0)
    res = cfd->opendir (fd);
  return res;
}

static int
readdir_worker (DIR *dir, dirent *de)
{
  int res = 0;

  __try
    {
      if (dir->__d_cookie != __DIRENT_COOKIE)
	{
	  syscall_printf ("%p = readdir (%p)", NULL, dir);
	  res = EBADF;
	  __leave;
	}

      de->d_ino = 0;
      de->d_type = DT_UNKNOWN;
      memset (&de->__d_unused1, 0, sizeof (de->__d_unused1));

      res = ((fhandler_base *) dir->__fh)->readdir (dir, de);

      if (res == ENMFILE)
	{
	  if (!(dir->__flags & dirent_saw_dot))
	    {
	      strcpy (de->d_name, ".");
	      dir->__flags |= dirent_saw_dot;
	      dir->__d_position++;
	      res = 0;
	    }
	  else if (!(dir->__flags & dirent_saw_dot_dot))
	    {
	      strcpy (de->d_name, "..");
	      dir->__flags |= dirent_saw_dot_dot;
	      dir->__d_position++;
	      res = 0;
	    }
	}

      if (!res && !de->d_ino)
	{
	  bool is_dot = false;
	  bool is_dot_dot = false;

	  if (de->d_name[0] == '.')
	    {
	      if (de->d_name[1] == '\0')
		is_dot = true;
	      else if (de->d_name[1] == '.' && de->d_name[2] == '\0')
		is_dot_dot = true;
	    }

	  if (is_dot_dot && !(dir->__flags & dirent_isroot))
	    de->d_ino = readdir_get_ino (((fhandler_base *)
	    				 dir->__fh)->get_name (),
					 true);
	  else
	    {
	      /* Compute d_ino by combining filename hash with directory hash. */
	      de->d_ino = ((fhandler_base *) dir->__fh)->get_ino ();
	      if (!is_dot && !is_dot_dot)
		{
		  PUNICODE_STRING w32name =
		      ((fhandler_base *) dir->__fh)->pc.get_nt_native_path ();
		  DWORD devn = ((fhandler_base *) dir->__fh)->get_device ();
		  /* Paths below /proc don't have a Win32 pendant. */
		  if (isproc_dev (devn))
		    de->d_ino = hash_path_name (de->d_ino, L"/");
		  else if (w32name->Buffer[w32name->Length / sizeof (WCHAR) - 1]
			   != L'\\')
		    de->d_ino = hash_path_name (de->d_ino, L"\\");
		  de->d_ino = hash_path_name (de->d_ino, de->d_name);
		}
	    }
	}
      /* This fills out the old 32 bit d_ino field for old applications,
	 build under Cygwin before 1.5.x. */
      de->__d_internal1 = de->d_ino;
    }
  __except (NO_ERROR)
    {
      res = EFAULT;
    }
  __endtry
  return res;
}

/* readdir: POSIX 5.1.2.1 */
extern "C" struct dirent *
readdir (DIR *dir)
{
  int res = readdir_worker (dir, dir->__d_dirent);
  if (res == 0)
    return dir->__d_dirent;
  if (res != ENMFILE)
    set_errno (res);
  return NULL;
}

extern "C" int
readdir_r (DIR *__restrict dir, dirent *__restrict de, dirent **__restrict ode)
{
  int res = readdir_worker (dir, de);
  if (!res)
    *ode = de;
  else
    {
      *ode = NULL;
      if (res == ENMFILE)
	res = 0;
    }
  return res;
}

/* telldir */
extern "C" long
telldir (DIR *dir)
{
  __try
    {
      if (dir->__d_cookie == __DIRENT_COOKIE)
	return ((fhandler_base *) dir->__fh)->telldir (dir);
      set_errno (EBADF);
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

/* telldir was never defined using off_t in POSIX, only in early versions
   of glibc.  We have to keep the function in as entry point for backward
   compatibility. */
extern "C" off_t
telldir64 (DIR *dir)
{
  return (off_t) telldir (dir);
}

/* seekdir */
extern "C" void
seekdir (DIR *dir, long loc)
{
  __try
    {
      if (dir->__d_cookie == __DIRENT_COOKIE)
	{
	  dir->__flags &= dirent_info_mask;
	  ((fhandler_base *) dir->__fh)->seekdir (dir, loc);
	}
      set_errno (EINVAL);	/* Diagnosis */
    }
  __except (EFAULT) {}
  __endtry
}

/* seekdir was never defined using off_t in POSIX, only in early versions
   of glibc.  We have to keep the function in as entry point for backward
   compatibility. */
extern "C" void
seekdir64 (DIR *dir, off_t loc)
{
  seekdir (dir, (long) loc);
}

/* rewinddir: POSIX 5.1.2.1 */
extern "C" void
rewinddir (DIR *dir)
{
  __try
    {
      if (dir->__d_cookie == __DIRENT_COOKIE)
	{
	  dir->__flags &= dirent_info_mask;
	  ((fhandler_base *) dir->__fh)->rewinddir (dir);
	}
      set_errno (EINVAL);	/* Diagnosis */
    }
  __except (EFAULT) {}
  __endtry
}

/* closedir: POSIX 5.1.2.1 */
extern "C" int
closedir (DIR *dir)
{
  __try
    {
      if (dir->__d_cookie == __DIRENT_COOKIE)
	{
	  /* Reset the marker in case the caller tries to use `dir' again.  */
	  dir->__d_cookie = 0;

	  int res = ((fhandler_base *) dir->__fh)->closedir (dir);

	  close (dir->__d_fd);
	  free (dir->__d_dirname);
	  free (dir->__d_dirent);
	  free (dir);
	  syscall_printf ("%R = closedir(%p)", res, dir);
	  return res;
	}
      set_errno (EBADF);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = closedir(%p)", -1, dir);
  return -1;
}

/* mkdir: POSIX 5.4.1.1 */
extern "C" int
mkdir (const char *dir, mode_t mode)
{
  int res = -1;
  fhandler_base *fh = NULL;
  tmp_pathbuf tp;

  __try
    {
      if (!*dir)
	{
	  set_errno (ENOENT);
	  __leave;
	}
      /* Following Linux, and intentionally ignoring POSIX, do not
	 resolve the last component of DIR if it is a symlink, even if
	 DIR has a trailing slash.  Achieve this by stripping trailing
	 slashes or backslashes.

	 Exception: If DIR == 'x:' followed by one or more slashes or
	 backslashes, and if there's at least one backslash, assume
	 that the user is referring to the root directory of drive x.
	 Retain one backslash in this case.  */
      if (isdirsep (dir[strlen (dir) - 1]))
	{
	  /* This converts // to /, but since both give EEXIST, we're okay.  */
	  char *buf;
	  char *p = stpcpy (buf = tp.c_get (), dir) - 1;
	  bool msdos = false;
	  dir = buf;
	  while (p > dir && isdirsep (*p))
	    {
	      if (*p == '\\')
		msdos = true;
	      *p-- = '\0';
	    }
	  if (msdos && p == dir + 1 && isdrive (dir))
	    p[1] = '\\';
	}
      if (!(fh = build_fh_name (dir, PC_SYM_NOFOLLOW)))
	__leave;   /* errno already set */;

      if (fh->error ())
	{
	  debug_printf ("got %d error from build_fh_name", fh->error ());
	  set_errno (fh->error ());
	}
      else if (fh->exists ())
	set_errno (EEXIST);
      else if (has_dot_last_component (dir, true))
	set_errno (ENOENT);
      else if (!fh->mkdir (mode))
	res = 0;
      delete fh;
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = mkdir(%s, %d)", res, dir, mode);
  return res;
}

/* rmdir: POSIX 5.5.2.1 */
extern "C" int
rmdir (const char *dir)
{
  int res = -1;
  fhandler_base *fh = NULL;
  tmp_pathbuf tp;

  __try
    {
      if (!*dir)
	{
	  set_errno (ENOENT);
	  __leave;
	}
      /* Following Linux, and intentionally ignoring POSIX, do not
	 resolve the last component of DIR if it is a symlink, even if
	 DIR has a trailing slash.  Achieve this by stripping trailing
	 slashes or backslashes.

	 Exception: If DIR == 'x:' followed by one or more slashes or
	 backslashes, and if there's at least one backslash, assume
	 that the user is referring to the root directory of drive x.
	 Retain one backslash in this case.  */
      if (isdirsep (dir[strlen (dir) - 1]))
	{
	  /* This converts // to /, but since both give ENOTEMPTY,
	     we're okay.  */
	  char *buf;
	  char *p = stpcpy (buf = tp.c_get (), dir) - 1;
	  bool msdos = false;
	  dir = buf;
	  while (p > dir && isdirsep (*p))
	    {
	      if (*p == '\\')
		msdos = true;
	      *p-- = '\0';
	    }
	  if (msdos && p == dir + 1 && isdrive (dir))
	    p[1] = '\\';
	}
      if (!(fh = build_fh_name (dir, PC_SYM_NOFOLLOW)))
	__leave;   /* errno already set */;

      if (fh->error ())
	{
	  debug_printf ("got %d error from build_fh_name", fh->error ());
	  set_errno (fh->error ());
	}
      else if (!fh->exists ())
	set_errno (ENOENT);
      else if (has_dot_last_component (dir, false))
	set_errno (EINVAL);
      else if (isdev_dev (fh->dev ()))
	set_errno (ENOTEMPTY);
      else if (!fh->rmdir ())
	res = 0;
      delete fh;
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = rmdir(%s)", res, dir);
  return res;
}
