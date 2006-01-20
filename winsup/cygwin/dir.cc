/* dir.cc: Posix directory-related routines

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

#include "pinfo.h"
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"
#include "perprocess.h"
#include "cygwin/version.h"

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
  return dir->__d_dirent->d_fd;
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
      res = fh->opendir ();
  else
    {
      set_errno (ENOENT);
      res = NULL;
    }

  if (res)
    /* nothing */;
  else if (fh)
    delete fh;
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

  if (res)
    /* error return */;
  else if (!CYGWIN_VERSION_CHECK_FOR_NEEDS_D_INO)
    {
      de->__invalid_d_ino = (ino_t) 0;
      de->__invalid_ino32 = (uint32_t) 0;
      if (de->d_name[0] == '.')
	{
	  if (de->d_name[1] == '\0')
	     dir->__flags |= dirent_saw_dot;
	   else if (de->d_name[1] == '.' && de->d_name[2] == '\0')
	     dir->__flags |= dirent_saw_dot_dot;
	 }
    }
  else
    {
      /* Compute d_ino by combining filename hash with the directory hash
	 (which was stored in dir->__d_dirhash when opendir was called). */
      if (de->d_name[0] == '.')
	{
	  if (de->d_name[1] == '\0')
	    {
	      de->__invalid_d_ino = dir->__d_dirhash;
	      dir->__flags |= dirent_saw_dot;
	    }
	  else if (de->d_name[1] != '.' || de->d_name[2] != '\0')
	    goto hashit;
	  else
	    {
	      dir->__flags |= dirent_saw_dot_dot;
	      char *p, up[strlen (dir->__d_dirname) + 1];
	      strcpy (up, dir->__d_dirname);
	      if (!(p = strrchr (up, '\\')))
		goto hashit;
	      *p = '\0';
	      if (!(p = strrchr (up, '\\')))
		de->__invalid_d_ino = hash_path_name (0, ".");
	      else
		{
		  *p = '\0';
		  de->__invalid_d_ino = hash_path_name (0, up);
		}
	    }
	}
      else
	{
      hashit:
	  __ino64_t dino = hash_path_name (dir->__d_dirhash, "\\");
	  de->__invalid_d_ino = hash_path_name (dino, de->d_name);
	}
      de->__invalid_ino32 = de->__invalid_d_ino;	// for legacy applications
    }
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
  dir->__flags &= dirent_isroot;
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
  dir->__flags &= dirent_isroot;
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

  cygheap->fdtab.release (dir->__d_dirent->d_fd);

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

  if (has_dot_last_component (dir))
    {
      set_errno (ENOENT);
      return -1;
    }

  if (!(fh = build_fh_name (dir, NULL, PC_SYM_NOFOLLOW)))
    goto done;   /* errno already set */;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
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

  if (has_dot_last_component (dir))
    {
      set_errno (EINVAL);
      return -1;
    }

  if (!(fh = build_fh_name (dir, NULL, PC_SYM_NOFOLLOW)))
    goto done;   /* errno already set */;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else if (!fh->rmdir ())
    res = 0;
  delete fh;

 done:
  syscall_printf ("%d = rmdir (%s)", res, dir);
  return res;
}
