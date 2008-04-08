/* path.cc: path support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <mntent.h>
#include <ctype.h>
#include <wingdi.h>
#include <winuser.h>
#include <winnetwk.h>
#include <shlobj.h>
#include <cygwin/version.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "registry.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include <ntdll.h>
#include <wchar.h>

/* Determine if path prefix matches current cygdrive */
#define iscygdrive(path) \
  (path_prefix_p (mount_table->cygdrive, (path), mount_table->cygdrive_len))

#define iscygdrive_device(path) \
  (isalpha (path[mount_table->cygdrive_len]) && \
   (path[mount_table->cygdrive_len + 1] == '/' || \
    !path[mount_table->cygdrive_len + 1]))

#define isproc(path) \
  (path_prefix_p (proc, (path), proc_len))

/* is_unc_share: Return non-zero if PATH begins with //UNC/SHARE */

static inline bool __stdcall
is_unc_share (const char *path)
{
  const char *p;
  return (isdirsep (path[0])
	 && isdirsep (path[1])
	 && (isalnum (path[2]) || path[2] == '.')
	 && ((p = strpbrk (path + 3, "\\/")) != NULL)
	 && isalnum (p[1]));
}

/* Return true if src_path is a valid, internally supported device name.
   In that case, win32_path gets the corresponding NT device name and
   dev is appropriately filled with device information. */

static bool
win32_device_name (const char *src_path, char *win32_path, device& dev)
{
  dev.parse (src_path);
  if (dev == FH_FS || dev == FH_DEV)
    return false;
  strcpy (win32_path, dev.native);
  return true;
}

/* init: Initialize the mount table.  */

void
mount_info::init ()
{
  nmounts = 0;

  if (from_fstab (false) | from_fstab (true))	/* The single | is correct! */
    return;

  /* FIXME: Remove fetching from registry before releasing 1.7.0. */

  /* Fetch the mount table and cygdrive-related information from
     the registry.  */
  system_printf ("Fallback to fetching mounts from registry");
  from_registry ();
}

static void
set_flags (unsigned *flags, unsigned val)
{
  *flags = val;
  if (!(*flags & PATH_BINARY))
    {
      *flags |= PATH_TEXT;
      debug_printf ("flags: text (%p)", *flags & (PATH_TEXT | PATH_BINARY));
    }
  else
    {
      *flags |= PATH_BINARY;
      debug_printf ("flags: binary (%p)", *flags & (PATH_TEXT | PATH_BINARY));
    }
}

static char dot_special_chars[] =
    "."
    "\001" "\002" "\003" "\004" "\005" "\006" "\007" "\010"
    "\011" "\012" "\013" "\014" "\015" "\016" "\017" "\020"
    "\021" "\022" "\023" "\024" "\025" "\026" "\027" "\030"
    "\031" "\032" "\033" "\034" "\035" "\036" "\037" ":"
    "\\"   "*"    "?"    "%"     "\""   "<"    ">"    "|"
    "A"    "B"    "C"    "D"    "E"    "F"    "G"    "H"
    "I"    "J"    "K"    "L"    "M"    "N"    "O"    "P"
    "Q"    "R"    "S"    "T"    "U"    "V"    "W"    "X"
    "Y"    "Z";
static char *special_chars = dot_special_chars + 1;
static char special_introducers[] =
    "anpcl";

static char
special_char (const char *s, const char *valid_chars = special_chars)
{
  if (*s != '%' || strlen (s) < 3)
    return 0;

  char *p;
  char hex[] = {s[1], s[2], '\0'};
  unsigned char c = strtoul (hex, &p, 16);
  p = strechr (valid_chars, c);
  return *p;
}

/* Determines if name is "special".  Assumes that name is empty or "absolute" */
static int
special_name (const char *s, int inc = 1)
{
  if (!*s)
    return false;

  s += inc;

  if (strcmp (s, ".") == 0 || strcmp (s, "..") == 0)
    return false;

  int n;
  const char *p = NULL;
  if (ascii_strncasematch (s, "conin$", n = 5)
      || ascii_strncasematch (s, "conout$", n = 7)
      || ascii_strncasematch (s, "nul", n = 3)
      || ascii_strncasematch (s, "aux", 3)
      || ascii_strncasematch (s, "prn", 3)
      || ascii_strncasematch (s, "con", 3))
    p = s + n;
  else if (ascii_strncasematch (s, "com", 3)
	   || ascii_strncasematch (s, "lpt", 3))
    strtoul (s + 3, (char **) &p, 10);
  if (p && (*p == '\0' || *p == '.'))
    return -1;

  return (strchr (s, '\0')[-1] == '.')
	 || (strpbrk (s, special_chars) && !ascii_strncasematch (s, "%2f", 3));
}

bool
fnunmunge (char *dst, const char *src)
{
  bool converted = false;
  char c;

  if ((c = special_char (src, special_introducers)))
    {
      __small_sprintf (dst, "%c%s", c, src + 3);
      if (special_name (dst, 0))
	{
	  *dst++ = c;
	  src += 3;
	}
    }

  while (*src)
    if (!(c = special_char (src, dot_special_chars)))
      *dst++ = *src++;
    else
      {
	converted = true;
	*dst++ = c;
	src += 3;
      }

  *dst = *src;
  return converted;
}

static bool
copy1 (char *&d, const char *&src, int& left)
{
  left--;
  if (left || !*src)
    *d++ = *src++;
  else
    return true;
  return false;
}

static bool
copyenc (char *&d, const char *&src, int& left)
{
  char buf[16];
  int n = __small_sprintf (buf, "%%%02x", (unsigned char) *src++);
  left -= n;
  if (left <= 0)
    return true;
  strcpy (d, buf);
  d += n;
  return false;
}

int
mount_item::fnmunge (char *dst, const char *src, int& left)
{
  int name_type;
  if (!(name_type = special_name (src)))
    {
      if ((int) strlen (src) >= left)
	return ENAMETOOLONG;
      else
	strcpy (dst, src);
    }
  else
    {
      char *d = dst;
      if (copy1 (d, src, left))
	  return ENAMETOOLONG;
      if (name_type < 0 && copyenc (d, src, left))
	return ENAMETOOLONG;

      while (*src)
	if (!strchr (special_chars, *src) || (*src == '%' && !special_char (src)))
	  {
	    if (copy1 (d, src, left))
	      return ENAMETOOLONG;
	  }
	else if (copyenc (d, src, left))
	  return ENAMETOOLONG;

      char dot[] = ".";
      const char *p = dot;
      if (*--d != '.')
	d++;
      else if (copyenc (d, p, left))
	return ENAMETOOLONG;

      *d = *src;
    }

  backslashify (dst, dst, 0);
  return 0;
}

int
mount_item::build_win32 (char *dst, const char *src, unsigned *outflags, unsigned chroot_pathlen)
{
  int n, err = 0;
  const char *real_native_path;
  int real_posix_pathlen;
  set_flags (outflags, (unsigned) flags);
  if (!cygheap->root.exists () || posix_pathlen != 1 || posix_path[0] != '/')
    {
      n = native_pathlen;
      real_native_path = native_path;
      real_posix_pathlen = chroot_pathlen ?: posix_pathlen;
    }
  else
    {
      n = cygheap->root.native_length ();
      real_native_path = cygheap->root.native_path ();
      real_posix_pathlen = posix_pathlen;
    }
  memcpy (dst, real_native_path, n + 1);
  const char *p = src + real_posix_pathlen;
  if (*p == '/')
    /* nothing */;
  else if ((!(flags & MOUNT_ENC) && isdrive (dst) && !dst[2]) || *p)
    dst[n++] = '\\';
  //if (!*p || !(flags & MOUNT_ENC))
    //{
      if ((n + strlen (p)) >= NT_MAX_PATH)
	err = ENAMETOOLONG;
      else
	backslashify (p, dst + n, 0);
#if 0
    }
  else
    {
      int left = NT_MAX_PATH - n;
      while (*p)
	{
	  char slash = 0;
	  char *s = strchr (p + 1, '/');
	  if (s)
	    {
	      slash = *s;
	      *s = '\0';
	    }
	  err = fnmunge (dst += n, p, left);
	  if (!s || err)
	    break;
	  n = strlen (dst);
	  *s = slash;
	  p = s;
	}
    }
#endif
  return err;
}

/* conv_to_win32_path: Ensure src_path is a pure Win32 path and store
   the result in win32_path.

   If win32_path != NULL, the relative path, if possible to keep, is
   stored in win32_path.  If the relative path isn't possible to keep,
   the full path is stored.

   If full_win32_path != NULL, the full path is stored there.

   The result is zero for success, or an errno value.

   {,full_}win32_path must have sufficient space (i.e. NT_MAX_PATH bytes).  */

int
mount_info::conv_to_win32_path (const char *src_path, char *dst, device& dev,
				unsigned *flags)
{
  bool chroot_ok = !cygheap->root.exists ();
  while (sys_mount_table_counter < cygwin_shared->sys_mount_table_counter)
    {
      int current = cygwin_shared->sys_mount_table_counter;
      init ();
      sys_mount_table_counter = current;
    }
  MALLOC_CHECK;

  dev.devn = FH_FS;

  *flags = 0;
  debug_printf ("conv_to_win32_path (%s)", src_path);

  int i, rc;
  mount_item *mi = NULL;	/* initialized to avoid compiler warning */

  /* The path is already normalized, without ../../ stuff, we need to have this
     so that we can move from one mounted directory to another with relative
     stuff.

     eg mounting c:/foo /foo
     d:/bar /bar

     cd /bar
     ls ../foo

     should look in c:/foo, not d:/foo.

     converting normalizex UNIX path to a DOS-style path, looking up the
     appropriate drive in the mount table.  */

  /* See if this is a cygwin "device" */
  if (win32_device_name (src_path, dst, dev))
    {
      *flags = MOUNT_BINARY;	/* FIXME: Is this a sensible default for devices? */
      rc = 0;
      goto out_no_chroot_check;
    }

  MALLOC_CHECK;
  /* If the path is on a network drive or a //./ resp.//?/ path prefix,
     bypass the mount table.  If it's // or //MACHINE, use the netdrive
     device. */
  if (src_path[1] == '/')
    {
      if (!strchr (src_path + 2, '/'))
	{
	  dev = *netdrive_dev;
	  set_flags (flags, PATH_BINARY);
	}
      backslashify (src_path, dst, 0);
      /* Go through chroot check */
      goto out;
    }
  if (isproc (src_path))
    {
      dev = *proc_dev;
      dev.devn = fhandler_proc::get_proc_fhandler (src_path);
      if (dev.devn == FH_BAD)
	return ENOENT;
      set_flags (flags, PATH_BINARY);
      strcpy (dst, src_path);
      goto out;
    }
  /* Check if the cygdrive prefix was specified.  If so, just strip
     off the prefix and transform it into an MS-DOS path. */
  else if (iscygdrive (src_path))
    {
      int n = mount_table->cygdrive_len - 1;
      int unit;

      if (!src_path[n])
	{
	  unit = 0;
	  dst[0] = '\0';
	  if (mount_table->cygdrive_len > 1)
	    dev = *cygdrive_dev;
	}
      else if (cygdrive_win32_path (src_path, dst, unit))
	{
	  set_flags (flags, (unsigned) cygdrive_flags);
	  goto out;
	}
      else if (mount_table->cygdrive_len > 1)
	return ENOENT;
    }

  int chroot_pathlen;
  chroot_pathlen = 0;
  /* Check the mount table for prefix matches. */
  for (i = 0; i < nmounts; i++)
    {
      const char *path;
      int len;

      mi = mount + posix_sorted[i];
      if (!cygheap->root.exists ()
	  || (mi->posix_pathlen == 1 && mi->posix_path[0] == '/'))
	{
	  path = mi->posix_path;
	  len = mi->posix_pathlen;
	}
      else if (cygheap->root.posix_ok (mi->posix_path))
	{
	  path = cygheap->root.unchroot (mi->posix_path);
	  chroot_pathlen = len = strlen (path);
	}
      else
	{
	  chroot_pathlen = 0;
	  continue;
	}

      if (path_prefix_p (path, src_path, len))
	break;
    }

  if (i < nmounts)
    {
      int err = mi->build_win32 (dst, src_path, flags, chroot_pathlen);
      if (err)
	return err;
      chroot_ok = true;
    }
  else
    {
      int offset = 0;
      if (src_path[1] != '/' && src_path[1] != ':')
	offset = cygheap->cwd.get_drive (dst);
      backslashify (src_path, dst + offset, 0);
    }
 out:
  MALLOC_CHECK;
  if (chroot_ok || cygheap->root.ischroot_native (dst))
    rc = 0;
  else
    {
      debug_printf ("attempt to access outside of chroot '%s - %s'",
		    cygheap->root.posix_path (), cygheap->root.native_path ());
      rc = ENOENT;
    }

 out_no_chroot_check:
  debug_printf ("src_path %s, dst %s, flags %p, rc %d", src_path, dst, *flags, rc);
  return rc;
}

int
mount_info::get_mounts_here (const char *parent_dir, int parent_dir_len,
			     PUNICODE_STRING mount_points,
			     PUNICODE_STRING cygd)
{
  int n_mounts = 0;

  for (int i = 0; i < nmounts; i++)
    {
      mount_item *mi = mount + posix_sorted[i];
      char *last_slash = strrchr (mi->posix_path, '/');
      if (!last_slash)
	continue;
      if (last_slash == mi->posix_path)
	{
	  if (parent_dir_len == 1 && mi->posix_pathlen > 1)
	    RtlCreateUnicodeStringFromAsciiz (&mount_points[n_mounts++],
					      last_slash + 1);
	}
      else if (parent_dir_len == last_slash - mi->posix_path
	       && strncasematch (parent_dir, mi->posix_path, parent_dir_len))
	RtlCreateUnicodeStringFromAsciiz (&mount_points[n_mounts++],
					  last_slash + 1);
    }
  RtlCreateUnicodeStringFromAsciiz (cygd, cygdrive + 1);
  cygd->Length -= 2;	// Strip trailing slash
  return n_mounts;
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
  dst[len++] = cyg_tolower (src[0]);
  if (!src[2] || (isdirsep (src[2]) && !src[3]))
    dst[len++] = '\000';
  else
    {
      int n;
      dst[len++] = '/';
      if (isdirsep (src[2]))
	n = 3;
      else
	n = 2;
      strcpy (dst + len, src + n);
    }
  slashify (dst, dst, trailing_slash_p);
}

int
mount_info::cygdrive_win32_path (const char *src, char *dst, int& unit)
{
  int res;
  const char *p = src + cygdrive_len;
  if (!isalpha (*p) || (!isdirsep (p[1]) && p[1]))
    {
      unit = -1; /* FIXME: should be zero, maybe? */
      dst[0] = '\0';
      res = 0;
    }
  else
    {
      dst[0] = cyg_tolower (*p);
      dst[1] = ':';
      strcpy (dst + 2, p + 1);
      backslashify (dst, dst, !dst[2]);
      unit = dst[0];
      res = 1;
    }
  debug_printf ("src '%s', dst '%s'", src, dst);
  return res;
}

/* conv_to_posix_path: Ensure src_path is a POSIX path.

   The result is zero for success, or an errno value.
   posix_path must have sufficient space (i.e. NT_MAX_PATH bytes).
   If keep_rel_p is non-zero, relative paths stay that way.  */

/* TODO: Change conv_to_posix_path to work with native paths. */

/* src_path is a wide Win32 path. */
int
mount_info::conv_to_posix_path (PWCHAR src_path, char *posix_path,
				int keep_rel_p)
{
  bool changed = false;
  if (!wcsncmp (src_path, L"\\\\?\\", 4))
    {
      src_path += 4;
      if (!wcsncmp (src_path, L"UNC\\", 4))
	{
	  src_path += 2;
	  src_path[0] = L'\\';
	  changed = true;
	}
    }
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  sys_wcstombs (buf, NT_MAX_PATH, src_path);
  int ret = conv_to_posix_path (buf, posix_path, keep_rel_p);
  if (changed)
    src_path[0] = L'C';
  return ret;
}

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
      trailing_slash_p = isdirsep (*lastchar) && lastchar[-1] != ':';
    }

  debug_printf ("conv_to_posix_path (%s, %s, %s)", src_path,
		keep_rel_p ? "keep-rel" : "no-keep-rel",
		trailing_slash_p ? "add-slash" : "no-add-slash");
  MALLOC_CHECK;

  if (src_path_len >= NT_MAX_PATH)
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

  tmp_pathbuf tp;
  char *pathbuf = tp.c_get ();
  char *tail;
  int rc = normalize_win32_path (src_path, pathbuf, tail);
  if (rc != 0)
    {
      debug_printf ("%d = conv_to_posix_path (%s)", rc, src_path);
      return rc;
    }

  int pathbuflen = tail - pathbuf;
  for (int i = 0; i < nmounts; ++i)
    {
      mount_item &mi = mount[native_sorted[i]];
      if (!path_prefix_p (mi.native_path, pathbuf, mi.native_pathlen))
	continue;

      if (cygheap->root.exists () && !cygheap->root.posix_ok (mi.posix_path))
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
      if ((mi.posix_pathlen + (pathbuflen - mi.native_pathlen) + addslash) >= NT_MAX_PATH)
	return ENAMETOOLONG;
      strcpy (posix_path, mi.posix_path);
      if (addslash)
	strcat (posix_path, "/");
      if (nextchar)
	slashify (p,
		  posix_path + addslash + (mi.posix_pathlen == 1 ? 0 : mi.posix_pathlen),
		  trailing_slash_p);

      if (cygheap->root.exists ())
	{
	  const char *p = cygheap->root.unchroot (posix_path);
	  memmove (posix_path, p, strlen (p) + 1);
	}
#if 0
      if (mi.flags & MOUNT_ENC)
	{
	  char *tmpbuf = tp.c_get ();
	  if (fnunmunge (tmpbuf, posix_path))
	    strcpy (posix_path, tmpbuf);
	}
#endif
      goto out;
    }

  if (!cygheap->root.exists ())
    /* nothing */;
  else if (!cygheap->root.ischroot_native (pathbuf))
    return ENOENT;
  else
    {
      const char *p = pathbuf + cygheap->root.native_length ();
      if (*p)
	slashify (p, posix_path, trailing_slash_p);
      else
	{
	  posix_path[0] = '/';
	  posix_path[1] = '\0';
	}
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
  return PATH_BINARY;
}

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
  while (sp = strstr (sp, "\\040"))
    {
      *sp++ = ' ';
      memmove (sp, sp + 3, strlen (sp + 3) + 1);
    }
  return field;
}

struct opt
{   
  const char *name;
  unsigned val;
  bool clear;
} oopts[] =
{
  {"user", MOUNT_SYSTEM, 1},
  {"nouser", MOUNT_SYSTEM, 0},
  {"binary", MOUNT_BINARY, 0},
  {"text", MOUNT_BINARY, 1},
  {"exec", MOUNT_EXEC, 0},
  {"notexec", MOUNT_NOTEXEC, 0},
  {"cygexec", MOUNT_CYGWIN_EXEC, 0},
  {"nosuid", 0, 0},
  {"managed", MOUNT_ENC, 0}
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
      system_printf ("invalid fstab option - '%s'", options);
      return false;

    gotit:
      options = p;
    }
  return true;
}

bool
mount_info::from_fstab_line (char *line, bool user)
{
  char *native_path, *posix_path, *fs_type;

  /* First field: Native path. */
  char *c = skip_ws (line);
  if (!*c || *c == '#')
    return true;
  char *cend = find_ws (c);
  *cend = '\0';
  native_path = conv_fstab_spaces (c);
  /* Second field: POSIX path. */
  c = skip_ws (cend + 1);
  if (!*c)
    return true;
  cend = find_ws (c);
  *cend = '\0';
  posix_path = conv_fstab_spaces (c);
  /* Third field: FS type. */
  c = skip_ws (cend + 1);
  if (!*c)
    return true;
  cend = find_ws (c);
  *cend = '\0';
  fs_type = c;
  /* Forth field: Flags. */
  c = skip_ws (cend + 1);
  if (!*c)
    return true;
  cend = find_ws (c);
  *cend = '\0';
  unsigned mount_flags = MOUNT_SYSTEM;
  if (!read_flags (c, mount_flags))
    return true;
  if (user)
    mount_flags &= ~MOUNT_SYSTEM;
  if (!strcmp (fs_type, "cygdrive"))
    {
      cygdrive_flags = mount_flags | MOUNT_CYGDRIVE;
      slashify (posix_path, cygdrive, 1);
      cygdrive_len = strlen (cygdrive);
    }
  else
    {
      int res = mount_table->add_item (native_path, posix_path, mount_flags);
      if (res && get_errno () == EMFILE)
	return false;
    }
  return true;
}

bool
mount_info::from_fstab (bool user)
{
  tmp_pathbuf tp;
  PWCHAR path = tp.w_get ();
  PWCHAR w;
  
  if (!GetModuleFileNameW (GetModuleHandleW (L"cygwin1.dll"),
			   path, NT_MAX_PATH))
    {
      debug_printf ("GetModuleFileNameW, %E");
      return false;
    }
  w = wcsrchr (path, L'\\');
  if (w)
    {
      *w = L'\0';
      w = wcsrchr (path, L'\\');
    }
  if (!w)
    {
      debug_printf ("Invalid DLL path");
      return false;
    }

  if (!user)
    {
      /* Create a default root dir from the path the Cygwin DLL is in. */
      *w = L'\0';
      char *native_root = tp.c_get ();
      sys_wcstombs (native_root, NT_MAX_PATH, path);
      mount_table->add_item (native_root, "/", MOUNT_SYSTEM | MOUNT_BINARY);
      /* Create a default cygdrive entry.  Note that this is a user entry.
         This allows to override it with mount, unless the sysadmin created
	 a cygdrive entry in /etc/fstab. */
      cygdrive_flags = MOUNT_BINARY | MOUNT_CYGDRIVE;
      strcpy (cygdrive, "/cygdrive/");
      cygdrive_len = strlen (cygdrive);
    }

  PWCHAR u = wcpcpy (w, L"\\etc\\fstab");
  if (user)
    sys_mbstowcs (wcpcpy (u, L".d\\"), NT_MAX_PATH - (u - path),
		  cygheap->user.name ());
  debug_printf ("Try to read mounts from %W", path);
  HANDLE h = CreateFileW (path, GENERIC_READ, FILE_SHARE_READ, &sec_none_nih,
			  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE)
    {
      debug_printf ("CreateFileW, %E");
      return false;
    }
  char *const buf = reinterpret_cast<char *const> (path);
  char *got = buf;
  DWORD len = 0;
  /* Using NT_MAX_PATH-1 leaves space to append two \0. */
  while (ReadFile (h, got, (NT_MAX_PATH - 1) * sizeof (WCHAR) - (got - buf),
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
	  if (!from_fstab_line (got, user))
	    goto done;
	  got = end + 1;
	}
      if (len < (NT_MAX_PATH - 1) * sizeof (WCHAR))
        break;
      /* We have to read once more.  Move remaining bytes to the start of
         the buffer and reposition got so that it points to the end of
	 the remaining bytes. */
      len = buf + len - got;
      memmove (buf, got, len);
      got = buf + len;
      buf[len] = buf[len + 1] = '\0';
    }
  if (got > buf)
    from_fstab_line (got, user);
done:
  CloseHandle (h);
  return true;
}

/* read_mounts: Given a specific regkey, read mounts from under its
   key. */
/* FIXME: Remove before releasing 1.7.0. */

void
mount_info::read_mounts (reg_key& r)
{
  tmp_pathbuf tp;
  char *native_path = tp.c_get ();
  /* FIXME: The POSIX path is stored as value name right now, which is
     restricted to 256 bytes. */
  char posix_path[CYG_MAX_PATH];
  HKEY key = r.get_key ();
  DWORD i, posix_path_size;
  int res;

  /* Loop through subkeys */
  /* FIXME: we would like to not check MAX_MOUNTS but the heap in the
     shared area is currently statically allocated so we can't have an
     arbitrarily large number of mounts. */
  for (i = 0; ; i++)
    {
      int mount_flags;

      posix_path_size = sizeof (posix_path);
      /* FIXME: if maximum posix_path_size is 256, we're going to
	 run into problems if we ever try to store a mount point that's
	 over 256 but is under CYG_MAX_PATH. */
      res = RegEnumKeyEx (key, i, posix_path, &posix_path_size, NULL,
			  NULL, NULL, NULL);

      if (res == ERROR_NO_MORE_ITEMS)
	break;
      else if (res != ERROR_SUCCESS)
	{
	  debug_printf ("RegEnumKeyEx failed, error %d!", res);
	  break;
	}

      /* Get a reg_key based on i. */
      reg_key subkey = reg_key (key, KEY_READ, posix_path, NULL);

      /* Fetch info from the subkey. */
      subkey.get_string ("native", native_path, NT_MAX_PATH, "");
      mount_flags = subkey.get_int ("flags", 0);

      /* Add mount_item corresponding to registry mount point. */
      res = mount_table->add_item (native_path, posix_path, mount_flags);
      if (res && get_errno () == EMFILE)
	break; /* The number of entries exceeds MAX_MOUNTS */
    }
}

/* from_registry: Build the entire mount table from the registry.  Also,
   read in cygdrive-related information from its registry location. */
/* FIXME: Remove before releasing 1.7.0. */

void
mount_info::from_registry ()
{

  /* Retrieve cygdrive-related information. */
  read_cygdrive_info_from_registry ();

  nmounts = 0;

  /* First read mounts from user's table.
     Then read mounts from system-wide mount table while deimpersonated . */
  for (int i = 0; i < 2; i++)
    {
      if (i)
	cygheap->user.deimpersonate ();
      reg_key r (i, KEY_READ, CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
      read_mounts (r);
      if (i)
	cygheap->user.reimpersonate ();
    }
}

/* read_cygdrive_info_from_registry: Read the default prefix and flags
   to use when creating cygdrives from the special user registry
   location used to store cygdrive information. */
/* FIXME: Remove before releasing 1.7.0. */

void
mount_info::read_cygdrive_info_from_registry ()
{
  /* First read cygdrive from user's registry.
     If failed, then read cygdrive from system-wide registry
     while deimpersonated. */
  for (int i = 0; i < 2; i++)
    {
      if (i)
	cygheap->user.deimpersonate ();
      reg_key r (i, KEY_READ, CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
      if (i)
	cygheap->user.reimpersonate ();

      if (r.get_string (CYGWIN_INFO_CYGDRIVE_PREFIX, cygdrive, sizeof (cygdrive),
			CYGWIN_INFO_CYGDRIVE_DEFAULT_PREFIX) != ERROR_SUCCESS && i == 0)
	continue;

      /* Fetch user cygdrive_flags from registry; returns MOUNT_CYGDRIVE on error. */
      cygdrive_flags = r.get_int (CYGWIN_INFO_CYGDRIVE_FLAGS,
				  MOUNT_CYGDRIVE | MOUNT_BINARY);
      /* Sanitize */
      if (i == 0)
	cygdrive_flags &= ~MOUNT_SYSTEM;
      else
	cygdrive_flags |= MOUNT_SYSTEM;
      slashify (cygdrive, cygdrive, 1);
      cygdrive_len = strlen (cygdrive);
      break;
    }
}

/* write_cygdrive_info: Store default prefix and flags
   to use when creating cygdrives to the special user shared mem
   location used to store cygdrive information. */

int
mount_info::write_cygdrive_info (const char *cygdrive_prefix, unsigned flags)
{
  /* Verify cygdrive prefix starts with a forward slash and if there's
     another character, it's not a slash. */
  if ((cygdrive_prefix == NULL) || (*cygdrive_prefix == 0) ||
      (!isslash (cygdrive_prefix[0])) ||
      ((cygdrive_prefix[1] != '\0') && (isslash (cygdrive_prefix[1]))))
    {
      set_errno (EINVAL);
      return -1;
    }
  /* Don't allow to override a system cygdrive prefix. */
  if (cygdrive_flags & MOUNT_SYSTEM)
    {
      set_errno (EPERM);
      return -1;
    }

  slashify (cygdrive_prefix, cygdrive, 1);
  cygdrive_flags = flags & ~MOUNT_SYSTEM;
  cygdrive_len = strlen (cygdrive);

  return 0;
}

int
mount_info::get_cygdrive_info (char *user, char *system, char* user_flags,
			       char* system_flags)
{
  if (user)
    *user = '\0';
  /* Get the user flags, if appropriate */
  if (user_flags)
    *user_flags = '\0';

  if (system)
    strcpy (system, cygdrive);

  if (system_flags)
    strcpy (system_flags,
	    (cygdrive_flags & MOUNT_BINARY) ? "binmode" : "textmode");

  return 0;
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
     if (!(bp->flags & MOUNT_SYSTEM))	/* user mount */
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
     if (!(bp->flags & MOUNT_SYSTEM))	/* user mount */
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
mount_info::add_item (const char *native, const char *posix,
		      unsigned mountflags)
{
  tmp_pathbuf tp;
  char *nativetmp = tp.c_get ();
  /* FIXME: The POSIX path is stored as value name right now, which is
     restricted to 256 bytes. */
  char posixtmp[CYG_MAX_PATH];
  char *nativetail, *posixtail, error[] = "error";
  int nativeerr, posixerr;

  /* Something's wrong if either path is NULL or empty, or if it's
     not a UNC or absolute path. */

  if (native == NULL || !isabspath (native) ||
      !(is_unc_share (native) || isdrive (native)))
    nativeerr = EINVAL;
  else
    nativeerr = normalize_win32_path (native, nativetmp, nativetail);

  if (posix == NULL || !isabspath (posix) ||
      is_unc_share (posix) || isdrive (posix))
    posixerr = EINVAL;
  else
    posixerr = normalize_posix_path (posix, posixtmp, posixtail);

  debug_printf ("%s[%s], %s[%s], %p",
		native, nativeerr ? error : nativetmp,
		posix, posixerr ? error : posixtmp, mountflags);

  if (nativeerr || posixerr)
    {
      set_errno (nativeerr?:posixerr);
      return -1;
    }

  /* Make sure both paths do not end in /. */
  if (nativetail > nativetmp + 1 && nativetail[-1] == '\\')
    nativetail[-1] = '\0';
  if (posixtail > posixtmp + 1 && posixtail[-1] == '/')
    posixtail[-1] = '\0';

  /* Write over an existing mount item with the same POSIX path if
     it exists and is from the same registry area. */
  int i;
  for (i = 0; i < nmounts; i++)
    {
      if (strcasematch (mount[i].posix_path, posixtmp))
        {
	  /* Don't allow to override a system mount with a user mount. */
	  if ((mount[i].flags & MOUNT_SYSTEM) && !(mountflags & MOUNT_SYSTEM))
	    {
	      set_errno (EPERM);
	      return -1;
	    }
	  if ((mount[i].flags & MOUNT_SYSTEM) == (mountflags & MOUNT_SYSTEM))
	    break;
	}
    }

  if (i == nmounts && nmounts == MAX_MOUNTS)
    {
      set_errno (EMFILE);
      return -1;
    }

  if (i == nmounts)
    nmounts++;
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
mount_info::del_item (const char *path, unsigned flags)
{
  tmp_pathbuf tp;
  char *pathtmp = tp.c_get ();
  int posix_path_p = false;

  /* Something's wrong if path is NULL or empty. */
  if (path == NULL || *path == 0 || !isabspath (path))
    {
      set_errno (EINVAL);
      return -1;
    }

  if (is_unc_share (path) || strpbrk (path, ":\\"))
    backslashify (path, pathtmp, 0);
  else
    {
      slashify (path, pathtmp, 0);
      posix_path_p = true;
    }
  nofinalslash (pathtmp, pathtmp);

  for (int i = 0; i < nmounts; i++)
    {
      int ent = native_sorted[i]; /* in the same order as getmntent() */
      if (((posix_path_p)
	   ? strcasematch (mount[ent].posix_path, pathtmp)
	   : strcasematch (mount[ent].native_path, pathtmp)))
	{
	  /* Don't allow to remove a system mount. */
	  if ((mount[ent].flags & MOUNT_SYSTEM))
	    {
	      set_errno (EPERM);
	      return -1;
	    }
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

/************************* mount_item class ****************************/

static mntent *
fillout_mntent (const char *native_path, const char *posix_path, unsigned flags)
{
  struct mntent& ret=_my_tls.locals.mntbuf;
  bool append_bs = false;

  /* Remove drivenum from list if we see a x: style path */
  if (strlen (native_path) == 2 && native_path[1] == ':')
    {
      int drivenum = cyg_tolower (native_path[0]) - 'a';
      if (drivenum >= 0 && drivenum <= 31)
	_my_tls.locals.available_drives &= ~(1 << drivenum);
      append_bs = true;
    }

  /* Pass back pointers to mount_table strings reserved for use by
     getmntent rather than pointers to strings in the internal mount
     table because the mount table might change, causing weird effects
     from the getmntent user's point of view. */

  strcpy (_my_tls.locals.mnt_fsname, native_path);
  ret.mnt_fsname = _my_tls.locals.mnt_fsname;
  strcpy (_my_tls.locals.mnt_dir, posix_path);
  ret.mnt_dir = _my_tls.locals.mnt_dir;

  /* Try to give a filesystem type that matches what a Linux application might
     expect. Naturally, this is a moving target, but we can make some
     reasonable guesses for popular types. */

  fs_info mntinfo;
  tmp_pathbuf tp;
  UNICODE_STRING unat;
  tp.u_get (&unat);
  get_nt_native_path (native_path, unat, flags & MOUNT_ENC);
  if (append_bs)
    RtlAppendUnicodeToString (&unat, L"\\");
  mntinfo.update (&unat, true);  /* this pulls from a cache, usually. */

  if (mntinfo.is_samba())
    strcpy (_my_tls.locals.mnt_type, (char *) "smbfs");
  else if (mntinfo.is_nfs ())
    strcpy (_my_tls.locals.mnt_type, (char *) "nfs");
  else if (mntinfo.is_fat ())
    strcpy (_my_tls.locals.mnt_type, (char *) "vfat");
  else if (mntinfo.is_ntfs ())
    strcpy (_my_tls.locals.mnt_type, (char *) "ntfs");
  else if (mntinfo.is_netapp ())
    strcpy (_my_tls.locals.mnt_type, (char *) "netapp");
  else if (mntinfo.is_cdrom ())
    strcpy (_my_tls.locals.mnt_type, (char *) "iso9660");
  else
    strcpy (_my_tls.locals.mnt_type, (char *) "unknown");

  ret.mnt_type = _my_tls.locals.mnt_type;

  /* mnt_opts is a string that details mount params such as
     binary or textmode, or exec.  We don't print
     `silent' here; it's a magic internal thing. */

  if (!(flags & MOUNT_BINARY))
    strcpy (_my_tls.locals.mnt_opts, (char *) "textmode");
  else
    strcpy (_my_tls.locals.mnt_opts, (char *) "binmode");

  if (flags & MOUNT_CYGWIN_EXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",cygexec");
  else if (flags & MOUNT_EXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",exec");
  else if (flags & MOUNT_NOTEXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",noexec");
  if (flags & MOUNT_ENC)
    strcat (_my_tls.locals.mnt_opts, ",managed");

  if ((flags & MOUNT_CYGDRIVE))		/* cygdrive */
    strcat (_my_tls.locals.mnt_opts, (char *) ",noumount");

  if (!(flags & MOUNT_SYSTEM))		/* user mount */
    strcat (_my_tls.locals.mnt_opts, (char *) ",user");
  else					/* system mount */
    strcat (_my_tls.locals.mnt_opts, (char *) ",system");

  ret.mnt_opts = _my_tls.locals.mnt_opts;

  ret.mnt_freq = 1;
  ret.mnt_passno = 1;
  return &ret;
}

struct mntent *
mount_item::getmntent ()
{
  return fillout_mntent (native_path, posix_path, flags);
}

static struct mntent *
cygdrive_getmntent ()
{
  char native_path[4];
  char posix_path[CYG_MAX_PATH];
  DWORD mask = 1, drive = 'a';
  struct mntent *ret = NULL;

  while (_my_tls.locals.available_drives)
    {
      for (/* nothing */; drive <= 'z'; mask <<= 1, drive++)
	if (_my_tls.locals.available_drives & mask)
	  break;

      __small_sprintf (native_path, "%c:\\", drive);
      if (GetFileAttributes (native_path) == INVALID_FILE_ATTRIBUTES)
	{
	  _my_tls.locals.available_drives &= ~mask;
	  continue;
	}
      native_path[2] = '\0';
      __small_sprintf (posix_path, "%s%c", mount_table->cygdrive, drive);
      ret = fillout_mntent (native_path, posix_path, mount_table->cygdrive_flags);
      break;
    }

  return ret;
}

struct mntent *
mount_info::getmntent (int x)
{
  if (x < 0 || x >= nmounts)
    return cygdrive_getmntent ();

  return mount[native_sorted[x]].getmntent ();
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

extern "C" int
mount (const char *win32_path, const char *posix_path, unsigned flags)
{
  int res = -1;
  flags &= ~MOUNT_SYSTEM;

  myfault efault;
  if (efault.faulted (EFAULT))
    /* errno set */;
  else if (!*posix_path)
    set_errno (EINVAL);
  else if (strpbrk (posix_path, "\\:"))
    set_errno (EINVAL);
  else if (flags & MOUNT_CYGDRIVE) /* normal mount */
    {
      /* When flags include MOUNT_CYGDRIVE, take this to mean that
	we actually want to change the cygdrive prefix and flags
	without actually mounting anything. */
      res = mount_table->write_cygdrive_info (posix_path, flags);
      win32_path = NULL;
    }
  else if (!*win32_path)
    set_errno (EINVAL);
  else
    res = mount_table->add_item (win32_path, posix_path, flags);

  syscall_printf ("%d = mount (%s, %s, %p)", res, win32_path, posix_path, flags);
  return res;
}

/* umount: The standard umount call only has a path parameter.  Since
   it is not possible for this call to specify whether to remove the
   mount from the user or global mount registry table, assume the user
   table. */

extern "C" int
umount (const char *path)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (!*path)
    {
      set_errno (EINVAL);
      return -1;
    }
  return cygwin_umount (path, 0);
}

/* cygwin_umount: This is like umount but takes an additional flags
   parameter that specifies whether to umount from the user or system-wide
   registry area. */

extern "C" int
cygwin_umount (const char *path, unsigned flags)
{
  int res = -1;

  if (!(flags & MOUNT_CYGDRIVE))
    res = mount_table->del_item (path, flags & ~MOUNT_SYSTEM);

  syscall_printf ("%d = cygwin_umount (%s, %d)", res,  path, flags);
  return res;
}

bool
is_floppy (const char *dos)
{
  char dev[256];
  if (!QueryDosDevice (dos, dev, 256))
    return false;
  return ascii_strncasematch (dev, "\\Device\\Floppy", 14);
}

extern "C" FILE *
setmntent (const char *filep, const char *)
{
  _my_tls.locals.iteration = 0;
  _my_tls.locals.available_drives = GetLogicalDrives ();
  /* Filter floppy drives on A: and B: */
  if ((_my_tls.locals.available_drives & 1) && is_floppy ("A:"))
    _my_tls.locals.available_drives &= ~1;
  if ((_my_tls.locals.available_drives & 2) && is_floppy ("B:"))
    _my_tls.locals.available_drives &= ~2;
  return (FILE *) filep;
}

extern "C" struct mntent *
getmntent (FILE *)
{
  return mount_table->getmntent (_my_tls.locals.iteration++);
}

extern "C" int
endmntent (FILE *)
{
  return 1;
}
