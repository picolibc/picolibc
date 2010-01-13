/* syscalls.cc: syscalls

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define fstat __FOOfstat__
#define lstat __FOOlstat__
#define stat __FOOstat__
#define _close __FOO_close__
#define _lseek __FOO_lseek__
#define _open __FOO_open__
#define _read __FOO_read__
#define _write __FOO_write__
#define _open64 __FOO_open64__
#define _lseek64 __FOO_lseek64__
#define _fstat64 __FOO_fstat64__
#define pread __FOO_pread
#define pwrite __FOO_pwrite

#include "winsup.h"
#include "winnls.h"
#include "miscfuncs.h"
#include <sys/stat.h>
#include <sys/vfs.h> /* needed for statfs */
#include <sys/statvfs.h> /* needed for statvfs */
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <utmp.h>
#include <utmpx.h>
#include <sys/uio.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>
#include <sys/wait.h>
#include <rpc.h>
#include "ntdll.h"

#undef fstat
#undef lstat
#undef stat
#undef pread
#undef pwrite

#include <cygwin/version.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "sigproc.h"
#include "pinfo.h"
#include "shared_info.h"
#include "cygheap.h"
#include "pwdgrp.h"
#include "cpuid.h"
#include "registry.h"
#include "environ.h"
#include "tls_pbuf.h"

#undef _close
#undef _lseek
#undef _open
#undef _read
#undef _write
#undef _open64
#undef _lseek64
#undef _fstat64

static int __stdcall mknod_worker (const char *, mode_t, mode_t, _major_t,
				   _minor_t);

/* Close all files and process any queued deletions.
   Lots of unix style applications will open a tmp file, unlink it,
   but never call close.  This function is called by _exit to
   ensure we don't leave any such files lying around.  */

void __stdcall
close_all_files (bool norelease)
{
  cygheap->fdtab.lock ();

  semaphore::terminate ();

  fhandler_base *fh;
  HANDLE h = NULL;

  for (int i = 0; i < (int) cygheap->fdtab.size; i++)
    if ((fh = cygheap->fdtab[i]) != NULL)
      {
#ifdef DEBUGGING
	debug_printf ("closing fd %d", i);
#endif
	if (i == 2)
	  DuplicateHandle (GetCurrentProcess (), fh->get_output_handle (),
			   GetCurrentProcess (), &h,
			   0, false, DUPLICATE_SAME_ACCESS);
	fh->close ();
	if (!norelease)
	  cygheap->fdtab.release (i);
      }

  if (!hExeced && cygheap->ctty)
    cygheap->close_ctty ();

  if (h)
    SetStdHandle (STD_ERROR_HANDLE, h);
  cygheap->fdtab.unlock ();
}

int
dup (int fd)
{
  return cygheap->fdtab.dup2 (fd, cygheap_fdnew ());
}

int
dup2 (int oldfd, int newfd)
{
  if (newfd >= OPEN_MAX_MAX)
    {
      syscall_printf ("-1 = dup2 (%d, %d) (%d too large)", oldfd, newfd, newfd);
      set_errno (EBADF);
      return -1;
    }
  return cygheap->fdtab.dup2 (oldfd, newfd);
}

static char desktop_ini[] =
  "[.ShellClassInfo]\r\nCLSID={645FF040-5081-101B-9F08-00AA002F954E}\r\n";
static BYTE info2[] =
{
  0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

enum bin_status
{
  dont_move,
  move_to_bin,
  has_been_moved
};

static bin_status
try_to_bin (path_conv &pc, HANDLE &fh, ACCESS_MASK access)
{
  bin_status bin_stat = move_to_bin;
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  HANDLE rootdir = NULL, recyclerdir = NULL, tmp_fh = NULL;
  USHORT recycler_base_len = 0, recycler_user_len = 0;
  UNICODE_STRING root, recycler, fname;
  WCHAR recyclerbuf[NAME_MAX + 1]; /* Enough for recycler + SID + filename */
  PFILE_NAME_INFORMATION pfni;
  PFILE_INTERNAL_INFORMATION pfii;
  PFILE_RENAME_INFORMATION pfri;
  FILE_DISPOSITION_INFORMATION disp = { TRUE };

  tmp_pathbuf tp;
  PBYTE infobuf = (PBYTE) tp.w_get ();

  pfni = (PFILE_NAME_INFORMATION) infobuf;
  status = NtQueryInformationFile (fh, &io, pfni, 65536, FileNameInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryInformationFile (FileNameInformation) failed, %08x",
		    status);
      goto out;
    }
  /* The filename could change, the parent dir not.  So we split both paths
     and take the prefix.  However, there are two special cases:
     - The handle refers to the root dir of the volume.
     - The handle refers to the recycler or a subdir.
     Both cases are handled by just returning and not even trying to move
     them into the recycler. */
  if (pfni->FileNameLength == 2) /* root dir. */
    goto out;
  /* Initialize recycler path. */
  RtlInitEmptyUnicodeString (&recycler, recyclerbuf, sizeof recyclerbuf);
  if (!pc.isremote ())
    {
      if (wincap.has_recycle_dot_bin ())	/* NTFS and FAT since Vista */
	RtlAppendUnicodeToString (&recycler, L"\\$Recycle.Bin\\");
      else if (pc.fs_is_ntfs ())	/* NTFS up to 2K3 */
	RtlAppendUnicodeToString (&recycler, L"\\RECYCLER\\");
      else if (pc.fs_is_fat ())	/* FAT up to 2K3 */
	RtlAppendUnicodeToString (&recycler, L"\\Recycled\\");
      else
	goto out;
      /* Is the file a subdir of the recycler? */
      RtlInitCountedUnicodeString(&fname, pfni->FileName, pfni->FileNameLength);
      if (RtlEqualUnicodePathPrefix (&fname, &recycler, TRUE))
	goto out;
      /* Is fname the recycler?  Temporarily hide trailing backslash. */
      recycler.Length -= sizeof (WCHAR);
      if (RtlEqualUnicodeString (&fname, &recycler, TRUE))
	goto out;

      /* Create root dir path from file name information. */
      RtlSplitUnicodePath (&fname, &fname, NULL);
      RtlSplitUnicodePath (pc.get_nt_native_path (), &root, NULL);
      root.Length -= fname.Length - sizeof (WCHAR);

      /* Open root directory.  All recycler bin ops are caseinsensitive. */
      InitializeObjectAttributes (&attr, &root, OBJ_CASE_INSENSITIVE,
				  NULL, NULL);
      status = NtOpenFile (&rootdir, FILE_TRAVERSE, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtOpenFile (%S) failed, %08x", &root, status);
	  goto out;
	}

      /* Strip leading backslash */
      ++recycler.Buffer;
      recycler.Length -= sizeof (WCHAR);
      /* Store length of recycler base dir, if it's necessary to create it. */
      recycler_base_len = recycler.Length;
      /* On NTFS the recycler dir contains user specific subdirs, which are the
	 actual recycle bins per user.  The name if this dir is the string
	 representation of the user SID. */
      if (pc.fs_is_ntfs ())
	{
	  UNICODE_STRING sid;
	  WCHAR sidbuf[128];
	  /* Unhide trailing backslash. */
	  recycler.Length += sizeof (WCHAR);
	  RtlInitEmptyUnicodeString (&sid, sidbuf, sizeof sidbuf);
	  /* In contrast to what MSDN claims, this function is already available
	     since NT4. */
	  RtlConvertSidToUnicodeString (&sid, cygheap->user.sid (), FALSE);
	  RtlAppendUnicodeStringToString (&recycler, &sid);
	  recycler_user_len = recycler.Length;
	}
      RtlAppendUnicodeToString (&recycler, L"\\");
    }
  /* Create hopefully unique filename.
     Since we have to stick to the current directory on remote shares, make
     the new filename at least very unlikely to match by accident.  It starts
     with ".cyg", with "cyg" transposed into the Unicode low surrogate area
     starting at U+dc00.  Use plain ASCII chars on filesystems not supporting
     Unicode.  The rest of the filename is the inode number in hex encoding
     and a hash of the full NT path in hex.  The combination allows to remove
     multiple hardlinks to the same file. */
  RtlAppendUnicodeToString (&recycler,
			    pc.fs_flags () & FILE_UNICODE_ON_DISK
			    ? L".\xdc63\xdc79\xdc67" : L".cyg");
  pfii = (PFILE_INTERNAL_INFORMATION) infobuf;
  status = NtQueryInformationFile (fh, &io, pfii, 65536,
				   FileInternalInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryInformationFile (FileInternalInformation) failed, "
		    "%08x", status);
      goto out;
    }
  RtlInt64ToHexUnicodeString (pfii->FileId.QuadPart, &recycler, TRUE);
  RtlInt64ToHexUnicodeString (hash_path_name (0, pc.get_nt_native_path ()),
			      &recycler, TRUE);
  /* Shoot. */
  pfri = (PFILE_RENAME_INFORMATION) infobuf;
  pfri->ReplaceIfExists = TRUE;
  pfri->RootDirectory = pc.isremote () ? NULL : rootdir;
  pfri->FileNameLength = recycler.Length;
  memcpy (pfri->FileName, recycler.Buffer, recycler.Length);
  status = NtSetInformationFile (fh, &io, pfri, 65536, FileRenameInformation);
  if (status == STATUS_OBJECT_PATH_NOT_FOUND && !pc.isremote ())
    {
      /* Ok, so the recycler and/or the recycler/SID directory don't exist.
	 First reopen root dir with permission to create subdirs. */
      NtClose (rootdir);
      status = NtOpenFile (&rootdir, FILE_ADD_SUBDIRECTORY, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtOpenFile (%S) failed, %08x", &recycler, status);
	  goto out;
	}
      /* Then check if recycler exists by opening and potentially creating it.
	 Yes, we can really do that.  Typically the recycle bin is created
	 by the first user actually using the bin.  The permissions are the
	 default permissions propagated from the root directory. */
      InitializeObjectAttributes (&attr, &recycler, OBJ_CASE_INSENSITIVE,
				  rootdir, NULL);
      recycler.Length = recycler_base_len;
      status = NtCreateFile (&recyclerdir,
			     READ_CONTROL
			     | (pc.fs_is_ntfs () ? 0 : FILE_ADD_FILE),
			     &attr, &io, NULL,
			     FILE_ATTRIBUTE_DIRECTORY
			     | FILE_ATTRIBUTE_SYSTEM
			     | FILE_ATTRIBUTE_HIDDEN,
			     FILE_SHARE_VALID_FLAGS, FILE_OPEN_IF,
			     FILE_DIRECTORY_FILE, NULL, 0);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtCreateFile (%S) failed, %08x", &recycler, status);
	  goto out;
	}
      /* Next, if necessary, check if the recycler/SID dir exists and
	 create it if not. */
      if (pc.fs_is_ntfs ())
	{
	  NtClose (recyclerdir);
	  recycler.Length = recycler_user_len;
	  status = NtCreateFile (&recyclerdir, READ_CONTROL | FILE_ADD_FILE,
				 &attr, &io, NULL, FILE_ATTRIBUTE_DIRECTORY
						   | FILE_ATTRIBUTE_SYSTEM
						   | FILE_ATTRIBUTE_HIDDEN,
				 FILE_SHARE_VALID_FLAGS, FILE_OPEN_IF,
				 FILE_DIRECTORY_FILE, NULL, 0);
	  if (!NT_SUCCESS (status))
	    {
	      debug_printf ("NtCreateFile (%S) failed, %08x",
			    &recycler, status);
	      goto out;
	    }
	}
      /* The desktop.ini and INFO2 (pre-Vista) files are expected by
	 Windows Explorer.  Otherwise, the created bin is treated as
	 corrupted */
      if (io.Information == FILE_CREATED)
	{
	  RtlInitUnicodeString (&fname, L"desktop.ini");
	  InitializeObjectAttributes (&attr, &fname, OBJ_CASE_INSENSITIVE,
				      recyclerdir, NULL);
	  status = NtCreateFile (&tmp_fh, FILE_GENERIC_WRITE, &attr, &io, NULL,
				 FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN,
				 FILE_SHARE_VALID_FLAGS, FILE_CREATE,
				 FILE_SYNCHRONOUS_IO_NONALERT
				 | FILE_NON_DIRECTORY_FILE, NULL, 0);
	  if (!NT_SUCCESS (status))
	    debug_printf ("NtCreateFile (%S) failed, %08x", &recycler, status);
	  else
	    {
	      status = NtWriteFile (tmp_fh, NULL, NULL, NULL, &io, desktop_ini,
				    sizeof desktop_ini - 1, NULL, NULL);
	      if (!NT_SUCCESS (status))
		debug_printf ("NtWriteFile (%S) failed, %08x", &fname, status);
	      NtClose (tmp_fh);
	    }
	  if (!wincap.has_recycle_dot_bin ()) /* No INFO2 file since Vista */
	    {
	      RtlInitUnicodeString (&fname, L"INFO2");
	      status = NtCreateFile (&tmp_fh, FILE_GENERIC_WRITE, &attr, &io,
				     NULL, FILE_ATTRIBUTE_ARCHIVE
					   | FILE_ATTRIBUTE_HIDDEN,
				     FILE_SHARE_VALID_FLAGS, FILE_CREATE,
				     FILE_SYNCHRONOUS_IO_NONALERT
				     | FILE_NON_DIRECTORY_FILE, NULL, 0);
		if (!NT_SUCCESS (status))
		  debug_printf ("NtCreateFile (%S) failed, %08x",
				&recycler, status);
		else
		{
		  status = NtWriteFile (tmp_fh, NULL, NULL, NULL, &io, info2,
					sizeof info2, NULL, NULL);
		  if (!NT_SUCCESS (status))
		    debug_printf ("NtWriteFile (%S) failed, %08x",
				  &fname, status);
		  NtClose (tmp_fh);
		}
	    }
	}
      NtClose (recyclerdir);
      /* Shoot again. */
      status = NtSetInformationFile (fh, &io, pfri, 65536,
				     FileRenameInformation);
    }
  if (!NT_SUCCESS (status))
    {
      debug_printf ("Move %S to %S failed, status = %p",
		    pc.get_nt_native_path (), &recycler, status);
      goto out;
    }
  /* Moving to the bin worked. */
  bin_stat = has_been_moved;
  /* Now we try to set the delete disposition.  If that worked, we're done.
     We try this here first, as long as we still have the open handle.
     Otherwise the below code closes the handle to allow replacing the file. */
  status = NtSetInformationFile (fh, &io, &disp, sizeof disp,
				 FileDispositionInformation);
  /* In case of success, restore R/O attribute to accommodate hardlinks.
     That leaves potentially hardlinks around with the R/O bit suddenly
     off if setting the delete disposition failed, but please, keep in
     mind this is really a border case only. */
  if ((access & FILE_WRITE_ATTRIBUTES) && NT_SUCCESS (status) && !pc.isdir ())
    NtSetAttributesFile (fh, pc.file_attributes ());
  NtClose (fh);
  fh = NULL; /* So unlink_nt doesn't close the handle twice. */
  /* On success or when trying to unlink a directory we just return here.
     The below code only works for files. */
  if (NT_SUCCESS (status) || pc.isdir ())
    goto out;
  /* The final trick.  We create a temporary file with delete-on-close
     semantic and rename that file to the file just moved to the bin.
     This typically overwrites the original file and we get rid of it,
     even if neither setting the delete dispostion, nor setting
     delete-on-close on the original file succeeds.  There are still
     cases in which this fails, for instance, when trying to delete a
     hardlink to a DLL used by the unlinking application itself. */
  RtlAppendUnicodeToString (&recycler, L"X");
  InitializeObjectAttributes (&attr, &recycler, 0, rootdir, NULL);
  status = NtCreateFile (&tmp_fh, DELETE, &attr, &io, NULL,
			 FILE_ATTRIBUTE_NORMAL, 0, FILE_SUPERSEDE,
			 FILE_NON_DIRECTORY_FILE | FILE_DELETE_ON_CLOSE,
			 NULL, 0);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("Creating file for overwriting failed, status = %p",
		    status);
      goto out;
    }
  status = NtSetInformationFile (tmp_fh, &io, pfri, 65536,
				 FileRenameInformation);
  NtClose (tmp_fh);
  if (!NT_SUCCESS (status))
    debug_printf ("Overwriting with another file failed, status = %p",
		  status);

out:
  if (rootdir)
    NtClose (rootdir);
  return bin_stat;
}

static NTSTATUS
check_dir_not_empty (HANDLE dir)
{
  IO_STATUS_BLOCK io;
  const ULONG bufsiz = 3 * sizeof (FILE_NAMES_INFORMATION)
		       + 3 * NAME_MAX * sizeof (WCHAR);
  PFILE_NAMES_INFORMATION pfni = (PFILE_NAMES_INFORMATION)
				 alloca (bufsiz);
  NTSTATUS status = NtQueryDirectoryFile (dir, NULL, NULL, 0, &io, pfni,
					  bufsiz, FileNamesInformation,
					  FALSE, NULL, TRUE);
  if (!NT_SUCCESS (status))
    {
      syscall_printf ("Checking if directory is empty failed, "
		      "status = %p", status);
      return status;
    }
  int cnt = 1;
  while (pfni->NextEntryOffset)
    {
      pfni = (PFILE_NAMES_INFORMATION)
	     ((caddr_t) pfni + pfni->NextEntryOffset);
      ++cnt;
    }
  if (cnt > 2)
    {
      syscall_printf ("Directory not empty");
      return STATUS_DIRECTORY_NOT_EMPTY;
    }
  return STATUS_SUCCESS;
}

NTSTATUS
unlink_nt (path_conv &pc)
{
  NTSTATUS status;
  HANDLE fh, fh_ro = NULL;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;

  bin_status bin_stat = dont_move;

  ACCESS_MASK access = DELETE;
  ULONG flags = FILE_OPEN_FOR_BACKUP_INTENT;
  /* Add the reparse point flag to native symlinks, otherwise we remove the
     target, not the symlink. */
  if (pc.is_rep_symlink ())
    flags |= FILE_OPEN_REPARSE_POINT;

  pc.get_object_attr (attr, sec_none_nih);
  /* If the R/O attribute is set, we have to open the file with
     FILE_WRITE_ATTRIBUTES to be able to remove this flags before trying
     to delete it.  We do this separately because there are filesystems
     out there (MVFS), which refuse a request to open a file for DELETE
     if the DOS R/O attribute is set for the file.  After removing the R/O
     attribute, just re-open the file for DELETE and go ahead. */
  if (pc.file_attributes () & FILE_ATTRIBUTE_READONLY)
    {
      access |= FILE_WRITE_ATTRIBUTES;
      status = NtOpenFile (&fh_ro, FILE_WRITE_ATTRIBUTES, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, flags);
      if (NT_SUCCESS (status))
	{
	  NtSetAttributesFile (fh_ro, pc.file_attributes ()
				      & ~FILE_ATTRIBUTE_READONLY);
	  InitializeObjectAttributes (&attr, &ro_u_empty,
				      pc.objcaseinsensitive (), fh_ro, NULL);
      	}
    }
  /* First try to open the file with only allowing sharing for delete.  If
     the file has an open handle on it, other than just for deletion, this
     will fail.  That indicates that the file has to be moved to the recycle
     bin so that it actually disappears from its directory even though its
     in use.  Otherwise, if opening doesn't fail, the file is not in use and
     we can go straight to setting the delete disposition flag. */
  status = NtOpenFile (&fh, access, &attr, &io, FILE_SHARE_DELETE, flags);
  if (status == STATUS_SHARING_VIOLATION || status == STATUS_LOCK_NOT_GRANTED)
    {
      /* STATUS_LOCK_NOT_GRANTED can be generated under not quite clear
	 circumstances when trying to open a file on NFS with FILE_SHARE_DELETE
	 only.  This has been observed with SFU 3.5 if the NFS share has been
	 mounted under a drive letter.  It's not generated for all files, but
	 only for some.  If it's generated once for a file, it will be
	 generated all the time.  It looks like wrong file state information
	 is stored within the NFS client, for no apparent reason, which never
	 times out.  Opening the file with FILE_SHARE_VALID_FLAGS will work,
	 though, and it is then possible to delete the file quite normally.

         NFS implements its own mechanism to remove in-use files which
	 looks quite similar to what we do in try_to_bin for remote files.
	 That's why we don't call try_to_bin on NFS. */
      if (!pc.fs_is_nfs ())
	bin_stat = move_to_bin;
      if (!pc.isdir () || pc.isremote ())
	status = NtOpenFile (&fh, access, &attr, &io,
			     FILE_SHARE_VALID_FLAGS, flags);
      else
	{
	  /* It's getting tricky.  The directory is opened in some process,
	     so we're supposed to move it to the recycler and mark it for
	     deletion.  But what if the directory is not empty?  The move
	     will work, but the subsequent delete will fail.  So we would
	     have to move it back.  That's bad, because the directory would
	     be moved around which results in a temporary inconsistent state.
	     So, what we do here is to test if the directory is empty.  If
	     not, we bail out with ERROR_DIR_NOT_EMTPY.  The below code
	     tests for at least three entries in the directory, ".", "..",
	     and another one.  Three entries means, not empty.  This doesn't
	     work for the root directory of a drive, but the root dir can
	     neither be deleted, nor moved anyway. */
	  status = NtOpenFile (&fh, access | FILE_LIST_DIRECTORY | SYNCHRONIZE,
			       &attr, &io, FILE_SHARE_VALID_FLAGS,
			       flags | FILE_SYNCHRONOUS_IO_NONALERT);
	  if (NT_SUCCESS (status))
	    {
	      status = check_dir_not_empty (fh);
	      if (!NT_SUCCESS (status))
		{
		  NtClose (fh);
		  if (fh_ro)
		    NtClose (fh_ro);
		  return status;
		}
	    }
	}
    }
  if (fh_ro)
    NtClose (fh_ro);
  if (!NT_SUCCESS (status))
    {
      if (status == STATUS_DELETE_PENDING)
	{
	  syscall_printf ("Delete already pending");
	  return 0;
	}
      syscall_printf ("Opening file for delete failed, status = %p", status);
      return status;
    }
  /* Try to move to bin if a sharing violation occured.  If that worked,
     we're done. */
  if (bin_stat == move_to_bin
      && (bin_stat = try_to_bin (pc, fh, access)) == has_been_moved)
    return 0;
  /* Try to set delete disposition. */
  FILE_DISPOSITION_INFORMATION disp = { TRUE };
  status = NtSetInformationFile (fh, &io, &disp, sizeof disp,
				 FileDispositionInformation);
  if (!NT_SUCCESS (status))
    {
      syscall_printf ("Setting delete disposition failed, status = %p",
		      status);
      /* Trying to delete a hardlink to a file in use by the system in some
	 way (for instance, font files) by setting the delete disposition fails
	 with STATUS_CANNOT_DELETE.  Strange enough, deleting these hardlinks
	 using delete-on-close semantic works... most of the time.

	 Don't use delete-on-close on remote shares.  If two processes
	 have open handles on a file and one of them calls unlink, the
	 file is removed from the remote share even though the other
	 process still has an open handle.  That process than gets Win32
	 error 59, ERROR_UNEXP_NET_ERR when trying to access the file.
	 Microsoft KB 837665 describes this problem as a bug in 2K3, but
	 I have reproduced it on other systems. */
      if (status == STATUS_CANNOT_DELETE && !pc.isremote ())
	{
	  HANDLE fh2;

	  /* Re-open from handle so we open the correct file no matter if it
	     has been moved to the bin or not. */
	  InitializeObjectAttributes (&attr, &ro_u_empty, 0, fh, NULL);
	  status = NtOpenFile (&fh2, DELETE, &attr, &io,
			       bin_stat == move_to_bin ? FILE_SHARE_VALID_FLAGS
						       : FILE_SHARE_DELETE,
			       flags | FILE_DELETE_ON_CLOSE);
	  if (!NT_SUCCESS (status))
	    {
	      syscall_printf ("Setting delete-on-close failed, status = %p",
			      status);
	      /* This is really the last chance.  If it hasn't been moved
		 to the bin already, try it now.  If moving to the bin
		 succeeds, we got rid of the file in some way, even if
		 unlinking didn't work. */
	      if (bin_stat == dont_move)
		bin_stat = try_to_bin (pc, fh, access);
	      if (bin_stat == has_been_moved)
		status = STATUS_SUCCESS;
	    }
	  else
	    NtClose (fh2);
	}
    }
  if (fh)
    {
      /* Restore R/O attribute to accommodate hardlinks.  Don't try this
	 with directories!  For some reason the below NtSetInformationFile
	 changes the delete disposition back to FALSE, at least on XP. */
      if ((access & FILE_WRITE_ATTRIBUTES)
	  && (!NT_SUCCESS (status) || !pc.isdir ()))
	NtSetAttributesFile (fh, pc.file_attributes ());
      NtClose (fh);
    }
  return status;
}

extern "C" int
unlink (const char *ourname)
{
  int res = -1;
  DWORD devn;
  NTSTATUS status;

  path_conv win32_name (ourname, PC_SYM_NOFOLLOW, stat_suffixes);

  if (win32_name.error)
    {
      set_errno (win32_name.error);
      goto done;
    }

  devn = win32_name.get_devn ();
  if (isproc_dev (devn))
    {
      set_errno (EROFS);
      goto done;
    }

  if (!win32_name.exists ())
    {
      syscall_printf ("unlinking a nonexistent file");
      set_errno (ENOENT);
      goto done;
    }
  else if (win32_name.isdir ())
    {
      syscall_printf ("unlinking a directory");
      set_errno (EPERM);
      goto done;
    }

  status = unlink_nt (win32_name);
  if (NT_SUCCESS (status))
    res = 0;
  else
    __seterrno_from_nt_status (status);

 done:
  syscall_printf ("%d = unlink (%s)", res, ourname);
  return res;
}

extern "C" int
_remove_r (struct _reent *, const char *ourname)
{
  path_conv win32_name (ourname, PC_SYM_NOFOLLOW);

  if (win32_name.error)
    {
      set_errno (win32_name.error);
      syscall_printf ("-1 = remove (%s)", ourname);
      return -1;
    }

  return win32_name.isdir () ? rmdir (ourname) : unlink (ourname);
}

extern "C" int
remove (const char *ourname)
{
  path_conv win32_name (ourname, PC_SYM_NOFOLLOW);

  if (win32_name.error)
    {
      set_errno (win32_name.error);
      syscall_printf ("-1 = remove (%s)", ourname);
      return -1;
    }

  return win32_name.isdir () ? rmdir (ourname) : unlink (ourname);
}

extern "C" pid_t
getpid ()
{
  return myself->pid;
}

extern "C" pid_t
_getpid_r (struct _reent *)
{
  return getpid ();
}

/* getppid: POSIX 4.1.1.1 */
extern "C" pid_t
getppid ()
{
  return myself->ppid;
}

/* setsid: POSIX 4.3.2.1 */
extern "C" pid_t
setsid (void)
{
#ifdef NEWVFORK
  vfork_save *vf = vfork_storage.val ();
  /* This is a horrible, horrible kludge */
  if (vf && vf->pid < 0)
    {
      pid_t pid = fork ();
      if (pid > 0)
	{
	  syscall_printf ("longjmping due to vfork");
	  vf->restore_pid (pid);
	}
      /* assuming that fork was successful */
    }
#endif

  if (myself->pgid == myself->pid)
    syscall_printf ("hmm.  pgid %d pid %d", myself->pgid, myself->pid);
  else
    {
      myself->ctty = -1;
      cygheap->manage_console_count ("setsid", 0);
      myself->sid = getpid ();
      myself->pgid = getpid ();
      if (cygheap->ctty)
	cygheap->close_ctty ();
      syscall_printf ("sid %d, pgid %d, %s", myself->sid, myself->pgid, myctty ());
      return myself->sid;
    }

  set_errno (EPERM);
  return -1;
}

extern "C" pid_t
getsid (pid_t pid)
{
  pid_t res;
  if (!pid)
    res = myself->sid;
  else
    {
      pinfo p (pid);
      if (p)
	res = p->sid;
      else
	{
	  set_errno (ESRCH);
	  res = -1;
	}
    }
  return res;
}

extern "C" ssize_t
read (int fd, void *ptr, size_t len)
{
  const iovec iov =
    {
      iov_base: ptr,
      iov_len: len
    };

  return readv (fd, &iov, 1);
}

extern "C" ssize_t
pread (int fd, void *ptr, size_t len, _off64_t off)
{
  ssize_t res;
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->pread (ptr, len, off);

  syscall_printf ("%d = pread (%d, %p, %d, %d), errno %d",
		  res, fd, ptr, len, off, get_errno ());
  return res;
}

extern "C" ssize_t
pwrite (int fd, void *ptr, size_t len, _off64_t off)
{
  ssize_t res;
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->pwrite (ptr, len, off);

  syscall_printf ("%d = pwrite (%d, %p, %d, %d), errno %d",
		  res, fd, ptr, len, off, get_errno ());
  return res;
}

EXPORT_ALIAS (read, _read)

extern "C" ssize_t
write (int fd, const void *ptr, size_t len)
{
  const struct iovec iov =
    {
      iov_base: (void *) ptr,	// const_cast
      iov_len: len
    };

  return writev (fd, &iov, 1);
}

EXPORT_ALIAS (write, _write)

extern "C" ssize_t
readv (int fd, const struct iovec *const iov, const int iovcnt)
{
  extern int sigcatchers;
  const int e = get_errno ();

  ssize_t res = -1;

  const ssize_t tot = check_iovec_for_read (iov, iovcnt);

  if (tot <= 0)
    {
      res = tot;
      goto done;
    }

  while (1)
    {
      cygheap_fdget cfd (fd);
      if (cfd < 0)
	break;

      if ((cfd->get_flags () & O_ACCMODE) == O_WRONLY)
	{
	  set_errno (EBADF);
	  break;
	}

      DWORD wait = cfd->is_nonblocking () ? 0 : INFINITE;

      /* Could block, so let user know we at least got here.  */
      syscall_printf ("readv (%d, %p, %d) %sblocking, sigcatchers %d",
		      fd, iov, iovcnt, wait ? "" : "non", sigcatchers);

      if (wait && (!cfd->is_slow () || cfd->uninterruptible_io ()))
	debug_printf ("no need to call ready_for_read");
      else if (!cfd->ready_for_read (fd, wait))
	{
	  res = -1;
	  goto out;
	}

      /* FIXME: This is not thread safe.  We need some method to
	 ensure that an fd, closed in another thread, aborts I/O
	 operations. */
      if (!cfd.isopen ())
	break;

      /* Check to see if this is a background read from a "tty",
	 sending a SIGTTIN, if appropriate */
      res = cfd->bg_check (SIGTTIN);

      if (!cfd.isopen ())
	{
	  res = -1;
	  break;
	}

      if (res > bg_eof)
	{
	  myself->process_state |= PID_TTYIN;
	  if (!cfd.isopen ())
	    {
	      res = -1;
	      break;
	    }
	  res = cfd->readv (iov, iovcnt, tot);
	  myself->process_state &= ~PID_TTYIN;
	}

    out:
      if (res >= 0 || get_errno () != EINTR || !_my_tls.call_signal_handler ())
	break;
      set_errno (e);
    }

done:
  syscall_printf ("%d = readv (%d, %p, %d), errno %d", res, fd, iov, iovcnt,
		  get_errno ());
  MALLOC_CHECK;
  return res;
}

extern "C" ssize_t
writev (const int fd, const struct iovec *const iov, const int iovcnt)
{
  int res = -1;
  const ssize_t tot = check_iovec_for_write (iov, iovcnt);

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    goto done;

  if (tot <= 0)
    {
      res = tot;
      goto done;
    }

  if ((cfd->get_flags () & O_ACCMODE) == O_RDONLY)
    {
      set_errno (EBADF);
      goto done;
    }

  /* Could block, so let user know we at least got here.  */
  if (fd == 1 || fd == 2)
    paranoid_printf ("writev (%d, %p, %d)", fd, iov, iovcnt);
  else
    syscall_printf  ("writev (%d, %p, %d)", fd, iov, iovcnt);

  res = cfd->bg_check (SIGTTOU);

  if (res > (int) bg_eof)
    {
      myself->process_state |= PID_TTYOU;
      res = cfd->writev (iov, iovcnt, tot);
      myself->process_state &= ~PID_TTYOU;
    }

done:
  if (fd == 1 || fd == 2)
    paranoid_printf ("%d = write (%d, %p, %d), errno %d",
		     res, fd, iov, iovcnt, get_errno ());
  else
    syscall_printf ("%d = write (%d, %p, %d), errno %d",
		    res, fd, iov, iovcnt, get_errno ());

  MALLOC_CHECK;
  return res;
}

/* _open */
/* newlib's fcntl.h defines _open as taking variable args so we must
   correspond.  The third arg if it exists is: mode_t mode. */
extern "C" int
open (const char *unix_path, int flags, ...)
{
  int res = -1;
  va_list ap;
  mode_t mode = 0;
  sig_dispatch_pending ();

  syscall_printf ("open (%s, %p)", unix_path, flags);
  myfault efault;
  if (efault.faulted (EFAULT))
    /* errno already set */;
  else if (!*unix_path)
    set_errno (ENOENT);
  else
    {
      /* check for optional mode argument */
      va_start (ap, flags);
      mode = va_arg (ap, mode_t);
      va_end (ap);

      fhandler_base *fh;
      cygheap_fdnew fd;

      if (fd >= 0)
	{
	  if (!(fh = build_fh_name (unix_path,
				    (flags & (O_NOFOLLOW | O_EXCL))
				    ?  PC_SYM_NOFOLLOW : PC_SYM_FOLLOW,
				    stat_suffixes)))
	    res = -1;		// errno already set
	  else if ((flags & O_NOFOLLOW) && fh->issymlink ())
	    {
	      delete fh;
	      res = -1;
	      set_errno (ELOOP);
	    }
	  else if ((flags & O_DIRECTORY) && !fh->pc.isdir ())
	    {
	      delete fh;
	      res = -1;
	      set_errno (ENOTDIR);
	    }
	  else if (((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) && fh->exists ())
	    {
	      delete fh;
	      res = -1;
	      set_errno (EEXIST);
	    }
	  else if (fh->is_fs_special () && fh->device_access_denied (flags))
	    {
	      delete fh;
	      res = -1;
	    }
	  else if (!fh->open (flags, (mode & 07777) & ~cygheap->umask))
	    {
	      delete fh;
	      res = -1;
	    }
	  else
	    {
	      cygheap->fdtab[fd] = fh;
	      if ((res = fd) <= 2)
		set_std_handle (res);
	    }
	}
    }

  syscall_printf ("%d = open (%s, %p)", res, unix_path, flags);
  return res;
}

EXPORT_ALIAS (open, _open )
EXPORT_ALIAS (open, _open64 )

extern "C" _off64_t
lseek64 (int fd, _off64_t pos, int dir)
{
  _off64_t res;

  if (dir != SEEK_SET && dir != SEEK_CUR && dir != SEEK_END)
    {
      set_errno (EINVAL);
      res = -1;
    }
  else
    {
      cygheap_fdget cfd (fd);
      if (cfd >= 0)
	res = cfd->lseek (pos, dir);
      else
	res = -1;
    }
  syscall_printf ("%D = lseek (%d, %D, %d)", res, fd, pos, dir);

  return res;
}

EXPORT_ALIAS (lseek64, _lseek64)

extern "C" _off_t
lseek (int fd, _off_t pos, int dir)
{
  return lseek64 (fd, (_off64_t) pos, dir);
}

EXPORT_ALIAS (lseek, _lseek)

extern "C" int
close (int fd)
{
  int res;

  syscall_printf ("close (%d)", fd);

  MALLOC_CHECK;
  cygheap_fdget cfd (fd, true);
  if (cfd < 0)
    res = -1;
  else
    {
      res = cfd->close ();
      cfd.release ();
    }

  syscall_printf ("%d = close (%d)", res, fd);
  MALLOC_CHECK;
  return res;
}

EXPORT_ALIAS (close, _close)

extern "C" int
isatty (int fd)
{
  int res;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = 0;
  else
    res = cfd->is_tty ();
  syscall_printf ("%d = isatty (%d)", res, fd);
  return res;
}
EXPORT_ALIAS (isatty, _isatty)

extern "C" int
link (const char *oldpath, const char *newpath)
{
  int res = -1;
  fhandler_base *fh;

  if (!(fh = build_fh_name (oldpath, PC_SYM_NOFOLLOW, stat_suffixes)))
    goto error;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else if (fh->pc.isdir ())
    set_errno (EPERM); /* We do not permit linking directories.  */
  else if (!fh->pc.exists ())
    set_errno (ENOENT);
  else
    res = fh->link (newpath);

  delete fh;
 error:
  syscall_printf ("%d = link (%s, %s)", res, oldpath, newpath);
  return res;
}

/* chown: POSIX 5.6.5.1 */
/*
 * chown () is only implemented for Windows NT.  Under other operating
 * systems, it is only a stub that always returns zero.
 */
static int
chown_worker (const char *name, unsigned fmode, __uid32_t uid, __gid32_t gid)
{
  int res = -1;
  fhandler_base *fh;

  if (!(fh = build_fh_name (name, fmode, stat_suffixes)))
    goto error;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else
    res = fh->fchown (uid, gid);

  delete fh;
 error:
  syscall_printf ("%d = %schown (%s,...)",
		  res, (fmode & PC_SYM_NOFOLLOW) ? "l" : "", name);
  return res;
}

extern "C" int
chown32 (const char * name, __uid32_t uid, __gid32_t gid)
{
  return chown_worker (name, PC_SYM_FOLLOW, uid, gid);
}

extern "C" int
chown (const char * name, __uid16_t uid, __gid16_t gid)
{
  return chown_worker (name, PC_SYM_FOLLOW,
		       uid16touid32 (uid), gid16togid32 (gid));
}

extern "C" int
lchown32 (const char * name, __uid32_t uid, __gid32_t gid)
{
  return chown_worker (name, PC_SYM_NOFOLLOW, uid, gid);
}

extern "C" int
lchown (const char * name, __uid16_t uid, __gid16_t gid)
{
  return chown_worker (name, PC_SYM_NOFOLLOW,
		       uid16touid32 (uid), gid16togid32 (gid));
}

extern "C" int
fchown32 (int fd, __uid32_t uid, __gid32_t gid)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    {
      syscall_printf ("-1 = fchown (%d,...)", fd);
      return -1;
    }

  int res = cfd->fchown (uid, gid);

  syscall_printf ("%d = fchown (%s,...)", res, cfd->get_name ());
  return res;
}

extern "C" int
fchown (int fd, __uid16_t uid, __gid16_t gid)
{
  return fchown32 (fd, uid16touid32 (uid), gid16togid32 (gid));
}

/* umask: POSIX 5.3.3.1 */
extern "C" mode_t
umask (mode_t mask)
{
  mode_t oldmask;

  oldmask = cygheap->umask;
  cygheap->umask = mask & 0777;
  return oldmask;
}

int
chmod_device (path_conv& pc, mode_t mode)
{
  return mknod_worker (pc.get_win32 (), pc.dev.mode & S_IFMT, mode, pc.dev.major, pc.dev.minor);
}

#define FILTERED_MODE(m)	((m) & (S_ISUID | S_ISGID | S_ISVTX \
					| S_IRWXU | S_IRWXG | S_IRWXO))

/* chmod: POSIX 5.6.4.1 */
extern "C" int
chmod (const char *path, mode_t mode)
{
  int res = -1;
  fhandler_base *fh;
  if (!(fh = build_fh_name (path, PC_SYM_FOLLOW, stat_suffixes)))
    goto error;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else
    res = fh->fchmod (FILTERED_MODE (mode));

  delete fh;
 error:
  syscall_printf ("%d = chmod (%s, %p)", res, path, mode);
  return res;
}

/* fchmod: P96 5.6.4.1 */

extern "C" int
fchmod (int fd, mode_t mode)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    {
      syscall_printf ("-1 = fchmod (%d, 0%o)", fd, mode);
      return -1;
    }

  return cfd->fchmod (FILTERED_MODE (mode));
}

static void
stat64_to_stat32 (struct __stat64 *src, struct __stat32 *dst)
{
  dst->st_dev = ((src->st_dev >> 8) & 0xff00) | (src->st_dev & 0xff);
  dst->st_ino = ((unsigned) (src->st_ino >> 32)) | (unsigned) src->st_ino;
  dst->st_mode = src->st_mode;
  dst->st_nlink = src->st_nlink;
  dst->st_uid = src->st_uid;
  dst->st_gid = src->st_gid;
  dst->st_rdev = ((src->st_rdev >> 8) & 0xff00) | (src->st_rdev & 0xff);
  dst->st_size = src->st_size;
  dst->st_atim = src->st_atim;
  dst->st_mtim = src->st_mtim;
  dst->st_ctim = src->st_ctim;
  dst->st_blksize = src->st_blksize;
  dst->st_blocks = src->st_blocks;
}

extern "C" int
fstat64 (int fd, struct __stat64 *buf)
{
  int res;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    {
      memset (buf, 0, sizeof (struct __stat64));
      res = cfd->fstat (buf);
      if (!res)
	{
	  if (!buf->st_ino)
	    buf->st_ino = cfd->get_ino ();
	  if (!buf->st_dev)
	    buf->st_dev = cfd->get_device ();
	  if (!buf->st_rdev)
	    buf->st_rdev = buf->st_dev;
	}
    }

  syscall_printf ("%d = fstat (%d, %p)", res, fd, buf);
  return res;
}

extern "C" int
_fstat64_r (struct _reent *ptr, int fd, struct __stat64 *buf)
{
  int ret;

  if ((ret = fstat64 (fd, buf)) == -1)
    ptr->_errno = get_errno ();
  return ret;
}

extern "C" int
fstat (int fd, struct __stat32 *buf)
{
  struct __stat64 buf64;
  int ret = fstat64 (fd, &buf64);
  if (!ret)
    stat64_to_stat32 (&buf64, buf);
  return ret;
}

extern "C" int
_fstat_r (struct _reent *ptr, int fd, struct __stat32 *buf)
{
  int ret;

  if ((ret = fstat (fd, buf)) == -1)
    ptr->_errno = get_errno ();
  return ret;
}

/* fsync: P96 6.6.1.1 */
extern "C" int
fsync (int fd)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    {
      syscall_printf ("-1 = fsync (%d)", fd);
      return -1;
    }
  return cfd->fsync ();
}

EXPORT_ALIAS (fsync, fdatasync)

static void
sync_worker (const char *vol)
{
  HANDLE fh = CreateFileA (vol, GENERIC_WRITE, FILE_SHARE_VALID_FLAGS,
			   &sec_none_nih, OPEN_EXISTING, 0, NULL);
  if (fh != INVALID_HANDLE_VALUE)
    {
      FlushFileBuffers (fh);
      CloseHandle (fh);
    }
  else
    debug_printf ("Open failed with %E");
}

/* sync: SUSv3 */
extern "C" void
sync ()
{
  /* Per MSDN, 50 bytes should be enough here. */
  char vol[MAX_PATH];

  if (wincap.has_guid_volumes ()) /* Win2k and newer */
    {
      char a_drive[MAX_PATH] = {0};
      char b_drive[MAX_PATH] = {0};

      if (is_floppy ("A:"))
	GetVolumeNameForVolumeMountPointA ("A:\\", a_drive, MAX_PATH);
      if (is_floppy ("B:"))
	GetVolumeNameForVolumeMountPointA ("B:\\", b_drive, MAX_PATH);

      HANDLE sh = FindFirstVolumeA (vol, MAX_PATH);
      if (sh != INVALID_HANDLE_VALUE)
	{
	  do
	    {
	      debug_printf ("Try volume %s", vol);

	      /* Check vol for being a floppy on A: or B:.  Skip them. */
	      if (strcasematch (vol, a_drive) || strcasematch (vol, b_drive))
		{
		  debug_printf ("Is floppy, don't sync");
		  continue;
		}

	      /* Eliminate trailing backslash. */
	      vol[strlen (vol) - 1] = '\0';
	      sync_worker (vol);
	    }
	  while (FindNextVolumeA (sh, vol, MAX_PATH));
	  FindVolumeClose (sh);
	}
    }
  else
    {
      DWORD drives = GetLogicalDrives ();
      DWORD mask = 1;
      /* Skip floppies on A: and B: as in setmntent. */
      if ((drives & 1) && is_floppy ("A:"))
	drives &= ~1;
      if ((drives & 2) && is_floppy ("B:"))
	drives &= ~2;
      strcpy (vol, "\\\\.\\A:");
      do
	{
	  /* Geeh.  Try to sync only non-floppy drives. */
	  if (drives & mask)
	    {
	      debug_printf ("Try volume %s", vol);
	      sync_worker (vol);
	    }
	  vol[4]++;
	}
      while ((mask <<= 1) <= 1 << 25);
    }
}

/* Cygwin internal */
int __stdcall
stat_worker (path_conv &pc, struct __stat64 *buf)
{
  int res = -1;

  myfault efault;
  if (efault.faulted (EFAULT))
    goto error;

  if (pc.error)
    {
      debug_printf ("got %d error from path_conv", pc.error);
      set_errno (pc.error);
    }
  else if (pc.exists ())
    {
      fhandler_base *fh;

      if (!(fh = build_fh_pc (pc)))
	goto error;

      debug_printf ("(%S, %p, %p), file_attributes %d",
		    pc.get_nt_native_path (), buf, fh, (DWORD) *fh);
      memset (buf, 0, sizeof (*buf));
      res = fh->fstat (buf);
      if (!res)
	{
	  if (!buf->st_ino)
	    buf->st_ino = fh->get_ino ();
	  if (!buf->st_dev)
	    buf->st_dev = fh->get_device ();
	  if (!buf->st_rdev)
	    buf->st_rdev = buf->st_dev;
	}
      delete fh;
    }
  else
    set_errno (ENOENT);

 error:
  MALLOC_CHECK;
  syscall_printf ("%d = (%S, %p)", res, pc.get_nt_native_path (), buf);
  return res;
}

extern "C" int
stat64 (const char *name, struct __stat64 *buf)
{
  syscall_printf ("entering");
  path_conv pc (name, PC_SYM_FOLLOW | PC_POSIX, stat_suffixes);
  return stat_worker (pc, buf);
}

extern "C" int
_stat64_r (struct _reent *ptr, const char *name, struct __stat64 *buf)
{
  int ret;

  if ((ret = stat64 (name, buf)) == -1)
    ptr->_errno = get_errno ();
  return ret;
}

extern "C" int
stat (const char *name, struct __stat32 *buf)
{
  struct __stat64 buf64;
  int ret = stat64 (name, &buf64);
  if (!ret)
    stat64_to_stat32 (&buf64, buf);
  return ret;
}

extern "C" int
_stat_r (struct _reent *ptr, const char *name, struct __stat32 *buf)
{
  int ret;

  if ((ret = stat (name, buf)) == -1)
    ptr->_errno = get_errno ();
  return ret;
}

/* lstat: Provided by SVR4 and 4.3+BSD, POSIX? */
extern "C" int
lstat64 (const char *name, struct __stat64 *buf)
{
  syscall_printf ("entering");
  path_conv pc (name, PC_SYM_NOFOLLOW | PC_POSIX, stat_suffixes);
  return stat_worker (pc, buf);
}

/* lstat: Provided by SVR4 and 4.3+BSD, POSIX? */
extern "C" int
lstat (const char *name, struct __stat32 *buf)
{
  struct __stat64 buf64;
  int ret = lstat64 (name, &buf64);
  if (!ret)
    stat64_to_stat32 (&buf64, buf);
  return ret;
}

extern "C" int
access (const char *fn, int flags)
{
  // flags were incorrectly specified
  int res = -1;
  if (flags & ~(F_OK|R_OK|W_OK|X_OK))
    set_errno (EINVAL);
  else
    {
      fhandler_base *fh = build_fh_name (fn, PC_SYM_FOLLOW, stat_suffixes);
      if (fh)
	{
	  res =  fh->fhaccess (flags, false);
	  delete fh;
	}
    }
  debug_printf ("returning %d", res);
  return res;
}

/* Linux provides this extension; it is basically a wrapper around the
   POSIX:2008 faccessat (AT_FDCWD, fn, flags, AT_EACCESS).  We also
   provide eaccess as an alias for this, in cygwin.din.  */
extern "C" int
euidaccess (const char *fn, int flags)
{
  // flags were incorrectly specified
  int res = -1;
  if (flags & ~(F_OK|R_OK|W_OK|X_OK))
    set_errno (EINVAL);
  else
    {
      fhandler_base *fh = build_fh_name (fn, PC_SYM_FOLLOW, stat_suffixes);
      if (fh)
	{
	  res =  fh->fhaccess (flags, true);
	  delete fh;
	}
    }
  debug_printf ("returning %d", res);
  return res;
}

static void
rename_append_suffix (path_conv &pc, const char *path, size_t len,
		      const char *suffix)
{
  char buf[len + 5];

  if (ascii_strcasematch (path + len - 4, ".lnk")
      || ascii_strcasematch (path + len - 4, ".exe"))
    len -= 4;
  stpcpy (stpncpy (buf, path, len), suffix);
  pc.check (buf, PC_SYM_NOFOLLOW);
}

static void
start_transaction (HANDLE &old_trans, HANDLE &trans)
{
  NTSTATUS status = NtCreateTransaction (&trans,
				SYNCHRONIZE | TRANSACTION_ALL_ACCESS,
				NULL, NULL, NULL, 0, 0, 0, NULL, NULL);
  if (NT_SUCCESS (status))
    {
      old_trans = RtlGetCurrentTransaction ();
      RtlSetCurrentTransaction (trans);
    }
  else
    {
      debug_printf ("NtCreateTransaction failed, %p", status);
      old_trans = trans = NULL;
    }
}

static NTSTATUS
stop_transaction (NTSTATUS status, HANDLE old_trans, HANDLE trans)
{
  RtlSetCurrentTransaction (old_trans);
  if (NT_SUCCESS (status))
    status = NtCommitTransaction (trans, TRUE);
  else
    status = NtRollbackTransaction (trans, TRUE);
  NtClose (trans);
  return status;
}

/* This function tests if a filename has one of the "approved" executable
   suffix.  This list is probably not complete... */
static inline bool
nt_path_has_executable_suffix (PUNICODE_STRING upath)
{
  const PUNICODE_STRING blessed_executable_suffixes[] =
  {
    &ro_u_com,
    &ro_u_dll,	/* Messy, messy.  Per MSDN, the GetBinaryType function is
		   supposed to return with ERROR_BAD_EXE_FORMAT. if the file
		   is a DLL.  On 64-bit Windows, this works as expected for
		   32-bit and 64-bit DLLs.  On 32-bit Windows this only works
		   for 32-bit DLLs.  For 64-bit DLLs, 32-bit Windows returns
		   true with the type set to SCS_64BIT_BINARY. */
    &ro_u_exe,
    &ro_u_scr,
    &ro_u_sys,
    NULL
  };

  USHORT pos = upath->Length / sizeof (WCHAR);
  PWCHAR path;
  UNICODE_STRING usuf;
  const PUNICODE_STRING *suf;

  /* Too short for a native path? */
  if (pos < 8)
    return false;
  /* Assumption: All executable suffixes have a length of three. */
  path = upath->Buffer + pos - 4;
  if (*path != L'.')
    return false;
  RtlInitCountedUnicodeString (&usuf, path, 4 * sizeof (WCHAR));
  for (suf = blessed_executable_suffixes; *suf; ++suf)
    if (RtlEqualUnicodeString (&usuf, *suf, TRUE))
      return true;
  return false;
}

extern "C" int
rename (const char *oldpath, const char *newpath)
{
  tmp_pathbuf tp;
  int res = -1;
  path_conv oldpc, newpc, new2pc, *dstpc, *removepc = NULL;
  bool old_dir_requested = false, new_dir_requested = false;
  bool old_explicit_suffix = false, new_explicit_suffix = false;
  size_t olen, nlen;
  bool equal_path;
  NTSTATUS status = 0;
  HANDLE fh = NULL, nfh;
  HANDLE old_trans = NULL, trans = NULL;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG size;
  FILE_STANDARD_INFORMATION ofsi;
  PFILE_RENAME_INFORMATION pfri;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (!*oldpath || !*newpath)
    {
      /* Reject rename("","x"), rename("x","").  */
      set_errno (ENOENT);
      goto out;
    }
  if (has_dot_last_component (oldpath, true))
    {
      /* Reject rename("dir/.","x").  */
      oldpc.check (oldpath, PC_SYM_NOFOLLOW, stat_suffixes);
      set_errno (oldpc.isdir () ? EINVAL : ENOTDIR);
      goto out;
    }
  if (has_dot_last_component (newpath, true))
    {
      /* Reject rename("dir","x/.").  */
      newpc.check (newpath, PC_SYM_NOFOLLOW, stat_suffixes);
      set_errno (!newpc.exists () ? ENOENT : newpc.isdir () ? EINVAL : ENOTDIR);
      goto out;
    }

  /* A trailing slash requires that the pathname points to an existing
     directory.  If it's not, it's a ENOTDIR condition.  The same goes
     for newpath a bit further down this function. */
  olen = strlen (oldpath);
  if (isdirsep (oldpath[olen - 1]))
    {
      char *buf;
      char *p = stpcpy (buf = tp.c_get (), oldpath) - 1;
      oldpath = buf;
      while (p >= oldpath && isdirsep (*p))
        *p-- = '\0';
      olen = p + 1 - oldpath;
      if (!olen)
        {
          /* The root directory cannot be renamed.  This also rejects
             the corner case of rename("/","/"), even though it is the
             same file.  */
          set_errno (EINVAL);
          goto out;
        }
      old_dir_requested = true;
    }
  oldpc.check (oldpath, PC_SYM_NOFOLLOW, stat_suffixes);
  if (oldpc.error)
    {
      set_errno (oldpc.error);
      goto out;
    }
  if (!oldpc.exists ())
    {
      set_errno (ENOENT);
      goto out;
    }
  if (oldpc.isspecial () && !oldpc.issocket ()) /* No renames from virtual FS */
    {
      set_errno (EROFS);
      goto out;
    }
  if (oldpc.has_attribute (FILE_ATTRIBUTE_REPARSE_POINT) && !oldpc.issymlink ())
    {
      /* Volume mount point.  If we try to rename a volume mount point, NT
	 returns STATUS_NOT_SAME_DEVICE ==> Win32 ERROR_NOT_SAME_DEVICE ==>
	 errno EXDEV.  That's bad since mv(1) will now perform a cross-device
	 move.  So what we do here is to treat the volume mount point just
	 like Linux treats a mount point. */
      set_errno (EBUSY);
      goto out;
    }
  if (old_dir_requested && !oldpc.isdir ())
    {
      /* Reject rename("file/","x").  */
      set_errno (ENOTDIR);
      goto out;
    }
  if ((oldpc.known_suffix
       && (ascii_strcasematch (oldpath + olen - 4, ".lnk")
	   || ascii_strcasematch (oldpath + olen - 4, ".exe")))
      || nt_path_has_executable_suffix (oldpc.get_nt_native_path ()))
    old_explicit_suffix = true;

  nlen = strlen (newpath);
  if (isdirsep (newpath[nlen - 1]))
    {
      char *buf;
      char *p = stpcpy (buf = tp.c_get (), newpath) - 1;
      newpath = buf;
      while (p >= newpath && isdirsep (*p))
        *p-- = '\0';
      nlen = p + 1 - newpath;
      if (!nlen) /* The root directory is never empty.  */
        {
          set_errno (ENOTEMPTY);
          goto out;
        }
      new_dir_requested = true;
    }
  newpc.check (newpath, PC_SYM_NOFOLLOW, stat_suffixes);
  if (newpc.error)
    {
      set_errno (newpc.error);
      goto out;
    }
  if (newpc.isspecial () && !newpc.issocket ()) /* No renames to virtual FSes */
    {
      set_errno (EROFS);
      goto out;
    }
  if (new_dir_requested && !(newpc.exists ()
                             ? newpc.isdir () : oldpc.isdir ()))
    {
      /* Reject rename("file1","file2/"), but allow rename("dir","d/").  */
      set_errno (newpc.exists () ? ENOTDIR : ENOENT);
      goto out;
    }
  if (newpc.exists () && (oldpc.isdir () ? !newpc.isdir () : newpc.isdir ()))
    {
      /* Reject rename("file","dir") and rename("dir","file").  */
      set_errno (newpc.isdir () ? EISDIR : ENOTDIR);
      goto out;
    }
  if (newpc.known_suffix
      && (ascii_strcasematch (newpath + nlen - 4, ".lnk")
	  || ascii_strcasematch (newpath + nlen - 4, ".exe")))
    new_explicit_suffix = true;

  /* This test is necessary in almost every case, so just do it once here. */
  equal_path = RtlEqualUnicodeString (oldpc.get_nt_native_path (),
				      newpc.get_nt_native_path (),
				      oldpc.objcaseinsensitive ());

  /* First check if oldpath and newpath only differ by case.  If so, it's
     just a request to change the case of the filename.  By simply setting
     the file attributes to INVALID_FILE_ATTRIBUTES (which translates to
     "file doesn't exist"), all later tests are skipped. */
  if (oldpc.objcaseinsensitive () && newpc.exists () && equal_path
      && old_explicit_suffix == new_explicit_suffix)
    {
      if (RtlEqualUnicodeString (oldpc.get_nt_native_path (),
				 newpc.get_nt_native_path (),
				 FALSE))
	{
	  res = 0;
	  goto out;
	}
      newpc.file_attributes (INVALID_FILE_ATTRIBUTES);
    }
  else if (oldpc.isdir ())
    {
      /* Check for newpath being identical or a subdir of oldpath. */
      if (RtlPrefixUnicodeString (oldpc.get_nt_native_path (),
				  newpc.get_nt_native_path (),
				  TRUE))
	{
	  if (newpc.get_nt_native_path ()->Length
	      == oldpc.get_nt_native_path ()->Length)
	    {
	      res = 0;
	      goto out;
	    }
	  if (*(PWCHAR) ((PBYTE) newpc.get_nt_native_path ()->Buffer
			 + oldpc.get_nt_native_path ()->Length) == L'\\')
	    {
	      set_errno (EINVAL);
	      goto out;
	    }
	}
    }
  else if (!newpc.exists ())
    {
      if (equal_path && old_explicit_suffix != new_explicit_suffix)
	{
	  newpc.check (newpath, PC_SYM_NOFOLLOW);
	  if (RtlEqualUnicodeString (oldpc.get_nt_native_path (),
				     newpc.get_nt_native_path (),
				     oldpc.objcaseinsensitive ()))
	    {
	      res = 0;
	      goto out;
	    }
	}
      else if (oldpc.is_lnk_symlink ()
	       && !RtlEqualUnicodePathSuffix (newpc.get_nt_native_path (),
					      &ro_u_lnk, TRUE))
	rename_append_suffix (newpc, newpath, nlen, ".lnk");
      else if (oldpc.is_binary () && !old_explicit_suffix
	       && !nt_path_has_executable_suffix (newpc.get_nt_native_path ()))
	/* To rename an executable foo.exe to bar-without-suffix, the
	   .exe suffix must be given explicitly in oldpath. */
	rename_append_suffix (newpc, newpath, nlen, ".exe");
    }
  else
    {
      if (equal_path && old_explicit_suffix != new_explicit_suffix)
	{
	  newpc.check (newpath, PC_SYM_NOFOLLOW);
	  if (RtlEqualUnicodeString (oldpc.get_nt_native_path (),
				     newpc.get_nt_native_path (),
				     oldpc.objcaseinsensitive ()))
	    {
	      res = 0;
	      goto out;
	    }
	}
      else if (oldpc.is_lnk_symlink ())
	{
	  if (!newpc.is_lnk_symlink ()
	      && !RtlEqualUnicodePathSuffix (newpc.get_nt_native_path (),
					     &ro_u_lnk, TRUE))
	    {
	      rename_append_suffix (new2pc, newpath, nlen, ".lnk");
	      removepc = &newpc;
	    }
	}
      else if (oldpc.is_binary ())
	{
	  /* Never append .exe suffix if file has any suffix already. */
	  if (!nt_path_has_executable_suffix (newpc.get_nt_native_path ()))
	    {
	      rename_append_suffix (new2pc, newpath, nlen, ".exe");
	      removepc = &newpc;
	    }
	}
      else
	{
	  if ((RtlEqualUnicodePathSuffix (newpc.get_nt_native_path (),
					  &ro_u_lnk, TRUE)
	       || RtlEqualUnicodePathSuffix (newpc.get_nt_native_path (),
					     &ro_u_exe, TRUE))
	      && !new_explicit_suffix)
	    {
	      new2pc.check (newpath, PC_SYM_NOFOLLOW, stat_suffixes);
	      newpc.get_nt_native_path ()->Length -= 4 * sizeof (WCHAR);
	      if (new2pc.is_binary () || new2pc.is_lnk_symlink ())
		removepc = &new2pc;
	    }
	}
    }
  dstpc = (removepc == &newpc) ? &new2pc : &newpc;

  /* Opening the file must be part of the transaction.  It's not sufficient
     to call only NtSetInformationFile under the transaction.  Therefore we
     have to start the transaction here, if necessary. */
  if (wincap.has_transactions ()
      && (dstpc->fs_flags () & FILE_SUPPORTS_TRANSACTIONS)
      && (dstpc->isdir ()
	  || (!removepc && dstpc->has_attribute (FILE_ATTRIBUTE_READONLY))))
    start_transaction (old_trans, trans);

  /* DELETE is required to rename a file.  Samba (only some versions?) doesn't
     like the FILE_SHARE_DELETE mode if the file has the R/O attribute set
     and returns STATUS_ACCESS_DENIED in that case. */
  status = NtOpenFile (&fh, DELETE, oldpc.get_object_attr (attr, sec_none_nih),
		     &io,
		     oldpc.fs_is_samba () ? FILE_SHARE_READ | FILE_SHARE_WRITE
					  : FILE_SHARE_VALID_FLAGS,
		     FILE_OPEN_FOR_BACKUP_INTENT
		     | (oldpc.is_rep_symlink () ? FILE_OPEN_REPARSE_POINT : 0));
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      goto out;
    }

  /* Renaming a dir to another, existing dir fails always, even if
     ReplaceIfExists is set to TRUE and the existing dir is empty.  So
     we have to remove the destination dir first.  This also covers the
     case that the destination directory is not empty.  In that case,
     unlink_nt returns with STATUS_DIRECTORY_NOT_EMPTY. */
  if (dstpc->isdir ())
    {
      status = unlink_nt (*dstpc);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  goto out;
	}
    }
  /* You can't copy a file if the destination exists and has the R/O
     attribute set.  Remove the R/O attribute first.  But first check
     if a removepc exists.  If so, dstpc points to a non-existing file
     due to a mangled suffix. */
  else if (!removepc && dstpc->has_attribute (FILE_ATTRIBUTE_READONLY))
    {
      status = NtOpenFile (&nfh, FILE_WRITE_ATTRIBUTES,
			   dstpc->get_object_attr (attr, sec_none_nih),
			   &io, FILE_SHARE_VALID_FLAGS,
			   FILE_OPEN_FOR_BACKUP_INTENT
			   | (dstpc->is_rep_symlink ()
			      ? FILE_OPEN_REPARSE_POINT : 0));
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  goto out;
	}
      status = NtSetAttributesFile (nfh, dstpc->file_attributes ()
					 & ~FILE_ATTRIBUTE_READONLY);
      NtClose (nfh);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  goto out;
	}
    }

  /* SUSv3: If the old argument and the new argument resolve to the same
     existing file, rename() shall return successfully and perform no
     other action.
     The test tries to be as quick as possible.  First it tests for identical
     volume serial numbers because that information is available anyway.
     Then it tests if oldpath has more than 1 hardlink, then it opens newpath
     and tests for identical file ids.  If so, oldpath and newpath refer to
     the same file. */
  if ((removepc || dstpc->exists ())
      && !oldpc.isdir ()
      && dstpc->fs_serial_number () == oldpc.fs_serial_number ()
      && NT_SUCCESS (NtQueryInformationFile (fh, &io, &ofsi, sizeof ofsi,
					     FileStandardInformation))
      && ofsi.NumberOfLinks > 1
      && NT_SUCCESS (NtOpenFile (&nfh, READ_CONTROL,
		     (removepc ?: dstpc)->get_object_attr (attr, sec_none_nih),
		     &io, FILE_SHARE_VALID_FLAGS,
		     FILE_OPEN_FOR_BACKUP_INTENT
		     | ((removepc ?: dstpc)->is_rep_symlink ()
			? FILE_OPEN_REPARSE_POINT : 0))))
    {
      FILE_INTERNAL_INFORMATION ofii, nfii;

      if (NT_SUCCESS (NtQueryInformationFile (fh, &io, &ofii, sizeof ofii,
					      FileInternalInformation))
	  && NT_SUCCESS (NtQueryInformationFile (nfh, &io, &nfii, sizeof nfii,
						 FileInternalInformation))
	  && ofii.FileId.QuadPart == nfii.FileId.QuadPart)
	{
	  debug_printf ("%s and %s are the same file", oldpath, newpath);
	  NtClose (nfh);
	  res = 0;
	  goto out;
	}
      NtClose (nfh);
    }
  size = sizeof (FILE_RENAME_INFORMATION)
	 + dstpc->get_nt_native_path ()->Length;
  if (size > NT_MAX_PATH * sizeof (WCHAR)) /* Hopefully very seldom. */
    pfri = (PFILE_RENAME_INFORMATION) alloca (size);
  else
    pfri = (PFILE_RENAME_INFORMATION) tp.w_get ();
  pfri->ReplaceIfExists = TRUE;
  pfri->RootDirectory = NULL;
  pfri->FileNameLength = dstpc->get_nt_native_path ()->Length;
  memcpy (&pfri->FileName,  dstpc->get_nt_native_path ()->Buffer,
	  pfri->FileNameLength);
  status = NtSetInformationFile (fh, &io, pfri, size, FileRenameInformation);
  /* This happens if the access rights don't allow deleting the destination.
     Even if the handle to the original file is opened with BACKUP
     and/or RECOVERY, these flags don't apply to the destination of the
     rename operation.  So, a privileged user can't rename a file to an
     existing file, if the permissions of the existing file aren't right.
     Like directories, we have to handle this separately by removing the
     destination before renaming. */
  if (status == STATUS_ACCESS_DENIED && dstpc->exists () && !dstpc->isdir ())
    {
      if (wincap.has_transactions ()
	  && (dstpc->fs_flags () & FILE_SUPPORTS_TRANSACTIONS)
	  && !trans)
	{
	  start_transaction (old_trans, trans);
	  /* As mentioned earlier, opening the file must be part of the
	     transaction.  Therefore we have to reopen the file here if the
	     transaction hasn't been started already.  Unfortunately we can't
	     use the NT "reopen file from existing handle" feature.  In that
	     case NtOpenFile returns STATUS_TRANSACTIONAL_CONFLICT.  We *have*
	     to close the handle to the file first, *then* we can re-open it.
	     Fortunately nothing has happened yet, so the atomicity of the
	     rename functionality is not spoiled. */
	  NtClose (fh);
	  status = NtOpenFile (&fh, DELETE,
			       oldpc.get_object_attr (attr, sec_none_nih),
			       &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT
			       | (oldpc.is_rep_symlink ()
				  ? FILE_OPEN_REPARSE_POINT : 0));
	  if (!NT_SUCCESS (status))
	    {
	      __seterrno_from_nt_status (status);
	      goto out;
	    }
	}
      if (NT_SUCCESS (status = unlink_nt (*dstpc)))
	status = NtSetInformationFile (fh, &io, pfri, size,
				       FileRenameInformation);
    }
  if (NT_SUCCESS (status))
    {
      if (removepc)
	unlink_nt (*removepc);
      res = 0;
    }
  else
    __seterrno_from_nt_status (status);

out:
  if (fh)
    NtClose (fh);
  if (wincap.has_transactions () && trans)
    stop_transaction (status, old_trans, trans);
  syscall_printf ("%d = rename (%s, %s)", res, oldpath, newpath);
  return res;
}

extern "C" int
system (const char *cmdstring)
{
  pthread_testcancel ();

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  int res;
  const char* command[4];

  if (cmdstring == NULL)
    return 1;

  command[0] = "sh";
  command[1] = "-c";
  command[2] = cmdstring;
  command[3] = (const char *) NULL;

  if ((res = spawnvp (_P_SYSTEM, "/bin/sh", command)) == -1)
    {
      // when exec fails, return value should be as if shell
      // executed exit (127)
      res = 127;
    }

  return res;
}

extern "C" int
setdtablesize (int size)
{
  if (size <= (int)cygheap->fdtab.size || cygheap->fdtab.extend (size - cygheap->fdtab.size))
    return 0;

  return -1;
}

extern "C" int
getdtablesize ()
{
  return cygheap->fdtab.size > OPEN_MAX ? cygheap->fdtab.size : OPEN_MAX;
}

extern "C" int
getpagesize ()
{
  if (!system_info.dwAllocationGranularity)
    GetSystemInfo (&system_info);
  return (size_t) system_info.dwAllocationGranularity;
}

size_t
getsystempagesize ()
{
  if (!system_info.dwPageSize)
    GetSystemInfo (&system_info);
  return (size_t) system_info.dwPageSize;
}

/* FIXME: not all values are correct... */
extern "C" long int
fpathconf (int fd, int v)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return -1;
  return cfd->fpathconf (v);
}

extern "C" long int
pathconf (const char *file, int v)
{
  fhandler_base *fh;
  long ret = -1;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (!*file)
    {
      set_errno (ENOENT);
      return -1;
    }
  if (!(fh = build_fh_name (file, PC_SYM_FOLLOW, stat_suffixes)))
    return -1;
  if (!fh->exists ())
    set_errno (ENOENT);
  else
    ret = fh->fpathconf (v);
  delete fh;
  return ret;
}

extern "C" int
ttyname_r (int fd, char *buf, size_t buflen)
{
  int ret = 0;
  myfault efault;
  if (efault.faulted ())
    ret = EFAULT;
  else
    {
      cygheap_fdget cfd (fd, true);
      if (cfd < 0)
	ret = EBADF;
      else if (!cfd->is_tty ())
	ret = ENOTTY;
      else if (buflen < strlen (cfd->ttyname ()) + 1)
	ret = ERANGE;
      else
	strcpy (buf, cfd->ttyname ());
    }
  debug_printf ("returning %d tty: %s", ret, ret ? "NULL" : buf);
  return ret;
}

extern "C" char *
ttyname (int fd)
{
  static char name[TTY_NAME_MAX];
  int ret = ttyname_r (fd, name, TTY_NAME_MAX);
  if (ret)
    {
      set_errno (ret);
      return NULL;
    }
  return name;
}

extern "C" char *
ctermid (char *str)
{
  static NO_COPY char buf[16];
  if (str == NULL)
    str = buf;
  if (!real_tty_attached (myself))
    strcpy (str, "/dev/conin");
  else
    __small_sprintf (str, "/dev/tty%d", myself->ctty);
  return str;
}

/* Tells stdio if it should do the cr/lf conversion for this file */
extern "C" int
_cygwin_istext_for_stdio (int fd)
{
  if (CYGWIN_VERSION_OLD_STDIO_CRLF_HANDLING)
    {
      syscall_printf ("fd %d: old API", fd);
      return 0; /* we do it for old apps, due to getc/putc macros */
    }

  cygheap_fdget cfd (fd, false, false);
  if (cfd < 0)
    {
      syscall_printf ("fd %d: not open", fd);
      return 0;
    }

#if 0
  if (cfd->get_device () != FH_FS)
    {
      syscall_printf ("fd not disk file.  Defaulting to binary.");
      return 0;
    }
#endif

  if (cfd->wbinary () || cfd->rbinary ())
    {
      syscall_printf ("fd %d: opened as binary", fd);
      return 0;
    }

  syscall_printf ("fd %d: defaulting to text", fd);
  return 1;
}

/* internal newlib function */
extern "C" int _fwalk (struct _reent *ptr, int (*function) (FILE *));

static int
setmode_helper (FILE *f)
{
  if (fileno (f) != _my_tls.locals.setmode_file)
    {
      syscall_printf ("improbable, but %d != %d", fileno (f), _my_tls.locals.setmode_file);
      return 0;
    }
  syscall_printf ("file was %s now %s", f->_flags & __SCLE ? "text" : "binary",
		  _my_tls.locals.setmode_mode & O_TEXT ? "text" : "binary");
  if (_my_tls.locals.setmode_mode & O_TEXT)
    f->_flags |= __SCLE;
  else
    f->_flags &= ~__SCLE;
  return 0;
}

extern "C" int
getmode (int fd)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return -1;

  return cfd->get_flags () & (O_BINARY | O_TEXT);
}

/* Set a file descriptor into text or binary mode, returning the
   previous mode.  */

extern "C" int
setmode (int fd, int mode)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return -1;
  if (mode != O_BINARY  && mode != O_TEXT && mode != 0)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* Note that we have no way to indicate the case that writes are
     binary but not reads, or vice-versa.  These cases can arise when
     using the tty or console interface.  People using those
     interfaces should not use setmode.  */

  int res;
  if (cfd->wbinary () && cfd->rbinary ())
    res = O_BINARY;
  else if (cfd->wbinset () && cfd->rbinset ())
    res = O_TEXT;	/* Specifically set O_TEXT */
  else
    res = 0;

  if (!mode)
    cfd->reset_to_open_binmode ();
  else
    cfd->set_flags ((cfd->get_flags () & ~(O_TEXT | O_BINARY)) | mode);

  syscall_printf ("(%d<%S>, %p) returning %s", fd,
		  cfd->pc.get_nt_native_path (), mode,
		  res & O_TEXT ? "text" : "binary");
  return res;
}

extern "C" int
cygwin_setmode (int fd, int mode)
{
  int res = setmode (fd, mode);
  if (res != -1)
    {
      _my_tls.locals.setmode_file = fd;
      if (_cygwin_istext_for_stdio (fd))
	_my_tls.locals.setmode_mode = O_TEXT;
      else
	_my_tls.locals.setmode_mode = O_BINARY;
      _fwalk (_GLOBAL_REENT, setmode_helper);
    }
  return res;
}

extern "C" int
posix_fadvise (int fd, _off64_t offset, _off64_t len, int advice)
{
  int res = -1;
  cygheap_fdget cfd (fd);
  if (cfd >= 0)
    res = cfd->fadvise (offset, len, advice);
  else
    set_errno (EBADF);
  syscall_printf ("%d = posix_fadvice (%d, %D, %D, %d)",
		  res, fd, offset, len, advice);
  return res;
}

extern "C" int
posix_fallocate (int fd, _off64_t offset, _off64_t len)
{
  int res = -1;
  if (offset < 0 || len == 0)
    set_errno (EINVAL);
  else
    {
      cygheap_fdget cfd (fd);
      if (cfd >= 0)
	res = cfd->ftruncate (offset + len, false);
      else
	set_errno (EBADF);
    }
  syscall_printf ("%d = posix_fallocate (%d, %D, %D)", res, fd, offset, len);
  return res;
}

extern "C" int
ftruncate64 (int fd, _off64_t length)
{
  int res = -1;
  cygheap_fdget cfd (fd);
  if (cfd >= 0)
    res = cfd->ftruncate (length, true);
  else
    set_errno (EBADF);
  syscall_printf ("%d = ftruncate (%d, %D)", res, fd, length);
  return res;
}

/* ftruncate: P96 5.6.7.1 */
extern "C" int
ftruncate (int fd, _off_t length)
{
  return ftruncate64 (fd, (_off64_t)length);
}

/* truncate: Provided by SVR4 and 4.3+BSD.  Not part of POSIX.1 or XPG3 */
extern "C" int
truncate64 (const char *pathname, _off64_t length)
{
  int fd;
  int res = -1;

  fd = open (pathname, O_RDWR);

  if (fd != -1)
    {
      res = ftruncate64 (fd, length);
      close (fd);
    }
  syscall_printf ("%d = truncate (%s, %D)", res, pathname, length);

  return res;
}

/* truncate: Provided by SVR4 and 4.3+BSD.  Not part of POSIX.1 or XPG3 */
extern "C" int
truncate (const char *pathname, _off_t length)
{
  return truncate64 (pathname, (_off64_t)length);
}

extern "C" long
get_osfhandle (int fd)
{
  long res;

  cygheap_fdget cfd (fd);
  if (cfd >= 0)
    res = (long) cfd->get_handle ();
  else
    res = -1;

  syscall_printf ("%d = get_osfhandle (%d)", res, fd);
  return res;
}

extern "C" int
fstatvfs (int fd, struct statvfs *sfs)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return -1;
  return cfd->fstatvfs (sfs);
}

extern "C" int
statvfs (const char *name, struct statvfs *sfs)
{
  int res = -1;
  fhandler_base *fh = NULL;

  myfault efault;
  if (efault.faulted (EFAULT))
    goto error;

  if (!(fh = build_fh_name (name, PC_SYM_FOLLOW, stat_suffixes)))
    goto error;

  if (fh->error ())
    {
      debug_printf ("got %d error from build_fh_name", fh->error ());
      set_errno (fh->error ());
    }
  else if (fh->exists ())
    {
      debug_printf ("(%s, %p), file_attributes %d", name, sfs, (DWORD) *fh);
      res = fh->fstatvfs (sfs);
    }
  else
    set_errno (ENOENT);

  delete fh;
 error:
  MALLOC_CHECK;
  syscall_printf ("%d = (%s, %p)", res, name, sfs);
  return res;
}

extern "C" int
fstatfs (int fd, struct statfs *sfs)
{
  struct statvfs vfs;
  int ret = fstatvfs (fd, &vfs);
  if (!ret)
    {
      sfs->f_type = vfs.f_flag;
      sfs->f_bsize = vfs.f_bsize;
      sfs->f_blocks = vfs.f_blocks;
      sfs->f_bavail = vfs.f_bavail;
      sfs->f_bfree = vfs.f_bfree;
      sfs->f_files = -1;
      sfs->f_ffree = -1;
      sfs->f_fsid = vfs.f_fsid;
      sfs->f_namelen = vfs.f_namemax;
    }
  return ret;
}

extern "C" int
statfs (const char *fname, struct statfs *sfs)
{
  struct statvfs vfs;
  int ret = statvfs (fname, &vfs);
  if (!ret)
    {
      sfs->f_type = vfs.f_flag;
      sfs->f_bsize = vfs.f_bsize;
      sfs->f_blocks = vfs.f_blocks;
      sfs->f_bavail = vfs.f_bavail;
      sfs->f_bfree = vfs.f_bfree;
      sfs->f_files = -1;
      sfs->f_ffree = -1;
      sfs->f_fsid = vfs.f_fsid;
      sfs->f_namelen = vfs.f_namemax;
    }
  return ret;
}

/* setpgid: POSIX 4.3.3.1 */
extern "C" int
setpgid (pid_t pid, pid_t pgid)
{
  int res = -1;
  if (pid == 0)
    pid = getpid ();
  if (pgid == 0)
    pgid = pid;

  if (pgid < 0)
    set_errno (EINVAL);
  else
    {
      pinfo p (pid, PID_MAP_RW);
      if (!p)
	set_errno (ESRCH);
      else if (p->pgid == pgid)
	res = 0;
      /* A process may only change the process group of itself and its children */
      else if (p != myself && p->ppid != myself->pid)
	set_errno (EPERM);
      else
	{
	  p->pgid = pgid;
	  if (p->pid != p->pgid)
	    p->set_has_pgid_children (0);
	  res = 0;
	}
    }

  syscall_printf ("pid %d, pgid %d, res %d", pid, pgid, res);
  return res;
}

extern "C" pid_t
getpgid (pid_t pid)
{
  if (pid == 0)
    pid = getpid ();

  pinfo p (pid);
  if (p == 0)
    {
      set_errno (ESRCH);
      return -1;
    }
  return p->pgid;
}

extern "C" int
setpgrp (void)
{
  return setpgid (0, 0);
}

extern "C" pid_t
getpgrp (void)
{
  return getpgid (0);
}

extern "C" char *
ptsname (int fd)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return 0;
  return (char *) (cfd->ptsname ());
}

static int __stdcall
mknod_worker (const char *path, mode_t type, mode_t mode, _major_t major,
	      _minor_t minor)
{
  char buf[sizeof (":\\00000000:00000000:00000000") + PATH_MAX];
  sprintf (buf, ":\\%x:%x:%x", major, minor,
	   type | (mode & (S_IRWXU | S_IRWXG | S_IRWXO)));
  return symlink_worker (buf, path, true, true);
}

extern "C" int
mknod32 (const char *path, mode_t mode, __dev32_t dev)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (!*path)
    {
      set_errno (ENOENT);
      return -1;
    }

  if (strlen (path) >= PATH_MAX)
    return -1;

  path_conv w32path (path, PC_SYM_NOFOLLOW);
  if (w32path.exists ())
    {
      set_errno (EEXIST);
      return -1;
    }

  mode_t type = mode & S_IFMT;
  _major_t major = _major (dev);
  _minor_t minor = _minor (dev);
  switch (type)
    {
    case S_IFCHR:
    case S_IFBLK:
      break;

    case S_IFIFO:
      major = _major (FH_FIFO);
      minor = _minor (FH_FIFO);
      break;

    case 0:
    case S_IFREG:
      {
	int fd = open (path, O_CREAT, mode);
	if (fd < 0)
	  return -1;
	close (fd);
	return 0;
      }

    default:
      set_errno (EINVAL);
      return -1;
    }

  return mknod_worker (w32path.get_win32 (), type, mode, major, minor);
}

extern "C" int
mknod (const char *_path, mode_t mode, __dev16_t dev)
{
  return mknod32 (_path, mode, (__dev32_t) dev);
}

extern "C" int
mkfifo (const char *path, mode_t mode)
{
  return mknod32 (path, (mode & ~S_IFMT) | S_IFIFO, 0);
}

/* seteuid: standards? */
extern "C" int
seteuid32 (__uid32_t uid)
{
  debug_printf ("uid: %u myself->uid: %u myself->gid: %u",
		uid, myself->uid, myself->gid);

  /* Same uid as we're just running under is usually a no-op.

     Except we have an external token which is a restricted token.  Or,
     the external token is NULL, but the current impersonation token is
     a restricted token.  This allows to restrict user rights temporarily
     like this:

       cygwin_internal(CW_SET_EXTERNAL_TOKEN, restricted_token,
                       CW_TOKEN_RESTRICTED);
       setuid (getuid ());
       [...do stuff with restricted rights...]
       cygwin_internal(CW_SET_EXTERNAL_TOKEN, INVALID_HANDLE_VALUE,
                       CW_TOKEN_RESTRICTED);
       setuid (getuid ());

    Note that using the current uid is a requirement!  Starting with Windows
    Vista, we have restricted tokens galore (UAC), so this is really just
    a special case to restict your own processes to lesser rights. */
  bool request_restricted_uid_switch = (uid == myself->uid
      && cygheap->user.ext_token_is_restricted);
  if (uid == myself->uid && !cygheap->user.groups.ischanged
      && !request_restricted_uid_switch)
    {
      debug_printf ("Nothing happens");
      return 0;
    }

  cygsid usersid;
  user_groups &groups = cygheap->user.groups;
  HANDLE new_token = INVALID_HANDLE_VALUE;
  struct passwd * pw_new;
  bool token_is_internal, issamesid = false;

  pw_new = internal_getpwuid (uid);
  if (!usersid.getfrompw (pw_new))
    {
      set_errno (EINVAL);
      return -1;
    }

  cygheap->user.deimpersonate ();

  /* Verify if the process token is suitable. */
  /* First of all, skip all checks if a switch to a restricted token has been
     requested, or if trying to switch back from it. */
  if (request_restricted_uid_switch)
    {
      if (cygheap->user.external_token != NO_IMPERSONATION)
	{
	  debug_printf ("Switch to restricted token");
	  new_token = cygheap->user.external_token;
	}
      else
	{
	  debug_printf ("Switch back from restricted token");
	  new_token = hProcToken;
	  cygheap->user.ext_token_is_restricted = false;
	}
    }
  /* TODO, CV 2008-11-25: The check against saved_sid is a kludge and a
     shortcut.  We must check if it's really feasible in the long run.
     The reason to add this shortcut is this:  sshd switches back to the
     privileged user running sshd at least twice in the process of
     authentication.  It calls seteuid first, then setegid.  Due to this
     order, the setgroups group list is still active when calling seteuid
     and verify_token treats the original token of the privileged user as
     insufficient.  This in turn results in creating a new user token for
     the privileged user instead of using the orignal token.  This can have
     unfortunate side effects.  The created token has different group
     memberships, different user rights, and misses possible network
     credentials.
     Therefore we try this shortcut now.  When switching back to the
     privileged user, we probably always want a correct (aka original)
     user token for this privileged user, not only in sshd. */
  else if ((uid == cygheap->user.saved_uid
	   && usersid == cygheap->user.saved_sid ())
	   || verify_token (hProcToken, usersid, groups))
    new_token = hProcToken;
  /* Verify if the external token is suitable */
  else if (cygheap->user.external_token != NO_IMPERSONATION
	   && verify_token (cygheap->user.external_token, usersid, groups))
    new_token = cygheap->user.external_token;
  /* Verify if the current token (internal or former external) is suitable */
  else if (cygheap->user.curr_primary_token != NO_IMPERSONATION
	   && cygheap->user.curr_primary_token != cygheap->user.external_token
	   && verify_token (cygheap->user.curr_primary_token, usersid, groups,
			    &token_is_internal))
    new_token = cygheap->user.curr_primary_token;
  /* Verify if the internal token is suitable */
  else if (cygheap->user.internal_token != NO_IMPERSONATION
	   && cygheap->user.internal_token != cygheap->user.curr_primary_token
	   && verify_token (cygheap->user.internal_token, usersid, groups,
			    &token_is_internal))
    new_token = cygheap->user.internal_token;

  debug_printf ("Found token %d", new_token);

  /* If no impersonation token is available, try to authenticate using
     LSA private data stored password, LSA authentication using our own
     LSA module, or, as last chance, NtCreateToken. */
  if (new_token == INVALID_HANDLE_VALUE)
    {
      new_token = lsaprivkeyauth (pw_new);
      if (new_token)
	{
	  /* We have to verify this token since settings in /etc/group
	     might render it unusable im terms of group membership. */
	  if (!verify_token (new_token, usersid, groups))
	    {
	      CloseHandle (new_token);
	      new_token = NULL;
	    }
	}
      if (!new_token)
	{
	  debug_printf ("lsaprivkeyauth failed, try lsaauth.");
	  if (!(new_token = lsaauth (usersid, groups, pw_new)))
	    {
	      debug_printf ("lsaauth failed, try create_token.");
	      new_token = create_token (usersid, groups, pw_new);
	      if (new_token == INVALID_HANDLE_VALUE)
		{
		  debug_printf ("create_token failed, bail out of here");
		  cygheap->user.reimpersonate ();
		  return -1;
		}
	    }
	}

      /* Keep at most one internal token */
      if (cygheap->user.internal_token != NO_IMPERSONATION)
	CloseHandle (cygheap->user.internal_token);
      cygheap->user.internal_token = new_token;
    }

  if (new_token != hProcToken)
    {
      if (!request_restricted_uid_switch)
	{
	  /* Avoid having HKCU use default user */
	  WCHAR name[128];
	  load_registry_hive (usersid.string (name));
	}

      /* Try setting owner to same value as user. */
      if (!SetTokenInformation (new_token, TokenOwner,
				&usersid, sizeof usersid))
	debug_printf ("SetTokenInformation(user.token, TokenOwner), %E");
      /* Try setting primary group in token to current group */
      if (!SetTokenInformation (new_token, TokenPrimaryGroup,
				&groups.pgsid, sizeof (cygsid)))
	debug_printf ("SetTokenInformation(user.token, TokenPrimaryGroup), %E");
      /* Try setting default DACL */
      PACL dacl_buf = (PACL) alloca (MAX_DACL_LEN (5));
      if (sec_acl (dacl_buf, true, true, usersid))
	{
	  TOKEN_DEFAULT_DACL tdacl = { dacl_buf };
	  if (!SetTokenInformation (new_token, TokenDefaultDacl,
				    &tdacl, sizeof (tdacl)))
	    debug_printf ("SetTokenInformation (TokenDefaultDacl), %E");
	}
    }

  issamesid = (usersid == cygheap->user.sid ());
  cygheap->user.set_sid (usersid);
  cygheap->user.curr_primary_token = new_token == hProcToken ? NO_IMPERSONATION
							: new_token;
  cygheap->user.curr_token_is_restricted = false;
  cygheap->user.setuid_to_restricted = false;
  if (cygheap->user.curr_imp_token != NO_IMPERSONATION)
    {
      CloseHandle (cygheap->user.curr_imp_token);
      cygheap->user.curr_imp_token = NO_IMPERSONATION;
    }
  if (cygheap->user.curr_primary_token != NO_IMPERSONATION)
    {
      /* HANDLE_FLAG_INHERIT may be missing in external token.  */
      if (!SetHandleInformation (cygheap->user.curr_primary_token,
				 HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)
	  || !DuplicateTokenEx (cygheap->user.curr_primary_token,
				MAXIMUM_ALLOWED, &sec_none,
				SecurityImpersonation, TokenImpersonation,
				&cygheap->user.curr_imp_token))
	{
	  __seterrno ();
	  cygheap->user.curr_primary_token = NO_IMPERSONATION;
	  return -1;
	}
      cygheap->user.curr_token_is_restricted = request_restricted_uid_switch;
      set_cygwin_privileges (cygheap->user.curr_primary_token);
      set_cygwin_privileges (cygheap->user.curr_imp_token);
    }
  if (!cygheap->user.reimpersonate ())
    {
      __seterrno ();
      return -1;
    }

  cygheap->user.set_name (pw_new->pw_name);
  myself->uid = uid;
  groups.ischanged = FALSE;
  if (!issamesid)
    /* Recreate and fill out the user shared region for a new user. */
    user_shared_create (true);
  return 0;
}

extern "C" int
seteuid (__uid16_t uid)
{
  return seteuid32 (uid16touid32 (uid));
}

/* setuid: POSIX 4.2.2.1 */
extern "C" int
setuid32 (__uid32_t uid)
{
  int ret = seteuid32 (uid);
  if (!ret)
    {
      cygheap->user.real_uid = myself->uid;
      /* If restricted token, forget original privileges on exec ().  */
      cygheap->user.setuid_to_restricted = cygheap->user.curr_token_is_restricted;
    }
  debug_printf ("real: %d, effective: %d", cygheap->user.real_uid, myself->uid);
  return ret;
}

extern "C" int
setuid (__uid16_t uid)
{
  return setuid32 (uid16touid32 (uid));
}

extern "C" int
setreuid32 (__uid32_t ruid, __uid32_t euid)
{
  int ret = 0;
  bool tried = false;
  __uid32_t old_euid = myself->uid;

  if (ruid != ILLEGAL_UID && cygheap->user.real_uid != ruid && euid != ruid)
    tried = !(ret = seteuid32 (ruid));
  if (!ret && euid != ILLEGAL_UID)
    ret = seteuid32 (euid);
  if (tried && (ret || euid == ILLEGAL_UID) && seteuid32 (old_euid))
    system_printf ("Cannot restore original euid %u", old_euid);
  if (!ret && ruid != ILLEGAL_UID)
    cygheap->user.real_uid = ruid;
  debug_printf ("real: %u, effective: %u", cygheap->user.real_uid, myself->uid);
  return ret;
}

extern "C" int
setreuid (__uid16_t ruid, __uid16_t euid)
{
  return setreuid32 (uid16touid32 (ruid), uid16touid32 (euid));
}

/* setegid: from System V.  */
extern "C" int
setegid32 (__gid32_t gid)
{
  debug_printf ("new egid: %u current: %u", gid, myself->gid);

  if (gid == myself->gid)
    {
      myself->gid = gid;
      return 0;
    }

  user_groups * groups = &cygheap->user.groups;
  cygsid gsid;
  struct __group32 * gr = internal_getgrgid (gid);

  if (!gsid.getfromgr (gr))
    {
      set_errno (EINVAL);
      return -1;
    }
  myself->gid = gid;

  groups->update_pgrp (gsid);
  if (cygheap->user.issetuid ())
    {
      /* If impersonated, update impersonation token... */
      if (!SetTokenInformation (cygheap->user.primary_token (),
				TokenPrimaryGroup, &gsid, sizeof gsid))
	debug_printf ("SetTokenInformation(primary_token, "
		      "TokenPrimaryGroup), %E");
      if (!SetTokenInformation (cygheap->user.imp_token (), TokenPrimaryGroup,
				&gsid, sizeof gsid))
	debug_printf ("SetTokenInformation(token, TokenPrimaryGroup), %E");
    }
  cygheap->user.deimpersonate ();
  if (!SetTokenInformation (hProcToken, TokenPrimaryGroup, &gsid, sizeof gsid))
    debug_printf ("SetTokenInformation(hProcToken, TokenPrimaryGroup), %E");
  clear_procimptoken ();
  cygheap->user.reimpersonate ();
  return 0;
}

extern "C" int
setegid (__gid16_t gid)
{
  return setegid32 (gid16togid32 (gid));
}

/* setgid: POSIX 4.2.2.1 */
extern "C" int
setgid32 (__gid32_t gid)
{
  int ret = setegid32 (gid);
  if (!ret)
    cygheap->user.real_gid = myself->gid;
  return ret;
}

extern "C" int
setgid (__gid16_t gid)
{
  int ret = setegid32 (gid16togid32 (gid));
  if (!ret)
    cygheap->user.real_gid = myself->gid;
  return ret;
}

extern "C" int
setregid32 (__gid32_t rgid, __gid32_t egid)
{
  int ret = 0;
  bool tried = false;
  __gid32_t old_egid = myself->gid;

  if (rgid != ILLEGAL_GID && cygheap->user.real_gid != rgid && egid != rgid)
    tried = !(ret = setegid32 (rgid));
  if (!ret && egid != ILLEGAL_GID)
    ret = setegid32 (egid);
  if (tried && (ret || egid == ILLEGAL_GID) && setegid32 (old_egid))
    system_printf ("Cannot restore original egid %u", old_egid);
  if (!ret && rgid != ILLEGAL_GID)
    cygheap->user.real_gid = rgid;
  debug_printf ("real: %u, effective: %u", cygheap->user.real_gid, myself->gid);
  return ret;
}

extern "C" int
setregid (__gid16_t rgid, __gid16_t egid)
{
  return setregid32 (gid16togid32 (rgid), gid16togid32 (egid));
}

/* chroot: privileged Unix system call.  */
/* FIXME: Not privileged here. How should this be done? */
extern "C" int
chroot (const char *newroot)
{
  path_conv path (newroot, PC_SYM_FOLLOW | PC_POSIX);

  int ret = -1;
  if (path.error)
    set_errno (path.error);
  else if (!path.exists ())
    set_errno (ENOENT);
  else if (!path.isdir ())
    set_errno (ENOTDIR);
  else if (path.isspecial ())
    set_errno (EPERM);
  else
    {
      getwinenv("PATH="); /* Save the native PATH */
      cygheap->root.set (path.normalized_path, path.get_win32 (),
			 !!path.objcaseinsensitive ());
      ret = 0;
    }

  syscall_printf ("%d = chroot (%s)", ret ? get_errno () : 0,
				      newroot ? newroot : "NULL");
  return ret;
}

extern "C" int
creat (const char *path, mode_t mode)
{
  return open (path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

extern "C" void
__assertfail ()
{
  exit (99);
}

extern "C" int
vhangup ()
{
  set_errno (ENOSYS);
  return -1;
}

extern "C" int
setpriority (int which, id_t who, int value)
{
  DWORD prio = nice_to_winprio (value);
  int error = 0;

  switch (which)
    {
    case PRIO_PROCESS:
      if (!who)
	who = myself->pid;
      if ((pid_t) who == myself->pid)
	{
	  if (!SetPriorityClass (GetCurrentProcess (), prio))
	    {
	      set_errno (EACCES);
	      return -1;
	    }
	  myself->nice = value;
	  debug_printf ("Set nice to %d", myself->nice);
	  return 0;
	}
      break;
    case PRIO_PGRP:
      if (!who)
	who = myself->pgid;
      break;
    case PRIO_USER:
      if (!who)
	who = myself->uid;
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }
  winpids pids ((DWORD) PID_MAP_RW);
  for (DWORD i = 0; i < pids.npids; ++i)
    {
      _pinfo *p = pids[i];
      if (p)
	{
	  switch (which)
	    {
	    case PRIO_PROCESS:
	      if ((pid_t) who != p->pid)
		continue;
	      break;
	    case PRIO_PGRP:
	      if ((pid_t) who != p->pgid)
		continue;
	      break;
	    case PRIO_USER:
		if ((__uid32_t) who != p->uid)
		continue;
	      break;
	    }
	  HANDLE proc_h = OpenProcess (PROCESS_SET_INFORMATION, FALSE,
				       p->dwProcessId);
	  if (!proc_h)
	    error = EPERM;
	  else
	    {
	      if (!SetPriorityClass (proc_h, prio))
		error = EACCES;
	      else
		p->nice = value;
	      CloseHandle (proc_h);
	    }
	}
    }
  pids.reset ();
  if (error)
    {
      set_errno (error);
      return -1;
    }
  return 0;
}

extern "C" int
getpriority (int which, id_t who)
{
  int nice = NZERO * 2; /* Illegal value */

  switch (which)
    {
    case PRIO_PROCESS:
      if (!who)
	who = myself->pid;
      if ((pid_t) who == myself->pid)
	return myself->nice;
      break;
    case PRIO_PGRP:
      if (!who)
	who = myself->pgid;
      break;
    case PRIO_USER:
      if (!who)
	who = myself->uid;
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }
  winpids pids ((DWORD) 0);
  for (DWORD i = 0; i < pids.npids; ++i)
    {
      _pinfo *p = pids[i];
      if (p)
	switch (which)
	  {
	  case PRIO_PROCESS:
	    if ((pid_t) who == p->pid)
	      {
		nice = p->nice;
		goto out;
	      }
	    break;
	  case PRIO_PGRP:
	    if ((pid_t) who == p->pgid && p->nice < nice)
	      nice = p->nice;
	    break;
	  case PRIO_USER:
	    if ((__uid32_t) who == p->uid && p->nice < nice)
	      nice = p->nice;
	      break;
	  }
    }
out:
  pids.reset ();
  if (nice == NZERO * 2)
    {
      set_errno (ESRCH);
      return -1;
    }
  return nice;
}

extern "C" int
nice (int incr)
{
  return setpriority (PRIO_PROCESS, myself->pid, myself->nice + incr);
}

/*
 * Find the first bit set in I.
 */

extern "C" int
ffs (int i)
{
  static const unsigned char table[] =
    {
      0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
      6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
      7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
      7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
    };
  unsigned long int a;
  unsigned long int x = i & -i;

  a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ?  16 : 24);

  return table[x >> a] + a;
}

static void
locked_append (int fd, const void * buf, size_t size)
{
  struct __flock64 lock_buffer = {F_WRLCK, SEEK_SET, 0, 0, 0};
  int count = 0;

  do
    if ((lock_buffer.l_start = lseek64 (fd, 0, SEEK_END)) != (_off64_t) -1
	&& fcntl64 (fd, F_SETLKW, &lock_buffer) != -1)
      {
	if (lseek64 (fd, 0, SEEK_END) != (_off64_t) -1)
	  write (fd, buf, size);
	lock_buffer.l_type = F_UNLCK;
	fcntl64 (fd, F_SETLK, &lock_buffer);
	break;
      }
  while (count++ < 1000
	 && (errno == EACCES || errno == EAGAIN)
	 && !usleep (1000));
}

extern "C" void
updwtmp (const char *wtmp_file, const struct utmp *ut)
{
  int fd;

  if ((fd = open (wtmp_file, O_WRONLY | O_BINARY, 0)) >= 0)
    {
      locked_append (fd, ut, sizeof *ut);
      close (fd);
    }
}

static int utmp_fd = -1;
static bool utmp_readonly = false;
static char *utmp_file = (char *) _PATH_UTMP;

static void
internal_setutent (bool force_readwrite)
{
  if (force_readwrite && utmp_readonly)
    endutent ();
  if (utmp_fd < 0)
    {
      utmp_fd = open (utmp_file, O_RDWR | O_BINARY);
      /* If open fails, we assume an unprivileged process (who?).  In this
	 case we try again for reading only unless the process calls
	 pututline() (==force_readwrite) in which case opening just fails. */
      if (utmp_fd < 0 && !force_readwrite)
	{
	  utmp_fd = open (utmp_file, O_RDONLY | O_BINARY);
	  if (utmp_fd >= 0)
	    utmp_readonly = true;
	}
    }
  else
    lseek (utmp_fd, 0, SEEK_SET);
}

extern "C" void
setutent ()
{
  internal_setutent (false);
}

extern "C" void
endutent ()
{
  if (utmp_fd >= 0)
    {
      close (utmp_fd);
      utmp_fd = -1;
      utmp_readonly = false;
    }
}

extern "C" void
utmpname (const char *file)
{
  myfault efault;
  if (efault.faulted () || !*file)
    {
      debug_printf ("Invalid file");
      return;
    }
  endutent ();
  utmp_file = strdup (file);
  debug_printf ("New UTMP file: %s", utmp_file);
}
EXPORT_ALIAS (utmpname, utmpxname)

/* Note: do not make NO_COPY */
static struct utmp utmp_data_buf[16];
static unsigned utix = 0;
#define nutdbuf (sizeof (utmp_data_buf) / sizeof (utmp_data_buf[0]))
#define utmp_data ({ \
  if (utix > nutdbuf) \
    utix = 0; \
  utmp_data_buf + utix++; \
})

static struct utmpx *
copy_ut_to_utx (struct utmp *ut, struct utmpx *utx)
{
  if (!ut)
    return NULL;
  memcpy (utx, ut, sizeof *ut);
  utx->ut_tv.tv_sec = ut->ut_time;
  utx->ut_tv.tv_usec = 0;
  return utx;
}

extern "C" struct utmp *
getutent ()
{
  if (utmp_fd < 0)
    {
      internal_setutent (false);
      if (utmp_fd < 0)
	return NULL;
    }

  utmp *ut = utmp_data;
  if (read (utmp_fd, ut, sizeof *ut) != sizeof *ut)
    return NULL;
  return ut;
}

extern "C" struct utmp *
getutid (const struct utmp *id)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  if (utmp_fd < 0)
    {
      internal_setutent (false);
      if (utmp_fd < 0)
	return NULL;
    }

  utmp *ut = utmp_data;
  while (read (utmp_fd, ut, sizeof *ut) == sizeof *ut)
    {
      switch (id->ut_type)
	{
	case RUN_LVL:
	case BOOT_TIME:
	case OLD_TIME:
	case NEW_TIME:
	  if (id->ut_type == ut->ut_type)
	    return ut;
	  break;
	case INIT_PROCESS:
	case LOGIN_PROCESS:
	case USER_PROCESS:
	case DEAD_PROCESS:
	   if (strncmp (id->ut_id, ut->ut_id, UT_IDLEN) == 0)
	    return ut;
	  break;
	default:
	  return NULL;
	}
    }
  return NULL;
}

extern "C" struct utmp *
getutline (const struct utmp *line)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  if (utmp_fd < 0)
    {
      internal_setutent (false);
      if (utmp_fd < 0)
	return NULL;
    }

  utmp *ut = utmp_data;
  while (read (utmp_fd, ut, sizeof *ut) == sizeof *ut)
    if ((ut->ut_type == LOGIN_PROCESS ||
	 ut->ut_type == USER_PROCESS) &&
	!strncmp (ut->ut_line, line->ut_line, sizeof (ut->ut_line)))
      return ut;

  return NULL;
}

extern "C" struct utmp *
pututline (const struct utmp *ut)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  internal_setutent (true);
  if (utmp_fd < 0)
    {
      debug_printf ("error: utmp_fd %d", utmp_fd);
      return NULL;
    }
  debug_printf ("ut->ut_type %d, ut->ut_pid %d, ut->ut_line '%s', ut->ut_id '%s'\n",
		ut->ut_type, ut->ut_pid, ut->ut_line, ut->ut_id);
  debug_printf ("ut->ut_user '%s', ut->ut_host '%s'\n",
		ut->ut_user, ut->ut_host);

  struct utmp *u;
  if ((u = getutid (ut)))
    {
      lseek (utmp_fd, -sizeof *ut, SEEK_CUR);
      write (utmp_fd, ut, sizeof *ut);
    }
  else
    locked_append (utmp_fd, ut, sizeof *ut);
  /* The documentation says to return a pointer to this which implies that
     this has to be cast from a const.  That doesn't seem right but the
     documentation seems pretty clear on this.  */
  return (struct utmp *) ut;
}

extern "C" void
setutxent ()
{
  internal_setutent (false);
}

extern "C" void
endutxent ()
{
  endutent ();
}

extern "C" struct utmpx *
getutxent ()
{
  /* UGH.  Not thread safe. */
  static struct utmpx utx;
  return copy_ut_to_utx (getutent (), &utx);
}

extern "C" struct utmpx *
getutxid (const struct utmpx *id)
{
  /* UGH.  Not thread safe. */
  static struct utmpx utx;

  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  ((struct utmpx *)id)->ut_time = id->ut_tv.tv_sec;
  return copy_ut_to_utx (getutid ((struct utmp *) id), &utx);
}

extern "C" struct utmpx *
getutxline (const struct utmpx *line)
{
  /* UGH.  Not thread safe. */
  static struct utmpx utx;

  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  ((struct utmpx *)line)->ut_time = line->ut_tv.tv_sec;
  return copy_ut_to_utx (getutline ((struct utmp *) line), &utx);
}

extern "C" struct utmpx *
pututxline (const struct utmpx *utmpx)
{
  /* UGH.  Not thread safe. */
  static struct utmpx utx;

  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  ((struct utmpx *)utmpx)->ut_time = utmpx->ut_tv.tv_sec;
  return copy_ut_to_utx (pututline ((struct utmp *) utmpx), &utx);
}

extern "C" void
updwtmpx (const char *wtmpx_file, const struct utmpx *utmpx)
{
  ((struct utmpx *)utmpx)->ut_time = utmpx->ut_tv.tv_sec;
  updwtmp (wtmpx_file, (const struct utmp *) utmpx);
}

extern "C"
long gethostid (void)
{
  unsigned data[13] = {0x92895012,
		       0x10293412,
		       0x29602018,
		       0x81928167,
		       0x34601329,
		       0x75630198,
		       0x89860395,
		       0x62897564,
		       0x00194362,
		       0x20548593,
		       0x96839102,
		       0x12219854,
		       0x00290012};

  bool has_cpuid = false;

  DWORD opmask = SetThreadAffinityMask (GetCurrentThread (), 1);
  if (!opmask)
    debug_printf ("SetThreadAffinityMask to 1 failed, %E");

  if (!can_set_flag (0x00040000))
    debug_printf ("386 processor - no cpuid");
  else
    {
      debug_printf ("486 processor");
      if (can_set_flag (0x00200000))
	{
	  debug_printf ("processor supports CPUID instruction");
	  has_cpuid = true;
	}
      else
	debug_printf ("processor does not support CPUID instruction");
    }
  if (has_cpuid)
    {
      unsigned maxf, unused[3];
      cpuid (&maxf, &unused[0], &unused[1], &unused[2], 0);
      maxf &= 0xffff;
      if (maxf >= 1)
	{
	  unsigned features;
	  cpuid (&data[0], &unused[0], &unused[1], &features, 1);
	  if (features & (1 << 18))
	    {
	      debug_printf ("processor has psn");
	      if (maxf >= 3)
		{
		  cpuid (&unused[0], &unused[1], &data[1], &data[2], 3);
		  debug_printf ("Processor PSN: %04x-%04x-%04x-%04x-%04x-%04x",
				data[0] >> 16, data[0] & 0xffff, data[2] >> 16, data[2] & 0xffff, data[1] >> 16, data[1] & 0xffff);
		}
	    }
	  else
	    debug_printf ("processor does not have psn");
	}
    }

  UUID Uuid;
  RPC_STATUS status = UuidCreateSequential (&Uuid);
  if (GetLastError () == ERROR_PROC_NOT_FOUND)
    status = UuidCreate (&Uuid);
  if (status == RPC_S_OK)
    {
      data[4] = *(unsigned *)&Uuid.Data4[2];
      data[5] = *(unsigned short *)&Uuid.Data4[6];
      // Unfortunately Windows will sometimes pick a virtual Ethernet card
      // e.g. VMWare Virtual Ethernet Adaptor
      debug_printf ("MAC address of first Ethernet card: %02x:%02x:%02x:%02x:%02x:%02x",
		    Uuid.Data4[2], Uuid.Data4[3], Uuid.Data4[4],
		    Uuid.Data4[5], Uuid.Data4[6], Uuid.Data4[7]);
    }
  else
    {
      debug_printf ("no Ethernet card installed");
    }

  reg_key key (HKEY_LOCAL_MACHINE, KEY_READ, "SOFTWARE", "Microsoft", "Windows", "CurrentVersion", NULL);
  key.get_string ("ProductId", (char *)&data[6], 24, "00000-000-0000000-00000");
  debug_printf ("Windows Product ID: %s", (char *)&data[6]);

  /* Contrary to MSDN, NT4 requires the second argument
     or a STATUS_ACCESS_VIOLATION is generated */
  ULARGE_INTEGER availb;
  GetDiskFreeSpaceEx ("C:\\", &availb, (PULARGE_INTEGER) &data[11], NULL);

  debug_printf ("hostid entropy: %08x %08x %08x %08x "
				"%08x %08x %08x %08x "
				"%08x %08x %08x %08x "
				"%08x",
				data[0], data[1],
				data[2], data[3],
				data[4], data[5],
				data[6], data[7],
				data[8], data[9],
				data[10], data[11],
				data[12]);

  long hostid = 0x40291372;
  // a random hashing algorithm
  // dependancy on md5 is probably too costly
  for (int i=0;i<13;i++)
    hostid ^= ((data[i] << (i << 2)) | (data[i] >> (32 - (i << 2))));

  if (opmask && !SetThreadAffinityMask (GetCurrentThread (), opmask))
    debug_printf ("SetThreadAffinityMask to %p failed, %E", opmask);

  debug_printf ("hostid: %08x", hostid);

  return hostid;
}

#define ETC_SHELLS "/etc/shells"
static int shell_index;
static struct __sFILE64 *shell_fp;

extern "C" char *
getusershell ()
{
  /* List of default shells if no /etc/shells exists, defined as on Linux.
     FIXME: SunOS has a far longer list, containing all shells which
     might be shipped with the OS.  Should we do the same for the Cygwin
     distro, adding bash, tcsh, ksh, pdksh and zsh?  */
  static NO_COPY const char *def_shells[] = {
    "/bin/sh",
    "/bin/csh",
    "/usr/bin/sh",
    "/usr/bin/csh",
    NULL
  };
  static char buf[PATH_MAX];
  int ch, buf_idx;

  if (!shell_fp && !(shell_fp = fopen64 (ETC_SHELLS, "rt")))
    {
      if (def_shells[shell_index])
	return strcpy (buf, def_shells[shell_index++]);
      return NULL;
    }
  /* Skip white space characters. */
  while ((ch = getc (shell_fp)) != EOF && isspace (ch))
    ;
  /* Get each non-whitespace character as part of the shell path as long as
     it fits in buf. */
  for (buf_idx = 0;
       ch != EOF && !isspace (ch) && buf_idx < PATH_MAX;
       buf_idx++, ch = getc (shell_fp))
    buf[buf_idx] = ch;
  /* Skip any trailing non-whitespace character not fitting in buf.  If the
     path is longer than PATH_MAX, it's invalid anyway. */
  while (ch != EOF && !isspace (ch))
    ch = getc (shell_fp);
  if (buf_idx)
    {
      buf[buf_idx] = '\0';
      return buf;
    }
  return NULL;
}

extern "C" void
setusershell ()
{
  if (shell_fp)
    fseek (shell_fp, 0L, SEEK_SET);
  shell_index = 0;
}

extern "C" void
endusershell ()
{
  if (shell_fp)
    {
      fclose (shell_fp);
      shell_fp = NULL;
    }
  shell_index = 0;
}

extern "C" void
flockfile (FILE *file)
{
  _flockfile (file);
}

extern "C" int
ftrylockfile (FILE *file)
{
  return _ftrylockfile (file);
}

extern "C" void
funlockfile (FILE *file)
{
  _funlockfile (file);
}

extern "C" FILE *
popen (const char *command, const char *in_type)
{
  const char *type = in_type;
  char rw = *type++;

  /* Sanity check in_type */
  if (*type == 'b' || *type == 't')
    type++;
  if ((rw != 'r' && rw != 'w') || (*type != '\0'))
    {
      set_errno (EINVAL);
      return NULL;
    }

  int fds[2];
  if (pipe (fds) < 0)
    return NULL;

  int myix = rw == 'r' ? 0 : 1;

  lock_process now;
  FILE *fp = fdopen (fds[myix], in_type);
  if (fp)
    {
      /* If fds are in the range of stdin/stdout/stderr, move them
	 out of the way (possibly temporarily).  Otherwise, spawn_guts
	 will be confused.  We do this here rather than adding logic to
	 spawn_guts because spawn_guts is likely to be a more frequently
	 used routine and having stdin/stdout/stderr closed and reassigned
	 to pipe handles is an unlikely event. */
      int orig_fds[2] = {fds[0], fds[1]};
      for (int i = 0; i < 2; i++)
	if (fds[i] <= 2)
	  {
	    cygheap_fdnew newfd(3);
	    cygheap->fdtab.move_fd (fds[i], newfd);
	    fds[i] = newfd;
	  }

      int myfd = fds[myix];	/* myfd - convenience variable for manipulation
				   of the "parent" end of the pipe. */
      int stdchild = myix ^ 1;	/* stdchild denotes the index into fd for the
				   handle which will be redirected to
				   stdin/stdout */
      int __std[2];
      __std[myix] = -1;		/* -1 means don't pass this fd to the child
				   process */
      __std[stdchild] = fds[stdchild]; /* Do pass this as the std handle */

      const char *argv[4] =
	{
	  "/bin/sh",
	  "-c",
	  command,
	  NULL
	};

      /* Don't pass our end of the pipe to the child process */
      int fd_state = fcntl64 (myfd, F_GETFD, 0);
      fcntl64 (myfd, F_SETFD, fd_state | FD_CLOEXEC);

      /* Also don't pass the file handle currently associated with stdin/stdout
	 to the child.  This function may actually fail if the stdchild fd
	 is closed.  But that's ok. */
      int stdchild_state = fcntl64 (stdchild, F_GETFD, 0);
      fcntl64 (stdchild, F_SETFD, stdchild_state | FD_CLOEXEC);

      /* Start a shell process to run the given command without forking. */
      pid_t pid = spawn_guts ("/bin/sh", argv, cur_environ (), _P_NOWAIT,
			       __std[0], __std[1]);

      /* Reinstate the close-on-exec state */
      fcntl64 (stdchild, F_SETFD, stdchild_state);

      /* If pid >= 0 then spawn_guts succeeded.  */
      if (pid >= 0)
	{
	  close (fds[stdchild]);	/* Close the child end of the pipe. */
	  /* Move the fd back to its original slot if it has been moved since
	     we're always supposed to open the lowest numbered available fd
	     and, if fds[mix] != orig_fds[myix] then orig_fds[myix] is
	     presumably lower. */
	  if (fds[myix] != orig_fds[myix])
	    cygheap->fdtab.move_fd (fds[myix], myfd = orig_fds[myix]);
	  fhandler_pipe *fh = (fhandler_pipe *) cygheap->fdtab[myfd];
	  /* Flag that this handle is associated with popen and then reset
	     the handle's original close-on-exec state. */
	  fh->set_popen_pid (pid);
	  fcntl64 (myfd, F_SETFD, fd_state);
	  return fp;
	}
    }

  /* If we reach here we've seen an error but the pipe handles are open.
     Close them and return NULL. */
  int save_errno = get_errno ();
  close (fds[0]);
  close (fds[1]);
  set_errno (save_errno);
  return NULL;
}

int
pclose (FILE *fp)
{
  fhandler_pipe *fh = (fhandler_pipe *) cygheap->fdtab[fileno(fp)];

  if (fh->get_device () != FH_PIPEW && fh->get_device () != FH_PIPER)
    {
      set_errno (EBADF);
      return -1;
    }

  int pid = fh->get_popen_pid ();
  if (!pid)
    {
      set_errno (ECHILD);
      return -1;
    }

  if (fclose (fp))
    return -1;

  int status;
  while (1)
    if (waitpid (pid, &status, 0) == pid)
      break;
    else if (get_errno () == EINTR)
      continue;
    else
      return -1;

  return status;
}

/* Preliminary(?) implementation of the openat family of functions. */

static int
gen_full_path_at (char *path_ret, int dirfd, const char *pathname,
		  bool null_pathname_allowed = false)
{
  /* Set null_pathname_allowed to true to allow GLIBC compatible behaviour
     for NULL pathname.  Only used by futimesat. */
  if (!pathname && !null_pathname_allowed)
    {
      set_errno (EFAULT);
      return -1;
    }
  if (pathname)
    {
      if (!*pathname)
	{
	  set_errno (ENOENT);
	  return -1;
	}
      if (strlen (pathname) >= PATH_MAX)
	{
	  set_errno (ENAMETOOLONG);
	  return -1;
	}
    }
  if (pathname && isabspath (pathname))
    stpcpy (path_ret, pathname);
  else
    {
      char *p;

      if (dirfd == AT_FDCWD)
	{
	  cwdstuff::cwd_lock.acquire ();
	  p = stpcpy (path_ret, cygheap->cwd.get_posix ());
	  cwdstuff::cwd_lock.release ();
	}
      else
	{
	  cygheap_fdget cfd (dirfd);
	  if (cfd < 0)
	    return -1;
	  if (!cfd->pc.isdir ())
	    {
	      set_errno (ENOTDIR);
	      return -1;
	    }
	  p = stpcpy (path_ret, cfd->get_name ());
	}
      if (!p)
	{
	  set_errno (ENOTDIR);
	  return -1;
	}
      if (pathname)
	{
	  if (p[-1] != '/')
	    *p++ = '/';
	  stpcpy (p, pathname);
	}
    }
  return 0;
}

extern "C" int
openat (int dirfd, const char *pathname, int flags, ...)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;

  va_list ap;
  mode_t mode;

  va_start (ap, flags);
  mode = va_arg (ap, mode_t);
  va_end (ap);
  return open (path, flags, mode);
}

extern "C" int
faccessat (int dirfd, const char *pathname, int mode, int flags)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  int res = -1;
  char *path = tp.c_get ();
  if (!gen_full_path_at (path, dirfd, pathname))
    {
      if ((mode & ~(F_OK|R_OK|W_OK|X_OK))
	  || (flags & ~(AT_SYMLINK_NOFOLLOW|AT_EACCESS)))
	set_errno (EINVAL);
      else
	{
	  fhandler_base *fh = build_fh_name (path,
					     (flags & AT_SYMLINK_NOFOLLOW)
					     ? PC_SYM_NOFOLLOW : PC_SYM_FOLLOW,
					     stat_suffixes);
	  if (fh)
	    {
	      res =  fh->fhaccess (mode, flags & AT_EACCESS);
	      delete fh;
	    }
	}
    }
  debug_printf ("returning %d", res);
  return res;
}

extern "C" int
fchmodat (int dirfd, const char *pathname, mode_t mode, int flags)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (flags)
    {
      /* BSD has lchmod, but Linux does not.  POSIX says
	 AT_SYMLINK_NOFOLLOW is allowed to fail on symlinks; but Linux
	 blindly fails even for non-symlinks.  */
      set_errno ((flags & ~AT_SYMLINK_NOFOLLOW) ? EINVAL : EOPNOTSUPP);
      return -1;
    }
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return chmod (path, mode);
}

extern "C" int
fchownat (int dirfd, const char *pathname, __uid32_t uid, __gid32_t gid,
	 int flags)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (flags & ~AT_SYMLINK_NOFOLLOW)
    {
      set_errno (EINVAL);
      return -1;
    }
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return chown_worker (path, (flags & AT_SYMLINK_NOFOLLOW)
			     ? PC_SYM_NOFOLLOW : PC_SYM_FOLLOW, uid, gid);
}

extern "C" int
fstatat (int dirfd, const char *pathname, struct __stat64 *st, int flags)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (flags & ~AT_SYMLINK_NOFOLLOW)
    {
      set_errno (EINVAL);
      return -1;
    }
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return (flags & AT_SYMLINK_NOFOLLOW) ? lstat64 (path, st) : stat64 (path, st);
}

extern int utimens_worker (path_conv &, const struct timespec *);

extern "C" int
utimensat (int dirfd, const char *pathname, const struct timespec *times,
	   int flags)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *path = tp.c_get ();
  if (flags & ~AT_SYMLINK_NOFOLLOW)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  path_conv win32 (path, PC_POSIX | ((flags & AT_SYMLINK_NOFOLLOW)
				     ? PC_SYM_NOFOLLOW : PC_SYM_FOLLOW),
		   stat_suffixes);
  return utimens_worker (win32, times);
}

extern "C" int
futimesat (int dirfd, const char *pathname, const struct timeval *times)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname, true))
    return -1;
  return utimes (path, times);
}

extern "C" int
linkat (int olddirfd, const char *oldpathname,
	int newdirfd, const char *newpathname,
	int flags)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (flags & ~AT_SYMLINK_FOLLOW)
    {
      set_errno (EINVAL);
      return -1;
    }
  char *oldpath = tp.c_get ();
  if (gen_full_path_at (oldpath, olddirfd, oldpathname))
    return -1;
  char *newpath = tp.c_get ();
  if (gen_full_path_at (newpath, newdirfd, newpathname))
    return -1;
  if (flags & AT_SYMLINK_FOLLOW)
    {
      path_conv old_name (oldpath, PC_SYM_FOLLOW | PC_POSIX, stat_suffixes);
      if (old_name.error)
	{
	  set_errno (old_name.error);
	  return -1;
	}
      strcpy (oldpath, old_name.normalized_path);
    }
  return link (oldpath, newpath);
}

extern "C" int
mkdirat (int dirfd, const char *pathname, mode_t mode)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return mkdir (path, mode);
}

extern "C" int
mkfifoat (int dirfd, const char *pathname, mode_t mode)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return mkfifo (path, mode);
}

extern "C" int
mknodat (int dirfd, const char *pathname, mode_t mode, __dev32_t dev)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return mknod32 (path, mode, dev);
}

extern "C" ssize_t
readlinkat (int dirfd, const char *pathname, char *buf, size_t bufsize)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return readlink (path, buf, bufsize);
}

extern "C" int
renameat (int olddirfd, const char *oldpathname,
	  int newdirfd, const char *newpathname)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *oldpath = tp.c_get ();
  if (gen_full_path_at (oldpath, olddirfd, oldpathname))
    return -1;
  char *newpath = tp.c_get ();
  if (gen_full_path_at (newpath, newdirfd, newpathname))
    return -1;
  return rename (oldpath, newpath);
}

extern "C" int
symlinkat (const char *oldpath, int newdirfd, const char *newpathname)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  char *newpath = tp.c_get ();
  if (gen_full_path_at (newpath, newdirfd, newpathname))
    return -1;
  return symlink (oldpath, newpath);
}

extern "C" int
unlinkat (int dirfd, const char *pathname, int flags)
{
  tmp_pathbuf tp;
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (flags & ~AT_REMOVEDIR)
    {
      set_errno (EINVAL);
      return -1;
    }
  char *path = tp.c_get ();
  if (gen_full_path_at (path, dirfd, pathname))
    return -1;
  return (flags & AT_REMOVEDIR) ? rmdir (path) : unlink (path);
}

static char *
check_codepage (char *ret)
{
  if (!wincap.has_always_all_codepages ())
    {
      /* Prior to Windows Vista, many codepages are not installed by
	 default, or can be deinstalled.  The following codepages require
	 that the respective conversion tables are installed into the OS.
	 So we check if they are installed and if not, setlocale should
	 fail. */
      CPINFO cpi;
      UINT cp = 0;
      if (__mbtowc == __sjis_mbtowc)
	cp = 932;
      else if (__mbtowc == __eucjp_mbtowc)
	cp = 20932;
      else if (__mbtowc == __gbk_mbtowc)
	cp = 936;
      else if (__mbtowc == __kr_mbtowc)
	cp = 949;
      else if (__mbtowc == __big5_mbtowc)
	cp = 950;
      if (cp && !GetCPInfo (cp, &cpi)
	  && GetLastError () == ERROR_INVALID_PARAMETER)
	return NULL;
    }
  return ret;
}

static void
internal_setlocale ()
{
  /* Each setlocale from the environment potentially changes the
     multibyte representation of the CWD.  Therefore we have to
     reevaluate the CWD's posix path and store in the new charset.
     Same for the PATH environment variable. */
  /* FIXME: Other buffered paths might be affected as well. */
  /* FIXME: It could be necessary to convert the entire environment,
	    not just PATH. */
  tmp_pathbuf tp;
  char *path = getenv ("PATH");
  wchar_t *w_path = NULL, *w_cwd;

  debug_printf ("Cygwin charset changed from %s to %s",
		cygheap->locale.charset, __locale_charset ());
  /* Fetch PATH and CWD and convert to wchar_t in previous charset. */
  if (path && *path)	/* $PATH can be potentially unset. */
    {
      w_path = tp.w_get ();
      sys_mbstowcs (w_path, 32768, path);
    }
  w_cwd = tp.w_get ();
  cwdstuff::cwd_lock.acquire ();
  sys_mbstowcs (w_cwd, 32768, cygheap->cwd.get_posix ());
  /* Set charset for internal conversion functions. */
  cygheap->locale.mbtowc = __mbtowc;
  cygheap->locale.wctomb = __wctomb;
  strcpy (cygheap->locale.charset, __locale_charset ());
  /* Restore CWD and PATH in new charset. */
  cygheap->cwd.reset_posix (w_cwd);
  cwdstuff::cwd_lock.release ();
  if (w_path)
    {
      char *c_path = tp.c_get ();
      sys_wcstombs (c_path, 32768, w_path);
      setenv ("PATH", c_path, 1);
    }
}

/* Called from dll_crt0_1, before calling the application's main().
   Set the internal charset according to the environment locale settings.
   Check if a required codepage is available, and only switch internal
   charset if so.  Afterwards, reset application locale to "C" per POSIX. */
void
initial_setlocale ()
{
  char *ret = _setlocale_r (_REENT, LC_CTYPE, "");
  if (ret && check_codepage (ret)
      && strcmp (cygheap->locale.charset, __locale_charset ()) != 0)
    internal_setlocale ();
  _setlocale_r (_REENT, LC_CTYPE, "C");
}

/* Like newlib's setlocale, but additionally check if the charset needs
   OS support and the required codepage is actually installed.  If codepage
   is not available, revert to previous locale and return NULL.  For details
   about codepage availability, see the comment in check_codepage() above. */
extern "C" char *
setlocale (int category, const char *locale)
{
  char old[(LC_MESSAGES + 1) * (ENCODING_LEN + 1/*"/"*/ + 1)];
  if (locale && !wincap.has_always_all_codepages ())
    stpcpy (old, _setlocale_r (_REENT, category, NULL));
  char *ret = _setlocale_r (_REENT, category, locale);
  if (ret && locale && !(ret = check_codepage (ret)))
    _setlocale_r (_REENT, category, old);
  return ret;
}
