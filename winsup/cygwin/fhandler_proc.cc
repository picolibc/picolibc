/* fhandler_proc.cc: fhandler for /proc virtual filesystem

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
#include "path.h"
#include "sigproc.h"
#include "pinfo.h"
#include <assert.h>
#include <sys/utsname.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

/* offsets in proc_listing */
static const int PROC_REGISTRY = 0;     // /proc/registry
static const int PROC_VERSION = 1;      // /proc/version
static const int PROC_UPTIME = 2;       // /proc/uptime

/* names of objects in /proc */
static const char *proc_listing[] = {
  "registry",
  "version",
  "uptime",
  NULL
};

static const int PROC_LINK_COUNT = (sizeof(proc_listing) / sizeof(const char *)) - 1;

/* FH_PROC in the table below means the file/directory is handles by
 * fhandler_proc.
 */
static const DWORD proc_fhandlers[] = {
  FH_REGISTRY,
  FH_PROC,
  FH_PROC
};

/* name of the /proc filesystem */
const char proc[] = "/proc";
const int proc_len = sizeof (proc) - 1;

/* auxillary function that returns the fhandler associated with the given path
 * this is where it would be nice to have pattern matching in C - polymorphism
 * just doesn't cut it
 */
DWORD
fhandler_proc::get_proc_fhandler (const char *path)
{
  debug_printf ("get_proc_fhandler(%s)", path);
  path += proc_len;
  /* Since this method is called from path_conv::check we can't rely on
   * it being normalised and therefore the path may have runs of slashes
   * in it.
   */
  while (SLASH_P (*path))
    path++;

  /* Check if this is the root of the virtual filesystem (i.e. /proc).  */
  if (*path == 0)
    return FH_PROC;

  for (int i = 0; proc_listing[i]; i++)
    {
      if (path_prefix_p (proc_listing[i], path, strlen (proc_listing[i])))
        return proc_fhandlers[i];
    }

  int pid = atoi (path);
  winpids pids;
  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];

      if (!proc_exists (p))
        continue;

      if (p->pid == pid)
        return FH_PROCESS;
    }
  return FH_BAD;
}

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * <0 if path is a file.
 */
int
fhandler_proc::exists (const char *path)
{
  debug_printf ("exists (%s)", path);
  path += proc_len;
  if (*path == 0)
    return 2;
  for (int i = 0; proc_listing[i]; i++)
    if (pathmatch (path + 1, proc_listing[i]))
      return (proc_fhandlers[i] == FH_PROC) ? -1 : 1;
  return 0;
}

fhandler_proc::fhandler_proc ():
  fhandler_virtual (FH_PROC)
{
}

fhandler_proc::fhandler_proc (DWORD devtype):
  fhandler_virtual (devtype)
{
}

int
fhandler_proc::fstat (struct __stat64 *buf, path_conv *pc)
{
  debug_printf ("fstat (%s)", get_name ());
  const char *path = get_name ();
  path += proc_len;
  (void) fhandler_base::fstat (buf, pc);

  buf->st_mode &= ~_IFMT & NO_W;

  if (!*path)
    {
      buf->st_nlink = PROC_LINK_COUNT;
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      return 0;
    }
  else
    {
      path++;
      for (int i = 0; proc_listing[i]; i++)
        if (pathmatch (path, proc_listing[i]))
          {
	    if (proc_fhandlers[i] != FH_PROC)
	      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
	    else
	      {
		buf->st_mode &= NO_X;
		buf->st_mode |= S_IFREG;
	      }
            return 0;
          }
    }
  set_errno (ENOENT);
  return -1;
}

struct dirent *
fhandler_proc::readdir (DIR * dir)
{
  if (dir->__d_position >= PROC_LINK_COUNT)
    {
      winpids pids;
      int found = 0;
      for (unsigned i = 0; i < pids.npids; i++)
        {
          _pinfo *p = pids[i];

          if (!proc_exists (p))
            continue;

          if (found == dir->__d_position - PROC_LINK_COUNT)
            {
              __small_sprintf (dir->__d_dirent->d_name, "%d", p->pid);
              dir->__d_position++;
              return dir->__d_dirent;
            }
          found++;
        }
      return NULL;
    }

  strcpy (dir->__d_dirent->d_name, proc_listing[dir->__d_position++]);
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
                  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

int
fhandler_proc::open (path_conv *pc, int flags, mode_t mode)
{
  int proc_file_no = -1;

  int res = fhandler_virtual::open (pc, flags, mode);
  if (!res)
    goto out;

  const char *path;

  path = get_name () + proc_len;

  if (!*path)
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

  proc_file_no = -1;
  for (int i = 0; proc_listing[i]; i++)
    if (path_prefix_p (proc_listing[i], path + 1, strlen (proc_listing[i])))
      {
        proc_file_no = i;
        if (proc_fhandlers[i] != FH_PROC)
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
      }

  if (proc_file_no == -1)
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
  switch (proc_file_no)
    {
    case PROC_VERSION:
      {
        struct utsname uts_name;
        uname (&uts_name);
        filesize = bufalloc = strlen (uts_name.sysname) + 1 +
          strlen (uts_name.release) + 1 + strlen (uts_name.version) + 2;
        filebuf = new char[bufalloc];
        __small_sprintf (filebuf, "%s %s %s\n", uts_name.sysname,
                         uts_name.release, uts_name.version);
        break;
      }
    case PROC_UPTIME:
      {
        /* GetTickCount() wraps after 49 days - on WinNT/2000/XP, should use
         * perfomance counters but I don't know Redhat's policy on
         * NT only dependancies.
         */
        DWORD ticks = GetTickCount ();
        filebuf = new char[bufalloc = 40];
        __small_sprintf (filebuf, "%d.%02d\n", ticks / 1000,
                         (ticks / 10) % 100);
        filesize = strlen (filebuf);
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
