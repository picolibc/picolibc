/* path.cc: path support.

     Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
     2006, 2007, 2008, 2009 Red Hat, Inc.

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
#include "miscfuncs.h"
#include <ctype.h>
#include <winioctl.h>
#include <wingdi.h>
#include <winuser.h>
#include <winnetwk.h>
#include <shlobj.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "environ.h"
#include "nfs.h"
#include <assert.h>
#include <ntdll.h>
#include <wchar.h>
#include <wctype.h>

bool dos_file_warning = true;

suffix_info stat_suffixes[] =
{
  suffix_info ("", 1),
  suffix_info (".exe", 1),
  suffix_info (NULL)
};

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
  bool isdevice;
  _major_t major;
  _minor_t minor;
  _mode_t mode;
  int check (char *path, const suffix_info *suffixes, unsigned opt,
	     fs_info &fs);
  int set (char *path);
  bool parse_device (const char *);
  int check_sysfile (HANDLE h);
  int check_shortcut (HANDLE h);
  int check_reparse_point (HANDLE h);
  int check_nfs_symlink (HANDLE h);
  int posixify (char *srcbuf);
  bool set_error (int);
};

muto NO_COPY cwdstuff::cwd_lock;

static const GUID GUID_shortcut
			= { 0x00021401L, 0, 0, {0xc0, 0, 0, 0, 0, 0, 0, 0x46}};

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
path_prefix_p (const char *path1, const char *path2, int len1,
	       bool caseinsensitive)
{
  /* Handle case where PATH1 has trailing '/' and when it doesn't.  */
  if (len1 > 0 && isdirsep (path1[len1 - 1]))
    len1--;

  if (len1 == 0)
    return isdirsep (path2[0]) && !isdirsep (path2[1]);

  if (isdirsep (path2[len1]) || path2[len1] == 0 || path1[len1 - 1] == ':')
    return caseinsensitive ? strncasematch (path1, path2, len1)
			   : !strncmp (path1, path2, len1);

  return 0;
}

/* Return non-zero if paths match in first len chars.
   Check is dependent of the case sensitivity setting. */
int
pathnmatch (const char *path1, const char *path2, int len, bool caseinsensitive)
{
  return caseinsensitive
	 ? strncasematch (path1, path2, len) : !strncmp (path1, path2, len);
}

/* Return non-zero if paths match. Check is dependent of the case
   sensitivity setting. */
int
pathmatch (const char *path1, const char *path2, bool caseinsensitive)
{
  return caseinsensitive
	 ? strcasematch (path1, path2) : !strcmp (path1, path2);
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
  const char *last_comp = strchr (dir, '\0');

  if (last_comp == dir)
    return false;	/* Empty string.  Probably shouldn't happen here? */

  /* Detect run of trailing slashes */
  while (last_comp > dir && *--last_comp == '/')
    continue;

  /* Detect just a run of slashes or a path that does not end with a slash. */
  if (*last_comp != '.')
    return false;

  /* We know we have a trailing dot here.  Check that it really is a standalone "."
     path component by checking that it is at the beginning of the string or is
     preceded by a "/" */
  if (last_comp == dir || *--last_comp == '/')
    return true;

  /* If we're not checking for '..' we're done.  Ditto if we're now pointing to
     a non-dot. */
  if (!test_dot_dot || *last_comp != '.')
    return false;		/* either not testing for .. or this was not '..' */

  /* Repeat previous test for standalone or path component. */
  return last_comp == dir || last_comp[-1] == '/';
}

/* Normalize a POSIX path.
   All duplicate /'s, except for 2 leading /'s, are deleted.
   The result is 0 for success, or an errno error value.  */

int
normalize_posix_path (const char *src, char *dst, char *&tail)
{
  const char *in_src = src;
  char *dst_start = dst;
  syscall_printf ("src %s", src);

  if ((isdrive (src) && isdirsep (src[2])) || *src == '\\')
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
	if ((tail - dst) >= NT_MAX_PATH)
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
	strcpy ((char *) known_suffix, sym.ext_here);
    }
}

static void __stdcall mkrelpath (char *dst, bool caseinsensitive)
  __attribute__ ((regparm (2)));

static void __stdcall
mkrelpath (char *path, bool caseinsensitive)
{
  tmp_pathbuf tp;
  char *cwd_win32 = tp.c_get ();
  if (!cygheap->cwd.get (cwd_win32, 0))
    return;

  unsigned cwdlen = strlen (cwd_win32);
  if (!path_prefix_p (cwd_win32, path, cwdlen, caseinsensitive))
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
path_conv::set_normalized_path (const char *path_copy)
{
  if (path_copy)
    {
      size_t n = strlen (path_copy) + 1;
      char *p = (char *) cmalloc_abort (HEAP_STR, n);
      normalized_path = (const char *) memcpy (p, path_copy, n);
    }
}

/* Transform characters invalid for Windows filenames to the Unicode private
   use area in the U+f0XX range.  The affected characters are all control
   chars 1 <= c <= 31, as well as the characters " * : < > ? |.  The backslash
   is affected as well, but we can't transform it as long as we accept Win32
   paths as input.
   The reverse functionality is in strfuncs.cc, function sys_cp_wcstombs. */
WCHAR tfx_chars[] NO_COPY = {
            0, 0xf000 |   1, 0xf000 |   2, 0xf000 |   3,
 0xf000 |   4, 0xf000 |   5, 0xf000 |   6, 0xf000 |   7,
 0xf000 |   8, 0xf000 |   9, 0xf000 |  10, 0xf000 |  11,
 0xf000 |  12, 0xf000 |  13, 0xf000 |  14, 0xf000 |  15,
 0xf000 |  16, 0xf000 |  17, 0xf000 |  18, 0xf000 |  19,
 0xf000 |  20, 0xf000 |  21, 0xf000 |  22, 0xf000 |  23,
 0xf000 |  24, 0xf000 |  25, 0xf000 |  26, 0xf000 |  27,
 0xf000 |  28, 0xf000 |  29, 0xf000 |  30, 0xf000 |  31,
          ' ',          '!', 0xf000 | '"',          '#',
          '$',          '%',          '&',           39,
          '(',          ')', 0xf000 | '*',          '+',
          ',',          '-',          '.',          '\\',
          '0',          '1',          '2',          '3',
          '4',          '5',          '6',          '7',
          '8',          '9', 0xf000 | ':',          ';',
 0xf000 | '<',          '=', 0xf000 | '>', 0xf000 | '?',
          '@',          'A',          'B',          'C',
          'D',          'E',          'F',          'G',
          'H',          'I',          'J',          'K',
          'L',          'M',          'N',          'O',
          'P',          'Q',          'R',          'S',
          'T',          'U',          'V',          'W',
          'X',          'Y',          'Z',          '[',
          '\\',          ']',          '^',          '_',
          '`',          'a',          'b',          'c',
          'd',          'e',          'f',          'g',
          'h',          'i',          'j',          'k',
          'l',          'm',          'n',          'o',
          'p',          'q',          'r',          's',
          't',          'u',          'v',          'w',
          'x',          'y',          'z',          '{',
 0xf000 | '|',          '}',          '~',          127
};

void
transform_chars (PWCHAR path, PWCHAR path_end)
{
  for (; path <= path_end; ++path)
    if (*path < 128)
      *path = tfx_chars[*path];
}

static inline
void
transform_chars (PUNICODE_STRING upath, USHORT start_idx)
{
  transform_chars (upath->Buffer + start_idx,
		   upath->Buffer + upath->Length / sizeof (WCHAR) - 1);
}

static inline void
str2uni_cat (UNICODE_STRING &tgt, const char *srcstr)
{
  int len = sys_mbstowcs (tgt.Buffer + tgt.Length / sizeof (WCHAR),
			  (tgt.MaximumLength - tgt.Length) / sizeof (WCHAR),
			  srcstr);
  if (len)
    tgt.Length += (len - 1) * sizeof (WCHAR);
}

PUNICODE_STRING
get_nt_native_path (const char *path, UNICODE_STRING& upath)
{
  upath.Length = 0;
  if (path[0] == '/')		/* special path w/o NT path representation. */
    str2uni_cat (upath, path);
  else if (path[0] != '\\')	/* X:\...  or relative path. */
    {
      if (path[1] == ':')	/* X:\... */
	{
	  RtlAppendUnicodeStringToString (&upath, &ro_u_natp);
	  str2uni_cat (upath, path);
	  /* The drive letter must be upper case. */
	  upath.Buffer[4] = towupper (upath.Buffer[4]);
	}
      else
	str2uni_cat (upath, path);
      transform_chars (&upath, 7);
    }
  else if (path[1] != '\\')	/* \Device\... */
    str2uni_cat (upath, path);
  else if ((path[2] != '.' && path[2] != '?')
	   || path[3] != '\\')	/* \\server\share\... */
    {
      RtlAppendUnicodeStringToString (&upath, &ro_u_uncp);
      str2uni_cat (upath, path + 2);
      transform_chars (&upath, 8);
    }
  else				/* \\.\device or \\?\foo */
    {
      RtlAppendUnicodeStringToString (&upath, &ro_u_natp);
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
			      objcaseinsensitive ()
			      | (sa.bInheritHandle ? OBJ_INHERIT : 0),
			      NULL, sa.lpSecurityDescriptor);
  return &attr;
}

PWCHAR
path_conv::get_wide_win32_path (PWCHAR wc)
{
  get_nt_native_path ();
  if (!wide_path)
    return NULL;
  wcpcpy (wc, wide_path);
  if (wc[1] == L'?')
    wc[1] = L'\\';
  return wc;
}

void
warn_msdos (const char *src)
{
  if (user_shared->warned_msdos || !dos_file_warning || !cygwin_finished_initializing)
    return;
  tmp_pathbuf tp;
  char *posix_path = tp.c_get ();
  small_printf ("cygwin warning:\n");
  if (cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_RELATIVE, src,
			posix_path, NT_MAX_PATH))
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

static DWORD
getfileattr (const char *path, bool caseinsensitive) /* path has to be always absolute. */
{
  tmp_pathbuf tp;
  UNICODE_STRING upath;
  OBJECT_ATTRIBUTES attr;
  FILE_BASIC_INFORMATION fbi;
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  tp.u_get (&upath);
  InitializeObjectAttributes (&attr, &upath,
			      caseinsensitive ? OBJ_CASE_INSENSITIVE : 0,
			      NULL, NULL);
  get_nt_native_path (path, upath);

  status = NtQueryAttributesFile (&attr, &fbi);
  if (NT_SUCCESS (status))
    return fbi.FileAttributes;

  if (status != STATUS_OBJECT_NAME_NOT_FOUND
      && status != STATUS_NO_SUCH_FILE) /* File not found on 9x share */
    {
      /* File exists but access denied.  Try to get attribute through
	 directory query. */
      UNICODE_STRING dirname, basename;
      HANDLE dir;
      FILE_DIRECTORY_INFORMATION fdi;

      RtlSplitUnicodePath (&upath, &dirname, &basename);
      InitializeObjectAttributes (&attr, &dirname,
				  caseinsensitive ? OBJ_CASE_INSENSITIVE : 0,
				  NULL, NULL);
      status = NtOpenFile (&dir, SYNCHRONIZE | FILE_LIST_DIRECTORY,
			   &attr, &io, FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT
			   | FILE_DIRECTORY_FILE);
      if (NT_SUCCESS (status))
	{
	  status = NtQueryDirectoryFile (dir, NULL, NULL, 0, &io,
					 &fdi, sizeof fdi,
					 FileDirectoryInformation,
					 TRUE, &basename, TRUE);
	  NtClose (dir);
	  if (NT_SUCCESS (status) || status == STATUS_BUFFER_OVERFLOW)
	    return fdi.FileAttributes;
	}
    }
  SetLastError (RtlNtStatusToDosError (status));
  return INVALID_FILE_ATTRIBUTES;
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
path_conv::check (const UNICODE_STRING *src, unsigned opt,
		  const suffix_info *suffixes)
{
  tmp_pathbuf tp;
  char *path = tp.c_get ();

  user_shared->warned_msdos = true;
  sys_wcstombs (path, NT_MAX_PATH, src->Buffer, src->Length / sizeof (WCHAR));
  path_conv::check (path, opt, suffixes);
}

void
path_conv::check (const char *src, unsigned opt,
		  const suffix_info *suffixes)
{
  /* The tmp_buf array is used when expanding symlinks.  It is NT_MAX_PATH * 2
     in length so that we can hold the expanded symlink plus a trailer.  */
  tmp_pathbuf tp;
  char *path_copy = tp.c_get ();
  char *pathbuf = tp.c_get ();
  char *tmp_buf = tp.t_get ();
  char *THIS_path = tp.c_get ();
  symlink_info sym;
  bool need_directory = 0;
  bool saw_symlinks = 0;
  bool add_ext = false;
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
  caseinsensitive = OBJ_CASE_INSENSITIVE;
  if (wide_path)
    cfree (wide_path);
  wide_path = NULL;
  if (path)
    {
      cfree (modifiable_path ());
      path = NULL;
    }
  memset (&dev, 0, sizeof (dev));
  fs.clear ();
  if (normalized_path)
    {
      cfree ((void *) normalized_path);
      normalized_path = NULL;
    }
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
	      full_path = THIS_path;
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
		  fileattr = getfileattr (THIS_path,
					  sym.pflags & MOUNT_NOPOSIX);
		  dev.devn = FH_FS;
		}
	      goto out;
	    }
	  else if (dev == FH_DEV)
	    {
	      dev.devn = FH_FS;
#if 0
	      fileattr = getfileattr (THIS_path, sym.pflags & MOUNT_NOPOSIX);
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

	  symlen = sym.check (full_path, suff, opt, fs);

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

	  if (!component)
	    {
	      fileattr = sym.fileattr;
	      path_flags = sym.pflags;
	      /* If the OS is caseinsensitive or the FS is caseinsensitive or
	         the incoming path was given in DOS notation, don't handle
		 path casesensitive. */
	      if (cygwin_shared->obcaseinsensitive || fs.caseinsensitive ()
		  || is_msdos)
		path_flags |= PATH_NOPOSIX;
	      caseinsensitive = (path_flags & PATH_NOPOSIX)
				? OBJ_CASE_INSENSITIVE : 0;
	    }

	  /* If symlink.check found an existing non-symlink file, then
	     it sets the appropriate flag.  It also sets any suffix found
	     into `ext_here'. */
	  if (!sym.issymlink && sym.fileattr != INVALID_FILE_ATTRIBUTES)
	    {
	      error = sym.error;
	      if (component == 0)
		add_ext = true;
	      else if (!(sym.fileattr & FILE_ATTRIBUTE_DIRECTORY))
		{
		  error = ENOTDIR;
		  goto out;
		}
	      goto out;	// file found
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
		      strcpy (THIS_path, sym.contents);
		      goto out;
		    }
		  add_ext = true;
		  goto out;
		}
	      else
		break;
	    }
	  else if (sym.error && sym.error != ENOENT)
	    {
	      error = sym.error;
	      goto out;
	    }
	  /* No existing file found. */

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
      if (headptr + symlen >= tmp_buf + (2 * NT_MAX_PATH))
	{
	too_long:
	  error = ENAMETOOLONG;
	  set_path ("::ENAMETOOLONG::");
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
	  if (headptr + taillen > tmp_buf + (2 * NT_MAX_PATH))
	    goto too_long;
	  memcpy (headptr, tail, taillen);
	}

      /* Evaluate everything all over again. */
      src = tmp_buf;
    }

  if (!(opt & PC_SYM_CONTENTS))
    add_ext = true;

out:
  set_path (THIS_path);
  if (add_ext)
    add_ext_from_sym (sym);
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
      error = ENOENT;
      return;
    }
  else if (!need_directory || error)
    /* nothing to do */;
  else if (fileattr == INVALID_FILE_ATTRIBUTES)
    strcat (modifiable_path (), "\\"); /* Reattach trailing dirsep in native path. */
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
	  if (!tail || tail == path)
	    /* nothing */;
	  else if (tail[-1] != '\\')
	    *tail = '\0';
	  else
	    {
	      error = ENOENT;
	      return;
	    }
	}

      /* FS has been checked already for existing files. */
      if (exists () || fs.update (get_nt_native_path (), NULL))
	{
	  /* Incoming DOS paths are treated like DOS paths in native
	     Windows applications.  No ACLs, just default settings. */
	  if (is_msdos)
	    fs.has_acls (false);
	  debug_printf ("this->path(%s), has_acls(%d)", path, fs.has_acls ());
	  /* CV: We could use this->has_acls() but I want to make sure that
	     we don't forget that the PATH_NOACL flag must be taken into
	     account here. */
	  if (!(path_flags & PATH_NOACL) && fs.has_acls ())
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

  if (opt & PC_NOFULL)
    {
      if (is_relpath)
	{
	  mkrelpath (this->modifiable_path (), !!caseinsensitive);
	  /* Invalidate wide_path so that wide relpath can be created
	     in later calls to get_nt_native_path or get_wide_win32_path. */
	  if (wide_path)
	    cfree (wide_path);
	  wide_path = NULL;
	}
      if (need_directory)
	{
	  size_t n = strlen (this->path);
	  /* Do not add trailing \ to UNC device names like \\.\a: */
	  if (this->path[n - 1] != '\\' &&
	      (strncmp (this->path, "\\\\.\\", 4) != 0))
	    {
	      this->modifiable_path ()[n] = '\\';
	      this->modifiable_path ()[n + 1] = '\0';
	    }
	}
    }

  if (saw_symlinks)
    set_has_symlinks ();

  if ((opt & PC_POSIX))
    {
      if (tail < path_end && tail > path_copy + 1)
	*tail = '/';
      set_normalized_path (path_copy);
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
  if (normalized_path)
    {
      cfree ((void *) normalized_path);
      normalized_path = NULL;
    }
  if (path)
    {
      cfree (modifiable_path ());
      path = NULL;
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
  tmp_pathbuf tp;
  PWCHAR bintest = tp.w_get ();
  DWORD bin;

  return GetBinaryTypeW (get_wide_win32_path (bintest), &bin)
	 && (bin == SCS_32BIT_BINARY || bin == SCS_64BIT_BINARY);
}

/* Normalize a Win32 path.
   /'s are converted to \'s in the process.
   All duplicate \'s, except for 2 leading \'s, are deleted.

   The result is 0 for success, or an errno error value.
   FIXME: A lot of this should be mergeable with the POSIX critter.  */
int
normalize_win32_path (const char *src, char *dst, char *&tail)
{
  const char *src_start = src;
  bool beg_src_slash = isdirsep (src[0]);

  tail = dst;
  /* Skip long path name prefixes in Win32 or NT syntax. */
  if (beg_src_slash && (src[1] == '?' || isdirsep (src[1]))
      && src[2] == '?' && isdirsep (src[3]))
    {
      src += 4;
      if (src[1] != ':') /* native UNC path */
	src += 2; /* Fortunately the first char is not copied... */
      else
	beg_src_slash = false;
    }
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
  if (tail == dst)
    {
      if (isdrive (src))
	/* Always convert drive letter to uppercase for case sensitivity. */
	*tail++ = cyg_toupper (*src++);
      else if (*src != '/')
	{
	  if (beg_src_slash)
	    tail += cygheap->cwd.get_drive (dst);
	  else if (!cygheap->cwd.get (dst, 0))
	    return get_errno ();
	  else
	    {
	      tail = strchr (tail, '\0');
	      if (tail[-1] != '\\')
		*tail++ = '\\';
	    }
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
      if ((tail - dst) >= NT_MAX_PATH)
	return ENAMETOOLONG;
    }
   if (tail > dst + 1 && tail[-1] == '.' && tail[-2] == '\\')
     tail--;
  *tail = '\0';
  debug_printf ("%s = normalize_win32_path (%s)", dst, src_start);
  return 0;
}

/* Various utilities.  */

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
conv_path_list (const char *src, char *dst, size_t size, int to_posix)
{
  tmp_pathbuf tp;
  char src_delim, dst_delim;
  cygwin_conv_path_t conv_fn;
  size_t len;

  if (to_posix)
    {
      src_delim = ';';
      dst_delim = ':';
      conv_fn = CCP_WIN_A_TO_POSIX | CCP_RELATIVE;
    }
  else
    {
      src_delim = ':';
      dst_delim = ';';
      conv_fn = CCP_POSIX_TO_WIN_A | CCP_RELATIVE;
    }

  char *srcbuf;
  len = strlen (src) + 1;
  if (len <= NT_MAX_PATH * sizeof (WCHAR))
    srcbuf = (char *) tp.w_get ();
  else
    srcbuf = (char *) alloca (len);

  int err = 0;
  char *d = dst - 1;
  bool saw_empty = false;
  do
    {
      char *s = strccpy (srcbuf, &src, src_delim);
      size_t len = s - srcbuf;
      if (len >= NT_MAX_PATH)
	{
	  err = ENAMETOOLONG;
	  break;
	}
      if (len)
	{
	  ++d;
	  err = cygwin_conv_path (conv_fn, srcbuf, d, size - (d - dst));
	}
      else if (!to_posix)
	{
	  ++d;
	  err = cygwin_conv_path (conv_fn, ".", d, size - (d - dst));
	}
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

/********************** Symbolic Link Support **************************/

/* Create a symlink from FROMPATH to TOPATH. */

/* If TRUE create symlinks as Windows shortcuts, if false create symlinks
   as normal files with magic number and system bit set. */
bool allow_winsymlinks = false;

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
  tmp_pathbuf tp;
  unsigned check_opt;
  bool mk_winsym = use_winsym;
  bool has_trailing_dirsep = false;

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

  /* Trailing dirsep is a no-no. */
  len = strlen (newpath);
  has_trailing_dirsep = isdirsep (newpath[len - 1]);
  if (has_trailing_dirsep)
    {
      newpath = strdup (newpath);
      ((char *) newpath)[len - 1] = '\0';
    }

  check_opt = PC_SYM_NOFOLLOW | PC_POSIX | (isdevice ? PC_NOWARN : 0);
  /* We need the normalized full path below. */
  win32_newpath.check (newpath, check_opt, stat_suffixes);
  /* MVFS doesn't handle the SYSTEM DOS attribute, but it handles the R/O
     attribute.  Therefore we create symlinks on MVFS always as shortcuts. */
  mk_winsym |= win32_newpath.fs_is_mvfs ();

  if (mk_winsym && !win32_newpath.exists ()
      && (isdevice || !win32_newpath.fs_is_nfs ()))
    {
      char *newplnk = tp.c_get ();
      stpcpy (stpcpy (newplnk, newpath), ".lnk");
      win32_newpath.check (newplnk, check_opt);
    }

  if (win32_newpath.error)
    {
      set_errno (win32_newpath.error);
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
  if (has_trailing_dirsep && !win32_newpath.exists ())
    {
      set_errno (ENOENT);
      goto done;
    }

  if (!isdevice && win32_newpath.fs_is_nfs ())
    {
      /* On NFS, create symlinks by calling NtCreateFile with an EA of type
	 NfsSymlinkTargetName containing ... the symlink target name. */
      PFILE_FULL_EA_INFORMATION pffei = (PFILE_FULL_EA_INFORMATION) tp.w_get ();
      pffei->NextEntryOffset = 0;
      pffei->Flags = 0;
      pffei->EaNameLength = sizeof (NFS_SYML_TARGET) - 1;
      char *EaValue = stpcpy (pffei->EaName, NFS_SYML_TARGET) + 1;
      pffei->EaValueLength = sizeof (WCHAR) *
	(sys_mbstowcs ((PWCHAR) EaValue, NT_MAX_PATH, oldpath) - 1);
      status = NtCreateFile (&fh, FILE_WRITE_DATA | FILE_WRITE_EA | SYNCHRONIZE,
			     win32_newpath.get_object_attr (attr, sa),
			     &io, NULL, FILE_ATTRIBUTE_SYSTEM,
			     FILE_SHARE_VALID_FLAGS, FILE_CREATE,
			     FILE_SYNCHRONOUS_IO_NONALERT
			     | FILE_OPEN_FOR_BACKUP_INTENT,
			     pffei, NT_MAX_PATH * sizeof (WCHAR));
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  goto done;
	}
      NtClose (fh);
      res = 0;
      goto done;
    }

  if (mk_winsym)
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
	if (isabspath (oldpath))
	  win32_oldpath.check (oldpath,
			       PC_SYM_NOFOLLOW,
			       stat_suffixes);
	else
	    {
	      len = strrchr (win32_newpath.normalized_path, '/')
		    - win32_newpath.normalized_path + 1;
	      char *absoldpath = tp.t_get ();
	      stpcpy (stpncpy (absoldpath, win32_newpath.normalized_path, len),
		      oldpath);
	      win32_oldpath.check (absoldpath, PC_SYM_NOFOLLOW, stat_suffixes);
	    }
	  if (SUCCEEDED (SHGetDesktopFolder (&psl)))
	    {
	      WCHAR wc_path[win32_oldpath.get_wide_win32_path_len () + 1];
	      win32_oldpath.get_wide_win32_path (wc_path);
	      /* Amazing but true:  Even though the ParseDisplayName method
		 takes a wide char path name, it does not understand the
		 Win32 prefix for long pathnames!  So we have to tack off
		 the prefix and convert the path to the "normal" syntax
		 for ParseDisplayName.  */
	      WCHAR *wc = wc_path + 4;
	      if (wc[1] != L':') /* native UNC path */
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
	  stpcpy (relpath = tp.c_get (), oldpath);
	}
      else
	{
	  relpath_len = strlen (win32_oldpath.get_win32 ());
	  stpcpy (relpath = tp.c_get (), win32_oldpath.get_win32 ());
	}
      full_len += sizeof (unsigned short) + relpath_len;
      full_len += sizeof (unsigned short) + oldpath_len;
      /* 1 byte more for trailing 0 written by stpcpy. */
      if (full_len < NT_MAX_PATH * sizeof (WCHAR))
	buf = (char *) tp.w_get ();
      else
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
      unsigned short *plen = (unsigned short *) cp;
      cp += 2;
      *(PWCHAR) cp = 0xfeff;		/* BOM */
      cp += 2;
      *plen = sys_mbstowcs ((PWCHAR) cp, NT_MAX_PATH, oldpath) * sizeof (WCHAR);
      cp += *plen;
    }
  else
    {
      /* Default technique creating a symlink. */
      buf = (char *) tp.w_get ();
      cp = stpcpy (buf, SYMLINK_COOKIE);
      *(PWCHAR) cp = 0xfeff;		/* BOM */
      cp += 2;
      /* Note that the terminating nul is written.  */
      cp += sys_mbstowcs ((PWCHAR) cp, NT_MAX_PATH, oldpath) * sizeof (WCHAR);
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
      status = NtSetAttributesFile (fh, FILE_ATTRIBUTE_NORMAL);
      NtClose (fh);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  goto done;
	}
    }
  /* See comments in fhander_base::open () for an explanation why we defer
     setting security attributes on remote files. */
  if (win32_newpath.has_acls () && !win32_newpath.isremote ())
    set_security_attribute (win32_newpath, S_IFLNK | STD_RBITS | STD_WBITS,
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
  if (win32_newpath.has_acls () && win32_newpath.isremote ())
    set_file_attribute (fh, win32_newpath, ILLEGAL_UID, ILLEGAL_GID,
			S_IFLNK | STD_RBITS | STD_WBITS);
  status = NtWriteFile (fh, NULL, NULL, NULL, &io, buf, cp - buf, NULL, NULL);
  if (NT_SUCCESS (status) && io.Information == (ULONG) (cp - buf))
    {
      status = NtSetAttributesFile (fh, mk_winsym ? FILE_ATTRIBUTE_READONLY
						  : FILE_ATTRIBUTE_SYSTEM);
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
		  newpath, mk_winsym, isdevice);
  if (has_trailing_dirsep)
    free ((void *) newpath);
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
symlink_info::check_shortcut (HANDLE in_h)
{
  tmp_pathbuf tp;
  win_shortcut_hdr *file_header;
  char *buf, *cp;
  unsigned short len;
  int res = 0;
  UNICODE_STRING same = { 0, 0, (PWCHAR) L"" };
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  HANDLE h;
  IO_STATUS_BLOCK io;
  FILE_STANDARD_INFORMATION fsi;

  InitializeObjectAttributes (&attr, &same, 0, in_h, NULL);
  status = NtOpenFile (&h, FILE_READ_DATA | SYNCHRONIZE,
		       &attr, &io, FILE_SHARE_VALID_FLAGS,
		       FILE_OPEN_FOR_BACKUP_INTENT
		       | FILE_SYNCHRONOUS_IO_NONALERT);
  if (!NT_SUCCESS (status))
    return 0;
  status = NtQueryInformationFile (h, &io, &fsi, sizeof fsi,
				   FileStandardInformation);
  if (!NT_SUCCESS (status))
    {
      set_error (EIO);
      goto out;
    }
  if (fsi.EndOfFile.QuadPart <= sizeof (win_shortcut_hdr)
      || fsi.EndOfFile.QuadPart > 4 * 65536)
    goto out;
  if (fsi.EndOfFile.LowPart < NT_MAX_PATH * sizeof (WCHAR))
    buf = (char *) tp.w_get ();
  else
    buf = (char *) alloca (fsi.EndOfFile.LowPart + 1);
  if (!NT_SUCCESS (NtReadFile (h, NULL, NULL, NULL,
			       &io, buf, fsi.EndOfFile.LowPart, NULL, NULL)))
    {
      set_error (EIO);
      goto out;
    }
  file_header = (win_shortcut_hdr *) buf;
  if (io.Information != fsi.EndOfFile.LowPart
      || !cmp_shortcut_header (file_header))
    goto out;
  cp = buf + sizeof (win_shortcut_hdr);
  if (file_header->flags & WSH_FLAG_IDLIST) /* Skip ITEMIDLIST */
    cp += *(unsigned short *) cp + 2;
  if (!(len = *(unsigned short *) cp))
    goto out;
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
      if (*(PWCHAR) cp == 0xfeff)	/* BOM */
	{
	  char *tmpbuf = tp.c_get ();
	  if (sys_wcstombs (tmpbuf, NT_MAX_PATH, (PWCHAR) (cp + 2))
	      > SYMLINK_MAX + 1)
	    goto out;
	  res = posixify (tmpbuf);
	}
      else if (len > SYMLINK_MAX)
	goto out;
      else
	{
	  cp[len] = '\0';
	  res = posixify (cp);
	}
    }
  if (res) /* It's a symlink.  */
    pflags = PATH_SYMLINK | PATH_LNK;

out:
  NtClose (h);
  return res;
}

int
symlink_info::check_sysfile (HANDLE in_h)
{
  tmp_pathbuf tp;
  char cookie_buf[sizeof (SYMLINK_COOKIE) - 1];
  char *srcbuf = tp.c_get ();
  int res = 0;
  UNICODE_STRING same = { 0, 0, (PWCHAR) L"" };
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  HANDLE h;
  IO_STATUS_BLOCK io;
  bool interix_symlink = false;

  InitializeObjectAttributes (&attr, &same, 0, in_h, NULL);
  status = NtOpenFile (&h, FILE_READ_DATA | SYNCHRONIZE,
		       &attr, &io, FILE_SHARE_VALID_FLAGS,
		       FILE_OPEN_FOR_BACKUP_INTENT
		       | FILE_SYNCHRONOUS_IO_NONALERT);
  if (!NT_SUCCESS (status))
    ;
  else if (!NT_SUCCESS (status = NtReadFile (h, NULL, NULL, NULL, &io,
					     cookie_buf, sizeof (cookie_buf),
					     NULL, NULL)))
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
    }
  else if (io.Information == sizeof (cookie_buf)
	   && memcmp (cookie_buf, SOCKET_COOKIE, sizeof (cookie_buf)) == 0)
    pflags |= PATH_SOCKET;
  else if (io.Information >= sizeof (INTERIX_SYMLINK_COOKIE)
	   && memcmp (cookie_buf, INTERIX_SYMLINK_COOKIE,
		      sizeof (INTERIX_SYMLINK_COOKIE) - 1) == 0)
    {
      /* It's an Interix symlink.  */
      pflags = PATH_SYMLINK;
      interix_symlink = true;
      /* Interix symlink cookies are shorter than Cygwin symlink cookies, so
         in case of an Interix symlink cooky we have read too far into the
	 file.  Set file pointer back to the position right after the cookie. */
      FILE_POSITION_INFORMATION fpi;
      fpi.CurrentByteOffset.QuadPart = sizeof (INTERIX_SYMLINK_COOKIE) - 1;
      NtSetInformationFile (h, &io, &fpi, sizeof fpi, FilePositionInformation);
    }
  if (pflags == PATH_SYMLINK)
    {
      status = NtReadFile (h, NULL, NULL, NULL, &io, srcbuf,
			   NT_MAX_PATH, NULL, NULL);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("ReadFile2 failed");
	  if (status != STATUS_END_OF_FILE)
	    set_error (EIO);
	}
      else if (*(PWCHAR) srcbuf == 0xfeff 	/* BOM */
	       || interix_symlink)
	{
	  /* Add trailing 0 to Interix symlink target.  Skip BOM in Cygwin
	     symlinks. */
	  if (interix_symlink)
	    ((PWCHAR) srcbuf)[io.Information / sizeof (WCHAR)] = L'\0';
	  else
	    srcbuf += 2;
	  char *tmpbuf = tp.c_get ();
	  if (sys_wcstombs (tmpbuf, NT_MAX_PATH, (PWCHAR) srcbuf)
	      > SYMLINK_MAX + 1)
	    debug_printf ("symlink string too long");
	  else
	    res = posixify (tmpbuf);
	}
      else if (io.Information > SYMLINK_MAX + 1)
	debug_printf ("symlink string too long");
      else
	res = posixify (srcbuf);
    }
  NtClose (h);
  return res;
}

int
symlink_info::check_reparse_point (HANDLE h)
{
  tmp_pathbuf tp;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  PREPARSE_DATA_BUFFER rp = (PREPARSE_DATA_BUFFER) tp.c_get ();
  char srcbuf[SYMLINK_MAX + 7];

  status = NtFsControlFile (h, NULL, NULL, NULL, &io, FSCTL_GET_REPARSE_POINT,
			    NULL, 0, (LPVOID) rp,
			    MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtFsControlFile(FSCTL_GET_REPARSE_POINT) failed, %p",
		    status);
      set_error (EIO);
      return 0;
    }
  if (rp->ReparseTag == IO_REPARSE_TAG_SYMLINK)
    {
      sys_wcstombs (srcbuf, SYMLINK_MAX + 1,
		    (WCHAR *)((char *)rp->SymbolicLinkReparseBuffer.PathBuffer
			  + rp->SymbolicLinkReparseBuffer.SubstituteNameOffset),
		    rp->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof (WCHAR));
      pflags = PATH_SYMLINK | PATH_REP;
      fileattr &= ~FILE_ATTRIBUTE_DIRECTORY;
    }
  else if (rp->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
    {
      UNICODE_STRING subst;

      RtlInitCountedUnicodeString (&subst, 
		  (WCHAR *)((char *)rp->MountPointReparseBuffer.PathBuffer
			  + rp->MountPointReparseBuffer.SubstituteNameOffset),
		  rp->MountPointReparseBuffer.SubstituteNameLength);
      if (rp->MountPointReparseBuffer.PrintNameLength == 0
	  || RtlEqualUnicodePathPrefix (&subst, &ro_u_volume, TRUE))
	{
	  /* Volume mount point.  Not treated as symlink. The return
	     value of -1 is a hint for the caller to treat this as a
	     volume mount point. */
	  return -1;
	}
      sys_wcstombs (srcbuf, SYMLINK_MAX + 1,
		    (WCHAR *)((char *)rp->MountPointReparseBuffer.PathBuffer
			    + rp->MountPointReparseBuffer.SubstituteNameOffset),
		    rp->MountPointReparseBuffer.SubstituteNameLength / sizeof (WCHAR));
      pflags = PATH_SYMLINK | PATH_REP;
      fileattr &= ~FILE_ATTRIBUTE_DIRECTORY;
    }
  return posixify (srcbuf);
}

int
symlink_info::check_nfs_symlink (HANDLE h)
{
  tmp_pathbuf tp;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  struct {
    FILE_GET_EA_INFORMATION fgei;
    char buf[sizeof (NFS_SYML_TARGET)];
  } fgei_buf;
  PFILE_FULL_EA_INFORMATION pffei;
  int res = 0;

  /* To find out if the file is a symlink and to get the symlink target,
     try to fetch the NfsSymlinkTargetName EA. */
  fgei_buf.fgei.NextEntryOffset = 0;
  fgei_buf.fgei.EaNameLength = sizeof (NFS_SYML_TARGET) - 1;
  stpcpy (fgei_buf.fgei.EaName, NFS_SYML_TARGET);
  pffei = (PFILE_FULL_EA_INFORMATION) tp.w_get ();
  status = NtQueryEaFile (h, &io, pffei, NT_MAX_PATH * sizeof (WCHAR), TRUE,
			  &fgei_buf.fgei, sizeof fgei_buf, NULL, TRUE);
  if (NT_SUCCESS (status) && pffei->EaValueLength > 0)
    {
      PWCHAR spath = (PWCHAR)
		     (pffei->EaName + pffei->EaNameLength + 1);
      res = sys_wcstombs (contents, SYMLINK_MAX + 1,
		      spath, pffei->EaValueLength);
      pflags = PATH_SYMLINK;
    }
  return res;
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
      if (srcbuf[1] != ':') /* native UNC path */
	*(srcbuf += 2) = '\\';
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
  SCAN_JUSTCHECKTHIS, /* Never try to append a suffix. */
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

  const char *fname = strrchr (in_path, '\\');
  fname = fname ? fname + 1 : in_path;
  char *ext_here = strrchr (fname, '.');
  path = in_path;
  eopath = strchr (path, '\0');

  if (!ext_here)
    goto noext;

  if (suffixes)
    {
      /* Check if the extension matches a known extension */
      for (const suffix_info *ex = in_suffixes; ex->name != NULL; ex++)
	if (ascii_strcasematch (ext_here, ex->name))
	  {
	    nextstate = SCAN_JUSTCHECK;
	    suffixes = NULL;	/* Has an extension so don't scan for one. */
	    goto done;
	  }
    }

  /* Didn't match.  Use last resort -- .lnk. */
  if (ascii_strcasematch (ext_here, ".lnk"))
    {
      nextstate = SCAN_HASLNK;
      suffixes = NULL;
    }

 noext:
  ext_here = eopath;

 done:
  /* Avoid attaching suffixes if the resulting filename would be invalid. */
  if (eopath - fname > NAME_MAX - 4)
    {
      nextstate = SCAN_JUSTCHECKTHIS;
      suffixes = NULL;
    }
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
	  case SCAN_JUSTCHECKTHIS:
	    nextstate = SCAN_DONE;
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
symlink_info::check (char *path, const suffix_info *suffixes, unsigned opt,
		     fs_info &fs)
{
  HANDLE h = NULL;
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
  ULONG ci_flag = cygwin_shared->obcaseinsensitive || (pflags & PATH_NOPOSIX)
		  ? OBJ_CASE_INSENSITIVE : 0;

  /* TODO: Temporarily do all char->UNICODE conversion here.  This should
     already be slightly faster than using Ascii functions. */
  tmp_pathbuf tp;
  UNICODE_STRING upath;
  OBJECT_ATTRIBUTES attr;
  tp.u_get (&upath);
  InitializeObjectAttributes (&attr, &upath, ci_flag, NULL, NULL);

  PVOID eabuf = &nfs_aol_ffei;
  ULONG easize = sizeof nfs_aol_ffei;

  while (suffix.next ())
    {
      FILE_BASIC_INFORMATION fbi;
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      bool no_ea = false;
      bool fs_update_called = false;

      error = 0;
      get_nt_native_path (suffix.path, upath);
      if (h)
	{
	  NtClose (h);
	  h = NULL;
	}
      /* The EA given to NtCreateFile allows to get a handle to a symlink on
	 an NFS share, rather than getting a handle to the target of the
	 symlink (which would spoil the task of this method quite a bit).
	 Fortunately it's ignored on most other file systems so we don't have
	 to special case NFS too much. */
      status = NtCreateFile (&h,
			     READ_CONTROL | FILE_READ_ATTRIBUTES | FILE_READ_EA,
			     &attr, &io, NULL, 0, FILE_SHARE_VALID_FLAGS,
			     FILE_OPEN,
			     FILE_OPEN_REPARSE_POINT
			     | FILE_OPEN_FOR_BACKUP_INTENT,
			     eabuf, easize);
      /* No right to access EAs or EAs not supported? */
      if (status == STATUS_ACCESS_DENIED || status == STATUS_EAS_NOT_SUPPORTED
	  || status == STATUS_NOT_SUPPORTED
	  /* Or a bug in Samba 3.2.x (x <= 7) when accessing a share's root dir
	     which has EAs enabled? */
	  || status == STATUS_INVALID_PARAMETER)
	{
	  no_ea = true;
	  /* If EAs are not supported, there's no sense to check them again
	     with suffixes attached.  So we set eabuf/easize to 0 here once. */
	  if (status == STATUS_EAS_NOT_SUPPORTED
	      || status == STATUS_NOT_SUPPORTED)
	    {
	      eabuf = NULL;
	      easize = 0;
	    }
	  status = NtOpenFile (&h, READ_CONTROL | FILE_READ_ATTRIBUTES,
			       &attr, &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_REPARSE_POINT
			       | FILE_OPEN_FOR_BACKUP_INTENT);
	}
      if (status == STATUS_OBJECT_NAME_NOT_FOUND && ci_flag == 0
	  && wincap.has_broken_udf ())
        {
	  /* On NT 5.x UDF is broken (at least) in terms of case sensitivity.
	     When trying to open a file case sensitive, the file appears to be
	     non-existant.  Another bug is described in fs_info::update. */
	  attr.Attributes = OBJ_CASE_INSENSITIVE;
	  status = NtOpenFile (&h, READ_CONTROL | FILE_READ_ATTRIBUTES,
			       &attr, &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_REPARSE_POINT
			       | FILE_OPEN_FOR_BACKUP_INTENT);
	  attr.Attributes = 0;
	  if (NT_SUCCESS (status))
	    {
	      fs.update (&upath, h);
	      if (fs.is_udf ())
		fs_update_called = true;
	      else
	      	{
		  NtClose (h);
		  status = STATUS_OBJECT_NAME_NOT_FOUND;
		}
	    }
	}
      if (NT_SUCCESS (status)
	  && NT_SUCCESS (status
			 = NtQueryInformationFile (h, &io, &fbi, sizeof fbi,
						   FileBasicInformation)))
	fileattr = fbi.FileAttributes;
      else
	{
	  debug_printf ("%p = NtQueryInformationFile (%S)", status, &upath);
	  h = NULL;
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
	      || status == STATUS_OBJECT_NAME_INVALID
	      || status == STATUS_NO_MEDIA_IN_DEVICE)
	    {
	      set_error (ENOENT);
	      goto file_not_symlink;
	    }
	  if (status != STATUS_OBJECT_NAME_NOT_FOUND
	      && status != STATUS_NO_SUCH_FILE) /* ENOENT on NFS or 9x share */
	    {
	      /* The file exists, but the user can't access it for one reason
		 or the other.  To get the file attributes we try to access the
		 information by opening the parent directory and getting the
		 file attributes using a matching NtQueryDirectoryFile call. */
	      UNICODE_STRING dirname, basename;
	      OBJECT_ATTRIBUTES dattr;
	      HANDLE dir;
	      struct {
		FILE_DIRECTORY_INFORMATION fdi;
		WCHAR dummy_buf[NAME_MAX + 1];
	      } fdi_buf;

	      RtlSplitUnicodePath (&upath, &dirname, &basename);
	      InitializeObjectAttributes (&dattr, &dirname, ci_flag,
					  NULL, NULL);
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
		  status = NtQueryDirectoryFile (dir, NULL, NULL, NULL, &io,
						 &fdi_buf, sizeof fdi_buf,
						 FileDirectoryInformation,
						 TRUE, &basename, TRUE);
		  /* Take the opportunity to check file system while we're
		     having the handle to the parent dir. */
		  fs.update (&upath, h);
		  NtClose (dir);
		  if (!NT_SUCCESS (status))
		    {
		      debug_printf ("%p = NtQueryDirectoryFile(%S)",
				    status, &dirname);
		      if (status == STATUS_NO_SUCH_FILE)
			{
			  /* This can happen when trying to access files
			     which match DOS device names on SMB shares.
			     NtOpenFile failed with STATUS_ACCESS_DENIED,
			     but the NtQueryDirectoryFile tells us the
			     file doesn't exist.  We're suspicious in this
			     case and retry with the next suffix instead of
			     just giving up. */
			  set_error (ENOENT);
			  continue;
			}
		      fileattr = 0;
		    }
		  else
		    fileattr = fdi_buf.fdi.FileAttributes;
		}
	      ext_tacked_on = !!*ext_here;
	      goto file_not_symlink;
	    }
	  set_error (ENOENT);
	  continue;
	}

      /* Check file system while we're having the file open anyway.
	 This speeds up path_conv noticably (~10%). */
      if (!fs_update_called)
	fs.update (&upath, h);

      ext_tacked_on = !!*ext_here;

      res = -1;

      /* Windows shortcuts are potentially treated as symlinks.  Valid Cygwin
	 & U/WIN shortcuts are R/O, but definitely not directories. */
      if ((fileattr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY))
	  == FILE_ATTRIBUTE_READONLY && suffix.lnk_match ())
	{
	  res = check_shortcut (h);
	  if (!res)
	    {
	      /* If searching for `foo' and then finding a `foo.lnk' which is
		 no shortcut, return the same as if file not found. */
	      if (ext_tacked_on)
		{
		  fileattr = INVALID_FILE_ATTRIBUTES;
		  set_error (ENOENT);
		  continue;
		}
	    }
	  else if (contents[0] != ':' || contents[1] != '\\'
		   || !parse_device (contents))
	    break;
	}

      /* If searching for `foo' and then finding a `foo.lnk' which is
	 no shortcut, return the same as if file not found. */
      else if (suffix.lnk_match () && ext_tacked_on)
        {
	  fileattr = INVALID_FILE_ATTRIBUTES;
	  set_error (ENOENT);
	  continue;
	}

      /* Reparse points are potentially symlinks.  This check must be
	 performed before checking the SYSTEM attribute for sysfile
	 symlinks, since reparse points can have this flag set, too.
	 For instance, Vista starts to create a couple of reparse points
	 with SYSTEM and HIDDEN flags set. */
      else if (fileattr & FILE_ATTRIBUTE_REPARSE_POINT)
	{
	  res = check_reparse_point (h);
	  if (res == -1)
	    {
	      /* Volume mount point.  The filesystem information for the top
		 level directory should be for the volume top level directory
		 itself, rather than for the reparse point itself.  So we
		 fetch the filesystem information again, but with a NULL
		 handle.  This does what we want because fs_info::update opens
		 the handle without FILE_OPEN_REPARSE_POINT. */
	      fs.update (&upath, NULL);
	    }
	  else if (res)
	    break;
	}

      /* This is the old Cygwin method creating symlinks.  A symlink will
	 have the `system' file attribute.  Only files can be symlinks
	 (which can be symlinks to directories). */
      else if ((fileattr & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY))
	       == FILE_ATTRIBUTE_SYSTEM)
	{
	  res = check_sysfile (h);
	  if (res)
	    break;
	}

      /* If the file could be opened with FILE_READ_EA, and if it's on a
	 NFS share, check if it's a symlink.  Only files can be symlinks
	 (which can be symlinks to directories). */
      else if (fs.is_nfs () && !no_ea && !(fileattr & FILE_ATTRIBUTE_DIRECTORY))
	{
	  res = check_nfs_symlink (h);
	  if (res)
	    break;
	}

    /* Normal file. */
    file_not_symlink:
      issymlink = false;
      syscall_printf ("%s", isdevice ? "is a device" : "not a symlink");
      res = 0;
      break;
    }

  if (h)
    NtClose (h);

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
  ext_tacked_on = false;
  ext_here = NULL;
  extn = major = minor = mode = 0;
  return strlen (path);
}

/* readlink system call */

extern "C" ssize_t
readlink (const char *path, char *buf, size_t buflen)
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

  ssize_t len = min (buflen, strlen (pathbuf.get_win32 ()));
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

extern "C" char *
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

/* getwd: Legacy. */
extern "C" char *
getwd (char *buf)
{
  return getcwd (buf, PATH_MAX + 1);  /*Per SuSv3!*/
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
  if (!path.exists ())
    set_errno (ENOENT);
  else if (!path.isdir ())
    set_errno (ENOTDIR);
  else if (!isvirtual_dev (devn))
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
		  cygheap->cwd.get_posix (), path.get_nt_native_path ());
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

extern "C" ssize_t
cygwin_conv_path (cygwin_conv_path_t what, const void *from, void *to,
		  size_t size)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  path_conv p;
  size_t lsiz = 0;
  char *buf = NULL;
  int error = 0;
  bool relative = !!(what & CCP_RELATIVE);
  what &= ~CCP_RELATIVE;

  switch (what)
    {
    case CCP_POSIX_TO_WIN_A:
      {
	p.check ((const char *) from,
		 PC_POSIX | PC_SYM_FOLLOW | PC_NO_ACCESS_CHECK | PC_NOWARN
		 | (relative ? PC_NOFULL : 0));
	if (p.error)
	  return_with_errno (p.error);
	PUNICODE_STRING up = p.get_nt_native_path ();
	buf = tp.c_get ();
	sys_wcstombs (buf, NT_MAX_PATH, up->Buffer, up->Length / sizeof (WCHAR));
	/* Convert native path to standard DOS path. */
	if (!strncmp (buf, "\\??\\", 4))
	  {
	    buf += 4;
	    if (buf[1] != ':') /* native UNC path */
	      *(buf += 2) = '\\';
	  }
	lsiz = strlen (buf) + 1;
      }
      break;
    case CCP_POSIX_TO_WIN_W:
      p.check ((const char *) from, PC_POSIX | PC_SYM_FOLLOW
				    | PC_NO_ACCESS_CHECK | PC_NOWARN
				    | (relative ? PC_NOFULL : 0));
      if (p.error)
	return_with_errno (p.error);
      /* Relative Windows paths are always restricted to MAX_PATH chars. */
      if (relative && !isabspath (p.get_win32 ())
	  && sys_mbstowcs (NULL, 0, p.get_win32 ()) > MAX_PATH)
	{
	  /* Recreate as absolute path. */
	  p.check ((const char *) from, PC_POSIX | PC_SYM_FOLLOW
					| PC_NO_ACCESS_CHECK | PC_NOWARN);
	  if (p.error)
	    return_with_errno (p.error);
	}
      lsiz = (p.get_wide_win32_path_len () + 1) * sizeof (WCHAR);
      break;
    case CCP_WIN_A_TO_POSIX:
      buf = tp.c_get ();
      error = mount_table->conv_to_posix_path ((const char *) from, buf,
					       relative);
      if (error)
	return_with_errno (error);
      lsiz = strlen (buf) + 1;
      break;
    case CCP_WIN_W_TO_POSIX:
      buf = tp.c_get ();
      error = mount_table->conv_to_posix_path ((const PWCHAR) from, buf,
					       relative);
      if (error)
	return_with_errno (error);
      lsiz = strlen (buf) + 1;
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }
  if (!size)
    return lsiz;
  if (size < lsiz)
    {
      set_errno (ENOSPC);
      return -1;
    }
  switch (what)
    {
    case CCP_POSIX_TO_WIN_A:
    case CCP_WIN_A_TO_POSIX:
    case CCP_WIN_W_TO_POSIX:
      strcpy ((char *) to, buf);
      break;
    case CCP_POSIX_TO_WIN_W:
      p.get_wide_win32_path ((PWCHAR) to);
      break;
    }
  return 0;
}

extern "C" void *
cygwin_create_path (cygwin_conv_path_t what, const void *from)
{
  void *to;
  ssize_t size = cygwin_conv_path (what, from, NULL, 0);
  if (size <= 0)
    return NULL;
  if (!(to = malloc (size)))
    return NULL;
  if (cygwin_conv_path (what, from, to, size) == -1)
    return NULL;
  return to;
}


extern "C" int
cygwin_conv_to_win32_path (const char *path, char *win32_path)
{
  return cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_RELATIVE, path, win32_path,
			   MAX_PATH);
}

extern "C" int
cygwin_conv_to_full_win32_path (const char *path, char *win32_path)
{
  return cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_ABSOLUTE, path, win32_path,
			   MAX_PATH);
}

/* This is exported to the world as cygwin_foo by cygwin.din.  */

extern "C" int
cygwin_conv_to_posix_path (const char *path, char *posix_path)
{
  return cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_RELATIVE, path, posix_path,
			   MAX_PATH);
}

extern "C" int
cygwin_conv_to_full_posix_path (const char *path, char *posix_path)
{
  return cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, path, posix_path,
			   MAX_PATH);
}

/* The realpath function is required by POSIX:2008.  */

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
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

  char *tpath;
  if (isdrive (path))
    {
      tpath = tp.c_get ();
      mount_table->cygdrive_posix_path (path, tpath, 0);
    }
  else
    tpath = (char *) path;

  path_conv real_path (tpath, PC_SYM_FOLLOW | PC_POSIX, stat_suffixes);


  /* POSIX 2008 requires malloc'ing if resolved is NULL, and states
     that using non-NULL resolved is asking for portability
     problems.  */

  if (!real_path.error && real_path.exists ())
    {
      if (!resolved)
	{
	  resolved = (char *) malloc (strlen (real_path.normalized_path) + 1);
	  if (!resolved)
	    return NULL;
	}
      strcpy (resolved, real_path.normalized_path);
      return resolved;
    }

  /* FIXME: on error, Linux puts the name of the path
     component which could not be resolved into RESOLVED, but POSIX
     does not require this.  */
  if (resolved)
    resolved[0] = '\0';
  set_errno (real_path.error ?: ENOENT);
  return NULL;
}

/* Linux provides this extension.  Since the only portable use of
   realpath requires a NULL second argument, we might as well have a
   one-argument wrapper.  */
extern "C" char *
canonicalize_file_name (const char *path)
{
  return realpath (path, NULL);
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
  /* FIXME: This method is questionable in the long run. */

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

extern "C" ssize_t
env_PATH_to_posix (const void *win32, void *posix, size_t size)
{
  return_with_errno (conv_path_list ((const char *) win32, (char *) posix,
				     size, ENV_CVT));
}

extern "C" int
cygwin_win32_to_posix_path_list (const char *win32, char *posix)
{
  return_with_errno (conv_path_list (win32, posix, MAX_PATH, 1));
}

extern "C" int
cygwin_posix_to_win32_path_list (const char *posix, char *win32)
{
  return_with_errno (conv_path_list (posix, win32, MAX_PATH, 0));
}

extern "C" ssize_t
cygwin_conv_path_list (cygwin_conv_path_t what, const void *from, void *to,
		       size_t size)
{
  /* FIXME: Path lists are (so far) always retaining relative paths. */
  what &= ~CCP_RELATIVE;
  switch (what)
    {
    case CCP_WIN_W_TO_POSIX:
    case CCP_POSIX_TO_WIN_W:
      /*FIXME*/
      api_fatal ("wide char path lists not yet supported");
      break;
    case CCP_WIN_A_TO_POSIX:
    case CCP_POSIX_TO_WIN_A:
      if (size == 0)
	return conv_path_list_buf_size ((const char *) from,
					what == CCP_WIN_A_TO_POSIX);
      return_with_errno (conv_path_list ((const char *) from, (char *) to,
					 size, what == CCP_WIN_A_TO_POSIX));
      break;
    default:
      break;
    }
  set_errno (EINVAL);
  return -1;
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
	  if (RtlEqualUnicodePathPrefix (&upath, &ro_u_uncp, TRUE))
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
      /* This is for Win32 apps only.  No case sensitivity here... */
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
      /* Workaround a problem in Vista/Longhorn which fails in subsequent
	 calls to CreateFile with ERROR_INVALID_HANDLE if the handle in
	 CurrentDirectoryHandle changes without calling SetCurrentDirectory,
	 and the filename given to CreateFile is a relative path.  It looks
	 like Vista stores a copy of the CWD handle in some other undocumented
	 place.  The NtClose/DuplicateHandle reuses the original handle for
	 the copy of the new handle and the next CreateFile works.
	 Note that this is not thread-safe (yet?) */
      NtClose (*phdl);
      if (DuplicateHandle (GetCurrentProcess (), h, GetCurrentProcess (), phdl,
			   0, TRUE, DUPLICATE_SAME_ACCESS))
	NtClose (h);
      else
	*phdl = h;
      dir = *phdl;

      /* No need to set path on init. */
      if (nat_cwd
	  /* TODO:
	     Check the length of the new CWD.  Windows can only handle
	     CWDs of up to MAX_PATH length, including a trailing backslash.
	     If the path is longer, it's not an error condition for Cygwin,
	     so we don't fail.  Windows on the other hand has a problem now.
	     For now, we just don't store the path in the PEB and proceed as
	     usual. */
	  && len <= MAX_PATH - (nat_cwd->Buffer[len - 1] == L'\\' ? 1 : 2))
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

	  PWSTR eoBuffer = win32.Buffer + (win32.Length / sizeof (WCHAR));
	  /* Remove trailing slash if one exists.  FIXME: Is there a better way to
	     do this? */
	  if ((eoBuffer - win32.Buffer) > 3 && eoBuffer[-1] == L'\\')
	    win32.Length -= sizeof (WCHAR);

	  posix_cwd = NULL;
	}
      else
	{
	  bool unc = false;

	  if (upath.Buffer[0] == L'/') /* Virtual path, don't mangle. */
	    ;
	  else if (!doit)
	    {
	      /* Convert to a Win32 path. */
	      upath.Buffer += upath.Length / sizeof (WCHAR) - len;
	      if (upath.Buffer[1] == L'\\') /* UNC path */
		unc = true;
	      upath.Length = len * sizeof (WCHAR);
	    }
	  else
	    {
	      PWSTR eoBuffer = upath.Buffer + (upath.Length / sizeof (WCHAR));
	      /* Remove trailing slash if one exists.  FIXME: Is there a better way to
		 do this? */
	      if ((eoBuffer - upath.Buffer) > 3 && eoBuffer[-1] == L'\\')
		upath.Length -= sizeof (WCHAR);
	    }
	  RtlInitEmptyUnicodeString (&win32,
				     (PWCHAR) crealloc_abort (win32.Buffer,
							      upath.Length + 2),
				     upath.Length + 2);
	  RtlCopyUnicodeString (&win32, &upath);
	  if (unc)
	    win32.Buffer[0] = L'\\';
	}
      /* Make sure it's NUL-terminated. */
      win32.Buffer[win32.Length / sizeof (WCHAR)] = L'\0';
      if (!doit)			 /* Virtual path */
	drive_length = 0;
      else if (win32.Buffer[1] == L':')	 /* X: */
	drive_length = 2;
      else if (win32.Buffer[1] == L'\\') /* UNC path */
	{
	  PWCHAR ptr = wcschr (win32.Buffer + 2, L'\\');
	  if (ptr)
	    ptr = wcschr (ptr + 1, L'\\');
	  if (ptr)
	    drive_length = ptr - win32.Buffer;
	  else
	    drive_length = win32.Length / sizeof (WCHAR);
	}
      else				 /* Shouldn't happen */
	drive_length = 0;

      tmp_pathbuf tp;
      if (!posix_cwd)
	{
	  posix_cwd = (const char *) tp.c_get ();
	  mount_table->conv_to_posix_path (win32.Buffer, (char *) posix_cwd, 0);
	}
      posix = (char *) crealloc_abort (posix, strlen (posix_cwd) + 1);
      stpcpy (posix, posix_cwd);
    }

out:
  cwd_lock.release ();
  return res;
}

/* Store incoming wchar_t path as current posix cwd.  This is called from
   setlocale so that the cwd is always stored in the right charset. */
void
cwdstuff::reset_posix (wchar_t *w_cwd)
{
  size_t len = sys_wcstombs (NULL, (size_t) -1, w_cwd);
  posix = (char *) crealloc_abort (posix, len + 1);
  sys_wcstombs (posix, len + 1, w_cwd);
}

char *
cwdstuff::get (char *buf, int need_posix, int with_chroot, unsigned ulen)
{
  MALLOC_CHECK;

  tmp_pathbuf tp;
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
      tocopy = tp.c_get ();
      sys_wcstombs (tocopy, NT_MAX_PATH, win32.Buffer,
		    win32.Length / sizeof (WCHAR));
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
etc::init (int n, POBJECT_ATTRIBUTES attr)
{
  if (n > 0)
    /* ok */;
  else if (++curr_ix <= MAX_ETC_FILES)
    n = curr_ix;
  else
    api_fatal ("internal error");

  fn[n] = *attr;
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
