/* fhandler_registry.cc: fhandler for /proc/registry virtual filesystem

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* FIXME: Access permissions are ignored at the moment.  */

#include "winsup.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include <assert.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static const int registry_len = sizeof ("registry") - 1;
/* If this bit is set in __d_position then we are enumerating values,
 * else sub-keys. keeping track of where we are is horribly messy
 * the bottom 16 bits are the absolute position and the top 15 bits
 * make up the value index if we are enuerating values.
 */
static const __off32_t REG_ENUM_VALUES_MASK = 0x8000000;
static const __off32_t REG_POSITION_MASK = 0xffff;

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
  "HKEY_DYN_DATA",		// 95/98/Me
  "HKEY_PERFOMANCE_DATA",	// NT/2000/XP
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
  HKEY_DYN_DATA,
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

static HKEY open_key (const char *name, REGSAM access, bool isValue);

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
  DWORD buf_size = MAX_PATH;
  LONG error;
  char buf[buf_size];
  const char *file;
  HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;

  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + registry_len + 2;
  if (*path == 0)
    {
      file_type = 2;
      goto out;
    }
  pathlen = strlen (path);
  file = path + pathlen - 1;
  if (SLASH_P (*file) && pathlen > 1)
    file--;
  while (!SLASH_P (*file))
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

  hKey = open_key (path, KEY_READ, false);
  if (hKey != (HKEY) INVALID_HANDLE_VALUE)
    file_type = 1;
  else
    {
      hKey = open_key (path, KEY_READ, true);
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
	  buf_size = MAX_PATH;
	}
      if (error != ERROR_NO_MORE_ITEMS)
	{
	  seterrno_from_win_error (__FILE__, __LINE__, error);
	  goto out;
	}
      index = 0;
      buf_size = MAX_PATH;
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
	  buf_size = MAX_PATH;
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

fhandler_registry::fhandler_registry ():
fhandler_proc (FH_REGISTRY)
{
}

int
fhandler_registry::fstat (struct __stat64 *buf, path_conv *pc)
{
  this->fhandler_base::fstat (buf, pc);
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
      const char *path = get_name () + proc_len + registry_len + 2;
      hKey =
	open_key (path, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE,
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
	      buf->st_ctim = buf->st_mtim;
	      time_as_timestruc_t (&buf->st_atim);
	      if (file_type > 0)
		buf->st_nlink = subkey_count;
	      else
		{
		  int pathlen = strlen (path);
		  const char *value_name = path + pathlen - 1;
		  if (SLASH_P (*value_name) && pathlen > 1)
		    value_name--;
		  while (!SLASH_P (*value_name))
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
	      if (get_object_attribute
		  ((HANDLE) hKey, SE_REGISTRY_KEY, &buf->st_mode, &uid,
		   &gid) == 0)
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
    }
  return 0;
}

struct dirent *
fhandler_registry::readdir (DIR * dir)
{
  DWORD buf_size = MAX_PATH;
  char buf[buf_size];
  HANDLE handle;
  struct dirent *res = NULL;
  const char *path = dir->__d_dirname + proc_len + 1 + registry_len;
  LONG error;

  if (*path == 0)
    {
      if (dir->__d_position >= ROOT_KEY_COUNT)
	goto out;
      strcpy (dir->__d_dirent->d_name, registry_listing[dir->__d_position++]);
      res = dir->__d_dirent;
      goto out;
    }
  if (dir->__d_u.__d_data.__handle == INVALID_HANDLE_VALUE
      && dir->__d_position == 0)
    {
      handle = open_key (path + 1, KEY_READ, false);
      dir->__d_u.__d_data.__handle = handle;
    }
  if (dir->__d_u.__d_data.__handle == INVALID_HANDLE_VALUE)
    goto out;
  if (dir->__d_position < SPECIAL_DOT_FILE_COUNT)
    {
      strcpy (dir->__d_dirent->d_name,
	      special_dot_files[dir->__d_position++]);
      res = dir->__d_dirent;
      goto out;
    }
retry:
  if (dir->__d_position & REG_ENUM_VALUES_MASK)
    /* For the moment, the type of key is ignored here. when write access is added,
     * maybe add an extension for the type of each value?
     */
    error = RegEnumValue ((HKEY) dir->__d_u.__d_data.__handle,
			  (dir->__d_position & ~REG_ENUM_VALUES_MASK) >> 16,
			  buf, &buf_size, NULL, NULL, NULL, NULL);
  else
    error =
      RegEnumKeyEx ((HKEY) dir->__d_u.__d_data.__handle, dir->__d_position -
		    SPECIAL_DOT_FILE_COUNT, buf, &buf_size, NULL, NULL, NULL,
		    NULL);
  if (error == ERROR_NO_MORE_ITEMS
      && (dir->__d_position & REG_ENUM_VALUES_MASK) == 0)
    {
      /* If we're finished with sub-keys, start on values under this key.  */
      dir->__d_position |= REG_ENUM_VALUES_MASK;
      buf_size = MAX_PATH;
      goto retry;
    }
  if (error != ERROR_SUCCESS && error != ERROR_MORE_DATA)
    {
      RegCloseKey ((HKEY) dir->__d_u.__d_data.__handle);
      dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
      seterrno_from_win_error (__FILE__, __LINE__, error);
      goto out;
    }

  /* We get here if `buf' contains valid data.  */
  if (*buf == 0)
    strcpy (dir->__d_dirent->d_name, DEFAULT_VALUE_NAME);
  else
    strcpy (dir->__d_dirent->d_name, buf);

  dir->__d_position++;
  if (dir->__d_position & REG_ENUM_VALUES_MASK)
    dir->__d_position += 0x10000;
  res = dir->__d_dirent;
out:
  syscall_printf ("%p = readdir (%p)", &dir->__d_dirent, dir);
  return res;
}

__off64_t
fhandler_registry::telldir (DIR * dir)
{
  return dir->__d_position & REG_POSITION_MASK;
}

void
fhandler_registry::seekdir (DIR * dir, __off64_t loc)
{
  /* Unfortunately cannot simply set __d_position due to transition from sub-keys to
   * values.
   */
  rewinddir (dir);
  while (loc > (dir->__d_position & REG_POSITION_MASK))
    if (!readdir (dir))
      break;
}

void
fhandler_registry::rewinddir (DIR * dir)
{
  if (dir->__d_u.__d_data.__handle != INVALID_HANDLE_VALUE)
    {
      (void) RegCloseKey ((HKEY) dir->__d_u.__d_data.__handle);
      dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
    }
  dir->__d_position = 0;
  return;
}

int
fhandler_registry::closedir (DIR * dir)
{
  int res = 0;
  if (dir->__d_u.__d_data.__handle != INVALID_HANDLE_VALUE &&
      RegCloseKey ((HKEY) dir->__d_u.__d_data.__handle) != ERROR_SUCCESS)
    {
      __seterrno ();
      res = -1;
    }
  syscall_printf ("%d = closedir (%p)", res, dir);
  return 0;
}

int
fhandler_registry::open (path_conv * pc, int flags, mode_t mode)
{
  int pathlen;
  const char *file;
  HKEY handle;

  int res = fhandler_virtual::open (pc, flags, mode);
  if (!res)
    goto out;

  const char *path;
  path = get_name () + proc_len + 1 + registry_len;
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
  if (SLASH_P (*file) && pathlen > 1)
    file--;
  while (!SLASH_P (*file))
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

  handle = open_key (path, KEY_READ, true);
  if (handle == (HKEY) INVALID_HANDLE_VALUE)
    {
      res = 0;
      goto out;
    }

  set_io_handle (handle);

  if (pathmatch (file, DEFAULT_VALUE_NAME))
    value_name = cstrdup ("");
  else
    value_name = cstrdup (file);

  if (!fill_filebuf ())
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
  if (value_name)
    cfree (value_name);
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
      filebuf = (char *) malloc (bufalloc);
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
	  filebuf = (char *) realloc (filebuf, bufalloc);
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
  DWORD buf_size = MAX_PATH;
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
      buf_size = MAX_PATH;
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
open_key (const char *name, REGSAM access, bool isValue)
{
  HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;
  HKEY hParentKey = (HKEY) INVALID_HANDLE_VALUE;
  bool parentOpened = false;
  char component[MAX_PATH];

  while (*name)
    {
      const char *anchor = name;
      while (*name && !SLASH_P (*name))
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
	  LONG
	    error =
	    RegOpenKeyEx (hParentKey, component, 0, effective_access, &hKey);
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
