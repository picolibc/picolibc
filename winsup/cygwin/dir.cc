/* dir.cc: Posix directory-related routines

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "perprocess.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"

/* Cygwin internal */
/* Return whether the directory of a file is writable.  Return 1 if it
   is.  Otherwise, return 0, and set errno appropriately.  */
int __stdcall
writable_directory (const char *file)
{
#if 0
  char dir[strlen (file) + 1];

  strcpy (dir, file);

  const char *usedir;
  char *slash = strrchr (dir, '\\');
  if (slash == NULL)
    usedir = ".";
  else if (slash == dir)
    {
      usedir = "\\";
    }
  else
    {
      *slash = '\0';
      usedir = dir;
    }

  int acc = access (usedir, W_OK);

  return acc == 0;
#else
  return 1;
#endif
}

extern "C" int
dirfd (DIR *dir)
{
  if (check_null_invalid_struct_errno (dir))
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
  path_conv pc;
  DIR *res;

  fh = cygheap->fdtab.build_fhandler_from_name (-1, name, NULL, pc,
						PC_SYM_FOLLOW | PC_FULL, NULL);
  if (!fh)
    res = NULL;
  else if (pc.exists ())
      res = fh->opendir (pc);
  else
    {
      set_errno (ENOENT);
      res = NULL;
    }

  if (!res && fh)
    delete fh;
  return res;
}

/* readdir: POSIX 5.1.2.1 */
extern "C" struct dirent *
readdir (DIR *dir)
{
  if (check_null_invalid_struct_errno (dir))
    return NULL;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    {
      set_errno (EBADF);
      syscall_printf ("%p = readdir (%p)", NULL, dir);
      return NULL;
    }

  return ((fhandler_base *) dir->__d_u.__d_data.__fh)->readdir (dir);
}

extern "C" __off64_t
telldir64 (DIR *dir)
{
  if (check_null_invalid_struct_errno (dir))
    return -1;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    return 0;
  return ((fhandler_base *) dir->__d_u.__d_data.__fh)->telldir (dir);
}

/* telldir */
extern "C" __off32_t
telldir (DIR *dir)
{
  return telldir64 (dir);
}

extern "C" void
seekdir64 (DIR *dir, __off64_t loc)
{
  if (check_null_invalid_struct_errno (dir))
    return;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    return;
  return ((fhandler_base *) dir->__d_u.__d_data.__fh)->seekdir (dir, loc);
}

/* seekdir */
extern "C" void
seekdir (DIR *dir, __off32_t loc)
{
  seekdir64 (dir, (__off64_t)loc);
}

/* rewinddir: POSIX 5.1.2.1 */
extern "C" void
rewinddir (DIR *dir)
{
  if (check_null_invalid_struct_errno (dir))
    return;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    return;
  return ((fhandler_base *) dir->__d_u.__d_data.__fh)->rewinddir (dir);
}

/* closedir: POSIX 5.1.2.1 */
extern "C" int
closedir (DIR *dir)
{
  if (check_null_invalid_struct_errno (dir))
    return -1;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    {
      set_errno (EBADF);
      syscall_printf ("-1 = closedir (%p)", dir);
      return -1;
    }

  /* Reset the marker in case the caller tries to use `dir' again.  */
  dir->__d_cookie = 0;

  int res = ((fhandler_base *) dir->__d_u.__d_data.__fh)->closedir (dir);

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
  SECURITY_ATTRIBUTES sa = sec_none_nih;

  path_conv real_dir (dir, PC_SYM_NOFOLLOW);

  if (real_dir.error)
    {
      set_errno (real_dir.case_clash ? ECASECLASH : real_dir.error);
      goto done;
    }

  nofinalslash(real_dir.get_win32 (), real_dir.get_win32 ());
  if (! writable_directory (real_dir.get_win32 ()))
    goto done;

  if (allow_ntsec && real_dir.has_acls ())
    set_security_attribute (S_IFDIR | ((mode & 07777) & ~cygheap->umask),
			    &sa, alloca (4096), 4096);

  if (CreateDirectoryA (real_dir.get_win32 (), &sa))
    {
      if (!allow_ntsec && allow_ntea)
	set_file_attribute (real_dir.has_acls (), real_dir.get_win32 (),
			    S_IFDIR | ((mode & 07777) & ~cygheap->umask));
#ifdef HIDDEN_DOT_FILES
      char *c = strrchr (real_dir.get_win32 (), '\\');
      if ((c && c[1] == '.') || *real_dir.get_win32 () == '.')
        SetFileAttributes (real_dir.get_win32 (), FILE_ATTRIBUTE_HIDDEN);
#endif
      res = 0;
    }
  else
    __seterrno ();

done:
  syscall_printf ("%d = mkdir (%s, %d)", res, dir, mode);
  return res;
}

/* rmdir: POSIX 5.5.2.1 */
extern "C" int
rmdir (const char *dir)
{
  int res = -1;

  path_conv real_dir (dir, PC_SYM_NOFOLLOW);

  if (real_dir.error)
    {
      set_errno (real_dir.error);
      res = -1;
    }
  else if (!real_dir.exists ())
    {
      set_errno (ENOENT);
      res = -1;
    }
  else if  (!real_dir.isdir ())
    {
      set_errno (ENOTDIR);
      res = -1;
    }
  else
    {
      /* Even own directories can't be removed if R/O attribute is set. */
      if (real_dir.has_attribute (FILE_ATTRIBUTE_READONLY))
	SetFileAttributes (real_dir,
			   (DWORD) real_dir & ~FILE_ATTRIBUTE_READONLY);

      if (RemoveDirectory (real_dir))
	{
	  /* RemoveDirectory on a samba drive doesn't return an error if the
	     directory can't be removed because it's not empty. Checking for
	     existence afterwards keeps us informed about success. */
	  if (GetFileAttributes (real_dir) != INVALID_FILE_ATTRIBUTES)
	    set_errno (ENOTEMPTY);
	  else
	    res = 0;
	}
      else
	{
	  /* This kludge detects if we are attempting to remove the current working
	     directory.  If so, we will move elsewhere to potentially allow the
	     rmdir to succeed.  This means that cygwin's concept of the current working
	     directory != Windows concept but, hey, whaddaregonnado?
	     Note that this will not cause something like the following to work:
		     $ cd foo
		     $ rmdir .
	     since the shell will have foo "open" in the above case and so Windows will
	     not allow the deletion.
	     FIXME: A potential workaround for this is for cygwin apps to *never* call
	     SetCurrentDirectory. */
	  if (strcasematch (real_dir, cygheap->cwd.win32)
	      && !strcasematch ("c:\\", cygheap->cwd.win32))
	    {
	      DWORD err = GetLastError ();
	      if (!SetCurrentDirectory ("c:\\"))
		SetLastError (err);
	      else if ((res = rmdir (dir)))
		SetCurrentDirectory (cygheap->cwd.win32);
	    }
	  if (GetLastError () == ERROR_ACCESS_DENIED)
	    {

	      /* On 9X ERROR_ACCESS_DENIED is returned if you try to remove
		 a non-empty directory. */
	      if (wincap.access_denied_on_delete ())
		set_errno (ENOTEMPTY);
	      else
		__seterrno ();
	    }
	  else
	    __seterrno ();

	  /* If directory still exists, restore R/O attribute. */
	  if (real_dir.has_attribute (FILE_ATTRIBUTE_READONLY))
	    SetFileAttributes (real_dir, real_dir);
	}
    }

  syscall_printf ("%d = rmdir (%s)", res, dir);
  return res;
}
