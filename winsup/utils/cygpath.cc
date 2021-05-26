/* cygpath.cc -- convert pathnames between Windows and Unix format

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <io.h>
#include <sys/fcntl.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <ctype.h>
#include <wctype.h>
#include <errno.h>

#define NOCOMATTRIBUTE
#include <windows.h>
#include <userenv.h>
#include <shlobj.h>
#include <ntdef.h>
#include <ntdll.h>

#include "wide_path.h"
#include "loadlib.h"

static char *prog_name;
static char *file_arg, *output_arg;
static int path_flag, unix_flag, windows_flag, absolute_flag, cygdrive_flag;
static int shortname_flag, longname_flag;
static int ignore_flag, allusers_flag, output_flag;
static int mixed_flag, options_from_file_flag, mode_flag;
static UINT codepage;

static const char *format_type_arg;

static struct option long_options[] = {
  {(char *) "absolute", no_argument, NULL, 'a'},
  {(char *) "close", required_argument, NULL, 'c'},
  {(char *) "dos", no_argument, NULL, 'd'},
  {(char *) "file", required_argument, NULL, 'f'},
  {(char *) "help", no_argument, NULL, 'h'},
  {(char *) "ignore", no_argument, NULL, 'i'},
  {(char *) "long-name", no_argument, NULL, 'l'},
  {(char *) "mixed", no_argument, NULL, 'm'},
  {(char *) "mode", no_argument, NULL, 'M'},
  {(char *) "option", no_argument, NULL, 'o'},
  {(char *) "path", no_argument, NULL, 'p'},
  {(char *) "proc-cygdrive", no_argument, NULL, 'U'},
  {(char *) "short-name", no_argument, NULL, 's'},
  {(char *) "type", required_argument, NULL, 't'},
  {(char *) "unix", no_argument, NULL, 'u'},
  {(char *) "version", no_argument, NULL, 'V'},
  {(char *) "windows", no_argument, NULL, 'w'},
  {(char *) "allusers", no_argument, NULL, 'A'},
  {(char *) "desktop", no_argument, NULL, 'D'},
  {(char *) "homeroot", no_argument, NULL, 'H'},
  {(char *) "mydocs", no_argument, NULL, 'O'},
  {(char *) "smprograms", no_argument, NULL, 'P'},
  {(char *) "sysdir", no_argument, NULL, 'S'},
  {(char *) "windir", no_argument, NULL, 'W'},
  {(char *) "folder", required_argument, NULL, 'F'},
  {(char *) "codepage", required_argument, NULL, 'C'},
  {0, no_argument, 0, 0}
};

static char options[] = "ac:df:hilmMopst:uUVwAC:DHOPSWF:";

static void __attribute__ ((__noreturn__))
usage (FILE * stream, int status)
{
  if (!ignore_flag || !status)
    fprintf (stream, "\
Usage: %1$s (-d|-m|-u|-w|-t TYPE) [-f FILE] [OPTION]... NAME...\n\
       %1$s [-c HANDLE] \n\
       %1$s [-ADHOPSW] \n\
       %1$s [-F ID] \n\
\n\
Convert Unix and Windows format paths, or output system path information\n\
\n\
Output type options:\n\
\n\
  -d, --dos             print DOS (short) form of NAMEs (C:\\PROGRA~1\\)\n\
  -m, --mixed           like --windows, but with regular slashes (C:/WINNT)\n\
  -M, --mode            report on mode of file (binmode or textmode)\n\
  -u, --unix            (default) print Unix form of NAMEs (/cygdrive/c/winnt)\n\
  -w, --windows         print Windows form of NAMEs (C:\\WINNT)\n\
  -t, --type TYPE       print TYPE form: 'dos', 'mixed', 'unix', or 'windows'\n\
\n\
Path conversion options:\n\
\n\
  -a, --absolute        output absolute path\n\
  -l, --long-name       print Windows long form of NAMEs (with -w, -m only)\n\
  -p, --path            NAME is a PATH list (i.e., '/bin:/usr/bin')\n\
  -U, --proc-cygdrive   Emit /proc/cygdrive path instead of cygdrive prefix\n\
                        when converting Windows path to UNIX path.\n\
  -s, --short-name      print DOS (short) form of NAMEs (with -w, -m only)\n\
  -C, --codepage CP     print DOS, Windows, or mixed pathname in Windows\n\
                        codepage CP.  CP can be a numeric codepage identifier,\n\
                        or one of the reserved words ANSI, OEM, or UTF8.\n\
                        If this option is missing, %1$s defaults to the\n\
                        character set defined by the current locale.\n\
\n\
System information:\n\
\n\
  -A, --allusers        use `All Users' instead of current user for -D, -O, -P\n\
  -D, --desktop         output `Desktop' directory and exit\n\
  -H, --homeroot        output `Profiles' directory (home root) and exit\n\
  -O, --mydocs          output `My Documents' directory and exit\n\
  -P, --smprograms      output Start Menu `Programs' directory and exit\n\
  -S, --sysdir          output system directory and exit\n\
  -W, --windir          output `Windows' directory and exit\n\
  -F, --folder ID       output special folder with numeric ID and exit\n\
", prog_name);
  if (ignore_flag)
    /* nothing to do */;
  else if (stream != stdout)
    fprintf(stream, "Try `%s --help' for more information.\n", prog_name);
  else
    {
      fprintf (stream, "\
\n\
Other options:\n\
\n\
  -f, --file FILE       read FILE for input; use - to read from STDIN\n\
  -o, --option          read options from FILE as well (for use with --file)\n\
  -c, --close HANDLE    close HANDLE (for use in captured process)\n\
  -i, --ignore          ignore missing argument\n\
  -h, --help            output usage information and exit\n\
  -V, --version         output version information and exit\n\
\n");
    }
  exit (ignore_flag ? 0 : status);
}

static inline BOOLEAN
RtlAllocateUnicodeString (PUNICODE_STRING uni, ULONG size)
{
  uni->Length = 0;
  uni->MaximumLength = size / sizeof (WCHAR);
  uni->Buffer = (WCHAR *) malloc (size);
  return uni->Buffer != NULL;
}

static size_t
my_wcstombs (char *dest, const wchar_t *src, size_t n)
{
  if (codepage)
    return WideCharToMultiByte (codepage, 0, src, -1, dest, n, NULL, NULL);
  else
    return wcstombs (dest, src, n);
}

#define	HARDDISK_PREFIX		L"\\Device\\Harddisk"
#define	GLOBALROOT_PREFIX	"\\\\.\\GLOBALROOT"

static char *
get_device_name (char *path)
{
  UNICODE_STRING ntdev, tgtdev, ntdevdir;
  ANSI_STRING ans;
  OBJECT_ATTRIBUTES ntobj;
  NTSTATUS status;
  HANDLE lnk, dir;
  bool got_one = false;
  char *ret = strdup (path);
  PDIRECTORY_BASIC_INFORMATION odi = (PDIRECTORY_BASIC_INFORMATION)
				     alloca (4096);
  BOOLEAN restart;
  ULONG cont;

  if (!strncasecmp (path, GLOBALROOT_PREFIX "\\", sizeof (GLOBALROOT_PREFIX)))
    path += sizeof (GLOBALROOT_PREFIX) - 1;
  if (strncasecmp (path, "\\Device\\", 8))
    return ret;

  if (!RtlAllocateUnicodeString (&ntdev, 65534))
    return ret;
  if (!RtlAllocateUnicodeString (&tgtdev, 65534))
    return ret;
  RtlInitAnsiString (&ans, path);
  RtlAnsiStringToUnicodeString (&ntdev, &ans, FALSE);

  /* First check if the given device name is a symbolic link itself.  If so,
     query it and use the new name as actual device name to search for in the
     DOS device name directory.  If not, just use the incoming device name. */
  InitializeObjectAttributes (&ntobj, &ntdev, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenSymbolicLinkObject (&lnk, SYMBOLIC_LINK_QUERY, &ntobj);
  if (NT_SUCCESS (status))
    {
      status = NtQuerySymbolicLinkObject (lnk, &tgtdev, NULL);
      NtClose (lnk);
      if (!NT_SUCCESS (status))
	goto out;
      RtlCopyUnicodeString (&ntdev, &tgtdev);
    }
  else if (status != STATUS_OBJECT_TYPE_MISMATCH
	   && status != STATUS_OBJECT_PATH_SYNTAX_BAD)
    goto out;

  for (int i = 0; i < 2; ++i)
    {
      /* There are two DOS device directories, the local and the global dir.
	 Try both, local first. */
      RtlInitUnicodeString (&ntdevdir, i ? L"\\GLOBAL??" : L"\\??");

      /* Open the directory... */
      InitializeObjectAttributes (&ntobj, &ntdevdir, OBJ_CASE_INSENSITIVE,
				  NULL, NULL);
      status = NtOpenDirectoryObject (&dir, DIRECTORY_QUERY, &ntobj);
      if (!NT_SUCCESS (status))
	break;

      /* ...and scan it. */
      for (restart = TRUE, cont = 0;
	   NT_SUCCESS (NtQueryDirectoryObject (dir, odi, 4096, TRUE,
					       restart, &cont, NULL));
	   restart = FALSE)
	{
	  /* For each entry check if it's a symbolic link. */
	  InitializeObjectAttributes (&ntobj, &odi->ObjectName,
				      OBJ_CASE_INSENSITIVE, dir, NULL);
	  status = NtOpenSymbolicLinkObject (&lnk, SYMBOLIC_LINK_QUERY, &ntobj);
	  if (!NT_SUCCESS (status))
	    continue;
	  tgtdev.Length = 0;
	  tgtdev.MaximumLength = 512;
	  /* If so, query it and compare the target of the symlink with the
	     incoming device name. */
	  status = NtQuerySymbolicLinkObject (lnk, &tgtdev, NULL);
	  NtClose (lnk);
	  if (!NT_SUCCESS (status))
	    continue;
	  if (tgtdev.Length /* There's actually a symlink pointing to an
			       empty string: \??\GLOBALROOT -> "" */
	      && RtlEqualUnicodePathPrefix (&ntdev, &tgtdev, TRUE))
	    {
	      /* If the comparison succeeds, the name of the directory entry is
		 a valid DOS device name, if prepended with "\\.\".  Return that
		 valid DOS path. */
	      wchar_t *trailing = NULL;
	      if (ntdev.Length > tgtdev.Length)
		trailing = ntdev.Buffer + tgtdev.Length / sizeof (WCHAR);
	      ULONG len = RtlUnicodeStringToAnsiSize (&odi->ObjectName);
	      if (trailing)
		len += my_wcstombs (NULL, trailing, 0);
	      free (ret);
	      ret = (char *) malloc (len + 4);
	      strcpy (ret, "\\\\.\\");
	      ans.Length = 0;
	      ans.MaximumLength = len;
	      ans.Buffer = ret + 4;
	      RtlUnicodeStringToAnsiString (&ans, &odi->ObjectName, FALSE);
	      if (trailing)
		my_wcstombs (ans.Buffer + ans.Length, trailing,
			     ans.MaximumLength - ans.Length);
	      ans.Buffer[ans.MaximumLength - 1] = '\0';
	      got_one = true;
	      /* Special case for local disks:  It's most feasible if the
		 DOS device name reflects the DOS drive, so we check for this
		 explicitly and only return prematurely if so. */
	      if (ntdev.Length < wcslen (HARDDISK_PREFIX)
		  || wcsncasecmp (ntdev.Buffer, HARDDISK_PREFIX, 8) != 0
		  || (odi->ObjectName.Length == 2 * sizeof (WCHAR)
		      && odi->ObjectName.Buffer[1] == L':'))
		{
		  if (trailing)
		    {
		      /* If there's a trailing path, it's a perfectly valid
			 DOS pathname without the \\.\ prefix.  Unless it's
			 longer than MAX_PATH - 1 in which case it needs
			 the \\?\ prefix. */
		      if ((len = strlen (ret + 4)) >= MAX_PATH)
			ret[2] = '?';
		      else
			memmove (ret, ret + 4, strlen (ret + 4) + 1);
		    }
		  NtClose (dir);
		  goto out;
		}
	    }
	}
      NtClose (dir);
    }

out:
  free (tgtdev.Buffer);
  free (ntdev.Buffer);
  if (!got_one)
    {
      free (ret);
      ret = (char *) malloc (sizeof (GLOBALROOT_PREFIX) + strlen (path));
      if (ret)
      	stpcpy (stpcpy (ret, GLOBALROOT_PREFIX), path);
    }
  return ret;
}

static char *
get_device_paths (char *path)
{
  char *sbuf;
  char *ptr;
  int n = 1;

  ptr = path;
  while ((ptr = strchr (ptr, ';')))
    {
      ptr++;
      n++;
    }

  char *paths[n];
  DWORD acc = 0;
  int i;
  if (!n)
    return strdup ("");

  for (i = 0, ptr = path; ptr; i++)
    {
      char *next = ptr;
      ptr = strchr (ptr, ';');
      if (ptr)
	*ptr++ = 0;
      paths[i] = get_device_name (next);
      acc += strlen (paths[i]) + 1;
    }

  sbuf = (char *) malloc (acc + 1);
  if (sbuf == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }

  sbuf[0] = '\0';
  for (i = 0; i < n; i++)
    {
      strcat (strcat (sbuf, paths[i]), ";");
      free (paths[i]);
    }

  strchr (sbuf, '\0')[-1] = '\0';
  return sbuf;
}

static char *
get_short_paths (char *path)
{
  wchar_t *sbuf;
  wchar_t *sptr;
  char *next;
  char *ptr = path;
  char *end = strrchr (path, 0);
  DWORD acc = 0;
  DWORD len;

  while (ptr != NULL)
    {
      next = ptr;
      ptr = strchr (ptr, ';');
      if (ptr)
	*ptr++ = 0;
      wide_path wpath (next);
      len = GetShortPathNameW (wpath, NULL, 0);
      if (!len)
	{
	  fprintf (stderr, "%s: cannot create short name of %s\n",
		   prog_name, next);
	  exit (2);
	}
      acc += len + 1;
    }
  sptr = sbuf = (wchar_t *) malloc ((acc + 1) * sizeof (wchar_t));
  if (sbuf == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }
  ptr = path;
  for (;;)
    {
      wide_path wpath (ptr);
      len = GetShortPathNameW (wpath, sptr, acc);
      if (!len)
	{
	  fprintf (stderr, "%s: cannot create short name of %s\n",
		   prog_name, ptr);
	  exit (2);
	}

      ptr = strrchr (ptr, 0);
      sptr = wcsrchr (sptr, 0);
      if (ptr == end)
	break;
      *sptr = L';';
      ++ptr, ++sptr;
      acc -= len + 1;
    }
  len = my_wcstombs (NULL, sbuf, 0) + 1;
  ptr = (char *) malloc (len);
  if (ptr == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }
  my_wcstombs (ptr, sbuf, len);
  free (sbuf);
  return ptr;
}

static char *
get_short_name (const char *filename)
{
  wchar_t buf[32768];
  char *sbuf;
  wide_path wpath (filename);
  DWORD len = GetShortPathNameW (wpath, buf, 32768);
  if (!len)
    {
      fprintf (stderr, "%s: cannot create short name of %s\n",
	       prog_name, filename);
      exit (2);
    }
  len = my_wcstombs (NULL, buf, 0) + 1;
  sbuf = (char *) malloc (len);
  if (sbuf == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }
  my_wcstombs (sbuf, buf, len);
  return sbuf;
}

static char *
get_long_name (const char *filename, DWORD& len)
{
  char *sbuf;
  wchar_t buf[32768];
  wide_path wpath (filename);

  if (!GetLongPathNameW (wpath, buf, 32768))
    wcscpy (buf, wpath);
  len = my_wcstombs (NULL, buf, 0);
  sbuf = (char *) malloc (len + 1);
  if (!sbuf)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }
  my_wcstombs (sbuf, buf, len + 1);
  return sbuf;
}

static char *
get_long_paths (char *path)
{
  char *sbuf;
  char *ptr;
  int n = 1;

  ptr = path;
  while ((ptr = strchr (ptr, ';')))
    {
      ptr++;
      n++;
    }

  char *paths[n];
  DWORD acc = 0;
  int i;
  if (!n)
    return strdup ("");

  for (i = 0, ptr = path; ptr; i++)
    {
      DWORD len;
      char *next = ptr;
      ptr = strchr (ptr, ';');
      if (ptr)
	*ptr++ = 0;
      paths[i] = get_long_name (next, len);
      acc += len + 1;
    }

  sbuf = (char *) malloc (acc + 1);
  if (sbuf == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }

  sbuf[0] = '\0';
  for (i = 0; i < n; i++)
    {
      strcat (strcat (sbuf, paths[i]), ";");
      free (paths[i]);
    }

  strchr (sbuf, '\0')[-1] = '\0';
  return sbuf;
}

static void
convert_slashes (char* name)
{
  while ((name = strchr (name, '\\')) != NULL)
    {
      if (*name == '\\')
	*name = '/';
       name++;
   }
}

static bool
get_special_folder (PWCHAR wpath, int id)
{
  LPITEMIDLIST pidl = 0;
  if (SHGetSpecialFolderLocation (NULL, id, &pidl) != S_OK)
    return false;
  if (!SHGetPathFromIDListW (pidl, wpath) || !wpath[0])
    return false;
  return true;
}

static void
do_sysfolders (char option)
{
  WCHAR wbuf[MAX_PATH];
  char buf[PATH_MAX];
  BOOL iswow64 = FALSE;

  wbuf[0] = L'\0';
  switch (option)
    {
    case 'D':
      get_special_folder (wbuf, allusers_flag ? CSIDL_COMMON_DESKTOPDIRECTORY
					     : CSIDL_DESKTOPDIRECTORY);
      break;

    case 'P':
      get_special_folder (wbuf, allusers_flag ? CSIDL_COMMON_PROGRAMS
					     : CSIDL_PROGRAMS);
      break;

    case 'O':
      get_special_folder (wbuf, allusers_flag ? CSIDL_COMMON_DOCUMENTS
					     : CSIDL_PERSONAL);
      break;

    case 'F':
      {
	int val = -1, len = -1;
	if (!(sscanf (output_arg, "%i%n", &val, &len) == 1
	      && len == (int) strlen (output_arg) && val >= 0))
	  {
	    fprintf (stderr, "%s: syntax error in special folder ID %s\n",
		     prog_name, output_arg);
	    exit (1);
	  }
	get_special_folder (wbuf, val);
      }
      break;

    case 'H':
      {
	DWORD len = MAX_PATH;
	GetProfilesDirectoryW (wbuf, &len);
      }
      break;

    case 'S':
      GetSystemDirectoryW (wbuf, MAX_PATH);
      if (!windows_flag
	  && IsWow64Process (GetCurrentProcess (), &iswow64) && iswow64)
	{
	  /* When calling NtQueryInformationFile(FileNameInformation) on WOW64,
	     the returned path will point to SysWOW64.  This breaks path
	     redirection to the network related files under device/etc.  This
	     here is a bad hack to make sure that the conversion will convert
	     the case *and* stick to System32. */
	  PWCHAR last_bs = wcsrchr (wbuf, L'\\');
	  if (last_bs)
	    wcpcpy (last_bs + 1, L"Sysnative");
	}
      break;

    case 'W':
      GetSystemWindowsDirectoryW (wbuf, MAX_PATH);
      break;

    default:
      usage (stderr, 1);
    }

  if (!wbuf[0])
    {
      fprintf (stderr, "%s: failed to retrieve special folder path\n",
	       prog_name);
      return;
    }
  else if (!windows_flag)
    {
      /* The system folders are not necessarily case-correct.  To allow
	 case-sensitivity, try to correct the case.  Note that this only
	 works for local filesystems. */
      if (iswalpha (wbuf[0]) && wbuf[1] == L':' && wbuf[2] == L'\\')
	{
	  OBJECT_ATTRIBUTES attr;
	  NTSTATUS status;
	  HANDLE h;
	  IO_STATUS_BLOCK io;
	  UNICODE_STRING upath;
	  const ULONG size = sizeof (FILE_NAME_INFORMATION)
			     + PATH_MAX * sizeof (WCHAR);
	  PFILE_NAME_INFORMATION pfni = (PFILE_NAME_INFORMATION) alloca (size);

	  /* Avoid another buffer, reuse pfni. */
	  wcpcpy (wcpcpy (pfni->FileName, L"\\??\\"), wbuf);
	  RtlInitUnicodeString (&upath, pfni->FileName);
	  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE,
				      NULL, NULL);
	  status = NtOpenFile (&h, READ_CONTROL, &attr, &io,
			       FILE_SHARE_VALID_FLAGS, FILE_OPEN_REPARSE_POINT);
	  if (NT_SUCCESS (status))
	    {
	      status = NtQueryInformationFile (h, &io, pfni, size,
					       FileNameInformation);
	      if (NT_SUCCESS (status))
		{
		  pfni->FileName[pfni->FileNameLength / sizeof (WCHAR)] = L'\0';
		  wcscpy (wbuf + 2, pfni->FileName);
		}
	      NtClose (h);
	    }
	}
      if (cygwin_conv_path (CCP_WIN_W_TO_POSIX | cygdrive_flag,
			    wbuf, buf, PATH_MAX))
	fprintf (stderr, "%s: error converting \"%ls\" - %s\n",
		 prog_name, wbuf, strerror (errno));
    }
  else
    {
      if (shortname_flag)
	/* System paths are never longer than MAX_PATH.  The buffer pointers
	   in a call to GetShortPathNameW may point to the same buffer. */
	GetShortPathNameW (wbuf, wbuf, MAX_PATH);
      my_wcstombs (buf, wbuf, MAX_PATH);
      if (mixed_flag)
	convert_slashes (buf);
    }
  printf ("%s\n", buf);
}

static void
report_mode (char *filename)
{
  switch (cygwin_internal (CW_GET_BINMODE, filename))
    {
    case O_BINARY:
      printf ("%s: binary\n", filename);
      break;
    case O_TEXT:
      printf ("%s: text\n", filename);
      break;
    default:
      fprintf (stderr, "%s: file '%s' - %s\n", prog_name,
	       filename, strerror (errno));
      break;
    }
}

static void
do_pathconv (char *filename)
{
  char *buf = NULL, *tmp;
  wchar_t *buf2 = NULL;
  DWORD len = 32768;
  ssize_t err;
  bool print_tmp = false;
  cygwin_conv_path_t conv_func =
		      (unix_flag ? CCP_WIN_W_TO_POSIX : CCP_POSIX_TO_WIN_W)
		      | absolute_flag | cygdrive_flag;

  if (!filename || !filename[0])
    {
      if (ignore_flag)
	return;
      fprintf (stderr, "%s: can't convert empty path\n", prog_name);
      exit (1);
    }

  buf = (char *) malloc (len);
  if (!unix_flag)
    buf2 = (wchar_t *) malloc (len * sizeof (wchar_t));
  if (buf == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }

  if (path_flag)
    {
      if (unix_flag)
	{
	  wide_path wpath (filename, false);
	  err = cygwin_conv_path_list (conv_func, wpath, buf, len);
	}
      else
	err = cygwin_conv_path_list (conv_func, filename, buf2, len);
      if (err)
	{
	  fprintf (stderr, "%s: error converting \"%s\" - %s\n",
		   prog_name, filename, strerror (errno));
	  exit (1);
	}
      if (!unix_flag)
	{
	  my_wcstombs (buf, buf2, 32768);
	  buf = get_device_paths (tmp = buf);
	  free (tmp);
	  if (shortname_flag)
	    {
	      buf = get_short_paths (tmp = buf);
	      free (tmp);
	    }
	  if (longname_flag)
	    {
	      buf = get_long_paths (tmp = buf);
	      free (tmp);
	    }
	  if (mixed_flag)
	    convert_slashes (buf);
	}
    }
  else
    {
      if (unix_flag)
	{
	  wide_path wpath (filename);
	  err = cygwin_conv_path (conv_func, wpath, (void *) buf, len);
	}
      else
	err = cygwin_conv_path (conv_func, filename, (void *) buf2, len);
      if (err)
	{
	  fprintf (stderr, "%s: error converting \"%s\" - %s\n",
		   prog_name, filename, strerror (errno));
	  exit (1);
	}
      if (!unix_flag)
	{
	  my_wcstombs (buf, buf2, 32768);
	  buf = get_device_name (tmp = buf);
	  free (tmp);
	  if (shortname_flag)
	    {
	      buf = get_short_name (tmp = buf);
	      free (tmp);
	    }
	  if (longname_flag)
	    {
	      buf = get_long_name (tmp = buf, len);
	      free (tmp);
	    }
	  tmp = buf;
	  if (strncmp (buf, "\\\\?\\", 4) == 0)
	    {
	      len = 0;
	      if (buf[5] == ':')
		len = 4;
	      else if (!strncmp (buf + 4, "UNC\\", 4))
		len = 6;
	      if (len && strlen (buf) < MAX_PATH + len)
		{
		  tmp += len;
		  if (len == 6)
		    *tmp = '\\';
		  print_tmp = true;
		}
	    }
	  if (mixed_flag)
	    convert_slashes (tmp);
	}
    }

  puts (print_tmp ? tmp : buf);
  if (buf2)
    free (buf2);
  if (buf)
    free (buf);
}

static void
print_version ()
{
  printf ("cygpath (cygwin) %d.%d.%d\n"
	  "Path Conversion Utility\n"
	  "Copyright (C) 1998 - %s Cygwin Authors\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

static int
do_options (int argc, char **argv, int from_file)
{
  int c, o = 0;
  path_flag = 0;
  unix_flag = 0;
  windows_flag = 0;
  shortname_flag = 0;
  longname_flag = 0;
  mixed_flag = 0;
  ignore_flag = 0;
  allusers_flag = 0;
  output_flag = 0;
  mode_flag = 0;
  codepage = 0;
  cygdrive_flag = 0;
  absolute_flag = CCP_RELATIVE;
  if (!from_file)
    options_from_file_flag = 0;
  optind = 0;
  while ((c = getopt_long (argc, argv, options,
			   long_options, (int *) NULL)) != EOF)
    {
      switch (c)
	{
	case 'a':
	  absolute_flag = CCP_ABSOLUTE;
	  break;

	case 'c':
	  if (!optarg)
	    usage (stderr, 1);
	  CloseHandle ((HANDLE) strtoul (optarg, NULL, 16));
	  break;

	case 'd':
	  windows_flag = 1;
	  shortname_flag = 1;
	  break;

	case 'f':
	  if (from_file || !optarg)
	    usage (stderr, 1);
	  file_arg = optarg;
	  break;

	case 'M':
	  mode_flag = 1;
	  break;

	case 'o':
	  if (from_file)
	    usage (stderr, 1);
	  options_from_file_flag = 1;
	  break;

	case 'p':
	  path_flag = 1;
	  break;

	case 'u':
	  unix_flag = 1;
	  break;

	case 'w':
	  windows_flag = 1;
	  break;

	 case 'm':
	  windows_flag = 1;
	  mixed_flag = 1;
	  break;

	case 'l':
	  longname_flag = 1;
	  break;

	case 's':
	  shortname_flag = 1;
	  break;

	case 't':
	  if (!optarg)
	    usage (stderr, 1);

	  format_type_arg = (*optarg == '=') ? (optarg + 1) : (optarg);
	  if (strcasecmp (format_type_arg, "dos") == 0)
	    {
	      windows_flag = 1;
	      shortname_flag = 1;
	    }
	  else if (!strcasecmp (format_type_arg, "mixed"))
	    {
	      windows_flag = 1;
	      mixed_flag = 1;
	    }
	  else if (!strcasecmp (format_type_arg, "unix"))
	    unix_flag = 1;
	  else if (!strcasecmp (format_type_arg, "windows"))
	    windows_flag = 1;
	  else
	    usage (stderr, 1);
	  break;

	case 'A':
	  allusers_flag = 1;
	  break;

	case 'U':
	  cygdrive_flag = CCP_PROC_CYGDRIVE;
	  break;

	case 'C':
	  if (!optarg)
	    usage (stderr, 1);
	  if (!strcasecmp (optarg, "ANSI"))
	    codepage = GetACP ();
	  else if (!strcasecmp (optarg, "OEM"))
	    codepage = GetOEMCP ();
	  else if (!strcasecmp (optarg, "UTF8")
		   || !strcasecmp (optarg, "UTF-8"))
	    codepage = CP_UTF8;
	  else
	    {
	      char *c;
	      codepage = (UINT) strtoul (optarg, &c, 10);
	      if (*c)
		usage (stderr, 1);
	    }
	  break;

	case 'D':
	case 'H':
	case 'O':
	case 'P':
	case 'S':
	case 'W':
	  ++output_flag;
	  o = c;
	  break;

	case 'F':
	  if (!optarg)
	    usage (stderr, 1);
	  ++output_flag;
	  output_arg = optarg;
	  o = c;
	  break;

	case 'i':
	  ignore_flag = 1;
	  break;

	case 'h':
	  usage (stdout, 0);

	case 'V':
	  print_version ();
	  exit (0);

	default:
	  fprintf (stderr, "Try `%s --help' for more information.\n",
		   prog_name);
	  exit (1);
	}
    }

  /* If none of the "important" flags are set, -u is default. */
  if (!unix_flag && !windows_flag && !mode_flag
      && (!from_file ? !options_from_file_flag : 1))
    unix_flag = 1;

  /* Only one of ... */
  if (unix_flag + windows_flag + mode_flag > 1
      + (!from_file ? options_from_file_flag : 0))
    usage (stderr, 1);

  /* options_from_file_flag requires a file. */
  if (!from_file && options_from_file_flag && !file_arg)
    usage (stderr, 1);

  /* longname and shortname don't play well together. */
  if (longname_flag && shortname_flag)
    usage (stderr, 1);

  /* longname and shortname only make sense with Windows paths. */
  if ((longname_flag || shortname_flag) && !windows_flag)
    usage (stderr, 1);

  return o;
}

static void
action (int argc, char **argv, int opt)
{
  if (output_flag)
    {
      if (argv[optind])
	usage (stderr, 1);

      do_sysfolders (opt);
    }
  else
    {
      if (optind > argc - 1)
	usage (stderr, 1);

      for (int i = optind; argv[i]; i++)
	if (mode_flag)
	  report_mode (argv[i]);
	else
	  do_pathconv (argv[i]);
    }
}

int
main (int argc, char **argv)
{
  int o;

  setlocale (LC_CTYPE, "");
  prog_name = program_invocation_short_name;

  o = do_options (argc, argv, 0);

  if (!file_arg)
    action (argc, argv, o);
  else
    {
      FILE *fp;
      char buf[PATH_MAX * 2 + 1];

      if (argv[optind])
	usage (stderr, 1);

      if (strcmp (file_arg, "-"))
	{
	  if (!(fp = fopen (file_arg, "rt")))
	    {
	      perror ("cygpath");
	      exit (1);
	    }
	}
      else
	{
	  fp = stdin;
	  setmode (0, O_TEXT);
	}
      setbuf (stdout, NULL);

      while (fgets (buf, sizeof (buf), fp))
	{
	  int ac = 0;
	  char *av[4] = { NULL, NULL, NULL, NULL };
	  char *p = strchr (buf, '\n');
	  if (p)
	    *p = '\0';
	  p = buf;
	  av[ac++] = prog_name;
	  av[ac++] = p;
	  if (options_from_file_flag && *p == '-')
	    {
	      while (*p && !isspace (*p))
		++p;
	      if (*p)
		{
		  *p++ = '\0';
		  while (*p && isspace (*p))
		    ++p;
		  av[ac++] = p;
		}
	      o = do_options (ac, av, 1);
	    }
	  else
	    {
	      output_flag = 0;
	      optind = 1;
	    }
	  action (ac, av, o);
	}
    }
  exit (0);
}
