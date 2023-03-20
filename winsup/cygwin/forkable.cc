/* forkable.cc

   Copyright 2015 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "perprocess.h"
#include "sync.h"
#include "environ.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "shared_info.h"
#include "dll_init.h"
#include "child_info.h"
#include "cygtls.h"
#include "exception.h"
#include <wchar.h>
#include <sys/reent.h>
#include <assert.h>
#include <tls_pbuf.h>

/* Allow concurrent processes to use the same dll or exe
 * via their hardlink while we delete our hardlink. */
extern NTSTATUS unlink_nt (path_conv &pc, bool sharable);

#define MUTEXSEP L"@"
#define PATHSEP L"\\"

/* Create the lastsepcount directories found in ntdirname, where
   counting is done along path separators (including trailing ones).
   Returns true when these directories exist afterwards, false otherways.
   The ntdirname is used for the path-splitting. */
static bool
mkdirs (PWCHAR ntdirname, int lastsepcount)
{
  bool success = true;
  int i = lastsepcount;
  for (--i; i > 0; --i)
    {
      PWCHAR lastsep = wcsrchr (ntdirname, L'\\');
      if (!lastsep)
	break;
      *lastsep = L'\0';
    }

  for (++i; i <= lastsepcount; ++i)
    {
      if (success && (i == 0 || wcslen (wcsrchr (ntdirname, L'\\')) > 1))
	{
	  UNICODE_STRING dn;
	  RtlInitUnicodeString (&dn, ntdirname);
	  OBJECT_ATTRIBUTES oa;
	  InitializeObjectAttributes (&oa, &dn, 0, NULL, NULL);
	  HANDLE dh = NULL;
	  NTSTATUS status;
	  IO_STATUS_BLOCK iosb;
	  status = NtCreateFile (&dh, GENERIC_READ | SYNCHRONIZE,
				 &oa, &iosb, NULL, FILE_ATTRIBUTE_NORMAL,
				 FILE_SHARE_READ | FILE_SHARE_WRITE,
				 FILE_CREATE,
				 FILE_DIRECTORY_FILE
				 | FILE_SYNCHRONOUS_IO_NONALERT,
				 NULL, 0);
	  if (NT_SUCCESS(status))
	    NtClose (dh);
	  else if (status != STATUS_OBJECT_NAME_COLLISION) /* already exists */
	    success = false;
	  debug_printf ("%y = NtCreateFile (%p, dir %W)", status, dh, ntdirname);
	}
      if (i < lastsepcount)
	ntdirname[wcslen (ntdirname)] = L'\\'; /* restore original value */
    }
  return success;
}

/* Recursively remove the directory specified in ntmaxpathbuf,
   using ntmaxpathbuf as the buffer to form subsequent filenames. */
static void
rmdirs (WCHAR ntmaxpathbuf[NT_MAX_PATH])
{
  PWCHAR basebuf = wcsrchr (ntmaxpathbuf, L'\\'); /* find last pathsep */
  if (basebuf && *(basebuf+1))
    basebuf += wcslen (basebuf); /* last pathsep is not trailing one */
  if (!basebuf)
    basebuf = ntmaxpathbuf + wcslen (ntmaxpathbuf);
  *basebuf = L'\0'; /* kill trailing pathsep, if any */

  NTSTATUS status;
  HANDLE hdir = dll_list::ntopenfile (ntmaxpathbuf, &status,
				      FILE_DIRECTORY_FILE |
				      FILE_DELETE_ON_CLOSE);
  if (hdir == INVALID_HANDLE_VALUE)
    return;

  *basebuf++ = L'\\'; /* (re-)add trailing pathsep */

  struct {
    FILE_DIRECTORY_INFORMATION fdi;
    WCHAR buf[NAME_MAX];
  } fdibuf;
  IO_STATUS_BLOCK iosb;

  while (NT_SUCCESS (status = NtQueryDirectoryFile (hdir, NULL, NULL, NULL,
						    &iosb,
						    &fdibuf, sizeof (fdibuf),
						    FileDirectoryInformation,
						    FALSE, NULL, FALSE)))
    {
      PFILE_DIRECTORY_INFORMATION pfdi = &fdibuf.fdi;
      while (true)
	{
	  int namelen = pfdi->FileNameLength / sizeof (WCHAR);
	  wcsncpy (basebuf, pfdi->FileName, namelen);
	  basebuf[namelen] = L'\0';

	  if (pfdi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	    {
	      if (wcscmp (basebuf, L".") && wcscmp (basebuf, L".."))
		rmdirs (ntmaxpathbuf);
	    }
	  else
	    {
	      UNICODE_STRING fn;
	      RtlInitUnicodeString (&fn, ntmaxpathbuf);

	      path_conv pc (&fn);
	      unlink_nt (pc, true); /* move to bin */
	    }

	  if (!pfdi->NextEntryOffset)
	    break;
	  pfdi = (PFILE_DIRECTORY_INFORMATION)((caddr_t)pfdi
					       + pfdi->NextEntryOffset);
	}
    }
  if (status != STATUS_NO_MORE_FILES)
    debug_printf ("%y = NtQueryDirectoryFile (%p, io %y, info %d)",
		  status, hdir, iosb.Status, iosb.Information);

  CloseHandle (hdir);
}

/* Get the NTFS file id for the real file behind the dll handle.
   As we may open a wrong (or no) file while the dll is renamed,
   we retry until we get the same file id a second time.
   We use NtQueryVirtualMemory (MemorySectionName) for the current
   file name, as GetModuleFileNameW () yields the as-loaded name.
   While we have the file handle open, also read the attributes.
   NOTE: Uses dll_list::nt_max_path_buf (). */
bool
dll::stat_real_file_once ()
{
  if (fii.IndexNumber.QuadPart != -1LL)
    return true;

  NTSTATUS fstatus;
  HANDLE fhandle = dll_list::ntopenfile (ntname, &fstatus);
  if (fhandle == INVALID_HANDLE_VALUE)
    {
      system_printf ("WARNING: Unable (ntstatus %y) to open real file %W",
		     fstatus, ntname);
      return false;
    }

  if (!dll_list::read_fii (fhandle, &fii))
    system_printf ("WARNING: Unable to read real file attributes for %W",
		   ntname);

  NtClose (fhandle);
  return fii.IndexNumber.QuadPart != -1LL;
}

/* easy use of NtOpenFile */
HANDLE
dll_list::ntopenfile (PCWCHAR ntname, NTSTATUS *pstatus, ULONG openopts,
		      ACCESS_MASK access, HANDLE rootDir)
{
  NTSTATUS status;
  if (!pstatus)
    pstatus = &status;

  UNICODE_STRING fn;
  if (openopts & FILE_OPEN_BY_FILE_ID)
    RtlInitCountedUnicodeString (&fn, ntname, 8);
  else
    RtlInitUnicodeString (&fn, ntname);

  OBJECT_ATTRIBUTES oa;
  InitializeObjectAttributes (&oa, &fn, 0, rootDir, NULL);

  access |= FILE_READ_ATTRIBUTES;
  if (openopts & FILE_DELETE_ON_CLOSE)
    access |= DELETE;
  if (openopts & FILE_DIRECTORY_FILE)
    access |= FILE_LIST_DIRECTORY;
  else
    openopts |= FILE_NON_DIRECTORY_FILE;

  access |= SYNCHRONIZE;
  openopts |= FILE_SYNCHRONOUS_IO_NONALERT;

  HANDLE fh = INVALID_HANDLE_VALUE;
  ULONG share = FILE_SHARE_VALID_FLAGS;
  IO_STATUS_BLOCK iosb;
  *pstatus = NtOpenFile (&fh, access, &oa, &iosb, share, openopts);
  if (openopts & FILE_OPEN_BY_FILE_ID)
    debug_printf ("%y = NtOpenFile (%p, a %xh, sh %xh, o %xh, io %y, by id %llX)",
	*pstatus, fh, access, share, openopts, iosb.Status, *(LONGLONG*)fn.Buffer);
  else
    debug_printf ("%y = NtOpenFile (%p, a %xh, sh %xh, o %xh, io %y, '%W')",
	*pstatus, fh, access, share, openopts, iosb.Status, fn.Buffer);

  return NT_SUCCESS(*pstatus) ? fh : INVALID_HANDLE_VALUE;
}

bool
dll_list::read_fii (HANDLE fh, PFILE_INTERNAL_INFORMATION pfii)
{
  pfii->IndexNumber.QuadPart = -1LL;

  NTSTATUS status;
  IO_STATUS_BLOCK iosb;
  status = NtQueryInformationFile (fh, &iosb,
				   pfii, sizeof (*pfii),
				   FileInternalInformation);
  if (!NT_SUCCESS (status))
    {
      system_printf ("WARNING: %y = NtQueryInformationFile (%p,"
		     " InternalInfo, io.Status %y)",
		     status, fh, iosb.Status);
      pfii->IndexNumber.QuadPart = -1LL;
      return false;
    }
  return true;
}

/* Into buf if not NULL, write the IndexNumber in pli.
   Return the number of characters (that would be) written. */
static int
format_IndexNumber (PWCHAR buf, ssize_t bufsize, LARGE_INTEGER const *pli)
{
  if (!buf)
    return 16;
  if (bufsize >= 0 && bufsize <= 16)
    return 0;
  return __small_swprintf (buf, L"%016X", pli->QuadPart);
}

/* Into buf if not NULL, write the ntname of cygwin installation_root.
   Return the number of characters (that would be) written. */
static int
rootname (PWCHAR buf, ssize_t bufsize)
{
  UNICODE_STRING &cygroot = cygheap->installation_root;
  if (!buf)
    return 6 /* "\??\UN" */ + cygroot.Length / sizeof (WCHAR);
  return dll_list::form_ntname (buf, bufsize, cygroot.Buffer) - buf;
}

/* Into buf if not NULL, write the string representation of current user sid.
   Return the number of characters (that would be) written. */
static int
sidname (PWCHAR buf, ssize_t bufsize)
{
  if (!buf)
    return 128;
  if (bufsize >= 0 && bufsize <= 128)
    return 0;
  UNICODE_STRING sid;
  WCHAR sidbuf[128+1];
  RtlInitEmptyUnicodeString (&sid, sidbuf, sizeof (sidbuf));
  RtlConvertSidToUnicodeString (&sid, cygheap->user.sid (), FALSE);
  return wcpcpy (buf, sid.Buffer) - buf;
}

/* Into buf if not NULL, write the IndexNumber of the main executable.
   Return the number of characters (that would be) written. */
static int
exename (PWCHAR buf, ssize_t bufsize)
{
  if (!buf)
    return format_IndexNumber (NULL, bufsize, NULL);
  dll *d = dlls.main_executable;
  return format_IndexNumber (buf, bufsize, &d->fii.IndexNumber);
}

/* Into buf if not NULL, write the current Windows Thread Identifier.
   Return the number of characters (that would be) written. */
static int
winthrname (PWCHAR buf, ssize_t bufsize)
{
  if (!buf)
    return sizeof (DWORD) * 4;
  if (bufsize >= 0 && bufsize <= (int)sizeof (DWORD) * 4)
    return 0;

  return __small_swprintf (buf, L"%08X%08X",
			   GetCurrentProcessId(), GetCurrentThreadId());
}

struct namepart {
  PCWCHAR text; /* used when no pathfunc, description otherwise */
  int (*textfunc)(PWCHAR buf, ssize_t bufsize);
  bool mutex_from_dir; /* on path-separators add mutex-separator */
  bool create_dir;
};
/* mutex name is formed along dir names */
static namepart const
forkable_nameparts[] = {
 /* text             textfunc  mutex_from_dir  create */
  { L"<cygroot>",    rootname,          false, false, },
  { L"\\var\\run\\",     NULL,          false, false, },
  { L"cygfork",          NULL,          true,  false, },
  { L"<sid>",         sidname,          true,  true,  },
  { L"<exe>",         exename,          false, false, },
  { MUTEXSEP,            NULL,          false, false, },
  { L"<winthr>",   winthrname,          true,  true,  },

  { NULL, NULL },
};

/* Nominate the hardlink to an individual DLL inside dirx_name,
   that ends with the path separator (hence the "x" varname).
   With NULL as dirx_name, never nominate the hardlink any more.
   With "" as dirx_name, denominate the hardlink. */
void
dll::nominate_forkable (PCWCHAR dirx_name)
{
  if (!dirx_name)
    {
      debug_printf ("type %d disable %W", type, ntname);
      forkable_ntname = NULL; /* never create a hardlink for this dll */
    }

  if (!forkable_ntname)
    return;

  PWCHAR next = wcpcpy (forkable_ntname, dirx_name);

  if (!*forkable_ntname)
    return; /* denominate */

  if (type == DLL_LOAD)
    {
      /* Multiple dynamically loaded dlls can have identical basenames
       * when loaded from different directories.  But still the original
       * basename may serve as linked dependency for another dynamically
       * loaded dll.  So we have to create a separate directory for the
       * dynamically loaded dll - using the dll's IndexNumber as name. */
      next += format_IndexNumber (next, -1, &fii.IndexNumber);
      next = wcpcpy (next, L"\\");
    }
  wcpcpy (next, modname);
}

/* Create the nominated hardlink for one indivitual dll,
   inside another subdirectory when dynamically loaded.

   We've not found a performant way yet to protect fork against
   updates to main executables and/or dlls that do not reside on
   the same NTFS filesystem as the <cygroot>/var/run/cygfork/
   directory.  But as long as the main executable can be hardlinked,
   dll redirection works for any other hardlink-able dll, while
   non-hardlink-able dlls are used from their original location. */
bool
dll::create_forkable ()
{
  if (!forkable_ntname || !*forkable_ntname)
    return true; /* disabled */

  PWCHAR ntname = forkable_ntname;

  PWCHAR last = NULL;
  bool success = true;
  if (type >= DLL_LOAD)
    {
      last = wcsrchr (ntname, L'\\');
      if (!last)
	return false;
      *last = L'\0';
      success = mkdirs (ntname, 1);
      *last = L'\\';
      if (!success)
	return false;
    }

  /* open device as parent handle for FILE_OPEN_BY_FILE_ID */
  PWCHAR devname = dll_list::nt_max_path_buf ();
  PWCHAR n = ntname;
  PWCHAR d = devname;
  int pathseps = 0;
  while (*n)
    {
      if (*d == L'\\' && ++pathseps > 4)
	break; // "\\??\\UNC\\server\\share"
      *d = *n++;
      if (*d++ == L':')
        break; // "\\??\\C:"
    }
  *d = L'\0';

  HANDLE devhandle = dll_list::ntopenfile (devname);
  if (devhandle == INVALID_HANDLE_VALUE)
    return false; /* impossible */

  int ntlen = wcslen (ntname);
  int bufsize = sizeof (FILE_LINK_INFORMATION) + ntlen * sizeof (*ntname);
  PFILE_LINK_INFORMATION pfli = (PFILE_LINK_INFORMATION) alloca (bufsize);

  wcscpy (pfli->FileName, ntname);

  pfli->FileNameLength = ntlen * sizeof (*ntname);
  pfli->ReplaceIfExists = FALSE; /* allow concurrency */
  pfli->RootDirectory = NULL;

  /* When we get STATUS_TRANSACTION_NOT_ACTIVE from hardlink creation,
     the current process has renamed the file while it had the readonly
     attribute.  The rename() function uses a transaction for combined
     writeable+rename action if possible to provide atomicity.
     Although the transaction is closed afterwards, creating a hardlink
     for this file requires the FILE_WRITE_ATTRIBUTES access, for unknown
     reason.  On the other hand, always requesting FILE_WRITE_ATTRIBUTES
     would fail for users that do not own the original file. */
  bool ret = false;
  int access = 0; /* first attempt */
  while (true)
    {
      HANDLE fh = dll_list::ntopenfile ((PCWCHAR)&fii.IndexNumber, NULL,
					FILE_OPEN_BY_FILE_ID,
					access,
					devhandle);
      if (fh == INVALID_HANDLE_VALUE)
	break; /* impossible */

      IO_STATUS_BLOCK iosb;
      NTSTATUS status = NtSetInformationFile (fh, &iosb, pfli, bufsize,
					      FileLinkInformation);
      NtClose (fh);
      debug_printf ("%y = NtSetInformationFile (%p, FileLink %W, iosb.Status %y)",
		    status, fh, pfli->FileName, iosb.Status);
      if (NT_SUCCESS (status) || status == STATUS_OBJECT_NAME_COLLISION)
	{
	  ret = true;
	  break;
	}

      if (status != STATUS_TRANSACTION_NOT_ACTIVE ||
	  access == FILE_WRITE_ATTRIBUTES)
	break;

      access = FILE_WRITE_ATTRIBUTES; /* second attempt */
    }

  NtClose (devhandle);

  return ret;
}

/* return the number of characters necessary to store one forkable name */
size_t
dll_list::forkable_ntnamesize (dll_type type, PCWCHAR fullntname, PCWCHAR modname)
{
  /* per process, this is the first forkables-method ever called */
  if (cygwin_shared->forkable_hardlink_support == 0) /* Unknown */
    {
      /* check existence of forkables dir */
      /* nt_max_path_buf () is already used in dll_list::alloc.
         But as this is run in the very first cygwin process only,
	 using some heap is not a performance issue here. */
      PWCHAR pbuf = (PWCHAR) cmalloc_abort (HEAP_BUF,
					    NT_MAX_PATH * sizeof (WCHAR));
      PWCHAR pnext = pbuf;
      for (namepart const *part = forkable_nameparts; part->text; ++part)
	{
	  if (part->textfunc)
	    pnext += part->textfunc (pnext, -1);
	  else
	    pnext += __small_swprintf (pnext, L"%W", part->text);
	  if (part->mutex_from_dir)
	    break; /* up to first mutex-naming dir */
	}

      UNICODE_STRING fn;
      RtlInitUnicodeString (&fn, pbuf);

      HANDLE dh = INVALID_HANDLE_VALUE;
      fs_info fsi;
      if (fsi.update (&fn, NULL) &&
/* FIXME: !fsi.is_readonly () && */
	  fsi.is_ntfs ())
	dh = ntopenfile (pbuf, NULL, FILE_DIRECTORY_FILE);
      if (dh != INVALID_HANDLE_VALUE)
	{
	  cygwin_shared->forkable_hardlink_support = 1; /* Yes */
	  NtClose (dh);
	  debug_printf ("enabled");
	}
      else
	{
	  cygwin_shared->forkable_hardlink_support = -1; /* No */
	  debug_printf ("disabled, missing or not on NTFS %W", fn.Buffer);
	}
      cfree (pbuf);
    }

  if (!forkables_supported ())
      return 0;

  if (!forkables_dirx_size)
    {
      DWORD forkables_mutex_size = 0;
      bool needsep = false;
      for (namepart const *part = forkable_nameparts; part->text; ++part)
	{
	  if (needsep)
	    {
	      forkables_dirx_size += wcslen (PATHSEP);
	      forkables_mutex_size += wcslen (MUTEXSEP);
	    }
	  needsep = part->mutex_from_dir;
	  int len = 0;
	  if (part->textfunc)
	    len = part->textfunc (NULL, 0);
	  else
	    len = wcslen (part->text);
	  forkables_dirx_size += len;
	  forkables_mutex_size += len;
	}
      /* trailing path sep */
      forkables_dirx_size += wcslen (PATHSEP);
      /* trailing zeros */
      ++forkables_dirx_size;
      ++forkables_mutex_size;

      /* allocate here, to avoid cygheap size changes during fork */
      forkables_dirx_ntname = (PWCHAR) cmalloc (HEAP_2_DLL,
	  (forkables_dirx_size + forkables_mutex_size) *
	    sizeof (*forkables_dirx_ntname));
      *forkables_dirx_ntname = L'\0';

      forkables_mutex_name = forkables_dirx_ntname + forkables_dirx_size;
      *forkables_mutex_name = L'\0';
    }

  size_t ret = forkables_dirx_size;
  if (type >= DLL_LOAD)
    ret += format_IndexNumber (NULL, -1, NULL) + 1; /* one more directory */
  return ret + wcslen (modname);
}

/* Prepare top-level names necessary to nominate individual DLL hardlinks,
   eventually releasing/removing previous forkable hardlinks. */
void
dll_list::prepare_forkables_nomination ()
{
  PWCHAR pbuf = nt_max_path_buf ();

  bool needsep = false;
  bool domutex = false;
  namepart const *part;
  for (part = forkable_nameparts; part->text; ++part)
    {
      if (part->mutex_from_dir)
	domutex = true; /* mutex naming starts with first mutex_from_dir */
      if (!domutex)
	continue;
      if (needsep)
	pbuf += __small_swprintf (pbuf, L"%W", MUTEXSEP);
      needsep = part->mutex_from_dir;
      if (part->textfunc)
	pbuf += part->textfunc (pbuf, -1);
      else
	pbuf += __small_swprintf (pbuf, L"%W", part->text);
    }

  if (!wcscmp (forkables_mutex_name, nt_max_path_buf ()))
    return; /* nothing changed */

  if (*forkables_mutex_name &&
      wcscmp (forkables_mutex_name, nt_max_path_buf ()))
    {
      /* The mutex name has changed since last fork and we either have
	 dlopen'ed a more recent or dlclose'd the most recent dll,
	 so we will not use the current forkable hardlinks any more.
	 Removing from the file system is done later, upon exit. */
      close_mutex ();
      denominate_forkables ();
    }
  wcscpy (forkables_mutex_name, nt_max_path_buf ());

  pbuf = forkables_dirx_ntname;
  needsep = false;
  for (namepart const *part = forkable_nameparts; part->text; ++part)
    {
      if (needsep)
	pbuf += __small_swprintf (pbuf, L"%W", PATHSEP);
      needsep = part->mutex_from_dir;
      if (part->textfunc)
	pbuf += part->textfunc (pbuf, -1);
      else
	pbuf += __small_swprintf (pbuf, L"%W", part->text);
    }
  pbuf += __small_swprintf (pbuf, L"%W", PATHSEP);

  debug_printf ("forkables dir %W", forkables_dirx_ntname);
  debug_printf ("forkables mutex %W", forkables_mutex_name);
}

/* Test if creating hardlinks is necessary. If creating hardlinks is possible
   in general, each individual dll is tested if its previously created
   hardlink (if any, or the original file) still is the same.
   Testing is protected against hardlink removal by concurrent processes. */
void
dll_list::update_forkables_needs ()
{
  if (forkables_created)
    {
      /* already have created hardlinks in this process, ... */
      dll *d = &start;
      while ((d = d->next) != NULL)
	if (d->forkable_ntname && !*d->forkable_ntname)
	  {
	    /* ... but another dll was loaded since last fork */
	    debug_printf ("needed, since last fork loaded %W", d->ntname);
	    forkables_created = false;
	    break;
	  }
    }
}

/* Create the nominated forkable hardlinks and directories as necessary,
   mutex-protected to avoid concurrent processes removing them. */
bool
dll_list::update_forkables ()
{
  /* existence of mutex indicates that we use these hardlinks */
  if (!forkables_mutex)
    {
      /* neither my parent nor myself did have need for hardlinks yet */
      forkables_mutex = CreateMutexW (&sec_none, FALSE,
				      forkables_mutex_name);
      debug_printf ("%p = CreateMutexW (%W): %E",
		    forkables_mutex, forkables_mutex_name);
      if (!forkables_mutex)
	return false;

      /* Make sure another process does not rmdirs_synchronized () */
      debug_printf ("WFSO (%p, %W, inf)...",
		    forkables_mutex, forkables_mutex_name);
      DWORD ret = WaitForSingleObject (forkables_mutex, INFINITE);
      debug_printf ("%u = WFSO (%p, %W)",
		    ret, forkables_mutex, forkables_mutex_name);
      switch (ret)
	{
	case WAIT_OBJECT_0:
	case WAIT_ABANDONED:
	  break;
	default:
	  system_printf ("cannot wait for mutex %W: %E",
			 forkables_mutex_name);
	  return false;
	}

      BOOL bret = ReleaseMutex (forkables_mutex);
      debug_printf ("%d = ReleaseMutex (%p, %W)",
		    bret, forkables_mutex, forkables_mutex_name);
    }

  return create_forkables ();
}

/* Create the nominated forkable hardlinks and directories as necessary,
   as well as the .local file for dll-redirection. */
bool
dll_list::create_forkables ()
{
  bool success = true;

  int lastsepcount = 1; /* we have trailing pathsep */
  for (namepart const *part = forkable_nameparts; part->text; ++part)
    if (part->create_dir)
      ++lastsepcount;

  PWCHAR ntname = nt_max_path_buf ();
  wcsncpy (ntname, forkables_dirx_ntname, NT_MAX_PATH);

  if (!mkdirs (ntname, lastsepcount))
    success = false;

  if (success)
    {
      /* create the DotLocal file as empty file */
      wcsncat (ntname, main_executable->modname, NT_MAX_PATH);
      wcsncat (ntname, L".local", NT_MAX_PATH);

      UNICODE_STRING fn;
      RtlInitUnicodeString (&fn, ntname);

      OBJECT_ATTRIBUTES oa;
      InitializeObjectAttributes (&oa, &fn, 0, NULL, NULL);
      HANDLE hlocal = NULL;
      NTSTATUS status;
      IO_STATUS_BLOCK iosb;
      status = NtCreateFile (&hlocal, GENERIC_WRITE | SYNCHRONIZE,
			     &oa, &iosb, NULL, FILE_ATTRIBUTE_NORMAL,
			     FILE_SHARE_READ | FILE_SHARE_WRITE,
			     FILE_CREATE,
			     FILE_NON_DIRECTORY_FILE
			     | FILE_SYNCHRONOUS_IO_NONALERT,
			     NULL, 0);
      if (NT_SUCCESS (status))
	CloseHandle (hlocal);
      else if (status != STATUS_OBJECT_NAME_COLLISION) /* already exists */
	success = false;
      debug_printf ("%y = NtCreateFile (%p, %W)", status, hlocal, ntname);
    }

  if (success)
    {
      dll *d = &start;
      while ((d = d->next))
	if (!d->create_forkable ())
	  d->nominate_forkable (NULL); /* never again */
      debug_printf ("hardlinks created");
    }

  return success;
}

static void
rmdirs_synchronized (WCHAR ntbuf[NT_MAX_PATH], int depth, int maxdepth,
		     PFILE_DIRECTORY_INFORMATION pfdi, ULONG fdisize)
{
  if (depth == maxdepth)
    {
      debug_printf ("sync on %W", ntbuf);
      /* calculate mutex name from path parts, using
	 full path name length to allocate mutex name buffer */
      WCHAR mutexname[wcslen (ntbuf)];
      mutexname[0] = L'\0';
      PWCHAR mutexnext = mutexname;

      /* mutex name is formed by dir names */
      int pathcount = 0;
      for (namepart const *part = forkable_nameparts; part->text; ++part)
	if (part->mutex_from_dir)
	  ++pathcount;

      PWCHAR pathseps[pathcount];

      /* along the path separators split needed path parts */
      int i = pathcount;
      while (--i >= 0)
	if ((pathseps[i] = wcsrchr (ntbuf, L'\\')))
	  *pathseps[i] = L'\0';
	else
	  return; /* something's wrong */

      /* build the mutex name from dir names */
      for (i = 0; i < pathcount; ++i)
	{
	  if (i > 0)
	    mutexnext = wcpcpy (mutexnext, MUTEXSEP);
	  mutexnext = wcpcpy (mutexnext, &pathseps[i][1]);
	  *pathseps[i] = L'\\'; /* restore full path */
	}

      HANDLE mutex = CreateMutexW (&sec_none_nih, TRUE, mutexname);
      DWORD lasterror = GetLastError ();
      debug_printf ("%p = CreateMutexW (%W): %E", mutex, mutexname);
      if (mutex)
	{
	  if (lasterror != ERROR_ALREADY_EXISTS)
	    {
	      debug_printf ("cleaning up for mutex %W", mutexname);
	      rmdirs (ntbuf);
	    }
	  BOOL bret = CloseHandle (mutex);
	  debug_printf ("%d = CloseHandle (%p, %W): %E",
			bret, mutex, mutexname);
	}
      return;
    }

  IO_STATUS_BLOCK iosb;
  NTSTATUS status;

  HANDLE hdir = dll_list::ntopenfile (ntbuf, &status,
				 FILE_DIRECTORY_FILE |
				 (depth ? FILE_DELETE_ON_CLOSE : 0));
  if (hdir == INVALID_HANDLE_VALUE)
    return;

  PWCHAR plast = ntbuf + wcslen (ntbuf);
  while (NT_SUCCESS (status = NtQueryDirectoryFile (hdir,
						    NULL, NULL, NULL, &iosb,
						    pfdi, fdisize,
						    FileDirectoryInformation,
						    TRUE, NULL, FALSE)))
    if (pfdi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
	int namelen = pfdi->FileNameLength / sizeof (WCHAR);
	if (!wcsncmp (pfdi->FileName, L".", namelen) ||
	    !wcsncmp (pfdi->FileName, L"..", namelen))
	  continue;
	*plast = L'\\';
	wcsncpy (plast+1, pfdi->FileName, namelen);
	plast[1+namelen] = L'\0';
	rmdirs_synchronized (ntbuf, depth+1, maxdepth, pfdi, fdisize);
	*plast = L'\0';
      }
  if (status != STATUS_NO_MORE_FILES)
    debug_printf ("%y = NtQueryDirectoryFile (%p, io %y, info %d)",
		  status, hdir, iosb.Status, iosb.Information);
  CloseHandle (hdir);
}

/* Try to lock the mutex handle with almost no timeout, then close the
   mutex handle.  Locking before closing is to get the mutex closing
   promoted synchronously, otherways we might end up with no one
   succeeding in create-with-lock, which is the precondition
   to actually remove the hardlinks from the filesystem. */
bool
dll_list::close_mutex ()
{
  if (!forkables_mutex || !*forkables_mutex_name)
    return false;

  HANDLE hmutex = forkables_mutex;
  forkables_mutex = NULL;

  bool locked = false;
  DWORD ret = WaitForSingleObject (hmutex, 1);
  debug_printf ("%u = WFSO (%p, %W, 1)",
		ret, hmutex, forkables_mutex_name);
  switch (ret)
    {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED:
      locked = true;
      break;
    case WAIT_TIMEOUT:
      break;
    default:
      system_printf ("error locking mutex %W: %E", forkables_mutex_name);
      break;
    }
  BOOL bret = CloseHandle (hmutex);
  debug_printf ("%d = CloseHandle (%p, %W): %E",
		bret, hmutex, forkables_mutex_name);
  return locked;
}

/* Release the forkable hardlinks, and remove them if the
   mutex can be create-locked after locked-closing. */
void
dll_list::cleanup_forkables ()
{
  if (!forkables_supported ())
    return;

  bool locked = close_mutex ();

  /* Start the removal below with current forkables dir,
     which is cleaned in denominate_forkables (). */
  PWCHAR buf = nt_max_path_buf ();
  PWCHAR pathsep = wcpncpy (buf, forkables_dirx_ntname, NT_MAX_PATH);
  buf[NT_MAX_PATH-1] = L'\0';

  denominate_forkables ();

  if (!locked)
    return;

  /* drop last path separator */
  while (--pathsep >= buf && *pathsep != L'\\');
  *pathsep = L'\0';

  try_remove_forkables (buf, pathsep - buf, NT_MAX_PATH);
}

void
dll_list::try_remove_forkables (PWCHAR dirbuf, size_t dirlen, size_t dirbufsize)
{
  /* Instead of just the current forkables, try to remove any forkables
     found, to ensure some cleanup even in situations like power-loss. */
  PWCHAR end = dirbuf + wcslen (dirbuf);
  int backcount = 0;
  for (namepart const *part = forkable_nameparts; part->text; ++part)
    if (part->create_dir)
      {
	/* drop one path separator per create_dir */
	while (--end >= dirbuf && *end != L'\\');
	if (end < dirbuf)
	  return;
	*end = L'\0';
	++backcount;
      }

  /* reading one at a time to reduce stack pressure */
  struct {
    FILE_DIRECTORY_INFORMATION fdi;
    WCHAR buf[NAME_MAX];
  } fdibuf;
  rmdirs_synchronized (dirbuf, 0, backcount, &fdibuf.fdi, sizeof (fdibuf));
}

void
dll_list::denominate_forkables ()
{
  *forkables_dirx_ntname = L'\0';
  *forkables_mutex_name = L'\0';

  dll *d = &start;
  while ((d = d->next))
    d->nominate_forkable (forkables_dirx_ntname);
}

/* Set or clear HANDLE_FLAG_INHERIT for all handles necessary
   to maintain forkables-hardlinks. */
void
dll_list::set_forkables_inheritance (bool inherit)
{
  DWORD mask = HANDLE_FLAG_INHERIT;
  DWORD flags = inherit ? HANDLE_FLAG_INHERIT : 0;

  if (forkables_mutex)
    SetHandleInformation (forkables_mutex, mask, flags);
}

/* create the forkable hardlinks, if necessary */
void
dll_list::request_forkables ()
{
  if (!forkables_supported ())
    return;

  prepare_forkables_nomination ();

  update_forkables_needs ();

  set_forkables_inheritance (true);

  if (forkables_created)
    return; /* nothing new to create */

  dll *d = &start;
  while ((d = d->next))
    d->nominate_forkable (forkables_dirx_ntname);

  if (update_forkables ())
    forkables_created = true;
}


void
dll_list::release_forkables ()
{
  if (!forkables_supported ())
    return;

  set_forkables_inheritance (false);
}
