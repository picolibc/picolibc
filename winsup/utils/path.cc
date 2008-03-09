/* path.cc

   Copyright 2001, 2002, 2003, 2005, 2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* The purpose of this file is to hide all the details about accessing
   Cygwin's mount table, shortcuts, etc.  If the format or location of
   the mount table, or the shortcut format changes, this is the file to
   change to match it. */

#define str(a) #a
#define scat(a,b) str(a##b)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "path.h"
#include "cygwin/include/cygwin/version.h"
#include "cygwin/include/sys/mount.h"
#include "cygwin/include/mntent.h"
#include "testsuite.h"

/* Used when treating / and \ as equivalent. */
#define isslash(ch) \
  ({ \
      char __c = (ch); \
      ((__c) == '/' || (__c) == '\\'); \
   })


static const GUID GUID_shortcut
			= { 0x00021401L, 0, 0, 0xc0, 0, 0, 0, 0, 0, 0, 0x46 };

enum {
  WSH_FLAG_IDLIST = 0x01,	/* Contains an ITEMIDLIST. */
  WSH_FLAG_FILE = 0x02,		/* Contains a file locator element. */
  WSH_FLAG_DESC = 0x04,		/* Contains a description. */
  WSH_FLAG_RELPATH = 0x08,	/* Contains a relative path. */
  WSH_FLAG_WD = 0x10,		/* Contains a working dir. */
  WSH_FLAG_CMDLINE = 0x20,	/* Contains command line args. */
  WSH_FLAG_ICON = 0x40		/* Contains a custom icon. */
};

struct win_shortcut_hdr
  {
    DWORD size;		/* Header size in bytes.  Must contain 0x4c. */
    GUID magic;		/* GUID of shortcut files. */
    DWORD flags;	/* Content flags.  See above. */

    /* The next fields from attr to icon_no are always set to 0 in Cygwin
       and U/Win shortcuts. */
    DWORD attr;	/* Target file attributes. */
    FILETIME ctime;	/* These filetime items are never touched by the */
    FILETIME mtime;	/* system, apparently. Values don't matter. */
    FILETIME atime;
    DWORD filesize;	/* Target filesize. */
    DWORD icon_no;	/* Icon number. */

    DWORD run;		/* Values defined in winuser.h. Use SW_NORMAL. */
    DWORD hotkey;	/* Hotkey value. Set to 0.  */
    DWORD dummy[2];	/* Future extension probably. Always 0. */
  };

static bool
cmp_shortcut_header (win_shortcut_hdr *file_header)
{
  /* A Cygwin or U/Win shortcut only contains a description and a relpath.
     Cygwin shortcuts also might contain an ITEMIDLIST. The run type is
     always set to SW_NORMAL. */
  return file_header->size == sizeof (win_shortcut_hdr)
      && !memcmp (&file_header->magic, &GUID_shortcut, sizeof GUID_shortcut)
      && (file_header->flags & ~WSH_FLAG_IDLIST)
	 == (WSH_FLAG_DESC | WSH_FLAG_RELPATH)
      && file_header->run == SW_NORMAL;
}

int
get_word (HANDLE fh, int offset)
{
  unsigned short rv;
  unsigned r;

  SetLastError(NO_ERROR);
  if (SetFilePointer (fh, offset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER
      && GetLastError () != NO_ERROR)
    return -1;

  if (!ReadFile (fh, &rv, 2, (DWORD *) &r, 0))
    return -1;

  return rv;
}

/*
 * Check the value of GetLastError() to find out whether there was an error.
 */
int
get_dword (HANDLE fh, int offset)
{
  int rv;
  unsigned r;

  SetLastError(NO_ERROR);
  if (SetFilePointer (fh, offset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER
      && GetLastError () != NO_ERROR)
    return -1;

  if (!ReadFile (fh, &rv, 4, (DWORD *) &r, 0))
    return -1;

  return rv;
}

#define EXE_MAGIC ((int)*(unsigned short *)"MZ")
#define SHORTCUT_MAGIC ((int)*(unsigned short *)"L\0")
#define SYMLINK_COOKIE "!<symlink>"
#define SYMLINK_MAGIC ((int)*(unsigned short *)SYMLINK_COOKIE)

bool
is_exe (HANDLE fh)
{
  int magic = get_word (fh, 0x0);
  return magic == EXE_MAGIC;
}

bool
is_symlink (HANDLE fh)
{
  int magic = get_word (fh, 0x0);
  if (magic != SHORTCUT_MAGIC && magic != SYMLINK_MAGIC)
    return false;
  DWORD got;
  BY_HANDLE_FILE_INFORMATION local;
  if (!GetFileInformationByHandle (fh, &local))
    return false;
  if (magic == SHORTCUT_MAGIC)
    {
      DWORD size;
      if (!local.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
	return false; /* Not a Cygwin symlink. */
      if ((size = GetFileSize (fh, NULL)) > 8192)
	return false; /* Not a Cygwin symlink. */
      char buf[size];
      SetFilePointer (fh, 0, 0, FILE_BEGIN);
      if (!ReadFile (fh, buf, size, &got, 0))
	return false;
      if (got != size || !cmp_shortcut_header ((win_shortcut_hdr *) buf))
	return false; /* Not a Cygwin symlink. */
      /* TODO: check for invalid path contents
	 (see symlink_info::check() in ../cygwin/path.cc) */
    }
  else /* magic == SYMLINK_MAGIC */
    {
      if (!local.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
	return false; /* Not a Cygwin symlink. */
      char buf[sizeof (SYMLINK_COOKIE) - 1];
      SetFilePointer (fh, 0, 0, FILE_BEGIN);
      if (!ReadFile (fh, buf, sizeof (buf), &got, 0))
	return false;
      if (got != sizeof (buf) ||
	  memcmp (buf, SYMLINK_COOKIE, sizeof (buf)) != 0)
	return false; /* Not a Cygwin symlink. */
    }
  return true;
}

/* Assumes is_symlink(fh) is true */
bool
readlink (HANDLE fh, char *path, int maxlen)
{
  int got;
  int magic = get_word (fh, 0x0);

  if (magic == SHORTCUT_MAGIC)
    {
      int offset = get_word (fh, 0x4c);
      int slen = get_word (fh, 0x4c + offset + 2);
      if (slen >= maxlen)
	{
	  SetLastError (ERROR_FILENAME_EXCED_RANGE);
	  return false;
	}
      if (SetFilePointer (fh, 0x4c + offset + 4, 0, FILE_BEGIN) ==
	  INVALID_SET_FILE_POINTER && GetLastError () != NO_ERROR)
	return false;

      if (!ReadFile (fh, path, slen, (DWORD *) &got, 0))
	return false;
      else if (got < slen)
	{
	  SetLastError (ERROR_READ_FAULT);
	  return false;
	}
      else
	path[got] = '\0';
    }
  else if (magic == SYMLINK_MAGIC)
    {
      char cookie_buf[sizeof (SYMLINK_COOKIE) - 1];

      if (SetFilePointer (fh, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER
	  && GetLastError () != NO_ERROR)
	return false;

      if (!ReadFile (fh, cookie_buf, sizeof (cookie_buf), (DWORD *) &got, 0))
	return false;
      else if (got == sizeof (cookie_buf)
	       && memcmp (cookie_buf, SYMLINK_COOKIE, sizeof (cookie_buf)) == 0)
	{
	  if (!ReadFile (fh, path, maxlen, (DWORD *) &got, 0))
	    return false;
	  else if (got >= maxlen)
	    {
	      SetLastError (ERROR_FILENAME_EXCED_RANGE);
	      path[0] = '\0';
	      return false;
	    }
	  else
	    path[got] = '\0';
	}
    }
  else
    return false;
  return true;
}

typedef struct mnt
  {
    const char *native;
    char *posix;
    unsigned flags;
    int issys;
  } mnt_t;

#ifndef TESTSUITE
static mnt_t mount_table[255];
#else
#  define TESTSUITE_MOUNT_TABLE
#  include "testsuite.h"
#  undef TESTSUITE_MOUNT_TABLE
#endif

struct mnt *root_here = NULL;

/* These functions aren't called when defined(TESTSUITE) which results
   in a compiler warning.  */
#ifndef TESTSUITE
static char *
find2 (HKEY rkey, unsigned *flags, char *what)
{
  char *retval = 0;
  DWORD retvallen = 0;
  DWORD type;
  HKEY key;

  if (RegOpenKeyEx (rkey, what, 0, KEY_READ, &key) != ERROR_SUCCESS)
    return 0;

  if (RegQueryValueEx (key, "native", 0, &type, 0, &retvallen)
      == ERROR_SUCCESS)
    {
      retval = (char *) malloc (MAX_PATH + 1);
      if (RegQueryValueEx (key, "native", 0, &type, (BYTE *) retval, &retvallen)
	  != ERROR_SUCCESS)
	{
	  free (retval);
	  retval = 0;
	}
    }

  retvallen = sizeof (flags);
  RegQueryValueEx (key, "flags", 0, &type, (BYTE *)flags, &retvallen);

  RegCloseKey (key);

  return retval;
}

static LONG
get_cygdrive0 (HKEY key, const char *what, void *val, DWORD len)
{
  LONG status = RegQueryValueEx (key, what, 0, 0, (BYTE *)val, &len);
  return status;
}

static mnt *
get_cygdrive (HKEY key, mnt *m, int issystem)
{
  if (get_cygdrive0 (key, CYGWIN_INFO_CYGDRIVE_FLAGS, &m->flags,
		     sizeof (m->flags)) != ERROR_SUCCESS) {
    free (m->posix);
    return m;
  }
  get_cygdrive0 (key, CYGWIN_INFO_CYGDRIVE_PREFIX, m->posix, MAX_PATH);
  m->native = strdup (".");
  m->issys = issystem;
  return m + 1;
}
#endif

static void
read_mounts ()
{
/* If TESTSUITE is defined, bypass this whole function as a harness
   mount table will be provided.  */
#ifndef TESTSUITE
  DWORD posix_path_size;
  int res;
  struct mnt *m = mount_table;
  DWORD disposition;
  char buf[10000];

  root_here = NULL;
  for (mnt *m1 = mount_table; m1->posix; m1++)
    {
      free (m1->posix);
      if (m1->native)
	free ((char *) m1->native);
      m1->posix = NULL;
    }

  /* Loop through subkeys */
  /* FIXME: we would like to not check MAX_MOUNTS but the heap in the
     shared area is currently statically allocated so we can't have an
     arbitrarily large number of mounts. */
  for (int issystem = 0; issystem <= 1; issystem++)
    {
      sprintf (buf, "Software\\%s\\%s\\%s",
	       CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
	       CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
	       CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME);

      HKEY key = issystem ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
      if (RegCreateKeyEx (key, buf, 0, (LPTSTR) "Cygwin", 0, KEY_READ,
			  0, &key, &disposition) != ERROR_SUCCESS)
	break;
      for (int i = 0; ;i++, m++)
	{
	  m->posix = (char *) malloc (MAX_PATH + 1);
	  posix_path_size = MAX_PATH;
	  /* FIXME: if maximum posix_path_size is 256, we're going to
	     run into problems if we ever try to store a mount point that's
	     over 256 but is under MAX_PATH. */
	  res = RegEnumKeyEx (key, i, m->posix, &posix_path_size, NULL,
			      NULL, NULL, NULL);

	  if (res == ERROR_NO_MORE_ITEMS)
	    {
	      m = get_cygdrive (key, m, issystem);
	      m->posix = NULL;
	      break;
	    }

	  if (!*m->posix)
	    goto no_go;
	  else if (res != ERROR_SUCCESS)
	    break;
	  else
	    {
	      m->native = find2 (key, &m->flags, m->posix);
	      m->issys = issystem;
	      if (!m->native)
		goto no_go;
	    }
	  continue;
	no_go:
	  free (m->posix);
	  m->posix = NULL;
	  m--;
	}
      RegCloseKey (key);
    }
#endif /* !defined(TESTSUITE) */
}

/* Return non-zero if PATH1 is a prefix of PATH2.
   Both are assumed to be of the same path style and / vs \ usage.
   Neither may be "".
   LEN1 = strlen (PATH1).  It's passed because often it's already known.

   Examples:
   /foo/ is a prefix of /foo  <-- may seem odd, but desired
   /foo is a prefix of /foo/
   / is a prefix of /foo/bar
   / is not a prefix of foo/bar
   foo/ is a prefix foo/bar
   /foo is not a prefix of /foobar
*/

static int
path_prefix_p (const char *path1, const char *path2, int len1)
{
  /* Handle case where PATH1 has trailing '/' and when it doesn't.  */
  if (len1 > 0 && isslash (path1[len1 - 1]))
    len1--;

  if (len1 == 0)
    return isslash (path2[0]) && !isslash (path2[1]);

  if (strncasecmp (path1, path2, len1) != 0)
    return 0;

  return isslash (path2[len1]) || path2[len1] == 0 || path1[len1 - 1] == ':';
}

static char *
vconcat (const char *s, va_list v)
{
  int len;
  char *rv, *arg;
  va_list save_v = v;
  int unc;

  if (!s)
    return 0;

  len = strlen (s);

  unc = isslash (*s) && isslash (s[1]);

  while (1)
    {
      arg = va_arg (v, char *);
      if (arg == 0)
	break;
      len += strlen (arg);
    }
  va_end (v);

  rv = (char *) malloc (len + 1);
  strcpy (rv, s);
  v = save_v;
  while (1)
  {
    arg = va_arg (v, char *);
    if (arg == 0)
      break;
    strcat (rv, arg);
  }
  va_end (v);

  char *d, *p;

  /* concat is only used for urls and files, so we can safely
     canonicalize the results */
  for (p = d = rv; *p; p++)
    {
      *d++ = *p;
      /* special case for URLs */
      if (*p == ':' && p[1] == '/' && p[2] == '/' && p > rv + 1)
	{
	  *d++ = *++p;
	  *d++ = *++p;
	}
      else if (isslash (*p))
	{
	  if (p == rv && unc)
	    *d++ = *p++;
	  while (p[1] == '/')
	    p++;
	}
    }
  *d = 0;

  return rv;
}

static char *
concat (const char *s, ...)
{
  va_list v;

  va_start (v, s);

  return vconcat (s, v);
}

static void
unconvert_slashes (char* name)
{
  while ((name = strchr (name, '/')) != NULL)
    *name++ = '\\';
}

static char *
rel_vconcat (const char *s, va_list v)
{
  char path[MAX_PATH + 1];
  if (!GetCurrentDirectory (MAX_PATH, path))
    return NULL;

  int max_len = -1;
  struct mnt *m, *match = NULL;

  if (s[0] == '.' && isslash (s[1]))
    s += 2;

  for (m = mount_table; m->posix ; m++)
    {
      if (m->flags & MOUNT_CYGDRIVE)
	continue;

      int n = strlen (m->native);
      if (n < max_len || !path_prefix_p (m->native, path, n))
	continue;
      max_len = n;
      match = m;
    }

  char *temppath;
  if (!match)
    // No prefix matched - best effort to return meaningful value.
    temppath = concat (path, "/", s, NULL);
  else if (strcmp (match->posix, "/") != 0)
    // Matched on non-root.  Copy matching prefix + remaining 'path'.
    temppath = concat (match->posix, path + max_len, "/", s, NULL);
  else if (path[max_len] == '\0')
    // Matched on root and there's no remaining 'path'.
    temppath = concat ("/", s, NULL);
  else if (isslash (path[max_len]))
    // Matched on root but remaining 'path' starts with a slash anyway.
    temppath = concat (path + max_len, "/", s, NULL);
  else
    temppath = concat ("/", path + max_len, "/", s, NULL);

  char *res = vconcat (temppath, v);
  free (temppath);
  return res;
}

char *
cygpath (const char *s, ...)
{
  va_list v;
  int max_len = -1;
  struct mnt *m, *match = NULL;

  if (!mount_table[0].posix)
    read_mounts ();
  va_start (v, s);
  char *path;
  if (s[0] == '.' && isslash (s[1]))
    s += 2;

  if (s[0] == '/' || s[1] == ':')	/* FIXME: too crude? */
    path = vconcat (s, v);
  else
    path = rel_vconcat (s, v);

  if (!path)
    return NULL;

  if (strncmp (path, "/./", 3) == 0)
    memmove (path + 1, path + 3, strlen (path + 3) + 1);

  for (m = mount_table; m->posix ; m++)
    {
      if (m->flags & MOUNT_CYGDRIVE)
	continue;

      int n = strlen (m->posix);
      if (n < max_len || !path_prefix_p (m->posix, path, n))
	continue;
      max_len = n;
      match = m;
    }

  char *native;
  if (match == NULL)
    native = strdup (path);
  else if (max_len == (int) strlen (path))
    native = strdup (match->native);
  else if (isslash (path[max_len]))
    native = concat (match->native, path + max_len, NULL);
  else
    native = concat (match->native, "\\", path + max_len, NULL);
  free (path);

  unconvert_slashes (native);
  for (char *s = strstr (native + 1, "\\.\\"); s && *s; s = strstr (s, "\\.\\"))
    memmove (s + 1, s + 3, strlen (s + 3) + 1);
  return native;
}

static mnt *m = NULL;

extern "C" FILE *
setmntent (const char *, const char *)
{
  m = mount_table;
  if (!m->posix)
    read_mounts ();
  return NULL;
}

extern "C" struct mntent *
getmntent (FILE *)
{
  static mntent mnt;
  if (!m->posix)
    return NULL;

  mnt.mnt_fsname = (char *) m->native;
  mnt.mnt_dir = (char *) m->posix;
  if (!mnt.mnt_type)
    mnt.mnt_type = (char *) malloc (1024);
  if (!mnt.mnt_opts)
    mnt.mnt_opts = (char *) malloc (1024);
  if (!m->issys)
    strcpy (mnt.mnt_type, (char *) "user");
  else
    strcpy (mnt.mnt_type, (char *) "system");
  if (!(m->flags & MOUNT_BINARY))
    strcpy (mnt.mnt_opts, (char *) "textmode");
  else
    strcpy (mnt.mnt_opts, (char *) "binmode");
  if (m->flags & MOUNT_CYGWIN_EXEC)
    strcat (mnt.mnt_opts, (char *) ",cygexec");
  else if (m->flags & MOUNT_EXEC)
    strcat (mnt.mnt_opts, (char *) ",exec");
  else if (m->flags & MOUNT_NOTEXEC)
    strcat (mnt.mnt_opts, (char *) ",noexec");
  if (m->flags & MOUNT_ENC)
    strcat (mnt.mnt_opts, ",managed");
  if ((m->flags & MOUNT_CYGDRIVE))	/* cygdrive */
    strcat (mnt.mnt_opts, (char *) ",cygdrive");
  mnt.mnt_freq = 1;
  mnt.mnt_passno = 1;
  m++;
  return &mnt;
}
