/* fhandler_disk_file.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <sys/acl.h>
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
#include <ntdef.h>
#include "ntdll.h"
#include <assert.h>
#include <ctype.h>
#include <winioctl.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

unsigned __stdcall
path_conv::ndisk_links (DWORD nNumberOfLinks)
{
  if (!isdir () || isremote ())
    return nNumberOfLinks;

  int len = strlen (*this);
  char fn[len + 3];
  strcpy (fn, *this);

  const char *s;
  unsigned count;
  if (nNumberOfLinks <= 1)
    {
      s = "/*";
      count = 0;
    }
  else
    {
      s = "/..";
      count = nNumberOfLinks;
    }

  if (len == 0 || isdirsep (fn[len - 1]))
    strcpy (fn + len, s + 1);
  else
    strcpy (fn + len, s);

  WIN32_FIND_DATA buf;
  HANDLE h = FindFirstFile (fn, &buf);

  int saw_dot = 2;
  if (h != INVALID_HANDLE_VALUE)
    {
      if (nNumberOfLinks > 1)
	saw_dot--;
      else
	do
	  {
	    if (buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	      count++;
	    if (buf.cFileName[0] == '.'
		&& (buf.cFileName[1] == '\0'
		    || (buf.cFileName[1] == '.' && buf.cFileName[2] == '\0')))
	      saw_dot--;
	  }
	while (FindNextFileA (h, &buf));
      FindClose (h);
    }

  if (nNumberOfLinks > 1)
    {
      fn[len + 2] = '\0';
      h = FindFirstFile (fn, &buf);
      if (h)
	saw_dot--;
      FindClose (h);
    }

  return count + saw_dot;
}

int __stdcall
fhandler_base::fstat_by_handle (struct __stat64 *buf)
{
  BY_HANDLE_FILE_INFORMATION local;
  BOOL res = GetFileInformationByHandle (get_handle (), &local);
  debug_printf ("%d = GetFileInformationByHandle (%s, %d)",
		res, get_win32_name (), get_handle ());
  /* GetFileInformationByHandle will fail if it's given stdio handle or pipe*/
  if (!res)
    {
      memset (&local, 0, sizeof (local));
      local.nFileSizeLow = GetFileSize (get_handle (), &local.nFileSizeHigh);
    }

  return fstat_helper (buf,
		       local.ftCreationTime,
		       local.ftLastAccessTime,
		       local.ftLastWriteTime,
		       local.nFileSizeHigh,
		       local.nFileSizeLow,
		       local.nFileIndexHigh,
		       local.nFileIndexLow,
		       local.nNumberOfLinks);
}

int __stdcall
fhandler_base::fstat_by_name (struct __stat64 *buf)
{
  int res;
  HANDLE handle;
  WIN32_FIND_DATA local;

  if (!pc.exists ())
    {
      debug_printf ("already determined that pc does not exist");
      set_errno (ENOENT);
      res = -1;
    }
  else if ((handle = FindFirstFile (pc, &local)) != INVALID_HANDLE_VALUE)
    {
      FindClose (handle);
      res = fstat_helper (buf,
			  local.ftCreationTime,
			  local.ftLastAccessTime,
			  local.ftLastWriteTime,
			  local.nFileSizeHigh,
			  local.nFileSizeLow);
    }
  else if (pc.isdir ())
    {
      FILETIME ft = {};
      res = fstat_helper (buf, ft, ft, ft, 0, 0);
    }
  else
    {
      debug_printf ("FindFirstFile failed for '%s', %E", (char *) pc);
      __seterrno ();
      res = -1;
    }
  return res;
}

int __stdcall
fhandler_base::fstat_fs (struct __stat64 *buf)
{
  int res = -1;
  int oret;
  int open_flags = O_RDONLY | O_BINARY;

  if (get_io_handle ())
    {
      if (nohandle ())
	return fstat_by_name (buf);
      else
	return fstat_by_handle (buf);
    }
  /* If we don't care if the file is executable or we already know if it is,
     then just do a "query open" as it is apparently much faster. */
  if (pc.exec_state () != dont_know_if_executable)
    {
      if (pc.fs_is_fat () && !strpbrk (get_win32_name (), "?*|<>"))
	return fstat_by_name (buf);
      query_open (query_stat_control);
    }
  if (!(oret = open_fs (open_flags, 0)) && get_errno () == EACCES)
    {
      /* If we couldn't open the file, try a query open with no permissions.
	 This allows us to determine *some* things about the file, at least. */
      pc.set_exec (0);
      query_open (query_read_control);
      oret = open_fs (open_flags, 0);
    }

  if (oret)
    {
      /* We now have a valid handle, regardless of the "nohandle" state.
	 Since fhandler_base::open only calls CloseHandle if !nohandle,
	 we have to set it to false before calling close_fs and restore
	 the state afterwards. */
      res = fstat_by_handle (buf);
      bool no_handle = nohandle ();
      nohandle (false);
      close_fs ();
      nohandle (no_handle);
      set_io_handle (NULL);
    }
  else
    res = fstat_by_name (buf);

  return res;
}

int __stdcall
fhandler_base::fstat_helper (struct __stat64 *buf,
			     FILETIME ftCreationTime,
			     FILETIME ftLastAccessTime,
			     FILETIME ftLastWriteTime,
			     DWORD nFileSizeHigh,
			     DWORD nFileSizeLow,
			     DWORD nFileIndexHigh,
			     DWORD nFileIndexLow,
			     DWORD nNumberOfLinks)
{
  IO_STATUS_BLOCK st;
  FILE_COMPRESSION_INFORMATION fci;

  /* This is for FAT filesystems, which don't support atime/ctime */
  if (ftLastAccessTime.dwLowDateTime == 0
      && ftLastAccessTime.dwHighDateTime == 0)
    ftLastAccessTime = ftLastWriteTime;
  if (ftCreationTime.dwLowDateTime == 0
      && ftCreationTime.dwHighDateTime == 0)
    ftCreationTime = ftLastWriteTime;

  to_timestruc_t (&ftLastAccessTime, &buf->st_atim);
  to_timestruc_t (&ftLastWriteTime, &buf->st_mtim);
  to_timestruc_t (&ftCreationTime, &buf->st_ctim);
  buf->st_dev = pc.volser ();
  buf->st_size = ((_off64_t) nFileSizeHigh << 32) + nFileSizeLow;
  /* The number of links to a directory includes the
     number of subdirectories in the directory, since all
     those subdirectories point to it.
     This is too slow on remote drives, so we do without it.
     Setting the count to 2 confuses `find (1)' command. So
     let's try it with `1' as link count. */
  buf->st_nlink = pc.ndisk_links (nNumberOfLinks);

  /* Assume that if a drive has ACL support it MAY have valid "inodes".
     It definitely does not have valid inodes if it does not have ACL
     support. */
  switch (pc.has_acls () && (nFileIndexHigh || nFileIndexLow)
	  ? pc.drive_type () : DRIVE_UNKNOWN)
    {
    case DRIVE_FIXED:
    case DRIVE_REMOVABLE:
    case DRIVE_CDROM:
    case DRIVE_RAMDISK:
      /* Although the documentation indicates otherwise, it seems like
	 "inodes" on these devices are persistent, at least across reboots. */
      buf->st_ino = (((__ino64_t) nFileIndexHigh) << 32)
		    | (__ino64_t) nFileIndexLow;
      break;
    default:
      /* Either the nFileIndex* fields are unreliable or unavailable.  Use the
	 next best alternative. */
      buf->st_ino = get_namehash ();
      break;
    }

  buf->st_blksize = S_BLKSIZE;

  /* On compressed and sparsed files, we request the actual amount of bytes
     allocated on disk.  */
  if (pc.has_attribute (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE)
      && get_io_handle ()
      && !NtQueryInformationFile (get_io_handle (), &st, (PVOID) &fci,
				  sizeof fci, FileCompressionInformation))
    buf->st_blocks = (fci.CompressedSize.QuadPart + S_BLKSIZE - 1) / S_BLKSIZE;
  else
    /* Just compute no. of blocks from file size. */
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
      get_file_attribute (pc.has_acls (), get_io_handle (), get_win32_name (),
			  NULL, &buf->st_uid, &buf->st_gid);
      goto done;
    }
  else if (pc.issocket ())
    buf->st_mode = S_IFSOCK;

  if (!get_file_attribute (pc.has_acls (), get_io_handle (), get_win32_name (),
			   &buf->st_mode, &buf->st_uid, &buf->st_gid))
    {
      /* If read-only attribute is set, modify ntsec return value */
      if (pc.has_attribute (FILE_ATTRIBUTE_READONLY) && !pc.issymlink ())
	buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);

      if (buf->st_mode & S_IFMT)
	/* nothing */;
      else if (!is_fs_special ())
	buf->st_mode |= S_IFREG;
      else
	{
	  buf->st_dev = dev ();
	  buf->st_mode = dev ().mode;
	}
    }
  else
    {
      buf->st_mode |= STD_RBITS;

      if (!pc.has_attribute (FILE_ATTRIBUTE_READONLY))
	buf->st_mode |= STD_WBITS;
      /* | S_IWGRP | S_IWOTH; we don't give write to group etc */

      if (S_ISDIR (buf->st_mode))
	buf->st_mode |= S_IFDIR | STD_XBITS;
      else if (buf->st_mode & S_IFMT)
	/* nothing */;
      else if (is_fs_special ())
	{
	  buf->st_dev = dev ();
	  buf->st_mode = dev ().mode;
	}
      else
	{
	  buf->st_mode |= S_IFREG;
	  if (pc.exec_state () == dont_know_if_executable)
	    {
	      DWORD cur, done;
	      char magic[3];

	      /* First retrieve current position, set to beginning
		 of file if not already there. */
	      cur = SetFilePointer (get_handle (), 0, NULL, FILE_CURRENT);
	      if (cur != INVALID_SET_FILE_POINTER
		  && (!cur || SetFilePointer (get_handle (), 0, NULL, FILE_BEGIN)
		      != INVALID_SET_FILE_POINTER))
		{
		  /* FIXME should we use /etc/magic ? */
		  magic[0] = magic[1] = magic[2] = '\0';
		  if (ReadFile (get_handle (), magic, 3, &done, NULL)
		      && has_exec_chars (magic, done))
		    {
		      pc.set_exec ();
		      buf->st_mode |= STD_XBITS;
		    }
		  (void) SetFilePointer (get_handle (), cur, NULL, FILE_BEGIN);
		}
	    }
	}

      if (pc.exec_state () == is_executable)
	buf->st_mode |= STD_XBITS;

      /* This fakes the permissions of all files to match the current umask. */
      buf->st_mode &= ~(cygheap->umask);
    }

 done:
  syscall_printf ("0 = fstat (, %p) st_atime=%x st_size=%D, st_mode=%p, st_ino=%d, sizeof=%d",
		  buf, buf->st_atime, buf->st_size, buf->st_mode,
		  (int) buf->st_ino, sizeof (*buf));
  return 0;
}

int __stdcall
fhandler_disk_file::fstat (struct __stat64 *buf)
{
  if (has_changed ())
    touch_ctime ();
  return fstat_fs (buf);
}

void
fhandler_disk_file::touch_ctime (void)
{
  SYSTEMTIME st;
  FILETIME ft;

  GetSystemTime (&st);
  SystemTimeToFileTime (&st, &ft);
  if (!SetFileTime (get_io_handle (), &ft, NULL, NULL))
    debug_printf ("SetFileTime (%s) failed, %E", get_win32_name ());
  else
    has_changed (false);
}

int __stdcall
fhandler_disk_file::fchmod (mode_t mode)
{
  extern int chmod_device (path_conv& pc, mode_t mode);
  int res = -1;
  int oret = 0;

  if (pc.is_fs_special ())
    return chmod_device (pc, mode);

  if (wincap.has_security ())
    {
      enable_restore_privilege ();
      if (!get_io_handle () && pc.has_acls ())
	{
	  query_open (query_write_control);
	  if (!(oret = open (O_BINARY, 0)))
	    return -1;
	}

      if (!allow_ntsec && allow_ntea) /* Not necessary when manipulating SD. */
	SetFileAttributes (pc, (DWORD) pc & ~FILE_ATTRIBUTE_READONLY);
      if (pc.isdir ())
	mode |= S_IFDIR;
      if (!set_file_attribute (pc.has_acls (), get_io_handle (), pc,
			       ILLEGAL_UID, ILLEGAL_GID, mode)
	  && allow_ntsec)
	res = 0;
    }

  /* if the mode we want has any write bits set, we can't be read only. */
  if (mode & (S_IWUSR | S_IWGRP | S_IWOTH))
    (DWORD) pc &= ~FILE_ATTRIBUTE_READONLY;
  else
    (DWORD) pc |= FILE_ATTRIBUTE_READONLY;

  if (!SetFileAttributes (pc, pc))
    __seterrno ();
  else if (!allow_ntsec || !pc.has_acls ())
    /* Correct NTFS security attributes have higher priority */
    res = 0;

  /* Set ctime on success. */
  if (!res)
    has_changed (true);

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

  enable_restore_privilege ();
  if (!get_io_handle ())
    {
      query_open (query_write_control);
      if (!(oret = open (O_BINARY, 0)))
	return -1;
    }

  mode_t attrib = 0;
  if (pc.isdir ())
    attrib |= S_IFDIR;
  int res = get_file_attribute (pc.has_acls (), get_io_handle (), pc, &attrib);
  if (!res)
    {
      res = set_file_attribute (pc.has_acls (), get_io_handle (), pc,
				uid, gid, attrib);
      /* Set ctime on success. */
      if (!res)
	has_changed (true);
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
      switch (cmd)
	{
	  struct __stat64 st;

	  case SETACL:
	    /* Open for writing required to be able to set ctime
	       (even though setting the ACL is just pretended). */
	    if (!get_io_handle ())
	      oret = open (O_WRONLY | O_BINARY, 0);
	    res = 0;
	    break;
	  case GETACL:
	    if (!aclbufp)
	      set_errno(EFAULT);
	    else if (nentries < MIN_ACL_ENTRIES)
	      set_errno (ENOSPC);
	    else
	      {
		if (!get_io_handle ())
		  {
		    query_open (query_read_control);
		    if (!(oret = open (O_BINARY, 0)))
		      return -1;
		  }
		if (!fstat_by_handle (&st))
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
      if (cmd == SETACL)
	enable_restore_privilege ();
      if (!get_io_handle ())
	{
	  query_open (cmd == SETACL ? query_write_control : query_read_control);
	  if (!(oret = open (O_BINARY, 0)))
	    return -1;
	}
      switch (cmd)
	{
	  case SETACL:
	    if (!aclsort32 (nentries, 0, aclbufp))
	      res = setacl (get_io_handle (), pc, nentries, aclbufp);
	    break;
	  case GETACL:
	    if (!aclbufp)
	      set_errno(EFAULT);
	    else
	      res = getacl (get_io_handle (), pc, pc, nentries, aclbufp);
	    break;
	  case GETACLCNT:
	    res = getacl (get_io_handle (), pc, pc, 0, NULL);
	  default:
	    set_errno (EINVAL);
	    break;
	}
    }

  /* Set ctime on success. */
  if (!res && cmd == SETACL)
    has_changed (true);

  if (oret)
    close ();

  return res;
}

int
fhandler_disk_file::ftruncate (_off64_t length)
{
  int res = -1, res_bug = 0;

  if (length < 0 || !get_output_handle ())
    set_errno (EINVAL);
  else if (pc.isdir ())
    set_errno (EISDIR);
  else if (!(get_access () & GENERIC_WRITE))
    set_errno (EBADF);
  else
    {
      _off64_t prev_loc = lseek (0, SEEK_CUR);
      if (lseek (length, SEEK_SET) >= 0)
        {
	  if (get_fs_flags (FILE_SUPPORTS_SPARSE_FILES))
	    {
	      _off64_t actual_length;
	      DWORD size_high = 0;
	      actual_length = GetFileSize (get_output_handle (), &size_high);
	      actual_length += ((_off64_t) size_high) << 32;
	      if (length >= actual_length + (128 * 1024))
	        {
		  DWORD dw;
		  BOOL r = DeviceIoControl (get_output_handle (),
					    FSCTL_SET_SPARSE, NULL, 0, NULL,
					    0, &dw, NULL);
		  syscall_printf ("%d = DeviceIoControl(%p, FSCTL_SET_SPARSE)",
		  		  r, get_output_handle ());
	        }
	    }
	  else if (wincap.has_lseek_bug ())
	    res_bug = write (&res, 0);
	  if (!SetEndOfFile (get_output_handle ()))
	    __seterrno ();
	  else
	    res = res_bug;
	  /* restore original file pointer location */
	  lseek (prev_loc, SEEK_SET);
	  /* Set ctime on success. */
	  if (!res)
	    has_changed (true);
	}
    }
  return res;
}

int
fhandler_disk_file::link (const char *newpath)
{
  path_conv newpc (newpath, PC_SYM_NOFOLLOW | PC_FULL | PC_POSIX);
  extern bool allow_winsymlinks;

  if (newpc.error)
    {
      set_errno (newpc.case_clash ? ECASECLASH : newpc.error);
      return -1;
    }

  if (newpc.exists ())
    {
      syscall_printf ("file '%s' exists?", (char *) newpc);
      set_errno (EEXIST);
      return -1;
    }

  if (newpc[strlen (newpc) - 1] == '.')
    {
      syscall_printf ("trailing dot, bailing out");
      set_errno (EINVAL);
      return -1;
    }

  /* Shortcut hack. */
  char new_lnk_buf[CYG_MAX_PATH + 5];
  if (allow_winsymlinks && pc.is_lnk_symlink () && !newpc.case_clash)
    {
      strcpy (new_lnk_buf, newpath);
      strcat (new_lnk_buf, ".lnk");
      newpath = new_lnk_buf;
      newpc.check (newpath, PC_SYM_NOFOLLOW | PC_FULL);
    }

  query_open (query_write_attributes);
  if (!open (O_BINARY, 0))
    {
      syscall_printf ("Opening file failed");
      __seterrno ();
      return -1;
    }

  /* Try to make hard link first on Windows NT */
  if (wincap.has_hard_links ())
    {
      if (CreateHardLinkA (newpc, pc, NULL))
	goto success;

      /* There are two cases to consider:
	 - The FS doesn't support hard links ==> ERROR_INVALID_FUNCTION
	   We copy the file.
	 - CreateHardLinkA is not supported  ==> ERROR_PROC_NOT_FOUND
	   In that case (<= NT4) we try the old-style method.
	 Any other error should be taken seriously. */
      if (GetLastError () == ERROR_INVALID_FUNCTION)
	{
	  syscall_printf ("FS doesn't support hard links: Copy file");
	  goto docopy;
	}
      if (GetLastError () != ERROR_PROC_NOT_FOUND)
	{
	  syscall_printf ("CreateHardLinkA failed");
	  __seterrno ();
	  close ();
	  return -1;
	}

      WIN32_STREAM_ID stream_id;
      LPVOID context;
      WCHAR wbuf[CYG_MAX_PATH];
      BOOL ret;
      DWORD written, write_err, path_len, size;

      path_len = sys_mbstowcs (wbuf, newpc, CYG_MAX_PATH) * sizeof (WCHAR);

      stream_id.dwStreamId = BACKUP_LINK;
      stream_id.dwStreamAttributes = 0;
      stream_id.dwStreamNameSize = 0;
      stream_id.Size.HighPart = 0;
      stream_id.Size.LowPart = path_len;
      size = sizeof (WIN32_STREAM_ID) - sizeof (WCHAR**)
	     + stream_id.dwStreamNameSize;
      context = NULL;
      write_err = 0;
      /* Write WIN32_STREAM_ID */
      ret = BackupWrite (get_handle (), (LPBYTE) &stream_id, size,	
			 &written, FALSE, FALSE, &context);
      if (ret)
	{
	  /* write the buffer containing the path */
	  /* FIXME: BackupWrite sometimes traps if linkname is invalid.
	     Need to handle. */
	  ret = BackupWrite (get_handle (), (LPBYTE) wbuf, path_len,
			     &written, FALSE, FALSE, &context);
	  if (!ret)
	    {
	      write_err = GetLastError ();
	      syscall_printf ("cannot write linkname, %E");
	    }
	  /* Free context */
	  BackupWrite (get_handle (), NULL, 0, &written,
		       TRUE, FALSE, &context);
	}
      else
	{
	  write_err = GetLastError ();
	  syscall_printf ("cannot write stream_id, %E");
	}

      if (!ret)
	{
	  /* Only copy file if FS doesn't support hard links */
	  if (write_err == ERROR_INVALID_FUNCTION)
	    {
	      syscall_printf ("FS doesn't support hard links: Copy file");
	      goto docopy;
	    }

	  close ();
	  __seterrno_from_win_error (write_err);
	  return -1;
	}

    success:
      /* Set ctime on success. */
      has_changed (true);
      close ();
      if (!allow_winsymlinks && pc.is_lnk_symlink ())
	SetFileAttributes (newpc, (DWORD) pc
				   | FILE_ATTRIBUTE_SYSTEM
				   | FILE_ATTRIBUTE_READONLY);
      return 0;
    }
docopy:
  /* do this with a copy */
  if (!CopyFileA (pc, newpc, 1))
    {
      __seterrno ();
      return -1;
    }
  /* Set ctime on success, also on the copy. */
  has_changed (true);
  close ();
  fhandler_disk_file fh (newpc);
  fh.query_open (query_write_attributes);
  if (fh.open (O_BINARY, 0))
    {
      fh.has_changed (true);
      fh.close ();
    }
  return 0;
}

int
fhandler_disk_file::utimes (const struct timeval *tvp)
{
  FILETIME lastaccess, lastwrite, lastchange;
  struct timeval tmp[2];

  query_open (query_write_attributes);
  if (!open (O_BINARY, 0))
    {
      /* It's documented in MSDN that FILE_WRITE_ATTRIBUTES is sufficient
         to change the timestamps.  Unfortunately it's not sufficient for a
	 remote HPFS which requires GENERIC_WRITE, so we just retry to open
	 for writing, though this fails for R/O files of course. */
      query_open (no_query);
      if (!open (O_WRONLY | O_BINARY, 0))
        {
	  syscall_printf ("Opening file failed");
	  __seterrno ();
	  if (pc.isdir ()) /* What we can do with directories more? */
	    return 0;
	    
	  __seterrno ();
	  return -1;
        }
    }

  gettimeofday (&tmp[0], 0);
  if (!tvp)
    {
      tmp[1] = tmp[0];
      tvp = tmp;
    }
  timeval_to_filetime (&tvp[0], &lastaccess);
  timeval_to_filetime (&tvp[1], &lastwrite);
  /* Update ctime */
  timeval_to_filetime (&tmp[0], &lastchange);
  debug_printf ("incoming lastaccess %08x %08x", tvp[0].tv_sec, tvp[0].tv_usec);
  if (!SetFileTime (get_handle (), &lastchange, &lastaccess, &lastwrite))
    {
      DWORD errcode = GetLastError ();
      close ();
      __seterrno_from_win_error (errcode);
      return -1;
    }
  close ();
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
  if (pc.isdir () && (flags & (O_WRONLY | O_RDWR)))
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

  /* Attributes may be set only if a file is _really_ created.
     This code is now only used for ntea here since the files
     security attributes are set in CreateFile () now. */
  if (flags & O_CREAT
      && GetLastError () != ERROR_ALREADY_EXISTS
      && !allow_ntsec && allow_ntea)
    set_file_attribute (false, NULL, get_win32_name (), mode);

  /* O_TRUNC on existing file requires setting ctime. */
  if ((flags & (O_CREAT | O_TRUNC)) == O_TRUNC)
    has_changed (true);

  set_fs_flags (pc.fs_flags ());

out:
  syscall_printf ("%d = fhandler_disk_file::open (%s, %p)", res,
		  get_win32_name (), flags);
  return res;
}

int
fhandler_disk_file::close ()
{
  /* Changing file data requires setting ctime. */
  if (has_changed ())
    touch_ctime ();
  return close_fs ();
}

int
fhandler_base::close_fs ()
{
  int res = fhandler_base::close ();
  if (!res)
    user_shared->delqueue.process_queue ();
  return res;
}

/* FIXME: The correct way to do this to get POSIX locking semantics is to
   keep a linked list of posix lock requests and map them into Win32 locks.
   he problem is that Win32 does not deal correctly with overlapping lock
   requests. Also another pain is that Win95 doesn't do non-blocking or
   non-exclusive locks at all. For '95 just convert all lock requests into
   blocking,exclusive locks.  This shouldn't break many apps but denying all
   locking would.  For now just convert to Win32 locks and hope for
   the best.  */

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
      /* CV, 2003-12-03: And yet another Win 9x bugginess.  For some reason
	 offset + length must be <= 0x100000000.  I'm using 0xffffffff as
	 upper border here, this should be sufficient. */
      len_low = UINT32_MAX - (wincap.lock_file_highword () ? 0 : off_low);
      len_high = wincap.lock_file_highword ();
    }
  else
    {
      len_low = (DWORD)(win32_len & UINT32_MAX);
      len_high = (DWORD)(win32_len >> 32);
    }

  BOOL res;

  if (wincap.has_lock_file_ex ())
    {
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
    }
  else
    {
      /* Windows 95 -- use primitive lock call */
      if (fl->l_type == F_UNLCK)
	res = UnlockFile (get_handle (), off_low, off_high, len_low, len_high);
      else
	res = LockFile (get_handle (), off_low, off_high, len_low, len_high);
    }

  if (res == 0)
    {
      __seterrno ();
      return -1;
    }

  return 0;
}

DIR *
fhandler_disk_file::opendir ()
{
  DIR *dir;
  DIR *res = NULL;
  size_t len;

  if (!pc.isdir ())
    set_errno (ENOTDIR);
  else if ((len = strlen (pc)) > CYG_MAX_PATH - 3)
    set_errno (ENAMETOOLONG);
  else if ((dir = (DIR *) malloc (sizeof (DIR))) == NULL)
    set_errno (ENOMEM);
  else if ((dir->__d_dirname = (char *) malloc (len + 3)) == NULL)
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
  else if (fhaccess (R_OK) != 0)
    goto free_dirent;
  else
    {
      strcpy (dir->__d_dirname, get_win32_name ());
      dir->__d_dirent->d_version = __DIRENT_VERSION;
      cygheap_fdnew fd;

      if (fd < 0)
	goto free_dirent;

      fd = this;
      fd->nohandle (true);
      dir->__d_dirent->d_fd = fd;
      dir->__fh = this;
      /* FindFirstFile doesn't seem to like duplicate /'s. */
      len = strlen (dir->__d_dirname);
      if (len == 0 || isdirsep (dir->__d_dirname[len - 1]))
	strcat (dir->__d_dirname, "*");
      else
	strcat (dir->__d_dirname, "\\*");  /**/
      dir->__d_cookie = __DIRENT_COOKIE;
      dir->__handle = INVALID_HANDLE_VALUE;
      dir->__d_position = 0;
      dir->__d_dirhash = get_namehash ();

      res = dir;

    }

  syscall_printf ("%p = opendir (%s)", res, get_name ());
  return res;

free_dirent:
  free (dir->__d_dirent);
free_dirname:
  free (dir->__d_dirname);
free_dir:
  free (dir);
  return res;
}

struct dirent *
fhandler_disk_file::readdir (DIR *dir)
{
  WIN32_FIND_DATA buf;
  HANDLE handle;
  struct dirent *res = NULL;

  if (dir->__handle == INVALID_HANDLE_VALUE
      && dir->__d_position == 0)
    {
      handle = FindFirstFileA (dir->__d_dirname, &buf);
      DWORD lasterr = GetLastError ();
      dir->__handle = handle;
      if (handle == INVALID_HANDLE_VALUE && (lasterr != ERROR_NO_MORE_FILES))
	{
	  seterrno_from_win_error (__FILE__, __LINE__, lasterr);
	  return res;
	}
    }
  else if (dir->__handle == INVALID_HANDLE_VALUE)
    return res;
  else if (!FindNextFileA (dir->__handle, &buf))
    {
      DWORD lasterr = GetLastError ();
      (void) FindClose (dir->__handle);
      dir->__handle = INVALID_HANDLE_VALUE;
      /* POSIX says you shouldn't set errno when readdir can't
	 find any more files; so, if another error we leave it set. */
      if (lasterr != ERROR_NO_MORE_FILES)
	  seterrno_from_win_error (__FILE__, __LINE__, lasterr);
      syscall_printf ("%p = readdir (%p)", res, dir);
      return res;
    }

  /* Check for Windows shortcut. If it's a Cygwin or U/WIN
     symlink, drop the .lnk suffix. */
  if (buf.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
    {
      char *c = buf.cFileName;
      int len = strlen (c);
      if (strcasematch (c + len - 4, ".lnk"))
	{
	  char fbuf[CYG_MAX_PATH + 1];
	  strcpy (fbuf, dir->__d_dirname);
	  strcpy (fbuf + strlen (fbuf) - 1, c);
	  path_conv fpath (fbuf, PC_SYM_NOFOLLOW);
	  if (fpath.issymlink () || fpath.isspecial ())
	    c[len - 4] = '\0';
	}
    }

  /* We get here if `buf' contains valid data.  */
  if (pc.isencoded ())
    (void) fnunmunge (dir->__d_dirent->d_name, buf.cFileName);
  else
    strcpy (dir->__d_dirent->d_name, buf.cFileName);

  dir->__d_position++;
  res = dir->__d_dirent;
  syscall_printf ("%p = readdir (%p) (%s)",
		  &dir->__d_dirent, dir, buf.cFileName);
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
      if (!readdir (dir))
	break;
}

void
fhandler_disk_file::rewinddir (DIR *dir)
{
  if (dir->__handle != INVALID_HANDLE_VALUE)
    {
      (void) FindClose (dir->__handle);
      dir->__handle = INVALID_HANDLE_VALUE;
    }
  dir->__d_position = 0;
}

int
fhandler_disk_file::closedir (DIR *dir)
{
  int res = 0;
  if (dir->__handle != INVALID_HANDLE_VALUE &&
      FindClose (dir->__handle) == 0)
    {
      __seterrno ();
      res = -1;
    }
  syscall_printf ("%d = closedir (%p)", res, dir);
  return 0;
}

fhandler_cygdrive::fhandler_cygdrive () :
  fhandler_disk_file (), ndrives (0), pdrive (NULL)
{
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
  if (!iscygdrive_root ())
    return fhandler_disk_file::fstat (buf);
  buf->st_mode = S_IFDIR | 0555;
  if (!ndrives)
    set_drives ();
  buf->st_nlink = ndrives + 2;
  return 0;
}

DIR *
fhandler_cygdrive::opendir ()
{
  DIR *dir;

  dir = fhandler_disk_file::opendir ();
  if (dir && iscygdrive_root () && !ndrives)
    set_drives ();

  return dir;
}

struct dirent *
fhandler_cygdrive::readdir (DIR *dir)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::readdir (dir);
  if (!pdrive || !*pdrive)
    return NULL;
  if (GetFileAttributes (pdrive) == INVALID_FILE_ATTRIBUTES)
    {
      pdrive = strchr (pdrive, '\0') + 1;
      return readdir (dir);
    }

  *dir->__d_dirent->d_name = cyg_tolower (*pdrive);
  dir->__d_dirent->d_name[1] = '\0';
  dir->__d_position++;
  pdrive = strchr (pdrive, '\0') + 1;
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
		  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

_off64_t
fhandler_cygdrive::telldir (DIR *dir)
{
  return fhandler_disk_file::telldir (dir);
}

void
fhandler_cygdrive::seekdir (DIR *dir, _off64_t loc)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::seekdir (dir, loc);

  for (pdrive = get_win32_name (), dir->__d_position = -1; *pdrive;
       pdrive = strchr (pdrive, '\0') + 1)
    if (++dir->__d_position >= loc)
      break;

  return;
}

void
fhandler_cygdrive::rewinddir (DIR *dir)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::rewinddir (dir);
  pdrive = get_win32_name ();
  dir->__d_position = 0;
  return;
}

int
fhandler_cygdrive::closedir (DIR *dir)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::closedir (dir);
  pdrive = get_win32_name ();
  return -1;
}
