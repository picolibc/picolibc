/* fhandler_registry.cc: fhandler for /proc/registry virtual filesystem

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* FIXME: Access permissions are ignored at the moment.  */

#include "winsup.h"
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"

#define _LIBC
#include <dirent.h>

/* If this bit is set in __d_position then we are enumerating values,
 * else sub-keys. keeping track of where we are is horribly messy
 * the bottom 16 bits are the absolute position and the top 15 bits
 * make up the value index if we are enuerating values.
 */
static const __int32_t REG_ENUM_VALUES_MASK = 0x8000000;
static const __int32_t REG_POSITION_MASK = 0xffff;

/* These key paths are used below whenever we return key information.
   The problem is UAC virtualization when running an admin account with
   restricted rights.  In that case the subkey "Classes" in the VirtualStore
   points to the HKEY_CLASSES_ROOT key again.  If "Classes" is handled as a
   normal subdirectory, applications recursing throught the directory
   hirarchy will invariably run into an infinite recursion.  What we do here
   is to handle the "Classes" subkey as a symlink to HKEY_CLASSES_ROOT.  This
   avoids the infinite recursion, unless the application blindly follows
   symlinks pointing to directories, in which case it's their own fault. */
#define VIRT_CLASSES_KEY_PREFIX "/VirtualStore/MACHINE/SOFTWARE"
#define VIRT_CLASSES_KEY_SUFFIX "Classes"
#define VIRT_CLASSES_KEY VIRT_CLASSES_KEY_PREFIX "/" VIRT_CLASSES_KEY_SUFFIX
#define VIRT_CLASSES_LINKTGT "/proc/registry/HKEY_CLASSES_ROOT"

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
  "HKEY_PERFORMANCE_DATA",
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

/* Make sure to access the correct per-user HKCR and HKCU hives, even if
   the current user is only impersonated in another user's session. */
static HKEY
fetch_hkey (int idx) /* idx *must* be valid */
{
  HKEY key;

  if (registry_keys[idx] == HKEY_CLASSES_ROOT)
    {
      if (RegOpenUserClassesRoot (cygheap->user.issetuid ()
				  ? cygheap->user.imp_token () : hProcToken,
				  0, KEY_READ, &key) == ERROR_SUCCESS)
	return key;
    }
  else if (registry_keys[idx] == HKEY_CURRENT_USER)
    {
      if (RegOpenCurrentUser (KEY_READ, &key) == ERROR_SUCCESS)
	return key;
    }
  else if (registry_keys[idx] == HKEY_CURRENT_CONFIG)
    {
      if (RegOpenKeyExW (HKEY_LOCAL_MACHINE,
		       L"System\\CurrentControlSet\\Hardware Profiles\\Current",
		       0, KEY_READ, &key) == ERROR_SUCCESS)
	return key;
    }
  /* Unfortunately there's no way to generate a valid OS registry key for
     the other root keys.  HKEY_USERS and HKEY_LOCAL_MACHINE are file
     handles internally, HKEY_PERFORMANCE_DATA is just a bad hack and
     no registry key at all. */
  return registry_keys[idx];
}

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

/* Value names for HKEY_PERFORMANCE_DATA.
 *
 * CAUTION: Never call RegQueryValueEx (HKEY_PERFORMANCE_DATA, "Add", ...).
 * It WRITES data and may destroy the perfc009.dat file.  Same applies to
 * name prefixes "Ad" and "A".
 */
static const char * const perf_data_files[] =
{
  "@",
  "Costly",
  "Global"
};

static const int PERF_DATA_FILE_COUNT =
  sizeof (perf_data_files) / sizeof (perf_data_files[0]);

static HKEY open_key (const char *name, REGSAM access, DWORD wow64, bool isValue);

/* Return true if char must be encoded.
 */
static inline bool
must_encode (wchar_t c)
{
  return (iswdirsep (c) || c == L':' || c == L'%');
}

/* Encode special chars in registry key or value name.
 * Returns 0: success, -1: error.
 */
static int
encode_regname (char *dst, const wchar_t *src, bool add_val)
{
  int di = 0;
  if (!src[0])
    dst[di++] = '@'; // Default value.
  else
    for (int si = 0; src[si]; si++)
      {
	wchar_t c = src[si];
	if (must_encode (c) ||
	    (si == 0 && ((c == L'.'
			  && (!src[1] || (src[1] == L'.' && !src[2])))
			 || (c == L'@' && !src[1]))))
	  {
	    if (di + 3 >= NAME_MAX + 1)
	      return -1;
	    __small_sprintf (dst + di, "%%%02x", c);
	    di += 3;
	  }
	else
	  di += sys_wcstombs (dst + di, NAME_MAX + 1 - di, &c, 1);
      }

  if (add_val)
    {
      if (di + 4 >= NAME_MAX + 1)
	return -1;
      memcpy (dst + di, "%val", 4);
      di += 4;
    }

  dst[di] = 0;
  return 0;
}

/* Decode special chars in registry key or value name.
 * Returns 0: success, 1: "%val" detected, -1: error.
 */
static int
decode_regname (wchar_t *wdst, const char *src, int len = -1)
{
  if (len < 0)
    len = strlen (src);
  char dst[len + 1];
  int res = 0;

  if (len > 4 && !memcmp (src + len - 4, "%val", 4))
    {
      len -= 4;
      res = 1;
    }

  int di = 0;
  if (len == 1 && src[0] == '@')
    ; // Default value.
  else
    for (int si = 0; si < len; si++)
      {
	char c = src[si];
	if (c == '%')
	  {
	    if (si + 2 >= len)
	      return -1;
	    char s[] = {src[si+1], src[si+2], '\0'};
	    char *p;
	    c = strtoul (s, &p, 16);
	    if (!(must_encode ((wchar_t) c) ||
		  (si == 0 && ((c == '.' && (len == 3 || (src[3] == '.' && len == 4))) ||
			       (c == '@' && len == 3)))))
	      return -1;
	    dst[di++] = c;
	    si += 2;
	  }
	else
	  dst[di++] = c;
      }

  dst[di] = 0;
  sys_mbstowcs (wdst, NAME_MAX + 1, dst);
  return res;
}


/* Hash table to limit calls to key_exists ().
 */
class __DIR_hash
{
public:
  __DIR_hash ()
    {
      memset (table, 0, sizeof(table));
    }

  void set (unsigned h)
    {
      table [(h >> 3) & (HASH_SIZE - 1)] |= (1 << (h & 0x3));
    }

  bool is_set (unsigned h) const
    {
      return (table [(h >> 3) & (HASH_SIZE - 1)] & (1 << (h & 0x3))) != 0;
    }

private:
  enum { HASH_SIZE = 1024 };
  unsigned char table[HASH_SIZE];
};

#define d_hash(d)	((__DIR_hash *) (d)->__d_internal)


/* Return true if subkey NAME exists in key PARENT.
 */
static bool
key_exists (HKEY parent, const wchar_t *name, DWORD wow64)
{
  HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;
  LONG error = RegOpenKeyExW (parent, name, 0, KEY_READ | wow64, &hKey);
  if (error == ERROR_SUCCESS)
    RegCloseKey (hKey);

  return (error == ERROR_SUCCESS || error == ERROR_ACCESS_DENIED);
}

static size_t
multi_wcstombs (char *dst, size_t len, const wchar_t *src, size_t nwc)
{
  size_t siz, sum = 0;
  const wchar_t *nsrc;

  while (nwc)
    {
      siz = sys_wcstombs (dst, len, src, nwc) + 1;
      sum += siz;
      if (dst)
	{
	  dst += siz;
	  len -= siz;
	}
      nsrc = wcschr (src, L'\0') + 1;
      if ((size_t) (nsrc - src) >= nwc)
	break;
      nwc -= nsrc - src;
      src = nsrc;
      if (*src == L'\0')
	{
	  if (dst)
	    *dst++ = '\0';
	  ++sum;
	  break;
	}
    }
  return sum;
}

virtual_ftype_t
fhandler_registry::exists ()
{
  virtual_ftype_t file_type = virt_none;
  int index = 0, pathlen;
  DWORD buf_size = NAME_MAX + 1;
  LONG error;
  wchar_t buf[buf_size];
  const char *file;
  HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;

  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + prefix_len + 1;
  if (*path)
    path++;
  else
    {
      file_type = virt_rootdir;
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
	if (path_prefix_p (registry_listing[i], path,
			   strlen (registry_listing[i]), true))
	  {
	    file_type = virt_directory;
	    break;
	  }
    }
  else
    {
      wchar_t dec_file[NAME_MAX + 1];

      int val_only = decode_regname (dec_file, file);
      if (val_only < 0)
	goto out;

      if (!val_only)
	hKey = open_key (path, KEY_READ, wow64, false);
      if (hKey != (HKEY) INVALID_HANDLE_VALUE)
	{
	  if (!strcasecmp (path + strlen (path)
			   - sizeof (VIRT_CLASSES_KEY) + 1,
			   VIRT_CLASSES_KEY))
	    file_type = virt_symlink;
	  else
	    file_type = virt_directory;
	}
      else
	{
	  /* Key does not exist or open failed with EACCES,
	     enumerate subkey and value names of parent key.  */
	  hKey = open_key (path, KEY_READ, wow64, true);
	  if (hKey == (HKEY) INVALID_HANDLE_VALUE)
	    return virt_none;

	  if (hKey == HKEY_PERFORMANCE_DATA)
	    {
	      /* RegEnumValue () returns garbage for this key.
		 RegQueryValueEx () returns a PERF_DATA_BLOCK even
		 if a value does not contain any counter objects.
		 So allow access to the generic names and to
		 (blank separated) lists of counter numbers.
		 Never allow access to "Add", see above comment.  */
	      for (int i = 0; i < PERF_DATA_FILE_COUNT
			      && file_type == virt_none; i++)
		{
		  if (strcasematch (perf_data_files[i], file))
		    file_type = virt_file;
		}
	      if (file_type == virt_none && !file[strspn (file, " 0123456789")])
		file_type = virt_file;
	      goto out;
	    }

	  if (!val_only && dec_file[0])
	    {
	      while (ERROR_SUCCESS ==
		     (error = RegEnumKeyExW (hKey, index++, buf, &buf_size,
					     NULL, NULL, NULL, NULL))
		     || (error == ERROR_MORE_DATA))
		{
		  if (!wcscasecmp (buf, dec_file))
		    {
		      file_type = virt_directory;
		      goto out;
		    }
		    buf_size = NAME_MAX + 1;
		}
	      if (error != ERROR_NO_MORE_ITEMS)
		{
		  seterrno_from_win_error (__FILE__, __LINE__, error);
		  goto out;
		}
	      index = 0;
	      buf_size = NAME_MAX + 1;
	    }

	  while (ERROR_SUCCESS ==
		 (error = RegEnumValueW (hKey, index++, buf, &buf_size,
					 NULL, NULL, NULL, NULL))
		 || (error == ERROR_MORE_DATA))
	    {
	      if (!wcscasecmp (buf, dec_file))
		{
		  file_type = virt_file;
		  goto out;
		}
	      buf_size = NAME_MAX + 1;
	    }
	  if (error != ERROR_NO_MORE_ITEMS)
	    {
	      seterrno_from_win_error (__FILE__, __LINE__, error);
	      goto out;
	    }
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
  if (strncasematch (in_pc.get_posix (), "/proc/registry32", 16))
    {
      wow64 = KEY_WOW64_32KEY;
      prefix_len += 2;
    }
  else if (strncasematch (in_pc.get_posix (), "/proc/registry64", 16))
    prefix_len += 2;
  fhandler_base::set_name (in_pc);
}

fhandler_registry::fhandler_registry ():
fhandler_proc ()
{
  wow64 = 0;
  prefix_len = sizeof ("registry") - 1;
}

int
fhandler_registry::fstat (struct stat *buf)
{
  fhandler_base::fstat (buf);
  buf->st_mode &= ~_IFMT & NO_W;
  virtual_ftype_t file_type = exists ();
  switch (file_type)
    {
    case virt_none:
      set_errno (ENOENT);
      return -1;
    case virt_symlink:
      buf->st_mode |= S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;
      break;
    case virt_directory:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      break;
    case virt_rootdir:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      buf->st_nlink = ROOT_KEY_COUNT;
      break;
    default:
    case virt_file:
      buf->st_mode |= S_IFREG;
      buf->st_mode &= NO_X;
      break;
    }
  if (file_type != virt_none && file_type != virt_rootdir)
    {
      HKEY hKey;
      const char *path = get_name () + proc_len + prefix_len + 2;
      hKey =
	open_key (path, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE, wow64,
		  virt_ftype_isfile (file_type) ? true : false);

      if (hKey == HKEY_PERFORMANCE_DATA)
	/* RegQueryInfoKey () always returns write time 0,
	   RegQueryValueEx () does not return required buffer size.  */
	;
      else if (hKey != (HKEY) INVALID_HANDLE_VALUE)
	{
	  FILETIME ftLastWriteTime;
	  DWORD subkey_count;
	  if (ERROR_SUCCESS ==
	      RegQueryInfoKeyW (hKey, NULL, NULL, NULL, &subkey_count, NULL,
				NULL, NULL, NULL, NULL, NULL, &ftLastWriteTime))
	    {
	      to_timestruc_t ((PLARGE_INTEGER) &ftLastWriteTime, &buf->st_mtim);
	      buf->st_ctim = buf->st_birthtim = buf->st_mtim;
	      time_as_timestruc_t (&buf->st_atim);
	      if (virt_ftype_isdir (file_type))
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
		  wchar_t dec_value_name[NAME_MAX + 1];
		  DWORD dwSize = 0;
		  DWORD type;
		  if (decode_regname (dec_value_name, value_name) >= 0
		      && RegQueryValueExW (hKey, dec_value_name, NULL, &type,
					   NULL, &dwSize) == ERROR_SUCCESS
		      && (type == REG_SZ || type == REG_EXPAND_SZ
			  || type == REG_MULTI_SZ || type == REG_LINK))
		    {
		      PBYTE tmpbuf = (PBYTE) malloc (dwSize);
		      if (!tmpbuf
			  || RegQueryValueExW (hKey, dec_value_name,
					       NULL, NULL, tmpbuf, &dwSize)
			     != ERROR_SUCCESS)
			buf->st_size = dwSize / sizeof (wchar_t);
		      else if (type == REG_MULTI_SZ)
			buf->st_size = multi_wcstombs (NULL, 0,
						     (wchar_t *) tmpbuf,
						     dwSize / sizeof (wchar_t));
		      else
			buf->st_size = sys_wcstombs (NULL, 0,
						     (wchar_t *) tmpbuf,
						     dwSize / sizeof (wchar_t))
				       + 1;
		      if (tmpbuf)
			free (tmpbuf);
		    }
		  else
		    buf->st_size = dwSize;
		}
	      uid_t uid;
	      gid_t gid;
	      if (get_reg_attribute (hKey, buf->st_mode, &uid, &gid) == 0)
		{
		  buf->st_uid = uid;
		  buf->st_gid = gid;
		  buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
		  if (virt_ftype_isdir (file_type))
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
	  buf->st_uid = ILLEGAL_UID;
	  buf->st_gid = ILLEGAL_GID;
	  buf->st_mode &= ~0777;
	}
    }
  return 0;
}

DIR *
fhandler_registry::opendir (int fd)
{
  /* Skip fhandler_proc::opendir, which allocates dir->_d_handle for its
     own devilish purposes... */
  return fhandler_virtual::opendir (fd);
}

int
fhandler_registry::readdir (DIR *dir, dirent *de)
{
  DWORD buf_size = NAME_MAX + 1;
  wchar_t buf[buf_size];
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
  if (dir->__handle == INVALID_HANDLE_VALUE)
    {
      if (dir->__d_position != 0)
	goto out;
      dir->__handle = open_key (path + 1, KEY_READ, wow64, false);
      if (dir->__handle == INVALID_HANDLE_VALUE)
	goto out;
      dir->__d_internal = (uintptr_t) new __DIR_hash ();
    }
  if (dir->__d_position < SPECIAL_DOT_FILE_COUNT)
    {
      strcpy (de->d_name, special_dot_files[dir->__d_position++]);
      res = 0;
      goto out;
    }
  if ((HKEY) dir->__handle == HKEY_PERFORMANCE_DATA)
    {
      /* RegEnumValue () returns garbage for this key,
	 simulate only a minimal listing of the generic names.  */
      if (dir->__d_position >= SPECIAL_DOT_FILE_COUNT + PERF_DATA_FILE_COUNT)
	goto out;
      strcpy (de->d_name, perf_data_files[dir->__d_position - SPECIAL_DOT_FILE_COUNT]);
      dir->__d_position++;
      res = 0;
      goto out;
    }

retry:
  if (dir->__d_position & REG_ENUM_VALUES_MASK)
    /* For the moment, the type of key is ignored here. when write access is added,
     * maybe add an extension for the type of each value?
     */
    error = RegEnumValueW ((HKEY) dir->__handle,
			   (dir->__d_position & ~REG_ENUM_VALUES_MASK) >> 16,
			   buf, &buf_size, NULL, NULL, NULL, NULL);
  else
    error =
      RegEnumKeyExW ((HKEY) dir->__handle, dir->__d_position -
		     SPECIAL_DOT_FILE_COUNT, buf, &buf_size,
		     NULL, NULL, NULL, NULL);
  if (error == ERROR_NO_MORE_ITEMS
      && (dir->__d_position & REG_ENUM_VALUES_MASK) == 0)
    {
      /* If we're finished with sub-keys, start on values under this key.  */
      dir->__d_position |= REG_ENUM_VALUES_MASK;
      buf_size = NAME_MAX + 1;
      goto retry;
    }
  if (error != ERROR_SUCCESS && error != ERROR_MORE_DATA)
    {
      delete d_hash (dir);
      RegCloseKey ((HKEY) dir->__handle);
      dir->__handle = INVALID_HANDLE_VALUE;
      if (error != ERROR_NO_MORE_ITEMS)
	seterrno_from_win_error (__FILE__, __LINE__, error);
      goto out;
    }

  /* We get here if `buf' contains valid data.  */
  dir->__d_position++;
  if (dir->__d_position & REG_ENUM_VALUES_MASK)
    dir->__d_position += 0x10000;

  {
    /* Append "%val" if value name is identical to a previous key name.  */
    unsigned h = hash_path_name (1, buf);
    bool add_val = false;
    if (! (dir->__d_position & REG_ENUM_VALUES_MASK))
      d_hash (dir)->set (h);
    else if (d_hash (dir)->is_set (h)
	     && key_exists ((HKEY) dir->__handle, buf, wow64))
      add_val = true;

    if (encode_regname (de->d_name, buf, add_val))
      {
	buf_size = NAME_MAX + 1;
	goto retry;
      }
  }

  if (dir->__d_position & REG_ENUM_VALUES_MASK)
    de->d_type = DT_REG;
  else if (!strcasecmp (de->d_name, "Classes")
	   && !strcasecmp (path + strlen (path)
			   - sizeof (VIRT_CLASSES_KEY_PREFIX) + 1,
			   VIRT_CLASSES_KEY_PREFIX))
    de->d_type = DT_LNK;
  else
    de->d_type = DT_DIR;

  res = 0;
out:
  syscall_printf ("%d = readdir(%p, %p)", res, dir, de);
  return res;
}

long
fhandler_registry::telldir (DIR * dir)
{
  return dir->__d_position & REG_POSITION_MASK;
}

void
fhandler_registry::seekdir (DIR * dir, long loc)
{
  /* Unfortunately cannot simply set __d_position due to transition from sub-keys to
   * values.
   */
  rewinddir (dir);
  while (loc > (dir->__d_position & REG_POSITION_MASK))
    if (readdir (dir, dir->__d_dirent))
      break;
}

void
fhandler_registry::rewinddir (DIR * dir)
{
  if (dir->__handle != INVALID_HANDLE_VALUE)
    {
      delete d_hash (dir);
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
  if (dir->__handle != INVALID_HANDLE_VALUE)
    {
      delete d_hash (dir);
      if (RegCloseKey ((HKEY) dir->__handle) != ERROR_SUCCESS)
	{
	  __seterrno ();
	  res = -1;
	}
    }
  syscall_printf ("%d = closedir(%p)", res, dir);
  return 0;
}

int
fhandler_registry::open (int flags, mode_t mode)
{
  int pathlen;
  const char *file;
  HKEY handle = (HKEY) INVALID_HANDLE_VALUE;

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
	  diropen = true;
	  /* Marking as nohandle allows to call dup. */
	  nohandle (true);
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
	if (path_prefix_p (registry_listing[i], path,
			   strlen (registry_listing[i]), true))
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
		set_handle (fetch_hkey (i));
		/* Marking as nohandle allows to call dup on pseudo registry
		   handles. */
		if (get_handle () >= HKEY_CLASSES_ROOT)
		  nohandle (true);
		diropen = true;
		goto success;
	      }
	  }

      if (flags & O_CREAT)
	{
	  set_errno (EROFS);
	  res = 0;
	}
      else
	{
	  set_errno (ENOENT);
	  res = 0;
	}
      goto out;
    }

  if (flags & O_WRONLY)
    {
      set_errno (EROFS);
      res = 0;
      goto out;
    }
  else
    {
      wchar_t dec_file[NAME_MAX + 1];
      int val_only = decode_regname (dec_file, file);
      if (val_only < 0)
	{
	  set_errno (EINVAL);
	  res = 0;
	  goto out;
	}

      if (!val_only)
	handle = open_key (path, KEY_READ, wow64, false);
      if (handle == (HKEY) INVALID_HANDLE_VALUE)
	{
	  if (val_only || get_errno () != EACCES)
	    handle = open_key (path, KEY_READ, wow64, true);
	  if (handle == (HKEY) INVALID_HANDLE_VALUE)
	    {
	      res = 0;
	      goto out;
	    }
	}
      else
	diropen = true;

      set_handle (handle);
      set_close_on_exec (!!(flags & O_CLOEXEC));
      value_name = cwcsdup (dec_file);

      if (!diropen && !fill_filebuf ())
	{
	  RegCloseKey (handle);
	  res = 0;
	  goto out;
	}

      if (flags & O_APPEND)
	position = filesize;
      else
	position = 0;
  }

success:
  res = 1;
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  set_open_status ();
out:
  syscall_printf ("%d = fhandler_registry::open(%p, 0%o)", res, flags, mode);
  return res;
}

int
fhandler_registry::close ()
{
  int res = fhandler_virtual::close ();
  if (res != 0)
    return res;
  HKEY handle = (HKEY) get_handle ();
  if (handle != (HKEY) INVALID_HANDLE_VALUE && handle < HKEY_CLASSES_ROOT)
    {
      if (RegCloseKey (handle) != ERROR_SUCCESS)
	{
	  __seterrno ();
	  res = -1;
	}
    }
  if (!have_execed && value_name)
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
  size_t bufalloc;

  if (handle != HKEY_PERFORMANCE_DATA)
    {
      error = RegQueryValueExW (handle, value_name, NULL, &type, NULL, &size);
      if (error != ERROR_SUCCESS)
	{
	  if (error == ERROR_INVALID_HANDLE
	      && !strcasecmp (get_name () + strlen (get_name ())
			      - sizeof (VIRT_CLASSES_KEY) + 1,
			      VIRT_CLASSES_KEY))
	    {
	      filesize = sizeof (VIRT_CLASSES_LINKTGT);
	      filebuf = (char *) cmalloc_abort (HEAP_BUF, filesize);
	      strcpy (filebuf, VIRT_CLASSES_LINKTGT);
	      return true;
	    }
	  if (error != ERROR_FILE_NOT_FOUND)
	    {
	      seterrno_from_win_error (__FILE__, __LINE__, error);
	      return false;
	    }
	  goto value_not_found;
	}
      PBYTE tmpbuf = (PBYTE) cmalloc_abort (HEAP_BUF, size);
      error =
	RegQueryValueExW (handle, value_name, NULL, NULL, tmpbuf, &size);
      if (error != ERROR_SUCCESS)
	{
	  seterrno_from_win_error (__FILE__, __LINE__, error);
	  return true;
	}
      if (type == REG_SZ || type == REG_EXPAND_SZ || type == REG_LINK)
	bufalloc = sys_wcstombs (NULL, 0, (wchar_t *) tmpbuf,
				 size / sizeof (wchar_t)) + 1;
      else if (type == REG_MULTI_SZ)
	bufalloc = multi_wcstombs (NULL, 0, (wchar_t *) tmpbuf,
				   size / sizeof (wchar_t));
      else
	bufalloc = size;
      filebuf = (char *) cmalloc_abort (HEAP_BUF, bufalloc);
      if (type == REG_SZ || type == REG_EXPAND_SZ || type == REG_LINK)
	sys_wcstombs (filebuf, bufalloc, (wchar_t *) tmpbuf,
		      size / sizeof (wchar_t));
      else if (type == REG_MULTI_SZ)
	multi_wcstombs (filebuf, bufalloc, (wchar_t *) tmpbuf,
			size / sizeof (wchar_t));
      else
	memcpy (filebuf, tmpbuf, bufalloc);
      filesize = bufalloc;
    }
  else
    {
      bufalloc = 0;
      do
	{
	  bufalloc += 16 * 1024;
	  filebuf = (char *) crealloc_abort (filebuf, bufalloc);
	  size = bufalloc;
	  error = RegQueryValueExW (handle, value_name, NULL, &type,
				    (PBYTE) filebuf, &size);
	  if (error != ERROR_SUCCESS && error != ERROR_MORE_DATA)
	    {
	      seterrno_from_win_error (__FILE__, __LINE__, error);
	      return false;
	    }
	}
      while (error == ERROR_MORE_DATA);
      filesize = size;
      /* RegQueryValueEx () opens HKEY_PERFORMANCE_DATA.  */
      RegCloseKey (handle);
    }
  return true;
value_not_found:
  DWORD buf_size = NAME_MAX + 1;
  wchar_t buf[buf_size];
  int index = 0;
  while (ERROR_SUCCESS ==
	 (error = RegEnumKeyExW (handle, index++, buf, &buf_size, NULL, NULL,
				 NULL, NULL)) || (error == ERROR_MORE_DATA))
    {
      if (!wcscasecmp (buf, value_name))
	{
	  set_errno (EISDIR);
	  return false;
	}
      buf_size = NAME_MAX + 1;
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
  wchar_t component[NAME_MAX + 1];

  while (*name)
    {
      const char *anchor = name;
      while (*name && !isdirsep (*name))
	name++;
      int val_only = decode_regname (component, anchor, name - anchor);
      if (val_only < 0)
	{
	  set_errno (EINVAL);
	  if (parentOpened)
	    RegCloseKey (hParentKey);
	  hKey = (HKEY) INVALID_HANDLE_VALUE;
	  break;
	}
      if (*name)
	name++;
      if (*name == 0 && isValue == true)
	break;

      if (val_only || !component[0] || hKey == HKEY_PERFORMANCE_DATA)
	{
	  set_errno (ENOENT);
	  if (parentOpened)
	    RegCloseKey (hParentKey);
	  hKey = (HKEY) INVALID_HANDLE_VALUE;
	  break;
	}

      if (hParentKey != (HKEY) INVALID_HANDLE_VALUE)
	{
	  REGSAM effective_access = KEY_READ;
	  if ((strchr (name, '/') == NULL && isValue == true) || *name == 0)
	    effective_access = access;
	  LONG error = RegOpenKeyExW (hParentKey, component, 0,
				      effective_access | wow64, &hKey);
	  if (error == ERROR_ACCESS_DENIED) /* Try opening with backup intent */
	    error = RegCreateKeyExW (hParentKey, component, 0, NULL,
				     REG_OPTION_BACKUP_RESTORE,
				     effective_access | wow64, NULL,
				     &hKey, NULL);
	  if (parentOpened)
	    RegCloseKey (hParentKey);
	  if (error != ERROR_SUCCESS)
	    {
	      hKey = (HKEY) INVALID_HANDLE_VALUE;
	      seterrno_from_win_error (__FILE__, __LINE__, error);
	      return hKey;
	    }
	  hParentKey = hKey;
	  parentOpened = true;
	}
      else
	{
	  for (int i = 0; registry_listing[i]; i++)
	    if (strncasematch (anchor, registry_listing[i], name - anchor - 1))
	      hKey = fetch_hkey (i);
	  if (hKey == (HKEY) INVALID_HANDLE_VALUE)
	    return hKey;
	  hParentKey = hKey;
	}
    }
  return hKey;
}

int
fhandler_registry::dup (fhandler_base *child, int flags)
{
  debug_printf ("here");
  fhandler_registry *fhs = (fhandler_registry *) child;

  int ret = fhandler_virtual::dup (fhs, flags);
  /* Pseudo registry handles can't be duplicated using DuplicateHandle.
     Therefore those fhandlers are marked with the nohandle flag.  This
     allows fhandler_base::dup to succeed as usual for nohandle fhandlers.
     Here we just have to fix up by copying the pseudo handle value. */
  if ((HKEY) get_handle () >= HKEY_CLASSES_ROOT)
    fhs->set_handle (get_handle ());
  if (value_name)
    fhs->value_name = cwcsdup (value_name);
  return ret;
}
