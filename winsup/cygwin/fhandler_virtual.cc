/* fhandler_virtual.cc: base fhandler class for virtual filesystems

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "shared_info.h"
#include "cygheap.h"
#include <assert.h>

#include <dirent.h>

fhandler_virtual::fhandler_virtual ():
  fhandler_base (), filebuf (NULL), bufalloc ((size_t) -1),
  fileid (-1)
{
}

fhandler_virtual::~fhandler_virtual ()
{
  if (filebuf)
    free (filebuf);
  filebuf = NULL;
}

void
fhandler_virtual::fixup_after_exec (HANDLE)
{
  if (filebuf)
    filebuf = NULL;
}

DIR *
fhandler_virtual::opendir ()
{
  DIR *dir;
  DIR *res = NULL;
  size_t len;

  if (exists () <= 0)
    set_errno (ENOTDIR);
  else if ((len = strlen (get_name ())) > CYG_MAX_PATH - 3)
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
      free (dir);
      set_errno (ENOMEM);
    }
  else
    {
      strcpy (dir->__d_dirname, get_name ());
      dir->__d_dirent->d_version = __DIRENT_VERSION;
      cygheap_fdnew fd;
      if (fd >= 0)
	{
	  fd = this;
	  fd->set_nohandle (true);
	  dir->__d_dirent->d_fd = fd;
	  dir->__fh = this;
	  dir->__d_cookie = __DIRENT_COOKIE;
	  dir->__handle = INVALID_HANDLE_VALUE;
	  dir->__d_position = 0;
	  dir->__d_dirhash = get_namehash ();

	  res = dir;
	}
    }

  syscall_printf ("%p = opendir (%s)", res, get_name ());
  return res;
}

_off64_t fhandler_virtual::telldir (DIR * dir)
{
  return dir->__d_position;
}

void
fhandler_virtual::seekdir (DIR * dir, _off64_t loc)
{
  dir->__d_position = loc;
  return;
}

void
fhandler_virtual::rewinddir (DIR * dir)
{
  dir->__d_position = 0;
  return;
}

int
fhandler_virtual::closedir (DIR * dir)
{
  return 0;
}

_off64_t
fhandler_virtual::lseek (_off64_t offset, int whence)
{
  /*
   * On Linux, when you lseek within a /proc file,
   * the contents of the file are updated.
   */
  if (!fill_filebuf ())
    return (_off64_t) -1;
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
      return (_off64_t) -1;
    }
  return position;
}

int
fhandler_virtual::dup (fhandler_base * child)
{
  int ret = fhandler_base::dup (child);

  if (!ret)
    {
      fhandler_virtual *fhproc_child = (fhandler_virtual *) child;
      fhproc_child->filebuf = (char *) malloc (filesize);
      fhproc_child->bufalloc = fhproc_child->filesize = filesize;
      fhproc_child->position = position;
      memcpy (fhproc_child->filebuf, filebuf, filesize);
      fhproc_child->set_flags (get_flags ());
    }
  return ret;
}

int
fhandler_virtual::close ()
{
  if (filebuf)
    free (filebuf);
  filebuf = NULL;
  bufalloc = (size_t) -1;
  user_shared->delqueue.process_queue ();
  return 0;
}

void
fhandler_virtual::read (void *ptr, size_t& len)
{
  if (len == 0)
    return;
  if (openflags & O_DIROPEN)
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

int
fhandler_virtual::write (const void *ptr, size_t len)
{
  set_errno (EACCES);
  return -1;
}

/* low-level open for all proc files */
int
fhandler_virtual::open (int flags, mode_t mode)
{
  set_r_binary (1);
  set_w_binary (1);

  /* what to do about symlinks? */
  set_symlink_p (false);
  set_execable_p (not_executable);
  set_socket_p (false);

  set_flags ((flags & ~O_TEXT) | O_BINARY);

  return 1;
}

int
fhandler_virtual::exists ()
{
  return 0;
}

bool
fhandler_virtual::fill_filebuf ()
{
  return true;
}
