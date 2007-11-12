/* path.cc

   Copyright 2001, 2002, 2003, 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* The purpose of this file is to hide all the details about accessing
   Cygwin's mount table.  If the format or location of the mount table
   changes, this is the file to change to match it. */

#define str(a) #a
#define scat(a,b) str(a##b)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "cygwin/include/cygwin/version.h"
#include "cygwin/include/sys/mount.h"
#include "cygwin/include/mntent.h"

/* Used when treating / and \ as equivalent. */
#define SLASH_P(ch) \
  ({ \
      char __c = (ch); \
      ((__c) == '/' || (__c) == '\\'); \
   })


static struct mnt
  {
    const char *native;
    char *posix;
    unsigned flags;
    int issys;
  } mount_table[255];

struct mnt *root_here = NULL;

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

static void
read_mounts ()
{
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
  if (len1 > 0 && SLASH_P (path1[len1 - 1]))
    len1--;

  if (len1 == 0)
    return SLASH_P (path2[0]) && !SLASH_P (path2[1]);

  if (strncasecmp (path1, path2, len1) != 0)
    return 0;

  return SLASH_P (path2[len1]) || path2[len1] == 0 || path1[len1 - 1] == ':';
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

  unc = SLASH_P (*s) && SLASH_P (s[1]);

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
      else if (*p == '/' || *p == '\\')
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

char *
cygpath (const char *s, ...)
{
  va_list v;
  int max_len = -1;
  struct mnt *m, *match = NULL;

  if (!mount_table[0].posix)
    read_mounts ();
  va_start (v, s);
  char *path = vconcat (s, v);
  if (strncmp (path, "./", 2) == 0)
    memmove (path, path + 2, strlen (path + 2) + 1);
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
  else
    native = concat (match->native, "\\", path + max_len, NULL);
  free (path);

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
  if ((m->flags & MOUNT_CYGDRIVE))             /* cygdrive */
    strcat (mnt.mnt_opts, (char *) ",cygdrive");
  mnt.mnt_freq = 1;
  mnt.mnt_passno = 1;
  m++;
  return &mnt;
}
