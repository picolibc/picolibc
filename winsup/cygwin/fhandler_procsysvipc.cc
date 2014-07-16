/* fhandler_procsysvipc.cc: fhandler for /proc/sysvipc virtual filesystem

   Copyright 2011, 2012, 2013, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "cygserver.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "fhandler_virtual.h"
#include "pinfo.h"
#include "shared_info.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include <sys/param.h>
#include <ctype.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

#define _KERNEL
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

static off_t format_procsysvipc_msg (void *, char *&);
static off_t format_procsysvipc_sem (void *, char *&);
static off_t format_procsysvipc_shm (void *, char *&);

static const virt_tab_t procsysvipc_tab[] =
{
  { _VN ("."),		FH_PROCSYSVIPC,   virt_directory, NULL },
  { _VN (".."),		FH_PROCSYSVIPC,   virt_directory, NULL },
  { _VN ("msg"),	FH_PROCSYSVIPC,   virt_file,   format_procsysvipc_msg },
  { _VN ("sem"),	FH_PROCSYSVIPC,   virt_file,   format_procsysvipc_sem },
  { _VN ("shm"),	FH_PROCSYSVIPC,   virt_file,   format_procsysvipc_shm },
  { NULL, 0,		FH_NADA,		  virt_none,   NULL }
};

static const int PROCSYSVIPC_LINK_COUNT =
  (sizeof (procsysvipc_tab) / sizeof (virt_tab_t)) - 1;

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * -1 if path is a file.
 */
virtual_ftype_t
fhandler_procsysvipc::exists ()
{
  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + 1;
  while (*path != 0 && !isdirsep (*path))
    path++;
  if (*path == 0)
    return virt_rootdir;

  virt_tab_t *entry = virt_tab_search (path + 1, true, procsysvipc_tab,
				       PROCSYSVIPC_LINK_COUNT);

  cygserver_init ();

  if (entry)
    {
      if (entry->type == virt_file)
	{
	  if (cygserver_running != CYGSERVER_OK)
	    return virt_none;
	}
	  fileid = entry - procsysvipc_tab;
	  return entry->type;
	}
  return virt_none;
}

fhandler_procsysvipc::fhandler_procsysvipc ():
  fhandler_proc ()
{
}

int __reg2
fhandler_procsysvipc::fstat (struct stat *buf)
{
  fhandler_base::fstat (buf);
  buf->st_mode &= ~_IFMT & NO_W;
  int file_type = exists ();
  switch (file_type)
    {
    case virt_none:
      set_errno (ENOENT);
      return -1;
    case virt_directory:
    case virt_rootdir:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      buf->st_nlink = 2;
      return 0;
    case virt_file:
    default:
      buf->st_mode |= S_IFREG | S_IRUSR | S_IRGRP | S_IROTH;
      return 0;
    }
}

int
fhandler_procsysvipc::readdir (DIR *dir, dirent *de)
{
  int res = ENMFILE;
  if (dir->__d_position >= PROCSYSVIPC_LINK_COUNT)
    goto out;
  {
    cygserver_init ();
    if (cygserver_running != CYGSERVER_OK)
      goto out;
  }
  strcpy (de->d_name, procsysvipc_tab[dir->__d_position++].name);
  dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
  res = 0;
out:
  syscall_printf ("%d = readdir(%p, %p) (%s)", res, dir, de, de->d_name);
  return res;
}

int
fhandler_procsysvipc::open (int flags, mode_t mode)
{
  int res = fhandler_virtual::open (flags, mode);
  if (!res)
    goto out;

  nohandle (true);

  const char *path;
  path = get_name () + proc_len + 1;
  pid = atoi (path);
  while (*path != 0 && !isdirsep (*path))
    path++;

  if (*path == 0)
    {
      if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
	{
	  set_errno (EEXIST);
	  res = 0;
	  goto out;
	}
      else if (flags & O_WRONLY)
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

  virt_tab_t *entry;
  entry = virt_tab_search (path + 1, true, procsysvipc_tab, PROCSYSVIPC_LINK_COUNT);
  if (!entry)
    {
      set_errno ((flags & O_CREAT) ? EROFS : ENOENT);
      res = 0;
      goto out;
    }
  if (flags & O_WRONLY)
    {
      set_errno (EROFS);
      res = 0;
      goto out;
    }

  fileid = entry - procsysvipc_tab;
  if (!fill_filebuf ())
	{
	  res = 0;
	  goto out;
	}

  if (flags & O_APPEND)
    position = filesize;
  else
    position = 0;

success:
  res = 1;
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  set_open_status ();
out:
  syscall_printf ("%d = fhandler_proc::open(%p, 0%o)", res, flags, mode);
  return res;
}

bool
fhandler_procsysvipc::fill_filebuf ()
{
  if (procsysvipc_tab[fileid].format_func)
    {
      filesize = procsysvipc_tab[fileid].format_func (NULL, filebuf);
      return true;
    }
  return false;
}

#define MSG_HEADLINE "       key      msqid perms      cbytes       qnum lspid lrpid   uid   gid  cuid  cgid      stime      rtime      ctime\n"

static off_t
format_procsysvipc_msg (void *, char *&destbuf)
{
  char *buf;
  struct msginfo msginfo;
  struct msqid_ds *xmsqids;

  msgctl (0, IPC_INFO, (struct msqid_ds *) &msginfo);
  /* Don't use tmp_pathbuf.  The required buffer sizes can be up to 128K! */
  xmsqids = (struct msqid_ds *) malloc (sizeof (struct msqid_ds)
					* msginfo.msgmni);
  if (!xmsqids)
    return 0;
  /* buf size = sizeof headline + 128 bytes per msg queue entry. */
  buf = (char *) malloc (sizeof (MSG_HEADLINE) + msginfo.msgmni * 128);
  if (!buf)
    {
      free (xmsqids);
      return 0;
    }

  char *bufptr = stpcpy (buf, MSG_HEADLINE);
  msgctl (msginfo.msgmni, IPC_INFO, (struct msqid_ds *) xmsqids);
  for (int i = 0; i < msginfo.msgmni; i++)
    {
      if (xmsqids[i].msg_qbytes != 0)
	{
	   bufptr += sprintf (bufptr,
		     "%10llu %10u %5o %11u %10u %5d %5d %5u %5u %5u %5u "
		     "%10ld %10ld %10ld\n",
		     xmsqids[i].msg_perm.key,
		     IXSEQ_TO_IPCID(i, xmsqids[i].msg_perm),
		     xmsqids[i].msg_perm.mode,
		     xmsqids[i].msg_cbytes,
		     xmsqids[i].msg_qnum,
		     xmsqids[i].msg_lspid,
		     xmsqids[i].msg_lrpid,
		     (unsigned) xmsqids[i].msg_perm.uid,
		     (unsigned) xmsqids[i].msg_perm.gid,
		     (unsigned) xmsqids[i].msg_perm.cuid,
		     (unsigned) xmsqids[i].msg_perm.cgid,
		     xmsqids[i].msg_stime,
		     xmsqids[i].msg_rtime,
		     xmsqids[i].msg_ctime);
	}
      }

  off_t size = bufptr - buf;
  destbuf = (char *) crealloc_abort (destbuf, size);
  memcpy (destbuf, buf, size);
  free (buf);
  free (xmsqids);
  return size;
}

#undef MSG_HEADLINE

#define SEM_HEADLINE "       key      semid perms      nsems   uid   gid  cuid  cgid      otime      ctime\n"

static off_t
format_procsysvipc_sem (void *, char *&destbuf)
{
  char *buf;
  union semun semun;
  struct seminfo seminfo;
  struct semid_ds *xsemids;

  semun.buf = (struct semid_ds *) &seminfo;
  semctl (0, 0, IPC_INFO, semun);
  /* Don't use tmp_pathbuf.  The required buffer sizes can be up to 96K! */
  xsemids = (struct semid_ds *) malloc (sizeof (struct semid_ds)
					* seminfo.semmni);
  if (!xsemids)
    return 0;
  /* buf size = sizeof headline + 96 bytes per semaphore entry. */
  buf = (char *) malloc (sizeof (SEM_HEADLINE) + seminfo.semmni * 96);
  if (!buf)
    {
      free (xsemids);
      return 0;
    }

  char *bufptr = stpcpy (buf, SEM_HEADLINE);
  semun.buf = xsemids;
  semctl (seminfo.semmni, 0, IPC_INFO, semun);
  for (int i = 0; i < seminfo.semmni; i++)
    {
      if ((xsemids[i].sem_perm.mode & SEM_ALLOC) != 0)
	{
	  bufptr += sprintf (bufptr,
		    "%10llu %10u %5o %10d %5u %5u %5u %5u %10ld %10ld\n",
		    xsemids[i].sem_perm.key,
		    IXSEQ_TO_IPCID(i, xsemids[i].sem_perm),
		    xsemids[i].sem_perm.mode,
		    xsemids[i].sem_nsems,
		    (unsigned) xsemids[i].sem_perm.uid,
		    (unsigned) xsemids[i].sem_perm.gid,
		    (unsigned) xsemids[i].sem_perm.cuid,
		    (unsigned) xsemids[i].sem_perm.cgid,
		    xsemids[i].sem_otime,
		    xsemids[i].sem_ctime);
	}
    }

  off_t size = bufptr - buf;
  destbuf = (char *) crealloc_abort (destbuf, size);
  memcpy (destbuf, buf, size);
  free (buf);
  free (xsemids);
  return size;
}

#undef SEM_HEADLINE

#define SHM_HEADLINE "       key      shmid perms       size  cpid  lpid nattch   uid   gid  cuid  cgid      atime      dtime      ctime\n"

static off_t
format_procsysvipc_shm (void *, char *&destbuf)
{
  char *buf;
  struct shminfo shminfo;
  struct shmid_ds *xshmids;

  shmctl (0, IPC_INFO, (struct shmid_ds *) &shminfo);
  /* Don't use tmp_pathbuf.  The required buffer sizes can be up to 120K! */
  xshmids = (struct shmid_ds *) malloc (sizeof (struct shmid_ds)
					* shminfo.shmmni);
  if (!xshmids)
    return 0;
  /* buf size = sizeof headline + 120 bytes per shmem entry. */
  buf = (char *) malloc (sizeof (SHM_HEADLINE) + shminfo.shmmni * 120);
  if (!buf)
    {
      free (xshmids);
      return 0;
    }

  char *bufptr = stpcpy (buf, SHM_HEADLINE);
  shmctl (shminfo.shmmni, IPC_INFO, (struct shmid_ds *) xshmids);
  for (int i = 0; i < shminfo.shmmni; i++)
    {
      if (xshmids[i].shm_perm.mode & 0x0800)
	{
	  bufptr += sprintf (bufptr,
		    "%10llu %10u %5o %10u %5d %5d %6u %5u %5u %5u %5u "
		    "%10ld %10ld %10ld\n",
		    xshmids[i].shm_perm.key,
		    IXSEQ_TO_IPCID(i, xshmids[i].shm_perm),
		    xshmids[i].shm_perm.mode,
		    xshmids[i].shm_segsz,
		    xshmids[i].shm_cpid,
		    xshmids[i].shm_lpid,
		    xshmids[i].shm_nattch,
		    (unsigned) xshmids[i].shm_perm.uid,
		    (unsigned) xshmids[i].shm_perm.gid,
		    (unsigned) xshmids[i].shm_perm.cuid,
		    (unsigned) xshmids[i].shm_perm.cgid,
		    xshmids[i].shm_atime,
		    xshmids[i].shm_dtime,
		    xshmids[i].shm_ctime);
		    }
	  }

  off_t size = bufptr - buf;
  destbuf = (char *) crealloc_abort (destbuf, size);
  memcpy (destbuf, buf, size);
  free (buf);
  free (xshmids);
  return size;
}

#undef SHM_HEADLINE
