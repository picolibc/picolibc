/* path.cc: path support.

   Copyright 1996, 1997, 1998, 1999, 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This module's job is to
   - convert between POSIX and Win32 style filenames,
   - support the `mount' functionality,
   - support symlinks for files and directories

   Pathnames are handled as follows:

   - A \ or : in a path denotes a pure windows spec.
   - Paths beginning with // (or \\) are not translated (i.e. looked
     up in the mount table) and are assumed to be UNC path names.

   The goal in the above set of rules is to allow both POSIX and Win32
   flavors of pathnames without either interfering.  The rules are
   intended to be as close to a superset of both as possible.

   Note that you can have more than one path to a file.  The mount
   table is always prefered when translating Win32 paths to POSIX
   paths.  Win32 paths in mount table entries may be UNC paths or
   standard Win32 paths starting with <drive-letter>:

   Text vs Binary issues are not considered here in path style
   decisions, although the appropriate flags are retrieved and
   stored in various structures.

   Removing mounted filesystem support would simplify things greatly,
   but having it gives us a mechanism of treating disk that lives on a
   UNIX machine as having UNIX semantics [it allows one to edit a text
   file on that disk and not have cr's magically appear and perhaps
   break apps running on UNIX boxes].  It also useful to be able to
   layout a hierarchy without changing the underlying directories.

   The semantics of mounting file systems is not intended to precisely
   follow normal UNIX systems.

   Each DOS drive is defined to have a current directory.  Supporting
   this would complicate things so for now things are defined so that
   c: means c:\.  FIXME: Is this still true?
*/

#include "winsup.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <mntent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <winioctl.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "path.h"
#include "thread.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygheap.h"
#include "shared_info.h"
#include "registry.h"
#include "security.h"

static int normalize_win32_path (const char *src, char *dst);
static void slashify (const char *src, char *dst, int trailing_slash_p);
static void backslashify (const char *src, char *dst, int trailing_slash_p);
static int path_prefix_p_ (const char *path1, const char *path2, int len1);

struct symlink_info
{
  char buf[3 + MAX_PATH * 3];
  char *known_suffix;
  char *ext_here;
  char *contents;
  unsigned pflags;
  DWORD fileattr;
  int is_symlink;
  int error;
  symlink_info (): known_suffix (NULL), contents (buf + MAX_PATH + 1) {}
  int check (const char *path, const suffix_info *suffixes);
};

cwdstuff cygcwd;	/* The current working directory. */

#define path_prefix_p(p1, p2, l1) \
       ((tolower(*(p1))==tolower(*(p2))) && \
       path_prefix_p_(p1, p2, l1))

#define SYMLINKATTR(x) \
  (((x) & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)) == \
   FILE_ATTRIBUTE_SYSTEM)

/* Determine if path prefix matches current cygdrive */
#define iscygdrive(path) \
  (path_prefix_p (cygwin_shared->mount.cygdrive, (path), cygwin_shared->mount.cygdrive_len))

#define iscygdrive_device(path) \
  (iscygdrive(path) && isalpha(path[cygwin_shared->mount.cygdrive_len]) && \
   (isdirsep(path[cygwin_shared->mount.cygdrive_len + 1]) || \
    !path[cygwin_shared->mount.cygdrive_len + 1]))

#define ischrootpath(path) \
	(myself->rootlen && \
	 strncasematch (myself->root, path, myself->rootlen) && \
	 (path[myself->rootlen] == '/' || path[myself->rootlen] == '\0'))

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
path_prefix_p_ (const char *path1, const char *path2, int len1)
{
  /* Handle case where PATH1 has trailing '/' and when it doesn't.  */
  if (len1 > 0 && SLASH_P (path1[len1 - 1]))
    len1--;

  if (len1 == 0)
    return SLASH_P (path2[0]) && !SLASH_P (path2[1]);

  if (!strncasematch (path1, path2, len1))
    return 0;

  return SLASH_P (path2[len1]) || path2[len1] == 0 || path1[len1 - 1] == ':';
}

/* Convert an arbitrary path SRC to a pure Win32 path, suitable for
   passing to Win32 API routines.

   If an error occurs, `error' is set to the errno value.
   Otherwise it is set to 0.

   follow_mode values:
	SYMLINK_FOLLOW	    - convert to PATH symlink points to
	SYMLINK_NOFOLLOW    - convert to PATH of symlink itself
	SYMLINK_IGNORE	    - do not check PATH for symlinks
	SYMLINK_CONTENTS    - just return symlink contents
*/

void
path_conv::check (const char *src, unsigned opt,
		  const suffix_info *suffixes)
{
  /* This array is used when expanding symlinks.  It is MAX_PATH * 2
     in length so that we can hold the expanded symlink plus a
     trailer.  */
  char path_buf[MAX_PATH];
  char path_copy[MAX_PATH];
  char tmp_buf[MAX_PATH];
  symlink_info sym;

  char *rel_path, *full_path;

  if (!(opt & PC_NULLEMPTY))
    error = 0;
  else if ((error = check_null_empty_path (src)))
    return;

  if (opt & PC_FULL)
    rel_path = path_buf, full_path = this->path;
  else
    rel_path = this->path, full_path = path_buf;

  /* This loop handles symlink expansion.  */
  int loop = 0;
  path_flags = 0;
  known_suffix = NULL;
  fileattr = (DWORD) -1;
  for (;;)
    {
      MALLOC_CHECK;
      /* Must look up path in mount table, etc.  */
      error = cygwin_shared->mount.conv_to_win32_path (src, rel_path,
						       full_path,
						       devn, unit, &path_flags);
      MALLOC_CHECK;
      if (error)
	return;
      if (devn != FH_BAD)
	{
	  fileattr = 0;
	  return;
	}

      /* Eat trailing slashes */
      char *tail = strchr (full_path, '\0');
      /* If path is only a drivename, Windows interprets it as
	 the current working directory on this drive instead of
	 the root dir which is what we want. So we need
	 the trailing backslash in this case. */
      while (tail > full_path + 3 && (*--tail == '\\'))
	*tail = '\0';
      if (full_path[0] && full_path[1] == ':' && full_path[2] == '\0')
	strcat (full_path, "\\");

      if (opt & PC_SYM_IGNORE)
	{
	  fileattr = GetFileAttributesA (path);
	  goto out;
	}

      /* Make a copy of the path that we can munge up */
      strcpy (path_copy, full_path);

      tail = path_copy + 1 + (tail - full_path);   // Point to end of copy

      /* Scan path_copy from right to left looking either for a symlink
	 or an actual existing file.  If an existing file is found, just
	 return.  If a symlink is found exit the for loop.
	 Also: be careful to preserve the errno returned from
	 symlink.check as the caller may need it. */
      /* FIXME: Do we have to worry about multiple \'s here? */
      int component = 0;		// Number of translated components
      sym.contents[0] = '\0';

      for (;;)
	{
	  const suffix_info *suff;

	  /* Don't allow symlink.check to set anything in the path_conv
	     class if we're working on an inner component of the path */
	  if (component)
	    {
	      suff = NULL;
	      sym.pflags = 0;
	    }
	  else
	    {
	      suff = suffixes;
	      sym.pflags = path_flags;
	    }

	  int len = sym.check (path_copy, suff);

	  if (!component)
	    path_flags = sym.pflags;

	  /* If symlink.check found an existing non-symlink file, then
	     it sets the appropriate flag.  It also sets any suffix found
	     into `ext_here'. */
	  if (!sym.is_symlink && sym.fileattr != (DWORD) -1)
	    {
	      error = sym.error;
	      if (component == 0)
		{
		  fileattr = sym.fileattr;
		  goto fillin;
		}
	      goto out;	// file found
	    }
	  /* Found a symlink if len > 0.  If component == 0, then the
	     src path itself was a symlink.  If !follow_mode then
	     we're done.  Otherwise we have to insert the path found
	     into the full path that we are building and perform all of
	     these operations again on the newly derived path. */
	  else if (len > 0)
	    {
	      if (component == 0 && !(opt & PC_SYM_FOLLOW))
		{
		  set_symlink (); // last component of path is a symlink.
		  fileattr = sym.fileattr;
		  if (opt & PC_SYM_CONTENTS)
		      strcpy (path, sym.contents);
		  goto fillin;
		}
	      break;
	    }

	  /* No existing file found. */

	  if (!(tail = strrchr (path_copy, '\\')) ||
	      (tail > path_copy && tail[-1] == ':'))
	    goto out;	// all done

	  /* Haven't found a valid pathname component yet.
	     Pinch off the tail and try again. */
	  *tail = '\0';
	  component++;
	}

      /* Arrive here if above loop detected a symlink. */
      if (++loop > MAX_LINK_DEPTH)
	{
	  error = ELOOP;   // Eep.
	  return;
	}
      MALLOC_CHECK;

      tail = full_path + (tail - path_copy);
      int taillen = strlen (tail);
      int buflen = strlen (sym.contents);
      if (buflen + taillen > MAX_PATH)
	  {
	    error = ENAMETOOLONG;
	    strcpy (path, "::ENAMETOOLONG::");
	    return;
	  }

      /* Copy tail of full_path to discovered symlink. */
      char *p;
      for (p = sym.contents + buflen; *tail; tail++)
	*p++ = *tail == '\\' ? '/' : *tail;
      *p = '\0';

      /* If symlink referred to an absolute path, then we
	 just use sym.contents and loop.  Otherwise tack the head of
	 path_copy before sym.contents and translate it back from a
	 Win32-style path to a POSIX-style one. */
      if (isabspath (sym.contents))
	src = sym.contents;
      else if (!(tail = strrchr (path_copy, '\\')))
	system_printf ("problem parsing %s - '%s'", src, full_path);
      else
	{
	  int headlen = 1 + tail - path_copy;
	  p = sym.contents - headlen;
	  memcpy (p, path_copy, headlen);
	  MALLOC_CHECK;
	  error = cygwin_shared->mount.conv_to_posix_path (p, tmp_buf, 1);
	  MALLOC_CHECK;
	  if (error)
	    return;
	  src = tmp_buf;
	}
    }

fillin:
  if (sym.known_suffix)
    known_suffix = this->path + (sym.known_suffix - path_copy);
  else if (sym.ext_here && !(opt & PC_SYM_CONTENTS))
    {
      known_suffix = strchr (this->path, '\0');
      strcpy (known_suffix, sym.ext_here);
    }

out:
  DWORD serial, volflags;

  strcpy (tmp_buf, full_path);
  if (!rootdir (tmp_buf) ||
      !GetVolumeInformation (tmp_buf, NULL, 0, &serial, NULL, &volflags, NULL, 0))
    {
      debug_printf ("GetVolumeInformation(%s) = ERR, full_path(%s), set_has_acls(FALSE)",
		    tmp_buf, full_path, GetLastError ());
      set_has_acls (FALSE);
    }
  else
    {
      debug_printf ("GetVolumeInformation(%s) = OK, full_path(%s), set_has_acls(%d)",
		    tmp_buf, full_path, volflags & FS_PERSISTENT_ACLS);
      set_has_acls (volflags & FS_PERSISTENT_ACLS);
    }
}

#define deveq(s) (strcasematch (name, (s)))
#define deveqn(s, n) (strncasematch (name, (s), (n)))

static __inline int
digits (const char *name)
{
  char *p;
  int n = strtol(name, &p, 10);

  return p > name && !*p ? n : -1;
}

const char *windows_device_names[] =
{
  NULL,
  "\\dev\\console",
  "conin",
  "conout",
  "\\dev\\ttym",
  "\\dev\\tty%d",
  "\\dev\\ptym",
  "\\\\.\\com%d",
  "\\dev\\pipe",
  "\\dev\\piper",
  "\\dev\\pipew",
  "\\dev\\socket",
  "\\dev\\windows",

  NULL, NULL, NULL,

  "\\dev\\disk",
  "\\dev\\fd%d",
  "\\dev\\st%d",
  "nul",
  "\\dev\\zero",
  "\\dev\\%srandom",
};

static int
get_raw_device_number (const char *uxname, const char *w32path, int &unit)
{
  DWORD devn = FH_BAD;

  if (strncasematch (w32path, "\\\\.\\tape", 8))
    {
      devn = FH_TAPE;
      unit = digits (w32path + 8);
      // norewind tape devices have leading n in name
      if (strncasematch (uxname, "/dev/n", 6))
	unit += 128;
    }
  else if (isdrive (w32path + 4))
    {
      devn = FH_FLOPPY;
      unit = tolower (w32path[4]) - 'a';
    }
  else if (strncasematch (w32path, "\\\\.\\physicaldrive", 17))
    {
      devn = FH_FLOPPY;
      unit = digits (w32path + 17) + 128;
    }
  return devn;
}

int __stdcall
get_device_number (const char *name, int &unit, BOOL from_conv)
{
  DWORD devn = FH_BAD;
  unit = 0;

  if ((*name == '/' && deveqn ("/dev/", 5)) ||
      (*name == '\\' && deveqn ("\\dev\\", 5)))
    {
      name += 5;
      if (deveq ("tty"))
	{
	  if (tty_attached (myself))
	    {
	      unit = myself->ctty;
	      devn = FH_TTYS;
	    }
	  else if (myself->ctty > 0)
	    devn = FH_CONSOLE;
	}
      else if (deveqn ("tty", 3) && (unit = digits (name + 3)) >= 0)
	devn = FH_TTYS;
      else if (deveq ("ttym"))
	devn = FH_TTYM;
      else if (deveq ("ptmx"))
	devn = FH_PTYM;
      else if (deveq ("windows"))
	devn = FH_WINDOWS;
      else if (deveq ("conin"))
	devn = FH_CONIN;
      else if (deveq ("conout"))
	devn = FH_CONOUT;
      else if (deveq ("null"))
	devn = FH_NULL;
      else if (deveq ("zero"))
	devn = FH_ZERO;
      else if (deveq ("random") || deveq ("urandom"))
	{
	  devn = FH_RANDOM;
	  unit = 8 + (deveqn ("u", 1) ? 1 : 0); /* Keep unit Linux conformant */
	}
      else if (deveqn ("com", 3) && (unit = digits (name + 3)) >= 0)
	devn = FH_SERIAL;
      else if (deveq ("pipe") || deveq ("piper") || deveq ("pipew"))
	devn = FH_PIPE;
      else if (deveq ("tcp") || deveq ("udp") || deveq ("streamsocket")
	       || deveq ("dgsocket"))
	devn = FH_SOCKET;
      else if (! from_conv)
	devn = get_raw_device_number (name - 5,
				      path_conv (name - 5,
						 PC_SYM_IGNORE).get_win32 (),
				      unit);
    }
  else if (deveqn ("com", 3) && (unit = digits (name + 3)) >= 0)
    devn = FH_SERIAL;

  return devn;
}

/* Return TRUE if src_path is a Win32 device name, filling out the device
   name in win32_path */

static BOOL
win32_device_name (const char *src_path, char *win32_path,
		   DWORD &devn, int &unit)
{
  const char *devfmt;

  devn = get_device_number (src_path, unit, TRUE);

  if (devn == FH_BAD)
    return FALSE;

  if ((devfmt = windows_device_names[FHDEVN (devn)]) == NULL)
    return FALSE;
  if (devn == FH_RANDOM)
    __small_sprintf (win32_path, devfmt, unit == 8 ? "" : "u");
  else
    __small_sprintf (win32_path, devfmt, unit);
  return TRUE;
}

/* Normalize a POSIX path.
   \'s are converted to /'s in the process.
   All duplicate /'s, except for 2 leading /'s, are deleted.
   The result is 0 for success, or an errno error value.  */

#define isslash(c) ((c) == '/')

static int
normalize_posix_path (const char *src, char *dst)
{
  const char *src_start = src;
  char *dst_start = dst;

  syscall_printf ("src %s", src);
  if (isdrive (src) || strpbrk (src, "\\:"))
    {
      cygwin_conv_to_full_posix_path (src, dst);
      return 0;
    }
  if (!isslash (src[0]))
    {
      char cwd[MAX_PATH];
      if (!cygcwd.get (cwd))
	return get_errno ();
      if (strlen (cwd) + 1 + strlen (src) >= MAX_PATH)
	{
	  debug_printf ("ENAMETOOLONG = normalize_posix_path (%s)", src);
	  return ENAMETOOLONG;
	}
      strcpy (dst, cwd);
      dst = strchr (dst, '\0');
      if (*src == '.')
	goto sawdot;
      if (dst > dst_start && !isslash (dst[-1]))
	*dst++ = '/';
    }
  /* Two leading /'s?  If so, preserve them.  */
  else if (isslash (src[1]))
    {
      if (myself->rootlen)
	{
	  debug_printf ("ENOENT = normalize_posix_path (%s)", src);
	  return ENOENT;
	}
      *dst++ = '/';
      *dst++ = '/';
      src += 2;
      if (isslash (*src))
	{ /* Starts with three or more slashes - reset. */
	  dst = dst_start;
	  *dst++ = '/';
	  src = src_start + 1;
	}
    }
  /* Exactly one leading slash. Absolute path. Check for chroot. */
  else if (myself->rootlen)
    {
      strcpy (dst, myself->root);
      dst += myself->rootlen;
    }

  while (*src)
    {
      /* Strip runs of /'s.  */
      if (!isslash (*src))
	*dst++ = *src++;
      else
	{
	  while (*++src)
	    {
	      if (isslash (*src))
		continue;

	      if (*src != '.')
		break;

	    sawdot:
	      if (src[1] != '.')
		{
		  if (!src[1])
		    {
		      if (dst == dst_start)
			*dst++ = '/';
		      goto done;
		    }
		  if (!isslash (src[1]))
		    break;
		}
	      else if (src[2] && !isslash (src[2]))
		break;
	      else
		{
		  if (!ischrootpath (dst_start) ||
		      dst - dst_start != (int) myself->rootlen)
		    while (dst > dst_start && !isslash (*--dst))
		      continue;
		  src++;
		}
	    }

	  *dst++ = '/';
	}
    }

done:
  *dst = '\0';
  if (--dst > dst_start && isslash (*dst))
    *dst = '\0';

  debug_printf ("%s = normalize_posix_path (%s)", dst_start, src_start);
  return 0;
}

/* Normalize a Win32 path.
   /'s are converted to \'s in the process.
   All duplicate \'s, except for 2 leading \'s, are deleted.

   The result is 0 for success, or an errno error value.
   FIXME: A lot of this should be mergeable with the POSIX critter.  */
static int
normalize_win32_path (const char *src, char *dst)
{
  const char *src_start = src;
  char *dst_start = dst;
  char *dst_root_start = dst;

  if (!SLASH_P (src[0]) && strchr (src, ':') == NULL)
    {
      char cwd[MAX_PATH];
      if (!cygcwd.get (cwd, 0))
	return get_errno ();
      if (strlen (cwd) + 1 + strlen (src) >= MAX_PATH)
	{
	  debug_printf ("ENAMETOOLONG = normalize_win32_path (%s)", src);
	  return ENAMETOOLONG;
	}
      strcpy (dst, cwd);
      dst += strlen (dst);
      if (!*cwd || !SLASH_P (dst[-1]))
	*dst++ = '\\';
    }
  /* Two leading \'s?  If so, preserve them.  */
  else if (SLASH_P (src[0]) && SLASH_P (src[1]))
    {
      if (myself->rootlen)
	{
	  debug_printf ("ENOENT = normalize_win32_path (%s)", src);
	  return ENOENT;
	}
      *dst++ = '\\';
      ++src;
    }
  /* If absolute path, care for chroot. */
  else if (SLASH_P (src[0]) && !SLASH_P (src[1]) && myself->rootlen)
    {
      strcpy (dst, myself->root);
      char *c;
      while ((c = strchr (dst, '/')) != NULL)
	*c = '\\';
      dst += myself->rootlen;
      dst_root_start = dst;
      *dst++ = '\\';
    }

  while (*src)
    {
      /* Strip duplicate /'s.  */
      if (SLASH_P (src[0]) && SLASH_P (src[1]))
	src++;
      /* Ignore "./".  */
      else if (src[0] == '.' && SLASH_P (src[1])
	       && (src == src_start || SLASH_P (src[-1])))
	{
	  src += 2;
	}

      /* Backup if "..".  */
      else if (src[0] == '.' && src[1] == '.'
	       /* dst must be greater than dst_start */
	       && dst[-1] == '\\'
	       && (SLASH_P (src[2]) || src[2] == 0))
	{
	  /* Back up over /, but not if it's the first one.  */
	  if (dst > dst_root_start + 1)
	    dst--;
	  /* Now back up to the next /.  */
	  while (dst > dst_root_start + 1 && dst[-1] != '\\' && dst[-2] != ':')
	    dst--;
	  src += 2;
	  if (SLASH_P (*src))
	    src++;
	}
      /* Otherwise, add char to result.  */
      else
	{
	  if (*src == '/')
	    *dst++ = '\\';
	  else
	    *dst++ = *src;
	  ++src;
	}
    }
  *dst = 0;
  debug_printf ("%s = normalize_win32_path (%s)", dst_start, src_start);
  return 0;
}


/* Various utilities.  */

/* slashify: Convert all back slashes in src path to forward slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

static void
slashify (const char *src, char *dst, int trailing_slash_p)
{
  const char *start = src;

  while (*src)
    {
      if (*src == '\\')
	*dst++ = '/';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '/';
  *dst++ = 0;
}

/* backslashify: Convert all forward slashes in src path to back slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

static void
backslashify (const char *src, char *dst, int trailing_slash_p)
{
  const char *start = src;

  while (*src)
    {
      if (*src == '/')
	*dst++ = '\\';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '\\';
  *dst++ = 0;
}

/* nofinalslash: Remove trailing / and \ from SRC (except for the
   first one).  It is ok for src == dst.  */

void __stdcall
nofinalslash (const char *src, char *dst)
{
  int len = strlen (src);
  if (src != dst)
    memcpy (dst, src, len + 1);
  while (len > 1 && SLASH_P (dst[--len]))
    dst[len] = '\0';
}

/* slash_drive_prefix_p: Return non-zero if PATH begins with
   //<letter>.  */

static int
slash_drive_prefix_p (const char *path)
{
  return (isdirsep(path[0])
	  && isdirsep(path[1])
	  && isalpha (path[2])
	  && (path[3] == 0 || path[3] == '/'));
}

/* slash_unc_prefix_p: Return non-zero if PATH begins with //UNC/SHARE */

int __stdcall
slash_unc_prefix_p (const char *path)
{
  char *p = NULL;
  int ret = (isdirsep (path[0])
	     && isdirsep (path[1])
	     && isalpha (path[2])
	     && path[3] != 0
	     && !isdirsep (path[3])
	     && ((p = strchr(&path[3], '/')) != NULL));
  if (!ret || p == NULL)
    return ret;
  return ret && isalnum (p[1]);
}

/* conv_path_list: Convert a list of path names to/from Win32/POSIX.

   SRC is not a const char * because we temporarily modify it to ease
   the implementation.

   I believe Win32 always has '.' in $PATH.   POSIX obviously doesn't.
   We certainly don't want to handle that here, but it is something for
   the caller to think about.  */

static void
conv_path_list (const char *src, char *dst, int to_posix_p)
{
  char *s;
  char *d = dst;
  char src_delim = to_posix_p ? ';' : ':';
  char dst_delim = to_posix_p ? ':' : ';';
  int (*conv_fn) (const char *, char *) = (to_posix_p
					   ? cygwin_conv_to_posix_path
					   : cygwin_conv_to_win32_path);

  do
    {
      s = strchr (src, src_delim);
      if (s)
	{
	  *s = 0;
	  (*conv_fn) (src[0] != 0 ? src : ".", d);
	  d += strlen (d);
	  *d++ = dst_delim;
	  *s = src_delim;
	  src = s + 1;
	}
      else
	{
	  /* Last one.  */
	  (*conv_fn) (src[0] != 0 ? src : ".", d);
	}
    }
  while (s != NULL);
}

/************************* mount_info class ****************************/

/* init: Initialize the mount table.  */

void
mount_info::init ()
{
  nmounts = 0;
  had_to_create_mount_areas = 0;

  /* Fetch the mount table and cygdrive-related information from
     the registry.  */
  from_registry ();
}

/* conv_to_win32_path: Ensure src_path is a pure Win32 path and store
   the result in win32_path.

   If win32_path != NULL, the relative path, if possible to keep, is
   stored in win32_path.  If the relative path isn't possible to keep,
   the full path is stored.

   If full_win32_path != NULL, the full path is stored there.

   The result is zero for success, or an errno value.

   {,full_}win32_path must have sufficient space (i.e. MAX_PATH bytes).  */

int
mount_info::conv_to_win32_path (const char *src_path, char *win32_path,
				char *full_win32_path, DWORD &devn, int &unit,
				unsigned *flags)
{
  int src_path_len = strlen (src_path);
  int trailing_slash_p = (src_path_len > 1
			  && SLASH_P (src_path[src_path_len - 1]));
  MALLOC_CHECK;
  int isrelpath;
  unsigned dummy_flags;

  devn = FH_BAD;
  unit = 0;

  if (!flags)
    flags = &dummy_flags;

  *flags = 0;
  debug_printf ("conv_to_win32_path (%s)", src_path);

  if (src_path_len >= MAX_PATH)
    {
      debug_printf ("ENAMETOOLONG = conv_to_win32_path (%s)", src_path);
      return ENAMETOOLONG;
    }

  int i, rc;
  char *dst = NULL;
  mount_item *mi = NULL;	/* initialized to avoid compiler warning */
  char pathbuf[MAX_PATH];

  /* Determine where the destination should be placed. */
  if (full_win32_path != NULL)
    dst = full_win32_path;
  else if (win32_path != NULL)
    dst = win32_path;

  if (dst == NULL)
    goto out;		/* Sanity check. */

  /* An MS-DOS spec has either a : or a \.  If this is found, short
     circuit most of the rest of this function. */
  if (strpbrk (src_path, ":\\") != NULL)
    {
      debug_printf ("%s already win32", src_path);
      rc = normalize_win32_path (src_path, dst);
      if (rc)
	{
	  debug_printf ("normalize_win32_path failed, rc %d", rc);
	  return rc;
	}
      isrelpath = !isabspath (src_path);
      *flags = set_flags_from_win32_path (dst);
      if (myself->rootlen && dst[0] && dst[1] == ':')
	{
	  char posix_path[MAX_PATH + 1];

	  rc = cygwin_shared->mount.conv_to_posix_path (dst, posix_path, 0);
	  if (rc)
	    {
	      debug_printf ("conv_to_posix_path failed, rc %d", rc);
	      return rc;
	    }
	  if (!ischrootpath (posix_path))
	    {
	      debug_printf ("ischrootpath failed");
	      return ENOENT;
	    }
	}
      goto fillin;
    }

  /* Normalize the path, taking out ../../ stuff, we need to do this
     so that we can move from one mounted directory to another with relative
     stuff.

     eg mounting c:/foo /foo
     d:/bar /bar

     cd /bar
     ls ../foo

     should look in c:/foo, not d:/foo.

     We do this by first getting an absolute UNIX-style path and then
     converting it to a DOS-style path, looking up the appropriate drive
     in the mount table.  */

  rc = normalize_posix_path (src_path, pathbuf);

  if (rc)
    {
      debug_printf ("%d = conv_to_win32_path (%s)", rc, src_path);
      *flags = 0;
      return rc;
    }

  isrelpath = !isslash (*src_path);

  /* See if this is a cygwin "device" */
  if (win32_device_name (pathbuf, dst, devn, unit))
    {
      *flags = MOUNT_BINARY;	/* FIXME: Is this a sensible default for devices? */
      goto fillin;
    }

  /* Check if the cygdrive prefix was specified.  If so, just strip
     off the prefix and transform it into an MS-DOS path. */
  MALLOC_CHECK;
  if (iscygdrive_device (pathbuf))
    {
      if (!cygdrive_win32_path (pathbuf, dst, trailing_slash_p))
	return ENOENT;
      *flags = cygdrive_flags;
      goto fillin;
    }

  /* Check the mount table for prefix matches. */
  for (i = 0; i < nmounts; i++)
    {
      mi = mount + posix_sorted[i];
      if (path_prefix_p (mi->posix_path, pathbuf, mi->posix_pathlen))
	break;
    }

  if (i >= nmounts)
    {
      if (slash_drive_prefix_p (pathbuf))
	slash_drive_to_win32_path (pathbuf, dst, trailing_slash_p);
      else
	backslashify (pathbuf, dst, trailing_slash_p);	/* just convert */
      *flags = 0;
    }
  else
    {
      int n = mi->native_pathlen;
      memcpy (dst, mi->native_path, n + 1);
      char *p = pathbuf + mi->posix_pathlen;
      if (!trailing_slash_p && !*p)
	{
	  if (isdrive (dst) && !dst[2])
	    dst[n++] = '\\';
	  dst[n] = '\0';
	}
      else
	{
	  /* Do not add trailing \ to UNC device names like \\.\a: */
	  if (*p != '/' &&  /* FIXME: this test seems wrong. */
	     (strncmp (mi->native_path, "\\\\.\\", 4) != 0 ||
	       strncmp (mi->native_path + 4, "UNC\\", 4) == 0))
	    dst[n++] = '\\';
	  strcpy (dst + n, p);
	}
      backslashify (dst, dst, trailing_slash_p);
      *flags = mi->flags;
    }

fillin:
  /* Compute relative path if asked to and able to.  */
  if (win32_path == NULL)
    /* nothing to do */;
  else if (isrelpath)
    {
      char cwd_win32[MAX_PATH];
      if (!cygcwd.get (cwd_win32, 0))
	return get_errno ();
      unsigned cwdlen = strlen (cwd_win32);
      if (!path_prefix_p (cwd_win32, dst, cwdlen))
	strcpy (win32_path, dst);
      else
	{
	  size_t n = strlen (dst);
	  if (n < cwdlen)
	    strcpy (win32_path, dst);
	  else
	    {
	      if (n == cwdlen)
		dst += cwdlen;
	      else
		dst += isdirsep (cwd_win32[cwdlen - 1]) ? cwdlen : cwdlen + 1;

	      memmove (win32_path, dst, strlen (dst) + 1);
	      if (!*win32_path)
		{
		  strcpy (win32_path, ".");
		  if (trailing_slash_p)
		    strcat (win32_path, "\\");
		}
	    }
	}
    }
  else if (win32_path != dst)
    strcpy (win32_path, dst);

out:
  MALLOC_CHECK;
  debug_printf ("%s(rel), %s(abs) %p(flags) = conv_to_win32_path (%s)",
		win32_path, full_win32_path, *flags,
		src_path);
  return 0;
}

/* Convert PATH (for which slash_drive_prefix_p returns 1) to WIN32 form.  */

void
mount_info::slash_drive_to_win32_path (const char *path, char *buf,
				       int trailing_slash_p)
{
  buf[0] = path[2];
  buf[1] = ':';
  if (path[3] == '0')
    strcpy (buf + 2, "\\");
  else
    backslashify (path + 3, buf + 2, trailing_slash_p);
}

/* cygdrive_posix_path: Build POSIX path used as the
   mount point for cygdrives created when there is no other way to
   obtain a POSIX path from a Win32 one. */

void
mount_info::cygdrive_posix_path (const char *src, char *dst, int trailing_slash_p)
{
  int len = cygdrive_len;

  memcpy (dst, cygdrive, len + 1);

  /* Now finish the path off with the drive letter to be used.
     The cygdrive prefix always ends with a trailing slash so
     the drive letter is added after the path. */
  dst[len++] = tolower (src[0]);
  if (!src[2] || (SLASH_P (src[2]) && !src[3]))
    dst[len++] = '\000';
  else
    {
      dst[len++] = '/';
      strcpy (dst + len, src + 3);
    }
  slashify (dst, dst, trailing_slash_p);
}

int
mount_info::cygdrive_win32_path (const char *src, char *dst, int trailing_slash_p)
{
  const char *p = src + cygdrive_len;
  if (!isalpha (*p) || (!isdirsep (p[1]) && p[1]))
    return 0;
  dst[0] = *p;
  dst[1] = ':';
  strcpy (dst + 2, p + 1);
  backslashify (dst, dst, trailing_slash_p || !dst[2]);
  debug_printf ("src '%s', dst '%s'", src, dst);
  return 1;
}

/* conv_to_posix_path: Ensure src_path is a POSIX path.

   The result is zero for success, or an errno value.
   posix_path must have sufficient space (i.e. MAX_PATH bytes).
   If keep_rel_p is non-zero, relative paths stay that way.  */

int
mount_info::conv_to_posix_path (const char *src_path, char *posix_path,
				int keep_rel_p)
{
  int src_path_len = strlen (src_path);
  int relative_path_p = !isabspath (src_path);
  int trailing_slash_p;

  if (src_path_len <= 1)
    trailing_slash_p = 0;
  else
    {
      const char *lastchar = src_path + src_path_len - 1;
      trailing_slash_p = SLASH_P (*lastchar) && lastchar[-1] != ':';
    }

  debug_printf ("conv_to_posix_path (%s, %s, %s)", src_path,
		keep_rel_p ? "keep-rel" : "no-keep-rel",
		trailing_slash_p ? "add-slash" : "no-add-slash");
  MALLOC_CHECK;

  if (src_path_len >= MAX_PATH)
    {
      debug_printf ("ENAMETOOLONG");
      return ENAMETOOLONG;
    }

  /* FIXME: For now, if the path is relative and it's supposed to stay
     that way, skip mount table processing. */

  if (keep_rel_p && relative_path_p)
    {
      slashify (src_path, posix_path, 0);
      debug_printf ("%s = conv_to_posix_path (%s)", posix_path, src_path);
      return 0;
    }

  char pathbuf[MAX_PATH];
  int rc = normalize_win32_path (src_path, pathbuf);
  if (rc != 0)
    {
      debug_printf ("%d = conv_to_posix_path (%s)", rc, src_path);
      return rc;
    }

  int pathbuflen = strlen (pathbuf);
  for (int i = 0; i < nmounts; ++i)
    {
      mount_item &mi = mount[native_sorted[i]];
      if (! path_prefix_p (mi.native_path, pathbuf, mi.native_pathlen))
	continue;

      /* SRC_PATH is in the mount table. */
      int nextchar;
      const char *p = pathbuf + mi.native_pathlen;

      if (!*p || !p[1])
	nextchar = 0;
      else if (isdirsep (*p))
	nextchar = -1;
      else
	nextchar = 1;

      int addslash = nextchar > 0 ? 1 : 0;
      if ((mi.posix_pathlen + (pathbuflen - mi.native_pathlen) + addslash) >= MAX_PATH)
	return ENAMETOOLONG;
      strcpy (posix_path, mi.posix_path);
      if (addslash)
	strcat (posix_path, "/");
      if (nextchar)
	slashify (p,
		  posix_path + addslash + (mi.posix_pathlen == 1 ? 0 : mi.posix_pathlen),
		  trailing_slash_p);
      goto out;
    }

  /* Not in the database.  This should [theoretically] only happen if either
     the path begins with //, or / isn't mounted, or the path has a drive
     letter not covered by the mount table.  If it's a relative path then the
     caller must want an absolute path (otherwise we would have returned
     above).  So we always return an absolute path at this point. */
  if (isdrive (pathbuf))
    cygdrive_posix_path (pathbuf, posix_path, trailing_slash_p);
  else
    {
      /* The use of src_path and not pathbuf here is intentional.
	 We couldn't translate the path, so just ensure no \'s are present. */
      slashify (src_path, posix_path, trailing_slash_p);
    }

out:
  debug_printf ("%s = conv_to_posix_path (%s)", posix_path, src_path);
  MALLOC_CHECK;
  return 0;
}

/* Return flags associated with a mount point given the win32 path. */

unsigned
mount_info::set_flags_from_win32_path (const char *p)
{
  for (int i = 0; i < nmounts; i++)
    {
      mount_item &mi = mount[native_sorted[i]];
      if (path_prefix_p (mi.native_path, p, mi.native_pathlen))
	return mi.flags;
    }
  return 0;
}

/* read_mounts: Given a specific regkey, read mounts from under its
   key. */

void
mount_info::read_mounts (reg_key& r)
{
  char posix_path[MAX_PATH];
  HKEY key = r.get_key ();
  DWORD i, posix_path_size;
  int found_cygdrive = FALSE;

  /* Loop through subkeys */
  /* FIXME: we would like to not check MAX_MOUNTS but the heap in the
     shared area is currently statically allocated so we can't have an
     arbitrarily large number of mounts. */
  for (DWORD i = 0; ; i++)
    {
      char native_path[MAX_PATH];
      int mount_flags;

      posix_path_size = MAX_PATH;
      /* FIXME: if maximum posix_path_size is 256, we're going to
	 run into problems if we ever try to store a mount point that's
	 over 256 but is under MAX_PATH! */
      LONG err = RegEnumKeyEx (key, i, posix_path, &posix_path_size, NULL,
			  NULL, NULL, NULL);

      if (err == ERROR_NO_MORE_ITEMS)
	break;
      else if (err != ERROR_SUCCESS)
	{
	  debug_printf ("RegEnumKeyEx failed, error %d!\n", err);
	  break;
	}

      if (iscygdrive (posix_path))
	{
	  found_cygdrive = TRUE;
	  continue;
	}

      /* Get a reg_key based on i. */
      reg_key subkey = reg_key (key, KEY_READ, posix_path, NULL);

      /* Fetch info from the subkey. */
      subkey.get_string ("native", native_path, sizeof (native_path), "");
      mount_flags = subkey.get_int ("flags", 0);

      /* Add mount_item corresponding to registry mount point. */
      int res = cygwin_shared->mount.add_item (native_path, posix_path, mount_flags, FALSE);
      if (res && get_errno () == EMFILE)
	break; /* The number of entries exceeds MAX_MOUNTS */
    }

  if (!found_cygdrive)
    return;

loop:
  for (i = 0; ;i++)
    {
      posix_path_size = MAX_PATH;
      LONG err = RegEnumKeyEx (key, i, posix_path, &posix_path_size, NULL,
			  NULL, NULL, NULL);

      if (err != ERROR_SUCCESS)
	break;

      if (iscygdrive (posix_path))
	{
	  /* This shouldn't be in the mount table. */
	  (void) r.kill (posix_path);
	  goto loop;
	}
    }
}

/* from_registry: Build the entire mount table from the registry.  Also,
   read in cygdrive-related information from its registry location. */

void
mount_info::from_registry ()
{
  /* Use current mount areas if either user or system mount areas
     already exist.  Otherwise, import old mounts. */

  reg_key r;

  /* Retrieve cygdrive-related information. */
  read_cygdrive_info_from_registry ();

  nmounts = 0;

  /* First read mounts from user's table. */
  read_mounts (r);

  /* Then read mounts from system-wide mount table. */
  reg_key r1 (HKEY_LOCAL_MACHINE, KEY_READ, "SOFTWARE",
	      CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
	      CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
	      CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
	      NULL);
  read_mounts (r1);

  /* If we had to create both user and system mount areas, import
     old mounts. */
  if (had_to_create_mount_areas == 2)
    import_v1_mounts ();
}

/* add_reg_mount: Add mount item to registry.  Return zero on success,
   non-zero on failure. */
/* FIXME: Need a mutex to avoid collisions with other tasks. */

int
mount_info::add_reg_mount (const char * native_path, const char * posix_path, unsigned mountflags)
{
  /* Add the mount to the right registry location, depending on
     whether MOUNT_SYSTEM is set in the mount flags. */
  if (!(mountflags & MOUNT_SYSTEM)) /* current_user mount */
    {
      /* reg_key for user mounts in HKEY_CURRENT_USER. */
      reg_key reg_user;

      /* Start by deleting existing mount if one exists. */
      reg_user.kill (posix_path);

      /* Create the new mount. */
      reg_key subkey = reg_key (reg_user.get_key (),
				KEY_ALL_ACCESS,
				posix_path, NULL);
      subkey.set_string ("native", native_path);
      subkey.set_int ("flags", mountflags);
    }
  else /* local_machine mount */
    {
      /* reg_key for system mounts in HKEY_LOCAL_MACHINE. */
      reg_key reg_sys (HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS, "SOFTWARE",
		       CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
		       CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
		       CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
		       NULL);

      if (reg_sys.get_key () == INVALID_HANDLE_VALUE)
	{
	  set_errno (EACCES);
	  return -1;
	}

      /* Start by deleting existing mount if one exists. */
      reg_sys.kill (posix_path);

      /* Create the new mount. */
      reg_key subkey = reg_key (reg_sys.get_key (),
				KEY_ALL_ACCESS,
				posix_path, NULL);
      subkey.set_string ("native", native_path);
      subkey.set_int ("flags", mountflags);
    }

  return 0; /* Success! */
}

/* del_reg_mount: delete mount item from registry indicated in flags.
   Return zero on success, non-zero on failure.*/
/* FIXME: Need a mutex to avoid collisions with other tasks. */

int
mount_info::del_reg_mount (const char * posix_path, unsigned flags)
{
  int killres;

  if ((flags & MOUNT_SYSTEM) == 0)	/* Delete from user registry */
    {
      reg_key reg_user (KEY_ALL_ACCESS,
			CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
      killres = reg_user.kill (posix_path);
    }
  else					/* Delete from system registry */
    {
      reg_key reg_sys (HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS, "SOFTWARE",
		       CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
		       CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
		       CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
		       NULL);

      if (reg_sys.get_key () == INVALID_HANDLE_VALUE)
	{
	  set_errno (EACCES);
	  return -1;
	}

      killres = reg_sys.kill (posix_path);
    }

  if (killres != ERROR_SUCCESS)
    {
      __seterrno_from_win_error (killres);
      return -1;
    }

  return 0; /* Success! */
}

/* read_cygdrive_info_from_registry: Read the default prefix and flags
   to use when creating cygdrives from the special user registry
   location used to store cygdrive information. */

void
mount_info::read_cygdrive_info_from_registry ()
{
  /* reg_key for user path prefix in HKEY_CURRENT_USER. */
  reg_key r;

  if (r.get_string ("cygdrive prefix", cygdrive, sizeof (cygdrive), "") != 0)
    {
      /* Didn't find the user path prefix so check the system path prefix. */

      /* reg_key for system path prefix in HKEY_LOCAL_MACHINE.  */
      reg_key r2 (HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS, "SOFTWARE",
		 CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
		 CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
		 CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
		 NULL);

    if (r2.get_string ("cygdrive prefix", cygdrive, sizeof (cygdrive), "") != 0)
      {
	/* Didn't find either so write the default to the registry and use it.
	   NOTE: We are writing and using the user path prefix.  */
	write_cygdrive_info_to_registry ("/cygdrive", MOUNT_AUTO);
      }
    else
      {
	/* Fetch system cygdrive_flags from registry; returns MOUNT_AUTO on
	   error. */
	cygdrive_flags = r2.get_int ("cygdrive flags", MOUNT_AUTO);
	slashify (cygdrive, cygdrive, 1);
	cygdrive_len = strlen(cygdrive);
      }
    }
  else
    {
      /* Fetch user cygdrive_flags from registry; returns MOUNT_AUTO on
	 error. */
      cygdrive_flags = r.get_int ("cygdrive flags", MOUNT_AUTO);
      slashify (cygdrive, cygdrive, 1);
      cygdrive_len = strlen(cygdrive);
    }
}

/* write_cygdrive_info_to_registry: Write the default prefix and flags
   to use when creating cygdrives to the special user registry
   location used to store cygdrive information. */

int
mount_info::write_cygdrive_info_to_registry (const char *cygdrive_prefix, unsigned flags)
{
  /* Determine whether to modify user or system cygdrive path prefix. */
  HKEY top = (flags & MOUNT_SYSTEM) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

  /* reg_key for user path prefix in HKEY_CURRENT_USER or system path prefix in
     HKEY_LOCAL_MACHINE.  */
  reg_key r (top, KEY_ALL_ACCESS, "SOFTWARE",
	     CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
	     CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
	     CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
	     NULL);

  /* Verify cygdrive prefix starts with a forward slash and if there's
     another character, it's not a slash. */
  if ((cygdrive_prefix == NULL) || (*cygdrive_prefix == 0) ||
      (!isslash (cygdrive_prefix[0])) ||
      ((cygdrive_prefix[1] != '\0') && (isslash (cygdrive_prefix[1]))))
      {
	set_errno (EINVAL);
	return -1;
      }

  char hold_cygdrive_prefix[strlen (cygdrive_prefix) + 1];
  /* Ensure that there is never a final slash */
  nofinalslash (cygdrive_prefix, hold_cygdrive_prefix);

  r.set_string ("cygdrive prefix", hold_cygdrive_prefix);
  r.set_int ("cygdrive flags", flags);

  /* This also needs to go in the in-memory copy of "cygdrive", but only if
     appropriate:
       1. setting user path prefix, or
       2. overwriting (a previous) system path prefix */
  if ((flags & MOUNT_SYSTEM) == 0 ||
      (cygwin_shared->mount.cygdrive_flags & MOUNT_SYSTEM) != 0)
    {
      slashify (cygdrive_prefix, cygwin_shared->mount.cygdrive, 1);
      cygwin_shared->mount.cygdrive_flags = flags;
      cygwin_shared->mount.cygdrive_len = strlen(cygwin_shared->mount.cygdrive);
    }

  return 0;
}

int
mount_info::remove_cygdrive_info_from_registry (const char *cygdrive_prefix, unsigned flags)
{
  /* Determine whether to modify user or system cygdrive path prefix. */
  HKEY top = (flags & MOUNT_SYSTEM) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

  /* reg_key for user path prefix in HKEY_CURRENT_USER or system path prefix in
     HKEY_LOCAL_MACHINE.  */
  reg_key r (top, KEY_ALL_ACCESS, "SOFTWARE",
	     CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
	     CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
	     CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
	     NULL);

  /* Delete cygdrive prefix and flags. */
  int res = r.killvalue ("cygdrive prefix");
  int res2 = r.killvalue ("cygdrive flags");

  /* Reinitialize the cygdrive path prefix to reflect to removal from the
     registry. */
  read_cygdrive_info_from_registry ();

  return (res != ERROR_SUCCESS) ? res : res2;
}

int
mount_info::get_cygdrive_prefixes (char *user, char *system)
{
  /* Get the user path prefix from HKEY_CURRENT_USER. */
  reg_key r;
  int res = r.get_string ("cygdrive prefix", user, MAX_PATH, "");

  /* Get the system path prefix from HKEY_LOCAL_MACHINE. */
  reg_key r2 (HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS, "SOFTWARE",
	      CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
	      CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
	      CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
	      NULL);
  int res2 = r2.get_string ("cygdrive prefix", system, MAX_PATH, "");

  return (res != ERROR_SUCCESS) ? res : res2;
}

struct mntent *
mount_info::getmntent (int x)
{
  if (x < 0 || x >= nmounts)
    return NULL;

  return mount[native_sorted[x]].getmntent ();
}

static mount_item *mounts_for_sort;

/* sort_by_posix_name: qsort callback to sort the mount entries.  Sort
   user mounts ahead of system mounts to the same POSIX path. */
/* FIXME: should the user should be able to choose whether to
   prefer user or system mounts??? */
static int
sort_by_posix_name (const void *a, const void *b)
{
  mount_item *ap = mounts_for_sort + (*((int*) a));
  mount_item *bp = mounts_for_sort + (*((int*) b));

  /* Base weighting on longest posix path first so that the most
     obvious path will be chosen. */
  size_t alen = strlen (ap->posix_path);
  size_t blen = strlen (bp->posix_path);

  int res = blen - alen;

  if (res)
    return res;		/* Path lengths differed */

  /* The two paths were the same length, so just determine normal
     lexical sorted order. */
  res = strcmp (ap->posix_path, bp->posix_path);

  if (res == 0)
   {
     /* need to select between user and system mount to same POSIX path */
     if ((bp->flags & MOUNT_SYSTEM) == 0) /* user mount */
      return 1;
     else
      return -1;
   }

  return res;
}

/* sort_by_native_name: qsort callback to sort the mount entries.  Sort
   user mounts ahead of system mounts to the same POSIX path. */
/* FIXME: should the user should be able to choose whether to
   prefer user or system mounts??? */
static int
sort_by_native_name (const void *a, const void *b)
{
  mount_item *ap = mounts_for_sort + (*((int*) a));
  mount_item *bp = mounts_for_sort + (*((int*) b));

  /* Base weighting on longest win32 path first so that the most
     obvious path will be chosen. */
  size_t alen = strlen (ap->native_path);
  size_t blen = strlen (bp->native_path);

  int res = blen - alen;

  if (res)
    return res;		/* Path lengths differed */

  /* The two paths were the same length, so just determine normal
     lexical sorted order. */
  res = strcmp (ap->native_path, bp->native_path);

  if (res == 0)
   {
     /* need to select between user and system mount to same POSIX path */
     if ((bp->flags & MOUNT_SYSTEM) == 0) /* user mount */
      return 1;
     else
      return -1;
   }

  return res;
}

void
mount_info::sort ()
{
  for (int i = 0; i < nmounts; i++)
    native_sorted[i] = posix_sorted[i] = i;
  /* Sort them into reverse length order, otherwise we won't
     be able to look for /foo in /.  */
  mounts_for_sort = mount;	/* ouch. */
  qsort (posix_sorted, nmounts, sizeof (posix_sorted[0]), sort_by_posix_name);
  qsort (native_sorted, nmounts, sizeof (native_sorted[0]), sort_by_native_name);
}

/* Add an entry to the mount table.
   Returns 0 on success, -1 on failure and errno is set.

   This is where all argument validation is done.  It may not make sense to
   do this when called internally, but it's cleaner to keep it all here.  */

int
mount_info::add_item (const char *native, const char *posix, unsigned mountflags, int reg_p)
{
  /* Something's wrong if either path is NULL or empty, or if it's
     not a UNC or absolute path. */

  if ((native == NULL) || (*native == 0) ||
      (posix == NULL) || (*posix == 0) ||
      (!slash_unc_prefix_p (native) && !isabspath (native)))
    {
      set_errno (EINVAL);
      return -1;
    }

  /* Make sure both paths do not end in /. */
  char nativetmp[MAX_PATH];
  char posixtmp[MAX_PATH];

  if (slash_drive_prefix_p (native))
    slash_drive_to_win32_path (native, nativetmp, 0);
  else
    backslashify (native, nativetmp, 0);
  nofinalslash (nativetmp, nativetmp);

  slashify (posix, posixtmp, 0);
  nofinalslash (posixtmp, posixtmp);

  debug_printf ("%s[%s], %s[%s], %p",
		native, nativetmp, posix, posixtmp, mountflags);

  /* Duplicate /'s in path are an error. */
  for (char *p = posixtmp + 1; *p; ++p)
    {
      if (p[-1] == '/' && p[0] == '/')
	{
	  set_errno (EINVAL);
	  return -1;
	}
    }

  /* Write over an existing mount item with the same POSIX path if
     it exists and is from the same registry area. */
  int i;
  for (i = 0; i < nmounts; i++)
    {
      if (strcasematch (mount[i].posix_path, posixtmp) &&
	  (mount[i].flags & MOUNT_SYSTEM) == (mountflags & MOUNT_SYSTEM))
	break;
    }

  if (i == nmounts)
    {
      if (nmounts < MAX_MOUNTS)
	i = nmounts++;
      else
	{
	  set_errno (EMFILE);
	  return -1;
	}
    }

  if (reg_p && add_reg_mount (nativetmp, posixtmp, mountflags))
    return -1;

  mount[i].init (nativetmp, posixtmp, mountflags);
  sort ();

  return 0;
}

/* Delete a mount table entry where path is either a Win32 or POSIX
   path. Since the mount table is really just a table of aliases,
   deleting / is ok (although running without a slash mount is
   strongly discouraged because some programs may run erratically
   without one).  If MOUNT_SYSTEM is set in flags, remove from system
   registry, otherwise remove the user registry mount.
*/

int
mount_info::del_item (const char *path, unsigned flags, int reg_p)
{
  char pathtmp[MAX_PATH];
  int posix_path_p = FALSE;

  /* Something's wrong if path is NULL or empty. */
  if (path == NULL || *path == 0 || !isabspath (path))
    {
      set_errno (EINVAL);
      return -1;
    }

  if (slash_drive_prefix_p (path))
      slash_drive_to_win32_path (path, pathtmp, 0);
  else if (slash_unc_prefix_p (path) || strpbrk (path, ":\\"))
      backslashify (path, pathtmp, 0);
  else
    {
      slashify (path, pathtmp, 0);
      posix_path_p = TRUE;
    }
  nofinalslash (pathtmp, pathtmp);

  if (reg_p && posix_path_p &&
      del_reg_mount (pathtmp, flags) &&
      del_reg_mount (path, flags)) /* for old irregular entries */
    return -1;

  for (int i = 0; i < nmounts; i++)
    {
      int ent = native_sorted[i]; /* in the same order as getmntent() */
      if (((posix_path_p)
	   ? strcasematch (mount[ent].posix_path, pathtmp)
	   : strcasematch (mount[ent].native_path, pathtmp)) &&
	  (mount[ent].flags & MOUNT_SYSTEM) == (flags & MOUNT_SYSTEM))
	{
	  if (!posix_path_p &&
	      reg_p && del_reg_mount (mount[ent].posix_path, flags))
	    return -1;

	  nmounts--; /* One less mount table entry */
	  /* Fill in the hole if not at the end of the table */
	  if (ent < nmounts)
	    memmove (mount + ent, mount + ent + 1,
		     sizeof (mount[ent]) * (nmounts - ent));
	  sort (); /* Resort the table */
	  return 0;
	}
    }
  set_errno (EINVAL);
  return -1;
}

/* read_v1_mounts: Given a reg_key to an old mount table registry area,
   read in the mounts.  The "which" arg contains zero if we're reading
   the user area and MOUNT_SYSTEM if we're reading the system area.
   This way we can store the mounts read in the appropriate place when
   they are written back to the new registry layout. */

void
mount_info::read_v1_mounts (reg_key r, unsigned which)
{
  unsigned mountflags = 0;

  /* MAX_MOUNTS was 30 when we stopped using the v1 layout */
  for (int i = 0; i < 30; i++)
    {
      char key_name[10];
      char win32path[MAX_PATH];
      char unixpath[MAX_PATH];

      __small_sprintf (key_name, "%02x", i);

      reg_key k (r.get_key (), KEY_ALL_ACCESS, key_name, NULL);

      /* The registry names are historical but useful so are left alone.  */
      k.get_string ("native", win32path, sizeof (win32path), "");
      k.get_string ("unix", unixpath, sizeof (unixpath), "");

      /* Does this entry contain something?  */
      if (*win32path != 0)
	{
	  mountflags = 0;

	  if (k.get_int ("fbinary", 0))
	    mountflags |= MOUNT_BINARY;

	  /* Or in zero or MOUNT_SYSTEM depending on which table
	     we're reading. */
	  mountflags |= which;

	  int res = cygwin_shared->mount.add_item (win32path, unixpath, mountflags, TRUE);
	  if (res && get_errno () == EMFILE)
	    break; /* The number of entries exceeds MAX_MOUNTS */
	}
    }
}

/* import_v1_mounts: If v1 mounts are present, load them and write
   the new entries to the new registry area. */

void
mount_info::import_v1_mounts ()
{
  reg_key r (HKEY_CURRENT_USER, KEY_ALL_ACCESS,
	     "SOFTWARE",
	     "Cygnus Solutions",
	     "CYGWIN.DLL setup",
	     "b15.0",
	     "mounts",
	     NULL);

  nmounts = 0;

  /* First read mounts from user's table. */
  read_v1_mounts (r, 0);

  /* Then read mounts from system-wide mount table. */
  reg_key r1 (HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS,
	      "SOFTWARE",
	      "Cygnus Solutions",
	      "CYGWIN.DLL setup",
	      "b15.0",
	      "mounts",
	      NULL);
  read_v1_mounts (r1, MOUNT_SYSTEM);
}

/************************* mount_item class ****************************/

struct mntent *
mount_item::getmntent ()
{
#ifdef _MT_SAFE
  struct mntent &ret=_reent_winsup()->_ret;
#else
  static NO_COPY struct mntent ret;
#endif

  /* Pass back pointers to mount_info strings reserved for use by
     getmntent rather than pointers to strings in the internal mount
     table because the mount table might change, causing weird effects
     from the getmntent user's point of view. */

  strcpy (cygwin_shared->mount.mnt_fsname, native_path);
  ret.mnt_fsname = cygwin_shared->mount.mnt_fsname;
  strcpy (cygwin_shared->mount.mnt_dir, posix_path);
  ret.mnt_dir = cygwin_shared->mount.mnt_dir;

  if (!(flags & MOUNT_SYSTEM))		/* user mount */
    strcpy (cygwin_shared->mount.mnt_type, (char *) "user");
  else					/* system mount */
    strcpy (cygwin_shared->mount.mnt_type, (char *) "system");

  if ((flags & MOUNT_AUTO))		/* cygdrive */
    strcat (cygwin_shared->mount.mnt_type, (char *) ",auto");

  ret.mnt_type = cygwin_shared->mount.mnt_type;

  /* mnt_opts is a string that details mount params such as
     binary or textmode, or exec.  We don't print
     `silent' here; it's a magic internal thing. */

  if (! (flags & MOUNT_BINARY))
    strcpy (cygwin_shared->mount.mnt_opts, (char *) "textmode");
  else
    strcpy (cygwin_shared->mount.mnt_opts, (char *) "binmode");

  if (flags & MOUNT_CYGWIN_EXEC)
    strcat (cygwin_shared->mount.mnt_opts, (char *) ",cygexec");
  else if (flags & MOUNT_EXEC)
    strcat (cygwin_shared->mount.mnt_opts, (char *) ",exec");


  ret.mnt_opts = cygwin_shared->mount.mnt_opts;

  ret.mnt_freq = 1;
  ret.mnt_passno = 1;
  return &ret;
}

/* Fill in the fields of a mount table entry.  */

void
mount_item::init (const char *native, const char *posix, unsigned mountflags)
{
  strcpy ((char *) native_path, native);
  strcpy ((char *) posix_path, posix);

  native_pathlen = strlen (native_path);
  posix_pathlen = strlen (posix_path);

  flags = mountflags;
}

/********************** Mount System Calls **************************/

/* Mount table system calls.
   Note that these are exported to the application.  */

/* mount: Add a mount to the mount table in memory and to the registry
   that will cause paths under win32_path to be translated to paths
   under posix_path. */

extern "C"
int
mount (const char *win32_path, const char *posix_path, unsigned flags)
{
  int res = -1;

  if (flags & MOUNT_AUTO) /* normal mount */
    {
      /* When flags include MOUNT_AUTO, take this to mean that
	we actually want to change the cygdrive prefix and flags
	without actually mounting anything. */
      res = cygwin_shared->mount.write_cygdrive_info_to_registry (posix_path, flags);
      win32_path = NULL;
    }
  else
    {
      if (iscygdrive (posix_path))
	{
	  set_errno (EINVAL);
	  return res;	/* Don't try to add cygdrive prefix. */
	}

      res = cygwin_shared->mount.add_item (win32_path, posix_path, flags, TRUE);
    }

  syscall_printf ("%d = mount (%s, %s, %p)", res, win32_path, posix_path, flags);
  return res;
}

/* umount: The standard umount call only has a path parameter.  Since
   it is not possible for this call to specify whether to remove the
   mount from the user or global mount registry table, assume the user
   table. */

extern "C"
int
umount (const char *path)
{
  return cygwin_umount (path, 0);
}

/* cygwin_umount: This is like umount but takes an additional flags
   parameter that specifies whether to umount from the user or system-wide
   registry area. */

extern "C"
int
cygwin_umount (const char *path, unsigned flags)
{
  int res = -1;

  if (flags & MOUNT_AUTO)
    {
      /* When flags include MOUNT_AUTO, take this to mean that we actually want
	 to remove the cygdrive prefix and flags without actually unmounting
	 anything. */
      res = cygwin_shared->mount.remove_cygdrive_info_from_registry (path, flags);
    }
  else
    {
      res = cygwin_shared->mount.del_item (path, flags, TRUE);
    }

  syscall_printf ("%d = cygwin_umount (%s, %d)", res,  path, flags);
  return res;
}

#ifdef _MT_SAFE
#define iteration _reent_winsup()->_iteration
#else
static int iteration;
#endif

extern "C"
FILE *
setmntent (const char *filep, const char *)
{
  iteration = 0;
  return (FILE *) filep;
}

extern "C"
struct mntent *
getmntent (FILE *)
{
  return cygwin_shared->mount.getmntent (iteration++);
}

extern "C"
int
endmntent (FILE *)
{
  return 1;
}

/********************** Symbolic Link Support **************************/

/* Create a symlink from FROMPATH to TOPATH. */

extern "C"
int
symlink (const char *topath, const char *frompath)
{
  HANDLE h;
  int res = -1;

  path_conv win32_path (frompath, PC_SYM_NOFOLLOW);
  if (win32_path.error)
    {
      set_errno (win32_path.error);
      goto done;
    }

  syscall_printf ("symlink (%s, %s)", topath, win32_path.get_win32 ());

  if (topath[0] == 0)
    {
      set_errno (EINVAL);
      goto done;
    }
  if (strlen (topath) >= MAX_PATH)
    {
      set_errno (ENAMETOOLONG);
      goto done;
    }

  if (win32_path.is_device () ||
      win32_path.file_attributes () != (DWORD) -1)
    {
      set_errno (EEXIST);
      goto done;
    }

  h = CreateFileA(win32_path.get_win32 (), GENERIC_WRITE, 0, &sec_none_nih,
		  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
  if (h == INVALID_HANDLE_VALUE)
      __seterrno ();
  else
    {
      char buf[sizeof (SYMLINK_COOKIE) + MAX_PATH + 10];

      __small_sprintf (buf, "%s%s", SYMLINK_COOKIE, topath);
      DWORD len = strlen (buf) + 1;

      /* Note that the terminating nul is written.  */
      DWORD written;
      if (!WriteFile (h, buf, len, &written, NULL) || written != len)
	{
	  __seterrno ();
	  CloseHandle (h);
	  DeleteFileA (win32_path.get_win32 ());
	}
      else
	{
	  CloseHandle (h);
	  set_file_attribute (win32_path.has_acls (),
			      win32_path.get_win32 (),
			      S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO);
	  SetFileAttributesA (win32_path.get_win32 (), FILE_ATTRIBUTE_SYSTEM);
	  res = 0;
	}
    }

done:
  syscall_printf ("%d = symlink (%s, %s)", res, topath, frompath);
  return res;
}

static __inline char *
has_suffix (const char *path, const suffix_info *suffixes)
{
  char *ext = strrchr (path, '.');
  if (ext)
    for (const suffix_info *ex = suffixes; ex->name != NULL; ex++)
      if (strcasematch (ext, ex->name))
	return ext;
  return NULL;
}

static __inline__ int
next_suffix (char *ext_here, const suffix_info *&suffixes)
{
  if (!suffixes)
    return 1;

  while (suffixes && suffixes->name)
    if (!suffixes->addon)
      suffixes++;
    else
      {
	strcpy (ext_here, suffixes->name);
	suffixes++;
	return 1;
      }
  return 0;
}

/* Check if PATH is a symlink.  PATH must be a valid Win32 path name.

   If PATH is a symlink, put the value of the symlink--the file to
   which it points--into BUF.  The value stored in BUF is not
   necessarily null terminated.  BUFLEN is the length of BUF; only up
   to BUFLEN characters will be stored in BUF.  BUF may be NULL, in
   which case nothing will be stored.

   Set *SYML if PATH is a symlink.

   Set *EXEC if PATH appears to be executable.  This is an efficiency
   hack because we sometimes have to open the file anyhow.  *EXEC will
   not be set for every executable file.

   Return -1 on error, 0 if PATH is not a symlink, or the length
   stored into BUF if PATH is a symlink.  */

int
symlink_info::check (const char *in_path, const suffix_info *suffixes)
{
  HANDLE h;
  int res = 0;
  char extbuf[MAX_PATH + 5];
  const char *path = in_path;

  if (!suffixes)
    ext_here = NULL;
  else if ((known_suffix = has_suffix (in_path, suffixes)) != NULL)
    {
      suffixes = NULL;
      ext_here = NULL;
    }
  else
    {
      path = strcpy (extbuf, in_path);
      ext_here = strchr (path, '\0');
    }

  is_symlink = TRUE;

  error = 0;
  do
    {
      if (!next_suffix (ext_here, suffixes))
	break;
      error = 0;
      fileattr = GetFileAttributesA (path);
      if (fileattr == (DWORD) -1)
	{
	  /* The GetFileAttributesA call can fail for reasons that don't
	     matter, so we just return 0.  For example, getting the
	     attributes of \\HOST will typically fail.  */
	  debug_printf ("GetFileAttributesA (%s) failed", path);
	  error = geterrno_from_win_error (GetLastError (), EACCES);
	  continue;
	}

      /* Windows allows path\. even when `path' isn't a directory.
	 Detect this scenario and disallow it, since it is non-UNIX like.
	 FIXME: This code actually checks for things like foo/ and foo/..
	 even though those usages have already been (erroneously?) eaten
	 by cygwin_shared->mount.conv_to_win32_path in path_conv::check. */

      char *p = strrchr (path, '\\');
      if (p && !(fileattr & FILE_ATTRIBUTE_DIRECTORY) &&
	  (*++p == '\0' || (*p == '.' && (*++p == '\0' || (*p == '.' && p[1] == '\0')))))
	{
	  debug_printf ("%s is a non-directory", path);
	  error = ENOTDIR;
	  goto file_not_symlink;
	}

      /* A symlink will have the `system' file attribute. */
      /* Only files can be symlinks (which can be symlinks to directories). */
      if (!(pflags & PATH_SYMLINK) && !SYMLINKATTR (fileattr))
	goto file_not_symlink;

      /* Open the file.  */

      h = CreateFileA (path, GENERIC_READ, FILE_SHARE_READ, &sec_none_nih, OPEN_EXISTING,
		       FILE_ATTRIBUTE_NORMAL, 0);
      res = -1;
      if (h == INVALID_HANDLE_VALUE)
	goto file_not_symlink;
      else
	{
	  char cookie_buf[sizeof (SYMLINK_COOKIE) - 1];
	  DWORD got;

	  if (! ReadFile (h, cookie_buf, sizeof (cookie_buf), &got, 0))
	    error = EIO;
	  else if (got == sizeof (cookie_buf)
		   && memcmp (cookie_buf, SYMLINK_COOKIE,
			      sizeof (cookie_buf)) == 0)
	    {
	      /* It's a symlink.  */
	      pflags = PATH_SYMLINK;

	      res = ReadFile (h, contents, MAX_PATH + 1, &got, 0);
	      if (!res)
		error = EIO;
	      else
		{
		  /* Versions prior to b16 stored several trailing
		     NULs with the path (to fill the path out to 1024
		     chars).  Current versions only store one trailing
		     NUL.  The length returned is the path without
		     *any* trailing NULs.  We also have to handle (or
		     at least not die from) corrupted paths.  */
		  if (memchr (contents, 0, got) != NULL)
		    res = strlen (contents);
		  else
		    res = got;
		}
	    }
	  else if (got == sizeof (cookie_buf)
		   && memcmp (cookie_buf, SOCKET_COOKIE,
			      sizeof (cookie_buf)) == 0)
	    {
	      pflags |= PATH_SOCKET;
	      goto close_and_return;
	    }
	  else
	    {
	      /* Not a symlink, see if executable.  */
	      if (!(pflags & (PATH_EXEC | PATH_CYGWIN_EXEC)) && got >= 2 &&
		  ((cookie_buf[0] == '#' && cookie_buf[1] == '!') ||
		   (cookie_buf[0] == ':' && cookie_buf[1] == '\n')))
		pflags |= PATH_EXEC;
	    close_and_return:
	      CloseHandle (h);
	      goto file_not_symlink;
	    }
	}

      CloseHandle (h);
      break;
    }
  while (suffixes);
  goto out;

file_not_symlink:
  is_symlink = FALSE;
  syscall_printf ("not a symlink");
  res = 0;

out:
  syscall_printf ("%d = symlink.check (%s, %p) (%p)",
		  res, path, contents, pflags);

  return res;
}

/* readlink system call */

extern "C"
int
readlink (const char *path, char *buf, int buflen)
{
  extern suffix_info stat_suffixes[];

  if (buflen < 0)
    {
      set_errno (ENAMETOOLONG);
      return -1;
    }

  path_conv pathbuf (path, PC_SYM_CONTENTS, stat_suffixes);

  if (pathbuf.error)
    {
      set_errno (pathbuf.error);
      syscall_printf ("-1 = readlink (%s, %p, %d)", path, buf, buflen);
      return -1;
    }

  if (!pathbuf.issymlink ())
    {
      if (pathbuf.fileattr != (DWORD) -1)
	set_errno (EINVAL);
      return -1;
    }

  int len = min (buflen, (int) strlen (pathbuf.get_win32 ()));
  memcpy (buf, pathbuf.get_win32 (), len);

  /* errno set by symlink.check if error */
  return len;
}

/* Some programs rely on st_dev/st_ino being unique for each file.
   Hash the path name and hope for the best.  The hash arg is not
   always initialized to zero since readdir needs to compute the
   dirent ino_t based on a combination of the hash of the directory
   done during the opendir call and the hash or the filename within
   the directory.  FIXME: Not bullet-proof. */
/* Cygwin internal */

unsigned long __stdcall
hash_path_name (unsigned long hash, const char *name)
{
  if (!*name)
    return hash;

  /* Perform some initial permutations on the pathname if this is
     not "seeded" */
  if (!hash)
    {
      /* Simplistic handling of drives.  If there is a drive specified,
	 make sure that the initial letter is upper case.  If there is
	 no \ after the ':' assume access through the root directory
	 of that drive.
	 FIXME:  Should really honor MS-Windows convention of using
	 the environment to track current directory on various drives. */
      if (name[1] == ':')
	{
	  char *nn, *newname = (char *) alloca (strlen (name) + 2);
	  nn = strncpy (newname, name, 2);
	  if (islower (*nn))
	    *newname = toupper (*nn);
	  *(nn += 2) = '\0';
	  name += 2;
	  if (*name != '\\')
	    {
	      *nn = '\\';
	      *++nn = '\0';
	    }
	  strcpy (nn, name);
	  name = newname;
	  goto hashit;
	}

      /* Fill out the hashed path name with the current working directory if
	 this is not an absolute path and there is no pre-specified hash value.
	 Otherwise the inodes same will differ depending on whether a file is
	 referenced with an absolute value or relatively. */

      if (*name != '\\')
	{
	  hash = cygcwd.get_hash ();
	  if (name[0] == '.' && name[1] == '\0')
	    return hash;
	  hash = hash_path_name (hash, "\\");
	}
    }

hashit:
  /* Build up hash.  Ignore single trailing slash or \a\b\ != \a\b or
     \a\b\.  but allow a single \ if that's all there is. */
  do
    {
      hash += *name + (*name << 17);
      hash ^= hash >> 2;
    }
  while (*++name != '\0' &&
	 !(*name == '\\' && (!name[1] || (name[1] == '.' && !name[2]))));
  return hash;
}

char *
getcwd (char *buf, size_t ulen)
{
  char *res;
  char *usebuf, uselen;

  if (buf != NULL)
    {
      usebuf = buf;
      uselen = TRUE;
    }
  else
    {
      if (ulen >= 0)
	uselen = TRUE;
      else
	{
	  uselen = FALSE;
	  ulen = MAX_PATH + 1;
	}

      usebuf = (char *) malloc (ulen);
      usebuf [ulen - 1] = '\0';
    }

  res = cygcwd.get (usebuf, 1, 1, ulen);

  if (res && !uselen)
    usebuf = (char *) realloc (usebuf, strlen (usebuf) + 1);
  else if (!res && buf == NULL)
    free (usebuf);

  return res;
}

/* getwd: standards? */
extern "C"
char *
getwd (char *buf)
{
  return getcwd (buf, MAX_PATH);
}

/* chdir: POSIX 5.2.1.1 */
extern "C"
int
chdir (const char *dir)
{
  syscall_printf ("dir %s", dir);
  path_conv path (dir, PC_FULL | PC_SYM_FOLLOW);

  if (path.error)
    {
      set_errno (path.error);
      syscall_printf ("-1 = chdir (%s)", dir);
      return -1;
    }

  /* Look for trailing path component consisting entirely of dots.  This
     is needed only in case of chdir since Windows simply ignores count
     of dots > 2 here instead of returning an error code.  Counts of dots
     <= 2 are already eliminated by normalize_posix_path. */
  const char *p = strrchr (dir, '/');
  if (!p)
    p = dir;
  else
    p++;

  size_t len = strlen (p);
  if (len > 2 && strspn (p, ".") == len)
    {
      set_errno (ENOENT);
      return -1;
    }

  char *native_dir = path.get_win32 ();

  /* Check to see if path translates to something like C:.
     If it does, append a \ to the native directory specification to
     defeat the Windows 95 (i.e. MS-DOS) tendency of returning to
     the last directory visited on the given drive. */
  if (isdrive (native_dir) && !native_dir[2])
    {
      native_dir[2] = '\\';
      native_dir[3] = '\0';
    }
  int res = SetCurrentDirectoryA (native_dir) ? 0 : -1;
  if (res == -1)
    __seterrno ();
  else
    cygcwd.set (path, dir);

  /* Note that we're accessing cwd.posix without a lock here.  I didn't think
     it was worth locking just for strace. */
  syscall_printf ("%d = chdir() cygcwd.posix '%s' native '%s'", res,
		  cygcwd.posix, native_dir);
  return res;
}

/******************** Exported Path Routines *********************/

/* Cover functions to the path conversion routines.
   These are exported to the world as cygwin_foo by cygwin.din.  */

extern "C"
int
cygwin_conv_to_win32_path (const char *path, char *win32_path)
{
  path_conv p (path, PC_SYM_FOLLOW);
  if (p.error)
    {
      set_errno (p.error);
      return -1;
    }

  strcpy (win32_path, p.get_win32 ());
  return 0;
}

extern "C"
int
cygwin_conv_to_full_win32_path (const char *path, char *win32_path)
{
  path_conv p (path, PC_SYM_FOLLOW | PC_FULL);
  if (p.error)
    {
      set_errno (p.error);
      return -1;
    }

  strcpy (win32_path, p.get_win32 ());
  return 0;
}

/* This is exported to the world as cygwin_foo by cygwin.din.  */

extern "C"
int
cygwin_conv_to_posix_path (const char *path, char *posix_path)
{
  if (check_null_empty_path_errno (path))
    return -1;
  cygwin_shared->mount.conv_to_posix_path (path, posix_path, 1);
  return 0;
}

extern "C"
int
cygwin_conv_to_full_posix_path (const char *path, char *posix_path)
{
  if (check_null_empty_path_errno (path))
    return -1;
  cygwin_shared->mount.conv_to_posix_path (path, posix_path, 0);
  return 0;
}

/* The realpath function is supported on some UNIX systems.  */

extern "C"
char *
realpath (const char *path, char *resolved)
{
  int err;

  path_conv real_path (path, PC_SYM_FOLLOW | PC_FULL);

  if (real_path.error)
    err = real_path.error;
  else
    {
      err = cygwin_shared->mount.conv_to_posix_path (real_path.get_win32 (), resolved, 0);
      if (err == 0)
	return resolved;
    }

  /* FIXME: on error, we are supposed to put the name of the path
     component which could not be resolved into RESOLVED.  */
  resolved[0] = '\0';

  set_errno (err);
  return NULL;
}

/* Return non-zero if path is a POSIX path list.
   This is exported to the world as cygwin_foo by cygwin.din.

DOCTOOL-START
<sect1 id="add-func-cygwin-posix-path-list-p">
  <para>Rather than use a mode to say what the "proper" path list
  format is, we allow any, and give apps the tools they need to
  convert between the two.  If a ';' is present in the path list it's
  a Win32 path list.  Otherwise, if the first path begins with
  [letter]: (in which case it can be the only element since if it
  wasn't a ';' would be present) it's a Win32 path list.  Otherwise,
  it's a POSIX path list.</para>
</sect1>
DOCTOOL-END
  */

extern "C"
int
cygwin_posix_path_list_p (const char *path)
{
  int posix_p = ! (strchr (path, ';') || isdrive (path));
  return posix_p;
}

/* These are used for apps that need to convert env vars like PATH back and
   forth.  The conversion is a two step process.  First, an upper bound on the
   size of the buffer needed is computed.  Then the conversion is done.  This
   allows the caller to use alloca if it wants.  */

static int
conv_path_list_buf_size (const char *path_list, int to_posix_p)
{
  int i, num_elms, max_mount_path_len, size;
  const char *p;

  /* The theory is that an upper bound is
     current_size + (num_elms * max_mount_path_len)  */

  char delim = to_posix_p ? ';' : ':';
  p = path_list;
  for (num_elms = 1; (p = strchr (p, delim)) != NULL; ++num_elms)
    ++p;

  /* 7: strlen ("//c") + slop, a conservative initial value */
  for (max_mount_path_len = 7, i = 0; i < cygwin_shared->mount.nmounts; ++i)
    {
      int mount_len = (to_posix_p
		       ? cygwin_shared->mount.mount[i].posix_pathlen
		       : cygwin_shared->mount.mount[i].native_pathlen);
      if (max_mount_path_len < mount_len)
	max_mount_path_len = mount_len;
    }

  /* 100: slop */
  size = strlen (path_list) + (num_elms * max_mount_path_len) + 100;
  return size;
}

extern "C"
int
cygwin_win32_to_posix_path_list_buf_size (const char *path_list)
{
  return conv_path_list_buf_size (path_list, 1);
}

extern "C"
int
cygwin_posix_to_win32_path_list_buf_size (const char *path_list)
{
  return conv_path_list_buf_size (path_list, 0);
}

extern "C"
int
cygwin_win32_to_posix_path_list (const char *win32, char *posix)
{
  conv_path_list (win32, posix, 1);
  return 0;
}

extern "C"
int
cygwin_posix_to_win32_path_list (const char *posix, char *win32)
{
  conv_path_list (posix, win32, 0);
  return 0;
}

/* cygwin_split_path: Split a path into directory and file name parts.
   Buffers DIR and FILE are assumed to be big enough.

   Examples (path -> `dir' / `file'):
   / -> `/' / `'
   "" -> `.' / `'
   . -> `.' / `.' (FIXME: should this be `.' / `'?)
   .. -> `.' / `..' (FIXME: should this be `..' / `'?)
   foo -> `.' / `foo'
   foo/bar -> `foo' / `bar'
   foo/bar/ -> `foo' / `bar'
   /foo -> `/' / `foo'
   /foo/bar -> `/foo' / `bar'
   c: -> `c:/' / `'
   c:/ -> `c:/' / `'
   c:foo -> `c:/' / `foo'
   c:/foo -> `c:/' / `foo'
 */

extern "C"
void
cygwin_split_path (const char *path, char *dir, char *file)
{
  int dir_started_p = 0;

  /* Deal with drives.
     Remember that c:foo <==> c:/foo.  */
  if (isdrive (path))
    {
      *dir++ = *path++;
      *dir++ = *path++;
      *dir++ = '/';
      if (! *path)
	{
	  *dir = 0;
	  *file = 0;
	  return;
	}
      if (SLASH_P (*path))
	++path;
      dir_started_p = 1;
    }

  /* Determine if there are trailing slashes and "delete" them if present.
     We pretend as if they don't exist.  */
  const char *end = path + strlen (path);
  /* path + 1: keep leading slash.  */
  while (end > path + 1 && SLASH_P (end[-1]))
    --end;

  /* At this point, END points to one beyond the last character
     (with trailing slashes "deleted").  */

  /* Point LAST_SLASH at the last slash (duh...).  */
  const char *last_slash;
  for (last_slash = end - 1; last_slash >= path; --last_slash)
    if (SLASH_P (*last_slash))
      break;

  if (last_slash == path)
    {
      *dir++ = '/';
      *dir = 0;
    }
  else if (last_slash > path)
    {
      memcpy (dir, path, last_slash - path);
      dir[last_slash - path] = 0;
    }
  else
    {
      if (dir_started_p)
	; /* nothing to do */
      else
	*dir++ = '.';
      *dir = 0;
    }

  memcpy (file, last_slash + 1, end - last_slash - 1);
  file[end - last_slash - 1] = 0;
}

/********************** String Helper Functions ************************/

#define CHXOR ('a' ^ 'A')
#define ch_case_eq(ch1, ch2) \
    ({ \
      unsigned char x; \
      !((x = ((unsigned char)ch1 ^ (unsigned char)ch2)) && \
       (x != CHXOR || !isalpha (ch1))); \
    })

/* Return TRUE if two strings match up to length n */
int __stdcall
strncasematch (const char *s1, const char *s2, size_t n)
{
  if (s1 == s2)
    return 1;

  n++;
  while (--n && *s1)
    {
      if (!ch_case_eq (*s1, *s2))
	return 0;
      s1++; s2++;
    }
  return !n || *s2 == '\0';
}

/* Return TRUE if two strings match */
int __stdcall
strcasematch (const char *s1, const char *s2)
{
  if (s1 == s2)
    return 1;

  while (*s1)
    {
      if (!ch_case_eq (*s1, *s2))
	return 0;
      s1++; s2++;
    }
  return *s2 == '\0';
}

char * __stdcall
strcasestr (const char *searchee, const char *lookfor)
{
  if (*searchee == 0)
    {
      if (*lookfor)
	return NULL;
      return (char *) searchee;
    }

  while (*searchee)
    {
      int i = 0;
      while (1)
	{
	  if (lookfor[i] == 0)
	    return (char *) searchee;

	  if (!ch_case_eq (lookfor[i], searchee[i]))
	    break;
	  lookfor++;
	}
      searchee++;
    }

  return NULL;
}

int __stdcall
check_null_empty_path (const char *name)
{
  MEMORY_BASIC_INFORMATION m;
  if (!name || !VirtualQuery (name, &m, sizeof (m)) || (m.State != MEM_COMMIT))
    return EFAULT;

  if (!*name)
    return ENOENT;

  return 0;
}

/* Return the hash value for the current win32 value.
   This is used when constructing inodes. */
DWORD
cwdstuff::get_hash ()
{
  DWORD hashnow;
  lock->acquire ();
  hashnow = hash;
  lock->release ();
  return hashnow;
}

/* Initialize cygcwd 'muto' for serializing access to cwd info. */
void
cwdstuff::init ()
{
  lock = new_muto (FALSE, "cwd");
}

/* Called to fill in cwd values after an exec. */
void
cwdstuff::fixup_after_exec (char *win32_cwd, char *posix_cwd, DWORD hash_cwd)
{
  win32 = win32_cwd;
  posix = posix_cwd;
  hash = hash_cwd;
}

/* Get initial cwd.  Should only be called once in a
   process tree. */
bool
cwdstuff::get_initial ()
{
  if (win32)
    return 1;

  lock->acquire ();
  int i;
  DWORD len, dlen;
  for (i = 0, dlen = MAX_PATH, len = 0; i < 3; dlen *= 2, i++)
    {
      win32 = (char *) crealloc (win32, dlen + 2);
      if ((len = GetCurrentDirectoryA (dlen, win32)) < dlen)
	break;
    }

  if (len == 0)
    {
      __seterrno ();
      lock->release ();
      debug_printf ("get_initial_cwd failed, %E");
      return 0;
    }
  set (NULL);
  return 1;	/* Leaves cwd lock unreleased */
}

/* Fill out the elements of a cwdstuff struct.
   It is assumed that the lock for the cwd is acquired if
   win32_cwd == NULL. */
void
cwdstuff::set (const char *win32_cwd, const char *posix_cwd)
{
  char pathbuf[MAX_PATH];

  if (win32_cwd)
    {
      lock->acquire ();
      win32 = (char *) crealloc (win32, strlen (win32_cwd) + 1);
      strcpy (win32, win32_cwd);
    }

  if (!posix_cwd)
    cygwin_shared->mount.conv_to_posix_path (win32, pathbuf, 0);
  else
    (void) normalize_posix_path (posix_cwd, pathbuf);

  posix = (char *) crealloc (posix, strlen (pathbuf) + 1);
  strcpy (posix, pathbuf);

  hash = hash_path_name (0, win32);

  if (win32_cwd)
    lock->release ();

  return;
}

/* Copy the value for either the posix or the win32 cwd into a buffer. */
char *
cwdstuff::get (char *buf, int need_posix, int with_chroot, unsigned ulen)
{
  size_t len = ulen;

  if (!get_initial ())	/* Get initial cwd and set cwd lock */
    return NULL;

  char *tocopy;
  if (!need_posix)
    tocopy = win32;
  else
    tocopy = with_chroot && ischrootpath(posix) ?
	     posix + myself->rootlen : posix;

  debug_printf("myself->root: %s, posix: %s", myself->root, posix);
  if (strlen (tocopy) >= ulen)
    {
      set_errno (ERANGE);
      buf = NULL;
    }
  else
    {
      strcpy (buf, tocopy);
      if (!buf[0])	/* Should only happen when chroot */
	strcpy (buf, "/");
    }

  lock->release ();
  syscall_printf ("(%s) = cwdstuff::get (%p, %d, %d, %d)",
		  buf, buf, len, need_posix, with_chroot);
  return buf;
}

/* Get copies of all cwdstuff elements.  Used by spawn_guts. */
void
cwdstuff::copy (char * &posix_cwd, char * &win32_cwd, DWORD hash_cwd)
{
  lock->acquire ();
  get_initial (); /* FIXME: Check return someday */
  posix_cwd = cstrdup (posix);
  win32_cwd = cstrdup (win32);
  hash_cwd = hash;
  lock->release ();
}
