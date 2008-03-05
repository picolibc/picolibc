/* path.cc: path support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Red Hat, Inc.

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
   c: means c:\.
*/

#include "winsup.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <mntent.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>
#include <winioctl.h>
#include <wingdi.h>
#include <winuser.h>
#include <winnls.h>
#include <winnetwk.h>
#include <shlobj.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "registry.h"
#include "cygtls.h"
#include <assert.h>

static int normalize_win32_path (const char *, char *, char *&);
static void slashify (const char *, char *, int);
static void backslashify (const char *, char *, int);

struct symlink_info
{
  char contents[CYG_MAX_PATH + 4];
  char *ext_here;
  int extn;
  unsigned pflags;
  DWORD fileattr;
  int issymlink;
  bool ext_tacked_on;
  int error;
  bool case_clash;
  bool isdevice;
  _major_t major;
  _minor_t minor;
  _mode_t mode;
  int check (char *path, const suffix_info *suffixes, unsigned opt);
  int set (char *path);
  bool parse_device (const char *);
  bool case_check (char *path);
  int check_sysfile (const char *path, HANDLE h);
  int check_shortcut (const char *path, HANDLE h);
  bool set_error (int);
};

muto NO_COPY cwdstuff::cwd_lock;

int pcheck_case = PCHECK_RELAXED; /* Determines the case check behaviour. */

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

/* Determine if path prefix matches current cygdrive */
#define iscygdrive(path) \
  (path_prefix_p (mount_table->cygdrive, (path), mount_table->cygdrive_len))

#define iscygdrive_device(path) \
  (isalpha (path[mount_table->cygdrive_len]) && \
   (path[mount_table->cygdrive_len + 1] == '/' || \
    !path[mount_table->cygdrive_len + 1]))

#define isproc(path) \
  (path_prefix_p (proc, (path), proc_len))

#define isvirtual_dev(devn) \
  (devn == FH_CYGDRIVE || devn == FH_PROC || devn == FH_REGISTRY \
   || devn == FH_PROCESS || devn == FH_NETDRIVE )

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

int
path_prefix_p (const char *path1, const char *path2, int len1)
{
  /* Handle case where PATH1 has trailing '/' and when it doesn't.  */
  if (len1 > 0 && isdirsep (path1[len1 - 1]))
    len1--;

  if (len1 == 0)
    return isdirsep (path2[0]) && !isdirsep (path2[1]);

  if (isdirsep (path2[len1]) || path2[len1] == 0 || path1[len1 - 1] == ':')
    return pathnmatch (path1, path2, len1);

  return 0;
}

/* Return non-zero if paths match in first len chars.
   Check is dependent of the case sensitivity setting. */
int
pathnmatch (const char *path1, const char *path2, int len)
{
  return pcheck_case == PCHECK_STRICT ? !strncmp (path1, path2, len)
				      : strncasematch (path1, path2, len);
}

/* Return non-zero if paths match. Check is dependent of the case
   sensitivity setting. */
int
pathmatch (const char *path1, const char *path2)
{
  return pcheck_case == PCHECK_STRICT ? !strcmp (path1, path2)
				      : strcasematch (path1, path2);
}

/* TODO: This function is used in mkdir and rmdir to generate correct
   error messages in case of paths ending in /. or /.. components.
   This test should eventually end up in path_conv::check in one way
   or another.  Right now, normalize_posix_path will just normalize
   those components away, which changes the semantics.  */
bool
has_dot_last_component (const char *dir)
{
  /* SUSv3: . and .. are not allowed as last components in various system
     calls.  Don't test for backslash path separator since that's a Win32
     path following Win32 rules. */
  const char *last_comp = strrchr (dir, '/');
  return last_comp
	 && last_comp[1] == '.'
	 && (last_comp[2] == '\0'
	     || (last_comp[2] == '.' && last_comp[3] == '\0'));
}

#define isslash(c) ((c) == '/')

/* Normalize a POSIX path.
   All duplicate /'s, except for 2 leading /'s, are deleted.
   The result is 0 for success, or an errno error value.  */

static int
normalize_posix_path (const char *src, char *dst, char *&tail)
{
  const char *in_src = src;
  char *dst_start = dst;
  syscall_printf ("src %s", src);

  if (isdrive (src) || *src == '\\')
    goto win32_path;

  tail = dst;
  if (!isslash (src[0]))
    {
      if (!cygheap->cwd.get (dst))
	return get_errno ();
      tail = strchr (tail, '\0');
      if (isslash (dst[0]) && isslash (dst[1]))
	++dst_start;
      if (*src == '.')
	{
	  if (tail == dst_start + 1 && *dst_start == '/')
	     tail--;
	  goto sawdot;
	}
      if (tail > dst && !isslash (tail[-1]))
	*tail++ = '/';
    }
  /* Two leading /'s?  If so, preserve them.  */
  else if (isslash (src[1]) && !isslash (src[2]))
    {
      *tail++ = *src++;
      ++dst_start;
    }

  while (*src)
    {
      if (*src == '\\')
	goto win32_path;
      /* Strip runs of /'s.  */
      if (!isslash (*src))
	*tail++ = *src++;
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
		      *tail++ = '/';
		      goto done;
		    }
		  if (!isslash (src[1]))
		    break;
		}
	      else if (src[2] && !isslash (src[2]))
		break;
	      else
		{
		  while (tail > dst_start && !isslash (*--tail))
		    continue;
		  src++;
		}
	    }

	  *tail++ = '/';
	}
	if ((tail - dst) >= CYG_MAX_PATH)
	  {
	    debug_printf ("ENAMETOOLONG = normalize_posix_path (%s)", src);
	    return ENAMETOOLONG;
	  }
    }

done:
  *tail = '\0';

  debug_printf ("%s = normalize_posix_path (%s)", dst, in_src);
  return 0;

win32_path:
  int err = normalize_win32_path (in_src, dst, tail);
  if (!err)
    for (char *p = dst; (p = strchr (p, '\\')); p++)
      *p = '/';
  return err;
}

inline void
path_conv::add_ext_from_sym (symlink_info &sym)
{
  if (sym.ext_here && *sym.ext_here)
    {
      known_suffix = path + sym.extn;
      if (sym.ext_tacked_on)
	strcpy (known_suffix, sym.ext_here);
    }
}

static void __stdcall mkrelpath (char *dst) __attribute__ ((regparm (2)));
static void __stdcall
mkrelpath (char *path)
{
  char cwd_win32[CYG_MAX_PATH];
  if (!cygheap->cwd.get (cwd_win32, 0))
    return;

  unsigned cwdlen = strlen (cwd_win32);
  if (!path_prefix_p (cwd_win32, path, cwdlen))
    return;

  size_t n = strlen (path);
  if (n < cwdlen)
    return;

  char *tail = path;
  if (n == cwdlen)
    tail += cwdlen;
  else
    tail += isdirsep (cwd_win32[cwdlen - 1]) ? cwdlen : cwdlen + 1;

  memmove (path, tail, strlen (tail) + 1);
  if (!*path)
    strcpy (path, ".");
}

#define MAX_FS_INFO_CNT 25
fs_info fsinfo[MAX_FS_INFO_CNT];
LONG fsinfo_cnt;

bool
fs_info::update (const char *win32_path)
{
  char fsname [CYG_MAX_PATH];
  char root_dir [CYG_MAX_PATH];
  bool ret;

  if (!rootdir (win32_path, root_dir))
    {
      debug_printf ("Cannot get root component of path %s", win32_path);
      clear ();
      return false;
    }

  __ino64_t tmp_name_hash = hash_path_name (1, root_dir);
  if (tmp_name_hash == name_hash)
    return true;
  int idx = 0;
  LONG cur_fsinfo_cnt = fsinfo_cnt;
  while (idx < cur_fsinfo_cnt && fsinfo[idx].name_hash)
    {
      if (tmp_name_hash == fsinfo[idx].name_hash)
	{
	  *this = fsinfo[idx];
	  return true;
	}
      ++idx;
    }
  name_hash = tmp_name_hash;

  /* I have no idea why, but some machines require SeChangeNotifyPrivilege
     to access volume information. */
  push_thread_privilege (SE_CHANGE_NOTIFY_PRIV, true);

  drive_type (GetDriveType (root_dir));
  if (drive_type () == DRIVE_REMOTE
      || (drive_type () == DRIVE_UNKNOWN
	  && (root_dir[0] == '\\' && root_dir[1] == '\\')))
    is_remote_drive (true);
  else
    is_remote_drive (false);

  ret = GetVolumeInformation (root_dir, NULL, 0, &status.serial, NULL,
			      &status.flags, fsname, sizeof (fsname));

  pop_thread_privilege ();

  if (!ret && !is_remote_drive ())
    {
      debug_printf ("Cannot get volume information (%s), %E", root_dir);
      has_buggy_open (false);
      has_ea (false);
      flags () = serial () = 0;
      return false;
    }

/* Test only flags indicating capabilities, ignore flags indicating a specific
   state of the volume.  At present ignore FILE_VOLUME_IS_COMPRESSED and
   FILE_READ_ONLY_VOLUME. */
#define GETVOLINFO_VALID_MASK (0x003701ffUL)
/* Volume quotas are potentially supported since Samba 3.0, object ids and
   the unicode on disk flag since Samba 3.2. */
#define SAMBA_IGNORE (FILE_VOLUME_QUOTAS \
			| FILE_SUPPORTS_OBJECT_IDS \
			| FILE_UNICODE_ON_DISK)
#define SAMBA_REQIURED (FILE_CASE_SENSITIVE_SEARCH \
			| FILE_CASE_PRESERVED_NAMES \
			| FILE_PERSISTENT_ACLS)
#define FS_IS_SAMBA	((flags () & GETVOLINFO_VALID_MASK & ~SAMBA_IGNORE) \
			 ==  SAMBA_REQIURED)

  is_fat (strncasematch (fsname, "FAT", 3));
  is_samba (strcmp (fsname, "NTFS") == 0 && is_remote_drive () && FS_IS_SAMBA);
  is_ntfs (strcmp (fsname, "NTFS") == 0 && !is_samba ());
  is_nfs (strcmp (fsname, "NFS") == 0);

  has_ea (is_ntfs ());
  has_acls ((flags () & FS_PERSISTENT_ACLS)
	    && (allow_smbntsec || !is_remote_drive ()));
  hasgood_inode (((flags () & FILE_PERSISTENT_ACLS)
		  && drive_type () != DRIVE_UNKNOWN)
		 || is_nfs ());
  /* Known file systems with buggy open calls. Further explanation
     in fhandler.cc (fhandler_disk_file::open). */
  has_buggy_open (!strcmp (fsname, "SUNWNFS"));

  /* Only append non-removable drives to the global fsinfo storage */
  if (drive_type () != DRIVE_REMOVABLE && drive_type () != DRIVE_CDROM
      && idx < MAX_FS_INFO_CNT)
    {
      LONG exc_cnt;
      while ((exc_cnt = InterlockedExchange (&fsinfo_cnt, -1)) == -1)
	low_priority_sleep (0);
      if (exc_cnt < MAX_FS_INFO_CNT)
	{
	  /* Check if another thread has already appended that very drive */
	  while (idx < exc_cnt)
	    {
	      if (fsinfo[idx++].name_hash == name_hash)
		goto done;
	    }
	  fsinfo[exc_cnt++] = *this;
	}
     done:
      InterlockedExchange (&fsinfo_cnt, exc_cnt);
    }
  return true;
}

void
path_conv::fillin (HANDLE h)
{
  BY_HANDLE_FILE_INFORMATION local;
  if (!GetFileInformationByHandle (h, &local))
    {
      fileattr = INVALID_FILE_ATTRIBUTES;
      fs.serial () = 0;
    }
  else
    {
      fileattr = local.dwFileAttributes;
      fs.serial () = local.dwVolumeSerialNumber;
    }
    fs.drive_type (DRIVE_UNKNOWN);
}

void
path_conv::set_normalized_path (const char *path_copy, bool strip_tail)
{
  char *p = strchr (path_copy, '\0');

  if (strip_tail)
    {
      while (*--p == '.' || *p == ' ')
	continue;
      *++p = '\0';
    }

  size_t n = 1 + p - path_copy;

  normalized_path = path + sizeof (path) - n;

  char *eopath = strchr (path, '\0');
  if (normalized_path > eopath)
    normalized_path_size = n;
  else
    {
      normalized_path = (char *) cmalloc (HEAP_STR, n);
      normalized_path_size = 0;
    }

  memcpy (normalized_path, path_copy, n);
}

PUNICODE_STRING
path_conv::get_nt_native_path (UNICODE_STRING &upath)
{
  if (path[0] != '\\')			/* X:\...  or NUL, etc. */
    {
      str2uni_cat (upath, "\\??\\");
      str2uni_cat (upath, path);
    }
  else if (path[1] != '\\')		/* \Device\... */
    str2uni_cat (upath, path);
  else if (path[2] != '.'
	   || path[3] != '\\')		/* \\server\share\... */
    {
      str2uni_cat (upath, "\\??\\UNC\\");
      str2uni_cat (upath, path + 2);
    }
  else					/* \\.\device */
    {
      str2uni_cat (upath, "\\??\\");
      str2uni_cat (upath, path + 4);
    }
  return &upath;
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
  /* This array is used when expanding symlinks.  It is CYG_MAX_PATH * 2
     in length so that we can hold the expanded symlink plus a
     trailer.  */
  char path_copy[CYG_MAX_PATH + 3];
  char tmp_buf[2 * CYG_MAX_PATH + 3];
  symlink_info sym;
  bool need_directory = 0;
  bool saw_symlinks = 0;
  bool is_relpath;
  char *tail, *path_end;

#if 0
  static path_conv last_path_conv;
  static char last_src[CYG_MAX_PATH];

  if (*last_src && strcmp (last_src, src) == 0)
    {
      *this = last_path_conv;
      return;
    }
#endif

  myfault efault;
  if (efault.faulted ())
    {
      error = EFAULT;
      return;
    }
  int loop = 0;
  path_flags = 0;
  known_suffix = NULL;
  fileattr = INVALID_FILE_ATTRIBUTES;
  case_clash = false;
  memset (&dev, 0, sizeof (dev));
  fs.clear ();
  normalized_path = NULL;
  int component = 0;		// Number of translated components

  if (!(opt & PC_NULLEMPTY))
    error = 0;
  else if (!*src)
    {
      error = ENOENT;
      return;
    }

  /* This loop handles symlink expansion.  */
  for (;;)
    {
      MALLOC_CHECK;
      assert (src);

      is_relpath = !isabspath (src);
      error = normalize_posix_path (src, path_copy, tail);
      if (error)
	return;

      /* Detect if the user was looking for a directory.  We have to strip the
	 trailing slash initially while trying to add extensions but take it
	 into account during processing */
      if (tail > path_copy + 2 && isslash (tail[-1]))
	{
	  need_directory = 1;
	  *--tail = '\0';
	}
      path_end = tail;

      /* Scan path_copy from right to left looking either for a symlink
	 or an actual existing file.  If an existing file is found, just
	 return.  If a symlink is found, exit the for loop.
	 Also: be careful to preserve the errno returned from
	 symlink.check as the caller may need it. */
      /* FIXME: Do we have to worry about multiple \'s here? */
      component = 0;		// Number of translated components
      sym.contents[0] = '\0';

      int symlen = 0;

      for (unsigned pflags_or = opt & PC_NO_ACCESS_CHECK; ; pflags_or = 0)
	{
	  const suffix_info *suff;
	  char pathbuf[CYG_MAX_PATH];
	  char *full_path;

	  /* Don't allow symlink.check to set anything in the path_conv
	     class if we're working on an inner component of the path */
	  if (component)
	    {
	      suff = NULL;
	      sym.pflags = 0;
	      full_path = pathbuf;
	    }
	  else
	    {
	      suff = suffixes;
	      sym.pflags = path_flags;
	      full_path = this->path;
	    }

	  /* Convert to native path spec sans symbolic link info. */
	  error = mount_table->conv_to_win32_path (path_copy, full_path, dev,
						   &sym.pflags);

	  if (error)
	    return;

	  sym.pflags |= pflags_or;

	  if (dev.major == DEV_CYGDRIVE_MAJOR)
	    {
	      if (!component)
		fileattr = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY;
	      else
		{
		  fileattr = GetFileAttributes (this->path);
		  dev.devn = FH_FS;
		}
	      goto out;
	    }
	  else if (dev == FH_DEV)
	    {
	      dev.devn = FH_FS;
#if 0
	      fileattr = GetFileAttributes (this->path);
	      if (!component && fileattr == INVALID_FILE_ATTRIBUTES)
		{
		  fileattr = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY;
		  goto out;
		}
#endif
	    }
	  else if (isvirtual_dev (dev.devn))
	    {
	      /* FIXME: Calling build_fhandler here is not the right way to handle this. */
	      fhandler_virtual *fh = (fhandler_virtual *) build_fh_dev (dev, path_copy);
	      int file_type = fh->exists ();
	      if (file_type == -2)
		{
		  fh->fill_filebuf ();
		  symlen = sym.set (fh->get_filebuf ());
		}
	      delete fh;
	      switch (file_type)
		{
		  case 1:
		  case 2:
		    if (component == 0)
		      fileattr = FILE_ATTRIBUTE_DIRECTORY;
		    break;
		  case -1:
		    if (component == 0)
		      fileattr = 0;
		    break;
		  case -2:	/* /proc/self or /proc/<pid>/symlinks */
		    goto is_virtual_symlink;
		  case -3:	/* /proc/<pid>/fd/pipe:[] */
		    if (component == 0)
		      {
			fileattr = 0;
			dev.parse (FH_PIPE);
		      }
		    break;
		  case -4:	/* /proc/<pid>/fd/socket:[] */
		    if (component == 0)
		      {
			fileattr = 0;
			dev.parse (FH_TCP);
		      }
		    break;
		  default:
		    if (component == 0)
		      fileattr = INVALID_FILE_ATTRIBUTES;
		    goto virtual_component_retry;
		}
	      if (component == 0 || dev.devn != FH_NETDRIVE)
		path_flags |= PATH_RO;
	      goto out;
	    }
	  /* devn should not be a device.  If it is, then stop parsing now. */
	  else if (dev.devn != FH_FS)
	    {
	      fileattr = 0;
	      path_flags = sym.pflags;
	      if (component)
		{
		  error = ENOTDIR;
		  return;
		}
	      goto out;		/* Found a device.  Stop parsing. */
	    }

	  /* If path is only a drivename, Windows interprets it as the
	     current working directory on this drive instead of the root
	     dir which is what we want. So we need the trailing backslash
	     in this case. */
	  if (full_path[0] && full_path[1] == ':' && full_path[2] == '\0')
	    {
	      full_path[2] = '\\';
	      full_path[3] = '\0';
	    }

	  symlen = sym.check (full_path, suff, opt);

is_virtual_symlink:

	  if (sym.isdevice)
	    {
	      dev.parse (sym.major, sym.minor);
	      dev.setfs (1);
	      dev.mode = sym.mode;
	      fileattr = sym.fileattr;
	      goto out;
	    }

	  if (sym.pflags & PATH_SOCKET)
	    {
	      if (component)
		{
		  error = ENOTDIR;
		  return;
		}
	      fileattr = sym.fileattr;
	      dev.parse (FH_UNIX);
	      dev.setfs (1);
	      goto out;
	    }

	  if (sym.case_clash)
	    {
	      if (pcheck_case == PCHECK_STRICT)
		{
		  case_clash = true;
		  error = ENOENT;
		  goto out;
		}
	      /* If pcheck_case==PCHECK_ADJUST the case_clash is remembered
		 if the last component is concerned. This allows functions
		 which shall create files to avoid overriding already existing
		 files with another case. */
	      if (!component)
		case_clash = true;
	    }
	  if (!(opt & PC_SYM_IGNORE))
	    {
	      if (!component)
		{
		  fileattr = sym.fileattr;
		  path_flags = sym.pflags;
		}

	      /* If symlink.check found an existing non-symlink file, then
		 it sets the appropriate flag.  It also sets any suffix found
		 into `ext_here'. */
	      if (!sym.issymlink && sym.fileattr != INVALID_FILE_ATTRIBUTES)
		{
		  error = sym.error;
		  if (component == 0)
		    add_ext_from_sym (sym);
		  else if (!(sym.fileattr & FILE_ATTRIBUTE_DIRECTORY))
		    {
		      error = ENOTDIR;
		      goto out;
		    }
		  if (pcheck_case == PCHECK_RELAXED)
		    goto out;	// file found
		  /* Avoid further symlink evaluation. Only case checks are
		     done now. */
		  opt |= PC_SYM_IGNORE;
		}
	      /* Found a symlink if symlen > 0.  If component == 0, then the
		 src path itself was a symlink.  If !follow_mode then
		 we're done.  Otherwise we have to insert the path found
		 into the full path that we are building and perform all of
		 these operations again on the newly derived path. */
	      else if (symlen > 0)
		{
		  saw_symlinks = 1;
		  if (component == 0 && !need_directory && !(opt & PC_SYM_FOLLOW))
		    {
		      set_symlink (symlen); // last component of path is a symlink.
		      if (opt & PC_SYM_CONTENTS)
			{
			  strcpy (path, sym.contents);
			  goto out;
			}
		      add_ext_from_sym (sym);
		      if (pcheck_case == PCHECK_RELAXED)
			goto out;
		      /* Avoid further symlink evaluation. Only case checks are
			 done now. */
		      opt |= PC_SYM_IGNORE;
		    }
		  else
		    break;
		}
	      else if (sym.error && sym.error != ENOENT && sym.error != ENOSHARE)
		{
		  error = sym.error;
		  goto out;
		}
	      /* No existing file found. */
	    }

virtual_component_retry:
	  /* Find the new "tail" of the path, e.g. in '/for/bar/baz',
	     /baz is the tail. */
	  if (tail != path_end)
	    *tail = '/';
	  while (--tail > path_copy + 1 && *tail != '/') {}
	  /* Exit loop if there is no tail or we are at the
	     beginning of a UNC path */
	  if (tail <= path_copy + 1)
	    goto out;	// all done

	  /* Haven't found an existing pathname component yet.
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


      /* Place the link content, possibly with head and/or tail, in tmp_buf */

      char *headptr;
      if (isabspath (sym.contents))
	headptr = tmp_buf;	/* absolute path */
      else
	{
	  /* Copy the first part of the path (with ending /) and point to the end. */
	  char *prevtail = tail;
	  while (--prevtail > path_copy  && *prevtail != '/') {}
	  int headlen = prevtail - path_copy + 1;;
	  memcpy (tmp_buf, path_copy, headlen);
	  headptr = &tmp_buf[headlen];
	}

      /* Make sure there is enough space */
      if (headptr + symlen >= tmp_buf + sizeof (tmp_buf))
	{
	too_long:
	  error = ENAMETOOLONG;
	  strcpy (path, "::ENAMETOOLONG::");
	  return;
	}

     /* Copy the symlink contents to the end of tmp_buf.
	Convert slashes. */
      for (char *p = sym.contents; *p; p++)
	*headptr++ = *p == '\\' ? '/' : *p;
      *headptr = '\0';

      /* Copy any tail component (with the 0) */
      if (tail++ < path_end)
	{
	  /* Add a slash if needed. There is space. */
	  if (*(headptr - 1) != '/')
	    *headptr++ = '/';
	  int taillen = path_end - tail + 1;
	  if (headptr + taillen > tmp_buf + sizeof (tmp_buf))
	    goto too_long;
	  memcpy (headptr, tail, taillen);
	}

      /* Evaluate everything all over again. */
      src = tmp_buf;
    }

  if (!(opt & PC_SYM_CONTENTS))
    add_ext_from_sym (sym);

out:
  bool strip_tail = false;
  if (dev.devn == FH_NETDRIVE && component)
    {
      /* This case indicates a non-existant resp. a non-retrievable
	 share.  This happens for instance if the share is a printer.
	 In this case the path must not be treated like a FH_NETDRIVE,
	 but like a FH_FS instead, so the usual open call for files
	 is used on it. */
      dev.parse (FH_FS);
    }
  else if (isvirtual_dev (dev.devn) && fileattr == INVALID_FILE_ATTRIBUTES)
    {
      error = dev.devn == FH_NETDRIVE ? ENOSHARE : ENOENT;
      return;
    }
  else if (!need_directory || error)
    /* nothing to do */;
  else if (fileattr == INVALID_FILE_ATTRIBUTES)
    strcat (path, "\\");
  else if (fileattr & FILE_ATTRIBUTE_DIRECTORY)
    path_flags &= ~PATH_SYMLINK;
  else
    {
      debug_printf ("%s is a non-directory", path);
      error = ENOTDIR;
      return;
    }

  if (dev.isfs ())
    {
      if (strncmp (path, "\\\\.\\", 4))
	{
	  /* Windows ignores trailing dots and spaces in the last path
	     component, and ignores exactly one trailing dot in inner
	     path components. */
	  char *tail = NULL;
	  for (char *p = path; *p; p++)
	    {
	      if (*p != '.' && *p != ' ')
		tail = NULL;
	      else if (!tail)
		tail = p;
	      if (tail && p[1] == '\\')
		{
		  if (p > tail || *tail != '.')
		    {
		      error = ENOENT;
		      return;
		    }
		  tail = NULL;
		}
	    }

	  if (!tail || tail == path)
	    /* nothing */;
	  else if (tail[-1] != '\\')
	    {
	      *tail = '\0';
	      strip_tail = true;
	    }
	  else
	    {
	      error = ENOENT;
	      return;
	    }
	}

      if (fs.update (path))
	{
	  debug_printf ("this->path(%s), has_acls(%d)", path, fs.has_acls ());
	  if (fs.has_acls () && allow_ntsec)
	    set_exec (0);  /* We really don't know if this is executable or not here
			      but set it to not executable since it will be figured out
			      later by anything which cares about this. */
	}
      if (exec_state () != dont_know_if_executable)
	/* ok */;
      else if (isdir ())
	set_exec (1);
      else if (issymlink () || issocket ())
	set_exec (0);
    }

#if 0
  if (issocket ())
    devn = FH_SOCKET;
#endif

  if (opt & PC_NOFULL)
    {
      if (is_relpath)
	mkrelpath (this->path);
      if (need_directory)
	{
	  size_t n = strlen (this->path);
	  /* Do not add trailing \ to UNC device names like \\.\a: */
	  if (this->path[n - 1] != '\\' &&
	      (strncmp (this->path, "\\\\.\\", 4) != 0))
	    {
	      this->path[n] = '\\';
	      this->path[n + 1] = '\0';
	    }
	}
    }

  if (saw_symlinks)
    set_has_symlinks ();

  if (!error && !isdir () && !(path_flags & PATH_ALL_EXEC))
    {
      const char *p = strchr (path, '\0') - 4;
      if (p >= path &&
	  (strcasematch (".exe", p) ||
	   strcasematch (".bat", p) ||
	   strcasematch (".com", p)))
	path_flags |= PATH_EXEC;
    }

  if (!(opt & PC_POSIX))
    normalized_path_size = 0;
  else
    {
      if (tail < path_end && tail > path_copy + 1)
	*tail = '/';
      set_normalized_path (path_copy, strip_tail);
    }

#if 0
  if (!error)
    {
      last_path_conv = *this;
      strcpy (last_src, src);
    }
#endif
}

void
path_conv::set_name (const char *win32, const char *posix)
{
  if (!normalized_path_size && normalized_path)
    cfree (normalized_path);
  strcpy (path, win32);
  set_normalized_path (posix, false);
}

path_conv::~path_conv ()
{
  if (!normalized_path_size && normalized_path)
    {
      cfree (normalized_path);
      normalized_path = NULL;
    }
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

/* is_unc_share: Return non-zero if PATH begins with //UNC/SHARE */

static bool __stdcall
is_unc_share (const char *path)
{
  const char *p;
  return (isdirsep (path[0])
	 && isdirsep (path[1])
	 && (isalnum (path[2]) || path[2] == '.')
	 && ((p = strpbrk (path + 3, "\\/")) != NULL)
	 && isalnum (p[1]));
}

/* Normalize a Win32 path.
   /'s are converted to \'s in the process.
   All duplicate \'s, except for 2 leading \'s, are deleted.

   The result is 0 for success, or an errno error value.
   FIXME: A lot of this should be mergeable with the POSIX critter.  */
static int
normalize_win32_path (const char *src, char *dst, char *&tail)
{
  const char *src_start = src;
  bool beg_src_slash = isdirsep (src[0]);

  tail = dst;
  if (beg_src_slash && isdirsep (src[1]))
    {
      if (isdirsep (src[2]))
	{
	  /* More than two slashes are just folded into one. */
	  src += 2;
	  while (isdirsep (src[1]))
	    ++src;
	}
      else
	{
	  /* Two slashes start a network or device path. */
	  *tail++ = '\\';
	  src++;
	  if (src[1] == '.' && isdirsep (src[2]))
	    {
	      *tail++ = '\\';
	      *tail++ = '.';
	      src += 2;
	    }
	}
    }
  if (tail == dst && !isdrive (src) && *src != '/')
    {
      if (beg_src_slash)
	tail += cygheap->cwd.get_drive (dst);
      else if (!cygheap->cwd.get (dst, 0))
	return get_errno ();
      else
	{
	  tail = strchr (tail, '\0');
	  *tail++ = '\\';
	}
    }

  while (*src)
    {
      /* Strip duplicate /'s.  */
      if (isdirsep (src[0]) && isdirsep (src[1]))
	src++;
      /* Ignore "./".  */
      else if (src[0] == '.' && isdirsep (src[1])
	       && (src == src_start || isdirsep (src[-1])))
	src += 2;

      /* Backup if "..".  */
      else if (src[0] == '.' && src[1] == '.'
	       /* dst must be greater than dst_start */
	       && tail[-1] == '\\')
	{
	  if (!isdirsep (src[2]) && src[2] != '\0')
	      *tail++ = *src++;
	  else
	    {
	      /* Back up over /, but not if it's the first one.  */
	      if (tail > dst + 1)
		tail--;
	      /* Now back up to the next /.  */
	      while (tail > dst + 1 && tail[-1] != '\\' && tail[-2] != ':')
		tail--;
	      src += 2;
	      if (isdirsep (*src))
		src++;
	    }
	}
      /* Otherwise, add char to result.  */
      else
	{
	  if (*src == '/')
	    *tail++ = '\\';
	  else
	    *tail++ = *src;
	  src++;
	}
      if ((tail - dst) >= CYG_MAX_PATH)
	return ENAMETOOLONG;
    }
   if (tail > dst + 1 && tail[-1] == '.' && tail[-2] == '\\')
     tail--;
  *tail = '\0';
  debug_printf ("%s = normalize_win32_path (%s)", dst, src_start);
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
  while (len > 1 && isdirsep (dst[--len]))
    dst[len] = '\0';
}

/* conv_path_list: Convert a list of path names to/from Win32/POSIX. */

static int
conv_path_list (const char *src, char *dst, int to_posix)
{
  char src_delim, dst_delim;
  int (*conv_fn) (const char *, char *);

  if (to_posix)
    {
      src_delim = ';';
      dst_delim = ':';
      conv_fn = cygwin_conv_to_posix_path;
    }
  else
    {
      src_delim = ':';
      dst_delim = ';';
      conv_fn = cygwin_conv_to_win32_path;
    }

  char *srcbuf = (char *) alloca (strlen (src) + 1);

  int err = 0;
  char *d = dst - 1;
  do
    {
      char *s = strccpy (srcbuf, &src, src_delim);
      int len = s - srcbuf;
      if (len >= CYG_MAX_PATH)
	{
	  err = ENAMETOOLONG;
	  break;
	}
      if (len)
	err = conv_fn (srcbuf, ++d);
      else if (!to_posix)
	err = conv_fn (".", ++d);
      else
	continue;
      if (err)
	break;
      d = strchr (d, '\0');
      *d = dst_delim;
    }
  while (*src++);

  if (d < dst)
    d++;
  *d = '\0';
  return err;
}

/* init: Initialize the mount table.  */

void
mount_info::init ()
{
  nmounts = 0;

  /* Fetch the mount table and cygdrive-related information from
     the registry.  */
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
  if (strncasematch (s, "conin$", n = 5)
      || strncasematch (s, "conout$", n = 7)
      || strncasematch (s, "nul", n = 3)
      || strncasematch (s, "aux", 3)
      || strncasematch (s, "prn", 3)
      || strncasematch (s, "con", 3))
    p = s + n;
  else if (strncasematch (s, "com", 3) || strncasematch (s, "lpt", 3))
    strtoul (s + 3, (char **) &p, 10);
  if (p && (*p == '\0' || *p == '.'))
    return -1;

  return (strchr (s, '\0')[-1] == '.')
	 || (strpbrk (s, special_chars) && !strncasematch (s, "%2f", 3));
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
  if (!*p || !(flags & MOUNT_ENC))
    {
      if ((n + strlen (p)) >= CYG_MAX_PATH)
	err = ENAMETOOLONG;
      else
	backslashify (p, dst + n, 0);
    }
  else
    {
      int left = CYG_MAX_PATH - n;
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
  return err;
}

/* conv_to_win32_path: Ensure src_path is a pure Win32 path and store
   the result in win32_path.

   If win32_path != NULL, the relative path, if possible to keep, is
   stored in win32_path.  If the relative path isn't possible to keep,
   the full path is stored.

   If full_win32_path != NULL, the full path is stored there.

   The result is zero for success, or an errno value.

   {,full_}win32_path must have sufficient space (i.e. CYG_MAX_PATH bytes).  */

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
  /* If the path is on a network drive, bypass the mount table.
     If it's // or //MACHINE, use the netdrive device. */
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
			     char **mount_points)
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
	    mount_points[n_mounts++] = last_slash + 1;
	}
      else if (parent_dir_len == last_slash - mi->posix_path
	       && strncasematch (parent_dir, mi->posix_path, parent_dir_len))
	mount_points[n_mounts++] = last_slash + 1;
    }
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
   posix_path must have sufficient space (i.e. CYG_MAX_PATH bytes).
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
      trailing_slash_p = isdirsep (*lastchar) && lastchar[-1] != ':';
    }

  debug_printf ("conv_to_posix_path (%s, %s, %s)", src_path,
		keep_rel_p ? "keep-rel" : "no-keep-rel",
		trailing_slash_p ? "add-slash" : "no-add-slash");
  MALLOC_CHECK;

  if (src_path_len >= CYG_MAX_PATH)
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

  char pathbuf[CYG_MAX_PATH];
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
      if ((mi.posix_pathlen + (pathbuflen - mi.native_pathlen) + addslash) >= CYG_MAX_PATH)
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
      if (mi.flags & MOUNT_ENC)
	{
	  char tmpbuf[CYG_MAX_PATH];
	  if (fnunmunge (tmpbuf, posix_path))
	    strcpy (posix_path, tmpbuf);
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
      if (path_prefix_p (mi.native_path, p, mi.native_pathlen))
	return mi.flags;
    }
  return PATH_BINARY;
}

/* read_mounts: Given a specific regkey, read mounts from under its
   key. */

void
mount_info::read_mounts (reg_key& r)
{
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
      char native_path[CYG_MAX_PATH];
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
      subkey.get_string ("native", native_path, sizeof (native_path), "");
      mount_flags = subkey.get_int ("flags", 0);

      /* Add mount_item corresponding to registry mount point. */
      res = mount_table->add_item (native_path, posix_path, mount_flags, false);
      if (res && get_errno () == EMFILE)
	break; /* The number of entries exceeds MAX_MOUNTS */
    }
}

/* from_registry: Build the entire mount table from the registry.  Also,
   read in cygdrive-related information from its registry location. */

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

/* add_reg_mount: Add mount item to registry.  Return zero on success,
   non-zero on failure. */
/* FIXME: Need a mutex to avoid collisions with other tasks. */

int
mount_info::add_reg_mount (const char *native_path, const char *posix_path, unsigned mountflags)
{
  int res;

  /* Add the mount to the right registry location, depending on
     whether MOUNT_SYSTEM is set in the mount flags. */

  reg_key reg (mountflags & MOUNT_SYSTEM,  KEY_ALL_ACCESS,
	       CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);

  /* Start by deleting existing mount if one exists. */
  res = reg.kill (posix_path);
  if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND)
    {
 err:
      __seterrno_from_win_error (res);
      return -1;
    }

  /* Create the new mount. */
  reg_key subkey (reg.get_key (), KEY_ALL_ACCESS, posix_path, NULL);

  res = subkey.set_string ("native", native_path);
  if (res != ERROR_SUCCESS)
    goto err;
  res = subkey.set_int ("flags", mountflags);

  if (mountflags & MOUNT_SYSTEM)
    {
      sys_mount_table_counter++;
      cygwin_shared->sys_mount_table_counter++;
    }
  return 0; /* Success */
}

/* del_reg_mount: delete mount item from registry indicated in flags.
   Return zero on success, non-zero on failure.*/
/* FIXME: Need a mutex to avoid collisions with other tasks. */

int
mount_info::del_reg_mount (const char * posix_path, unsigned flags)
{
  int res;

  reg_key reg (flags & MOUNT_SYSTEM, KEY_ALL_ACCESS,
	       CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
  res = reg.kill (posix_path);

  if (res != ERROR_SUCCESS)
    {
      __seterrno_from_win_error (res);
      return -1;
    }

  if (flags & MOUNT_SYSTEM)
    {
      sys_mount_table_counter++;
      cygwin_shared->sys_mount_table_counter++;
    }

  return 0; /* Success */
}

/* read_cygdrive_info_from_registry: Read the default prefix and flags
   to use when creating cygdrives from the special user registry
   location used to store cygdrive information. */

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

/* write_cygdrive_info_to_registry: Write the default prefix and flags
   to use when creating cygdrives to the special user registry
   location used to store cygdrive information. */

int
mount_info::write_cygdrive_info_to_registry (const char *cygdrive_prefix, unsigned flags)
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

  char hold_cygdrive_prefix[strlen (cygdrive_prefix) + 1];
  /* Ensure that there is never a final slash */
  nofinalslash (cygdrive_prefix, hold_cygdrive_prefix);

  reg_key r (flags & MOUNT_SYSTEM, KEY_ALL_ACCESS,
	     CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
  int res;
  res = r.set_string (CYGWIN_INFO_CYGDRIVE_PREFIX, hold_cygdrive_prefix);
  if (res != ERROR_SUCCESS)
    {
      __seterrno_from_win_error (res);
      return -1;
    }
  r.set_int (CYGWIN_INFO_CYGDRIVE_FLAGS, flags);

  if (flags & MOUNT_SYSTEM)
    sys_mount_table_counter = ++cygwin_shared->sys_mount_table_counter;

  /* This also needs to go in the in-memory copy of "cygdrive", but only if
     appropriate:
       1. setting user path prefix, or
       2. overwriting (a previous) system path prefix */
  if (!(flags & MOUNT_SYSTEM) || (mount_table->cygdrive_flags & MOUNT_SYSTEM))
    {
      slashify (cygdrive_prefix, cygdrive, 1);
      cygdrive_flags = flags;
      cygdrive_len = strlen (cygdrive);
    }

  return 0;
}

int
mount_info::remove_cygdrive_info_from_registry (const char *cygdrive_prefix, unsigned flags)
{
  reg_key r (flags & MOUNT_SYSTEM, KEY_ALL_ACCESS,
	     CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME,
	     NULL);

  /* Delete cygdrive prefix and flags. */
  int res = r.killvalue (CYGWIN_INFO_CYGDRIVE_PREFIX);
  int res2 = r.killvalue (CYGWIN_INFO_CYGDRIVE_FLAGS);

  if (flags & MOUNT_SYSTEM)
    sys_mount_table_counter = ++cygwin_shared->sys_mount_table_counter;

  /* Reinitialize the cygdrive path prefix to reflect to removal from the
     registry. */
  read_cygdrive_info_from_registry ();

  if (res == ERROR_SUCCESS)
    res = res2;
  if (res == ERROR_SUCCESS)
    return 0;

  __seterrno_from_win_error (res);
  return -1;
}

int
mount_info::get_cygdrive_info (char *user, char *system, char* user_flags,
			       char* system_flags)
{
  /* Get the user path prefix from HKEY_CURRENT_USER. */
  reg_key r (false,  KEY_READ, CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
  int res = r.get_string (CYGWIN_INFO_CYGDRIVE_PREFIX, user, CYG_MAX_PATH, "");

  /* Get the user flags, if appropriate */
  if (res == ERROR_SUCCESS)
    {
      int flags = r.get_int (CYGWIN_INFO_CYGDRIVE_FLAGS, MOUNT_CYGDRIVE | MOUNT_BINARY);
      strcpy (user_flags, (flags & MOUNT_BINARY) ? "binmode" : "textmode");
    }

  /* Get the system path prefix from HKEY_LOCAL_MACHINE. */
  reg_key r2 (true,  KEY_READ, CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
  int res2 = r2.get_string (CYGWIN_INFO_CYGDRIVE_PREFIX, system, CYG_MAX_PATH, "");

  /* Get the system flags, if appropriate */
  if (res2 == ERROR_SUCCESS)
    {
      int flags = r2.get_int (CYGWIN_INFO_CYGDRIVE_FLAGS, MOUNT_CYGDRIVE | MOUNT_BINARY);
      strcpy (system_flags, (flags & MOUNT_BINARY) ? "binmode" : "textmode");
    }

  return (res != ERROR_SUCCESS) ? res : res2;
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
mount_info::add_item (const char *native, const char *posix, unsigned mountflags, int reg_p)
{
  char nativetmp[CYG_MAX_PATH];
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
      if (strcasematch (mount[i].posix_path, posixtmp) &&
	  (mount[i].flags & MOUNT_SYSTEM) == (mountflags & MOUNT_SYSTEM))
	break;
    }

  if (i == nmounts && nmounts == MAX_MOUNTS)
    {
      set_errno (EMFILE);
      return -1;
    }

  if (reg_p && add_reg_mount (nativetmp, posixtmp, mountflags))
    return -1;

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
mount_info::del_item (const char *path, unsigned flags, int reg_p)
{
  char pathtmp[CYG_MAX_PATH];
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

/************************* mount_item class ****************************/

static mntent *
fillout_mntent (const char *native_path, const char *posix_path, unsigned flags)
{
  struct mntent& ret=_my_tls.locals.mntbuf;

  /* Remove drivenum from list if we see a x: style path */
  if (strlen (native_path) == 2 && native_path[1] == ':')
    {
      int drivenum = cyg_tolower (native_path[0]) - 'a';
      if (drivenum >= 0 && drivenum <= 31)
	_my_tls.locals.available_drives &= ~(1 << drivenum);
    }

  /* Pass back pointers to mount_table strings reserved for use by
     getmntent rather than pointers to strings in the internal mount
     table because the mount table might change, causing weird effects
     from the getmntent user's point of view. */

  strcpy (_my_tls.locals.mnt_fsname, native_path);
  ret.mnt_fsname = _my_tls.locals.mnt_fsname;
  strcpy (_my_tls.locals.mnt_dir, posix_path);
  ret.mnt_dir = _my_tls.locals.mnt_dir;

  if (!(flags & MOUNT_SYSTEM))		/* user mount */
    strcpy (_my_tls.locals.mnt_type, (char *) "user");
  else					/* system mount */
    strcpy (_my_tls.locals.mnt_type, (char *) "system");

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
      res = mount_table->write_cygdrive_info_to_registry (posix_path, flags);
      win32_path = NULL;
    }
  else if (!*win32_path)
    set_errno (EINVAL);
  else
    res = mount_table->add_item (win32_path, posix_path, flags, true);

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

  if (flags & MOUNT_CYGDRIVE)
    {
      /* When flags include MOUNT_CYGDRIVE, take this to mean that we actually want
	 to remove the cygdrive prefix and flags without actually unmounting
	 anything. */
      res = mount_table->remove_cygdrive_info_from_registry (path, flags);
    }
  else
    {
      res = mount_table->del_item (path, flags, true);
    }

  syscall_printf ("%d = cygwin_umount (%s, %d)", res,  path, flags);
  return res;
}

bool
is_floppy (const char *dos)
{
  char dev[256];
  if (!QueryDosDevice (dos, dev, 256))
    return false;
  return strncasematch (dev, "\\Device\\Floppy", 14)
	 || strcasematch (dev, "A:");
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

/********************** Symbolic Link Support **************************/

/* Create a symlink from FROMPATH to TOPATH. */

/* If TRUE create symlinks as Windows shortcuts, if false create symlinks
   as normal files with magic number and system bit set. */
bool allow_winsymlinks = true;

extern "C" int
symlink (const char *oldpath, const char *newpath)
{
  return symlink_worker (oldpath, newpath, allow_winsymlinks, false);
}

int
symlink_worker (const char *oldpath, const char *newpath, bool use_winsym,
		bool isdevice)
{
  extern suffix_info stat_suffixes[];

  HANDLE h;
  int res = -1;
  path_conv win32_path, win32_oldpath;
  char from[CYG_MAX_PATH + 5];
  char cwd[CYG_MAX_PATH], *cp = NULL, c = 0;
  char w32oldpath[CYG_MAX_PATH];
  char reloldpath[CYG_MAX_PATH] = { 0 };
  DWORD written;
  SECURITY_ATTRIBUTES sa = sec_none_nih;
  security_descriptor sd;

  /* POSIX says that empty 'newpath' is invalid input while empty
     'oldpath' is valid -- it's symlink resolver job to verify if
     symlink contents point to existing filesystem object */
  myfault efault;
  if (efault.faulted (EFAULT))
    goto done;
  if (!*oldpath || !*newpath)
    {
      set_errno (ENOENT);
      goto done;
    }

  if (strlen (oldpath) >= CYG_MAX_PATH)
    {
      set_errno (ENAMETOOLONG);
      goto done;
    }

  win32_path.check (newpath, PC_SYM_NOFOLLOW,
		    transparent_exe ? stat_suffixes : NULL);
  if (use_winsym && !win32_path.exists ())
    {
      strcpy (from, newpath);
      strcat (from, ".lnk");
      win32_path.check (from, PC_SYM_NOFOLLOW);
    }

  if (win32_path.error)
    {
      set_errno (win32_path.case_clash ? ECASECLASH : win32_path.error);
      goto done;
    }

  syscall_printf ("symlink (%s, %s)", oldpath, win32_path.get_win32 ());

  if ((!isdevice && win32_path.exists ())
      || win32_path.is_auto_device ())
    {
      set_errno (EEXIST);
      goto done;
    }

  DWORD create_how;
  if (!use_winsym)
    create_how = CREATE_NEW;
  else if (isdevice)
    {
      strcpy (w32oldpath, oldpath);
      create_how = CREATE_ALWAYS;
      SetFileAttributes (win32_path, FILE_ATTRIBUTE_NORMAL);
    }
  else
    {
      if (!isabspath (oldpath))
	{
	  getcwd (cwd, CYG_MAX_PATH);
	  if ((cp = strrchr (from, '/')) || (cp = strrchr (from, '\\')))
	    {
	      c = *cp;
	      *cp = '\0';
	      chdir (from);
	    }
	  backslashify (oldpath, reloldpath, 0);
	  /* Creating an ITEMIDLIST requires an absolute path.  So if we
	     create a shortcut file, we create relative and absolute Win32
	     paths, the first for the relpath field and the latter for the
	     ITEMIDLIST field. */
	  if (GetFileAttributes (reloldpath) == INVALID_FILE_ATTRIBUTES)
	    {
	      win32_oldpath.check (oldpath, PC_SYM_NOFOLLOW,
				   transparent_exe ? stat_suffixes : NULL);
	      if (win32_oldpath.error != ENOENT)
		strcpy (use_winsym ? reloldpath : w32oldpath, win32_oldpath);
	    }
	  else if (!use_winsym)
	    strcpy (w32oldpath, reloldpath);
	  if (use_winsym)
	    {
	      win32_oldpath.check (oldpath, PC_SYM_NOFOLLOW,
				   transparent_exe ? stat_suffixes : NULL);
	      strcpy (w32oldpath, win32_oldpath);
	    }
	  if (cp)
	    {
	      *cp = c;
	      chdir (cwd);
	    }
	}
      else
	{
	  win32_oldpath.check (oldpath, PC_SYM_NOFOLLOW,
			       transparent_exe ? stat_suffixes : NULL);
	  strcpy (w32oldpath, win32_oldpath);
	}
      create_how = CREATE_NEW;
    }

  if (allow_ntsec && win32_path.has_acls ())
    set_security_attribute (S_IFLNK | STD_RBITS | STD_WBITS,
			    &sa, sd);

  h = CreateFile (win32_path, GENERIC_WRITE, 0, &sa, create_how,
		  FILE_ATTRIBUTE_NORMAL, 0);
  if (h == INVALID_HANDLE_VALUE)
    __seterrno ();
  else
    {
      bool success = false;

      if (use_winsym)
	{
	  /* A path of 240 chars with 120 one character directories in it
	     can result in a 6K shortcut. */
	  char *buf = (char *) alloca (8192);
	  win_shortcut_hdr *shortcut_header = (win_shortcut_hdr *) buf;
	  HRESULT hres;
	  IShellFolder *psl;
	  WCHAR wc_path[CYG_MAX_PATH];
	  ITEMIDLIST *pidl = NULL, *p;
	  unsigned short len;

	  memset (shortcut_header, 0, sizeof *shortcut_header);
	  shortcut_header->size = sizeof *shortcut_header;
	  shortcut_header->magic = GUID_shortcut;
	  shortcut_header->flags = (WSH_FLAG_DESC | WSH_FLAG_RELPATH);
	  shortcut_header->run = SW_NORMAL;
	  cp = buf + sizeof (win_shortcut_hdr);
	  /* Creating an IDLIST */
	  hres = SHGetDesktopFolder (&psl);
	  if (SUCCEEDED (hres))
	    {
	      MultiByteToWideChar (CP_ACP, 0, w32oldpath, -1, wc_path,
				   CYG_MAX_PATH);
	      hres = psl->ParseDisplayName (NULL, NULL, wc_path, NULL,
					    &pidl, NULL);
	      if (SUCCEEDED (hres))
		{
		  shortcut_header->flags |= WSH_FLAG_IDLIST;
		  for (p = pidl; p->mkid.cb > 0;
		       p = (ITEMIDLIST *)((char *) p + p->mkid.cb))
		    ;
		  len = (char *) p - (char *) pidl + 2;
		  *(unsigned short *)cp = len;
		  memcpy (cp += 2, pidl, len);
		  cp += len;
		  CoTaskMemFree (pidl);
		}
	      psl->Release ();
	    }
	  /* Creating a description */
	  *(unsigned short *)cp = len = strlen (oldpath);
	  memcpy (cp += 2, oldpath, len);
	  cp += len;
	  /* Creating a relpath */
	  if (reloldpath[0])
	    {
	      *(unsigned short *)cp = len = strlen (reloldpath);
	      memcpy (cp += 2, reloldpath, len);
	    }
	  else
	    {
	      *(unsigned short *)cp = len = strlen (w32oldpath);
	      memcpy (cp += 2, w32oldpath, len);
	    }
	  cp += len;
	  success = WriteFile (h, buf, cp - buf, &written, NULL)
		    && written == (DWORD) (cp - buf);
	}
      else
	{
	  /* This is the old technique creating a symlink. */
	  char buf[sizeof (SYMLINK_COOKIE) + CYG_MAX_PATH + 10];

	  __small_sprintf (buf, "%s%s", SYMLINK_COOKIE, oldpath);
	  DWORD len = strlen (buf) + 1;

	  /* Note that the terminating nul is written.  */
	  success = WriteFile (h, buf, len, &written, NULL)
		    || written != len;

	}
      if (success)
	{
	  CloseHandle (h);
	  if (!allow_ntsec && allow_ntea)
	    set_file_attribute (false, NULL, win32_path.get_win32 (),
				S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO);

	  DWORD attr = use_winsym ? FILE_ATTRIBUTE_READONLY
				  : FILE_ATTRIBUTE_SYSTEM;
#ifdef HIDDEN_DOT_FILES
	  cp = strrchr (win32_path, '\\');
	  if ((cp && cp[1] == '.') || *win32_path == '.')
	    attr |= FILE_ATTRIBUTE_HIDDEN;
#endif
	  SetFileAttributes (win32_path, attr);

	  res = 0;
	}
      else
	{
	  __seterrno ();
	  CloseHandle (h);
	  DeleteFileA (win32_path.get_win32 ());
	}
    }

done:
  syscall_printf ("%d = symlink_worker (%s, %s, %d, %d)", res, oldpath,
		  newpath, use_winsym, isdevice);
  return res;
}

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
symlink_info::check_shortcut (const char *path, HANDLE h)
{
  win_shortcut_hdr *file_header;
  char *buf, *cp;
  unsigned short len;
  int res = 0;
  DWORD size, got = 0;

  /* Valid Cygwin & U/WIN shortcuts are R/O. */
  if (!(fileattr & FILE_ATTRIBUTE_READONLY))
    goto file_not_symlink;

  if ((size = GetFileSize (h, NULL)) > 8192) /* Not a Cygwin symlink. */
    goto file_not_symlink;
  buf = (char *) alloca (size);
  if (!ReadFile (h, buf, size, &got, 0))
    {
      set_error (EIO);
      goto close_it;
    }
  file_header = (win_shortcut_hdr *) buf;
  if (got != size || !cmp_shortcut_header (file_header))
    goto file_not_symlink;
  cp = buf + sizeof (win_shortcut_hdr);
  if (file_header->flags & WSH_FLAG_IDLIST) /* Skip ITEMIDLIST */
    cp += *(unsigned short *) cp + 2;
  if ((len = *(unsigned short *) cp) == 0 || len >= CYG_MAX_PATH)
    goto file_not_symlink;
  strncpy (contents, cp += 2, len);
  contents[len] = '\0';
  res = len;
  if (res) /* It's a symlink.  */
    pflags = PATH_SYMLINK | PATH_LNK;
  goto close_it;

file_not_symlink:
  /* Not a symlink, see if executable.  */
  if (!(pflags & PATH_ALL_EXEC) && has_exec_chars ((const char *) &file_header, got))
    pflags |= PATH_EXEC;

close_it:
  CloseHandle (h);
  return res;
}


int
symlink_info::check_sysfile (const char *path, HANDLE h)
{
  char cookie_buf[sizeof (SYMLINK_COOKIE) - 1];
  DWORD got;
  int res = 0;

  if (!ReadFile (h, cookie_buf, sizeof (cookie_buf), &got, 0))
    {
      debug_printf ("ReadFile1 failed");
      set_error (EIO);
    }
  else if (got == sizeof (cookie_buf)
	   && memcmp (cookie_buf, SYMLINK_COOKIE, sizeof (cookie_buf)) == 0)
    {
      /* It's a symlink.  */
      pflags = PATH_SYMLINK;

      res = ReadFile (h, contents, CYG_MAX_PATH, &got, 0);
      if (!res)
	{
	  debug_printf ("ReadFile2 failed");
	  set_error (EIO);
	}
      else
	{
	  /* Versions prior to b16 stored several trailing
	     NULs with the path (to fill the path out to 1024
	     chars).  Current versions only store one trailing
	     NUL.  The length returned is the path without
	     *any* trailing NULs.  We also have to handle (or
	     at least not die from) corrupted paths.  */
	  char *end;
	  if ((end = (char *) memchr (contents, 0, got)) != NULL)
	    res = end - contents;
	  else
	    res = got;
	}
    }
  else if (got == sizeof (cookie_buf)
	   && memcmp (cookie_buf, SOCKET_COOKIE, sizeof (cookie_buf)) == 0)
    pflags |= PATH_SOCKET;
  else
    {
      /* Not a symlink, see if executable.  */
      if (pflags & PATH_ALL_EXEC)
	/* Nothing to do */;
      else if (has_exec_chars (cookie_buf, got))
	pflags |= PATH_EXEC;
      else
	pflags |= PATH_NOTEXEC;
      }
  syscall_printf ("%d = symlink.check_sysfile (%s, %s) (%p)",
		  res, path, contents, pflags);

  CloseHandle (h);
  return res;
}

enum
{
  SCAN_BEG,
  SCAN_LNK,
  SCAN_HASLNK,
  SCAN_JUSTCHECK,
  SCAN_APPENDLNK,
  SCAN_EXTRALNK,
  SCAN_DONE,
};

class suffix_scan
{
  const suffix_info *suffixes, *suffixes_start;
  int nextstate;
  char *eopath;
public:
  const char *path;
  char *has (const char *, const suffix_info *);
  int next ();
  int lnk_match () {return nextstate >= SCAN_APPENDLNK;}
};

char *
suffix_scan::has (const char *in_path, const suffix_info *in_suffixes)
{
  nextstate = SCAN_BEG;
  suffixes = suffixes_start = in_suffixes;

  char *ext_here = strrchr (in_path, '.');
  path = in_path;
  eopath = strchr (path, '\0');

  if (!ext_here)
    goto noext;

  if (suffixes)
    {
      /* Check if the extension matches a known extension */
      for (const suffix_info *ex = in_suffixes; ex->name != NULL; ex++)
	if (strcasematch (ext_here, ex->name))
	  {
	    nextstate = SCAN_JUSTCHECK;
	    suffixes = NULL;	/* Has an extension so don't scan for one. */
	    goto done;
	  }
    }

  /* Didn't match.  Use last resort -- .lnk. */
  if (strcasematch (ext_here, ".lnk"))
    {
      nextstate = SCAN_HASLNK;
      suffixes = NULL;
    }

 noext:
  ext_here = eopath;

 done:
  return ext_here;
}

int
suffix_scan::next ()
{
  for (;;)
    {
      if (!suffixes)
	switch (nextstate)
	  {
	  case SCAN_BEG:
	    suffixes = suffixes_start;
	    if (!suffixes)
	      {
		nextstate = SCAN_LNK;
		return 1;
	      }
	    nextstate = SCAN_EXTRALNK;
	    /* fall through to suffix checking below */
	    break;
	  case SCAN_HASLNK:
	    nextstate = SCAN_APPENDLNK;	/* Skip SCAN_BEG */
	    return 1;
	  case SCAN_EXTRALNK:
	    nextstate = SCAN_DONE;
	    *eopath = '\0';
	    return 0;
	  case SCAN_JUSTCHECK:
	    nextstate = SCAN_LNK;
	    return 1;
	  case SCAN_LNK:
	  case SCAN_APPENDLNK:
	    strcat (eopath, ".lnk");
	    nextstate = SCAN_DONE;
	    return 1;
	  default:
	    *eopath = '\0';
	    return 0;
	  }

      while (suffixes && suffixes->name)
	if (nextstate == SCAN_EXTRALNK && !suffixes->addon)
	  suffixes++;
	else
	  {
	    strcpy (eopath, suffixes->name);
	    if (nextstate == SCAN_EXTRALNK)
	      strcat (eopath, ".lnk");
	    suffixes++;
	    return 1;
	  }
      suffixes = NULL;
    }
}

bool
symlink_info::set_error (int in_errno)
{
  bool res;
  if (!(pflags & PATH_NO_ACCESS_CHECK) || in_errno == ENAMETOOLONG || in_errno == EIO)
    {
      error = in_errno;
      res = true;
    }
  else if (in_errno == ENOENT)
    res = true;
  else
    {
      fileattr = FILE_ATTRIBUTE_NORMAL;
      res = false;
    }
  return res;
}

bool
symlink_info::parse_device (const char *contents)
{
  char *endptr;
  _major_t mymajor;
  _major_t myminor;
  _mode_t mymode;

  mymajor = strtol (contents += 2, &endptr, 16);
  if (endptr == contents)
    return isdevice = false;

  contents = endptr;
  myminor = strtol (++contents, &endptr, 16);
  if (endptr == contents)
    return isdevice = false;

  contents = endptr;
  mymode = strtol (++contents, &endptr, 16);
  if (endptr == contents)
    return isdevice = false;

  if ((mymode & S_IFMT) == S_IFIFO)
    {
      mymajor = _major (FH_FIFO);
      myminor = _minor (FH_FIFO);
    }

  major = mymajor;
  minor = myminor;
  mode = mymode;
  return isdevice = true;
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
symlink_info::check (char *path, const suffix_info *suffixes, unsigned opt)
{
  HANDLE h;
  int res = 0;
  suffix_scan suffix;
  contents[0] = '\0';

  issymlink = true;
  isdevice = false;
  ext_here = suffix.has (path, suffixes);
  extn = ext_here - path;
  major = 0;
  minor = 0;
  mode = 0;
  pflags &= ~(PATH_SYMLINK | PATH_LNK);
  case_clash = false;

  while (suffix.next ())
    {
      error = 0;
      fileattr = GetFileAttributes (suffix.path);
      if (fileattr == INVALID_FILE_ATTRIBUTES)
	{
	  /* The GetFileAttributes call can fail for reasons that don't
	     matter, so we just return 0.  For example, getting the
	     attributes of \\HOST will typically fail.  */
	  debug_printf ("GetFileAttributes (%s) failed", suffix.path);

	  /* The above comment is not *quite* right.  When calling
	     GetFileAttributes for a non-existant file an a Win9x share,
	     GetLastError returns ERROR_INVALID_FUNCTION.  Go figure!
	     Also, GetFileAttributes fails with ERROR_SHARING_VIOLATION
	     if the file is locked exclusively by another process.
	     If we don't special handle this here, the file is accidentally
	     treated as non-existant. */
	  DWORD win_error = GetLastError ();
	  if (win_error == ERROR_INVALID_FUNCTION)
	    win_error = ERROR_FILE_NOT_FOUND;
	  else if (win_error == ERROR_SHARING_VIOLATION)
	    {
	      ext_tacked_on = !!*ext_here;
	      fileattr = 0;
	      goto file_not_symlink;
	    }
	  if (set_error (geterrno_from_win_error (win_error, EACCES)))
	    continue;
	}

      ext_tacked_on = !!*ext_here;

      if (pcheck_case != PCHECK_RELAXED && !case_check (path)
	  || (opt & PC_SYM_IGNORE))
	goto file_not_symlink;

      int sym_check;

      sym_check = 0;

      if (fileattr & FILE_ATTRIBUTE_DIRECTORY)
	goto file_not_symlink;

      /* Windows shortcuts are treated as symlinks. */
      if (suffix.lnk_match ())
	sym_check = 1;

      /* This is the old Cygwin method creating symlinks: */
      /* A symlink will have the `system' file attribute. */
      /* Only files can be symlinks (which can be symlinks to directories). */
      if (fileattr & FILE_ATTRIBUTE_SYSTEM)
	sym_check = 2;

      if (!sym_check)
	goto file_not_symlink;

      /* Open the file.  */

      h = CreateFile (suffix.path, GENERIC_READ, FILE_SHARE_READ,
		      &sec_none_nih, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
      res = -1;
      if (h == INVALID_HANDLE_VALUE)
	goto file_not_symlink;

      switch (sym_check)
	{
	case 1:
	  res = check_shortcut (suffix.path, h);
	  if (!res)
	    /* check more below */;
	  else if (contents[0] == ':' && contents[1] == '\\' && parse_device (contents))
	    goto file_not_symlink;
	  else
	    break;
	  /* If searching for `foo' and then finding a `foo.lnk' which is
	     no shortcut, return the same as if file not found. */
	  if (!suffix.lnk_match () || !ext_tacked_on)
	    goto file_not_symlink;

	  fileattr = INVALID_FILE_ATTRIBUTES;
	  continue;		/* in case we're going to tack *another* .lnk on this filename. */
	case 2:
	  res = check_sysfile (suffix.path, h);
	  if (!res)
	    goto file_not_symlink;
	  break;
	}
      break;

    file_not_symlink:
      issymlink = false;
      syscall_printf ("%s", isdevice ? "is a device" : "not a symlink");
      res = 0;
      break;
    }

  syscall_printf ("%d = symlink.check (%s, %p) (%p)",
		  res, suffix.path, contents, pflags);
  return res;
}

/* "path" is the path in a virtual symlink.  Set a symlink_info struct from
   that and proceed with further path checking afterwards. */
int
symlink_info::set (char *path)
{
  strcpy (contents, path);
  pflags = PATH_SYMLINK;
  fileattr = FILE_ATTRIBUTE_NORMAL;
  error = 0;
  issymlink = true;
  isdevice = false;
  ext_tacked_on = case_clash = false;
  ext_here = NULL;
  extn = major = minor = mode = 0;
  return strlen (path);
}

/* Check the correct case of the last path component (given in DOS style).
   Adjust the case in this->path if pcheck_case == PCHECK_ADJUST or return
   false if pcheck_case == PCHECK_STRICT.
   Dont't call if pcheck_case == PCHECK_RELAXED.
*/

bool
symlink_info::case_check (char *path)
{
  WIN32_FIND_DATA data;
  HANDLE h;
  char *c;

  /* Set a pointer to the beginning of the last component. */
  if (!(c = strrchr (path, '\\')))
    c = path;
  else
    ++c;

  if ((h = FindFirstFile (path, &data))
      != INVALID_HANDLE_VALUE)
    {
      FindClose (h);

      /* If that part of the component exists, check the case. */
      if (strncmp (c, data.cFileName, strlen (data.cFileName)))
	{
	  case_clash = true;

	  /* If check is set to STRICT, a wrong case results
	     in returning a ENOENT. */
	  if (pcheck_case == PCHECK_STRICT)
	    return false;

	  /* PCHECK_ADJUST adjusts the case in the incoming
	     path which points to the path in *this. */
	  strcpy (c, data.cFileName);
	}
    }
  return true;
}

/* readlink system call */

extern "C" int
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

  if (!pathbuf.exists ())
    {
      set_errno (ENOENT);
      return -1;
    }

  if (!pathbuf.issymlink ())
    {
      if (pathbuf.exists ())
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

__ino64_t __stdcall
hash_path_name (__ino64_t hash, const char *name)
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
	  nn = newname;
	  *nn = isupper (*name) ? cyg_tolower (*name) : *name;
	  *++nn = ':';
	  name += 2;
	  if (*name != '\\')
	    *++nn = '\\';
	  strcpy (++nn, name);
	  name = newname;
	  goto hashit;
	}

      /* Fill out the hashed path name with the current working directory if
	 this is not an absolute path and there is no pre-specified hash value.
	 Otherwise the inodes same will differ depending on whether a file is
	 referenced with an absolute value or relatively. */

      if (!hash && !isabspath (name))
	{
	  hash = cygheap->cwd.get_hash ();
	  if (name[0] == '.' && name[1] == '\0')
	    return hash;
	  hash = '\\' + (hash << 6) + (hash << 16) - hash;
	}
    }

hashit:
  /* Build up hash. Name is already normalized */
  do
    {
      int ch = cyg_tolower (*name);
      hash = ch + (hash << 6) + (hash << 16) - hash;
    }
  while (*++name != '\0');
  return hash;
}

char *
getcwd (char *buf, size_t ulen)
{
  char* res = NULL;
  myfault efault;
  if (efault.faulted (EFAULT))
      /* errno set */;
  else if (ulen == 0 && buf)
    set_errno (EINVAL);
  else
    res = cygheap->cwd.get (buf, 1, 1, ulen);
  return res;
}

/* getwd: standards? */
extern "C" char *
getwd (char *buf)
{
  return getcwd (buf, CYG_MAX_PATH);
}

/* chdir: POSIX 5.2.1.1 */
extern "C" int
chdir (const char *in_dir)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (!*in_dir)
    {
      set_errno (ENOENT);
      return -1;
    }

  syscall_printf ("dir '%s'", in_dir);

  /* Convert path.  First argument ensures that we don't check for NULL/empty/invalid
     again. */
  path_conv path (PC_NONULLEMPTY, in_dir, PC_SYM_FOLLOW | PC_POSIX);
  if (path.error)
    {
      set_errno (path.error);
      syscall_printf ("-1 = chdir (%s)", in_dir);
      return -1;
    }

  int res = -1;
  bool doit = false;
  const char *native_dir = path, *posix_cwd = NULL;
  int devn = path.get_devn ();
  if (!isvirtual_dev (devn))
    {
      /* Check to see if path translates to something like C:.
	 If it does, append a \ to the native directory specification to
	 defeat the Windows 95 (i.e. MS-DOS) tendency of returning to
	 the last directory visited on the given drive. */
      if (isdrive (native_dir) && !native_dir[2])
	{
	  path.get_win32 ()[2] = '\\';
	  path.get_win32 ()[3] = '\0';
	}
      /* The sequence chdir("xx"); chdir(".."); must be a noop if xx
	 is not a symlink. This is exploited by find.exe.
	 The posix_cwd is just path.normalized_path.
	 In other cases we let cwd.set obtain the Posix path through
	 the mount table. */
      if (!isdrive(path.normalized_path))
	posix_cwd = path.normalized_path;
      res = 0;
      doit = true;
    }
  else if (!path.exists ())
    set_errno (ENOENT);
  else if (!path.isdir ())
    set_errno (ENOTDIR);
  else
   {
     posix_cwd = path.normalized_path;
     res = 0;
   }

  if (!res)
    res = cygheap->cwd.set (native_dir, posix_cwd, doit);

  /* Note that we're accessing cwd.posix without a lock here.  I didn't think
     it was worth locking just for strace. */
  syscall_printf ("%d = chdir() cygheap->cwd.posix '%s' native '%s'", res,
		  cygheap->cwd.posix, native_dir);
  MALLOC_CHECK;
  return res;
}

extern "C" int
fchdir (int fd)
{
  int res;
  cygheap_fdget cfd (fd);
  if (cfd >= 0)
    res = chdir (cfd->get_name ());
  else
    res = -1;

  syscall_printf ("%d = fchdir (%d)", res, fd);
  return res;
}

/******************** Exported Path Routines *********************/

/* Cover functions to the path conversion routines.
   These are exported to the world as cygwin_foo by cygwin.din.  */

#define return_with_errno(x) \
  do {\
    int err = (x);\
    if (!err)\
     return 0;\
    set_errno (err);\
    return -1;\
  } while (0)

extern "C" int
cygwin_conv_to_win32_path (const char *path, char *win32_path)
{
  path_conv p (path, PC_SYM_FOLLOW | PC_NO_ACCESS_CHECK | PC_NOFULL);
  if (p.error)
    {
      win32_path[0] = '\0';
      set_errno (p.error);
      return -1;
    }


  strcpy (win32_path, strcmp ((char *) p, ".\\") == 0 ? "." : (char *) p);
  return 0;
}

extern "C" int
cygwin_conv_to_full_win32_path (const char *path, char *win32_path)
{
  path_conv p (path, PC_SYM_FOLLOW | PC_NO_ACCESS_CHECK);
  if (p.error)
    {
      win32_path[0] = '\0';
      set_errno (p.error);
      return -1;
    }

  strcpy (win32_path, p);
  return 0;
}

/* This is exported to the world as cygwin_foo by cygwin.din.  */

extern "C" int
cygwin_conv_to_posix_path (const char *path, char *posix_path)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (!*path)
    {
      set_errno (ENOENT);
      return -1;
    }
  return_with_errno (mount_table->conv_to_posix_path (path, posix_path, 1));
}

extern "C" int
cygwin_conv_to_full_posix_path (const char *path, char *posix_path)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (!*path)
    {
      set_errno (ENOENT);
      return -1;
    }
  return_with_errno (mount_table->conv_to_posix_path (path, posix_path, 0));
}

/* The realpath function is supported on some UNIX systems.  */

extern "C" char *
realpath (const char *path, char *resolved)
{
  extern suffix_info stat_suffixes[];

  /* Make sure the right errno is returned if path is NULL. */
  if (!path)
    {
      set_errno (EINVAL);
      return NULL;
    }

  /* Guard reading from a potentially invalid path and writing to a
     potentially invalid resolved. */
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

  char *tpath;
  if (isdrive (path))
    {
      tpath = (char *) alloca (strlen (path)
			       + strlen (mount_table->cygdrive)
			       + 1);
      mount_table->cygdrive_posix_path (path, tpath, 0);
    }
  else
    tpath = (char *) path;

  path_conv real_path (tpath, PC_SYM_FOLLOW | PC_POSIX, stat_suffixes);


  /* Linux has this funny non-standard extension.  If "resolved" is NULL,
     realpath mallocs the space by itself and returns it to the application.
     The application is responsible for calling free() then.  This extension
     is backed by POSIX, which allows implementation-defined behaviour if
     "resolved" is NULL.  That's good enough for us to do the same here. */

  if (!real_path.error && real_path.exists ())
    {
      /* Check for the suffix being tacked on. */
      int tack_on = 0;
      if (!transparent_exe && real_path.known_suffix)
	{
	  char *c = strrchr (real_path.normalized_path, '.');
	  if (!c || !strcasematch (c, real_path.known_suffix))
	    tack_on = strlen (real_path.known_suffix);
	}

      if (!resolved)
	{
	  resolved = (char *) malloc (strlen (real_path.normalized_path)
				      + tack_on + 1);
	  if (!resolved)
	    return NULL;
	}
      strcpy (resolved, real_path.normalized_path);
      if (tack_on)
	strcat (resolved, real_path.known_suffix);
      return resolved;
    }

  /* FIXME: on error, we are supposed to put the name of the path
     component which could not be resolved into RESOLVED.  */
  if (resolved)
    resolved[0] = '\0';
  set_errno (real_path.error ?: ENOENT);
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

extern "C" int
cygwin_posix_path_list_p (const char *path)
{
  int posix_p = !(strchr (path, ';') || isdrive (path));
  return posix_p;
}

/* These are used for apps that need to convert env vars like PATH back and
   forth.  The conversion is a two step process.  First, an upper bound on the
   size of the buffer needed is computed.  Then the conversion is done.  This
   allows the caller to use alloca if it wants.  */

static int
conv_path_list_buf_size (const char *path_list, bool to_posix)
{
  int i, num_elms, max_mount_path_len, size;
  const char *p;

  path_conv pc(".", PC_POSIX);
  /* The theory is that an upper bound is
     current_size + (num_elms * max_mount_path_len)  */

  unsigned nrel;
  char delim = to_posix ? ';' : ':';
  for (p = path_list, num_elms = nrel = 0; p; num_elms++)
    {
      if (!isabspath (p))
	nrel++;
      p = strchr (++p, delim);
    }

  /* 7: strlen ("//c") + slop, a conservative initial value */
  for (max_mount_path_len = sizeof ("/cygdrive/X"), i = 0;
       i < mount_table->nmounts; i++)
    {
      int mount_len = (to_posix
		       ? mount_table->mount[i].posix_pathlen
		       : mount_table->mount[i].native_pathlen);
      if (max_mount_path_len < mount_len)
	max_mount_path_len = mount_len;
    }

  /* 100: slop */
  size = strlen (path_list)
    + (num_elms * max_mount_path_len)
    + (nrel * strlen (to_posix ? pc.normalized_path : pc.get_win32 ()))
    + 100;

  return size;
}


extern "C" int
cygwin_win32_to_posix_path_list_buf_size (const char *path_list)
{
  return conv_path_list_buf_size (path_list, true);
}

extern "C" int
cygwin_posix_to_win32_path_list_buf_size (const char *path_list)
{
  return conv_path_list_buf_size (path_list, false);
}

extern "C" int
cygwin_win32_to_posix_path_list (const char *win32, char *posix)
{
  return_with_errno (conv_path_list (win32, posix, 1));
}

extern "C" int
cygwin_posix_to_win32_path_list (const char *posix, char *win32)
{
  return_with_errno (conv_path_list (posix, win32, 0));
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

extern "C" void
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
      if (!*path)
	{
	  *dir = 0;
	  *file = 0;
	  return;
	}
      if (isdirsep (*path))
	++path;
      dir_started_p = 1;
    }

  /* Determine if there are trailing slashes and "delete" them if present.
     We pretend as if they don't exist.  */
  const char *end = path + strlen (path);
  /* path + 1: keep leading slash.  */
  while (end > path + 1 && isdirsep (end[-1]))
    --end;

  /* At this point, END points to one beyond the last character
     (with trailing slashes "deleted").  */

  /* Point LAST_SLASH at the last slash (duh...).  */
  const char *last_slash;
  for (last_slash = end - 1; last_slash >= path; --last_slash)
    if (isdirsep (*last_slash))
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

/*****************************************************************************/

/* Return the hash value for the current win32 value.
   This is used when constructing inodes. */
DWORD
cwdstuff::get_hash ()
{
  DWORD hashnow;
  cwd_lock.acquire ();
  hashnow = hash;
  cwd_lock.release ();
  return hashnow;
}

/* Initialize cygcwd 'muto' for serializing access to cwd info. */
void
cwdstuff::init ()
{
  cwd_lock.init ("cwd_lock");
}

/* Get initial cwd.  Should only be called once in a
   process tree. */
bool
cwdstuff::get_initial ()
{
  cwd_lock.acquire ();

  if (win32)
    return 1;

  /* Leaves cwd lock unreleased, if success */
  return !set (NULL, NULL, false);
}

/* Chdir and fill out the elements of a cwdstuff struct.
   It is assumed that the lock for the cwd is acquired if
   win32_cwd == NULL. */
int
cwdstuff::set (const char *win32_cwd, const char *posix_cwd, bool doit)
{
  char pathbuf[2 * CYG_MAX_PATH];
  int res = -1;

  if (win32_cwd)
    {
       cwd_lock.acquire ();
       if (doit && !SetCurrentDirectory (win32_cwd))
	 {
	    /* When calling SetCurrentDirectory for a non-existant dir on a
	       Win9x share, it returns ERROR_INVALID_FUNCTION. */
	    if (GetLastError () == ERROR_INVALID_FUNCTION)
	      set_errno (ENOENT);
	    else
	      __seterrno ();
	    goto out;
	 }
    }
  /* If there is no win32 path or it has the form c:xxx, get the value */
  if (!win32_cwd || (isdrive (win32_cwd) && win32_cwd[2] != '\\'))
    {
      int i;
      DWORD len, dlen;
      for (i = 0, dlen = CYG_MAX_PATH/3; i < 2; i++, dlen = len)
	{
	  win32 = (char *) crealloc (win32, dlen);
	  if ((len = GetCurrentDirectoryA (dlen, win32)) < dlen)
	    break;
	}
      if (len == 0)
	{
	  __seterrno ();
	  debug_printf ("GetCurrentDirectory, %E");
	  win32_cwd = pathbuf; /* Force lock release */
	  goto out;
	}
      posix_cwd = NULL;
    }
  else
    {
      win32 = (char *) crealloc (win32, strlen (win32_cwd) + 1);
      strcpy (win32, win32_cwd);
    }
  if (win32[1] == ':')
    drive_length = 2;
  else if (win32[1] == '\\')
    {
      char * ptr = strechr (win32 + 2, '\\');
      if (*ptr)
	ptr = strechr (ptr + 1, '\\');
      drive_length = ptr - win32;
    }
  else
    drive_length = 0;

  if (!posix_cwd)
    {
      mount_table->conv_to_posix_path (win32, pathbuf, 0);
      posix_cwd = pathbuf;
    }
  posix = (char *) crealloc (posix, strlen (posix_cwd) + 1);
  strcpy (posix, posix_cwd);

  hash = hash_path_name (0, win32);

  res = 0;
out:
  if (win32_cwd)
    cwd_lock.release ();
  return res;
}

/* Copy the value for either the posix or the win32 cwd into a buffer. */
char *
cwdstuff::get (char *buf, int need_posix, int with_chroot, unsigned ulen)
{
  MALLOC_CHECK;

  if (ulen)
    /* nothing */;
  else if (buf == NULL)
    ulen = (unsigned) -1;
  else
    {
      set_errno (EINVAL);
      goto out;
    }

  if (!get_initial ())	/* Get initial cwd and set cwd lock */
    return NULL;

  char *tocopy;
  if (!need_posix)
    tocopy = win32;
  else
    tocopy = posix;

  debug_printf ("posix %s", posix);
  if (strlen (tocopy) >= ulen)
    {
      set_errno (ERANGE);
      buf = NULL;
    }
  else
    {
      if (!buf)
	buf = (char *) malloc (strlen (tocopy) + 1);
      strcpy (buf, tocopy);
      if (!buf[0])	/* Should only happen when chroot */
	strcpy (buf, "/");
    }

  cwd_lock.release ();

out:
  syscall_printf ("(%s) = cwdstuff::get (%p, %d, %d, %d), errno %d",
		  buf, buf, ulen, need_posix, with_chroot, errno);
  MALLOC_CHECK;
  return buf;
}

int etc::curr_ix = 0;
/* Note that the first elements of the below arrays are unused */
bool etc::change_possible[MAX_ETC_FILES + 1];
const char *etc::fn[MAX_ETC_FILES + 1];
FILETIME etc::last_modified[MAX_ETC_FILES + 1];

int
etc::init (int n, const char *etc_fn)
{
  if (n > 0)
    /* ok */;
  else if (++curr_ix <= MAX_ETC_FILES)
    n = curr_ix;
  else
    api_fatal ("internal error");

  fn[n] = etc_fn;
  change_possible[n] = false;
  test_file_change (n);
  paranoid_printf ("fn[%d] %s, curr_ix %d", n, fn[n], curr_ix);
  return n;
}

bool
etc::test_file_change (int n)
{
  HANDLE h;
  WIN32_FIND_DATA data;
  bool res;

  if ((h = FindFirstFile (fn[n], &data)) == INVALID_HANDLE_VALUE)
    {
      res = true;
      memset (last_modified + n, 0, sizeof (last_modified[n]));
      debug_printf ("FindFirstFile failed, %E");
    }
  else
    {
      FindClose (h);
      res = CompareFileTime (&data.ftLastWriteTime, last_modified + n) > 0;
      last_modified[n] = data.ftLastWriteTime;
      debug_printf ("FindFirstFile succeeded");
    }

  paranoid_printf ("fn[%d] %s res %d", n, fn[n], res);
  return res;
}

bool
etc::dir_changed (int n)
{
  if (!change_possible[n])
    {
      static HANDLE changed_h NO_COPY;

      if (!changed_h)
	{
	  path_conv pwd ("/etc");
	  changed_h = FindFirstChangeNotification (pwd, FALSE,
						  FILE_NOTIFY_CHANGE_LAST_WRITE
						  | FILE_NOTIFY_CHANGE_FILE_NAME);
#ifdef DEBUGGING
	  if (changed_h == INVALID_HANDLE_VALUE)
	    system_printf ("Can't open %s for checking, %E", (char *) pwd);
#endif
	  memset (change_possible, true, sizeof (change_possible));
	}

      if (changed_h == INVALID_HANDLE_VALUE)
	change_possible[n] = true;
      else if (WaitForSingleObject (changed_h, 0) == WAIT_OBJECT_0)
	{
	  FindNextChangeNotification (changed_h);
	  memset (change_possible, true, sizeof change_possible);
	}
    }

  paranoid_printf ("fn[%d] %s change_possible %d", n, fn[n], change_possible[n]);
  return change_possible[n];
}

bool
etc::file_changed (int n)
{
  bool res = false;
  if (dir_changed (n) && test_file_change (n))
    res = true;
  change_possible[n] = false;	/* Change is no longer possible */
  paranoid_printf ("fn[%d] %s res %d", n, fn[n], res);
  return res;
}

/* No need to be reentrant or thread-safe according to SUSv3.
   / and \\ are treated equally.  Leading drive specifiers are
   kept intact as far as it makes sense.  Everything else is
   POSIX compatible. */
extern "C" char *
basename (char *path)
{
  static char buf[CYG_MAX_PATH];
  char *c, *d, *bs = buf;

  if (!path || !*path)
    return strcpy (buf, ".");
  strncpy (buf, path, CYG_MAX_PATH);
  if (isalpha (buf[0]) && buf[1] == ':')
    bs += 2;
  else if (strspn (buf, "/\\") > 1)
    ++bs;
  c = strrchr (bs, '/');
  if ((d = strrchr (c ?: bs, '\\')) > c)
    c = d;
  if (c)
    {
      /* Trailing (back)slashes are eliminated. */
      while (c && c > bs && c[1] == '\0')
	{
	  *c = '\0';
	  c = strrchr (bs, '/');
	  if ((d = strrchr (c ?: bs, '\\')) > c)
	    c = d;
	}
      if (c && (c > bs || c[1]))
	return c + 1;
    }
  else if (!bs[0])
    strcpy (bs, ".");
  return buf;
}

/* No need to be reentrant or thread-safe according to SUSv3.
   / and \\ are treated equally.  Leading drive specifiers and
   leading double (back)slashes are kept intact as far as it
   makes sense.  Everything else is POSIX compatible. */
extern "C" char *
dirname (char *path)
{
  static char buf[CYG_MAX_PATH];
  char *c, *d, *bs = buf;

  if (!path || !*path)
    return strcpy (buf, ".");
  strncpy (buf, path, CYG_MAX_PATH);
  if (isalpha (buf[0]) && buf[1] == ':')
    bs += 2;
  else if (strspn (buf, "/\\") > 1)
    ++bs;
  c = strrchr (bs, '/');
  if ((d = strrchr (c ?: bs, '\\')) > c)
    c = d;
  if (c)
    {
      /* Trailing (back)slashes are eliminated. */
      while (c && c > bs && c[1] == '\0')
	{
	  *c = '\0';
	  c = strrchr (bs, '/');
	  if ((d = strrchr (c ?: bs, '\\')) > c)
	    c = d;
	}
      if (!c)
	strcpy (bs, ".");
      else if (c > bs)
	{
	  /* More trailing (back)slashes are eliminated. */
	  while (c > bs && (*c == '/' || *c == '\\'))
	    *c-- = '\0';
	}
      else
	c[1] = '\0';
    }
  else
    strcpy (bs, ".");
  return buf;
}
