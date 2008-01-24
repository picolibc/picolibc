/* path.cc: path support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

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
#include "environ.h"
#include <assert.h>
#include <ntdll.h>
#include <wchar.h>
#include <wctype.h>

bool dos_file_warning = true;
static int normalize_win32_path (const char *, char *, char *&);
static void slashify (const char *, char *, int);
static void backslashify (const char *, char *, int);

struct symlink_info
{
  char contents[SYMLINK_MAX + 1];
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
  int check_sysfile (HANDLE h);
  int check_shortcut (HANDLE h);
  int check_reparse_point (HANDLE h);
  int posixify (char *srcbuf);
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
   Right now, normalize_posix_path will just normalize
   those components away, which changes the semantics.  */
bool
has_dot_last_component (const char *dir, bool test_dot_dot)
{
  /* SUSv3: . and .. are not allowed as last components in various system
     calls.  Don't test for backslash path separator since that's a Win32
     path following Win32 rules. */
  const char *last_comp = strrchr (dir, '/');
  if (!last_comp)
    last_comp = dir;
  else {
    /* Check for trailing slash.  If so, hop back to the previous slash. */
    if (!last_comp[1])
      while (last_comp > dir)
	if (*--last_comp == '/')
	  break;
    if (*last_comp == '/')
      ++last_comp;
  }
  return last_comp[0] == '.'
	 && ((last_comp[1] == '\0' || last_comp[1] == '/')
	     || (test_dot_dot
		 && last_comp[1] == '.'
		 && (last_comp[2] == '\0' || last_comp[2] == '/')));
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
  return err ?: -1;
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

/* Beginning with Samba 3.2, Samba allows to get version information using
   the ExtendedInfo member returned by a FileFsObjectIdInformation request.
   We just store the samba_version information for now.  Older versions than
   3.2 are still guessed at by testing the file system flags. */
#define SAMBA_EXTENDED_INFO_MAGIC 0x536d4261 /* "SmBa" */
#define SAMBA_EXTENDED_INFO_VERSION_STRING_LENGTH 28
#pragma pack(push,4)
struct smb_extended_info {
  DWORD         samba_magic;             /* Always SAMBA_EXTENDED_INFO_MAGIC */
  DWORD         samba_version;           /* Major/Minor/Release/Revision */
  DWORD         samba_subversion;        /* Prerelease/RC/Vendor patch */
  LARGE_INTEGER samba_gitcommitdate;
  char          samba_version_string[SAMBA_EXTENDED_INFO_VERSION_STRING_LENGTH];
};
#pragma pack(pop)

bool
fs_info::update (PUNICODE_STRING upath, bool exists)
{
  NTSTATUS status = STATUS_OBJECT_NAME_NOT_FOUND;
  HANDLE vol;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  bool no_media = false;
  FILE_FS_DEVICE_INFORMATION ffdi;
  FILE_FS_OBJECTID_INFORMATION ffoi;
  PFILE_FS_ATTRIBUTE_INFORMATION pffai;
  UNICODE_STRING fsname, testname;

  InitializeObjectAttributes (&attr, upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  if (exists)
    status = NtOpenFile (&vol, READ_CONTROL, &attr, &io, FILE_SHARE_VALID_FLAGS,
			 FILE_OPEN_FOR_BACKUP_INTENT);
  while (!NT_SUCCESS (status)
	 && (attr.ObjectName->Length > 7 * sizeof (WCHAR)
	     || status == STATUS_NO_MEDIA_IN_DEVICE))
    {
      UNICODE_STRING dir;
      RtlSplitUnicodePath (attr.ObjectName, &dir, NULL);
      attr.ObjectName = &dir;
      if (status == STATUS_NO_MEDIA_IN_DEVICE)
        {
	  no_media = true;
	  dir.Length = 6 * sizeof (WCHAR);
	}
      else if (dir.Length > 7 * sizeof (WCHAR))
	dir.Length -= sizeof (WCHAR);
      status = NtOpenFile (&vol, READ_CONTROL, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, FILE_OPEN_FOR_BACKUP_INTENT);
    }
  if (!NT_SUCCESS (status))
    {
      debug_printf ("Cannot access path %S, status %08lx", attr.ObjectName,
							   status);
      clear ();
      NtClose (vol);
      return false;
    }
  status = NtQueryVolumeInformationFile (vol, &io, &ffdi, sizeof ffdi,
  					 FileFsDeviceInformation);
  if (!NT_SUCCESS (status))
    ffdi.DeviceType = ffdi.Characteristics = 0;

  if (ffdi.Characteristics & FILE_REMOTE_DEVICE
      || (!ffdi.DeviceType
	  && RtlEqualUnicodePathPrefix (attr.ObjectName, L"\\??\\UNC\\", TRUE)))
    is_remote_drive (true);
  else
    is_remote_drive (false);

  if (!no_media)
    {
      const ULONG size = sizeof (FILE_FS_ATTRIBUTE_INFORMATION)
			 + NAME_MAX * sizeof (WCHAR);
      pffai = (PFILE_FS_ATTRIBUTE_INFORMATION) alloca (size);
      status = NtQueryVolumeInformationFile (vol, &io, pffai, size,
					     FileFsAttributeInformation);
    }
  if (no_media || !NT_SUCCESS (status))
    {
      debug_printf ("Cannot get volume attributes (%S), %08lx",
		    attr.ObjectName, status);
      has_buggy_open (false);
      flags (0);
      NtClose (vol);
      return false;
    }
   flags (pffai->FileSystemAttributes);
/* Should be reevaluated for each new OS.  Right now this mask is valid up
   to Vista.  The important point here is to test only flags indicating
   capabilities and to ignore flags indicating a specific state of this
   volume.  At present these flags to ignore are FILE_VOLUME_IS_COMPRESSED
   and FILE_READ_ONLY_VOLUME. */
#define GETVOLINFO_VALID_MASK (0x003701ffUL)
#define TEST_GVI(f,m) (((f) & GETVOLINFO_VALID_MASK) == (m))

/* Volume quotas are potentially supported since Samba 3.0, object ids and
   the unicode on disk flag since Samba 3.2. */
#define SAMBA_IGNORE (FILE_VOLUME_QUOTAS \
		      | FILE_SUPPORTS_OBJECT_IDS \
		      | FILE_UNICODE_ON_DISK)
#define FS_IS_SAMBA TEST_GVI(flags () & ~SAMBA_IGNORE, \
			     FILE_CASE_SENSITIVE_SEARCH \
			     | FILE_CASE_PRESERVED_NAMES \
			     | FILE_PERSISTENT_ACLS)
#define FS_IS_NETAPP_DATAONTAP TEST_GVI(flags (), \
			     FILE_CASE_SENSITIVE_SEARCH \
			     | FILE_CASE_PRESERVED_NAMES \
			     | FILE_UNICODE_ON_DISK \
			     | FILE_PERSISTENT_ACLS \
			     | FILE_NAMED_STREAMS)
  RtlInitCountedUnicodeString (&fsname, pffai->FileSystemName,
			       pffai->FileSystemNameLength);
  is_fat (RtlEqualUnicodePathPrefix (&fsname, L"FAT", TRUE));
  RtlInitUnicodeString (&testname, L"NTFS");
  if (is_remote_drive ())
    {
      /* This always fails on NT4. */
      status = NtQueryVolumeInformationFile (vol, &io, &ffoi, sizeof ffoi,
					     FileFsObjectIdInformation);
      if (NT_SUCCESS (status))
	{
	  smb_extended_info *extended_info = (smb_extended_info *)
					     &ffoi.ExtendedInfo;
	  if (extended_info->samba_magic == SAMBA_EXTENDED_INFO_MAGIC)
	    {
	      is_samba (true);
	      samba_version (extended_info->samba_version);
	    }
	}
      /* Test for Samba on NT4 or for older Samba releases not supporting
	 extended info. */
      if (!is_samba ())
	is_samba (RtlEqualUnicodeString (&fsname, &testname, FALSE)
		  && FS_IS_SAMBA);

      is_netapp (!is_samba ()
		 && RtlEqualUnicodeString (&fsname, &testname, FALSE)
		 && FS_IS_NETAPP_DATAONTAP);
    }
  is_ntfs (RtlEqualUnicodeString (&fsname, &testname, FALSE)
	   && !is_samba () && !is_netapp ());
  RtlInitUnicodeString (&testname, L"NFS");
  is_nfs (RtlEqualUnicodeString (&fsname, &testname, FALSE));
  is_cdrom (ffdi.DeviceType == FILE_DEVICE_CD_ROM);

  has_acls ((flags () & FS_PERSISTENT_ACLS)
	    && (allow_smbntsec || !is_remote_drive ()));
  hasgood_inode (((flags () & FILE_PERSISTENT_ACLS) && !is_netapp ())
		 || is_nfs ());
  /* Known file systems with buggy open calls. Further explanation
     in fhandler.cc (fhandler_disk_file::open). */
  RtlInitUnicodeString (&testname, L"SUNWNFS");
  has_buggy_open (RtlEqualUnicodeString (&fsname, &testname, FALSE));

  NtClose (vol);
  return true;
}

void
path_conv::fillin (HANDLE h)
{
  IO_STATUS_BLOCK io;
  FILE_BASIC_INFORMATION fbi;

  if (NT_SUCCESS (NtQueryInformationFile (h, &io, &fbi, sizeof fbi,
					  FileBasicInformation)))
    fileattr = fbi.FileAttributes;
  else
    fileattr = INVALID_FILE_ATTRIBUTES;
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
      normalized_path = (char *) cmalloc_abort (HEAP_STR, n);
      normalized_path_size = 0;
    }

  memcpy (normalized_path, path_copy, n);
}

PUNICODE_STRING
get_nt_native_path (const char *path, UNICODE_STRING& upath)
{
  upath.Length = 0;
  if (path[0] == '/')		/* special path w/o NT path representation. */
    str2uni_cat (upath, path);
  else if (path[0] != '\\')	/* X:\...  or NUL, etc. */
    {
      str2uni_cat (upath, "\\??\\");
      str2uni_cat (upath, path);
    }
  else if (path[1] != '\\')	/* \Device\... */
    str2uni_cat (upath, path);
  else if ((path[2] != '.' && path[2] != '?')
	   || path[3] != '\\')	/* \\server\share\... */
    {
      str2uni_cat (upath, "\\??\\UNC\\");
      str2uni_cat (upath, path + 2);
    }
  else				/* \\.\device or \\?\foo */
    {
      str2uni_cat (upath, "\\??\\");
      str2uni_cat (upath, path + 4);
    }
  return &upath;
}

PUNICODE_STRING
path_conv::get_nt_native_path ()
{
  if (!wide_path)
    {
      uni_path.Length = 0;
      uni_path.MaximumLength = (strlen (path) + 10) * sizeof (WCHAR);
      wide_path = (PWCHAR) cmalloc_abort (HEAP_STR, uni_path.MaximumLength);
      uni_path.Buffer = wide_path;
      ::get_nt_native_path (path, uni_path);
    }
  return &uni_path;
}

POBJECT_ATTRIBUTES
path_conv::get_object_attr (OBJECT_ATTRIBUTES &attr, SECURITY_ATTRIBUTES &sa)
{
  if (!get_nt_native_path ())
    return NULL;
  InitializeObjectAttributes (&attr, &uni_path,
			      OBJ_CASE_INSENSITIVE
			      | (sa.bInheritHandle ? OBJ_INHERIT : 0),
			      NULL, sa.lpSecurityDescriptor);
  return &attr;
}

PWCHAR
path_conv::get_wide_win32_path (PWCHAR wc)
{
  get_nt_native_path ();
  if (!wide_path || wide_path[1] != L'?') /* Native NT device path */
    return NULL;
  wcscpy (wc, wide_path);
  wc[1] = L'\\';
  return wc;
}

void
warn_msdos (const char *src)
{
  if (user_shared->warned_msdos || !dos_file_warning)
    return;
  char posix_path[CYG_MAX_PATH];
  small_printf ("cygwin warning:\n");
  if (cygwin_conv_to_full_posix_path (src, posix_path))
    small_printf ("  MS-DOS style path detected: %s\n  POSIX equivalent preferred.\n",
		  src);
  else
    small_printf ("  MS-DOS style path detected: %s\n  Preferred POSIX equivalent is: %s\n",
		  src, posix_path);
  small_printf ("  CYGWIN environment variable option \"nodosfilewarning\" turns off this warning.\n"
		"  Consult the user's guide for more details about POSIX paths:\n"
		"    http://cygwin.com/cygwin-ug-net/using.html#using-pathnames\n");

  user_shared->warned_msdos = true;
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

/* TODO: This implementation is only preliminary.  For internal
   purposes it's necessary to have a path_conv::check function which
   takes a UNICODE_STRING src path, otherwise we waste a lot of time
   for converting back and forth.  The below implementation does
   realy nothing but converting to char *, until path_conv handles
   wide-char paths directly. */
void
path_conv::check (PUNICODE_STRING src, unsigned opt,
		  const suffix_info *suffixes)
{
  char path[CYG_MAX_PATH];

  user_shared->warned_msdos = true;
  sys_wcstombs (path, CYG_MAX_PATH, src->Buffer, src->Length / 2);
  path_conv::check (path, opt, suffixes);
}

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
  wide_path = NULL;
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

  bool is_msdos = false;
  /* This loop handles symlink expansion.  */
  for (;;)
    {
      MALLOC_CHECK;
      assert (src);

      is_relpath = !isabspath (src);
      error = normalize_posix_path (src, path_copy, tail);
      if (error > 0)
	return;
      if (error < 0)
	{
	  if (component == 0)
	    is_msdos = true;
	  error = 0;
	}

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
      if (++loop > SYMLOOP_MAX)
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

      if (fs.update (get_nt_native_path (), exists ()))
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
      if (is_msdos && !(opt & PC_NOWARN))
	warn_msdos (src);
    }

#if 0
  if (!error)
    {
      last_path_conv = *this;
      strcpy (last_src, src);
    }
#endif
}

path_conv::~path_conv ()
{
  if (!normalized_path_size && normalized_path)
    {
      cfree (normalized_path);
      normalized_path = NULL;
    }
  if (wide_path)
    {
      cfree (wide_path);
      wide_path = NULL;
    }
}

bool
path_conv::is_binary ()
{
  DWORD bin;
  PBYTE bintest[get_nt_native_path ()->Length + sizeof (WCHAR)];
  return exec_state () == is_executable
	 && RtlEqualUnicodePathSuffix (get_nt_native_path (), L".exe", TRUE)
	 && GetBinaryTypeW (get_wide_win32_path ((PWCHAR) bintest), &bin);
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
  bool saw_empty = false;
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
	{
	  if (to_posix == ENV_CVT)
	    saw_empty = true;
	  continue;
	}
      if (err)
	break;
      d = strchr (d, '\0');
      *d = dst_delim;
    }
  while (*src++);

  if (saw_empty)
    err = EIDRM;

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
   posix_path must have sufficient space (i.e. CYG_MAX_PATH bytes).
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
  char buf[PATH_MAX];
  sys_wcstombs (buf, PATH_MAX, src_path);
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
  if (user_flags && res == ERROR_SUCCESS)
    {
      int flags = r.get_int (CYGWIN_INFO_CYGDRIVE_FLAGS, MOUNT_CYGDRIVE | MOUNT_BINARY);
      strcpy (user_flags, (flags & MOUNT_BINARY) ? "binmode" : "textmode");
    }

  /* Get the system path prefix from HKEY_LOCAL_MACHINE. */
  reg_key r2 (true,  KEY_READ, CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
  int res2 = r2.get_string (CYGWIN_INFO_CYGDRIVE_PREFIX, system, CYG_MAX_PATH, "");

  /* Get the system flags, if appropriate */
  if (system_flags && res2 == ERROR_SUCCESS)
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
  UNICODE_STRING unat;
  /* Size must allow prepending the native NT path prefixes. */
  size_t size = (strlen (native_path) + 10) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&unat, (PWSTR) alloca (size), size);
  get_nt_native_path (native_path, unat);
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
  return strncasematch (dev, "\\Device\\Floppy", 14);
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
  int res = -1;
  size_t len;
  path_conv win32_newpath, win32_oldpath;
  char *buf, *cp;
  SECURITY_ATTRIBUTES sa = sec_none_nih;
  security_descriptor sd;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  HANDLE fh;
  FILE_BASIC_INFORMATION fbi;

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

  if (strlen (oldpath) > SYMLINK_MAX)
    {
      set_errno (ENAMETOOLONG);
      goto done;
    }

  len = strlen (newpath);
  /* Trailing dirsep is a no-no. */
  if (isdirsep (newpath[len - 1]))
    {
      set_errno (ENOENT);
      goto done;
    }
  /* We need the normalized full path below. */
  win32_newpath.check (newpath, PC_SYM_NOFOLLOW | PC_POSIX, stat_suffixes);
  if (use_winsym && !win32_newpath.exists ())
    {
      char *newplnk = (char *) alloca (len + 5);
      stpcpy (stpcpy (newplnk, newpath), ".lnk");
      win32_newpath.check (newplnk, PC_SYM_NOFOLLOW | PC_POSIX);
    }

  if (win32_newpath.error)
    {
      set_errno (win32_newpath.case_clash ? ECASECLASH : win32_newpath.error);
      goto done;
    }

  syscall_printf ("symlink (%s, %S)", oldpath,
  		  win32_newpath.get_nt_native_path ());

  if ((!isdevice && win32_newpath.exists ())
      || win32_newpath.is_auto_device ())
    {
      set_errno (EEXIST);
      goto done;
    }

  if (use_winsym)
    {
      ITEMIDLIST *pidl = NULL;
      size_t full_len = 0;
      unsigned short oldpath_len, desc_len, relpath_len, pidl_len = 0;
      char desc[MAX_PATH + 1], *relpath;

      if (!isdevice)
        {
	  /* First create an IDLIST to learn how big our shortcut is
	     going to be. */
	  IShellFolder *psl;
	  
	  /* The symlink target is relative to the directory in which
	     the symlink gets created, not relative to the cwd.  Therefore
	     we have to mangle the path quite a bit before calling path_conv. */
	  if (!isabspath (oldpath))
	    {
	      len = strrchr (win32_newpath.normalized_path, '/')
		    - win32_newpath.normalized_path + 1;
	      char *absoldpath = (char *) alloca (len + strlen (oldpath) + 1);
	      stpcpy (stpncpy (absoldpath, win32_newpath.normalized_path, len),
		      oldpath);
	      win32_oldpath.check (absoldpath, PC_SYM_NOFOLLOW, stat_suffixes);
	    }
	  else
	    win32_oldpath.check (oldpath, PC_SYM_NOFOLLOW, stat_suffixes);
	  if (SUCCEEDED (SHGetDesktopFolder (&psl)))
	    {
	      WCHAR wc_path[win32_oldpath.get_wide_win32_path_len () + 1];
	      win32_oldpath.get_wide_win32_path (wc_path);
	      /* Amazing but true:  Even though the ParseDisplayName method
		 takes a wide char path name, it does not understand the
		 Win32 prefix for long pathnames!  So we have to tack off
		 the prefix and convert tyhe path to the "normal" syntax
		 for ParseDisplayName.  I have no idea if it's able to take
		 long path names at all since I can't test it right now. */
	      WCHAR *wc = wc_path + 4;
	      if (!wcscmp (wc, L"UNC\\"))
		*(wc += 2) = L'\\';
	      HRESULT res;
	      if (SUCCEEDED (res = psl->ParseDisplayName (NULL, NULL, wc, NULL,
						    &pidl, NULL)))
		{
		  ITEMIDLIST *p;

		  for (p = pidl; p->mkid.cb > 0;
		       p = (ITEMIDLIST *)((char *) p + p->mkid.cb))
		    ;
		  pidl_len = (char *) p - (char *) pidl + 2;
		}
	      psl->Release ();
	    }
	}
      /* Compute size of shortcut file. */
      full_len = sizeof (win_shortcut_hdr);
      if (pidl_len)
	full_len += sizeof (unsigned short) + pidl_len;
      oldpath_len = strlen (oldpath);
      /* Unfortunately the length of the description is restricted to a
	 length of MAX_PATH up to NT4, and to a length of 2000 bytes
	 since W2K.  We don't want to add considerations for the different
	 lengths and even 2000 bytes is not enough for long path names.
	 So what we do here is to set the description to the POSIX path
	 only if the path is not longer than MAX_PATH characters.  We
	 append the full path name after the regular shortcut data
	 (see below), which works fine with Windows Explorer as well
	 as older Cygwin versions (as long as the whole file isn't bigger
	 than 8K).  The description field is only used for backward
	 compatibility to older Cygwin versions and those versions are
	 not capable of handling long path names anyway. */
      desc_len = stpcpy (desc, oldpath_len > MAX_PATH
			       ? "[path too long]" : oldpath) - desc;
      full_len += sizeof (unsigned short) + desc_len;
      /* Devices get the oldpath string unchanged as relative path. */
      if (isdevice)
	{
	  relpath_len = oldpath_len;
	  stpcpy (relpath = (char *) alloca (relpath_len + 1), oldpath);
	}
      else
	{
	  relpath_len = strlen (win32_oldpath.get_win32 ());
	  stpcpy (relpath = (char *) alloca (relpath_len + 1),
		  win32_oldpath.get_win32 ());
	}
      full_len += sizeof (unsigned short) + relpath_len;
      full_len += sizeof (unsigned short) + oldpath_len;
      /* 1 byte more for trailing 0 written by stpcpy. */
      buf = (char *) alloca (full_len + 1);

      /* Create shortcut header */
      win_shortcut_hdr *shortcut_header = (win_shortcut_hdr *) buf;
      memset (shortcut_header, 0, sizeof *shortcut_header);
      shortcut_header->size = sizeof *shortcut_header;
      shortcut_header->magic = GUID_shortcut;
      shortcut_header->flags = (WSH_FLAG_DESC | WSH_FLAG_RELPATH);
      if (pidl)
	shortcut_header->flags |= WSH_FLAG_IDLIST;
      shortcut_header->run = SW_NORMAL;
      cp = buf + sizeof (win_shortcut_hdr);

      /* Create IDLIST */
      if (pidl)
	{
	  *(unsigned short *)cp = pidl_len;
	  memcpy (cp += 2, pidl, pidl_len);
	  cp += pidl_len;
	  CoTaskMemFree (pidl);
	}

      /* Create description */
      *(unsigned short *)cp = desc_len;
      cp = stpcpy (cp += 2, desc);

      /* Create relpath */
      *(unsigned short *)cp = relpath_len;
      cp = stpcpy (cp += 2, relpath);

      /* Append the POSIX path after the regular shortcut data for
	 the long path support. */
      *(unsigned short *)cp = oldpath_len;
      cp = stpcpy (cp += 2, oldpath);
    }
  else
    {
      /* This is the old technique creating a symlink. */
      unsigned short oldpath_len = strlen (oldpath);
      buf = (char *) alloca (sizeof (SYMLINK_COOKIE) + oldpath_len + 1);
      /* Note that the terminating nul is written.  */
      cp = stpcpy (stpcpy (buf, SYMLINK_COOKIE), oldpath) + 1;
    }
  
  if (isdevice && win32_newpath.exists ())
    {
      status = NtOpenFile (&fh, FILE_WRITE_ATTRIBUTES,
			   win32_newpath.get_object_attr (attr, sa),
			   &io, 0, FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  goto done;
	}
      fbi.CreationTime.QuadPart = fbi.LastAccessTime.QuadPart
      = fbi.LastWriteTime.QuadPart = fbi.ChangeTime.QuadPart = 0LL;
      fbi.FileAttributes = FILE_ATTRIBUTE_NORMAL;
      status = NtSetInformationFile (fh, &io, &fbi, sizeof fbi,
				     FileBasicInformation);
      NtClose (fh);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status  (status);
	  goto done;
	}
    }
  if (allow_ntsec && win32_newpath.has_acls ())
    set_security_attribute (S_IFLNK | STD_RBITS | STD_WBITS,
			    &sa, sd);
  status = NtCreateFile (&fh, DELETE | FILE_GENERIC_WRITE,
			 win32_newpath.get_object_attr (attr, sa),
			 &io, NULL, FILE_ATTRIBUTE_NORMAL,
			 FILE_SHARE_VALID_FLAGS,
			 isdevice ? FILE_OVERWRITE_IF : FILE_CREATE,
			 FILE_SYNCHRONOUS_IO_NONALERT
			 | FILE_NON_DIRECTORY_FILE
			 | FILE_OPEN_FOR_BACKUP_INTENT,
			 NULL, 0);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      goto done;
    }
  status = NtWriteFile (fh, NULL, NULL, NULL, &io, buf, cp - buf, NULL, NULL);
  if (NT_SUCCESS (status) && io.Information == (ULONG) (cp - buf))
    {
      fbi.CreationTime.QuadPart = fbi.LastAccessTime.QuadPart
      = fbi.LastWriteTime.QuadPart = fbi.ChangeTime.QuadPart = 0LL;
      fbi.FileAttributes = use_winsym ? FILE_ATTRIBUTE_READONLY
				      : FILE_ATTRIBUTE_SYSTEM;
      status = NtSetInformationFile (fh, &io, &fbi, sizeof fbi,
				     FileBasicInformation);
      if (!NT_SUCCESS (status))
        debug_printf ("Setting attributes failed, status = %p", status);
      res = 0;
    }
  else
    {
      __seterrno_from_nt_status (status);
      FILE_DISPOSITION_INFORMATION fdi = { TRUE };
      status = NtSetInformationFile (fh, &io, &fdi, sizeof fdi,
				     FileDispositionInformation);
      if (!NT_SUCCESS (status))
        debug_printf ("Setting delete dispostion failed, status = %p", status);
    }
  NtClose (fh);

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
symlink_info::check_shortcut (HANDLE h)
{
  win_shortcut_hdr *file_header;
  char *buf, *cp;
  unsigned short len;
  int res = 0;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_STANDARD_INFORMATION fsi;

  status = NtQueryInformationFile (h, &io, &fsi, sizeof fsi,
				   FileStandardInformation);
  if (!NT_SUCCESS (status))
    {
      set_error (EIO);
      return 0;
    }
  if (fsi.EndOfFile.QuadPart <= sizeof (win_shortcut_hdr)
      || fsi.EndOfFile.QuadPart > 4 * 65536)
    return 0;
  buf = (char *) alloca (fsi.EndOfFile.LowPart + 1);
  if (!NT_SUCCESS (NtReadFile (h, NULL, NULL, NULL,
			       &io, buf, fsi.EndOfFile.LowPart, NULL, NULL)))
    {
      set_error (EIO);
      return 0;
    }
  file_header = (win_shortcut_hdr *) buf;
  if (io.Information != fsi.EndOfFile.LowPart
      || !cmp_shortcut_header (file_header))
    goto file_not_symlink;
  cp = buf + sizeof (win_shortcut_hdr);
  if (file_header->flags & WSH_FLAG_IDLIST) /* Skip ITEMIDLIST */
    cp += *(unsigned short *) cp + 2;
  if (!(len = *(unsigned short *) cp))
    goto file_not_symlink;
  cp += 2;
  /* Check if this is a device file - these start with the sequence :\\ */
  if (strncmp (cp, ":\\", 2) == 0)
    res = strlen (strcpy (contents, cp)); /* Don't mess with device files */
  else
    {
      /* Has appended full path?  If so, use it instead of description. */
      unsigned short relpath_len = *(unsigned short *) (cp + len);
      if (cp + len + 2 + relpath_len < buf + fsi.EndOfFile.LowPart)
	{
	  cp += len + 2 + relpath_len;
	  len = *(unsigned short *) cp;
	  cp += 2;
	}
      if (len > SYMLINK_MAX)
	goto file_not_symlink;
      cp[len] = '\0';
      res = posixify (cp);
    }
  if (res) /* It's a symlink.  */
    pflags = PATH_SYMLINK | PATH_LNK;
  return res;

file_not_symlink:
  /* Not a symlink, see if executable.  */
  if (!(pflags & PATH_ALL_EXEC) && has_exec_chars ((const char *) &file_header, io.Information))
    pflags |= PATH_EXEC;
  return 0;
}

int
symlink_info::check_sysfile (HANDLE h)
{
  char cookie_buf[sizeof (SYMLINK_COOKIE) - 1];
  char srcbuf[SYMLINK_MAX + 2];
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  int res = 0;

  status = NtReadFile (h, NULL, NULL, NULL, &io, cookie_buf,
		       sizeof (cookie_buf), NULL, NULL);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("ReadFile1 failed");
      if (status != STATUS_END_OF_FILE)
	set_error (EIO);
    }
  else if (io.Information == sizeof (cookie_buf)
	   && memcmp (cookie_buf, SYMLINK_COOKIE, sizeof (cookie_buf)) == 0)
    {
      /* It's a symlink.  */
      pflags = PATH_SYMLINK;

      status = NtReadFile (h, NULL, NULL, NULL, &io, srcbuf,
			   SYMLINK_MAX + 2, NULL, NULL);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("ReadFile2 failed");
	  if (status != STATUS_END_OF_FILE)
	    set_error (EIO);
	}
      else if (io.Information > SYMLINK_MAX + 1)
        {
	  debug_printf ("symlink string too long");
	  
	}
      else
	res = posixify (srcbuf);
    }
  else if (io.Information == sizeof (cookie_buf)
	   && memcmp (cookie_buf, SOCKET_COOKIE, sizeof (cookie_buf)) == 0)
    pflags |= PATH_SOCKET;
  else
    {
      /* Not a symlink, see if executable.  */
      if (pflags & PATH_ALL_EXEC)
	/* Nothing to do */;
      else if (has_exec_chars (cookie_buf, io.Information))
	pflags |= PATH_EXEC;
      else
	pflags |= PATH_NOTEXEC;
      }
  return res;
}

int
symlink_info::check_reparse_point (HANDLE h)
{
  PREPARSE_DATA_BUFFER rp = (PREPARSE_DATA_BUFFER)
			    alloca (MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  DWORD size;
  char srcbuf[SYMLINK_MAX + 7];

  if (!DeviceIoControl (h, FSCTL_GET_REPARSE_POINT, NULL, 0, (LPVOID) rp,
			MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &size, NULL))
    {
      debug_printf ("DeviceIoControl(FSCTL_GET_REPARSE_POINT) failed, %E");
      set_error (EIO);
      return 0;
    }
  if (rp->ReparseTag == IO_REPARSE_TAG_SYMLINK)
    {
      sys_wcstombs (srcbuf, SYMLINK_MAX + 1,
		    (WCHAR *)((char *)rp->SymbolicLinkReparseBuffer.PathBuffer
			  + rp->SymbolicLinkReparseBuffer.SubstituteNameOffset),
		    rp->SymbolicLinkReparseBuffer.SubstituteNameLength / 2);
      pflags = PATH_SYMLINK | PATH_REP;
      fileattr &= ~FILE_ATTRIBUTE_DIRECTORY;
    }
  else if (rp->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
    {
      if (rp->SymbolicLinkReparseBuffer.PrintNameLength == 0)
	{
	  /* Likely a volume mount point.  Not treated as symlink. */
	  return 0;
	}
      sys_wcstombs (srcbuf, SYMLINK_MAX + 1,
		    (WCHAR *)((char *)rp->MountPointReparseBuffer.PathBuffer
			    + rp->MountPointReparseBuffer.SubstituteNameOffset),
		    rp->MountPointReparseBuffer.SubstituteNameLength / 2);
      pflags = PATH_SYMLINK | PATH_REP;
      fileattr &= ~FILE_ATTRIBUTE_DIRECTORY;
    }
  return posixify (srcbuf);
}

int
symlink_info::posixify (char *srcbuf)
{
  /* The definition for a path in a native symlink is a bit weird.  The Flags
     value seem to contain 0 for absolute paths (stored as NT native path)
     and 1 for relative paths.  Relative paths are paths not starting with a
     drive letter.  These are not converted to NT native, but stored as
     given.  A path starting with a single backslash is relative to the
     current drive thus a "relative" value (Flags == 1).
     Funny enough it's possible to store paths with slashes instead of
     backslashes, but they are evaluated incorrectly by subsequent Windows
     calls like CreateFile (ERROR_INVALID_NAME).  So, what we do here is to
     take paths starting with slashes at face value, evaluating them as
     Cygwin specific POSIX paths.
     A path starting with two slashes(!) or backslashes is converted into an
     NT UNC path.  Unfortunately, in contrast to POSIX rules, paths starting
     with three or more (back)slashes are also converted into UNC paths,
     just incorrectly sticking to one redundant leading backslashe.  We go
     along with this behaviour to avoid scenarios in which native tools access
     other files than Cygwin.
     The above rules are used exactly the same way on Cygwin specific symlinks
     (sysfiles and shortcuts) to eliminate non-POSIX paths in the output. */

  /* Eliminate native NT prefixes. */
  if (srcbuf[0] == '\\' && !strncmp (srcbuf + 1, "??\\", 3))
    {
      srcbuf += 4;
      if (!strncmp (srcbuf, "UNC\\", 4))
	{
	  srcbuf += 2;
	  *srcbuf = '\\';
	}
    }
  if (isdrive (srcbuf))
    mount_table->conv_to_posix_path (srcbuf, contents, 0);
  else if (srcbuf[0] == '\\')
    {
      if (srcbuf[1] == '\\') /* UNC path */
	slashify (srcbuf, contents, 0);
      else /* Paths starting with \ are current drive relative. */
	{
	  char cvtbuf[SYMLINK_MAX + 1];

	  stpcpy (cvtbuf + cygheap->cwd.get_drive (cvtbuf), srcbuf);
	  mount_table->conv_to_posix_path (cvtbuf, contents, 0);
	}
    }
  else /* Everything else is taken as is. */
    slashify (srcbuf, contents, 0);
  return strlen (contents);
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
  pflags &= ~(PATH_SYMLINK | PATH_LNK | PATH_REP);
  case_clash = false;

  /* TODO: Temporarily do all char->UNICODE conversion here.  This should
     already be slightly faster than using Ascii functions. */
  UNICODE_STRING upath;
  OBJECT_ATTRIBUTES attr;
  size_t len = (strlen (path) + 8 + 8 + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&upath, (PCWSTR) alloca (len), len);
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);

  while (suffix.next ())
    {
      FILE_BASIC_INFORMATION fbi;
      NTSTATUS status;
      IO_STATUS_BLOCK io;

      error = 0;
      get_nt_native_path (suffix.path, upath);
      status = NtQueryAttributesFile (&attr, &fbi);
      if (NT_SUCCESS (status))
        fileattr = fbi.FileAttributes;
      else
	{
	  debug_printf ("%p = NtQueryAttributesFile (%S)", status, &upath);
	  fileattr = INVALID_FILE_ATTRIBUTES;

	  /* One of the inner path components is invalid, or the path contains
	     invalid characters.  Bail out with ENOENT.

	     Note that additional STATUS_OBJECT_PATH_INVALID and
	     STATUS_OBJECT_PATH_SYNTAX_BAD status codes exist.  The first one
	     is seemingly not generated by NtQueryAttributesFile, the latter
	     is only generated if the path is no absolute path within the
	     NT name space, which should not happen and would point to an
	     error in get_nt_native_path.  Both status codes are deliberately
	     not tested here unless proved necessary. */
	  if (status == STATUS_OBJECT_PATH_NOT_FOUND
	      || status == STATUS_OBJECT_NAME_INVALID)
	    {
	      set_error (ENOENT);
	      break;
	    }
	  if (status != STATUS_OBJECT_NAME_NOT_FOUND
	      && status != STATUS_NO_SUCH_FILE) /* File not found on 9x share */
	    {
	      /* The file exists, but the user can't access it for one reason
		 or the other.  To get the file attributes we try to access the
		 information by opening the parent directory and getting the
		 file attributes using a matching NtQueryDirectoryFile call. */
	      UNICODE_STRING dirname, basename;
	      OBJECT_ATTRIBUTES dattr;
	      HANDLE dir;
	      FILE_DIRECTORY_INFORMATION fdi;

	      RtlSplitUnicodePath (&upath, &dirname, &basename);
	      InitializeObjectAttributes (&dattr, &dirname,
					  OBJ_CASE_INSENSITIVE, NULL, NULL);
	      status = NtOpenFile (&dir, SYNCHRONIZE | FILE_LIST_DIRECTORY,
				   &dattr, &io, FILE_SHARE_VALID_FLAGS,
				   FILE_SYNCHRONOUS_IO_NONALERT
				   | FILE_OPEN_FOR_BACKUP_INTENT
				   | FILE_DIRECTORY_FILE);
	      if (!NT_SUCCESS (status))
	        {
		  debug_printf ("%p = NtOpenFile(%S)", status, &dirname);
		  fileattr = 0;
		}
	      else
	        {
		  status = NtQueryDirectoryFile (dir, NULL, NULL, 0, &io,
						 &fdi, sizeof fdi,
						 FileDirectoryInformation,
						 TRUE, &basename, TRUE);
		  NtClose (dir);
		  /* Per MSDN, ZwQueryDirectoryFile allows to specify a buffer
		     which only fits the static parts of the structure (without
		     filename that is) in the first call.  The buffer actually
		     contains valid data, even though ZwQueryDirectoryFile
		     returned STATUS_BUFFER_OVERFLOW.

		     Please note that this doesn't work for the info class
		     FileIdBothDirectoryInformation, unfortunately, so we don't
		     use this technique in fhandler_base::fstat_by_name, */
		  if (!NT_SUCCESS (status) && status != STATUS_BUFFER_OVERFLOW)
		    {
		      debug_printf ("%p = NtQueryDirectoryFile(%S)",
				    status, &dirname);
		      fileattr = 0;
		    }
		  else
		    fileattr = fdi.FileAttributes;
		}
	      ext_tacked_on = !!*ext_here;
	      goto file_not_symlink;
	    }
	  if (set_error (geterrno_from_win_error
	  			(RtlNtStatusToDosError (status), EACCES)))
	    continue;
	}

      ext_tacked_on = !!*ext_here;

      if (pcheck_case != PCHECK_RELAXED && !case_check (path)
	  || (opt & PC_SYM_IGNORE))
	goto file_not_symlink;

      int sym_check;

      sym_check = 0;

      if ((fileattr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
	  == FILE_ATTRIBUTE_DIRECTORY)
	goto file_not_symlink;

      /* Windows shortcuts are potentially treated as symlinks. */
      /* Valid Cygwin & U/WIN shortcuts are R/O. */
      if ((fileattr & FILE_ATTRIBUTE_READONLY) && suffix.lnk_match ())
	sym_check = 1;

      /* This is the old Cygwin method creating symlinks: */
      /* A symlink will have the `system' file attribute. */
      /* Only files can be symlinks (which can be symlinks to directories). */
      else if (fileattr & FILE_ATTRIBUTE_SYSTEM)
	sym_check = 2;

      /* Reparse points are potentially symlinks. */
      else if (fileattr & FILE_ATTRIBUTE_REPARSE_POINT)
	sym_check = 3;

      if (!sym_check)
	goto file_not_symlink;

      res = -1;

      /* Open the file.  */
      status = NtOpenFile (&h, FILE_GENERIC_READ, &attr, &io,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT
			   | (sym_check == 3 ? FILE_OPEN_REPARSE_POINT : 0));
      if (!NT_SUCCESS (status))
	goto file_not_symlink;

      switch (sym_check)
	{
	case 1:
	  res = check_shortcut (h);
	  NtClose (h);
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
	  res = check_sysfile (h);
	  NtClose (h);
	  if (!res)
	    goto file_not_symlink;
	  break;
	case 3:
	  res = check_reparse_point (h);
	  NtClose (h);
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
hash_path_name (__ino64_t hash, PUNICODE_STRING name)
{
  if (name->Length == 0)
    return hash;

  /* Build up hash. Name is already normalized */
  USHORT len = name->Length / sizeof (WCHAR);
  for (USHORT idx = 0; idx < len; ++idx)
    hash = RtlUpcaseUnicodeChar (name->Buffer[idx])
	   + (hash << 6) + (hash << 16) - hash;
  return hash;
}

__ino64_t __stdcall
hash_path_name (__ino64_t hash, PCWSTR name)
{
  UNICODE_STRING uname;
  RtlInitUnicodeString (&uname, name);
  return hash_path_name (hash, &uname);
}

__ino64_t __stdcall
hash_path_name (__ino64_t hash, const char *name)
{
  UNICODE_STRING uname;
  RtlCreateUnicodeStringFromAsciiz (&uname, name);
  __ino64_t ret = hash_path_name (hash, &uname);
  RtlFreeUnicodeString (&uname);
  return ret;
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
  const char *posix_cwd = NULL;
  int devn = path.get_devn ();
  if (!isvirtual_dev (devn))
    {
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
    res = cygheap->cwd.set (path.get_nt_native_path (), posix_cwd, doit);

  /* Note that we're accessing cwd.posix without a lock here.  I didn't think
     it was worth locking just for strace. */
  syscall_printf ("%d = chdir() cygheap->cwd.posix '%s' native '%S'", res,
		  cygheap->cwd.posix, path.get_nt_native_path ());
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
  path_conv p (path, PC_SYM_FOLLOW | PC_NO_ACCESS_CHECK | PC_NOFULL | PC_NOWARN);
  if (p.error)
    {
      win32_path[0] = '\0';
      set_errno (p.error);
      return -1;
    }


  strcpy (win32_path,
	  strcmp (p.get_win32 (), ".\\") == 0 ? "." : p.get_win32 ());
  return 0;
}

extern "C" int
cygwin_conv_to_full_win32_path (const char *path, char *win32_path)
{
  path_conv p (path, PC_SYM_FOLLOW | PC_NO_ACCESS_CHECK | PC_NOWARN);
  if (p.error)
    {
      win32_path[0] = '\0';
      set_errno (p.error);
      return -1;
    }

  strcpy (win32_path, p.get_win32 ());
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
env_win32_to_posix_path_list (const char *win32, char *posix)
{
  return_with_errno (conv_path_list (win32, posix, ENV_CVT));
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

static inline PRTL_USER_PROCESS_PARAMETERS
get_user_proc_parms ()
{
  return NtCurrentTeb ()->Peb->ProcessParameters;
}

/* Initialize cygcwd 'muto' for serializing access to cwd info. */
void
cwdstuff::init ()
{
  cwd_lock.init ("cwd_lock");
  /* Initially re-open the cwd to allow POSIX semantics. */
  set (NULL, NULL, true);
}

/* Chdir and fill out the elements of a cwdstuff struct. */
int
cwdstuff::set (PUNICODE_STRING nat_cwd, const char *posix_cwd, bool doit)
{
  int res = 0;
  UNICODE_STRING upath;
  size_t len = 0;

  cwd_lock.acquire ();

  if (nat_cwd)
    {
      upath = *nat_cwd;
      if (upath.Buffer[0] == L'/') /* Virtual path.  Never use in PEB. */
	doit = false;
      else
	{
	  len = upath.Length / sizeof (WCHAR) - 4;
	  if (RtlEqualUnicodePathPrefix (&upath, L"\\??\\UNC\\", TRUE))
	    len -= 2;
	}
    }

  if (doit)
    {
      /* We utilize the user parameter block.  The directory is
	 stored manually there.  Why the hassle?

	 - SetCurrentDirectory fails for directories with strict
	   permissions even for processes with the SE_BACKUP_NAME
	   privilege enabled.  The reason is apparently that
	   SetCurrentDirectory calls NtOpenFile without the
	   FILE_OPEN_FOR_BACKUP_INTENT flag set.

	 - Unlinking a cwd fails because SetCurrentDirectory seems to
	   open directories so that deleting the directory is disallowed.
	   The below code opens with *all* sharing flags set. */
      HANDLE h;
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      OBJECT_ATTRIBUTES attr;
      PHANDLE phdl;

      RtlAcquirePebLock ();
      phdl = &get_user_proc_parms ()->CurrentDirectoryHandle;
      if (!nat_cwd) /* On init, just reopen CWD with desired access flags. */
	RtlInitUnicodeString (&upath, L"");
      else
	{
	  /* TODO:
	     Check the length of the new CWD.  Windows can only handle
	     CWDs of up to MAX_PATH length, including a trailing backslash.
	     If the path is longer, it's not an error condition for Cygwin,
	     so we don't fail.  Windows on the other hand has a problem now.
	     For now, we just don't store the path in the PEB and proceed as
	     usual. */
	  if (len > MAX_PATH - (nat_cwd->Buffer[len - 1] == L'\\' ? 1 : 2))
	    goto skip_peb_storing;
	}
      InitializeObjectAttributes (&attr, &upath,
				  OBJ_CASE_INSENSITIVE | OBJ_INHERIT,
				  nat_cwd ? NULL : *phdl, NULL);
      status = NtOpenFile (&h, SYNCHRONIZE | FILE_TRAVERSE, &attr, &io,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_DIRECTORY_FILE
			   | FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	{
	  RtlReleasePebLock ();
	  __seterrno_from_nt_status (status);
	  res = -1;
	  goto out;
	}

      if (nat_cwd) /* No need to set path on init. */
        {
	  /* Convert to a Win32 path. */
	  upath.Buffer += upath.Length / sizeof (WCHAR) - len;
	  if (upath.Buffer[1] == L'\\') /* UNC path */
	    upath.Buffer[0] = L'\\';
	  upath.Length = len * sizeof (WCHAR);
	  /* Append backslash if necessary. */
	  if (upath.Buffer[len - 1] != L'\\')
	    {
	      upath.Buffer[len] = L'\\';
	      upath.Length += sizeof (WCHAR);
	    }
	  RtlCopyUnicodeString (&get_user_proc_parms ()->CurrentDirectoryName,
				&upath);
	}
      NtClose (*phdl);
      /* Workaround a problem in Vista which fails in subsequent calls to
	 CreateFile with ERROR_INVALID_HANDLE if the handle in
	 CurrentDirectoryHandle changes without calling SetCurrentDirectory,
	 and the filename given to CreateFile is a relative path.  It looks
	 like Vista stores a copy of the CWD handle in some other undocumented
	 place.  The NtClose/DuplicateHandle reuses the original handle for
	 the copy of the new handle and the next CreateFile works.
	 Note that this is not thread-safe (yet?) */
      if (DuplicateHandle (GetCurrentProcess (), h, GetCurrentProcess (), phdl,
			   0, TRUE, DUPLICATE_SAME_ACCESS))
	NtClose (h);
      else
	*phdl = h;

skip_peb_storing:

      RtlReleasePebLock ();
    }

  if (nat_cwd || !win32.Buffer)
    {
      /* If there is no win32 path */
      if (!nat_cwd)
	{
	  PUNICODE_STRING pdir;

	  RtlAcquirePebLock ();
	  pdir = &get_user_proc_parms ()->CurrentDirectoryName;
	  RtlInitEmptyUnicodeString (&win32,
				     (PWCHAR) crealloc_abort (win32.Buffer,
							      pdir->Length + 2),
				     pdir->Length + 2);
	  RtlCopyUnicodeString (&win32, pdir);
	  RtlReleasePebLock ();
	  /* Remove trailing slash. */
	  if (win32.Length > 3 * sizeof (WCHAR))
	    win32.Length -= sizeof (WCHAR);
	  posix_cwd = NULL;
	}
      else
	{
	  if (upath.Buffer[0] == L'/') /* Virtual path, don't mangle. */
	    ;
	  else if (!doit)
	    {
	      /* Convert to a Win32 path. */
	      upath.Buffer += upath.Length / sizeof (WCHAR) - len;
	      if (upath.Buffer[1] == L'\\') /* UNC path */
		upath.Buffer[0] = L'\\';
	      upath.Length = len * sizeof (WCHAR);
	    }
	  else if (upath.Length > 3 * sizeof (WCHAR))
	    upath.Length -= sizeof (WCHAR); /* Strip trailing backslash */
	  RtlInitEmptyUnicodeString (&win32,
				     (PWCHAR) crealloc_abort (win32.Buffer,
							      upath.Length + 2),
				     upath.Length + 2);
	  RtlCopyUnicodeString (&win32, &upath);
	}
      /* Make sure it's NUL-termniated. */
      win32.Buffer[win32.Length / sizeof (WCHAR)] = L'\0';
      if (win32.Buffer[1] == L':')
	drive_length = 2;
      else if (win32.Buffer[1] == L'\\')
	{
	  PWCHAR ptr = wcschr (win32.Buffer + 2, L'\\');
	  if (*ptr)
	    ptr = wcschr (ptr + 1, L'\\');
	  if (ptr)
	    drive_length = ptr - win32.Buffer;
	  else
	    drive_length = win32.Length / sizeof (WCHAR);
	}
      else
	drive_length = 0;

      if (!posix_cwd)
	{
	  posix_cwd = (const char *) alloca (PATH_MAX);
	  mount_table->conv_to_posix_path (win32.Buffer, (char *) posix_cwd, 0);
	}
      posix = (char *) crealloc_abort (posix, strlen (posix_cwd) + 1);
      stpcpy (posix, posix_cwd);
    }

out:
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

  cwd_lock.acquire ();

  char *tocopy;
  if (!need_posix)
    {
      tocopy = (char *) alloca (PATH_MAX);
      sys_wcstombs (tocopy, PATH_MAX, win32.Buffer, win32.Length);
    }
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
OBJECT_ATTRIBUTES etc::fn[MAX_ETC_FILES + 1];
LARGE_INTEGER etc::last_modified[MAX_ETC_FILES + 1];

int
etc::init (int n, PUNICODE_STRING etc_fn)
{
  if (n > 0)
    /* ok */;
  else if (++curr_ix <= MAX_ETC_FILES)
    n = curr_ix;
  else
    api_fatal ("internal error");

  InitializeObjectAttributes (&fn[n], etc_fn, OBJ_CASE_INSENSITIVE, NULL, NULL);
  change_possible[n] = false;
  test_file_change (n);
  paranoid_printf ("fn[%d] %S, curr_ix %d", n, fn[n].ObjectName, curr_ix);
  return n;
}

bool
etc::test_file_change (int n)
{
  NTSTATUS status;
  FILE_NETWORK_OPEN_INFORMATION fnoi;
  bool res;

  status = NtQueryFullAttributesFile (&fn[n], &fnoi);
  if (!NT_SUCCESS (status))
    {
      res = true;
      memset (last_modified + n, 0, sizeof (last_modified[n]));
      debug_printf ("NtQueryFullAttributesFile (%S) failed, %p",
		    fn[n].ObjectName, status);
    }
  else
    {
      res = CompareFileTime ((FILETIME *) &fnoi.LastWriteTime,
			     (FILETIME *) last_modified + n) > 0;
      last_modified[n].QuadPart = fnoi.LastWriteTime.QuadPart;
    }

  paranoid_printf ("fn[%d] %S res %d", n, fn[n].ObjectName, res);
  return res;
}

bool
etc::dir_changed (int n)
{
  if (!change_possible[n])
    {
      static HANDLE changed_h NO_COPY;
      NTSTATUS status;
      IO_STATUS_BLOCK io;

      if (!changed_h)
	{
	  OBJECT_ATTRIBUTES attr;

	  path_conv dir ("/etc");
	  status = NtOpenFile (&changed_h, SYNCHRONIZE | FILE_LIST_DIRECTORY,
			       dir.get_object_attr (attr, sec_none_nih), &io,
			       FILE_SHARE_VALID_FLAGS, FILE_DIRECTORY_FILE);
	  if (!NT_SUCCESS (status))
	    {
#ifdef DEBUGGING
	      system_printf ("NtOpenFile (%S) failed, %p",
			     dir.get_nt_native_path (), status);
#endif
	      changed_h = INVALID_HANDLE_VALUE;
	    }
	  else
	    {
	      status = NtNotifyChangeDirectoryFile (changed_h, NULL, NULL,
						NULL, &io, NULL, 0,
						FILE_NOTIFY_CHANGE_LAST_WRITE
						| FILE_NOTIFY_CHANGE_FILE_NAME,
						FALSE);
	      if (!NT_SUCCESS (status))
		{
#ifdef DEBUGGING
		  system_printf ("NtNotifyChangeDirectoryFile (1) failed, %p",
				 status);
#endif
		  NtClose (changed_h);
		  changed_h = INVALID_HANDLE_VALUE;
		}
	    }
	  memset (change_possible, true, sizeof (change_possible));
	}

      if (changed_h == INVALID_HANDLE_VALUE)
	change_possible[n] = true;
      else if (WaitForSingleObject (changed_h, 0) == WAIT_OBJECT_0)
	{
	  status = NtNotifyChangeDirectoryFile (changed_h, NULL, NULL,
						NULL, &io, NULL, 0,
						FILE_NOTIFY_CHANGE_LAST_WRITE
						| FILE_NOTIFY_CHANGE_FILE_NAME,
						FALSE);
	  if (!NT_SUCCESS (status))
	    {
#ifdef DEBUGGING
	      system_printf ("NtNotifyChangeDirectoryFile (2) failed, %p",
			     status);
#endif
	      NtClose (changed_h);
	      changed_h = INVALID_HANDLE_VALUE;
	    }
	  memset (change_possible, true, sizeof change_possible);
	}
    }

  paranoid_printf ("fn[%d] %S change_possible %d",
		   n, fn[n].ObjectName, change_possible[n]);
  return change_possible[n];
}

bool
etc::file_changed (int n)
{
  bool res = false;
  if (dir_changed (n) && test_file_change (n))
    res = true;
  change_possible[n] = false;	/* Change is no longer possible */
  paranoid_printf ("fn[%d] %S res %d", n, fn[n].ObjectName, res);
  return res;
}

/* No need to be reentrant or thread-safe according to SUSv3.
   / and \\ are treated equally.  Leading drive specifiers are
   kept intact as far as it makes sense.  Everything else is
   POSIX compatible. */
extern "C" char *
basename (char *path)
{
  static char buf[4];
  char *c, *d, *bs = path;

  if (!path || !*path)
    return strcpy (buf, ".");
  if (isalpha (path[0]) && path[1] == ':')
    bs += 2;
  else if (strspn (path, "/\\") > 1)
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
    {
      stpncpy (buf, path, bs - path);
      stpcpy (buf + (bs - path), ".");
      return buf;
    }
  return path;
}

/* No need to be reentrant or thread-safe according to SUSv3.
   / and \\ are treated equally.  Leading drive specifiers and
   leading double (back)slashes are kept intact as far as it
   makes sense.  Everything else is POSIX compatible. */
extern "C" char *
dirname (char *path)
{
  static char buf[4];
  char *c, *d, *bs = path;

  if (!path || !*path)
    return strcpy (buf, ".");
  if (isalpha (path[0]) && path[1] == ':')
    bs += 2;
  else if (strspn (path, "/\\") > 1)
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
    {
      stpncpy (buf, path, bs - path);
      stpcpy (buf + (bs - path), ".");
      return buf;
    }
  return path;
}
