/* fhandler_registry.cc: fhandler for /proc/registry virtual filesystem

   Copyright 2002, 2003, 2003, 2004, 2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* FIXME: Access permissions are ignored at the moment.  */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include <assert.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

/* If this bit is set in __d_position then we are enumerating values,
 * else sub-keys. keeping track of where we are is horribly messy
 * the bottom 16 bits are the absolute position and the top 15 bits
 * make up the value index if we are enuerating values.
 */
static const _off_t REG_ENUM_VALUES_MASK = 0x8000000;
static const _off_t REG_POSITION_MASK = 0xffff;

/* List of root keys in /proc/registry.
 * Possibly we should filter out those not relevant to the flavour of Windows
 * Cygwin is running on.
 */
static const char *registry_listing[] =
{
  ".",
  "..",
  "HKEY_CLASSES_ROOT",
  "HKEY_CURRENT_CONFIG",
  "HKEY_CURRENT_USER",
  "HKEY_LOCAL_MACHINE",
  "HKEY_USERS",
  "HKEY_PERFORMANCE_DATA",	// NT/2000/XP
  NULL
};

static const HKEY registry_keys[] =
{
  (HKEY) INVALID_HANDLE_VALUE,
  (HKEY) INVALID_HANDLE_VALUE,
  HKEY_CLASSES_ROOT,
  HKEY_CURRENT_CONFIG,
  HKEY_CURRENT_USER,
  HKEY_LOCAL_MACHINE,
  HKEY_USERS,
  HKEY_PERFORMANCE_DATA
};

static const int ROOT_KEY_COUNT = sizeof (registry_keys) / sizeof (HKEY);

/* These get added to each subdirectory in /proc/registry.
 * If we wanted to implement writing, we could maybe add a '.writable' entry or
 * suchlike.
 */
static const char *special_dot_files[] =
{
  ".",
  "..",
  NULL
};

static const int SPECIAL_DOT_FILE_COUNT =
  (sizeof (special_dot_files) / sizeof (const char *)) - 1;

/* Name given to default values */
static const char *DEFAULT_VALUE_NAME = "@";

static HKEY open_key (const char *name, REGSAM access, DWORD wow64, bool isValue);

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * <0 if path is a file.
 *
 * We open the last key but one and then enum it's sub-keys and values to see if the
 * final component is there. This gets round the problem of not having security access
 * to the final key in the path.
 */
int
fhandler_registry::exists ()
{
  int file_type = 0, index = 0, pathlen;
  DWORD buf_size = CYG_MAX_PATH;
  LONG error;
  char buf[buf_size];
  const char *file;
  HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;

  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + prefix_len + 1;
  if (*path)
    path++;
  else
    {
      file_type = 2;
      goto out;
    }
  pathlen = strlen (path);
  file = path + pathlen - 1;
  if (isdirsep (*file) && pathlen > 1)
    file--;
  while (!isdirsep (*file))
    file--;
  file++;

  if (file == path)
    {
      for (int i = 0; registry_listing[i]; i++)
	if (path_prefix_p
	    (registry_listing[i], path, strlen (registry_listing[i])))
	  {
	    file_type = 1;
	    goto out;
	  }
      goto out;
    }

  hKey = open_key (path, KEY_READ, wow64, false);
  if (hKey != (HKEY) INVALID_HANDLE_VALUE)
    file_type = 1;
  else
    {
      hKey = open_key (path, KEY_READ, wow64, true);
      if (hKey == (HKEY) INVALID_HANDLE_VALUE)
	return 0;

      while (ERROR_SUCCESS ==
	     (error = RegEnumKeyEx (hKey, index++, buf, &buf_size, NULL, NULL,
				    NULL, NULL))
	     || (error == ERROR_MORE_DATA))
	{
	  if (pathmatch (buf, file))
	    {
	      file_type = 1;
	      goto out;
	    }
	  buf_size = CYG_MAX_PATH;
	}
      if (error != ERROR_NO_MORE_ITEMS)
	{
	  seterrno_from_win_error (__FILE__, __LINE__, error);
	  goto out;
	}
      index = 0;
      buf_size = CYG_MAX_PATH;
      while (ERROR_SUCCESS ==
	     (error = RegEnumValue (hKey, index++, buf, &buf_size, NULL, NULL,
				    NULL, NULL))
	     || (error == ERROR_MORE_DATA))
	{
	  if (pathmatch (buf, file) || (buf[0] == '\0' &&
					pathmatch (file, DEFAULT_VALUE_NAME)))
	    {
	      file_type = -1;
	      goto out;
	    }
	  buf_size = CYG_MAX_PATH;
	}
      if (error != ERROR_NO_MORE_ITEMS)
	{
	  seterrno_from_win_error (__FILE__, __LINE__, error);
	  goto out;
	}
    }
out:
  if (hKey != (HKEY) INVALID_HANDLE_VALUE)
    RegCloseKey (hKey);
  return file_type;
}

void
fhandler_registry::set_name (path_conv &in_pc)
{
  if (strncasematch (in_pc.normalized_path, "/proc/registry32", 16))
    {
      wow64 = KEY_WOW64_32KEY;
      prefix_len += 2;
    }
  else if (strncasematch (in_pc.normalized_path, "/proc/registry64", 16))
    {
      wow64 = KEY_WOW64_64KEY;
      prefix_len += 2;
    }
  fhandler_base::set_name (in_pc);
}

fhandler_registry::fhandler_registry ():
fhandler_proc ()
{
  wow64 = 0;
  prefix_len = sizeof ("registry") - 1;
}

int
fhandler_registry::fstat (struct __stat64 *buf)
{
  fhandler_base::fstat (buf);
  buf->st_mode &= ~_IFMT & NO_W;
  int file_type = exists ();
  switch (file_type)
    {
    case 0:
      set_errno (ENOENT);
      return -1;
    case 1:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      break;
    case 2:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      buf->st_nlink = ROOT_KEY_COUNT;
      break;
    default:
    case -1:
      buf->st_mode |= S_IFREG;
      buf->st_mode &= NO_X;
      break;
    }
  if (file_type != 0 && file_type != 2)
    {
      HKEY hKey;
      const char *path = get_name () + proc_len + prefix_len + 2;
      hKey =
	open_key (path, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE, wow64,
		  (file_type < 0) ? true : false);

      if (hKey != (HKEY) INVALID_HANDLE_VALUE)
	{
	  FILETIME ftLastWriteTime;
	  DWORD subkey_count;
	  if (ERROR_SUCCESS ==
	      RegQueryInfoKey (hKey, NULL, NULL, NULL, &subkey_count, NULL,
			       NULL, NULL, NULL, NULL, NULL,
			       &ftLastWriteTime))
	    {
	      to_timestruc_t (&ftLastWriteTime, &buf->st_mtim);
	      buf->st_ctim = buf->st_birthtim = buf->st_mtim;
	      time_as_timestruc_t (&buf->st_atim);
	      if (file_type > 0)
		buf->st_nlink = subkey_count + 2;
	      else
		{
		  int pathlen = strlen (path);
		  const char *value_name = path + pathlen - 1;
		  if (isdirsep (*value_name) && pathlen > 1)
		    value_name--;
		  while (!isdirsep (*value_name))
		    value_name--;
		  value_name++;
		  DWORD dwSize;
		  if (ERROR_SUCCESS ==
		      RegQueryValueEx (hKey, value_name, NULL, NULL, NULL,
				       &dwSize))
		    buf->st_size = dwSize;
		}
	      __uid32_t uid;
	      __gid32_t gid;
	      if (get_reg_attribute (hKey, &buf->st_mode, &uid, &gid) == 0)
		{
		  buf->st_uid = uid;
		  buf->st_gid = gid;
		  buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
		  if (file_type > 0)
		    buf->st_mode |= S_IFDIR;
		  else
		    buf->st_mode &= NO_X;
		}
	    }
	  RegCloseKey (hKey);
	}
      else
	{
	  /* Here's the problem:  If we can't open the key, we don't know
	     nothing at all about the key/value.  It's only clear that
	     the current user has no read access.  At this point it's
	     rather unlikely that the user has write or execute access
	     and it's also rather unlikely that the user is the owner.
	     Therefore it's probably most safe to assume unknown ownership
	     and no permissions for nobody. */
	  buf->st_uid = UNKNOWN_UID;
	  buf->st_gid = UNKNOWN_GID;
	  buf->st_mode &= ~0777;
	}
    }
  return 0;
}

int
fhandler_registry::readdir (DIR *dir, dirent *de)
{
  DWORD buf_size = CYG_MAX_PATH;
  char buf[buf_size];
  HANDLE handle;
  const char *path = dir->__d_dirname + proc_len + 1 + prefix_len;
  LONG error;
  int res = ENMFILE;

  dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
  if (*path == 0)
    {
      if (dir->__d_position >= ROOT_KEY_COUNT)
	goto out;
      strcpy (de->d_name, registry_listing[dir->__d_position++]);
      res = 0;
      goto out;
    }
  if (dir->__handle == INVALID_HANDLE_VALUE && dir->__d_position == 0)
    {
      handle = open_key (path + 1, KEY_READ, wow64, false);
      dir->__handle = handle;
    }
  if (dir->__handle == INVALID_HANDLE_VALUE)
    goto out;
  if (dir->__d_position < SPECIAL_DOT_FILE_COUNT)
    {
      strcpy (de->d_name, special_dot_files[dir->__d_position++]);
      res = 0;
      goto out;
    }
retry:
  if (dir->__d_position & REG_ENUM_VALUES_MASK)
    /* For the moment, the type of key is ignored here. when write access is added,
     * maybe add an extension for the type of each value?
     */
    error = RegEnumValue ((HKEY) dir->__handle,
			  (dir->__d_position & ~REG_ENUM_VALUES_MASK) >> 16,
			  buf, &buf_size, NULL, NULL, NULL, NULL);
  else
    error =
      RegEnumKeyEx ((HKEY) dir->__handle, dir->__d_position -
		    SPECIAL_DOT_FILE_COUNT, buf, &buf_size, NULL, NULL, NULL,
		    NULL);
  if (error == ERROR_NO_MORE_ITEMS
      && (dir->__d_position & REG_ENUM_VALUES_MASK) == 0)
    {
      /* If we're finished with sub-keys, start on values under this key.  */
      dir->__d_position |= REG_ENUM_VALUES_MASK;
      buf_size = CYG_MAX_PATH;
      goto retry;
    }
  if (error != ERROR_SUCCESS && error != ERROR_MORE_DATA)
    {
      RegCloseKey ((HKEY) dir->__handle);
      dir->__handle = INVALID_HANDLE_VALUE;
      if (error != ERROR_NO_MORE_ITEMS)
	seterrno_from_win_error (__FILE__, __LINE__, error);
      goto out;
    }

  /* We get here if `buf' contains valid data.  */
  if (*buf == 0)
    strcpy (de->d_name, DEFAULT_VALUE_NAME);
  else
    strcpy (de->d_name, buf);

  dir->__d_position++;
  if (dir->__d_position & REG_ENUM_VALUES_MASK)
    dir->__d_position += 0x10000;
  res = 0;
out:
  syscall_printf ("%d = readdir (%p, %p)", res, dir, de);
  return res;
}

_off64_t
fhandler_registry::telldir (DIR * dir)
{
  return dir->__d_position & REG_POSITION_MASK;
}

void
fhandler_registry::seekdir (DIR * dir, _off64_t loc)
{
  /* Unfortunately cannot simply set __d_position due to transition from sub-keys to
   * values.
   */
  rewinddir (dir);
  while (loc > (dir->__d_position & REG_POSITION_MASK))
    if (!readdir (dir, dir->__d_dirent))
      break;
}

void
fhandler_registry::rewinddir (DIR * dir)
{
  if (dir->__handle != INVALID_HANDLE_VALUE)
    {
      RegCloseKey ((HKEY) dir->__handle);
      dir->__handle = INVALID_HANDLE_VALUE;
    }
  dir->__d_position = 0;
  dir->__flags = dirent_saw_dot | dirent_saw_dot_dot;
}

int
fhandler_registry::closedir (DIR * dir)
{
  int res = 0;
  if (dir->__handle != INVALID_HANDLE_VALUE &&
      RegCloseKey ((HKEY) dir->__handle) != ERROR_SUCCESS)
    {
      __seterrno ();
      res = -1;
    }
  syscall_printf ("%d = closedir (%p)", res, dir);
  return 0;
}

int
fhandler_registry::open (int flags, mode_t mode)
{
  int pathlen;
  const char *file;
  HKEY handle;

  int res = fhandler_virtual::open (flags, mode);
  if (!res)
    goto out;

  const char *path;
  path = get_name () + proc_len + 1 + prefix_len;
  if (!*path)
    {
      if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
	{
	  set_errno (EEXIST);
	  res = 0;
	  goto out;
	}
      else if (flags & O_WRONLY)
	{
	  set_errno (EISDIR);
	  res = 0;
	  goto out;
	}
      else
	{
	  flags |= O_DIROPEN;
	  goto success;
	}
    }
  path++;
  pathlen = strlen (path);
  file = path + pathlen - 1;
  if (isdirsep (*file) && pathlen > 1)
    file--;
  while (!isdirsep (*file))
    file--;
  file++;

  if (file == path)
    {
      for (int i = 0; registry_listing[i]; i++)
	if (path_prefix_p
	    (registry_listing[i], path, strlen (registry_listing[i])))
	  {
	    if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
	      {
		set_errno (EEXIST);
		res = 0;
		goto out;
	      }
	    else if (flags & O_WRONLY)
	      {
		set_errno (EISDIR);
		res = 0;
		goto out;
	      }
	    else
	      {
		set_io_handle (registry_keys[i]);
		flags |= O_DIROPEN;
		goto success;
	      }
	  }

      if (flags & O_CREAT)
	{
	  set_errno (EROFS);
	  res = 0;
	  goto out;
	}
      else
	{
	  set_errno (ENOENT);
	  res = 0;
	  goto out;
	}
    }

  if (flags & O_WRONLY)
    {
      set_errno (EROFS);
      res = 0;
      goto out;
    }

  handle = open_key (path, KEY_READ, wow64, false);
  if (handle == (HKEY) INVALID_HANDLE_VALUE)
    {
      handle = open_key (path, KEY_READ, wow64, true);
      if (handle == (HKEY) INVALID_HANDLE_VALUE)
	{
	  res = 0;
	  goto out;
	}
    }
  else
    flags |= O_DIROPEN;

  set_io_handle (handle);

  if (pathmatch (file, DEFAULT_VALUE_NAME))
    value_name = cstrdup ("");
  else
    value_name = cstrdup (file);

  if (!(flags & O_DIROPEN) && !fill_filebuf ())
    {
      RegCloseKey (handle);
      res = 0;
      goto out;
    }

  if (flags & O_APPEND)
    position = filesize;
  else
    position = 0;

success:
  res = 1;
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  set_open_status ();
out:
  syscall_printf ("%d = fhandler_registry::open (%p, %d)", res, flags, mode);
  return res;
}

int
fhandler_registry::close ()
{
  int res = fhandler_virtual::close ();
  if (res != 0)
    return res;
  HKEY handle = (HKEY) get_handle ();
  if (handle != (HKEY) INVALID_HANDLE_VALUE)
    {
      if (RegCloseKey (handle) != ERROR_SUCCESS)
	{
	  __seterrno ();
	  res = -1;
	}
    }
  if (!hExeced && value_name)
    {
      cfree (value_name);
      value_name = NULL;
    }
  return res;
}

bool
fhandler_registry::fill_filebuf ()
{
  DWORD type, size;
  LONG error;
  HKEY handle = (HKEY) get_handle ();
  if (handle != HKEY_PERFORMANCE_DATA)
    {
      error = RegQueryValueEx (handle, value_name, NULL, &type, NULL, &size);
      if (error != ERROR_SUCCESS)
	{
	  if (error != ERROR_FILE_NOT_FOUND)
	    {
	      seterrno_from_win_error (__FILE__, __LINE__, error);
	      return false;
	    }
	  goto value_not_found;
	}
      bufalloc = size;
      filebuf = (char *) cmalloc_abort (HEAP_BUF, bufalloc);
      error =
	RegQueryValueEx (handle, value_name, NULL, NULL, (BYTE *) filebuf,
			 &size);
      if (error != ERROR_SUCCESS)
	{
	  seterrno_from_win_error (__FILE__, __LINE__, error);
	  return true;
	}
      filesize = size;
    }
  else
    {
      bufalloc = 0;
      do
	{
	  bufalloc += 1000;
	  filebuf = (char *) crealloc_abort (filebuf, bufalloc);
	  size = bufalloc;
	  error = RegQueryValueEx (handle, value_name, NULL, &type,
				   (BYTE *) filebuf, &size);
	  if (error != ERROR_SUCCESS && error != ERROR_MORE_DATA)
	    {
	      if (error != ERROR_FILE_NOT_FOUND)
		{
		  seterrno_from_win_error (__FILE__, __LINE__, error);
		  return true;
		}
	      goto value_not_found;
	    }
	}
      while (error == ERROR_MORE_DATA);
      filesize = size;
    }
  return true;
value_not_found:
  DWORD buf_size = CYG_MAX_PATH;
  char buf[buf_size];
  int index = 0;
  while (ERROR_SUCCESS ==
	 (error = RegEnumKeyEx (handle, index++, buf, &buf_size, NULL, NULL,
				NULL, NULL)) || (error == ERROR_MORE_DATA))
    {
      if (pathmatch (buf, value_name))
	{
	  set_errno (EISDIR);
	  return false;
	}
      buf_size = CYG_MAX_PATH;
    }
  if (error != ERROR_NO_MORE_ITEMS)
    {
      seterrno_from_win_error (__FILE__, __LINE__, error);
      return false;
    }
  set_errno (ENOENT);
  return false;
}

/* Auxillary member function to open registry keys.  */
static HKEY
open_key (const char *name, REGSAM access, DWORD wow64, bool isValue)
{
  HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;
  HKEY hParentKey = (HKEY) INVALID_HANDLE_VALUE;
  bool parentOpened = false;
  char component[CYG_MAX_PATH];

  while (*name)
    {
      const char *anchor = name;
      while (*name && !isdirsep (*name))
	name++;
      strncpy (component, anchor, name - anchor);
      component[name - anchor] = '\0';
      if (*name)
	name++;
      if (*name == 0 && isValue == true)
	goto out;

      if (hParentKey != (HKEY) INVALID_HANDLE_VALUE)
	{
	  REGSAM effective_access = KEY_READ;
	  if ((strchr (name, '/') == NULL && isValue == true) || *name == 0)
	    effective_access = access;
	  LONG error = RegOpenKeyEx (hParentKey, component, 0,
				     effective_access | wow64, &hKey);
	  if (error == ERROR_ACCESS_DENIED) /* Try opening with backup intent */
	    error = RegCreateKeyEx (hParentKey, component, 0, NULL,
				    REG_OPTION_BACKUP_RESTORE,
				    effective_access | wow64, NULL,
				    &hKey, NULL);
	  if (error != ERROR_SUCCESS)
	    {
	      hKey = (HKEY) INVALID_HANDLE_VALUE;
	      seterrno_from_win_error (__FILE__, __LINE__, error);
	      return hKey;
	    }
	  if (parentOpened)
	    RegCloseKey (hParentKey);
	  hParentKey = hKey;
	  parentOpened = true;
	}
      else
	{
	  for (int i = 0; registry_listing[i]; i++)
	    if (pathmatch (component, registry_listing[i]))
	      hKey = registry_keys[i];
	  if (hKey == (HKEY) INVALID_HANDLE_VALUE)
	    return hKey;
	  hParentKey = hKey;
	}
    }
out:
  return hKey;
}
