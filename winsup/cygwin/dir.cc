/* dir.cc: Posix directory-related routines

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
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
#include "fhandler.h"
#include "path.h"
#include "security.h"
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

/* opendir: POSIX 5.1.2.1 */
extern "C" DIR *
opendir (const char *dirname)
{
  int len;
  DIR *dir;
  DIR *res = 0;
  struct stat statbuf;

  path_conv real_dirname (dirname, PC_SYM_FOLLOW | PC_FULL);

  if (real_dirname.error)
    {
      set_errno (real_dirname.error);
      goto failed;
    }

  if (stat (cygheap->rootlen ? dirname : real_dirname.get_win32 (),
	    &statbuf) == -1)
    goto failed;

  if (!(statbuf.st_mode & S_IFDIR))
    {
      set_errno (ENOTDIR);
      goto failed;
    }

  len = strlen (real_dirname.get_win32 ());
  if (len > MAX_PATH - 3)
    {
      set_errno (ENAMETOOLONG);
      goto failed;
    }

  if ((dir = (DIR *) malloc (sizeof (DIR))) == NULL)
    {
      set_errno (ENOMEM);
      goto failed;
    }
  if ((dir->__d_dirname = (char *) malloc (len + 3)) == NULL)
    {
      free (dir);
      set_errno (ENOMEM);
      goto failed;
    }
  if ((dir->__d_dirent =
	    (struct dirent *) malloc (sizeof (struct dirent))) == NULL)
    {
      free (dir->__d_dirname);
      free (dir);
      set_errno (ENOMEM);
      goto failed;
    }
  strcpy (dir->__d_dirname, real_dirname.get_win32 ());
  /* FindFirstFile doesn't seem to like duplicate /'s. */
  len = strlen (dir->__d_dirname);
  if (len == 0 || SLASH_P (dir->__d_dirname[len - 1]))
    strcat (dir->__d_dirname, "*");
  else
    strcat (dir->__d_dirname, "\\*");  /**/
  dir->__d_cookie = __DIRENT_COOKIE;
  dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
  dir->__d_position = 0;
  dir->__d_dirhash = statbuf.st_ino;

  res = dir;

failed:
  syscall_printf ("%p = opendir (%s)", res, dirname);
  return res;
}

/* readdir: POSIX 5.1.2.1 */
extern "C" struct dirent *
readdir (DIR * dir)
{
  WIN32_FIND_DATA buf;
  HANDLE handle;
  struct dirent *res = NULL;

  if (dir->__d_cookie != __DIRENT_COOKIE)
    {
      set_errno (EBADF);
      syscall_printf ("%p = readdir (%p)", res, dir);
      return res;
    }

  if (dir->__d_u.__d_data.__handle == INVALID_HANDLE_VALUE
      && dir->__d_position == 0)
    {
      handle = FindFirstFileA (dir->__d_dirname, &buf);
      DWORD lasterr = GetLastError ();
      dir->__d_u.__d_data.__handle = handle;
      if (handle == INVALID_HANDLE_VALUE && (lasterr != ERROR_NO_MORE_FILES))
	{
	  seterrno_from_win_error (__FILE__, __LINE__, lasterr);
	  return res;
	}
    }
  else if (!FindNextFileA (dir->__d_u.__d_data.__handle, &buf))
    {
      DWORD lasterr = GetLastError ();
      (void) FindClose (dir->__d_u.__d_data.__handle);
      dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
      /* POSIX says you shouldn't set errno when readdir can't
	 find any more files; so, if another error we leave it set. */
      if (lasterr != ERROR_NO_MORE_FILES)
	  seterrno_from_win_error (__FILE__, __LINE__, lasterr);
      syscall_printf ("%p = readdir (%p)", res, dir);
      return res;
    }

  /* We get here if `buf' contains valid data.  */
  strcpy (dir->__d_dirent->d_name, buf.cFileName);

  /* Compute d_ino by combining filename hash with the directory hash
     (which was stored in dir->__d_dirhash when opendir was called). */
  if (buf.cFileName[0] == '.')
    {
      if (buf.cFileName[1] == '\0')
	dir->__d_dirent->d_ino = dir->__d_dirhash;
      else if (buf.cFileName[1] != '.' || buf.cFileName[2] != '\0')
	goto hashit;
      else
	{
	  char *p, up[strlen (dir->__d_dirname) + 1];
	  strcpy (up, dir->__d_dirname);
	  if (!(p = strrchr (up, '\\')))
	    goto hashit;
	  *p = '\0';
	  if (!(p = strrchr (up, '\\')))
	    dir->__d_dirent->d_ino = hash_path_name (0, ".");
	  else
	    {
	      *p = '\0';
	      dir->__d_dirent->d_ino = hash_path_name (0, up);
	    }
	}
    }
  else
    {
  hashit:
      ino_t dino = hash_path_name (dir->__d_dirhash, "\\");
      dir->__d_dirent->d_ino = hash_path_name (dino, buf.cFileName);
    }

  ++dir->__d_position;
  res = dir->__d_dirent;
  syscall_printf ("%p = readdir (%p) (%s)",
		  &dir->__d_dirent, dir, buf.cFileName);
  return res;
}

/* telldir */
extern "C" off_t
telldir (DIR * dir)
{
  if (dir->__d_cookie != __DIRENT_COOKIE)
    return 0;
  return dir->__d_position;
}

/* seekdir */
extern "C" void
seekdir (DIR * dir, off_t loc)
{
  if (dir->__d_cookie != __DIRENT_COOKIE)
    return;
  rewinddir (dir);
  while (loc > dir->__d_position)
    if (! readdir (dir))
      break;
}

/* rewinddir: POSIX 5.1.2.1 */
extern "C" void
rewinddir (DIR * dir)
{
  syscall_printf ("rewinddir (%p)", dir);

  if (dir->__d_cookie != __DIRENT_COOKIE)
    return;
  if (dir->__d_u.__d_data.__handle != INVALID_HANDLE_VALUE)
    {
      (void) FindClose (dir->__d_u.__d_data.__handle);
      dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
    }
  dir->__d_position = 0;
}

/* closedir: POSIX 5.1.2.1 */
extern "C" int
closedir (DIR * dir)
{
  if (dir->__d_cookie != __DIRENT_COOKIE)
    {
      set_errno (EBADF);
      syscall_printf ("-1 = closedir (%p)", dir);
      return -1;
    }

  if (dir->__d_u.__d_data.__handle != INVALID_HANDLE_VALUE &&
      FindClose (dir->__d_u.__d_data.__handle) == 0)
    {
      __seterrno ();
      syscall_printf ("-1 = closedir (%p)", dir);
      return -1;
    }

  /* Reset the marker in case the caller tries to use `dir' again.  */
  dir->__d_cookie = 0;

  free (dir->__d_dirname);
  free (dir->__d_dirent);
  free (dir);
  syscall_printf ("0 = closedir (%p)", dir);
  return 0;
}

/* mkdir: POSIX 5.4.1.1 */
extern "C" int
mkdir (const char *dir, mode_t mode)
{
  int res = -1;

  path_conv real_dir (dir, PC_SYM_NOFOLLOW);

  if (real_dir.error)
    {
      set_errno (real_dir.error);
      goto done;
    }

  nofinalslash(real_dir.get_win32 (), real_dir.get_win32 ());
  if (! writable_directory (real_dir.get_win32 ()))
    goto done;

  if (CreateDirectoryA (real_dir.get_win32 (), 0))
    {
      set_file_attribute (real_dir.has_acls (), real_dir.get_win32 (),
			  S_IFDIR | ((mode & 0777) & ~cygheap->umask));
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
      goto done;
    }

  if (RemoveDirectoryA (real_dir.get_win32 ()))
    {
      /* RemoveDirectory on a samba drive doesn't return an error if the
	 directory can't be removed because it's not empty. Checking for
	 existence afterwards keeps us informed about success. */
      if (GetFileAttributesA (real_dir.get_win32 ()) != (DWORD) -1)
	set_errno (ENOTEMPTY);
      else
	res = 0;
    }
  else if (GetLastError() == ERROR_ACCESS_DENIED)
    {
      /* Under Windows 9X or on a samba share, ERROR_ACCESS_DENIED is
	 returned if you try to remove a file. On 9X the same error is
	 returned if you try to remove a non-empty directory. */
     if (real_dir.file_attributes () != (DWORD) -1 &&
	 !(real_dir.file_attributes () & FILE_ATTRIBUTE_DIRECTORY))
       set_errno (ENOTDIR);
     else if (os_being_run != winNT)
       set_errno (ENOTEMPTY);
     else
       __seterrno ();
    }
  else
    __seterrno ();

done:
  syscall_printf ("%d = rmdir (%s)", res, dir);
  return res;
}
