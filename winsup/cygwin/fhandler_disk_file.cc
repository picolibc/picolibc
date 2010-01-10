/* fhandler_disk_file.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/acl.h>
#include <sys/statvfs.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "pinfo.h"
#include "ntdll.h"
#include "tls_pbuf.h"
#include "nfs.h"
#include "pwdgrp.h"
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
	  if (RtlEqualUnicodeString (fname, &ro_u_proc, FALSE))
	    {
	      found[__DIR_PROC] = true;
	      return 2;
	    }
	  if (fname->Length / sizeof (WCHAR) == mount_table->cygdrive_len - 2
	      && RtlEqualUnicodeString (fname, &cygdrive, FALSE))
	    {
	      found[__DIR_CYGDRIVE] = true;
	      return 2;
	    }
	}
      for (int i = 0; i < count; ++i)
	if (RtlEqualUnicodeString (fname, &mounts[i], FALSE))
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

static inline bool
is_volume_mountpoint (POBJECT_ATTRIBUTES attr)
{
  bool ret = false;
  IO_STATUS_BLOCK io;
  HANDLE reph;
  UNICODE_STRING subst;

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
	  && (RtlInitCountedUnicodeString (&subst, 
		(WCHAR *)((char *)rp->MountPointReparseBuffer.PathBuffer
			  + rp->MountPointReparseBuffer.SubstituteNameOffset),
		rp->MountPointReparseBuffer.SubstituteNameLength),
	      RtlEqualUnicodePathPrefix (&subst, &ro_u_volume, TRUE)))
	ret = true;
      NtClose (reph);
    }
  return ret;
}

static inline __ino64_t
get_ino_by_handle (path_conv &pc, HANDLE hdl)
{
  IO_STATUS_BLOCK io;
  FILE_INTERNAL_INFORMATION fai;

  if (NT_SUCCESS (NtQueryInformationFile (hdl, &io, &fai, sizeof fai,
					  FileInternalInformation))
      && pc.isgood_inode (fai.FileId.QuadPart))
    return fai.FileId.QuadPart;
  return 0;
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
  while (NT_SUCCESS (NtQueryDirectoryFile (fh, NULL, NULL, NULL, &io, fdibuf,
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
					    objcaseinsensitive (), fh, NULL);
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

/* For files on NFS shares, we request an EA of type NfsV3Attributes.
   This returns the content of a struct fattr3 as defined in RFC 1813.
   The content is the NFS equivalent of struct stat. so there's not much
   to do here except for copying. */
int __stdcall
fhandler_base::fstat_by_nfs_ea (struct __stat64 *buf)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  struct {
    FILE_FULL_EA_INFORMATION ffei;
    char buf[sizeof (NFS_V3_ATTR) + sizeof (fattr3)];
  } ffei_buf;
  struct {
     FILE_GET_EA_INFORMATION fgei;
     char buf[sizeof (NFS_V3_ATTR)];
   } fgei_buf;

  fgei_buf.fgei.NextEntryOffset = 0;
  fgei_buf.fgei.EaNameLength = sizeof (NFS_V3_ATTR) - 1;
  stpcpy (fgei_buf.fgei.EaName, NFS_V3_ATTR);
  status = NtQueryEaFile (get_handle (), &io,
			  &ffei_buf.ffei, sizeof ffei_buf, TRUE,
			  &fgei_buf.fgei, sizeof fgei_buf, NULL, TRUE);
  if (NT_SUCCESS (status))
    {
      fattr3 *nfs_attr = (fattr3 *) (ffei_buf.ffei.EaName
				     + ffei_buf.ffei.EaNameLength + 1);
      buf->st_dev = nfs_attr->fsid;
      buf->st_ino = nfs_attr->fileid;
      buf->st_mode = (nfs_attr->mode & 0xfff)
		     | nfs_type_mapping[nfs_attr->type & 7];
      buf->st_nlink = nfs_attr->nlink;
      /* FIXME: How to convert UNIX uid/gid to Windows SIDs? */
#if 0
      buf->st_uid = nfs_attr->uid;
      buf->st_gid = nfs_attr->gid;
#else
      buf->st_uid = myself->uid;
      buf->st_gid = myself->gid;
#endif
      buf->st_rdev = makedev (nfs_attr->rdev.specdata1,
			      nfs_attr->rdev.specdata2);
      buf->st_size = nfs_attr->size;
      buf->st_blksize = PREFERRED_IO_BLKSIZE;
      buf->st_blocks = nfs_attr->used / 512;
      buf->st_atim = nfs_attr->atime;
      buf->st_mtim = nfs_attr->mtime;
      buf->st_ctim = nfs_attr->ctime;
      return 0;
    }
  debug_printf ("%p = NtQueryEaFile(%S)", status, pc.get_nt_native_path ());
  return -1;
}

int __stdcall
fhandler_base::fstat_by_handle (struct __stat64 *buf)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  if (pc.fs_is_nfs ())
    return fstat_by_nfs_ea (buf);

  /* Don't use FileAllInformation info class.  It returns a pathname rather
     than a filename, so it needs a really big buffer for no good reason
     since we don't need the name anyway.  So we just call the three info
     classes necessary to get all information required by stat(2). */
  FILE_BASIC_INFORMATION fbi;
  FILE_STANDARD_INFORMATION fsi;
  FILE_INTERNAL_INFORMATION fii;

  status = NtQueryInformationFile (get_handle (), &io, &fbi, sizeof fbi,
				   FileBasicInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("%p = NtQueryInformationFile(%S, FileBasicInformation)",
		    status, pc.get_nt_native_path ());
      return -1;
    }
  status = NtQueryInformationFile (get_handle (), &io, &fsi, sizeof fsi,
				   FileStandardInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("%p = NtQueryInformationFile(%S, FileStandardInformation)",
		    status, pc.get_nt_native_path ());
      return -1;
    }
  status = NtQueryInformationFile (get_handle (), &io, &fii, sizeof fii,
				   FileInternalInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("%p = NtQueryInformationFile(%S, FileInternalInformation)",
		    status, pc.get_nt_native_path ());
      return -1;
    }
  /* If the change time is 0, it's a file system which doesn't
     support a change timestamp.  In that case use the LastWriteTime
     entry, as in other calls to fstat_helper. */
  if (pc.is_rep_symlink ())
    fbi.FileAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;
  pc.file_attributes (fbi.FileAttributes);
  return fstat_helper (buf,
		   fbi.ChangeTime.QuadPart ? &fbi.ChangeTime
					   : &fbi.LastWriteTime,
		   &fbi.LastAccessTime,
		   &fbi.LastWriteTime,
		   &fbi.CreationTime,
		   get_dev (),
		   fsi.EndOfFile.QuadPart,
		   fsi.AllocationSize.QuadPart,
		   fii.FileId.QuadPart,
		   fsi.NumberOfLinks,
		   fbi.FileAttributes);
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
  struct {
    FILE_ID_BOTH_DIR_INFORMATION fdi;
    WCHAR buf[NAME_MAX + 1];
  } fdi_buf;
  LARGE_INTEGER FileId;

  RtlSplitUnicodePath (pc.get_nt_native_path (), &dirname, &basename);
  InitializeObjectAttributes (&attr, &dirname, pc.objcaseinsensitive (),
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
  if (wincap.has_fileid_dirinfo () && !pc.has_buggy_fileid_dirinfo ()
      && NT_SUCCESS (status = NtQueryDirectoryFile (dir, NULL, NULL, NULL, &io,
						 &fdi_buf.fdi, sizeof fdi_buf,
						 FileIdBothDirectoryInformation,
						 TRUE, &basename, TRUE)))
    FileId = fdi_buf.fdi.FileId;
  else if (NT_SUCCESS (status = NtQueryDirectoryFile (dir, NULL, NULL, NULL,
						 &io, &fdi_buf.fdi,
						 sizeof fdi_buf,
						 FileDirectoryInformation,
						 TRUE, &basename, TRUE)))
    FileId.QuadPart = 0; /* get_ino is called in fstat_helper. */
  if (!NT_SUCCESS (status))
    {
      debug_printf ("%p = NtQueryDirectoryFile(%S)", status,
		    pc.get_nt_native_path ());
      NtClose (dir);
      goto too_bad;
    }
  NtClose (dir);
  /* If the change time is 0, it's a file system which doesn't
     support a change timestamp.  In that case use the LastWriteTime
     entry, as in other calls to fstat_helper. */
  if (pc.is_rep_symlink ())
    fdi_buf.fdi.FileAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;
  pc.file_attributes (fdi_buf.fdi.FileAttributes);
  return fstat_helper (buf,
		       fdi_buf.fdi.ChangeTime.QuadPart
		       ? &fdi_buf.fdi.ChangeTime : &fdi_buf.fdi.LastWriteTime,
		       &fdi_buf.fdi.LastAccessTime,
		       &fdi_buf.fdi.LastWriteTime,
		       &fdi_buf.fdi.CreationTime,
		       pc.fs_serial_number (),
		       fdi_buf.fdi.EndOfFile.QuadPart,
		       fdi_buf.fdi.AllocationSize.QuadPart,
		       FileId.QuadPart,
		       1,
		       fdi_buf.fdi.FileAttributes);

too_bad:
  LARGE_INTEGER ft;
  /* Arbitrary value: 2006-12-01 */
  RtlSecondsSince1970ToTime (1164931200L, &ft);
  return fstat_helper (buf,
		       &ft,
		       &ft,
		       &ft,
		       &ft,
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

/* The ChangeTime is taken from the NTFS ChangeTime entry, if reading
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
			     PLARGE_INTEGER ChangeTime,
			     PLARGE_INTEGER LastAccessTime,
			     PLARGE_INTEGER LastWriteTime,
			     PLARGE_INTEGER CreationTime,
			     DWORD dwVolumeSerialNumber,
			     ULONGLONG nFileSize,
			     LONGLONG nAllocSize,
			     ULONGLONG nFileIndex,
			     DWORD nNumberOfLinks,
			     DWORD dwFileAttributes)
{
  IO_STATUS_BLOCK st;
  FILE_COMPRESSION_INFORMATION fci;

  to_timestruc_t ((PFILETIME) LastAccessTime, &buf->st_atim);
  to_timestruc_t ((PFILETIME) LastWriteTime, &buf->st_mtim);
  to_timestruc_t ((PFILETIME) ChangeTime, &buf->st_ctim);
  to_timestruc_t ((PFILETIME) CreationTime, &buf->st_birthtim);
  buf->st_dev = dwVolumeSerialNumber;
  buf->st_size = (_off64_t) nFileSize;
  /* The number of links to a directory includes the
     number of subdirectories in the directory, since all
     those subdirectories point to it.
     This is too slow on remote drives, so we do without it.
     Setting the count to 2 confuses `find (1)' command. So
     let's try it with `1' as link count. */
#if 0
  buf->st_nlink = pc.ndisk_links (nNumberOfLinks);
#else
  buf->st_nlink = nNumberOfLinks;
#endif

  /* Enforce namehash as inode number on untrusted file systems. */
  if (pc.isgood_inode (nFileIndex))
    buf->st_ino = (__ino64_t) nFileIndex;
  else
    buf->st_ino = get_ino ();

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
  /* Using a side effect: get_file_attributes checks for directory.
     This is used, to set S_ISVTX, if needed.  */
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
	  /* Check suffix for executable file. */
	  if (pc.exec_state () == dont_know_if_executable)
	    {
	      PUNICODE_STRING path = pc.get_nt_native_path ();

	      if (RtlEqualUnicodePathSuffix (path, &ro_u_exe, TRUE)
		  || RtlEqualUnicodePathSuffix (path, &ro_u_lnk, TRUE)
		  || RtlEqualUnicodePathSuffix (path, &ro_u_com, TRUE))
		pc.set_exec ();
	    }
	  /* No known suffix, check file header.  This catches binaries and
	     shebang scripts. */
	  if (pc.exec_state () == dont_know_if_executable)
	    {
	      OBJECT_ATTRIBUTES attr;
	      HANDLE h;
	      IO_STATUS_BLOCK io;

	      InitializeObjectAttributes (&attr, &ro_u_empty, 0, get_handle (),
					  NULL);
	      if (NT_SUCCESS (NtOpenFile (&h, SYNCHRONIZE | FILE_READ_DATA,
					  &attr, &io, FILE_SHARE_VALID_FLAGS,
					  FILE_SYNCHRONOUS_IO_NONALERT)))
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
      /* If the FS supports ACLs, we're here because we couldn't even open
	 the file for READ_CONTROL access.  Chances are high that the file's
	 security descriptor has no ACE for "Everyone", so we should not fake
	 any access for "others". */
      if (has_acls ())
	buf->st_mode &= ~(S_IROTH | S_IWOTH | S_IXOTH);
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

  sfs->f_files = ULONG_MAX;
  sfs->f_ffree = ULONG_MAX;
  sfs->f_favail = ULONG_MAX;
  sfs->f_fsid = pc.fs_serial_number ();
  sfs->f_flag = pc.fs_flags ();
  sfs->f_namemax = pc.fs_name_len ();
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
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  if (pc.is_fs_special ())
    return chmod_device (pc, mode);

  if (!get_handle ())
    {
      query_open (query_write_control);
      if (!(oret = open (O_BINARY, 0)))
	{
	  /* Need WRITE_DAC|WRITE_OWNER to write ACLs. */
	  if (pc.has_acls ())
	    return -1;
	  /* Otherwise FILE_WRITE_ATTRIBUTES is sufficient. */
	  query_open (query_write_attributes);
	  if (!(oret = open (O_BINARY, 0)))
	    return -1;
	}
    }

  if (pc.fs_is_nfs ())
    {
      /* chmod on NFS shares works by writing an EA of type NfsV3Attributes.
	 Only type and mode have to be set.  Apparently type isn't checked
	 for consistency, so it's sufficent to set it to NF3REG all the time. */
      struct {
	FILE_FULL_EA_INFORMATION ffei;
	char buf[sizeof (NFS_V3_ATTR) + sizeof (fattr3)];
      } ffei_buf;
      ffei_buf.ffei.NextEntryOffset = 0;
      ffei_buf.ffei.Flags = 0;
      ffei_buf.ffei.EaNameLength = sizeof (NFS_V3_ATTR) - 1;
      ffei_buf.ffei.EaValueLength = sizeof (fattr3);
      strcpy (ffei_buf.ffei.EaName, NFS_V3_ATTR);
      fattr3 *nfs_attr = (fattr3 *) (ffei_buf.ffei.EaName
				     + ffei_buf.ffei.EaNameLength + 1);
      memset (nfs_attr, 0, sizeof (fattr3));
      nfs_attr->type = NF3REG;
      nfs_attr->mode = mode;
      status = NtSetEaFile (get_handle (), &io,
			    &ffei_buf.ffei, sizeof ffei_buf);
      if (!NT_SUCCESS (status))
	__seterrno_from_nt_status (status);
      else
	res = 0;
      goto out;
    }

  if (pc.has_acls ())
    {
      if (pc.isdir ())
	mode |= S_IFDIR;
      if (!set_file_attribute (get_handle (), pc,
			       ILLEGAL_UID, ILLEGAL_GID, mode))
	res = 0;
    }

  /* If the mode has any write bits set, the DOS R/O flag is in the way. */
  if (mode & (S_IWUSR | S_IWGRP | S_IWOTH))
    pc &= (DWORD) ~FILE_ATTRIBUTE_READONLY;
  else if (!pc.has_acls ())	/* Never set DOS R/O if security is used. */
    pc |= (DWORD) FILE_ATTRIBUTE_READONLY;
  if (S_ISSOCK (mode))
    pc |= (DWORD) FILE_ATTRIBUTE_SYSTEM;

  status = NtSetAttributesFile (get_handle (), pc.file_attributes ());
  /* MVFS needs a good amount of kicking to be convinced that it has to write
     back metadata changes and to invalidate the cached metadata information
     stored for the given handle.  This method to open a second handle to
     the file and write the same metadata information twice has been found
     experimentally: http://cygwin.com/ml/cygwin/2009-07/msg00533.html */
  if (pc.fs_is_mvfs () && NT_SUCCESS (status) && !oret)
    {
      OBJECT_ATTRIBUTES attr;
      HANDLE fh;

      InitializeObjectAttributes (&attr, &ro_u_empty, 0, get_handle (), NULL);
      if (NT_SUCCESS (NtOpenFile (&fh, FILE_WRITE_ATTRIBUTES, &attr, &io,
				  FILE_SHARE_VALID_FLAGS,
				  FILE_OPEN_FOR_BACKUP_INTENT)))
	{
	  NtSetAttributesFile (fh, pc.file_attributes ());
	  NtClose (fh);
	}
    }
  /* Correct NTFS security attributes have higher priority */
  if (!pc.has_acls ())
    {
      if (!NT_SUCCESS (status))
	__seterrno_from_nt_status (status);
      else
	res = 0;
    }

out:
  if (oret)
    close_fs ();

  return res;
}

int __stdcall
fhandler_disk_file::fchown (__uid32_t uid, __gid32_t gid)
{
  int oret = 0;

  if (!pc.has_acls ())
    {
      /* fake - if not supported, pretend we're like win95
	 where it just works */
      /* FIXME: Could be supported on NFS when user->uid mapping is in place. */
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
  __uid32_t old_uid;
  int res = get_file_attribute (get_handle (), pc, &attrib, &old_uid, NULL);
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
      /* If you're running a Samba server which has no winbidd running, the
         uid<->SID mapping is disfunctional.  Even trying to chown to your
	 own account fails since the account used on the server is the UNIX
	 account which gets used for the standard user mapping.  This is a
	 default mechanism which doesn't know your real Windows SID.
	 There are two possible error codes in different Samba releases for
	 this situation, one of them is unfortunately the not very significant
	 STATUS_ACCESS_DENIED.  Instead of relying on the error codes, we're
	 using the below very simple heuristic.  If set_file_attribute failed,
	 and the original user account was either already unknown, or one of
	 the standard UNIX accounts, we're faking success. */
      if (res == -1 && pc.fs_is_samba ())
	{
	  cygsid sid;

	  if (old_uid == ILLEGAL_UID
	      || (sid.getfrompw (internal_getpwuid (old_uid))
		  && EqualPrefixSid (sid, well_known_samba_unix_user_fake_sid)))
	    {
	      debug_printf ("Faking chown worked on standalone Samba");
	      res = 0;
	    }
	}
    }
  if (oret)
    close_fs ();

  return res;
}

int _stdcall
fhandler_disk_file::facl (int cmd, int nentries, __aclent32_t *aclbufp)
{
  int res = -1;
  int oret = 0;

  if (!pc.has_acls ())
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
		if ((oret && !fstat_by_handle (&st))
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
    close_fs ();

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
	  && (pc.fs_flags () & FILE_SUPPORTS_SPARSE_FILES)
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
  size_t nlen = strlen (newpath);
  path_conv newpc (newpath, PC_SYM_NOFOLLOW | PC_POSIX | PC_NULLEMPTY, stat_suffixes);
  if (newpc.error)
    {
      set_errno (newpc.error);
      return -1;
    }

  if (newpc.exists ())
    {
      syscall_printf ("file '%S' exists?", newpc.get_nt_native_path ());
      set_errno (EEXIST);
      return -1;
    }

  if (isdirsep (newpath[nlen - 1]) || has_dot_last_component (newpath, false))
    {
      set_errno (ENOENT);
      return -1;
    }

  char new_buf[nlen + 5];
  if (!newpc.error)
    {
      /* If the original file is a lnk special file (except for sockets),
	 and if the original file has a .lnk suffix, add one to the hardlink
	 as well. */
      if (pc.is_lnk_special () && !pc.issocket ()
	  && RtlEqualUnicodePathSuffix (pc.get_nt_native_path (),
					&ro_u_lnk, TRUE))
	{
	  /* Shortcut hack. */
	  stpcpy (stpcpy (new_buf, newpath), ".lnk");
	  newpath = new_buf;
	  newpc.check (newpath, PC_SYM_NOFOLLOW);
	}
      else if (!pc.isdir ()
	       && pc.is_binary ()
	       && RtlEqualUnicodePathSuffix (pc.get_nt_native_path (),
					     &ro_u_exe, TRUE)
	       && !RtlEqualUnicodePathSuffix (newpc.get_nt_native_path (),
					      &ro_u_exe, TRUE))
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
	  /* FS doesn't support hard links.  Linux returns EPERM. */
	  set_errno (EPERM);
	  return -1;
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
fhandler_disk_file::utimens (const struct timespec *tvp)
{
  return utimens_fs (tvp);
}

int
fhandler_base::utimens_fs (const struct timespec *tvp)
{
  struct timespec timeofday;
  struct timespec tmp[2];
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

  clock_gettime (CLOCK_REALTIME, &timeofday);
  if (!tvp)
    tmp[1] = tmp[0] = timeofday;
  else
    {
      if ((tvp[0].tv_nsec < UTIME_NOW || tvp[0].tv_nsec > 999999999L)
	  || (tvp[1].tv_nsec < UTIME_NOW || tvp[1].tv_nsec > 999999999L))
	{
	  if (closeit)
	    close_fs ();
	  set_errno (EINVAL);
	  return -1;
	}
      tmp[0] = (tvp[0].tv_nsec == UTIME_NOW) ? timeofday : tvp[0];
      tmp[1] = (tvp[1].tv_nsec == UTIME_NOW) ? timeofday : tvp[1];
    }
  debug_printf ("incoming lastaccess %08x %08x", tmp[0].tv_sec, tmp[0].tv_nsec);

  IO_STATUS_BLOCK io;
  FILE_BASIC_INFORMATION fbi;

  fbi.CreationTime.QuadPart = 0LL;
  /* UTIME_OMIT is handled in timespec_to_filetime by setting FILETIME to 0. */
  timespec_to_filetime (&tmp[0], (LPFILETIME) &fbi.LastAccessTime);
  timespec_to_filetime (&tmp[1], (LPFILETIME) &fbi.LastWriteTime);
  fbi.ChangeTime.QuadPart = 0LL;
  fbi.FileAttributes = 0;
  NTSTATUS status = NtSetInformationFile (get_handle (), &io, &fbi, sizeof fbi,
					  FileBasicInformation);
  /* For this special case for MVFS see the comment in
     fhandler_disk_file::fchmod. */
  if (pc.fs_is_mvfs () && NT_SUCCESS (status) && !closeit)
    {
      OBJECT_ATTRIBUTES attr;
      HANDLE fh;

      InitializeObjectAttributes (&attr, &ro_u_empty, 0, get_handle (), NULL);
      if (NT_SUCCESS (NtOpenFile (&fh, FILE_WRITE_ATTRIBUTES, &attr, &io,
				  FILE_SHARE_VALID_FLAGS,
				  FILE_OPEN_FOR_BACKUP_INTENT)))
	{
	  NtSetInformationFile (fh, &io, &fbi, sizeof fbi,
				FileBasicInformation);
	  NtClose (fh);
	}
    }
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

    ino = get_ino_by_handle (pc, get_handle ());
    /* A unique ID is necessary to recognize fhandler entries which are
       duplicated by dup(2) or fork(2). */
    AllocateLocallyUniqueId ((PLUID) &unique_id);

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

int
fhandler_disk_file::mkdir (mode_t mode)
{
  int res = -1;
  SECURITY_ATTRIBUTES sa = sec_none_nih;
  NTSTATUS status;
  HANDLE dir;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  PFILE_FULL_EA_INFORMATION p = NULL;
  ULONG plen = 0;

  if (pc.fs_is_nfs ())
    {
      /* When creating a dir on an NFS share, we have to set the
	 file mode by writing a NFS fattr3 structure with the
	 correct mode bits set. */
      plen = sizeof (FILE_FULL_EA_INFORMATION) + sizeof (NFS_V3_ATTR)
	     + sizeof (fattr3);
      p = (PFILE_FULL_EA_INFORMATION) alloca (plen);
      p->NextEntryOffset = 0;
      p->Flags = 0;
      p->EaNameLength = sizeof (NFS_V3_ATTR) - 1;
      p->EaValueLength = sizeof (fattr3);
      strcpy (p->EaName, NFS_V3_ATTR);
      fattr3 *nfs_attr = (fattr3 *) (p->EaName + p->EaNameLength + 1);
      memset (nfs_attr, 0, sizeof (fattr3));
      nfs_attr->type = NF3DIR;
      nfs_attr->mode = (mode & 07777) & ~cygheap->umask;
    }
  status = NtCreateFile (&dir, FILE_LIST_DIRECTORY | SYNCHRONIZE,
			 pc.get_object_attr (attr, sa), &io, NULL,
			 FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_VALID_FLAGS,
			 FILE_CREATE,
			 FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
			 | FILE_OPEN_FOR_BACKUP_INTENT,
			 p, plen);
  if (NT_SUCCESS (status))
    {
      if (has_acls ())
	set_file_attribute (dir, pc, ILLEGAL_UID, ILLEGAL_GID,
			    S_JUSTCREATED | S_IFDIR
			    | ((mode & 07777) & ~cygheap->umask));
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
			    + (NAME_MAX + 1) * sizeof (WCHAR)))

struct __DIR_cache
{
  char  __cache[DIR_BUF_SIZE];	/* W2K needs this buffer 8 byte aligned. */
  ULONG __pos;
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
	     persistent ACLs, FileDirectoryInformation otherwise.

	     NFS clients hide dangling symlinks from directory queries,
	     unless you use the FileNamesInformation info class.
	     On newer NFS clients (>=Vista) FileIdBothDirectoryInformation
	     works fine, but only if the NFS share is mounted to a drive
	     letter.  TODO: We don't test that here for now, but it might
	     be worth to test if there's a speed gain in using
	     FileIdBothDirectoryInformation, because it doesn't require to
	     open the file to read the inode number. */
	  if (pc.hasgood_inode ())
	    {
	      dir->__flags |= dirent_set_d_ino;
	      if (pc.fs_is_nfs ())
		dir->__flags |= dirent_nfs_d_ino;
	      else if (wincap.has_fileid_dirinfo ()
		       && !pc.has_buggy_fileid_dirinfo ())
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
  path_conv pc (path, PC_SYM_NOFOLLOW | PC_POSIX | PC_NOWARN);
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
      ino = get_ino_by_handle (pc, hdl);
      if (!ino)
	ino = hash_path_name (0, pc.get_nt_native_path ());
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

  /* Set d_type if type can be determined from file attributes.
     FILE_ATTRIBUTE_SYSTEM ommitted to leave DT_UNKNOWN for old symlinks.
     For new symlinks, d_type will be reset to DT_UNKNOWN below.  */
  if (attr &&
      !(attr & (  ~FILE_ATTRIBUTE_VALID_FLAGS
		| FILE_ATTRIBUTE_SYSTEM
		| FILE_ATTRIBUTE_REPARSE_POINT)))
    {
      if (attr & FILE_ATTRIBUTE_DIRECTORY)
	de->d_type = DT_DIR;
      else
	de->d_type = DT_REG;
    }

  /* Check for directory reparse point.  These are potential volume mount
     points which have another inode than the underlying directory. */
  if ((attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
      == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
    {
      HANDLE reph;
      OBJECT_ATTRIBUTES attr;
      IO_STATUS_BLOCK io;

      InitializeObjectAttributes (&attr, fname, pc.objcaseinsensitive (),
				  get_handle (), NULL);
      if (is_volume_mountpoint (&attr)
	  && (NT_SUCCESS (NtOpenFile (&reph, READ_CONTROL, &attr, &io,
				      FILE_SHARE_VALID_FLAGS,
				      FILE_OPEN_FOR_BACKUP_INTENT))))
	{
	  de->d_ino = get_ino_by_handle (pc, reph);
	  NtClose (reph);
	}
    }

  /* Check for Windows shortcut. If it's a Cygwin or U/WIN
     symlink, drop the .lnk suffix. */
  if ((attr & FILE_ATTRIBUTE_READONLY) && fname->Length > 4 * sizeof (WCHAR))
    {
      UNICODE_STRING uname;

      RtlInitCountedUnicodeString (&uname,
				   fname->Buffer
				   + fname->Length / sizeof (WCHAR) - 4,
				   4 * sizeof (WCHAR));
      if (RtlEqualUnicodeString (&uname, &ro_u_lnk, TRUE))
	{
	  tmp_pathbuf tp;
	  UNICODE_STRING fbuf;

	  tp.u_get (&fbuf);
	  RtlCopyUnicodeString (&fbuf, pc.get_nt_native_path ());
	  RtlAppendUnicodeToString (&fbuf, L"\\");
	  RtlAppendUnicodeStringToString (&fbuf, fname);
	  fbuf.Buffer += 4; /* Skip leading \??\ */
	  fbuf.Length -= 4 * sizeof (WCHAR);
	  if (fbuf.Buffer[1] != L':') /* UNC path */
	    {
	      *(fbuf.Buffer += 2) = L'\\';
	      fbuf.Length -= 2 * sizeof (WCHAR);
	    }
	  path_conv fpath (&fbuf, PC_SYM_NOFOLLOW);
	  if (fpath.issymlink () || fpath.is_fs_special ())
	    {
	      fname->Length -= 4 * sizeof (WCHAR);
	      de->d_type = DT_UNKNOWN;
	    }
	}
    }

  sys_wcstombs (de->d_name, NAME_MAX + 1, fname->Buffer,
		fname->Length / sizeof (WCHAR));

  /* Don't try to optimize relative to dir->__d_position.  On several
     filesystems it's no safe bet that "." and ".." entries always
     come first. */
  if (de->d_name[0] == '.')
    {
      if (de->d_name[1] == '\0')
	dir->__flags |= dirent_saw_dot;
      else if (de->d_name[1] == '.' && de->d_name[2] == '\0')
	dir->__flags |= dirent_saw_dot_dot;
    }
  return 0;
}

int
fhandler_disk_file::readdir (DIR *dir, dirent *de)
{
  int res = 0;
  NTSTATUS status = STATUS_SUCCESS;
  PFILE_ID_BOTH_DIR_INFORMATION buf = NULL;
  PWCHAR FileName;
  ULONG FileNameLength;
  ULONG FileAttributes = 0;
  IO_STATUS_BLOCK io;
  UNICODE_STRING fname;

  /* d_cachepos always refers to the next cache entry to use.  If it's 0
     we must reload the cache. */
  if (d_cachepos (dir) == 0)
    {
      if ((dir->__flags & dirent_get_d_ino))
	{
	  status = NtQueryDirectoryFile (get_handle (), NULL, NULL, NULL, &io,
					 d_cache (dir), DIR_BUF_SIZE,
					 FileIdBothDirectoryInformation,
					 FALSE, NULL, dir->__d_position == 0);
	  /* FileIdBothDirectoryInformation isn't supported for remote drives
	     on NT4 and 2K systems, and it's also not supported on 2K at all,
	     when accessing network drives on any remote OS.  There are also
	     hacked versions of Samba 3.0.x out there (Debian-based it seems),
	     which return STATUS_NOT_SUPPORTED rather than handling this info
	     class.  We just fall back to using a standard directory query in
	     this case and note this case using the dirent_get_d_ino flag. */
	  if (status == STATUS_INVALID_LEVEL
	      || status == STATUS_NOT_SUPPORTED
	      || status == STATUS_INVALID_PARAMETER
	      || status == STATUS_INVALID_INFO_CLASS)
	    dir->__flags &= ~dirent_get_d_ino;
	  /* Something weird happens on Samba up to version 3.0.21c, which is
	     fixed in 3.0.22.  FileIdBothDirectoryInformation seems to work
	     nicely, but only up to the 128th entry in the directory.  After
	     reaching this entry, the next call to NtQueryDirectoryFile
	     (FileIdBothDirectoryInformation) returns STATUS_INVALID_LEVEL.
	     Why should we care, we can just switch to FileDirectoryInformation,
	     isn't it?  Nope!  The next call to
	       NtQueryDirectoryFile(FileDirectoryInformation)
	     actually returns STATUS_NO_MORE_FILES, regardless how many files
	     are left unread in the directory.  This does not happen when using
	     FileDirectoryInformation right from the start, but since
	     we can't decide whether the server we're talking with has this
	     bug or not, we end up serving Samba shares always in the slow
	     mode using FileDirectoryInformation.  So, what we do here is
	     to implement the solution suggested by Andrew Tridgell,  we just
	     reread all entries up to dir->d_position using
	     FileDirectoryInformation.
	     However, We do *not* mark this server as broken and fall back to
	     using FileDirectoryInformation further on.  This would slow
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
					   NULL, &io, d_cache (dir),
					   DIR_BUF_SIZE,
					   FileDirectoryInformation,
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
	status = NtQueryDirectoryFile (get_handle (), NULL, NULL, NULL, &io,
				       d_cache (dir), DIR_BUF_SIZE,
				       (dir->__flags & dirent_nfs_d_ino)
				       ? FileNamesInformation
				       : FileDirectoryInformation,
				       FALSE, NULL, dir->__d_position == 0);
    }

go_ahead:

  if (status == STATUS_NO_MORE_FILES)
    /*nothing*/;
  else if (!NT_SUCCESS (status))
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
	  FileNameLength = buf->FileNameLength;
	  FileAttributes = buf->FileAttributes;
	  if ((dir->__flags & dirent_set_d_ino))
	    de->d_ino = buf->FileId.QuadPart;
	}
      else if ((dir->__flags & dirent_nfs_d_ino))
	{
	  FileName = ((PFILE_NAMES_INFORMATION) buf)->FileName;
	  FileNameLength = ((PFILE_NAMES_INFORMATION) buf)->FileNameLength;
	}
      else
	{
	  FileName = ((PFILE_DIRECTORY_INFORMATION) buf)->FileName;
	  FileNameLength = ((PFILE_DIRECTORY_INFORMATION) buf)->FileNameLength;
	  FileAttributes = ((PFILE_DIRECTORY_INFORMATION) buf)->FileAttributes;
	}
      RtlInitCountedUnicodeString (&fname, FileName, FileNameLength);
      de->d_ino = d_mounts (dir)->check_mount (&fname, de->d_ino);
      if (de->d_ino == 0 && (dir->__flags & dirent_set_d_ino))
	{
	  /* Don't try to optimize relative to dir->__d_position.  On several
	     filesystems it's no safe bet that "." and ".." entries always
	     come first. */
	  if (FileNameLength == sizeof (WCHAR) && FileName[0] == '.')
	    de->d_ino = get_ino_by_handle (pc, get_handle ());
	  else if (FileNameLength == 2 * sizeof (WCHAR)
		   && FileName[0] == L'.' && FileName[1] == L'.')
	    {
	      if (!(dir->__flags & dirent_isroot))
		de->d_ino = readdir_get_ino (get_name (), true);
	      else
		de->d_ino = get_ino_by_handle (pc, get_handle ());
	    }
	  else
	    {
	      OBJECT_ATTRIBUTES attr;
	      HANDLE hdl;
	      NTSTATUS f_status;

	      InitializeObjectAttributes (&attr, &fname,
					  pc.objcaseinsensitive (),
					  get_handle (), NULL);
	      /* FILE_OPEN_REPARSE_POINT on NFS is a no-op, so the normal
	         NtOpenFile here returns the inode number of the symlink target,
		 rather than the inode number of the symlink itself.
		 
		 Worse, trying to open a symlink without setting the special
		 "ActOnSymlink" EA triggers a bug in Windows 7 which results
		 in a timeout of up to 20 seconds, followed by two exceptions
		 in the NT kernel.

		 Since both results are far from desirable, we open symlinks
		 on NFS so that we get the right inode and a happy W7.
		 And, since some filesystems choke on the EAs, we don't
		 use them unconditionally. */
	      f_status = (dir->__flags & dirent_nfs_d_ino)
			 ? NtCreateFile (&hdl, READ_CONTROL, &attr, &io,
					 NULL, 0, FILE_SHARE_VALID_FLAGS,
					 FILE_OPEN, FILE_OPEN_FOR_BACKUP_INTENT,
					 &nfs_aol_ffei, sizeof nfs_aol_ffei)
			 : NtOpenFile (&hdl, READ_CONTROL, &attr, &io,
				       FILE_SHARE_VALID_FLAGS,
				       FILE_OPEN_FOR_BACKUP_INTENT
				       | FILE_OPEN_REPARSE_POINT);
	      if (NT_SUCCESS (f_status))
		{
		  de->d_ino = get_ino_by_handle (pc, hdl);
		  NtClose (hdl);
		}
	    }
	  /* Untrusted file system.  Don't try to fetch inode number again. */
	  if (de->d_ino == 0)
	    dir->__flags &= ~dirent_set_d_ino;
	}
    }

  if (!(res = readdir_helper (dir, de, RtlNtStatusToDosError (status),
			      buf ? FileAttributes : 0, &fname)))
    dir->__d_position++;
  else if (!(dir->__flags & dirent_saw_dot))
    {
      strcpy (de->d_name , ".");
      de->d_ino = get_ino_by_handle (pc, get_handle ());
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
	de->d_ino = get_ino_by_handle (pc, get_handle ());
      dir->__d_position++;
      dir->__flags |= dirent_saw_dot_dot;
      res = 0;
    }

  syscall_printf ("%d = readdir (%p, %p) (L\"%lS\" > \"%ls\")", res, dir, &de,
		  res ? NULL : &fname, res ? "***" : de->d_name);
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
      OBJECT_ATTRIBUTES attr;
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      HANDLE new_dir;

      InitializeObjectAttributes (&attr, &ro_u_empty, pc.objcaseinsensitive (),
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

void
fhandler_cygdrive::set_drives ()
{
  pdrive = pdrive_buf;
  ndrives = GetLogicalDriveStrings (sizeof pdrive_buf, pdrive_buf) / DRVSZ;
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
      n--;
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
  pdrive = pdrive_buf;
  dir->__d_position = 0;
}

int
fhandler_cygdrive::closedir (DIR *dir)
{
  pdrive = pdrive_buf;
  return 0;
}
