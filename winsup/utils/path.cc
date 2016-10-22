/* path.cc

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
#include <lmcons.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <wchar.h>
#include "path.h"
#include "../cygwin/include/cygwin/version.h"
#include "../cygwin/include/sys/mount.h"
#define _NOMNTENT_MACROS
#include "../cygwin/include/mntent.h"
#include "testsuite.h"
#ifdef FSTAB_ONLY
#include <sys/cygwin.h>
#endif
#include "loadlib.h"

#ifndef FSTAB_ONLY
/* Used when treating / and \ as equivalent. */
#define isslash(ch) \
  ({ \
      char __c = (ch); \
      ((__c) == '/' || (__c) == '\\'); \
   })


static const GUID GUID_shortcut =
  {0x00021401L, 0, 0, {0xc0, 0, 0, 0, 0, 0, 0, 0x46}};

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
  bool ret = false;
  int magic = get_word (fh, 0x0);
  if (magic != SHORTCUT_MAGIC && magic != SYMLINK_MAGIC)
    goto out;
  DWORD got;
  BY_HANDLE_FILE_INFORMATION local;
  if (!GetFileInformationByHandle (fh, &local))
    return false;
  if (magic == SHORTCUT_MAGIC)
    {
      DWORD size;
      if (!local.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
	goto out; /* Not a Cygwin symlink. */
      if ((size = GetFileSize (fh, NULL)) > 8192)
	goto out; /* Not a Cygwin symlink. */
      char buf[size];
      SetFilePointer (fh, 0, 0, FILE_BEGIN);
      if (!ReadFile (fh, buf, size, &got, 0))
	goto out;
      if (got != size || !cmp_shortcut_header ((win_shortcut_hdr *) buf))
	goto out; /* Not a Cygwin symlink. */
      /* TODO: check for invalid path contents
	 (see symlink_info::check() in ../cygwin/path.cc) */
    }
  else /* magic == SYMLINK_MAGIC */
    {
      if (!(local.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
	goto out; /* Not a Cygwin symlink. */
      char buf[sizeof (SYMLINK_COOKIE) - 1];
      SetFilePointer (fh, 0, 0, FILE_BEGIN);
      if (!ReadFile (fh, buf, sizeof (buf), &got, 0))
	goto out;
      if (got != sizeof (buf) ||
	  memcmp (buf, SYMLINK_COOKIE, sizeof (buf)) != 0)
	goto out; /* Not a Cygwin symlink. */
    }
  ret = true;
out:
  SetFilePointer (fh, 0, 0, FILE_BEGIN);
  return ret;
}

/* Assumes is_symlink(fh) is true */
bool
readlink (HANDLE fh, char *path, int maxlen)
{
  DWORD rv;
  char *buf, *cp;
  unsigned short len;
  win_shortcut_hdr *file_header;
  BY_HANDLE_FILE_INFORMATION fi;

  if (!GetFileInformationByHandle (fh, &fi)
      || fi.nFileSizeHigh != 0
      || fi.nFileSizeLow > 4 * 65536)
    return false;

  buf = (char *) alloca (fi.nFileSizeLow + 1);
  file_header = (win_shortcut_hdr *) buf;

  if (!ReadFile (fh, buf, fi.nFileSizeLow, &rv, NULL)
      || rv != fi.nFileSizeLow)
    return false;

  if (fi.nFileSizeLow > sizeof (file_header)
      && cmp_shortcut_header (file_header))
    {
      cp = buf + sizeof (win_shortcut_hdr);
      if (file_header->flags & WSH_FLAG_IDLIST) /* Skip ITEMIDLIST */
	cp += *(unsigned short *) cp + 2;
      if (!(len = *(unsigned short *) cp))
	return false;
      cp += 2;
      /* Has appended full path?  If so, use it instead of description. */
      unsigned short relpath_len = *(unsigned short *) (cp + len);
      if (cp + len + 2 + relpath_len < buf + fi.nFileSizeLow)
	{
	  cp += len + 2 + relpath_len;
	  len = *(unsigned short *) cp;
	  cp += 2;
	}
      if (*(PWCHAR) cp == 0xfeff)	/* BOM */
	{
	  size_t wlen = wcstombs (NULL, (wchar_t *) (cp + 2), 0);
	  if (wlen == (size_t) -1 || wlen + 1 > maxlen)
	    return false;
	  wcstombs (path, (wchar_t *) (cp + 2), wlen + 1);
	}
      else if (len + 1 > maxlen)
	return false;
      else
	memcpy (path, cp, len);
      path[len] = '\0';
      return true;
    }
  else if (strncmp (buf, SYMLINK_COOKIE, strlen (SYMLINK_COOKIE)) == 0
	   && buf[fi.nFileSizeLow - 1] == '\0')
    {
      cp = buf + strlen (SYMLINK_COOKIE);
      if (*(PWCHAR) cp == 0xfeff)	/* BOM */
	{
	  size_t wlen = wcstombs (NULL, (wchar_t *) (cp + 2), 0);
	  if (wlen == (size_t) -1 || wlen + 1 > maxlen)
	    return false;
	  wcstombs (path, (wchar_t *) (cp + 2), wlen + 1);
	}
      else if (fi.nFileSizeLow - strlen (SYMLINK_COOKIE) > (unsigned) maxlen)
	return false;
      else
	strcpy (path, cp);
      return true;
    }
  else
    return false;
}
#endif /* !FSTAB_ONLY */

#ifndef TESTSUITE
mnt_t mount_table[255];
int max_mount_entry;
#else
#  define TESTSUITE_MOUNT_TABLE
#  include "testsuite.h"
#  undef TESTSUITE_MOUNT_TABLE
#endif

inline void
unconvert_slashes (char* name)
{
  while ((name = strchr (name, '/')) != NULL)
    *name++ = '\\';
}

/* These functions aren't called when defined(TESTSUITE) which results
   in a compiler warning.  */
#ifndef TESTSUITE
inline char *
skip_ws (char *in)
{
  while (*in == ' ' || *in == '\t')
    ++in;
  return in;
}

inline char *
find_ws (char *in)
{
  while (*in && *in != ' ' && *in != '\t')
    ++in;
  return in;
}

inline char *
conv_fstab_spaces (char *field)
{
  register char *sp = field;
  while ((sp = strstr (sp, "\\040")) != NULL)
    {
      *sp++ = ' ';
      memmove (sp, sp + 3, strlen (sp + 3) + 1);
    }
  return field;
}

#ifndef FSTAB_ONLY
static struct opt
{
  const char *name;
  unsigned val;
  bool clear;
} oopts[] =
{
  {"acl", MOUNT_NOACL, 1},
  {"auto", 0, 0},
  {"binary", MOUNT_BINARY, 0},
  {"cygexec", MOUNT_CYGWIN_EXEC, 0},
  {"dos", MOUNT_DOS, 0},
  {"exec", MOUNT_EXEC, 0},
  {"ihash", MOUNT_IHASH, 0},
  {"noacl", MOUNT_NOACL, 0},
  {"nosuid", 0, 0},
  {"notexec", MOUNT_NOTEXEC, 0},
  {"nouser", MOUNT_SYSTEM, 0},
  {"override", MOUNT_OVERRIDE, 0},
  {"posix=0", MOUNT_NOPOSIX, 0},
  {"posix=1", MOUNT_NOPOSIX, 1},
  {"text", MOUNT_BINARY, 1},
  {"user", MOUNT_SYSTEM, 1}
};

static bool
read_flags (char *options, unsigned &flags)
{
  while (*options)
    {
      char *p = strchr (options, ',');
      if (p)
	*p++ = '\0';
      else
	p = strchr (options, '\0');

      for (opt *o = oopts;
	   o < (oopts + (sizeof (oopts) / sizeof (oopts[0])));
	   o++)
	if (strcmp (options, o->name) == 0)
	  {
	    if (o->clear)
	      flags &= ~o->val;
	    else
	      flags |= o->val;
	    goto gotit;
	  }
      return false;

    gotit:
      options = p;
    }
  return true;
}
#endif

bool
from_fstab_line (mnt_t *m, char *line, bool user)
{
  char *native_path, *posix_path, *fs_type;

  /* First field: Native path. */
  char *c = skip_ws (line);
  if (!*c || *c == '#')
    return false;
  char *cend = find_ws (c);
  *cend = '\0';
  native_path = conv_fstab_spaces (c);
  /* Second field: POSIX path. */
  c = skip_ws (cend + 1);
  if (!*c)
    return false;
  cend = find_ws (c);
  *cend = '\0';
  posix_path = conv_fstab_spaces (c);
  /* Third field: FS type. */
  c = skip_ws (cend + 1);
  if (!*c)
    return false;
  cend = find_ws (c);
  *cend = '\0';
  fs_type = c;
  /* Forth field: Flags. */
  c = skip_ws (cend + 1);
  if (!*c)
    return false;
  cend = find_ws (c);
  *cend = '\0';
  unsigned mount_flags = MOUNT_SYSTEM;
#ifndef FSTAB_ONLY
  if (!read_flags (c, mount_flags))
#else
  if (cygwin_internal (CW_CVT_MNT_OPTS, &c, &mount_flags))
#endif
    return false;
  if (user)
    mount_flags &= ~MOUNT_SYSTEM;
  if (!strcmp (fs_type, "cygdrive"))
    {
      for (mnt_t *sm = mount_table; sm < m; ++sm)
	if (sm->flags & MOUNT_CYGDRIVE)
	  {
	    if ((mount_flags & MOUNT_SYSTEM) || !(sm->flags & MOUNT_SYSTEM))
	      {
		if (sm->posix)
		  free (sm->posix);
		sm->posix = strdup (posix_path);
		sm->flags = mount_flags | MOUNT_CYGDRIVE;
	      }
	    return false;
	  }
      m->posix = strdup (posix_path);
      m->native = strdup ("cygdrive prefix");
      m->flags = mount_flags | MOUNT_CYGDRIVE;
    }
  else
    {
      for (mnt_t *sm = mount_table; sm < m; ++sm)
	if (!strcmp (sm->posix, posix_path))
	  {
	    /* Don't allow overriding of a system mount with a user mount. */
	    if ((sm->flags & MOUNT_SYSTEM) && !(mount_flags & MOUNT_SYSTEM))
	      return false;
	    if ((sm->flags & MOUNT_SYSTEM) != (mount_flags & MOUNT_SYSTEM))
	      continue;
	    /* Changing immutable mount points require the override flag. */
	    if ((sm->flags & MOUNT_IMMUTABLE)
		&& !(mount_flags & MOUNT_OVERRIDE))
	      return false;
	    if (mount_flags & MOUNT_OVERRIDE)
	      mount_flags |= MOUNT_IMMUTABLE;
	    if (sm->native)
	      free (sm->native);
	    sm->native = strdup (native_path);
	    sm->flags = mount_flags;
	    return false;
	  }
      m->posix = strdup (posix_path);
      /* Bind mounts require POSIX paths, otherwise the path is wrongly
	 prefixed with the Cygwin root dir when trying to convert it to
	 a Win32 path in mount(2). So don't convert slashes to backslashes
         in this case. */
      if (!(mount_flags & MOUNT_BIND))
	unconvert_slashes (native_path);
      m->native = strdup (native_path);
      m->flags = mount_flags;
    }
  return true;
}

#ifndef FSTAB_ONLY

#define BUFSIZE 65536

static char *
get_user ()
{
  static char user[UNLEN + 1];
  char *userenv;

  user[0] = '\0';
  if ((userenv = getenv ("USER")) || (userenv = getenv ("USERNAME")))
    strncat (user, userenv, UNLEN);
  return user;
}

void
from_fstab (bool user, PWCHAR path, PWCHAR path_end)
{
  mnt_t *m = mount_table + max_mount_entry;
  char buf[BUFSIZE];

  if (!user)
    {
      /* Create a default root dir from path. */
      wcstombs (buf, path, BUFSIZE);
      unconvert_slashes (buf);
      char *native_path = buf;
      if (!strncmp (native_path, "\\\\?\\", 4))
	native_path += 4;
      if (!strncmp (native_path, "UNC\\", 4))
	*(native_path += 2) = '\\';
      m->posix = strdup ("/");
      m->native = strdup (native_path);
      m->flags = MOUNT_SYSTEM | MOUNT_BINARY | MOUNT_IMMUTABLE
		 | MOUNT_AUTOMATIC;
      ++m;
      /* Create default /usr/bin and /usr/lib entries. */
      char *trail = strchr (native_path, '\0');
      strcpy (trail, "\\bin");
      m->posix = strdup ("/usr/bin");
      m->native = strdup (native_path);
      m->flags = MOUNT_SYSTEM | MOUNT_BINARY | MOUNT_AUTOMATIC;
      ++m;
      strcpy (trail, "\\lib");
      m->posix = strdup ("/usr/lib");
      m->native = strdup (native_path);
      m->flags = MOUNT_SYSTEM | MOUNT_BINARY | MOUNT_AUTOMATIC;
      ++m;
      /* Create a default cygdrive entry.  Note that this is a user entry.
	 This allows to override it with mount, unless the sysadmin created
	 a cygdrive entry in /etc/fstab. */
      m->posix = strdup (CYGWIN_INFO_CYGDRIVE_DEFAULT_PREFIX);
      m->native = strdup ("cygdrive prefix");
      m->flags = MOUNT_BINARY | MOUNT_CYGDRIVE;
      ++m;
      max_mount_entry = m - mount_table;
    }

  PWCHAR u = wcscpy (path_end, L"\\etc\\fstab") + 10;
  if (user)
    mbstowcs (wcscpy (u, L".d\\") + 3, get_user (), BUFSIZE - (u - path));
  HANDLE h = CreateFileW (path, GENERIC_READ, FILE_SHARE_READ, NULL,
			  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE)
    return;
  char *got = buf;
  DWORD len = 0;
  /* Using BUFSIZE-1 leaves space to append two \0. */
  while (ReadFile (h, got, BUFSIZE - 1 - (got - buf),
		   &len, NULL))
    {
      char *end;

      /* Set end marker. */
      got[len] = got[len + 1] = '\0';
      /* Set len to the absolute len of bytes in buf. */
      len += got - buf;
      /* Reset got to start reading at the start of the buffer again. */
      got = buf;
      while (got < buf + len && (end = strchr (got, '\n')))
	{
	  end[end[-1] == '\r' ? -1 : 0] = '\0';
	  if (from_fstab_line (m, got, user))
	    ++m;
	  got = end + 1;
	}
      if (len < BUFSIZE - 1)
	break;
      /* We have to read once more.  Move remaining bytes to the start of
	 the buffer and reposition got so that it points to the end of
	 the remaining bytes. */
      len = buf + len - got;
      memmove (buf, got, len);
      got = buf + len;
      buf[len] = buf[len + 1] = '\0';
    }
  if (got > buf && from_fstab_line (m, got, user))
    ++m;
  max_mount_entry = m - mount_table;
  CloseHandle (h);
}
#endif /* !FSTAB_ONLY */
#endif /* !TESTSUITE */

#ifndef FSTAB_ONLY

static int
mnt_sort (const void *a, const void *b)
{
  const mnt_t *ma = (const mnt_t *) a;
  const mnt_t *mb = (const mnt_t *) b;
  int ret;

  ret = (ma->flags & MOUNT_CYGDRIVE) - (mb->flags & MOUNT_CYGDRIVE);
  if (ret)
    return ret;
  ret = (ma->flags & MOUNT_SYSTEM) - (mb->flags & MOUNT_SYSTEM);
  if (ret)
    return ret;
  return strcmp (ma->posix, mb->posix);
}

extern "C" WCHAR cygwin_dll_path[];

static void
read_mounts ()
{
/* If TESTSUITE is defined, bypass this whole function as a harness
   mount table will be provided.  */
#ifndef TESTSUITE
  HKEY setup_key;
  LONG ret;
  DWORD len;
  WCHAR path[32768];
  PWCHAR path_end;

  for (mnt_t *m1 = mount_table; m1->posix; m1++)
    {
      free (m1->posix);
      if (m1->native)
	free ((char *) m1->native);
      m1->posix = NULL;
    }
  max_mount_entry = 0;

  /* First fetch the cygwin1.dll path from the LoadLibrary call in load_cygwin.
     This utilizes the DLL search order to find a matching cygwin1.dll and to
     compute the installation path from that DLL's path. */
  if (cygwin_dll_path[0])
    wcscpy (path, cygwin_dll_path);
  /* If we can't load cygwin1.dll, check where cygcheck is living itself and
     try to fetch installation path from here.  Does cygwin1.dll exist in the
     same path?  This should only kick in if the cygwin1.dll in the same path
     has been made non-executable for the current user accidentally. */
  else if (!GetModuleFileNameW (NULL, path, 32768))
    return;
  path_end = wcsrchr (path, L'\\');
  if (path_end)
    {
      if (!cygwin_dll_path[0])
	{
	  wcscpy (path_end, L"\\cygwin1.dll");
	  DWORD attr = GetFileAttributesW (path);
	  if (attr == (DWORD) -1
	      || (attr & (FILE_ATTRIBUTE_DIRECTORY
			  | FILE_ATTRIBUTE_REPARSE_POINT)))
	    path_end = NULL;
	}
      if (path_end)
	{
	  *path_end = L'\0';
	  path_end = wcsrchr (path, L'\\');
	}
    }
  /* If we can't create a valid installation dir from that, try to fetch
     the installation dir from the setup registry key. */
  if (!path_end)
    {
      for (int i = 0; i < 2; ++i)
	if ((ret = RegOpenKeyExW (i ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
				  L"Software\\Cygwin\\setup", 0,
				  KEY_READ, &setup_key)) == ERROR_SUCCESS)
	  {
	    len = 32768 * sizeof (WCHAR);
	    ret = RegQueryValueExW (setup_key, L"rootdir", NULL, NULL,
				    (PBYTE) path, &len);
	    RegCloseKey (setup_key);
	    if (ret == ERROR_SUCCESS)
	      break;
	  }
      if (ret == ERROR_SUCCESS)
	path_end = wcschr (path, L'\0');
    }
  /* If we can't fetch an installation dir, bail out. */
  if (!path_end)
    return;
  *path_end = L'\0';

  from_fstab (false, path, path_end);
  from_fstab (true, path, path_end);
  qsort (mount_table, max_mount_entry, sizeof (mnt_t), mnt_sort);
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

/* This is a helper function for when vcygpath is passed what appears
   to be a relative POSIX path.  We take a Win32 CWD (either as specified
   in 'cwd' or as retrieved with GetCurrentDirectory() if 'cwd' is NULL)
   and find the mount table entry with the longest match.  We replace the
   matching portion with the corresponding POSIX prefix, and to that append
   's' and anything in 'v'.  The returned result is a mostly-POSIX
   absolute path -- 'mostly' because the portions of CWD that didn't
   match the mount prefix will still have '\\' separators.  */
static char *
rel_vconcat (const char *cwd, const char *s, va_list v)
{
  char pathbuf[MAX_PATH];
  if (!cwd || *cwd == '\0')
    {
      if (!GetCurrentDirectory (MAX_PATH, pathbuf))
	return NULL;
      cwd = pathbuf;
    }

  int max_len = -1;
  mnt_t *m, *match = NULL;

  for (m = mount_table; m->posix; m++)
    {
      if (m->flags & MOUNT_CYGDRIVE)
	continue;

      int n = strlen (m->native);
      if (n < max_len || !path_prefix_p (m->native, cwd, n))
	continue;
      max_len = n;
      match = m;
    }

  char *temppath;
  if (!match)
    // No prefix matched - best effort to return meaningful value.
    temppath = concat (cwd, "/", s, NULL);
  else if (strcmp (match->posix, "/") != 0)
    // Matched on non-root.  Copy matching prefix + remaining 'path'.
    temppath = concat (match->posix, cwd + max_len, "/", s, NULL);
  else if (cwd[max_len] == '\0')
    // Matched on root and there's no remaining 'path'.
    temppath = concat ("/", s, NULL);
  else if (isslash (cwd[max_len]))
    // Matched on root but remaining 'path' starts with a slash anyway.
    temppath = concat (cwd + max_len, "/", s, NULL);
  else
    temppath = concat ("/", cwd + max_len, "/", s, NULL);

  char *res = vconcat (temppath, v);
  free (temppath);
  return res;
}

/* Convert a POSIX path in 's' to an absolute Win32 path, and append
   anything in 'v' to the end, returning the result.  If 's' is a
   relative path then 'cwd' is used as the working directory to make
   it absolute.  Pass NULL in 'cwd' to use GetCurrentDirectory.  */
static char *
vcygpath (const char *cwd, const char *s, va_list v)
{
  int max_len = -1;
  mnt_t *m, *match = NULL;

  if (!max_mount_entry)
    read_mounts ();
  char *path;
  if (s[0] == '.' && isslash (s[1]))
    s += 2;

  if (s[0] == '/' || s[1] == ':')	/* FIXME: too crude? */
    path = vconcat (s, v);
  else
    path = rel_vconcat (cwd, s, v);

  if (!path)
    return NULL;

  if (strncmp (path, "/./", 3) == 0)
    memmove (path + 1, path + 3, strlen (path + 3) + 1);

  for (m = mount_table; m->posix; m++)
    {
      int n = strlen (m->posix);
      if (n < max_len || !path_prefix_p (m->posix, path, n))
	continue;
      if ((m->flags & MOUNT_CYGDRIVE)
	  && ((int) strlen (path) < n + 2
	      || path[n] != '/'
	      || !isalpha (path[n + 1])
	      || path[n + 2] != '/'))
	continue;
      max_len = n;
      match = m;
    }

  char *native;
  if (match == NULL)
    native = strdup (path);
  else if (max_len == (int) strlen (path))
    native = strdup (match->native);
  else if (match->flags & MOUNT_CYGDRIVE)
    {
      char drive[3] = { path[max_len + 1], ':', '\0' };
      native = concat (drive, path + max_len + 2, NULL);
    }
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

char *
cygpath_rel (const char *cwd, const char *s, ...)
{
  va_list v;

  va_start (v, s);

  return vcygpath (cwd, s, v);
}

char *
cygpath (const char *s, ...)
{
  va_list v;

  va_start (v, s);

  return vcygpath (NULL, s, v);
}

static mnt_t *m = NULL;

extern "C" FILE *
setmntent (const char *, const char *)
{
  m = mount_table;
  if (!max_mount_entry)
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
    mnt.mnt_type = (char *) malloc (16);
  if (!mnt.mnt_opts)
    mnt.mnt_opts = (char *) malloc (64);

  strcpy (mnt.mnt_type,
	  (char *) ((m->flags & MOUNT_SYSTEM) ? "system" : "user"));

  if (!(m->flags & MOUNT_BINARY))
    strcpy (mnt.mnt_opts, (char *) "text");
  else
    strcpy (mnt.mnt_opts, (char *) "binary");

  if (m->flags & MOUNT_CYGWIN_EXEC)
    strcat (mnt.mnt_opts, (char *) ",cygexec");
  else if (m->flags & MOUNT_EXEC)
    strcat (mnt.mnt_opts, (char *) ",exec");
  else if (m->flags & MOUNT_NOTEXEC)
    strcat (mnt.mnt_opts, (char *) ",notexec");

  if (m->flags & MOUNT_NOACL)
    strcat (mnt.mnt_opts, (char *) ",noacl");

  if (m->flags & MOUNT_NOPOSIX)
    strcat (mnt.mnt_opts, (char *) ",posix=0");

  if (m->flags & (MOUNT_AUTOMATIC | MOUNT_CYGDRIVE))
    strcat (mnt.mnt_opts, (char *) ",auto");

  mnt.mnt_freq = 1;
  mnt.mnt_passno = 1;
  m++;
  return &mnt;
}

#endif /* !FSTAB_ONLY */
