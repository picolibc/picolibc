/* fhandler_disk_file.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/fcntl.h>
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
#include "sigproc.h"
#include "pinfo.h"
#include <assert.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static int
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

int
fhandler_disk_file::fstat (struct stat *buf, path_conv *pc)
{
  int res = -1;
  int oret;
  uid_t uid;
  gid_t gid;
  int open_flags = O_RDONLY | O_BINARY | O_DIROPEN;

  if (!pc)
    return fstat_helper (buf);

  if ((oret = open (pc, open_flags, 0)))
    /* ok */;
  else
    {
      int ntsec_atts = 0;
      /* If we couldn't open the file, try a "query open" with no permissions.
	 This will allow us to determine *some* things about the file, at least. */
      set_query_open (TRUE);
      if ((oret = open (pc, open_flags, 0)))
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
  if (oret)
    {
      res = fstat_helper (buf);
      /* The number of links to a directory includes the
	 number of subdirectories in the directory, since all
	 those subdirectories point to it.
	 This is too slow on remote drives, so we do without it and
	 set the number of links to 2. */
      /* Unfortunately the count of 2 confuses `find (1)' command. So
	 let's try it with `1' as link count. */
      if (pc->isdir ())
	buf->st_nlink = (pc->isremote ()
			 ? 1 : num_entries (pc->get_win32 ()));
      close ();
    }
  else if (pc->exists ())
    {
      /* Unfortunately, the above open may fail if the file exists, though.
	 So we have to care for this case here, too. */
      WIN32_FIND_DATA wfd;
      HANDLE handle;
      buf->st_nlink = 1;
      if (pc->isdir () && pc->isremote ())
	buf->st_nlink = num_entries (pc->get_win32 ());
      buf->st_dev = FHDEVN (FH_DISK) << 8;
      buf->st_ino = hash_path_name (0, pc->get_win32 ());
      if (pc->isdir ())
	buf->st_mode = S_IFDIR;
      else if (pc->issymlink ())
	buf->st_mode = S_IFLNK;
      else if (pc->issocket ())
	buf->st_mode = S_IFSOCK;
      else
	buf->st_mode = S_IFREG;
      if (!pc->has_acls ()
	  || get_file_attribute (TRUE, pc->get_win32 (),
				 &buf->st_mode,
				 &buf->st_uid, &buf->st_gid))
	{
	  buf->st_mode |= STD_RBITS | STD_XBITS;
	  if (!(pc->has_attribute (FILE_ATTRIBUTE_READONLY)))
	    buf->st_mode |= STD_WBITS;
	  if (pc->issymlink ())
	    buf->st_mode |= S_IRWXU | S_IRWXG | S_IRWXO;
	  get_file_attribute (FALSE, pc->get_win32 (),
			      NULL, &buf->st_uid, &buf->st_gid);
	}
      if ((handle = FindFirstFile (pc->get_win32 (), &wfd))
	  != INVALID_HANDLE_VALUE)
	{
	  /* This is for FAT filesystems, which don't support atime/ctime */
	  if (wfd.ftLastAccessTime.dwLowDateTime == 0
	      && wfd.ftLastAccessTime.dwHighDateTime == 0)
	    wfd.ftLastAccessTime = wfd.ftLastWriteTime;
	  if (wfd.ftCreationTime.dwLowDateTime == 0
	      && wfd.ftCreationTime.dwHighDateTime == 0)
	    wfd.ftCreationTime = wfd.ftLastWriteTime;

	  buf->st_atime   = to_time_t (&wfd.ftLastAccessTime);
	  buf->st_mtime   = to_time_t (&wfd.ftLastWriteTime);
	  buf->st_ctime   = to_time_t (&wfd.ftCreationTime);
	  buf->st_size    = wfd.nFileSizeLow;
	  buf->st_blksize = S_BLKSIZE;
	  buf->st_blocks  = ((unsigned long) buf->st_size +
			    S_BLKSIZE-1) / S_BLKSIZE;
	  FindClose (handle);
	}
      res = 0;
    }

  return res;
}

int
fhandler_disk_file::fstat_helper (struct stat *buf)
{
  int res = 0;	// avoid a compiler warning
  BY_HANDLE_FILE_INFORMATION local;
  save_errno saved_errno;

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
    {
      /* GetFileInformationByHandle will fail if it's given stdin/out/err
	 or a pipe*/
      DWORD lsize, hsize;

      if (GetFileType (get_handle ()) != FILE_TYPE_DISK)
	buf->st_mode = S_IFCHR;

      lsize = GetFileSize (get_handle (), &hsize);
      if (lsize == 0xffffffff && GetLastError () != NO_ERROR)
	buf->st_mode = S_IFCHR;
      else
	buf->st_size = lsize;
      /* We expect these to fail! */
      buf->st_mode |= STD_RBITS | STD_WBITS;
      buf->st_blksize = S_BLKSIZE;
      buf->st_ino = get_namehash ();
      syscall_printf ("0 = fstat (, %p)",  buf);
      return 0;
    }

  if (!get_win32_name ())
    {
      saved_errno.set (ENOENT);
      return -1;
    }

  /* This is for FAT filesystems, which don't support atime/ctime */
  if (local.ftLastAccessTime.dwLowDateTime == 0
      && local.ftLastAccessTime.dwHighDateTime == 0)
    local.ftLastAccessTime = local.ftLastWriteTime;
  if (local.ftCreationTime.dwLowDateTime == 0
      && local.ftCreationTime.dwHighDateTime == 0)
    local.ftCreationTime = local.ftLastWriteTime;

  buf->st_atime   = to_time_t (&local.ftLastAccessTime);
  buf->st_mtime   = to_time_t (&local.ftLastWriteTime);
  buf->st_ctime   = to_time_t (&local.ftCreationTime);
  buf->st_nlink   = local.nNumberOfLinks;
  buf->st_dev     = local.dwVolumeSerialNumber;
  buf->st_size    = local.nFileSizeLow;

  /* Allocate some place to determine the root directory. Need to allocate
     enough so that rootdir can add a trailing slash if path starts with \\. */
  char root[strlen (get_win32_name ()) + 3];
  strcpy (root, get_win32_name ());

  /* Assume that if a drive has ACL support it MAY have valid "inodes".
     It definitely does not have valid inodes if it does not have ACL
     support. */
  switch (has_acls () ? GetDriveType (rootdir (root)) : DRIVE_UNKNOWN)
    {
    case DRIVE_FIXED:
    case DRIVE_REMOVABLE:
    case DRIVE_CDROM:
    case DRIVE_RAMDISK:
      /* Although the documentation indicates otherwise, it seems like
	 "inodes" on these devices are persistent, at least across reboots. */
      buf->st_ino = local.nFileIndexHigh | local.nFileIndexLow;
      break;
    default:
      /* Either the nFileIndex* fields are unreliable or unavailable.  Use the
	 next best alternative. */
      buf->st_ino = get_namehash ();
      break;
    }

  buf->st_blksize = S_BLKSIZE;
  buf->st_blocks  = ((unsigned long) buf->st_size + S_BLKSIZE-1) / S_BLKSIZE;

  buf->st_mode = 0;
  /* Using a side effect: get_file_attibutes checks for
     directory. This is used, to set S_ISVTX, if needed.  */
  if (local.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    buf->st_mode = S_IFDIR;
  else if (get_symlink_p ())
    buf->st_mode = S_IFLNK;
  else if (get_socket_p ())
    buf->st_mode = S_IFSOCK;
  if (get_file_attribute (has_acls (), get_win32_name (), &buf->st_mode,
			  &buf->st_uid, &buf->st_gid) == 0)
    {
      /* If read-only attribute is set, modify ntsec return value */
      if ((local.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
	  && !get_symlink_p ())
	buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);

      if (!(buf->st_mode & S_IFMT))
	buf->st_mode |= S_IFREG;
    }
  else
    {
      buf->st_mode |= STD_RBITS;

      if (!(local.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
	buf->st_mode |= STD_WBITS;
      /* | S_IWGRP | S_IWOTH; we don't give write to group etc */

      if (buf->st_mode & S_IFDIR)
	buf->st_mode |= S_IFDIR | STD_XBITS;
      else if (buf->st_mode & S_IFMT)
	/* nothing */;
      else if (get_socket_p ())
	buf->st_mode |= S_IFSOCK;
      else
	switch (GetFileType (get_handle ()))
	  {
	  case FILE_TYPE_CHAR:
	  case FILE_TYPE_UNKNOWN:
	    buf->st_mode |= S_IFCHR;
	    break;
	  case FILE_TYPE_DISK:
	    buf->st_mode |= S_IFREG;
	    if (!dont_care_if_execable () && !get_execable_p ())
	      {
		DWORD cur, done;
		char magic[3];

		/* First retrieve current position, set to beginning
		   of file if not already there. */
		cur = SetFilePointer (get_handle(), 0, NULL, FILE_CURRENT);
		if (cur != INVALID_SET_FILE_POINTER &&
		    (!cur ||
		     SetFilePointer (get_handle(), 0, NULL, FILE_BEGIN)
		     != INVALID_SET_FILE_POINTER))
		  {
		    /* FIXME should we use /etc/magic ? */
		    magic[0] = magic[1] = magic[2] = '\0';
		    if (ReadFile (get_handle (), magic, 3, &done, NULL) &&
			has_exec_chars (magic, done))
			set_execable_p ();
		    SetFilePointer (get_handle(), cur, NULL, FILE_BEGIN);
		  }
	      }
	    if (get_execable_p ())
	      buf->st_mode |= STD_XBITS;
	    break;
	  case FILE_TYPE_PIPE:
	    buf->st_mode |= S_IFSOCK;
	    break;
	  }
    }

  syscall_printf ("0 = fstat (, %p) st_atime=%x st_size=%d, st_mode=%p, st_ino=%d, sizeof=%d",
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

  if (real_path->isbinary ())
    {
      set_r_binary (1);
      set_w_binary (1);
    }

  set_has_acls (real_path->has_acls ());
  set_isremote (real_path->isremote ());

  if (real_path->isdir ())
    flags |= O_DIROPEN;

  int res = this->fhandler_base::open (real_path, flags, mode);

  if (!res)
    goto out;

  /* This is for file systems known for having a buggy CreateFile call
     which might return a valid HANDLE without having actually opened
     the file.
     The only known file system to date is the SUN NFS Solstice Client 3.1
     which returns a valid handle when trying to open a file in a nonexistent
     directory. */
  if (real_path->has_buggy_open ()
      && GetFileAttributes (win32_path_name) == INVALID_FILE_ATTRIBUTES)
    {
      debug_printf ("Buggy open detected.");
      close ();
      set_errno (ENOENT);
      return 0;
    }

  if (flags & O_APPEND)
    SetFilePointer (get_handle(), 0, 0, FILE_END);

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
  int win32_start;
  int win32_len;
  DWORD win32_upper;
  DWORD startpos;

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
	if ((off_t) (startpos = lseek (0, SEEK_CUR)) == (off_t)-1)
	  return -1;
	break;
      case SEEK_END:
	{
	  BY_HANDLE_FILE_INFORMATION finfo;
	  if (GetFileInformationByHandle (get_handle(), &finfo) == 0)
	    {
	      __seterrno ();
	      return -1;
	    }
	  startpos = finfo.nFileSizeLow; /* Nowhere to keep high word */
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

  /* Compute d_ino by combining filename hash with the directory hash
     (which was stored in dir->__d_dirhash when opendir was called). */
  if (buf.cFileName[0] == '.')
    {
      if (buf.cFileName[1] == '\0')
	dir->__d_dirent->d_ino = dir->__d_dirhash;
      else if (buf.cFileName[1] != '.' || buf.cFileName[2] != '\0')
	goto hashit;
      else
	{
	  char *p, up[strlen (dir->__d_dirname) + 1];
	  strcpy (up, dir->__d_dirname);
	  if (!(p = strrchr (up, '\\')))
	    goto hashit;
	  *p = '\0';
	  if (!(p = strrchr (up, '\\')))
	    dir->__d_dirent->d_ino = hash_path_name (0, ".");
	  else
	    {
	      *p = '\0';
	      dir->__d_dirent->d_ino = hash_path_name (0, up);
	    }
	}
    }
  else
    {
  hashit:
      ino_t dino = hash_path_name (dir->__d_dirhash, "\\");
      dir->__d_dirent->d_ino = hash_path_name (dino, buf.cFileName);
    }

  dir->__d_position++;
  res = dir->__d_dirent;
  syscall_printf ("%p = readdir (%p) (%s)",
		  &dir->__d_dirent, dir, buf.cFileName);
  return res;
}

off_t
fhandler_disk_file::telldir (DIR *dir)
{
  return dir->__d_position;
}

void
fhandler_disk_file::seekdir (DIR *dir, off_t loc)
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
  win32_path_name = (char *) crealloc (win32_path_name, len);

  ndrives = GetLogicalDriveStrings (len, win32_path_name) / DRVSZ;
  pdrive = win32_path_name;
}

int
fhandler_cygdrive::fstat (struct stat *buf, path_conv *pc)
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
  if (GetFileAttributes (pdrive) == INVALID_FILE_ATTRIBUTES)
    {
      pdrive += DRVSZ;
      return readdir (dir);
    }
  *dir->__d_dirent->d_name = cyg_tolower (*pdrive);
  dir->__d_dirent->d_name[1] = '\0';
  dir->__d_position++;
  pdrive += DRVSZ;
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
      		  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

off_t
fhandler_cygdrive::telldir (DIR *dir)
{
  return fhandler_disk_file::telldir (dir);
}

void
fhandler_cygdrive::seekdir (DIR *dir, off_t loc)
{
  if (!iscygdrive_root ())
    return fhandler_disk_file::seekdir (dir, loc);

  for (pdrive = win32_path_name, dir->__d_position = -1; *pdrive; pdrive += DRVSZ)
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
