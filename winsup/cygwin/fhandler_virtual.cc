/* fhandler_virtual.cc: base fhandler class for virtual filesystems

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <cygwin/acl.h>
#include <sys/statvfs.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sync.h"
#include "child_info.h"

#include <dirent.h>

fhandler_virtual::fhandler_virtual ():
  fhandler_base (), filebuf (NULL), fileid (-1)
{
}

fhandler_virtual::~fhandler_virtual ()
{
  if (filebuf)
    {
      cfree (filebuf);
      filebuf = NULL;
    }
}

void
fhandler_virtual::fixup_after_exec ()
{
}

DIR *
fhandler_virtual::opendir (int fd)
{
  DIR *dir;
  DIR *res = NULL;
  size_t len;

  if (!virt_ftype_isdir (exists ()))
    set_errno (ENOTDIR);
  else if ((len = strlen (get_name ())) > PATH_MAX - 3)
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
      strcpy (dir->__d_dirname, get_name ());
      dir->__d_dirent->__d_version = __DIRENT_VERSION;
      dir->__d_cookie = __DIRENT_COOKIE;
      dir->__handle = INVALID_HANDLE_VALUE;
      dir->__d_position = 0;
      dir->__flags = 0;

      if (fd >= 0)
	{
	  dir->__d_fd = fd;
	  dir->__fh = this;
	  res = dir;
	}
      else
	{
	  cygheap_fdnew cfd;
	  if (cfd >= 0 && open (O_RDONLY, 0))
	    {
	      cfd = this;
	      dir->__d_fd = cfd;
	      dir->__fh = this;
	      res = dir;
	    }
	}
      close_on_exec (true);
    }

  syscall_printf ("%p = opendir (%s)", res, get_name ());
  return res;
}

long
fhandler_virtual::telldir (DIR * dir)
{
  return dir->__d_position;
}

void
fhandler_virtual::seekdir (DIR * dir, long loc)
{
  dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
  dir->__d_position = loc;
}

void
fhandler_virtual::rewinddir (DIR * dir)
{
  dir->__d_position = 0;
  dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
}

int
fhandler_virtual::closedir (DIR * dir)
{
  return 0;
}

off_t
fhandler_virtual::lseek (off_t offset, int whence)
{
  /*
   * On Linux, when you lseek within a /proc file,
   * the contents of the file are updated.
   */
  if (!fill_filebuf ())
    return (off_t) -1;
  switch (whence)
    {
    case SEEK_SET:
      position = offset;
      break;
    case SEEK_CUR:
      position += offset;
      break;
    case SEEK_END:
      position = filesize + offset;
      break;
    default:
      set_errno (EINVAL);
      return (off_t) -1;
    }
  return position;
}

int
fhandler_virtual::dup (fhandler_base * child, int flags)
{
  int ret = fhandler_base::dup (child, flags);

  if (!ret)
    {
      fhandler_virtual *fhproc_child = (fhandler_virtual *) child;
      fhproc_child->filebuf = (char *) cmalloc_abort (HEAP_BUF, filesize);
      memcpy (fhproc_child->filebuf, filebuf, filesize);
    }
  return ret;
}

int
fhandler_virtual::close ()
{
  if (!have_execed)
    {
      if (filebuf)
	{
	  cfree (filebuf);
	  filebuf = NULL;
	}
    }
  return 0;
}

void
fhandler_virtual::read (void *ptr, size_t& len)
{
  if (len == 0)
    return;
  if (diropen)
    {
      set_errno (EISDIR);
      len = (size_t) -1;
      return;
    }
  if (!filebuf)
    {
      len = (size_t) 0;
      return;
    }
  if ((ssize_t) len > filesize - position)
    len = (size_t) (filesize - position);
  if ((ssize_t) len < 0)
    len = 0;
  else
    memcpy (ptr, filebuf + position, len);
  position += len;
}

ssize_t
fhandler_virtual::write (const void *ptr, size_t len)
{
  set_errno (EACCES);
  return -1;
}

/* low-level open for all proc files */
int
fhandler_virtual::open (int flags, mode_t mode)
{
  rbinary (true);
  wbinary (true);

  set_flags ((flags & ~O_TEXT) | O_BINARY);

  return 1;
}

virtual_ftype_t
fhandler_virtual::exists ()
{
  return virt_none;
}

bool
fhandler_virtual::fill_filebuf ()
{
  return true;
}

int
fhandler_virtual::fchmod (mode_t mode)
{
  /* Same as on Linux. */
  set_errno (EPERM);
  return -1;
}

int
fhandler_virtual::fchown (uid_t uid, gid_t gid)
{
  /* Same as on Linux. */
  set_errno (EPERM);
  return -1;
}

int
fhandler_virtual::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  int res = fhandler_base::facl (cmd, nentries, aclbufp);
  if (res >= 0 && cmd == GETACL)
    {
      aclbufp[0].a_perm = (S_IRUSR | (pc.isdir () ? S_IXUSR : 0)) >> 6;
      aclbufp[1].a_perm = (S_IRGRP | (pc.isdir () ? S_IXGRP : 0)) >> 3;
      aclbufp[2].a_perm = S_IROTH | (pc.isdir () ? S_IXOTH : 0);
    }
  return res;
}

int
fhandler_virtual::fstatvfs (struct statvfs *sfs)
{
  /* Virtual file system.  Just return an empty buffer with a few values
     set to something useful.  Just as on Linux. */
  memset (sfs, 0, sizeof (*sfs));
  sfs->f_bsize = sfs->f_frsize = 4096;
  sfs->f_flag = ST_RDONLY;
  sfs->f_namemax = NAME_MAX;
  return 0;
}
