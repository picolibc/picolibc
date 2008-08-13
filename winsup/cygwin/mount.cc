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
#include "cygtls.h"
#include "tls_pbuf.h"
#include <ntdll.h>
#include <wchar.h>

/* Determine if path prefix matches current cygdrive */
#define iscygdrive(path) \
  (path_prefix_p (mount_table->cygdrive, (path), mount_table->cygdrive_len, false))

#define iscygdrive_device(path) \
  (isalpha (path[mount_table->cygdrive_len]) && \
   (path[mount_table->cygdrive_len + 1] == '/' || \
    !path[mount_table->cygdrive_len + 1]))

#define isproc(path) \
  (path_prefix_p (proc, (path), proc_len, false))

/* is_unc_share: Return non-zero if PATH begins with //server/share 
                 or with one of the native prefixes //./ or //?/ 
   This function is only used to test for valid input strings.
   The later normalization drops the native prefixes. */

static inline bool __stdcall
is_native_path (const char *path)
{
  return isdirsep (path[0])
	 && (isdirsep (path[1]) || path[1] == '?')
	 && (path[2] == '?' || path[2] == '.')
	 && isdirsep (path[3])
	 && isalpha (path[4]);
}

static inline bool __stdcall
is_unc_share (const char *path)
{
  const char *p;
  return (isdirsep (path[0])
	 && isdirsep (path[1])
	 && isalnum (path[2])
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

inline void
mount_info::create_root_entry (const PWCHAR root)
{
  /* Create a default root dir from the path the Cygwin DLL is in. */
  char native_root[PATH_MAX];
  sys_wcstombs (native_root, PATH_MAX, root);
  mount_table->add_item (native_root, "/", MOUNT_SYSTEM | MOUNT_BINARY);
  /* Create a default cygdrive entry.  Note that this is a user entry.
     This allows to override it with mount, unless the sysadmin created
     a cygdrive entry in /etc/fstab. */
  cygdrive_flags = MOUNT_BINARY | MOUNT_CYGDRIVE;
  strcpy (cygdrive, CYGWIN_INFO_CYGDRIVE_DEFAULT_PREFIX "/");
  cygdrive_len = strlen (cygdrive);
}

/* init: Initialize the mount table.  */

void
mount_info::init ()
{
  nmounts = 0;
  PWCHAR pathend;
  WCHAR path[PATH_MAX];
  
  pathend = wcpcpy (path, cygwin_shared->installation_root);
  create_root_entry (path);
  pathend = wcpcpy (pathend, L"\\etc\\fstab");
  if (from_fstab (false, path, pathend)   /* The single | is correct! */
      | from_fstab (true, path, pathend))
      return;

  /* FIXME: Remove warning message before releasing 1.7.0. */
  small_printf ("Huh?  No /etc/fstab file in %W?  Using default root and cygdrive prefix...\n", path);
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
  else if ((isdrive (dst) && !dst[2]) || *p)
    dst[n++] = '\\';
  if ((n + strlen (p)) >= NT_MAX_PATH)
    err = ENAMETOOLONG;
  else
    backslashify (p, dst + n, 0);
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

      if (path_prefix_p (path, src_path, len, mi->flags & MOUNT_NOPOSIX))
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
      if (src_path[1] != L':') /* native UNC path */
	{
	  *(src_path += 2) = L'\\';
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
      if (!path_prefix_p (mi.native_path, pathbuf, mi.native_pathlen,
			  mi.flags & MOUNT_NOPOSIX))
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
      if (path_prefix_p (mi.native_path, p, mi.native_pathlen,
			 mi.flags & MOUNT_NOPOSIX))
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
  {"acl", MOUNT_NOACL, 1},
  {"noacl", MOUNT_NOACL, 0},
  {"posix=1", MOUNT_NOPOSIX, 1},
  {"posix=0", MOUNT_NOPOSIX, 0}
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
  unsigned mount_flags = MOUNT_SYSTEM | MOUNT_BINARY;
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
mount_info::from_fstab (bool user, WCHAR fstab[], PWCHAR fstab_end)
{
  UNICODE_STRING upath;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  HANDLE fh;

  if (user)
    {
      extern void transform_chars (PWCHAR, PWCHAR);
      PWCHAR username;
      sys_mbstowcs (username = wcpcpy (fstab_end, L".d\\"),
		    NT_MAX_PATH - (fstab_end - fstab),
		    cygheap->user.name ());
      /* Make sure special chars in the username are converted according to
         the rules. */
      transform_chars (username, username + wcslen (username) - 1);
    }
  RtlInitUnicodeString (&upath, fstab);
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  debug_printf ("Try to read mounts from %W", fstab);
  status = NtOpenFile (&fh, SYNCHRONIZE | FILE_READ_DATA, &attr, &io,
		       FILE_SHARE_VALID_FLAGS, FILE_SYNCHRONOUS_IO_NONALERT);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtOpenFile(%S) failed, %p", &upath, status);
      return false;
    }

  char buf[NT_MAX_PATH];
  char *got = buf;
  DWORD len = 0;
  unsigned line = 1;
  /* Using buffer size - 2 leaves space to append two \0. */
  while (NT_SUCCESS (NtReadFile (fh, NULL, NULL, NULL, &io, got,
				 (sizeof (buf) - 2) - (got - buf), NULL, NULL)))
    {
      char *end;

      len = io.Information;
      /* Set end marker. */
      got[len] = got[len + 1] = '\0';
      /* Set len to the absolute len of bytes in buf. */
      len += got - buf;
      /* Reset got to start reading at the start of the buffer again. */
      got = buf;
retry:
      bool got_nl = false;
      while (got < buf + len && (end = strchr (got, '\n')))
        {
	  got_nl = true;
	  end[end[-1] == '\r' ? -1 : 0] = '\0';
	  if (!from_fstab_line (got, user))
	    goto done;
	  got = end + 1;
	  ++line;
	}
      if (len < (sizeof (buf) - 2))
        break;
      /* Check if the buffer contained at least one \n.  If not, the
         line length is > 32K.  We don't take such long lines.  Print
	 a debug message and skip this line entirely. */
      if (!got_nl)
        {
	  system_printf ("%W: Line %d too long, skipping...", fstab, line);
	  while (NT_SUCCESS (NtReadFile (fh, NULL, NULL, NULL, &io, buf,
	  				 (sizeof (buf) - 2), NULL, NULL)))
	    {
	      len = io.Information;
	      buf[len] = buf[len + 1] = '\0';
	      got = strchr (buf, '\n');
	      if (got)
	        {
		  ++got;
		  ++line;
		  goto retry;
		}
	    }
	  got = buf;
	  break;
	}
      /* We have to read once more.  Move remaining bytes to the start of
         the buffer and reposition got so that it points to the end of
	 the remaining bytes. */
      len = buf + len - got;
      memmove (buf, got, len);
      got = buf + len;
      buf[len] = buf[len + 1] = '\0';
    }
  /* Catch a last line without trailing \n. */
  if (got > buf)
    from_fstab_line (got, user);
done:
  NtClose (fh);
  return true;
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
mount_info::get_cygdrive_info (char *user, char *system, char *user_flags,
			       char *system_flags)
{
  if (user)
    *user = '\0';
  if (system)
    *system = '\0';
  if (user_flags)
    *user_flags = '\0';
  if (system_flags)
    *system_flags = '\0';

  char *path = (cygdrive_flags & MOUNT_SYSTEM) ? system : user;
  char *flags = (cygdrive_flags & MOUNT_SYSTEM) ? system_flags : user_flags;

  if (path)
    {
      strcpy (path, cygdrive);
      /* Strip trailing slash for backward compatibility. */
      if (cygdrive_len > 2)
        path[cygdrive_len - 1] = '\0';
    }
  if (flags)
    strcpy (flags, (cygdrive_flags & MOUNT_BINARY) ? "binmode" : "textmode");
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
      !(is_native_path (native) || is_unc_share (native) || isdrive (native)))
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
      if (!strcmp (mount[i].posix_path, posixtmp))
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
	   ? !strcmp (mount[ent].posix_path, pathtmp)
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
  get_nt_native_path (native_path, unat);
  if (append_bs)
    RtlAppendUnicodeToString (&unat, L"\\");
  mntinfo.update (&unat, NULL);

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
    strcpy (_my_tls.locals.mnt_opts, (char *) "text");
  else
    strcpy (_my_tls.locals.mnt_opts, (char *) "binary");

  if (flags & MOUNT_CYGWIN_EXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",cygexec");
  else if (flags & MOUNT_EXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",exec");
  else if (flags & MOUNT_NOTEXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",noexec");

  if (flags & MOUNT_NOACL)
    strcat (_my_tls.locals.mnt_opts, (char *) ",noacl");

  if (flags & MOUNT_NOPOSIX)
    strcat (_my_tls.locals.mnt_opts, (char *) ",posix=0");

  if ((flags & MOUNT_CYGDRIVE))		/* cygdrive */
    strcat (_my_tls.locals.mnt_opts, (char *) ",noumount");

  if (!(flags & MOUNT_SYSTEM))		/* user mount */
    strcat (_my_tls.locals.mnt_opts, (char *) ",user");

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
