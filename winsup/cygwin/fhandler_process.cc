/* fhandler_process.cc: fhandler for /proc/<pid> virtual filesystem

   Copyright 2002 Red Hat, Inc.

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
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "sigproc.h"
#include "pinfo.h"
#include "path.h"
#include "shared_info.h"
#include <assert.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static const int PROCESS_PPID = 0;
static const int PROCESS_EXENAME = 1;
static const int PROCESS_WINPID = 2;
static const int PROCESS_WINEXENAME = 3;
static const int PROCESS_STATUS = 4;
static const int PROCESS_UID = 5;
static const int PROCESS_GID = 6;
static const int PROCESS_PGID = 7;
static const int PROCESS_SID = 8;
static const int PROCESS_CTTY = 9;

static const char *process_listing[] = {
  "ppid",
  "exename",
  "winpid",
  "winexename",
  "status",
  "uid",
  "gid",
  "pgid",
  "sid",
  "ctty",
  NULL
};

static const int PROCESS_LINK_COUNT = (sizeof(process_listing) / sizeof(const char *)) - 1;

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * <0 if path is a file.
 */
int
fhandler_process::exists (const char *path)
{
  debug_printf ("exists (%s)", path);
  path += proc_len + 1;
  while (*path != 0 && !SLASH_P (*path))
    path++;
  if (*path == 0)
    return 2;

  for (int i = 0; process_listing[i]; i++)
    if (pathmatch (path + 1, process_listing[i]))
      return -1;
  return 1;
}

fhandler_process::fhandler_process ():
  fhandler_proc (FH_PROCESS)
{
}

int
fhandler_process::fstat (struct __stat64 *buf, path_conv *pc)
{
  int file_type = exists (get_name ());
  (void) fhandler_base::fstat (buf, pc);
  buf->st_mode &= ~_IFMT & NO_W;

  switch (file_type)
    {
    case 0:
      set_errno (ENOENT);
      return -1;
    case 1:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      return 0;
    case 2:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      buf->st_nlink = PROCESS_LINK_COUNT;
      return 0;
    default:
    case -1:
      buf->st_mode |= S_IFREG;
      return 0;
    }
}

struct dirent *
fhandler_process::readdir (DIR * dir)
{
  if (dir->__d_position >= PROCESS_LINK_COUNT)
    return NULL;
  strcpy (dir->__d_dirent->d_name, process_listing[dir->__d_position++]);
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
                  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

int
fhandler_process::open (path_conv *pc, int flags, mode_t mode)
{
  int process_file_no = -1, pid;
  winpids pids;
  _pinfo *p;

  int res = fhandler_virtual::open (pc, flags, mode);
  if (!res)
    goto out;

  const char *path;
  path = get_name () + proc_len + 1;
  pid = atoi (path);
  while (*path != 0 && !SLASH_P (*path))
    path++;

  if (*path == 0)
    {
      if ((mode & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
        {
          set_errno (EEXIST);
          res = 0;
          goto out;
        }
      else if (mode & O_WRONLY)
        {
          set_errno (EISDIR);
          res = 0;
          goto out;
        }
      else
        {
          flags |= O_DIROPEN;
          goto success;
        }
    }

  process_file_no = -1;
  for (int i = 0; process_listing[i]; i++)
    {
      if (path_prefix_p
          (process_listing[i], path + 1, strlen (process_listing[i])))
        process_file_no = i;
    }
  if (process_file_no == -1)
    {
      if ((mode & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
        {
          set_errno (EROFS);
          res = 0;
          goto out;
        }
      else
        {
          set_errno (ENOENT);
          res = 0;
          goto out;
        }
    }
  if (mode & O_WRONLY)
    {
      set_errno (EROFS);
      res = 0;
      goto out;
    }
  for (unsigned i = 0; i < pids.npids; i++)
    {
      p = pids[i];

      if (!proc_exists (p))
        continue;

      if (p->pid == pid)
        goto found;
    }
  set_errno (ENOENT);
  res = 0;
  goto out;
found:
  switch (process_file_no)
    {
    case PROCESS_UID:
    case PROCESS_GID:
    case PROCESS_PGID:
    case PROCESS_SID:
    case PROCESS_CTTY:
    case PROCESS_PPID:
      {
        filebuf = new char[bufalloc = 40];
        int num;
        switch (process_file_no)
          {
          case PROCESS_PPID:
            num = p->ppid;
            break;
          case PROCESS_UID:
            num = p->uid;
            break;
          case PROCESS_PGID:
            num = p->pgid;
            break;
          case PROCESS_SID:
            num = p->sid;
            break;
	  default:
          case PROCESS_CTTY:
            num = p->ctty;
            break;
          }
        __small_sprintf (filebuf, "%d\n", num);
        filesize = strlen (filebuf);
        break;
      }
    case PROCESS_EXENAME:
      {
        filebuf = new char[bufalloc = MAX_PATH];
        if (p->process_state & (PID_ZOMBIE | PID_EXITED))
          strcpy (filebuf, "<defunct>");
        else
          {
            mount_table->conv_to_posix_path (p->progname, filebuf, 1);
            int len = strlen (filebuf);
            if (len > 4)
              {
                char *s = filebuf + len - 4;
                if (strcasecmp (s, ".exe") == 0)
                  *s = 0;
              }
          }
        filesize = strlen (filebuf);
        break;
      }
    case PROCESS_WINPID:
      {
        filebuf = new char[bufalloc = 40];
        __small_sprintf (filebuf, "%d\n", p->dwProcessId);
        filesize = strlen (filebuf);
        break;
      }
    case PROCESS_WINEXENAME:
      {
        int len = strlen (p->progname);
        filebuf = new char[len + 2];
        strcpy (filebuf, p->progname);
        filebuf[len] = '\n';
        filesize = len + 1;
        break;
      }
    case PROCESS_STATUS:
      {
        filebuf = new char[bufalloc = 3];
        filebuf[0] = ' ';
        filebuf[1] = '\n';
        filebuf[2] = 0;
        if (p->process_state & PID_STOPPED)
          filebuf[0] = 'S';
        else if (p->process_state & PID_TTYIN)
          filebuf[0] = 'I';
        else if (p->process_state & PID_TTYOU)
          filebuf[0] = 'O';
        filesize = 2;
        break;
      }
    }

  if (flags & O_APPEND)
    position = filesize;
  else
    position = 0;

success:
  res = 1;
  set_open_status ();
  set_flags (flags);
out:
  syscall_printf ("%d = fhandler_proc::open (%p, %d)", res, flags, mode);
  return res;
}
