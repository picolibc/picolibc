/* dir.cc: Posix directory-related routines

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"

extern "C" int
dirfd (DIR *dir)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (dir->__d_cookie != __DIRENT_COOKIE)
    {
      set_errno (EBADF);
      syscall_printf ("-1 = dirfd (%p)", dir);
      return -1;
    }
  return dir->__d_fd;
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

  fh = build_fh_name (name, NULL, PC_SYM_FOLLOW);
  if (!fh)
    res = NULL;
  else if (fh->exists ())
    res = fh->opendir (-1);
  else
    {
      set_errno (ENOENT);
      res = NULL;
    }

  if (!res && fh)
    delete fh;
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
  myfault efault;
  if (efault.faulted ())
    return EFAULT;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    {
      syscall_printf ("%p = readdir (%p)", NULL, dir);
      return EBADF;
    }

  de->d_ino = 0;
  int res = ((fhandler_base *) dir->__fh)->readdir (dir, de);

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
	de->d_ino = readdir_get_ino (((fhandler_base *) dir->__fh)->get_name (),
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
readdir_r (DIR *dir, dirent *de, dirent **ode)
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

extern "C" _off64_t
telldir64 (DIR *dir)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    return 0;
  return ((fhandler_base *) dir->__fh)->telldir (dir);
}

/* telldir */
extern "C" _off_t
telldir (DIR *dir)
{
  return telldir64 (dir);
}

extern "C" void
seekdir64 (DIR *dir, _off64_t loc)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    return;
  dir->__flags &= dirent_info_mask;
  return ((fhandler_base *) dir->__fh)->seekdir (dir, loc);
}

/* seekdir */
extern "C" void
seekdir (DIR *dir, _off_t loc)
{
  seekdir64 (dir, (_off64_t)loc);
}

/* rewinddir: POSIX 5.1.2.1 */
extern "C" void
rewinddir (DIR *dir)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    return;
  dir->__flags &= dirent_info_mask;
  return ((fhandler_base *) dir->__fh)->rewinddir (dir);
}

/* closedir: POSIX 5.1.2.1 */
extern "C" int
closedir (DIR *dir)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    {
      set_errno (EBADF);
      syscall_printf ("-1 = closedir (%p)", dir);
      return -1;
    }

  /* Reset the marker in case the caller tries to use `dir' again.  */
  dir->__d_cookie = 0;

  int res = ((fhandler_base *) dir->__fh)->closedir (dir);

  cygheap->fdtab.release (dir->__d_fd);

  free (dir->__d_dirname);
  free (dir->__d_dirent);
  free (dir);
  syscall_printf ("%d = closedir (%p)", res);
  return res;
}

/* mkdir: POSIX 5.4.1.1 */
extern "C" int
mkdir (const char *dir, mode_t mode)
{
  int res = -1;
  fhandler_base *fh = NULL;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (!(fh = build_fh_name (dir, NULL, PC_SYM_NOFOLLOW)))
    goto done;   /* errno already set */;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else if (has_dot_last_component (dir, true))
    set_errno (fh->exists () ? EEXIST : ENOENT);
  else if (!fh->mkdir (mode))
    res = 0;
  delete fh;

 done:
  syscall_printf ("%d = mkdir (%s, %d)", res, dir, mode);
  return res;
}

/* rmdir: POSIX 5.5.2.1 */
extern "C" int
rmdir (const char *dir)
{
  int res = -1;
  fhandler_base *fh = NULL;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (!(fh = build_fh_name (dir, NULL, PC_SYM_NOFOLLOW)))
    goto done;   /* errno already set */;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else if (!fh->exists ())
    set_errno (ENOENT);
  else if (has_dot_last_component (dir, false))
    set_errno (EINVAL);
  else if (!fh->rmdir ())
    res = 0;

  delete fh;

 done:
  syscall_printf ("%d = rmdir (%s)", res, dir);
  return res;
}
