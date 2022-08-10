/* posix_ipc.cc: POSIX IPC API for Cygwin.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "shared_info.h"
#include "thread.h"
#include "path.h"
#include "cygtls.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "ntdll.h"
#include "tls_pbuf.h"
#include <io.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <semaphore.h>

/* The prefix_len is the length of the path prefix including trailing "/"
   (or "/sem." for semaphores) as well as the trailing NUL. */
static struct
{
  const char *prefix;
  const size_t prefix_len;
  const char *description;
} ipc_names[] = {
  { "/dev/shm", 10, "POSIX shared memory object" },
  { "/dev/mqueue", 13, "POSIX message queue" },
  { "/dev/shm", 14, "POSIX semaphore" }
};

enum ipc_type_t
{
  shmem,
  mqueue,
  semaphore
};

static bool
check_path (char *res_name, ipc_type_t type, const char *name, size_t len)
{
  /* Note that we require the existance of the appropriate /dev subdirectories
     for POSIX IPC object support, similar to Linux (which supports the
     directories, but doesn't require to mount them).  We don't create
     these directory here, that's the task of the installer.  But we check
     for existance and give ample warning. */
  path_conv path (ipc_names[type].prefix, PC_SYM_NOFOLLOW);
  if (path.error || !path.exists () || !path.isdir ())
    {
      small_printf (
	"Warning: '%s' does not exists or is not a directory.\n\n"
	"%ss require the existance of this directory.\n"
	"Create the directory '%s' and set the permissions to 01777.\n"
	"For instance on the command line: mkdir -m 01777 %s\n",
	ipc_names[type].prefix, ipc_names[type].description,
	ipc_names[type].prefix, ipc_names[type].prefix);
      set_errno (EINVAL);
      return false;
    }
  /* Apart from handling backslash like slash, the naming rules are identical
     to Linux, including the names and requirements for subdirectories, if
     the name contains further slashes. */
  /* Name must not be empty and has to start with a slash (or backslash) */
  if (!name || !strchr ("/\\", name[0]))
    {
      debug_printf ("Invalid %s name '%s'", ipc_names[type].description, name);
      set_errno (EINVAL);
      return false;
    }
  /* Name must not consist of just a single slash (or backslash) */
  if (!name[1])
    {
      debug_printf ("Invalid %s name '%s'", ipc_names[type].description, name);
      set_errno (ENOENT);
      return false;
    }
  /* Name must not contain slashes after the leading one */
  if (strpbrk (name + 1, "/\\"))
    {
      debug_printf ("Invalid %s name '%s'", ipc_names[type].description, name);
      set_errno (EACCES);
      return false;
    }
  /* Length must be less than or equal to NAME_MAX, or NAME_MAX - 4 in
     case of semaphores, due to the leading "sem." prefix */
  if (len > NAME_MAX - (type == semaphore ? strlen ("sem.") : 0))
    {
      debug_printf ("%s name '%s' too long", ipc_names[type].description, name);
      set_errno (ENAMETOOLONG);
      return false;
    }
  __small_sprintf (res_name, "%s/%s%s", ipc_names[type].prefix,
					type == semaphore ? "sem." : "",
					name + 1);
  return true;
}

class ipc_flock
{
  struct flock fl;

public:
  ipc_flock () { memset (&fl, 0, sizeof fl); }

  int lock (int fd, size_t size)
  {
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = size;
    return fcntl (fd, F_SETLKW, &fl);
  }
  int unlock (int fd)
  {
    if (!fl.l_len)
      return 0;
    fl.l_type = F_UNLCK;
    return fcntl (fd, F_SETLKW, &fl);
  }
};

/* POSIX shared memory object implementation. */

extern "C" int
shm_open (const char *name, int oflag, mode_t mode)
{
  size_t len = strlen (name);
  char shmname[ipc_names[shmem].prefix_len + len];

  if (!check_path (shmname, shmem, name, len))
    return -1;

  /* Check for valid flags. */
  if (((oflag & O_ACCMODE) != O_RDONLY && (oflag & O_ACCMODE) != O_RDWR)
      || (oflag & ~(O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC)))
    {
      debug_printf ("Invalid oflag 0%o", oflag);
      set_errno (EINVAL);
      return -1;
    }

  return open (shmname, oflag | O_CLOEXEC, mode & 0777);
}

extern "C" int
shm_unlink (const char *name)
{
  size_t len = strlen (name);
  char shmname[ipc_names[shmem].prefix_len + len];

  if (!check_path (shmname, shmem, name, len))
    return -1;

  return unlink (shmname);
}

/* The POSIX message queue implementation is based on W. Richard STEVENS
   implementation, just tweaked for Cygwin.  The main change is
   the usage of Windows mutexes and events instead of using the pthread
   synchronization objects.  The pathname is massaged so that the
   files are created under /dev/mqueue.  mq_timedsend and mq_timedreceive
   are implemented additionally. */

extern "C" mqd_t
mq_open (const char *name, int oflag, ...)
{
  va_list ap;
  mode_t mode = 0;
  fhandler_mqueue *fh = NULL;
  struct mq_attr *attr = NULL;

  size_t len = strlen (name);
  char mqname[ipc_names[mqueue].prefix_len + len];

  if (!check_path (mqname, mqueue, name, len))
    return (mqd_t) -1;

  __try
    {
      if (oflag & O_CREAT)
	{
	  va_start (ap, oflag);		/* init ap to final named argument */
	  mode = va_arg (ap, mode_t) & ~S_IXUSR;
	  attr = va_arg (ap, struct mq_attr *);
	  va_end (ap);
	}

      /* Create file descriptor for mqueue */
      cygheap_fdnew fd;

      if (fd < 0)
	__leave;
      fh = (fhandler_mqueue *) build_fh_name (mqname,
					      PC_OPEN | PC_POSIX
					      | PC_SYM_NOFOLLOW | PC_NULLEMPTY,
					      NULL);
      if (!fh)
	__leave;

      if (fh->mq_open (oflag, mode, attr))
	{
	  fd = fh;
	  return (mqd_t) fd;
	}
    }
  __except (EFAULT) {}
  __endtry
  if (fh)
    delete fh;
  return (mqd_t) -1;
}

extern "C" int
mq_getattr (mqd_t mqd, struct mq_attr *mqstat)
{
  int ret = -1;

  cygheap_fdget fd ((int) mqd, true);
  fhandler_mqueue *fh = fd->is_mqueue ();
  if (!fh)
    set_errno (EBADF);
  else
    ret = fh->mq_getattr (mqstat);
  return ret;
}

extern "C" int
mq_setattr (mqd_t mqd, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
  int ret = -1;

  cygheap_fdget fd ((int) mqd, true);
  fhandler_mqueue *fh = fd->is_mqueue ();
  if (!fh)
    set_errno (EBADF);
  else
    ret = fh->mq_setattr (mqstat, omqstat);
  return ret;
}

extern "C" int
mq_notify (mqd_t mqd, const struct sigevent *notification)
{
  int ret = -1;

  cygheap_fdget fd ((int) mqd, true);
  fhandler_mqueue *fh = fd->is_mqueue ();
  if (!fh)
    set_errno (EBADF);
  else
    ret = fh->mq_notify (notification);
  return ret;
}

extern "C" int
mq_timedsend (mqd_t mqd, const char *ptr, size_t len, unsigned int prio,
	      const struct timespec *abstime)
{
  int ret = -1;

  cygheap_fdget fd ((int) mqd, true);
  fhandler_mqueue *fh = fd->is_mqueue ();
  if (!fh)
    set_errno (EBADF);
  else
    ret = fh->mq_timedsend (ptr, len, prio, abstime);
  return ret;
}

extern "C" int
mq_send (mqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
  return mq_timedsend (mqd, ptr, len, prio, NULL);
}

extern "C" ssize_t
mq_timedreceive (mqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop,
	     const struct timespec *abstime)
{
  int ret = -1;

  cygheap_fdget fd ((int) mqd, true);
  fhandler_mqueue *fh = fd->is_mqueue ();
  if (!fh)
    set_errno (EBADF);
  else
    ret = fh->mq_timedrecv (ptr, maxlen, priop, abstime);
  return ret;
}

extern "C" ssize_t
mq_receive (mqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop)
{
  return mq_timedreceive (mqd, ptr, maxlen, priop, NULL);
}

extern "C" int
mq_close (mqd_t mqd)
{
  __try
    {
      cygheap_fdget fd ((int) mqd, true);
      if (!fd->is_mqueue ())
	{
	  set_errno (EBADF);
	  __leave;
	}

      if (mq_notify (mqd, NULL))	/* unregister calling process */
	__leave;

      fd->isclosed (true);
      fd->close ();
      fd.release ();
      return 0;
    }
  __except (EBADF) {}
  __endtry
  return -1;
}

extern "C" int
mq_unlink (const char *name)
{
  size_t len = strlen (name);
  char mqname[ipc_names[mqueue].prefix_len + len];

  if (!check_path (mqname, mqueue, name, len))
    return -1;
  if (unlink (mqname) == -1)
    return -1;
  return 0;
}

/* POSIX named semaphore implementation.  Loosely based on Richard W. STEPHENS
   implementation as far as sem_open is concerned, but under the hood using
   the already existing semaphore class in thread.cc.  Using a file backed
   solution allows to implement kernel persistent named semaphores.  */

#define	 MAX_TRIES	10	/* for waiting for initialization */

struct sem_finfo
{
  unsigned int       value;
  unsigned long long hash;
  LUID               luid;
};

extern "C" sem_t *
sem_open (const char *name, int oflag, ...)
{
  int i, fd = -1, created = 0;
  va_list ap;
  mode_t mode = 0;
  unsigned int value = 0;
  struct stat statbuff;
  sem_t *sem = SEM_FAILED;
  sem_finfo sf;
  bool wasopen = false;
  ipc_flock file;

  size_t len = strlen (name);
  char semname[ipc_names[semaphore].prefix_len + len];

  if (!check_path (semname, semaphore, name, len))
    return SEM_FAILED;

  __try
    {
      oflag &= (O_CREAT | O_EXCL);

    again:
      if (oflag & O_CREAT)
	{
	  va_start (ap, oflag);		/* init ap to final named argument */
	  mode = va_arg (ap, mode_t) & ~S_IXUSR;
	  value = va_arg (ap, unsigned int);
	  va_end (ap);

	  /* Open and specify O_EXCL and user-execute */
	  fd = open (semname, oflag | O_EXCL | O_RDWR | O_CLOEXEC,
		     mode | S_IXUSR);
	  if (fd < 0)
	    {
	      if (errno == EEXIST && (oflag & O_EXCL) == 0)
		goto exists;		/* already exists, OK */
	      return SEM_FAILED;
	    }
	  created = 1;
	  /* First one to create the file initializes it. */
	  NtAllocateLocallyUniqueId (&sf.luid);
	  sf.value = value;
	  sf.hash = hash_path_name (0, semname);
	  if (write (fd, &sf, sizeof sf) != sizeof sf)
	    __leave;
	  sem = semaphore::open (sf.hash, sf.luid, fd, oflag, mode, value,
				 wasopen);
	  if (sem == SEM_FAILED)
	    __leave;
	  /* Initialization complete, turn off user-execute bit */
	  if (fchmod (fd, mode) == -1)
	    __leave;
	  /* Don't close (fd); */
	  return sem;
	}

    exists:
      /* Open the file and fetch the semaphore name. */
      if ((fd = open (semname, O_RDWR | O_CLOEXEC)) < 0)
	{
	  if (errno == ENOENT && (oflag & O_CREAT))
	    goto again;
	  __leave;
	}
      /* Make certain initialization is complete */
      for (i = 0; i < MAX_TRIES; i++)
	{
	  if (stat (semname, &statbuff) == -1)
	    {
	      if (errno == ENOENT && (oflag & O_CREAT))
		{
		  close (fd);
		  fd = -1;
		  goto again;
		}
	      __leave;
	    }
	  if ((statbuff.st_mode & S_IXUSR) == 0)
	    break;
	  sleep (1);
	}
      if (i == MAX_TRIES)
	{
	  set_errno (ETIMEDOUT);
	  __leave;
	}
      if (file.lock (fd, sizeof sf))
	__leave;
      if (read (fd, &sf, sizeof sf) != sizeof sf)
	__leave;
      sem = semaphore::open (sf.hash, sf.luid, fd, oflag, mode, sf.value,
			     wasopen);
      file.unlock (fd);
      if (sem == SEM_FAILED)
	__leave;
      /* If wasopen is set, the semaphore was already opened and we already have
	 an open file descriptor pointing to the file.  This means, we have to
	 close the file descriptor created in this call.  It won't be stored
	 anywhere anyway. */
      if (wasopen)
	close (fd);
      return sem;
    }
  __except (EFAULT) {}
  __endtry
  /* Don't let following function calls change errno */
  save_errno save;

  if (fd >= 0)
    file.unlock (fd);
  if (created)
    unlink (semname);
  if (sem != SEM_FAILED)
    semaphore::close (sem);
  if (fd >= 0)
    close (fd);
  return SEM_FAILED;
}

extern "C" off_t lseek (int, off_t, int);

int
_sem_close (sem_t *sem, bool do_close)
{
  sem_finfo sf;
  int fd, ret = -1;
  ipc_flock file;

  if (semaphore::getinternal (sem, &fd, &sf.hash, &sf.luid, &sf.value) == -1)
    return -1;
  if (!file.lock (fd, sizeof sf)
      && lseek (fd, 0LL, SEEK_SET) != (off_t) -1
      && write (fd, &sf, sizeof sf) == sizeof sf)
    ret = do_close ? semaphore::close (sem) : 0;

  /* Don't let following function calls change errno */
  save_errno save;
  file.unlock (fd);
  close (fd);

  return ret;
}

extern "C" int
sem_close (sem_t *sem)
{
  return _sem_close (sem, true);
}

extern "C" int
sem_unlink (const char *name)
{
  size_t len = strlen (name);
  char semname[ipc_names[semaphore].prefix_len + len];

  if (!check_path (semname, semaphore, name, len))
    return -1;
  if (unlink (semname) == -1)
    return -1;
  return 0;
}

extern "C" int
sem_init (sem_t * sem, int pshared, unsigned int value)
{
  return semaphore::init (sem, pshared, value);
}

extern "C" int
sem_destroy (sem_t * sem)
{
  return semaphore::destroy (sem);
}

extern "C" int
sem_wait (sem_t * sem)
{
  return semaphore::wait (sem);
}

extern "C" int
sem_trywait (sem_t * sem)
{
  return semaphore::trywait (sem);
}

extern "C" int
sem_clockwait (sem_t * sem, clockid_t clock_id, const struct timespec *abstime)
{
  return semaphore::clockwait (sem, clock_id, abstime);
}

extern "C" int
sem_timedwait (sem_t * sem, const struct timespec *abstime)
{
  return semaphore::clockwait (sem, CLOCK_REALTIME, abstime);
}

extern "C" int
sem_post (sem_t *sem)
{
  return semaphore::post (sem);
}

extern "C" int
sem_getvalue (sem_t * sem, int *sval)
{
  return semaphore::getvalue (sem, sval);
}
