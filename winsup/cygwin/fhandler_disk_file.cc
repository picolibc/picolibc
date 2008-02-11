/* fhandler_disk_file.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <sys/acl.h>
#include <sys/statvfs.h>
#include <signal.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "cygwin/version.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "pinfo.h"
#include "ntdll.h"
#include <assert.h>
#include <ctype.h>
#include <winioctl.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

class __DIR_mounts
{
  int		 count;
  const char	*parent_dir;
  int		 parent_dir_len;
  UNICODE_STRING mounts[MAX_MOUNTS];
  bool		 found[MAX_MOUNTS + 2];
  UNICODE_STRING cygdrive;

#define __DIR_PROC	(MAX_MOUNTS)
#define __DIR_CYGDRIVE	(MAX_MOUNTS+1)

  __ino64_t eval_ino (int idx)
    {
      __ino64_t ino = 0;
      char fname[parent_dir_len + mounts[idx].Length + 2];
      struct __stat64 st;

      char *c = stpcpy (fname, parent_dir);
      if (c[- 1] != '/')
	*c++ = '/';
      sys_wcstombs (c, mounts[idx].Length + 1,
		    mounts[idx].Buffer, mounts[idx].Length / sizeof (WCHAR));
      path_conv pc (fname, PC_SYM_NOFOLLOW | PC_POSIX);
      if (!stat_worker (pc, &st))
	ino = st.st_ino;
      return ino;
    }

public:
  __DIR_mounts (const char *posix_path)
  : parent_dir (posix_path)
    {
      parent_dir_len = strlen (parent_dir);
      count = mount_table->get_mounts_here (parent_dir, parent_dir_len, mounts,
					    &cygdrive);
      rewind ();
    }
  ~__DIR_mounts ()
    {
      for (int i = 0; i < count; ++i)
        RtlFreeUnicodeString (&mounts[i]);
      RtlFreeUnicodeString (&cygdrive);
    }
  __ino64_t check_mount (PUNICODE_STRING fname, __ino64_t ino,
			 bool eval = true)
    {
      if (parent_dir_len == 1)	/* root dir */
	{
	  UNICODE_STRING proc;

	  RtlInitUnicodeString (&proc, L"proc");
	  if (RtlEqualUnicodeString (fname, &proc, TRUE))
	    {
	      found[__DIR_PROC] = true;
	      return 2;
	    }
	  if (fname->Length / sizeof (WCHAR) == mount_table->cygdrive_len - 2
	      && RtlEqualUnicodeString (fname, &cygdrive, TRUE))
	    {
	      found[__DIR_CYGDRIVE] = true;
	      return 2;
	    }
	}
      for (int i = 0; i < count; ++i)
	if (RtlEqualUnicodeString (fname, &mounts[i], TRUE))
	  {
	    found[i] = true;
	    return eval ? eval_ino (i) : 1;
	  }
      return ino;
    }
  __ino64_t check_missing_mount (PUNICODE_STRING retname = NULL)
    {
      for (int i = 0; i < count; ++i)
	if (!found[i])
	  {
	    found[i] = true;
	    if (retname)
	      {
		*retname = mounts[i];
		return eval_ino (i);
	      }
	    return 1;
	  }
      if (parent_dir_len == 1)  /* root dir */
	{
	  if (!found[__DIR_PROC])
	    {
	      found[__DIR_PROC] = true;
	      if (retname)
		RtlInitUnicodeString (retname, L"proc");
	      return 2;
	    }
	  if (!found[__DIR_CYGDRIVE])
	    {
	      found[__DIR_CYGDRIVE] = true;
	      if (cygdrive.Length > 0)
		{
		  if (retname)
		    *retname = cygdrive;
		  return 2;
		}
	    }
	}
      return 0;
    }
    void rewind () { memset (found, 0, sizeof found); }
};

static inline bool
is_volume_mountpoint (POBJECT_ATTRIBUTES attr)
{
  bool ret = false;
  IO_STATUS_BLOCK io;
  HANDLE reph;

  if (NT_SUCCESS (NtOpenFile (&reph, READ_CONTROL, attr, &io,
			      FILE_SHARE_VALID_FLAGS,
			      FILE_OPEN_FOR_BACKUP_INTENT
			      | FILE_OPEN_REPARSE_POINT)))
    {
      PREPARSE_DATA_BUFFER rp = (PREPARSE_DATA_BUFFER)
		  alloca (MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
      if (NT_SUCCESS (NtFsControlFile (reph, NULL, NULL, NULL,
		      &io, FSCTL_GET_REPARSE_POINT, NULL, 0,
		      (LPVOID) rp, MAXIMUM_REPARSE_DATA_BUFFER_SIZE))
	  && rp->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT
	  && rp->SymbolicLinkReparseBuffer.PrintNameLength == 0)
	ret = true;
      NtClose (reph);
    }
  return ret;
}

unsigned __stdcall
path_conv::ndisk_links (DWORD nNumberOfLinks)
{
  if (!isdir () || isremote ())
    return nNumberOfLinks;

  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  HANDLE fh;

  if (!NT_SUCCESS (NtOpenFile (&fh, SYNCHRONIZE | FILE_LIST_DIRECTORY,
			       get_object_attr (attr, sec_none_nih),
			       &io, FILE_SHARE_VALID_FLAGS,
			       FILE_SYNCHRONOUS_IO_NONALERT
			       | FILE_OPEN_FOR_BACKUP_INTENT
			       | FILE_DIRECTORY_FILE)))
    return nNumberOfLinks;

  unsigned count = 0;
  bool first = true;
  PFILE_DIRECTORY_INFORMATION fdibuf = (PFILE_DIRECTORY_INFORMATION)
				       alloca (65536);
  __DIR_mounts *dir = new __DIR_mounts (normalized_path);
  while (NT_SUCCESS (NtQueryDirectoryFile (fh, NULL, NULL, 0, &io, fdibuf,
					   65536, FileDirectoryInformation,
					   FALSE, NULL, first)))
    {
      if (first)
	{
	  first = false;
	  /* All directories have . and .. as their first entries.
	     If . is not present as first entry, we're on a drive's
	     root direcotry, which doesn't have these entries. */
	  if (fdibuf->FileNameLength != 2 || fdibuf->FileName[0] != L'.')
	    count = 2;
	}
      for (PFILE_DIRECTORY_INFORMATION pfdi = fdibuf;
	   pfdi;
	   pfdi = (PFILE_DIRECTORY_INFORMATION)
		  (pfdi->NextEntryOffset ? (PBYTE) pfdi + pfdi->NextEntryOffset
					 : NULL))
	{
	  switch (pfdi->FileAttributes
		  & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
	    {
	    case FILE_ATTRIBUTE_DIRECTORY:
	      /* Just a directory */
	      ++count;
	      break;
	    case FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT:
	      /* Volume mount point or symlink to directory */
	      {
		UNICODE_STRING fname;

		RtlInitCountedUnicodeString (&fname, pfdi->FileName,
					     pfdi->FileNameLength);
		InitializeObjectAttributes (&attr, &fname,
					    OBJ_CASE_INSENSITIVE, fh, NULL);
		if (is_volume_mountpoint (&attr))
		  ++count;
	      }
	      break;
	    default:
	      break;
	    }
	  UNICODE_STRING fname;
	  RtlInitCountedUnicodeString (&fname, pfdi->FileName,
				       pfdi->FileNameLength);
	  dir->check_mount (&fname, 0, false);
	}
    }
  while (dir->check_missing_mount ())
    ++count;
  NtClose (fh);
  delete dir;
  return count;
}

inline bool
path_conv::isgood_inode (__ino64_t ino) const
{
  /* We can't trust remote inode numbers of only 32 bit.  That means,
     all remote inode numbers when running under NT4, as well as remote NT4
     NTFS, as well as shares of Samba version < 3.0.
     The known exception are SFU NFS shares, which return the valid 32 bit
     inode number from the remote file system unchanged. */
  return hasgood_inode () && (ino > UINT32_MAX || !isremote () || fs_is_nfs ());
}

int __stdcall
fhandler_base::fstat_by_handle (struct __stat64 *buf)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  /* The entries potentially contain a name of MAX_PATH wide characters. */
  const DWORD fvi_size = (NAME_MAX + 1) * sizeof (WCHAR)
			 + sizeof (FILE_FS_VOLUME_INFORMATION);
  const DWORD fai_size = (NAME_MAX + 1) * sizeof (WCHAR)
			 + sizeof (FILE_ALL_INFORMATION);

  PFILE_FS_VOLUME_INFORMATION pfvi = (PFILE_FS_VOLUME_INFORMATION)
				     alloca (fvi_size);
  PFILE_ALL_INFORMATION pfai = (PFILE_ALL_INFORMATION) alloca (fai_size);

  status = NtQueryVolumeInformationFile (get_handle (), &io, pfvi, fvi_size,
					 FileFsVolumeInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("%p = NtQueryVolumeInformationFile(%S)", status,
		    pc.get_nt_native_path ());
      pfvi->VolumeSerialNumber = 0;
    }
  status = NtQueryInformationFile (get_handle (), &io, pfai, fai_size,
				   FileAllInformation);
  if (NT_SUCCESS (status))
    {
      /* If the change time is 0, it's a file system which doesn't
	 support a change timestamp.  In that case use the LastWriteTime
	 entry, as in other calls to fstat_helper. */
      if (pc.is_rep_symlink ())
	pfai->BasicInformation.FileAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;
      pc.file_attributes (pfai->BasicInformation.FileAttributes);
      return fstat_helper (buf,
		       pfai->BasicInformation.ChangeTime.QuadPart
		       ? *(FILETIME *) &pfai->BasicInformation.ChangeTime
		       : *(FILETIME *) &pfai->BasicInformation.LastWriteTime,
		       *(FILETIME *) &pfai->BasicInformation.LastAccessTime,
		       *(FILETIME *) &pfai->BasicInformation.LastWriteTime,
		       *(FILETIME *) &pfai->BasicInformation.CreationTime,
		       pfvi->VolumeSerialNumber,
		       pfai->StandardInformation.EndOfFile.QuadPart,
		       pfai->StandardInformation.AllocationSize.QuadPart,
		       pfai->InternalInformation.FileId.QuadPart,
		       pfai->StandardInformation.NumberOfLinks,
		       pfai->BasicInformation.FileAttributes);
    }
  debug_printf ("%p = NtQueryInformationFile(%S)",
  		status, pc.get_nt_native_path ());
  return -1;
}

int __stdcall
fhandler_base::fstat_by_name (struct __stat64 *buf)
{
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  UNICODE_STRING dirname;
  UNICODE_STRING basename;
  HANDLE dir;
  const DWORD fdi_size = (NAME_MAX + 1) * sizeof (WCHAR)
			 + sizeof (FILE_ID_BOTH_DIR_INFORMATION);
  const DWORD fvi_size = (NAME_MAX + 1) * sizeof (WCHAR)
			 + sizeof (FILE_FS_VOLUME_INFORMATION);
  PFILE_ID_BOTH_DIR_INFORMATION pfdi = (PFILE_ID_BOTH_DIR_INFORMATION)
				       alloca (fdi_size);
  PFILE_FS_VOLUME_INFORMATION pfvi = (PFILE_FS_VOLUME_INFORMATION)
				     alloca (fvi_size);
  LARGE_INTEGER FileId;

  if (!pc.exists ())
    {
      debug_printf ("already determined that pc does not exist");
      set_errno (ENOENT);
      return -1;
    }
  RtlSplitUnicodePath (pc.get_nt_native_path (), &dirname, &basename);
  InitializeObjectAttributes (&attr, &dirname, OBJ_CASE_INSENSITIVE,
			      NULL, NULL);
  if (!NT_SUCCESS (status = NtOpenFile (&dir, SYNCHRONIZE | FILE_LIST_DIRECTORY,
				       &attr, &io, FILE_SHARE_VALID_FLAGS,
				       FILE_SYNCHRONOUS_IO_NONALERT
				       | FILE_OPEN_FOR_BACKUP_INTENT
				       | FILE_DIRECTORY_FILE)))
    {
      debug_printf ("%p = NtOpenFile(%S)", status, pc.get_nt_native_path ());
      goto too_bad;
    }
  if (wincap.has_fileid_dirinfo ()
      && NT_SUCCESS (status = NtQueryDirectoryFile (dir, NULL, NULL, 0, &io,
						 pfdi, fdi_size,
						 FileIdBothDirectoryInformation,
						 TRUE, &basename, TRUE)))
    FileId = pfdi->FileId;
  else if (NT_SUCCESS (status = NtQueryDirectoryFile (dir, NULL, NULL, 0, &io,
  						 pfdi, fdi_size,
						 FileBothDirectoryInformation,
						 TRUE, &basename, TRUE)))
    FileId.QuadPart = 0; /* get_namehash is called in fstat_helper. */
  if (!NT_SUCCESS (status))
    {
      debug_printf ("%p = NtQueryDirectoryFile(%S)", status,
		    pc.get_nt_native_path ());
      NtClose (dir);
      goto too_bad;
    }
  status = NtQueryVolumeInformationFile (dir, &io, pfvi, fvi_size,
					 FileFsVolumeInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("%p = NtQueryVolumeInformationFile(%S)",
		    status, pc.get_nt_native_path ());
      pfvi->VolumeSerialNumber = 0;
    }
  NtClose (dir);
  /* If the change time is 0, it's a file system which doesn't
     support a change timestamp.  In that case use the LastWriteTime
     entry, as in other calls to fstat_helper. */
  if (pc.is_rep_symlink ())
    pfdi->FileAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;
  pc.file_attributes (pfdi->FileAttributes);
  return fstat_helper (buf,
		       pfdi->ChangeTime.QuadPart ?
		       *(FILETIME *) &pfdi->ChangeTime :
		       *(FILETIME *) &pfdi->LastWriteTime,
		       *(FILETIME *) &pfdi->LastAccessTime,
		       *(FILETIME *) &pfdi->LastWriteTime,
		       *(FILETIME *) &pfdi->CreationTime,
		       pfvi->VolumeSerialNumber,
		       pfdi->EndOfFile.QuadPart,
		       pfdi->AllocationSize.QuadPart,
		       pfdi->FileId.QuadPart,
		       1,
		       pfdi->FileAttributes);

too_bad:
  LARGE_INTEGER ft;
  /* Arbitrary value: 2006-12-01 */
  RtlSecondsSince1970ToTime (1164931200L, &ft);
  return fstat_helper (buf,
		       *(FILETIME *) &ft,
		       *(FILETIME *) &ft,
		       *(FILETIME *) &ft,
		       *(FILETIME *) &ft,
		       0,
		       0ULL,
		       -1LL,
		       0ULL,
		       1,
		       pc.file_attributes ());
}

int __stdcall
fhandler_base::fstat_fs (struct __stat64 *buf)
{
  int res = -1;
  int oret;
  int open_flags = O_RDONLY | O_BINARY;

  if (get_handle ())
    {
      if (!nohandle () && !is_fs_special ())
        res = fstat_by_handle (buf);
      if (res)
	res = fstat_by_name (buf);
      return res;
    }
  query_open (query_read_attributes);
  oret = open_fs (open_flags, 0);
  if (oret)
    {
      /* We now have a valid handle, regardless of the "nohandle" state.
	 Since fhandler_base::open only calls CloseHandle if !nohandle,
	 we have to set it to false before calling close and restore
	 the state afterwards. */
      res = fstat_by_handle (buf);
      bool no_handle = nohandle ();
      nohandle (false);
      close_fs ();
      nohandle (no_handle);
      set_io_handle (NULL);
    }
  if (res)
    res = fstat_by_name (buf);

  return res;
}

/* The ftChangeTime is taken from the NTFS ChangeTime entry, if reading
   the file information using NtQueryInformationFile succeeded.  If not,
   it's faked using the LastWriteTime entry from GetFileInformationByHandle
   or FindFirstFile.  We're deliberatly not using the creation time anymore
   to simplify interaction with native Windows applications which choke on
   creation times >= access or write times.

   Note that the dwFileAttributes member of the file information evaluated
   in the calling function is used here, not the pc.fileattr member, since
   the latter might be old and not reflect the actual state of the file. */
int __stdcall
fhandler_base::fstat_helper (struct __stat64 *buf,
			     FILETIME ftChangeTime,
			     FILETIME ftLastAccessTime,
			     FILETIME ftLastWriteTime,
			     FILETIME ftCreationTime,
			     DWORD dwVolumeSerialNumber,
			     ULONGLONG nFileSize,
			     LONGLONG nAllocSize,
			     ULONGLONG nFileIndex,
			     DWORD nNumberOfLinks,
			     DWORD dwFileAttributes)
{
  IO_STATUS_BLOCK st;
  FILE_COMPRESSION_INFORMATION fci;

  to_timestruc_t (&ftLastAccessTime, &buf->st_atim);
  to_timestruc_t (&ftLastWriteTime, &buf->st_mtim);
  to_timestruc_t (&ftChangeTime, &buf->st_ctim);
  to_timestruc_t (&ftCreationTime, &buf->st_birthtim);
  buf->st_dev = dwVolumeSerialNumber;
  buf->st_size = (_off64_t) nFileSize;
  /* The number of links to a directory includes the
     number of subdirectories in the directory, since all
     those subdirectories point to it.
     This is too slow on remote drives, so we do without it.
     Setting the count to 2 confuses `find (1)' command. So
     let's try it with `1' as link count. */
  buf->st_nlink = pc.ndisk_links (nNumberOfLinks);

  /* Enforce namehash as inode number on untrusted file systems. */
  if (pc.isgood_inode (nFileIndex))
    buf->st_ino = (__ino64_t) nFileIndex;
  else
    buf->st_ino = get_namehash ();

  buf->st_blksize = PREFERRED_IO_BLKSIZE;

  if (nAllocSize >= 0LL)
    /* A successful NtQueryInformationFile returns the allocation size
       correctly for compressed and sparse files as well. */
    buf->st_blocks = (nAllocSize + S_BLKSIZE - 1) / S_BLKSIZE;
  else if (::has_attribute (dwFileAttributes, FILE_ATTRIBUTE_COMPRESSED
					      | FILE_ATTRIBUTE_SPARSE_FILE)
	   && get_handle () && !is_fs_special ()
	   && !NtQueryInformationFile (get_handle (), &st, (PVOID) &fci,
				      sizeof fci, FileCompressionInformation))
    /* Otherwise we request the actual amount of bytes allocated for
       compressed and sparsed files. */
    buf->st_blocks = (fci.CompressedFileSize.QuadPart + S_BLKSIZE - 1)
		     / S_BLKSIZE;
  else
    /* Otherwise compute no. of blocks from file size. */
    buf->st_blocks  = (buf->st_size + S_BLKSIZE - 1) / S_BLKSIZE;

  buf->st_mode = 0;
  /* Using a side effect: get_file_attibutes checks for
     directory. This is used, to set S_ISVTX, if needed.  */
  if (pc.isdir ())
    buf->st_mode = S_IFDIR;
  else if (pc.issymlink ())
    {
      buf->st_size = pc.get_symlink_length ();
      /* symlinks are everything for everyone! */
      buf->st_mode = S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;
      get_file_attribute (get_handle (), pc, NULL,
			  &buf->st_uid, &buf->st_gid);
      goto done;
    }
  else if (pc.issocket ())
    buf->st_mode = S_IFSOCK;

  if (!get_file_attribute (is_fs_special () && !pc.issocket ()
			   ? NULL : get_handle (), pc,
			   &buf->st_mode, &buf->st_uid, &buf->st_gid))
    {
      /* If read-only attribute is set, modify ntsec return value */
      if (::has_attribute (dwFileAttributes, FILE_ATTRIBUTE_READONLY)
	  && !pc.isdir () && !pc.issymlink ())
	buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);

      if (buf->st_mode & S_IFMT)
	/* nothing */;
      else if (!is_fs_special ())
	buf->st_mode |= S_IFREG;
      else
	{
	  buf->st_dev = dev ();
	  buf->st_mode = dev ().mode;
	  buf->st_size = 0;
	}
    }
  else
    {
      buf->st_mode |= STD_RBITS;

      if (!::has_attribute (dwFileAttributes, FILE_ATTRIBUTE_READONLY))
	buf->st_mode |= STD_WBITS;
      /* | S_IWGRP | S_IWOTH; we don't give write to group etc */

      if (pc.isdir ())
	buf->st_mode |= S_IFDIR | STD_WBITS | STD_XBITS;
      else if (buf->st_mode & S_IFMT)
	/* nothing */;
      else if (is_fs_special ())
	{
	  buf->st_dev = dev ();
	  buf->st_mode = dev ().mode;
	  buf->st_size = 0;
	}
      else
	{
	  buf->st_mode |= S_IFREG;
	  if (pc.exec_state () == dont_know_if_executable)
	    {
	      UNICODE_STRING same;
	      OBJECT_ATTRIBUTES attr;
	      HANDLE h;
	      IO_STATUS_BLOCK io;

	      RtlInitUnicodeString (&same, L"");
	      InitializeObjectAttributes (&attr, &same, 0, get_handle (), NULL);
	      if (NT_SUCCESS (NtOpenFile (&h, FILE_READ_DATA, &attr, &io,
					  FILE_SHARE_VALID_FLAGS, 0)))
	        {
		  LARGE_INTEGER off = { QuadPart:0LL };
		  char magic[3];

		  if (NT_SUCCESS (NtReadFile (h, NULL, NULL, NULL, &io, magic,
					      3, &off, NULL))
		      && has_exec_chars (magic, io.Information))
		    {
		      pc.set_exec ();
		      buf->st_mode |= STD_XBITS;
		    }
		  NtClose (h);
		}
	    }
	}
      if (pc.exec_state () == is_executable)
	buf->st_mode |= STD_XBITS;

      /* This fakes the permissions of all files to match the current umask. */
      buf->st_mode &= ~(cygheap->umask);
    }

 done:
  syscall_printf ("0 = fstat (, %p) st_atime=%x st_size=%D, st_mode=%p, st_ino=%D, sizeof=%d",
		  buf, buf->st_atime, buf->st_size, buf->st_mode,
		  buf->st_ino, sizeof (*buf));
  return 0;
}

int __stdcall
fhandler_disk_file::fstat (struct __stat64 *buf)
{
  return fstat_fs (buf);
}

int __stdcall
fhandler_disk_file::fstatvfs (struct statvfs *sfs)
{
  int ret = -1, opened = 0;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  const size_t fvi_size = sizeof (FILE_FS_VOLUME_INFORMATION)
			  + (NAME_MAX + 1) * sizeof (WCHAR);
  PFILE_FS_VOLUME_INFORMATION pfvi = (PFILE_FS_VOLUME_INFORMATION)
				     alloca (fvi_size);
  const size_t fai_size = sizeof (FILE_FS_ATTRIBUTE_INFORMATION)
			  + (NAME_MAX + 1) * sizeof (WCHAR);
  PFILE_FS_ATTRIBUTE_INFORMATION pfai = (PFILE_FS_ATTRIBUTE_INFORMATION)
					alloca (fai_size);
  FILE_FS_FULL_SIZE_INFORMATION full_fsi;
  FILE_FS_SIZE_INFORMATION fsi;
  HANDLE fh = get_handle ();

  if (!fh)
    {
      OBJECT_ATTRIBUTES attr;
      opened = NT_SUCCESS (NtOpenFile (&fh, READ_CONTROL,
				     pc.get_object_attr (attr, sec_none_nih),
				     &io, FILE_SHARE_VALID_FLAGS,
				     FILE_OPEN_FOR_BACKUP_INTENT));
      if (!opened)
	{
	  /* Can't open file.  Try again with parent dir. */
	  UNICODE_STRING dirname;
	  RtlSplitUnicodePath (pc.get_nt_native_path (), &dirname, NULL);
	  attr.ObjectName = &dirname;
	  opened = NT_SUCCESS (NtOpenFile (&fh, READ_CONTROL, &attr, &io,
					 FILE_SHARE_VALID_FLAGS,
					 FILE_OPEN_FOR_BACKUP_INTENT));
	  if (!opened)
	    goto out;
	}
    }

  /* Get basic volume information. */
  status = NtQueryVolumeInformationFile (fh, &io, pfvi, fvi_size,
					 FileFsVolumeInformation);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      goto out;
    }
  status = NtQueryVolumeInformationFile (fh, &io, pfai, fai_size,
					 FileFsAttributeInformation);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      goto out;
    }
  sfs->f_files = ULONG_MAX;
  sfs->f_ffree = ULONG_MAX;
  sfs->f_favail = ULONG_MAX;
  sfs->f_fsid = pfvi->VolumeSerialNumber;
  sfs->f_flag = pfai->FileSystemAttributes;
  sfs->f_namemax = pfai->MaximumComponentNameLength;
  /* Get allocation related information.  Try to get "full" information
     first, which is only available since W2K.  If that fails, try to
     retrieve normal allocation information. */
  status = NtQueryVolumeInformationFile (fh, &io, &full_fsi, sizeof full_fsi,
					 FileFsFullSizeInformation);
  if (NT_SUCCESS (status))
    {
      sfs->f_bsize = full_fsi.BytesPerSector * full_fsi.SectorsPerAllocationUnit;
      sfs->f_frsize = sfs->f_bsize;
      sfs->f_blocks = full_fsi.TotalAllocationUnits.LowPart;
      sfs->f_bfree = full_fsi.ActualAvailableAllocationUnits.LowPart;
      sfs->f_bavail = full_fsi.CallerAvailableAllocationUnits.LowPart;
      if (sfs->f_bfree > sfs->f_bavail)
	{
	  /* Quotas active.  We can't trust TotalAllocationUnits. */
	  NTFS_VOLUME_DATA_BUFFER nvdb;

	  status = NtFsControlFile (fh, NULL, NULL, NULL, &io,
				    FSCTL_GET_NTFS_VOLUME_DATA,
				    NULL, 0, &nvdb, sizeof nvdb);
	  if (!NT_SUCCESS (status))
	    debug_printf ("%p = NtFsControlFile(%S, FSCTL_GET_NTFS_VOLUME_DATA)",
			  status, pc.get_nt_native_path ());
	  else
	    sfs->f_blocks = nvdb.TotalClusters.QuadPart;
	}
      ret = 0;
    }
  else
    {
      status = NtQueryVolumeInformationFile (fh, &io, &fsi, sizeof fsi,
					     FileFsSizeInformation);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  goto out;
	}
      sfs->f_bsize = fsi.BytesPerSector * fsi.SectorsPerAllocationUnit;
      sfs->f_frsize = sfs->f_bsize;
      sfs->f_blocks = fsi.TotalAllocationUnits.LowPart;
      sfs->f_bfree = fsi.AvailableAllocationUnits.LowPart;
      sfs->f_bavail = sfs->f_bfree;
      ret = 0;
    }
out:
  if (opened)
    NtClose (fh);
  syscall_printf ("%d = fstatvfs (%s, %p)", ret, get_name (), sfs);
  return ret;
}

int __stdcall
fhandler_disk_file::fchmod (mode_t mode)
{
  extern int chmod_device (path_conv& pc, mode_t mode);
  int res = -1;
  int oret = 0;

  if (pc.is_fs_special ())
    return chmod_device (pc, mode);

  if (!get_handle ())
    {
      query_open (query_write_control);
      if (!(oret = open (O_BINARY, 0)))
	{
	  /* Need WRITE_DAC|WRITE_OWNER to write ACLs. */
	  if (allow_ntsec && pc.has_acls ())
	    return -1;
	  /* Otherwise FILE_WRITE_ATTRIBUTES is sufficient. */
	  query_open (query_write_attributes);
	  if (!(oret = open (O_BINARY, 0)))
	    return -1;
	}
    }

  if (allow_ntsec && pc.has_acls ())
    {
      if (pc.isdir ())
	mode |= S_IFDIR;
      if (!set_file_attribute (get_handle (), pc,
			       ILLEGAL_UID, ILLEGAL_GID, mode)
	  && allow_ntsec)
	res = 0;
    }

  /* if the mode we want has any write bits set, we can't be read only. */
  if (mode & (S_IWUSR | S_IWGRP | S_IWOTH))
    pc &= (DWORD) ~FILE_ATTRIBUTE_READONLY;
  else
    pc |= (DWORD) FILE_ATTRIBUTE_READONLY;
  if (mode & S_IFSOCK)
    pc |= (DWORD) FILE_ATTRIBUTE_SYSTEM;

  IO_STATUS_BLOCK io;
  FILE_BASIC_INFORMATION fbi;
  fbi.CreationTime.QuadPart = fbi.LastAccessTime.QuadPart
  = fbi.LastWriteTime.QuadPart = fbi.ChangeTime.QuadPart = 0LL;
  fbi.FileAttributes = pc.file_attributes () ?: FILE_ATTRIBUTE_NORMAL;
  NTSTATUS status = NtSetInformationFile (get_handle (), &io, &fbi, sizeof fbi,
					  FileBasicInformation);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  else if (!allow_ntsec || !pc.has_acls ())
    /* Correct NTFS security attributes have higher priority */
    res = 0;

  if (oret)
    close ();

  return res;
}

int __stdcall
fhandler_disk_file::fchown (__uid32_t uid, __gid32_t gid)
{
  int oret = 0;

  if (!pc.has_acls () || !allow_ntsec)
    {
      /* fake - if not supported, pretend we're like win95
	 where it just works */
      return 0;
    }

  if (!get_handle ())
    {
      query_open (query_write_control);
      if (!(oret = fhandler_disk_file::open (O_BINARY, 0)))
	return -1;
    }

  mode_t attrib = 0;
  if (pc.isdir ())
    attrib |= S_IFDIR;
  int res = get_file_attribute (get_handle (), pc, &attrib, NULL, NULL);
  if (!res)
    {
      /* Typical Windows default ACLs can contain permissions for one
	 group, while being owned by another user/group.  The permission
	 bits returned above are pretty much useless then.  Creating a
	 new ACL with these useless permissions results in a potentially
	 broken symlink.  So what we do here is to set the underlying
	 permissions of symlinks to a sensible value which allows the
	 world to read the symlink and only the new owner to change it. */
      if (pc.issymlink ())
	attrib = S_IFLNK | STD_RBITS | STD_WBITS;
      res = set_file_attribute (get_handle (), pc, uid, gid, attrib);
    }
  if (oret)
    close ();

  return res;
}

int _stdcall
fhandler_disk_file::facl (int cmd, int nentries, __aclent32_t *aclbufp)
{
  int res = -1;
  int oret = 0;

  if (!pc.has_acls () || !allow_ntsec)
    {
cant_access_acl:
      switch (cmd)
	{
	  struct __stat64 st;

	  case SETACL:
	    /* Open for writing required to be able to set ctime
	       (even though setting the ACL is just pretended). */
	    if (!get_handle ())
	      oret = open (O_WRONLY | O_BINARY, 0);
	    res = 0;
	    break;
	  case GETACL:
	    if (!aclbufp)
	      set_errno (EFAULT);
	    else if (nentries < MIN_ACL_ENTRIES)
	      set_errno (ENOSPC);
	    else
	      {
		if (!get_handle ())
		  {
		    query_open (query_read_attributes);
		    oret = open (O_BINARY, 0);
		  }
		if ((!oret && !fstat_by_handle (&st))
		    || !fstat_by_name (&st))
		  {
		    aclbufp[0].a_type = USER_OBJ;
		    aclbufp[0].a_id = st.st_uid;
		    aclbufp[0].a_perm = (st.st_mode & S_IRWXU) >> 6;
		    aclbufp[1].a_type = GROUP_OBJ;
		    aclbufp[1].a_id = st.st_gid;
		    aclbufp[1].a_perm = (st.st_mode & S_IRWXG) >> 3;
		    aclbufp[2].a_type = OTHER_OBJ;
		    aclbufp[2].a_id = ILLEGAL_GID;
		    aclbufp[2].a_perm = st.st_mode & S_IRWXO;
		    aclbufp[3].a_type = CLASS_OBJ;
		    aclbufp[3].a_id = ILLEGAL_GID;
		    aclbufp[3].a_perm = S_IRWXU | S_IRWXG | S_IRWXO;
		    res = MIN_ACL_ENTRIES;
		  }
	      }
	    break;
	  case GETACLCNT:
	    res = MIN_ACL_ENTRIES;
	    break;
	  default:
	    set_errno (EINVAL);
	    break;
	}
    }
  else
    {
      if (!get_handle ())
	{
	  query_open (cmd == SETACL ? query_write_control : query_read_control);
	  if (!(oret = open (O_BINARY, 0)))
	    {
	      if (cmd == GETACL || cmd == GETACLCNT)
		goto cant_access_acl;
	      return -1;
	    }
	}
      switch (cmd)
	{
	  case SETACL:
	    if (!aclsort32 (nentries, 0, aclbufp))
	      {
		bool rw = false;
		res = setacl (get_handle (), pc, nentries, aclbufp, rw);
		if (rw)
		  {
		    IO_STATUS_BLOCK io;
		    FILE_BASIC_INFORMATION fbi;
		    fbi.CreationTime.QuadPart
		    = fbi.LastAccessTime.QuadPart
		    = fbi.LastWriteTime.QuadPart
		    = fbi.ChangeTime.QuadPart = 0LL;
		    fbi.FileAttributes = (pc.file_attributes ()
					  & ~FILE_ATTRIBUTE_READONLY)
					 ?: FILE_ATTRIBUTE_NORMAL;
		    NtSetInformationFile (get_handle (), &io, &fbi, sizeof fbi,
					  FileBasicInformation);
		  }
	      }
	    break;
	  case GETACL:
	    if (!aclbufp)
	      set_errno(EFAULT);
	    else
	      res = getacl (get_handle (), pc, nentries, aclbufp);
	    break;
	  case GETACLCNT:
	    res = getacl (get_handle (), pc, 0, NULL);
	    break;
	  default:
	    set_errno (EINVAL);
	    break;
	}
    }

  if (oret)
    close ();

  return res;
}

ssize_t
fhandler_disk_file::fgetxattr (const char *name, void *value, size_t size)
{
  if (pc.is_fs_special ())
    {
      set_errno (ENOTSUP);
      return -1;
    }
  return read_ea (get_handle (), pc, name, (char *) value, size);
}

int
fhandler_disk_file::fsetxattr (const char *name, const void *value, size_t size,
			       int flags)
{
  if (pc.is_fs_special ())
    {
      set_errno (ENOTSUP);
      return -1;
    }
  return write_ea (get_handle (), pc, name, (const char *) value, size, flags);
}

int
fhandler_disk_file::fadvise (_off64_t offset, _off64_t length, int advice)
{
  if (advice < POSIX_FADV_NORMAL || advice > POSIX_FADV_NOREUSE)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* Windows only supports advice flags for the whole file.  We're using
     a simplified test here so that we don't have to ask for the actual
     file size.  Length == 0 means all bytes starting at offset anyway.
     So we only actually follow the advice, if it's given for offset == 0. */
  if (offset != 0)
    return 0;

  /* We only support normal and sequential mode for now.  Everything which
     is not POSIX_FADV_SEQUENTIAL is treated like POSIX_FADV_NORMAL. */
  if (advice != POSIX_FADV_SEQUENTIAL)
    advice = POSIX_FADV_NORMAL;

  IO_STATUS_BLOCK io;
  FILE_MODE_INFORMATION fmi;
  NTSTATUS status = NtQueryInformationFile (get_handle (), &io,
					    &fmi, sizeof fmi,
					    FileModeInformation);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  else
    {
      fmi.Mode &= ~FILE_SEQUENTIAL_ONLY;
      if (advice == POSIX_FADV_SEQUENTIAL)
	fmi.Mode |= FILE_SEQUENTIAL_ONLY;
      status = NtSetInformationFile (get_handle (), &io, &fmi, sizeof fmi,
				     FileModeInformation);
      if (NT_SUCCESS (status))
	return 0;
      __seterrno_from_nt_status (status);
    }

  return -1;
}

int
fhandler_disk_file::ftruncate (_off64_t length, bool allow_truncate)
{
  int res = -1;

  if (length < 0 || !get_handle ())
    set_errno (EINVAL);
  else if (pc.isdir ())
    set_errno (EISDIR);
  else if (!(get_access () & GENERIC_WRITE))
    set_errno (EBADF);
  else
    {
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      FILE_STANDARD_INFORMATION fsi;
      FILE_END_OF_FILE_INFORMATION feofi;

      status = NtQueryInformationFile (get_handle (), &io, &fsi, sizeof fsi,
				       FileStandardInformation);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return -1;
	}

      /* If called through posix_fallocate, silently succeed if length
	 is less than the file's actual length. */
      if (!allow_truncate && length < fsi.EndOfFile.QuadPart)
	return 0;

      feofi.EndOfFile.QuadPart = length;
      /* Create sparse files only when called through ftruncate, not when
	 called through posix_fallocate. */
      if (allow_truncate
	  && get_fs_flags (FILE_SUPPORTS_SPARSE_FILES)
	  && length >= fsi.EndOfFile.QuadPart + (128 * 1024))
	{
	  status = NtFsControlFile (get_handle (), NULL, NULL, NULL, &io,
				    FSCTL_SET_SPARSE, NULL, 0, NULL, 0);
	  syscall_printf ("%p = NtFsControlFile(%S, FSCTL_SET_SPARSE)",
			  status, pc.get_nt_native_path ());
	}
      status = NtSetInformationFile (get_handle (), &io,
				     &feofi, sizeof feofi,
				     FileEndOfFileInformation);
      if (!NT_SUCCESS (status))
	__seterrno_from_nt_status (status);
      else
	res = 0;
    }
  return res;
}

int
fhandler_disk_file::link (const char *newpath)
{
  extern bool allow_winsymlinks;

  path_conv newpc (newpath, PC_SYM_NOFOLLOW | PC_POSIX,
		   transparent_exe ? stat_suffixes : NULL);
  if (newpc.error)
    {
      set_errno (newpc.case_clash ? ECASECLASH : newpc.error);
      return -1;
    }

  if (newpc.exists ())
    {
      syscall_printf ("file '%S' exists?", newpc.get_nt_native_path ());
      set_errno (EEXIST);
      return -1;
    }

  char new_buf[strlen (newpath) + 5];
  if (!newpc.error && !newpc.case_clash)
    {
      if (allow_winsymlinks && pc.is_lnk_special ())
	{
	  /* Shortcut hack. */
	  stpcpy (stpcpy (new_buf, newpath), ".lnk");
	  newpath = new_buf;
	  newpc.check (newpath, PC_SYM_NOFOLLOW);
	}
      else if (!pc.isdir ()
	       && pc.is_binary ()
	       && !RtlEqualUnicodePathSuffix (newpc.get_nt_native_path (),
					      L".exe", TRUE))
	{
	  /* Executable hack. */
	  stpcpy (stpcpy (new_buf, newpath), ".exe");
	  newpath = new_buf;
	  newpc.check (newpath, PC_SYM_NOFOLLOW);
	}
    }

  HANDLE fh;
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  status = NtOpenFile (&fh, READ_CONTROL,
		       pc.get_object_attr (attr, sec_none_nih), &io,
		       FILE_SHARE_VALID_FLAGS,
		       FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  PUNICODE_STRING tgt = newpc.get_nt_native_path ();
  ULONG size = sizeof (FILE_LINK_INFORMATION) + tgt->Length;
  PFILE_LINK_INFORMATION pfli = (PFILE_LINK_INFORMATION) alloca (size);
  pfli->ReplaceIfExists = FALSE;
  pfli->RootDirectory = NULL;
  memcpy (pfli->FileName, tgt->Buffer, pfli->FileNameLength = tgt->Length);
  status = NtSetInformationFile (fh, &io, pfli, size, FileLinkInformation);
  NtClose (fh);
  if (!NT_SUCCESS (status))
    {
      if (status == STATUS_INVALID_DEVICE_REQUEST)
        {
	  /* FS doesn't support hard links.  Try to copy file. */
	  WCHAR pcw[pc.get_nt_native_path ()->Length + 1];
	  WCHAR newpcw[newpc.get_nt_native_path ()->Length + 1];
	  if (!CopyFileW (pc.get_wide_win32_path (pcw),
			  newpc.get_wide_win32_path (newpcw), TRUE))
	    {
	      __seterrno ();
	      return -1;
	    }
	  if (!allow_winsymlinks && pc.is_lnk_special ())
	    SetFileAttributesW (newpcw, pc.file_attributes ()
					| FILE_ATTRIBUTE_SYSTEM
					| FILE_ATTRIBUTE_READONLY);
	}
      else
        {
	  __seterrno_from_nt_status (status);
	  return -1;
	}
    }
  return 0;
}

int
fhandler_disk_file::utimes (const struct timeval *tvp)
{
  return utimes_fs (tvp);
}

int
fhandler_base::utimes_fs (const struct timeval *tvp)
{
  LARGE_INTEGER lastaccess, lastwrite;
  struct timeval tmp[2];
  bool closeit = false;

  if (!get_handle ())
    {
      query_open (query_write_attributes);
      if (!open_fs (O_BINARY, 0))
	{
	  /* It's documented in MSDN that FILE_WRITE_ATTRIBUTES is sufficient
	     to change the timestamps.  Unfortunately it's not sufficient for a
	     remote HPFS which requires GENERIC_WRITE, so we just retry to open
	     for writing, though this fails for R/O files of course. */
	  query_open (no_query);
	  if (!open_fs (O_WRONLY | O_BINARY, 0))
	    {
	      syscall_printf ("Opening file failed");
	      return -1;
	    }
	}
      closeit = true;
    }

  gettimeofday (&tmp[0], 0);
  if (!tvp)
    {
      tmp[1] = tmp[0];
      tvp = tmp;
    }
  timeval_to_filetime (&tvp[0], (FILETIME *) &lastaccess);
  timeval_to_filetime (&tvp[1], (FILETIME *) &lastwrite);
  debug_printf ("incoming lastaccess %08x %08x", tvp[0].tv_sec, tvp[0].tv_usec);

  IO_STATUS_BLOCK io;
  FILE_BASIC_INFORMATION fbi;
  fbi.CreationTime.QuadPart = 0LL;
  fbi.LastAccessTime = lastaccess;
  fbi.LastWriteTime = lastwrite;
  fbi.ChangeTime.QuadPart = 0LL;
  fbi.FileAttributes = 0;
  NTSTATUS status = NtSetInformationFile (get_handle (), &io, &fbi, sizeof fbi,
					  FileBasicInformation);
  if (closeit)
    close_fs ();
  /* Opening a directory on a 9x share from a NT machine works(!), but
     then NtSetInformationFile fails with STATUS_NOT_SUPPORTED.  Oh well... */
  if (!NT_SUCCESS (status) && status != STATUS_NOT_SUPPORTED)
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  return 0;
}

fhandler_disk_file::fhandler_disk_file () :
  fhandler_base ()
{
}

fhandler_disk_file::fhandler_disk_file (path_conv &pc) :
  fhandler_base ()
{
  set_name (pc);
}

int
fhandler_disk_file::open (int flags, mode_t mode)
{
  return open_fs (flags, mode);
}

int
fhandler_base::open_fs (int flags, mode_t mode)
{
  if (pc.case_clash && flags & O_CREAT)
    {
      debug_printf ("case clash detected");
      set_errno (ECASECLASH);
      return 0;
    }

  /* Unfortunately NT allows to open directories for writing, but that's
     disallowed according to SUSv3. */
  if (pc.isdir () && (flags & O_ACCMODE) != O_RDONLY)
    {
      set_errno (EISDIR);
      return 0;
    }

  int res = fhandler_base::open (flags | O_DIROPEN, mode);
  if (!res)
    goto out;

  /* This is for file systems known for having a buggy CreateFile call
     which might return a valid HANDLE without having actually opened
     the file.
     The only known file system to date is the SUN NFS Solstice Client 3.1
     which returns a valid handle when trying to open a file in a nonexistent
     directory. */
  if (pc.has_buggy_open () && !pc.exists ())
    {
      debug_printf ("Buggy open detected.");
      close_fs ();
      set_errno (ENOENT);
      return 0;
    }

  set_fs_flags (pc.fs_flags ());

out:
  syscall_printf ("%d = fhandler_disk_file::open (%S, %p)", res,
		  pc.get_nt_native_path (), flags);
  return res;
}

ssize_t __stdcall
fhandler_disk_file::pread (void *buf, size_t count, _off64_t offset)
{
  ssize_t res;
  _off64_t curpos = lseek (0, SEEK_CUR);
  if (curpos < 0 || lseek (offset, SEEK_SET) < 0)
    res = -1;
  else
    {
      size_t tmp_count = count;
      read (buf, tmp_count);
      if (lseek (curpos, SEEK_SET) >= 0)
	res = (ssize_t) tmp_count;
      else
	res = -1;
    }
  debug_printf ("%d = pread (%p, %d, %d)\n", res, buf, count, offset);
  return res;
}

ssize_t __stdcall
fhandler_disk_file::pwrite (void *buf, size_t count, _off64_t offset)
{
  int res;
  _off64_t curpos = lseek (0, SEEK_CUR);
  if (curpos < 0 || lseek (offset, SEEK_SET) < 0)
    res = curpos;
  else
    {
      res = (ssize_t) write (buf, count);
      if (lseek (curpos, SEEK_SET) < 0)
	res = -1;
    }
  debug_printf ("%d = pwrite (%p, %d, %d)\n", res, buf, count, offset);
  return res;
}

/* FIXME: The correct way to do this to get POSIX locking semantics is to
   keep a linked list of posix lock requests and map them into Win32 locks.
   he problem is that Win32 does not deal correctly with overlapping lock
   requests. */

int
fhandler_disk_file::lock (int cmd, struct __flock64 *fl)
{
  _off64_t win32_start;
  _off64_t win32_len;
  _off64_t startpos;

  /*
   * We don't do getlck calls yet.
   */

  if (cmd == F_GETLK)
    {
      set_errno (ENOSYS);
      return -1;
    }

  /*
   * Calculate where in the file to start from,
   * then adjust this by fl->l_start.
   */

  switch (fl->l_whence)
    {
      case SEEK_SET:
	startpos = 0;
	break;
      case SEEK_CUR:
	if ((startpos = lseek (0, SEEK_CUR)) == ILLEGAL_SEEK)
	  return -1;
	break;
      case SEEK_END:
	{
	  BY_HANDLE_FILE_INFORMATION finfo;
	  if (GetFileInformationByHandle (get_handle (), &finfo) == 0)
	    {
	      __seterrno ();
	      return -1;
	    }
	  startpos = ((_off64_t)finfo.nFileSizeHigh << 32)
		     + finfo.nFileSizeLow;
	  break;
	}
      default:
	set_errno (EINVAL);
	return -1;
    }

  /*
   * Now the fun starts. Adjust the start and length
   *  fields until they make sense.
   */

  win32_start = startpos + fl->l_start;
  if (fl->l_len < 0)
    {
      win32_start -= fl->l_len;
      win32_len = -fl->l_len;
    }
  else
    win32_len = fl->l_len;

  if (win32_start < 0)
    {
      /* watch the signs! */
      win32_len -= -win32_start;
      if (win32_len <= 0)
	{
	  /* Failure ! */
	  set_errno (EINVAL);
	  return -1;
	}
      win32_start = 0;
    }

  DWORD off_high, off_low, len_high, len_low;

  off_low = (DWORD)(win32_start & UINT32_MAX);
  off_high = (DWORD)(win32_start >> 32);
  if (win32_len == 0)
    {
      /* Special case if len == 0 for POSIX means lock to the end of
	 the entire file (and all future extensions).  */
      len_low = len_high = UINT32_MAX;
    }
  else
    {
      len_low = (DWORD)(win32_len & UINT32_MAX);
      len_high = (DWORD)(win32_len >> 32);
    }

  BOOL res;

  DWORD lock_flags = (cmd == F_SETLK) ? LOCKFILE_FAIL_IMMEDIATELY : 0;
  lock_flags |= (fl->l_type == F_WRLCK) ? LOCKFILE_EXCLUSIVE_LOCK : 0;

  OVERLAPPED ov;

  ov.Internal = 0;
  ov.InternalHigh = 0;
  ov.Offset = off_low;
  ov.OffsetHigh = off_high;
  ov.hEvent = (HANDLE) 0;

  if (fl->l_type == F_UNLCK)
    {
      res = UnlockFileEx (get_handle (), 0, len_low, len_high, &ov);
      if (res == 0 && GetLastError () == ERROR_NOT_LOCKED)
	res = 1;
    }
  else
    {
      res = LockFileEx (get_handle (), lock_flags, 0,
			len_low, len_high, &ov);
      /* Deal with the fail immediately case. */
      /*
       * FIXME !! I think this is the right error to check for
       * but I must admit I haven't checked....
       */
      if ((res == 0) && (lock_flags & LOCKFILE_FAIL_IMMEDIATELY) &&
			(GetLastError () == ERROR_LOCK_FAILED))
	{
	  set_errno (EAGAIN);
	  return -1;
	}
    }

  if (res == 0)
    {
      __seterrno ();
      return -1;
    }

  return 0;
}

int
fhandler_disk_file::mkdir (mode_t mode)
{
  int res = -1;
  SECURITY_ATTRIBUTES sa = sec_none_nih;
  security_descriptor sd;

  if (allow_ntsec && has_acls ())
    set_security_attribute (S_IFDIR | ((mode & 07777) & ~cygheap->umask),
			    &sa, sd);

  NTSTATUS status;
  HANDLE dir;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG fattr = FILE_ATTRIBUTE_DIRECTORY;
  status = NtCreateFile (&dir, FILE_LIST_DIRECTORY | SYNCHRONIZE,
			 pc.get_object_attr (attr, sa), &io, NULL,
			 fattr, FILE_SHARE_VALID_FLAGS, FILE_CREATE,
			 FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
			 | FILE_OPEN_FOR_BACKUP_INTENT,
			 NULL, 0);
  if (NT_SUCCESS (status))
    {
      NtClose (dir);
      res = 0;
    }
  else
    __seterrno_from_nt_status (status);

  return res;
}

int
fhandler_disk_file::rmdir ()
{
  extern NTSTATUS unlink_nt (path_conv &pc);

  if (!pc.isdir ())
    {
      set_errno (ENOTDIR);
      return -1;
    }
  if (!pc.exists ())
    {
      set_errno (ENOENT);
      return -1;
    }

  NTSTATUS status = unlink_nt (pc);

  /* Check for existence of remote dirs after trying to delete them.
     Two reasons:
     - Sometimes SMB indicates failure when it really succeeds.
     - Removeing a directory on a samba drive doesn't return an error if the
       directory can't be removed because it's not empty.  */
  if (isremote ())
    {
      OBJECT_ATTRIBUTES attr;
      FILE_BASIC_INFORMATION fbi;

      if (NT_SUCCESS (NtQueryAttributesFile
			    (pc.get_object_attr (attr, sec_none_nih), &fbi)))
	status = STATUS_DIRECTORY_NOT_EMPTY;
      else
        status = STATUS_SUCCESS;
    }
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  return 0;
}

/* This is the minimal number of entries which fit into the readdir cache.
   The number of bytes allocated by the cache is determined by this number,
   To tune caching, just tweak this number.  To get a feeling for the size,
   the size of the readdir cache is DIR_NUM_ENTRIES * 624 + 4 bytes.  */

#define DIR_NUM_ENTRIES	100		/* Cache size 62404 bytes */

#define DIR_BUF_SIZE	(DIR_NUM_ENTRIES \
			 * (sizeof (FILE_ID_BOTH_DIR_INFORMATION) \
			    + 2 * (NAME_MAX + 1)))

struct __DIR_cache
{
  ULONG __pos;
  char  __cache[DIR_BUF_SIZE];
};

#define d_cachepos(d)	(((__DIR_cache *) (d)->__d_dirname)->__pos)
#define d_cache(d)	(((__DIR_cache *) (d)->__d_dirname)->__cache)

#define d_mounts(d)	((__DIR_mounts *) (d)->__d_internal)

DIR *
fhandler_disk_file::opendir (int fd)
{
  DIR *dir;
  DIR *res = NULL;

  if (!pc.isdir ())
    set_errno (ENOTDIR);
  else if ((dir = (DIR *) malloc (sizeof (DIR))) == NULL)
    set_errno (ENOMEM);
  else if ((dir->__d_dirname = (char *) malloc ( sizeof (struct __DIR_cache)))
	   == NULL)
    {
      set_errno (ENOMEM);
      goto free_dir;
    }
  else if ((dir->__d_dirent =
	    (struct dirent *) malloc (sizeof (struct dirent))) == NULL)
    {
      set_errno (ENOMEM);
      goto free_dirname;
    }
  else
    {
      cygheap_fdnew cfd;
      if (cfd < 0 && fd < 0)
	goto free_dirent;

      dir->__d_dirent->__d_version = __DIRENT_VERSION;
      dir->__d_cookie = __DIRENT_COOKIE;
      dir->__handle = INVALID_HANDLE_VALUE;
      dir->__d_position = 0;
      dir->__flags = (get_name ()[0] == '/' && get_name ()[1] == '\0')
		     ? dirent_isroot : 0;
      dir->__d_internal = (unsigned) new __DIR_mounts (get_name ());
      d_cachepos (dir) = 0;

      if (!pc.iscygdrive ())
	{
	  if (fd < 0)
	    {
	      /* opendir() case.  Initialize with given directory name and
		 NULL directory handle. */
	      OBJECT_ATTRIBUTES attr;
	      NTSTATUS status;
	      IO_STATUS_BLOCK io;

	      status = NtOpenFile (&get_handle (),
				   SYNCHRONIZE | FILE_LIST_DIRECTORY,
				   pc.get_object_attr (attr, sec_none_nih),
				   &io, FILE_SHARE_VALID_FLAGS,
				   FILE_SYNCHRONOUS_IO_NONALERT
				   | FILE_OPEN_FOR_BACKUP_INTENT
				   | FILE_DIRECTORY_FILE);
	      if (!NT_SUCCESS (status))
		{
		  __seterrno_from_nt_status (status);
		  goto free_mounts;
		}
	    }

	  /* FileIdBothDirectoryInformation is apparently unsupported on
	     XP when accessing directories on UDF.  When trying to use it
	     so, NtQueryDirectoryFile returns with STATUS_ACCESS_VIOLATION.
	     It's not clear if the call isn't also unsupported on other
	     OS/FS combinations (say, Win2K/CDFS or so).  Instead of
	     testing in readdir for yet another error code, let's use
	     FileIdBothDirectoryInformation only on filesystems supporting
	     persistent ACLs, FileBothDirectoryInformation otherwise. */
	  if (pc.hasgood_inode ())
	    {
	      dir->__flags |= dirent_set_d_ino;
	      if (wincap.has_fileid_dirinfo ())
		dir->__flags |= dirent_get_d_ino;
	    }
	}
      if (fd >= 0)
	dir->__d_fd = fd;
      else
	{
	  /* Filling cfd with `this' (aka storing this in the file
	     descriptor table should only happen after it's clear that
	     opendir doesn't fail, otherwise we end up cfree'ing the
	     fhandler twice, once in opendir() in dir.cc, the second
	     time on exit.  Nasty, nasty... */
	  cfd = this;
	  dir->__d_fd = cfd;
	  if (pc.iscygdrive ())
	    cfd->nohandle (true);
	}
      set_close_on_exec (true);
      dir->__fh = this;
      res = dir;
    }

  syscall_printf ("%p = opendir (%s)", res, get_name ());
  return res;

free_mounts:
  delete d_mounts (dir);
free_dirent:
  free (dir->__d_dirent);
free_dirname:
  free (dir->__d_dirname);
free_dir:
  free (dir);
  return res;
}

static inline __ino64_t
readdir_get_ino_by_handle (HANDLE hdl)
{
  IO_STATUS_BLOCK io;
  FILE_INTERNAL_INFORMATION pfai;

  if (!NtQueryInformationFile (hdl, &io, &pfai, sizeof pfai,
			       FileInternalInformation))
    return pfai.FileId.QuadPart;
  return 0;
}

__ino64_t __stdcall
readdir_get_ino (const char *path, bool dot_dot)
{
  char *fname;
  struct __stat64 st;
  HANDLE hdl;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  __ino64_t ino = 0;

  if (dot_dot)
    {
      fname = (char *) alloca (strlen (path) + 4);
      char *c = stpcpy (fname, path);
      if (c[-1] != '/')
        *c++ = '/';
      strcpy (c, "..");
      path = fname;
    }
  path_conv pc (path, PC_SYM_NOFOLLOW | PC_POSIX);
  if (pc.isspecial ())
    {
      if (!stat_worker (pc, &st))
	ino = st.st_ino;
    }
  else if (!pc.hasgood_inode ())
    ino = hash_path_name (0, pc.get_nt_native_path ());
  else if (NT_SUCCESS (NtOpenFile (&hdl, READ_CONTROL,
				   pc.get_object_attr (attr, sec_none_nih),
				   &io, FILE_SHARE_VALID_FLAGS,
				   FILE_OPEN_FOR_BACKUP_INTENT
				   | (pc.is_rep_symlink ()
				      ? FILE_OPEN_REPARSE_POINT : 0))))
    {
      ino = readdir_get_ino_by_handle (hdl);
      NtClose (hdl);
    }
  return ino;
}

int
fhandler_disk_file::readdir_helper (DIR *dir, dirent *de, DWORD w32_err,
				    DWORD attr, PUNICODE_STRING fname)
{
  if (w32_err)
    {
      bool added = false;
      if ((de->d_ino = d_mounts (dir)->check_missing_mount (fname)))
	added = true;
      if (!added)
	return geterrno_from_win_error (w32_err);

      attr = 0;
      dir->__flags &= ~dirent_set_d_ino;
    }

  /* Check for directory reparse point.  These are potential volume mount
     points which have another inode than the underlying directory. */
  if ((attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
      == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
    {
      HANDLE reph;
      OBJECT_ATTRIBUTES attr;
      IO_STATUS_BLOCK io;

      InitializeObjectAttributes (&attr, fname, OBJ_CASE_INSENSITIVE,
				  get_handle (), NULL);
      if (is_volume_mountpoint (&attr)
	  && (NT_SUCCESS (NtOpenFile (&reph, READ_CONTROL, &attr, &io,
				      FILE_SHARE_VALID_FLAGS,
				      FILE_OPEN_FOR_BACKUP_INTENT))))
	{
	  de->d_ino = readdir_get_ino_by_handle (reph);
	  NtClose (reph);
	}
    }

  /* Check for Windows shortcut. If it's a Cygwin or U/WIN
     symlink, drop the .lnk suffix. */
  if ((attr & FILE_ATTRIBUTE_READONLY) && fname->Length > 4 * sizeof (WCHAR))
    {
      UNICODE_STRING uname;
      UNICODE_STRING lname;

      RtlInitCountedUnicodeString (&uname,
				   fname->Buffer
				   + fname->Length / sizeof (WCHAR) - 4,
				   4 * sizeof (WCHAR));
      RtlInitCountedUnicodeString (&lname, (PWCHAR) L".lnk",
				   4 * sizeof (WCHAR));

      if (RtlEqualUnicodeString (&uname, &lname, TRUE))
	{
	  UNICODE_STRING dirname = *pc.get_nt_native_path ();
	  dirname.Buffer += 4; /* Skip leading \??\ */
	  dirname.Length -= 4 * sizeof (WCHAR);
	  UNICODE_STRING fbuf;
	  ULONG len = dirname.Length + fname->Length + 2 * sizeof (WCHAR);

	  RtlInitEmptyUnicodeString (&fbuf, (PCWSTR) alloca (len), len);
	  RtlCopyUnicodeString (&fbuf, &dirname);
	  RtlAppendUnicodeToString (&fbuf, L"\\");
	  RtlAppendUnicodeStringToString (&fbuf, fname);
	  path_conv fpath (&fbuf, PC_SYM_NOFOLLOW);
	  if (fpath.issymlink () || fpath.is_fs_special ())
	    fname->Length -= 4 * sizeof (WCHAR);
	}
    }

  char tmp[NAME_MAX + 1];
  sys_wcstombs (tmp, NAME_MAX, fname->Buffer, fname->Length / sizeof (WCHAR)); 
  if (pc.isencoded ())
    fnunmunge (de->d_name, tmp);
  else
    strcpy (de->d_name, tmp);
  if (dir->__d_position == 0 && !strcmp (tmp, "."))
    dir->__flags |= dirent_saw_dot;
  else if (dir->__d_position == 1 && !strcmp (tmp, ".."))
    dir->__flags |= dirent_saw_dot_dot;
  return 0;
}

int
fhandler_disk_file::readdir (DIR *dir, dirent *de)
{
  int res = 0;
  NTSTATUS status = STATUS_SUCCESS;
  PFILE_ID_BOTH_DIR_INFORMATION buf = NULL;
  PWCHAR FileName;
  IO_STATUS_BLOCK io;
  UNICODE_STRING fname;

  /* d_cachepos always refers to the next cache entry to use.  If it's 0
     we must reload the cache. */
  if (d_cachepos (dir) == 0)
    {
      if ((dir->__flags & dirent_get_d_ino))
	{
	  status = NtQueryDirectoryFile (get_handle (), NULL, NULL, 0, &io,
					 d_cache (dir), DIR_BUF_SIZE,
					 FileIdBothDirectoryInformation,
					 FALSE, NULL, dir->__d_position == 0);
	  /* FileIdBothDirectoryInformation isn't supported for remote drives
	     on NT4 and 2K systems, and it's also not supported on 2K at all,
	     when accessing network drives on any remote OS.  We just fall
	     back to using a standard directory query in this case and note
	     this case using the dirent_get_d_ino flag. */
	  if (status == STATUS_INVALID_LEVEL
	      || status == STATUS_INVALID_PARAMETER
	      || status == STATUS_INVALID_INFO_CLASS)
	    dir->__flags &= ~dirent_get_d_ino;
	  /* Something weird happens on Samba up to version 3.0.21c, which is
	     fixed in 3.0.22.  FileIdBothDirectoryInformation seems to work
	     nicely, but only up to the 128th entry in the directory.  After
	     reaching this entry, the next call to NtQueryDirectoryFile
	     (FileIdBothDirectoryInformation) returns STATUS_INVALID_LEVEL.
	     Why should we care, we can just switch to
	     FileBothDirectoryInformation, isn't it?  Nope!  The next call to
	     NtQueryDirectoryFile(FileBothDirectoryInformation) actually
	     returns STATUS_NO_MORE_FILES, regardless how many files are left
	     unread in the directory.  This does not happen when using
	     FileBothDirectoryInformation right from the start, but since
	     we can't decide whether the server we're talking with has this
	     bug or not, we end up serving Samba shares always in the slow
	     mode using FileBothDirectoryInformation.  So, what we do here is
	     to implement the solution suggested by Andrew Tridgell,  we just
	     reread all entries up to dir->d_position using
	     FileBothDirectoryInformation.
	     However, We do *not* mark this server as broken and fall back to
	     using FileBothDirectoryInformation further on.  This would slow
	     down every access to such a server, even for directories under
	     128 entries.  Also, bigger dirs only suffer from one additional
	     call per full directory scan, which shouldn't be too big a hit.
	     This can easily be changed if necessary. */
	  if (status == STATUS_INVALID_LEVEL && dir->__d_position)
	    {
	      d_cachepos (dir) = 0;
	      for (int cnt = 0; cnt < dir->__d_position; ++cnt)
		{
		  if (d_cachepos (dir) == 0)
		    {
		      status = NtQueryDirectoryFile (get_handle (), NULL, NULL,
					   0, &io, d_cache (dir), DIR_BUF_SIZE,
					   FileBothDirectoryInformation,
					   FALSE, NULL, cnt == 0);
		      if (!NT_SUCCESS (status))
			goto go_ahead;
		    }
		  buf = (PFILE_ID_BOTH_DIR_INFORMATION) (d_cache (dir)
							 + d_cachepos (dir));
		  if (buf->NextEntryOffset == 0)
		    d_cachepos (dir) = 0;
		  else
		    d_cachepos (dir) += buf->NextEntryOffset;
		}
	      goto go_ahead;
	    }
	}
      if (!(dir->__flags & dirent_get_d_ino))
	status = NtQueryDirectoryFile (get_handle (), NULL, NULL, 0, &io,
				       d_cache (dir), DIR_BUF_SIZE,
				       FileBothDirectoryInformation,
				       FALSE, NULL, dir->__d_position == 0);
    }

go_ahead:

  if (!NT_SUCCESS (status))
    debug_printf ("NtQueryDirectoryFile failed, status %p, win32 error %lu",
		  status, RtlNtStatusToDosError (status));
  else
    {
      buf = (PFILE_ID_BOTH_DIR_INFORMATION) (d_cache (dir) + d_cachepos (dir));
      if (buf->NextEntryOffset == 0)
	d_cachepos (dir) = 0;
      else
	d_cachepos (dir) += buf->NextEntryOffset;
      if ((dir->__flags & dirent_get_d_ino))
	{
	  FileName = buf->FileName;
	  if ((dir->__flags & dirent_set_d_ino))
	    de->d_ino = buf->FileId.QuadPart;
	}
      else
	FileName = ((PFILE_BOTH_DIR_INFORMATION) buf)->FileName;
      RtlInitCountedUnicodeString (&fname, FileName, buf->FileNameLength);
      de->d_ino = d_mounts (dir)->check_mount (&fname, de->d_ino);
      if (de->d_ino == 0 && (dir->__flags & dirent_set_d_ino))
	{
	  OBJECT_ATTRIBUTES attr;

	  if (dir->__d_position == 0 && buf->FileNameLength == 2
	      && FileName[0] == '.')
	    de->d_ino = readdir_get_ino_by_handle (get_handle ());
	  else if (dir->__d_position == 1 && buf->FileNameLength == 4
		   && FileName[0] == L'.' && FileName[1] == L'.')
	    if (!(dir->__flags & dirent_isroot))
	      de->d_ino = readdir_get_ino (get_name (), true);
	    else
	      de->d_ino = readdir_get_ino_by_handle (get_handle ());
	  else
	    {
	      HANDLE hdl;

	      InitializeObjectAttributes (&attr, &fname, OBJ_CASE_INSENSITIVE,
					  get_handle (), NULL);
	      if (NT_SUCCESS (NtOpenFile (&hdl, READ_CONTROL, &attr, &io,
					  FILE_SHARE_VALID_FLAGS,
					  FILE_OPEN_FOR_BACKUP_INTENT)))
		{
		  de->d_ino = readdir_get_ino_by_handle (hdl);
		  NtClose (hdl);
		}
	    }
	  /* Enforce namehash as inode number on untrusted file systems. */
	  if (!pc.isgood_inode (de->d_ino))
	    {
	      dir->__flags &= ~dirent_set_d_ino;
	      de->d_ino = 0;
	    }
	}
    }

  if (!(res = readdir_helper (dir, de, RtlNtStatusToDosError (status),
			      buf ? buf->FileAttributes : 0, &fname)))
    dir->__d_position++;
  else if (!(dir->__flags & dirent_saw_dot))
    {
      strcpy (de->d_name , ".");
      de->d_ino = readdir_get_ino_by_handle (get_handle ());
      dir->__d_position++;
      dir->__flags |= dirent_saw_dot;
      res = 0;
    }
  else if (!(dir->__flags & dirent_saw_dot_dot))
    {
      strcpy (de->d_name , "..");
      if (!(dir->__flags & dirent_isroot))
	de->d_ino = readdir_get_ino (get_name (), true);
      else
	de->d_ino = readdir_get_ino_by_handle (get_handle ());
      dir->__d_position++;
      dir->__flags |= dirent_saw_dot_dot;
      res = 0;
    }

  syscall_printf ("%d = readdir (%p, %p) (%s)", res, dir, &de, res ? "***" : de->d_name);
  return res;
}

_off64_t
fhandler_disk_file::telldir (DIR *dir)
{
  return dir->__d_position;
}

void
fhandler_disk_file::seekdir (DIR *dir, _off64_t loc)
{
  rewinddir (dir);
  while (loc > dir->__d_position)
    if (!::readdir (dir))
      break;
}

void
fhandler_disk_file::rewinddir (DIR *dir)
{
  d_cachepos (dir) = 0;
  if (wincap.has_buggy_restart_scan () && isremote ())
    {
      /* This works around a W2K bug.  The RestartScan parameter in calls
	 to NtQueryDirectoryFile on remote shares is ignored, thus
	 resulting in not being able to rewind on remote shares.  By
	 reopening the directory, we get a fresh new directory pointer. */
      UNICODE_STRING fname;
      OBJECT_ATTRIBUTES attr;
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      HANDLE new_dir;

      RtlInitUnicodeString (&fname, L"");
      InitializeObjectAttributes (&attr, &fname, OBJ_CASE_INSENSITIVE,
				  get_handle (), NULL);
      status = NtOpenFile (&new_dir, SYNCHRONIZE | FILE_LIST_DIRECTORY,
			   &attr, &io, FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT
			   | FILE_DIRECTORY_FILE);
      if (!NT_SUCCESS (stat))
	debug_printf ("Unable to reopen dir %s, NT error: %p",
		      get_name (), status);
      else
	{
	  NtClose (get_handle ());
	  set_io_handle (new_dir);
	}
    }
  dir->__d_position = 0;
  d_mounts (dir)->rewind ();
}

int
fhandler_disk_file::closedir (DIR *dir)
{
  int res = 0;
  NTSTATUS status;

  delete d_mounts (dir);
  if (!get_handle ())
    /* ignore */;
  else if (get_handle () == INVALID_HANDLE_VALUE)
    {
      set_errno (EBADF);
      res = -1;
    }
  else if (!NT_SUCCESS (status = NtClose (get_handle ())))
    {
      __seterrno_from_nt_status (status);
      res = -1;
    }
  syscall_printf ("%d = closedir (%p, %s)", res, dir, get_name ());
  return res;
}

fhandler_cygdrive::fhandler_cygdrive () :
  fhandler_disk_file (), ndrives (0), pdrive (NULL)
{
}

int
fhandler_cygdrive::open (int flags, mode_t mode)
{
  if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    {
      set_errno (EEXIST);
      return 0;
    }
  if (flags & O_WRONLY)
    {
      set_errno (EISDIR);
      return 0;
    }
  flags |= O_DIROPEN;
  set_flags (flags);
  nohandle (true);
  return 1;
}

int
fhandler_cygdrive::close ()
{
  return 0;
}

#define DRVSZ sizeof ("x:\\")
void
fhandler_cygdrive::set_drives ()
{
  const int len = 2 + 26 * DRVSZ;
  char *p = const_cast<char *> (get_win32_name ());
  pdrive = p;
  ndrives = GetLogicalDriveStrings (len, p) / DRVSZ;
}

int
fhandler_cygdrive::fstat (struct __stat64 *buf)
{
  fhandler_base::fstat (buf);
  buf->st_ino = 2;
  buf->st_mode = S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
  if (!ndrives)
    set_drives ();
  char flptst[] = "X:";
  int n = ndrives;
  for (const char *p = pdrive; p && *p; p = strchr (p, '\0') + 1)
    if (is_floppy ((flptst[0] = *p, flptst))
	|| GetFileAttributes (p) == INVALID_FILE_ATTRIBUTES)
      --n;
  buf->st_nlink = n + 2;
  return 0;
}

DIR *
fhandler_cygdrive::opendir (int fd)
{
  DIR *dir;

  dir = fhandler_disk_file::opendir (fd);
  if (dir && !ndrives)
    set_drives ();

  return dir;
}

int
fhandler_cygdrive::readdir (DIR *dir, dirent *de)
{
  char flptst[] = "X:";

  while (true)
    {
      if (!pdrive || !*pdrive)
	{
	  if (!(dir->__flags & dirent_saw_dot))
	    {
	      de->d_name[0] = '.';
	      de->d_name[1] = '\0';
	      de->d_ino = 2;
	    }
	  return ENMFILE;
	}
      if (!is_floppy ((flptst[0] = *pdrive, flptst))
	  && GetFileAttributes (pdrive) != INVALID_FILE_ATTRIBUTES)
	break;
      pdrive = strchr (pdrive, '\0') + 1;
    }
  *de->d_name = cyg_tolower (*pdrive);
  de->d_name[1] = '\0';
  user_shared->warned_msdos = true;
  de->d_ino = readdir_get_ino (pdrive, false);
  dir->__d_position++;
  pdrive = strchr (pdrive, '\0') + 1;
  syscall_printf ("%p = readdir (%p) (%s)", &de, dir, de->d_name);
  return 0;
}

void
fhandler_cygdrive::rewinddir (DIR *dir)
{
  pdrive = get_win32_name ();
  dir->__d_position = 0;
}

int
fhandler_cygdrive::closedir (DIR *dir)
{
  pdrive = get_win32_name ();
  return 0;
}
