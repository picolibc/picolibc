/* fhandler_disk_file.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <signal.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "cygwin/version.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "pinfo.h"
#include <assert.h>
#include <ctype.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static int __stdcall
num_entries (const char *win32_name)
{
  WIN32_FIND_DATA buf;
  HANDLE handle;
  char buf1[MAX_PATH];
  int count = 0;

  strcpy (buf1, win32_name);
  int len = strlen (buf1);
  if (len == 0 || isdirsep (buf1[len - 1]))
    strcat (buf1, "*");
  else
    strcat (buf1, "/*");	/* */

  handle = FindFirstFileA (buf1, &buf);

  if (handle == INVALID_HANDLE_VALUE)
    return 0;
  count ++;
  while (FindNextFileA (handle, &buf))
    {
      if ((buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	count ++;
    }
  FindClose (handle);
  return count;
}

int __stdcall
fhandler_disk_file::fstat_by_handle (struct __stat64 *buf, path_conv *pc)
{
  int res = 0;
  BY_HANDLE_FILE_INFORMATION local;

  /* NT 3.51 seems to have a bug when attempting to get vol serial
     numbers.  This loop gets around this. */
  for (int i = 0; i < 2; i++)
    {
      if (!(res = GetFileInformationByHandle (get_handle (), &local)))
	break;
      if (local.dwVolumeSerialNumber && (long) local.dwVolumeSerialNumber != -1)
	break;
    }
  debug_printf ("%d = GetFileInformationByHandle (%s, %d)",
		res, get_win32_name (), get_handle ());
  if (res == 0)
    /* GetFileInformationByHandle will fail if it's given stdin/out/err
       or a pipe*/
    {
      memset (&local, 0, sizeof (local));
      local.nFileSizeLow = GetFileSize (get_handle (), &local.nFileSizeHigh);
    }

  return fstat_helper (buf, pc,
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
fhandler_disk_file::fstat_by_name (struct __stat64 *buf, path_conv *pc)
{
  int res;
  HANDLE handle;
  WIN32_FIND_DATA local;

  if (!pc->exists ())
    {
      debug_printf ("already determined that pc does not exist");
      set_errno (ENOENT);
      res = -1;
    }
  else
    {
      char drivebuf[5];
      char *name;
      if ((*pc)[3] != '\0' || !isalpha ((*pc)[0]) || (*pc)[1] != ':' || (*pc)[2] != '\\')
	name = *pc;
      else
	{
	  /* FIXME: Does this work on empty disks? */
	  drivebuf[0] = (*pc)[0];
	  drivebuf[1] = (*pc)[1];
	  drivebuf[2] = (*pc)[2];
	  drivebuf[3] = '*';
	  drivebuf[4] = '\0';
	  name = drivebuf;
	}

      if ((handle = FindFirstFile (name, &local)) == INVALID_HANDLE_VALUE)
      {
	debug_printf ("FindFirstFile failed for '%s', %E", name);
	__seterrno ();
	res = -1;
      }
    else
      {
	FindClose (handle);
	res = fstat_helper (buf, pc,
			    local.ftCreationTime,
			    local.ftLastAccessTime,
			    local.ftLastWriteTime,
			    local.nFileSizeHigh,
			    local.nFileSizeLow);
      }
    }
  return res;
}

int __stdcall
fhandler_disk_file::fstat (struct __stat64 *buf, path_conv *pc)
{
  int res = -1;
  int oret;
  __uid32_t uid;
  __gid32_t gid;
  int open_flags = O_RDONLY | O_BINARY | O_DIROPEN;
  bool query_open_already;

  if (get_io_handle ())
    {
      if (get_nohandle ())
	return fstat_by_name (buf, pc);
      else
	return fstat_by_handle (buf, pc);
    }
  /* If we don't care if the file is executable or we already know if it is,
     then just do a "query open" as it is apparently much faster. */
  if (pc->exec_state () != dont_know_if_executable)
    set_query_open (query_open_already = true);
  else
    query_open_already = false;

  if (query_open_already && strncasematch (pc->volname (), "FAT", 3)
      && !strpbrk (get_win32_name (), "?*|<>"))
    oret = 0;
  else if (!(oret = open (pc, open_flags, 0)))
    {
      int ntsec_atts = 0;
      /* If we couldn't open the file, try a "query open" with no permissions.
	 This will allow us to determine *some* things about the file, at least. */
      set_query_open (true);
      if (!query_open_already && (oret = open (pc, open_flags, 0)))
	/* ok */;
      else if (allow_ntsec && pc->has_acls () && get_errno () == EACCES
		&& !get_file_attribute (TRUE, get_win32_name (), &ntsec_atts, &uid, &gid)
		&& !ntsec_atts && uid == myself->uid && gid == myself->gid)
	{
	  /* Check a special case here. If ntsec is ON it happens
	     that a process creates a file using mode 000 to disallow
	     other processes access. In contrast to UNIX, this results
	     in a failing open call in the same process. Check that
	     case. */
	  set_file_attribute (TRUE, get_win32_name (), 0400);
	  oret = open (pc, open_flags, 0);
	  set_file_attribute (TRUE, get_win32_name (), ntsec_atts);
	}
    }

  if (!oret || get_nohandle ())
    res = fstat_by_name (buf, pc);
  else
    {
      res = fstat_by_handle (buf, pc);
      close ();
    }

  return res;
}

int __stdcall
fhandler_disk_file::fstat_helper (struct __stat64 *buf, path_conv *pc,
				  FILETIME ftCreationTime,
				  FILETIME ftLastAccessTime,
				  FILETIME ftLastWriteTime,
				  DWORD nFileSizeHigh,
				  DWORD nFileSizeLow,
				  DWORD nFileIndexHigh,
				  DWORD nFileIndexLow,
				  DWORD nNumberOfLinks)
{
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
  buf->st_dev = pc->volser ();
  buf->st_size = ((__off64_t)nFileSizeHigh << 32) + nFileSizeLow;
  /* Unfortunately the count of 2 confuses `find (1)' command. So
     let's try it with `1' as link count. */
  if (pc->isdir () && !pc->isremote () && nNumberOfLinks == 1)
    buf->st_nlink = num_entries (pc->get_win32 ());
  else
    buf->st_nlink = nNumberOfLinks;

  /* Assume that if a drive has ACL support it MAY have valid "inodes".
     It definitely does not have valid inodes if it does not have ACL
     support. */
  switch (pc->has_acls () && (nFileIndexHigh || nFileIndexLow)
	  ? pc->drive_type () : DRIVE_UNKNOWN)
    {
    case DRIVE_FIXED:
    case DRIVE_REMOVABLE:
    case DRIVE_CDROM:
    case DRIVE_RAMDISK:
      /* Although the documentation indicates otherwise, it seems like
	 "inodes" on these devices are persistent, at least across reboots. */
      buf->st_ino = nFileIndexHigh | nFileIndexLow;
      break;
    default:
      /* Either the nFileIndex* fields are unreliable or unavailable.  Use the
	 next best alternative. */
      buf->st_ino = get_namehash ();
      break;
    }

  buf->st_blksize = S_BLKSIZE;
  buf->st_blocks  = (buf->st_size + S_BLKSIZE - 1) / S_BLKSIZE;

  buf->st_mode = 0;
  /* Using a side effect: get_file_attibutes checks for
     directory. This is used, to set S_ISVTX, if needed.  */
  if (pc->isdir ())
    buf->st_mode = S_IFDIR;
  else if (pc->issymlink ())
    buf->st_mode = S_IFLNK;
  else if (pc->issocket ())
    buf->st_mode = S_IFSOCK;

  __uid32_t uid;
  __gid32_t gid;
  if (get_file_attribute (pc->has_acls (), get_win32_name (), &buf->st_mode,
			  &uid, &gid) == 0)
    {
      /* If read-only attribute is set, modify ntsec return value */
      if (pc->has_attribute (FILE_ATTRIBUTE_READONLY) && !get_symlink_p ())
	buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);

      if (!(buf->st_mode & S_IFMT))
	buf->st_mode |= S_IFREG;
    }
  else
    {
      buf->st_mode |= STD_RBITS;

      if (!pc->has_attribute (FILE_ATTRIBUTE_READONLY))
	buf->st_mode |= STD_WBITS;
      /* | S_IWGRP | S_IWOTH; we don't give write to group etc */

      if (buf->st_mode & S_IFDIR)
	buf->st_mode |= S_IFDIR | STD_XBITS;
      else if (buf->st_mode & S_IFMT)
	/* nothing */;
      else if (pc->issocket ())
	buf->st_mode |= S_IFSOCK;
      else
	{
	  buf->st_mode |= S_IFREG;
	  if (pc->exec_state () == dont_know_if_executable)
	    {
	      DWORD cur, done;
	      char magic[3];

	      /* First retrieve current position, set to beginning
		 of file if not already there. */
	      cur = SetFilePointer (get_handle (), 0, NULL, FILE_CURRENT);
	      if (cur != INVALID_SET_FILE_POINTER &&
		  (!cur ||
		   SetFilePointer (get_handle (), 0, NULL, FILE_BEGIN)
		   != INVALID_SET_FILE_POINTER))
		{
		  /* FIXME should we use /etc/magic ? */
		  magic[0] = magic[1] = magic[2] = '\0';
		  if (ReadFile (get_handle (), magic, 3, &done, NULL) &&
		      has_exec_chars (magic, done))
		    {
		      set_execable_p ();
		      pc->set_exec ();
		    }
		  SetFilePointer (get_handle (), cur, NULL, FILE_BEGIN);
		}
	    }
	  if (pc->exec_state () == is_executable)
	    buf->st_mode |= STD_XBITS;
	}
    }

  buf->st_uid = uid;
  buf->st_gid = gid;

  /* The number of links to a directory includes the
     number of subdirectories in the directory, since all
     those subdirectories point to it.
     This is too slow on remote drives, so we do without it and
     set the number of links to 2. */

  syscall_printf ("0 = fstat (, %p) st_atime=%x st_size=%D, st_mode=%p, st_ino=%d, sizeof=%d",
		  buf, buf->st_atime, buf->st_size, buf->st_mode,
		  (int) buf->st_ino, sizeof (*buf));
  return 0;
}

fhandler_disk_file::fhandler_disk_file (DWORD devtype) :
  fhandler_base (devtype)
{
}

fhandler_disk_file::fhandler_disk_file () :
  fhandler_base (FH_DISK)
{
}

int
fhandler_disk_file::open (path_conv *real_path, int flags, mode_t mode)
{
  if (real_path->case_clash && flags & O_CREAT)
    {
      debug_printf ("case clash detected");
      set_errno (ECASECLASH);
      return 0;
    }

  set_has_acls (real_path->has_acls ());
  set_isremote (real_path->isremote ());

  int res = this->fhandler_base::open (real_path, flags | O_DIROPEN, mode);
  if (!res)
    goto out;

  /* This is for file systems known for having a buggy CreateFile call
     which might return a valid HANDLE without having actually opened
     the file.
     The only known file system to date is the SUN NFS Solstice Client 3.1
     which returns a valid handle when trying to open a file in a nonexistent
     directory. */
  if (real_path->has_buggy_open () && !real_path->exists ())
    {
      debug_printf ("Buggy open detected.");
      close ();
      set_errno (ENOENT);
      return 0;
    }

  set_symlink_p (real_path->issymlink ());
  set_execable_p (real_path->exec_state ());
  set_socket_p (real_path->issocket ());

out:
  syscall_printf ("%d = fhandler_disk_file::open (%s, %p)", res,
		  get_win32_name (), flags);
  return res;
}

int
fhandler_disk_file::close ()
{
  int res = this->fhandler_base::close ();
  if (!res)
    cygwin_shared->delqueue.process_queue ();
  return res;
}

/*
 * FIXME !!!
 * The correct way to do this to get POSIX locking
 * semantics is to keep a linked list of posix lock
 * requests and map them into Win32 locks. The problem
 * is that Win32 does not deal correctly with overlapping
 * lock requests. Also another pain is that Win95 doesn't do
 * non-blocking or non exclusive locks at all. For '95 just
 * convert all lock requests into blocking,exclusive locks.
 * This shouldn't break many apps but denying all locking
 * would.
 * For now just convert to Win32 locks and hope for the best.
 */

int
fhandler_disk_file::lock (int cmd, struct flock *fl)
{
  __off64_t win32_start;
  int win32_len;
  DWORD win32_upper;
  __off64_t startpos;

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
	  startpos = ((__off64_t)finfo.nFileSizeHigh << 32)
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

  /*
   * Special case if len == 0 for POSIX means lock
   * to the end of the entire file (and all future extensions).
   */
  if (win32_len == 0)
    {
      win32_len = 0xffffffff;
      win32_upper = wincap.lock_file_highword ();
    }
  else
    win32_upper = 0;

  BOOL res;

  if (wincap.has_lock_file_ex ())
    {
      DWORD lock_flags = (cmd == F_SETLK) ? LOCKFILE_FAIL_IMMEDIATELY : 0;
      lock_flags |= (fl->l_type == F_WRLCK) ? LOCKFILE_EXCLUSIVE_LOCK : 0;

      OVERLAPPED ov;

      ov.Internal = 0;
      ov.InternalHigh = 0;
      ov.Offset = (DWORD)win32_start;
      ov.OffsetHigh = 0;
      ov.hEvent = (HANDLE) 0;

      if (fl->l_type == F_UNLCK)
	{
	  res = UnlockFileEx (get_handle (), 0, (DWORD)win32_len, win32_upper, &ov);
	}
      else
	{
	  res = LockFileEx (get_handle (), lock_flags, 0, (DWORD)win32_len,
							win32_upper, &ov);
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
	res = UnlockFile (get_handle (), (DWORD)win32_start, 0, (DWORD)win32_len,
							win32_upper);
      else
	res = LockFile (get_handle (), (DWORD)win32_start, 0, (DWORD)win32_len, win32_upper);
    }

  if (res == 0)
    {
      __seterrno ();
      return -1;
    }

  return 0;
}

DIR *
fhandler_disk_file::opendir (path_conv& real_name)
{
  DIR *dir;
  DIR *res = NULL;
  size_t len;

  if (!real_name.isdir ())
    set_errno (ENOTDIR);
  else if ((len = strlen (real_name))> MAX_PATH - 3)
    set_errno (ENAMETOOLONG);
  else if ((dir = (DIR *) malloc (sizeof (DIR))) == NULL)
    set_errno (ENOMEM);
  else if ((dir->__d_dirname = (char *) malloc (len + 3)) == NULL)
    {
      free (dir);
      set_errno (ENOMEM);
    }
  else if ((dir->__d_dirent =
	    (struct dirent *) malloc (sizeof (struct dirent))) == NULL)
    {
      free (dir->__d_dirname);
      free (dir);
      set_errno (ENOMEM);
    }
  else
    {
      strcpy (dir->__d_dirname, real_name.get_win32 ());
      dir->__d_dirent->d_version = __DIRENT_VERSION;
      cygheap_fdnew fd;
      fd = this;
      fd->set_nohandle (true);
      dir->__d_dirent->d_fd = fd;
      dir->__d_u.__d_data.__fh = this;
      /* FindFirstFile doesn't seem to like duplicate /'s. */
      len = strlen (dir->__d_dirname);
      if (len == 0 || SLASH_P (dir->__d_dirname[len - 1]))
	strcat (dir->__d_dirname, "*");
      else
	strcat (dir->__d_dirname, "\\*");  /**/
      dir->__d_cookie = __DIRENT_COOKIE;
      dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
      dir->__d_position = 0;
      dir->__d_dirhash = get_namehash ();

      res = dir;
    }

  syscall_printf ("%p = opendir (%s)", res, get_name ());
  return res;
}

struct dirent *
fhandler_disk_file::readdir (DIR *dir)
{
  WIN32_FIND_DATA buf;
  HANDLE handle;
  struct dirent *res = NULL;

  if (dir->__d_u.__d_data.__handle == INVALID_HANDLE_VALUE
      && dir->__d_position == 0)
    {
      handle = FindFirstFileA (dir->__d_dirname, &buf);
      DWORD lasterr = GetLastError ();
      dir->__d_u.__d_data.__handle = handle;
      if (handle == INVALID_HANDLE_VALUE && (lasterr != ERROR_NO_MORE_FILES))
	{
	  seterrno_from_win_error (__FILE__, __LINE__, lasterr);
	  return res;
	}
    }
  else if (dir->__d_u.__d_data.__handle == INVALID_HANDLE_VALUE)
    {
      return res;
    }
  else if (!FindNextFileA (dir->__d_u.__d_data.__handle, &buf))
    {
      DWORD lasterr = GetLastError ();
      (void) FindClose (dir->__d_u.__d_data.__handle);
      dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
      /* POSIX says you shouldn't set errno when readdir can't
	 find any more files; so, if another error we leave it set. */
      if (lasterr != ERROR_NO_MORE_FILES)
	  seterrno_from_win_error (__FILE__, __LINE__, lasterr);
      syscall_printf ("%p = readdir (%p)", res, dir);
      return res;
    }

  /* We get here if `buf' contains valid data.  */
  strcpy (dir->__d_dirent->d_name, buf.cFileName);

  /* Check for Windows shortcut. If it's a Cygwin or U/WIN
     symlink, drop the .lnk suffix. */
  if (buf.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
    {
      char *c = dir->__d_dirent->d_name;
      int len = strlen (c);
      if (strcasematch (c + len - 4, ".lnk"))
	{
	  char fbuf[MAX_PATH + 1];
	  strcpy (fbuf, dir->__d_dirname);
	  strcpy (fbuf + strlen (fbuf) - 1, dir->__d_dirent->d_name);
	  path_conv fpath (fbuf, PC_SYM_NOFOLLOW);
	  if (fpath.issymlink ())
	    c[len - 4] = '\0';
	}
    }

  dir->__d_position++;
  res = dir->__d_dirent;
  syscall_printf ("%p = readdir (%p) (%s)",
		  &dir->__d_dirent, dir, buf.cFileName);
  return res;
}

__off64_t
fhandler_disk_file::telldir (DIR *dir)
{
  return dir->__d_position;
}

void
fhandler_disk_file::seekdir (DIR *dir, __off64_t loc)
{
    rewinddir (dir);
    while (loc > dir->__d_position)
      if (!readdir (dir))
	break;
}

void
fhandler_disk_file::rewinddir (DIR *dir)
{
  if (dir->__d_u.__d_data.__handle != INVALID_HANDLE_VALUE)
    {
      (void) FindClose (dir->__d_u.__d_data.__handle);
      dir->__d_u.__d_data.__handle = INVALID_HANDLE_VALUE;
    }
  dir->__d_position = 0;
}

int
fhandler_disk_file::closedir (DIR *dir)
{
  int res = 0;
  if (dir->__d_u.__d_data.__handle != INVALID_HANDLE_VALUE &&
      FindClose (dir->__d_u.__d_data.__handle) == 0)
    {
      __seterrno ();
      res = -1;
    }
  syscall_printf ("%d = closedir (%p)", res, dir);
  return 0;
}

fhandler_cygdrive::fhandler_cygdrive (int unit) :
  fhandler_disk_file (FH_CYGDRIVE), unit (unit), ndrives (0), pdrive (NULL)
{
}

#define DRVSZ sizeof ("x:\\")
void
fhandler_cygdrive::set_drives ()
{
  const int len = 1 + 26 * DRVSZ;
  char *p = (char *) crealloc ((void *) win32_path_name,
				sizeof (".") + sizeof ("..") + len);

  win32_path_name = pdrive = p;
  strcpy (p, ".");
  strcpy (p + sizeof ("."), "..");
  p += sizeof (".") + sizeof ("..");
  ndrives = GetLogicalDriveStrings (len, p) / DRVSZ;
}

int
fhandler_cygdrive::fstat (struct __stat64 *buf, path_conv *pc)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::fstat (buf, pc);
  buf->st_mode = S_IFDIR | 0555;
  if (!ndrives)
    set_drives ();
  buf->st_nlink = ndrives;
  return 0;
}

DIR *
fhandler_cygdrive::opendir (path_conv& real_name)
{
  DIR *dir;

  dir = fhandler_disk_file::opendir (real_name);
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
    {
      set_errno (ENMFILE);
      return NULL;
    }
  else if (dir->__d_position > 1
	   && GetFileAttributes (pdrive) == INVALID_FILE_ATTRIBUTES)
    {
      pdrive = strchr (pdrive, '\0') + 1;
      return readdir (dir);
    }
  else if (*pdrive == '.')
    strcpy (dir->__d_dirent->d_name, pdrive);
  else
    {
      *dir->__d_dirent->d_name = cyg_tolower (*pdrive);
      dir->__d_dirent->d_name[1] = '\0';
    }
  dir->__d_position++;
  pdrive = strchr (pdrive, '\0') + 1;
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
		  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

__off64_t
fhandler_cygdrive::telldir (DIR *dir)
{
  return fhandler_disk_file::telldir (dir);
}

void
fhandler_cygdrive::seekdir (DIR *dir, __off64_t loc)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::seekdir (dir, loc);

  for (pdrive = win32_path_name, dir->__d_position = -1; *pdrive;
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
  pdrive = win32_path_name;
  dir->__d_position = 0;
  return;
}

int
fhandler_cygdrive::closedir (DIR *dir)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::closedir (dir);
  pdrive = win32_path_name;
  return -1;
}
