/* posix_ipc.cc: POSIX IPC API for Cygwin.

   Copyright 2007, 2008, 2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "thread.h"
#include "path.h"
#include "cygtls.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "ntdll.h"
#include <sys/mman.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <semaphore.h>

/* The prefix_len is the length of the path prefix ncluding trailing "/"
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
  /* Name must not be empty, or just be a single slash, or start with more
     than one slash.  Same for backslash.
     Apart from handling backslash like slash, the naming rules are identical
     to Linux, including the names and requirements for subdirectories, if
     the name contains further slashes. */
  if (!name || (strchr ("/\\", name[0])
		&& (!name[1] || strchr ("/\\", name[1]))))
    {
      debug_printf ("Invalid %s name '%s'", ipc_names[type].description, name);
      set_errno (EINVAL);
      return false;
    }
  /* Skip leading (back-)slash. */
  if (strchr ("/\\", name[0]))
    ++name;
  if (len > PATH_MAX - ipc_names[type].prefix_len)
    {
      debug_printf ("%s name '%s' too long", ipc_names[type].description, name);
      set_errno (ENAMETOOLONG);
      return false;
    }
  __small_sprintf (res_name, "%s/%s%s", ipc_names[type].prefix,
					type == semaphore ? "sem." : "",
					name);
  return true;
}

static int
ipc_mutex_init (HANDLE *pmtx, const char *name)
{
  char buf[MAX_PATH];
  SECURITY_ATTRIBUTES sa = sec_none;

  __small_sprintf (buf, "mqueue/mtx_%s", name);
  sa.lpSecurityDescriptor = everyone_sd (CYG_MUTANT_ACCESS);
  *pmtx = CreateMutex (&sa, FALSE, buf);
  if (!*pmtx)
    debug_printf ("CreateMutex: %E");
  return *pmtx ? 0 : geterrno_from_win_error ();
}

static int
ipc_mutex_lock (HANDLE mtx)
{
  HANDLE h[2] = { mtx, signal_arrived };

  switch (WaitForMultipleObjects (2, h, FALSE, INFINITE))
    {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED_0:
      return 0;
    case WAIT_OBJECT_0 + 1:
      set_errno (EINTR);
      return 1;
    default:
      break;
    }
  return geterrno_from_win_error ();
}

static inline int
ipc_mutex_unlock (HANDLE mtx)
{
  return ReleaseMutex (mtx) ? 0 : geterrno_from_win_error ();
}

static inline int
ipc_mutex_close (HANDLE mtx)
{
  return CloseHandle (mtx) ? 0 : geterrno_from_win_error ();
}

static int
ipc_cond_init (HANDLE *pevt, const char *name)
{
  char buf[MAX_PATH];
  SECURITY_ATTRIBUTES sa = sec_none;

  __small_sprintf (buf, "mqueue/evt_%s", name);
  sa.lpSecurityDescriptor = everyone_sd (CYG_EVENT_ACCESS);
  *pevt = CreateEvent (&sa, TRUE, FALSE, buf);
  if (!*pevt)
    debug_printf ("CreateEvent: %E");
  return *pevt ? 0 : geterrno_from_win_error ();
}

static int
ipc_cond_timedwait (HANDLE evt, HANDLE mtx, const struct timespec *abstime)
{
  struct timeval tv;
  DWORD timeout;
  HANDLE h[2] = { mtx, evt };

  if (!abstime)
    timeout = INFINITE;
  else if (abstime->tv_sec < 0
	   || abstime->tv_nsec < 0
	   || abstime->tv_nsec > 999999999)
    return EINVAL;
  else
    {
      gettimeofday (&tv, NULL);
      /* Check for immediate timeout. */
      if (tv.tv_sec > abstime->tv_sec
	  || (tv.tv_sec == abstime->tv_sec
	      && tv.tv_usec > abstime->tv_nsec / 1000))
	return ETIMEDOUT;
      timeout = (abstime->tv_sec - tv.tv_sec) * 1000;
      timeout += (abstime->tv_nsec / 1000 - tv.tv_usec) / 1000;
    }
  if (ipc_mutex_unlock (mtx))
    return -1;
  switch (WaitForMultipleObjects (2, h, TRUE, timeout))
    {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED_0:
      ResetEvent (evt);
      return 0;
    case WAIT_TIMEOUT:
      ipc_mutex_lock (mtx);
      return ETIMEDOUT;
    default:
      break;
    }
  return geterrno_from_win_error ();
}

static inline int
ipc_cond_signal (HANDLE evt)
{
  return SetEvent (evt) ? 0 : geterrno_from_win_error ();
}

static inline int
ipc_cond_close (HANDLE evt)
{
  return CloseHandle (evt) ? 0 : geterrno_from_win_error ();
}

class ipc_flock
{
  struct __flock64 fl;

public:
  ipc_flock () { memset (&fl, 0, sizeof fl); }

  int lock (int fd, size_t size)
  {
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = size;
    return fcntl64 (fd, F_SETLKW, &fl);
  }
  int unlock (int fd)
  {
    if (!fl.l_len)
      return 0;
    fl.l_type = F_UNLCK;
    return fcntl64 (fd, F_SETLKW, &fl);
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

struct mq_hdr
{
  struct mq_attr  mqh_attr;	 /* the queue's attributes */
  long            mqh_head;	 /* index of first message */
  long            mqh_free;	 /* index of first free message */
  long            mqh_nwait;	 /* #threads blocked in mq_receive() */
  pid_t           mqh_pid;	 /* nonzero PID if mqh_event set */
  char            mqh_uname[36]; /* unique name used to identify synchronization
				    objects connected to this queue */
  struct sigevent mqh_event;	 /* for mq_notify() */
};

struct msg_hdr
{
  long            msg_next;	 /* index of next on linked list */
  ssize_t         msg_len;	 /* actual length */
  unsigned int    msg_prio;	 /* priority */
};

struct mq_info
{
  struct mq_hdr  *mqi_hdr;	 /* start of mmap'ed region */
  unsigned long   mqi_magic;	 /* magic number if open */
  int             mqi_flags;	 /* flags for this process */
  HANDLE          mqi_lock;	 /* mutex lock */
  HANDLE          mqi_wait;	 /* and condition variable */
};

#define MQI_MAGIC	0x98765432UL

#define MSGSIZE(i)	roundup((i), sizeof(long))

#define	 MAX_TRIES	10	/* for waiting for initialization */

struct mq_attr defattr = { 0, 10, 8192, 0 };	/* Linux defaults. */

extern "C" _off64_t lseek64 (int, _off64_t, int);
extern "C" void *mmap64 (void *, size_t, int, int, int, _off64_t);

extern "C" mqd_t
mq_open (const char *name, int oflag, ...)
{
  int i, fd = -1, nonblock, created;
  long msgsize, index;
  _off64_t filesize = 0;
  va_list ap;
  mode_t mode;
  int8_t *mptr;
  struct __stat64 statbuff;
  struct mq_hdr *mqhdr;
  struct msg_hdr *msghdr;
  struct mq_attr *attr;
  struct mq_info *mqinfo;
  LUID luid;

  size_t len = strlen (name);
  char mqname[ipc_names[mqueue].prefix_len + len];

  if (!check_path (mqname, mqueue, name, len))
    return (mqd_t) -1;

  myfault efault;
  if (efault.faulted (EFAULT))
    return (mqd_t) -1;

  oflag &= (O_CREAT | O_EXCL | O_NONBLOCK);
  created = 0;
  nonblock = oflag & O_NONBLOCK;
  oflag &= ~O_NONBLOCK;
  mptr = (int8_t *) MAP_FAILED;
  mqinfo = NULL;

again:
  if (oflag & O_CREAT)
    {
      va_start (ap, oflag);		/* init ap to final named argument */
      mode = va_arg (ap, mode_t) & ~S_IXUSR;
      attr = va_arg (ap, struct mq_attr *);
      va_end (ap);

      /* Open and specify O_EXCL and user-execute */
      fd = open (mqname, oflag | O_EXCL | O_RDWR | O_CLOEXEC, mode | S_IXUSR);
      if (fd < 0)
	{
	  if (errno == EEXIST && (oflag & O_EXCL) == 0)
	    goto exists;		/* already exists, OK */
	  return (mqd_t) -1;
	}
      created = 1;
      /* First one to create the file initializes it */
      if (attr == NULL)
	attr = &defattr;
      else if (attr->mq_maxmsg <= 0 || attr->mq_msgsize <= 0)
	{
	  set_errno (EINVAL);
	  goto err;
	}
      /* Calculate and set the file size */
      msgsize = MSGSIZE (attr->mq_msgsize);
      filesize = sizeof (struct mq_hdr)
		 + (attr->mq_maxmsg * (sizeof (struct msg_hdr) + msgsize));
      if (lseek64 (fd, filesize - 1, SEEK_SET) == -1)
	goto err;
      if (write (fd, "", 1) == -1)
	goto err;

      /* Memory map the file */
      mptr = (int8_t *) mmap64 (NULL, (size_t) filesize, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0);
      if (mptr == (int8_t *) MAP_FAILED)
	goto err;

      /* Allocate one mq_info{} for the queue */
      if (!(mqinfo = (struct mq_info *) malloc (sizeof (struct mq_info))))
	goto err;
      mqinfo->mqi_hdr = mqhdr = (struct mq_hdr *) mptr;
      mqinfo->mqi_magic = MQI_MAGIC;
      mqinfo->mqi_flags = nonblock;

      /* Initialize header at beginning of file */
      /* Create free list with all messages on it */
      mqhdr->mqh_attr.mq_flags = 0;
      mqhdr->mqh_attr.mq_maxmsg = attr->mq_maxmsg;
      mqhdr->mqh_attr.mq_msgsize = attr->mq_msgsize;
      mqhdr->mqh_attr.mq_curmsgs = 0;
      mqhdr->mqh_nwait = 0;
      mqhdr->mqh_pid = 0;
      if (!AllocateLocallyUniqueId (&luid))
	{
	  __seterrno ();
	  goto err;
	}
      __small_sprintf (mqhdr->mqh_uname, "%016X%08x%08x",
		       hash_path_name (0,mqname),
		       luid.HighPart, luid.LowPart);
      mqhdr->mqh_head = 0;
      index = sizeof (struct mq_hdr);
      mqhdr->mqh_free = index;
      for (i = 0; i < attr->mq_maxmsg - 1; i++)
	{
	  msghdr = (struct msg_hdr *) &mptr[index];
	  index += sizeof (struct msg_hdr) + msgsize;
	  msghdr->msg_next = index;
	}
      msghdr = (struct msg_hdr *) &mptr[index];
      msghdr->msg_next = 0;		/* end of free list */

      /* Initialize mutex & condition variable */
      i = ipc_mutex_init (&mqinfo->mqi_lock, mqhdr->mqh_uname);
      if (i != 0)
	goto pthreaderr;

      i = ipc_cond_init (&mqinfo->mqi_wait, mqhdr->mqh_uname);
      if (i != 0)
	goto pthreaderr;

      /* Initialization complete, turn off user-execute bit */
      if (fchmod (fd, mode) == -1)
	goto err;
      close (fd);
      return ((mqd_t) mqinfo);
    }

exists:
  /* Open the file then memory map */
  if ((fd = open (mqname, O_RDWR | O_CLOEXEC)) < 0)
    {
      if (errno == ENOENT && (oflag & O_CREAT))
	goto again;
      goto err;
    }
  /* Make certain initialization is complete */
  for (i = 0; i < MAX_TRIES; i++)
    {
      if (stat64 (mqname, &statbuff) == -1)
	{
	  if (errno == ENOENT && (oflag & O_CREAT))
	    {
	      close (fd);
	      fd = -1;
	      goto again;
	    }
	  goto err;
	}
      if ((statbuff.st_mode & S_IXUSR) == 0)
	break;
      sleep (1);
    }
  if (i == MAX_TRIES)
    {
      set_errno (ETIMEDOUT);
      goto err;
    }

  filesize = statbuff.st_size;
  mptr = (int8_t *) mmap64 (NULL, (size_t) filesize, PROT_READ | PROT_WRITE,
			    MAP_SHARED, fd, 0);
  if (mptr == (int8_t *) MAP_FAILED)
    goto err;
  close (fd);
  fd = -1;

  /* Allocate one mq_info{} for each open */
  if (!(mqinfo = (struct mq_info *) malloc (sizeof (struct mq_info))))
    goto err;
  mqinfo->mqi_hdr = mqhdr = (struct mq_hdr *) mptr;
  mqinfo->mqi_magic = MQI_MAGIC;
  mqinfo->mqi_flags = nonblock;

  /* Initialize mutex & condition variable */
  i = ipc_mutex_init (&mqinfo->mqi_lock, mqhdr->mqh_uname);
  if (i != 0)
    goto pthreaderr;

  i = ipc_cond_init (&mqinfo->mqi_wait, mqhdr->mqh_uname);
  if (i != 0)
    goto pthreaderr;

  return (mqd_t) mqinfo;

pthreaderr:
  errno = i;
err:
  /* Don't let following function calls change errno */
  save_errno save;

  if (created)
    unlink (mqname);
  if (mptr != (int8_t *) MAP_FAILED)
    munmap((void *) mptr, (size_t) filesize);
  if (mqinfo)
    free (mqinfo);
  if (fd >= 0)
    close (fd);
  return (mqd_t) -1;
}

extern "C" int
mq_getattr (mqd_t mqd, struct mq_attr *mqstat)
{
  int n;
  struct mq_hdr *mqhdr;
  struct mq_attr *attr;
  struct mq_info *mqinfo;

  myfault efault;
  if (efault.faulted (EBADF))
      return -1;

  mqinfo = (struct mq_info *) mqd;
  if (mqinfo->mqi_magic != MQI_MAGIC)
    {
      set_errno (EBADF);
      return -1;
    }
  mqhdr = mqinfo->mqi_hdr;
  attr = &mqhdr->mqh_attr;
  if ((n = ipc_mutex_lock (mqinfo->mqi_lock)) != 0)
    {
      errno = n;
      return -1;
    }
  mqstat->mq_flags = mqinfo->mqi_flags;   /* per-open */
  mqstat->mq_maxmsg = attr->mq_maxmsg;    /* remaining three per-queue */
  mqstat->mq_msgsize = attr->mq_msgsize;
  mqstat->mq_curmsgs = attr->mq_curmsgs;

  ipc_mutex_unlock (mqinfo->mqi_lock);
  return 0;
}

extern "C" int
mq_setattr (mqd_t mqd, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
  int n;
  struct mq_hdr *mqhdr;
  struct mq_attr *attr;
  struct mq_info *mqinfo;

  myfault efault;
  if (efault.faulted (EBADF))
      return -1;

  mqinfo = (struct mq_info *) mqd;
  if (mqinfo->mqi_magic != MQI_MAGIC)
    {
      set_errno (EBADF);
      return -1;
    }
  mqhdr = mqinfo->mqi_hdr;
  attr = &mqhdr->mqh_attr;
  if ((n = ipc_mutex_lock (mqinfo->mqi_lock)) != 0)
    {
      errno = n;
      return -1;
    }

  if (omqstat != NULL)
    {
      omqstat->mq_flags = mqinfo->mqi_flags;  /* previous attributes */
      omqstat->mq_maxmsg = attr->mq_maxmsg;
      omqstat->mq_msgsize = attr->mq_msgsize;
      omqstat->mq_curmsgs = attr->mq_curmsgs; /* and current status */
    }

  if (mqstat->mq_flags & O_NONBLOCK)
    mqinfo->mqi_flags |= O_NONBLOCK;
  else
    mqinfo->mqi_flags &= ~O_NONBLOCK;

  ipc_mutex_unlock (mqinfo->mqi_lock);
  return 0;
}

extern "C" int
mq_notify (mqd_t mqd, const struct sigevent *notification)
{
  int n;
  pid_t pid;
  struct mq_hdr *mqhdr;
  struct mq_info *mqinfo;

  myfault efault;
  if (efault.faulted (EBADF))
      return -1;

  mqinfo = (struct mq_info *) mqd;
  if (mqinfo->mqi_magic != MQI_MAGIC)
    {
      set_errno (EBADF);
      return -1;
    }
  mqhdr = mqinfo->mqi_hdr;
  if ((n = ipc_mutex_lock (mqinfo->mqi_lock)) != 0)
    {
      errno = n;
      return -1;
    }

  pid = getpid ();
  if (!notification)
    {
      if (mqhdr->mqh_pid == pid)
	  mqhdr->mqh_pid = 0;     /* unregister calling process */
    }
  else
    {
      if (mqhdr->mqh_pid != 0)
	{
	  if (kill (mqhdr->mqh_pid, 0) != -1 || errno != ESRCH)
	    {
	      set_errno (EBUSY);
	      ipc_mutex_unlock (mqinfo->mqi_lock);
	      return -1;
	    }
	}
      mqhdr->mqh_pid = pid;
      mqhdr->mqh_event = *notification;
    }
  ipc_mutex_unlock (mqinfo->mqi_lock);
  return 0;
}

static int
_mq_send (mqd_t mqd, const char *ptr, size_t len, unsigned int prio,
	  const struct timespec *abstime)
{
  int n;
  long index, freeindex;
  int8_t *mptr;
  struct sigevent *sigev;
  struct mq_hdr *mqhdr;
  struct mq_attr *attr;
  struct msg_hdr *msghdr, *nmsghdr, *pmsghdr;
  struct mq_info *mqinfo;

  myfault efault;
  if (efault.faulted (EBADF))
      return -1;

  mqinfo = (struct mq_info *) mqd;
  if (mqinfo->mqi_magic != MQI_MAGIC)
    {
      set_errno (EBADF);
      return -1;
    }
  if (prio > MQ_PRIO_MAX)
    {
      set_errno (EINVAL);
      return -1;
    }

  mqhdr = mqinfo->mqi_hdr;        /* struct pointer */
  mptr = (int8_t *) mqhdr;        /* byte pointer */
  attr = &mqhdr->mqh_attr;
  if ((n = ipc_mutex_lock (mqinfo->mqi_lock)) != 0)
    {
      errno = n;
      return -1;
    }

  if (len > (size_t) attr->mq_msgsize)
    {
      set_errno (EMSGSIZE);
      goto err;
    }
  if (attr->mq_curmsgs == 0)
    {
      if (mqhdr->mqh_pid != 0 && mqhdr->mqh_nwait == 0)
	{
	  sigev = &mqhdr->mqh_event;
	  if (sigev->sigev_notify == SIGEV_SIGNAL)
	    sigqueue (mqhdr->mqh_pid, sigev->sigev_signo, sigev->sigev_value);
	  mqhdr->mqh_pid = 0;             /* unregister */
	}
    }
  else if (attr->mq_curmsgs >= attr->mq_maxmsg)
    {
      /* Queue is full */
      if (mqinfo->mqi_flags & O_NONBLOCK)
	{
	  set_errno (EAGAIN);
	  goto err;
	}
      /* Wait for room for one message on the queue */
      while (attr->mq_curmsgs >= attr->mq_maxmsg)
	ipc_cond_timedwait (mqinfo->mqi_wait, mqinfo->mqi_lock, abstime);
    }

  /* nmsghdr will point to new message */
  if ((freeindex = mqhdr->mqh_free) == 0)
    api_fatal ("mq_send: curmsgs = %ld; free = 0", attr->mq_curmsgs);

  nmsghdr = (struct msg_hdr *) &mptr[freeindex];
  nmsghdr->msg_prio = prio;
  nmsghdr->msg_len = len;
  memcpy (nmsghdr + 1, ptr, len);          /* copy message from caller */
  mqhdr->mqh_free = nmsghdr->msg_next;    /* new freelist head */

  /* Find right place for message in linked list */
  index = mqhdr->mqh_head;
  pmsghdr = (struct msg_hdr *) &(mqhdr->mqh_head);
  while (index)
    {
      msghdr = (struct msg_hdr *) &mptr[index];
      if (prio > msghdr->msg_prio)
	{
	  nmsghdr->msg_next = index;
	  pmsghdr->msg_next = freeindex;
	  break;
	}
      index = msghdr->msg_next;
      pmsghdr = msghdr;
    }
  if (index == 0)
    {
      /* Queue was empty or new goes at end of list */
      pmsghdr->msg_next = freeindex;
      nmsghdr->msg_next = 0;
    }
  /* Wake up anyone blocked in mq_receive waiting for a message */
  if (attr->mq_curmsgs == 0)
    ipc_cond_signal (mqinfo->mqi_wait);
  attr->mq_curmsgs++;

  ipc_mutex_unlock (mqinfo->mqi_lock);
  return 0;

err:
  ipc_mutex_unlock (mqinfo->mqi_lock);
  return -1;
}

extern "C" int
mq_send (mqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
  return _mq_send (mqd, ptr, len, prio, NULL);
}

extern "C" int
mq_timedsend (mqd_t mqd, const char *ptr, size_t len, unsigned int prio,
	      const struct timespec *abstime)
{
  return _mq_send (mqd, ptr, len, prio, abstime);
}

static ssize_t
_mq_receive (mqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop,
	     const struct timespec *abstime)
{
  int n;
  long index;
  int8_t *mptr;
  ssize_t len;
  struct mq_hdr *mqhdr;
  struct mq_attr *attr;
  struct msg_hdr *msghdr;
  struct mq_info *mqinfo;

  myfault efault;
  if (efault.faulted (EBADF))
      return -1;

  mqinfo = (struct mq_info *) mqd;
  if (mqinfo->mqi_magic != MQI_MAGIC)
    {
      set_errno (EBADF);
      return -1;
    }
  mqhdr = mqinfo->mqi_hdr;        /* struct pointer */
  mptr = (int8_t *) mqhdr;        /* byte pointer */
  attr = &mqhdr->mqh_attr;
  if ((n = ipc_mutex_lock (mqinfo->mqi_lock)) != 0)
    {
      errno = n;
      return -1;
    }

  if (maxlen < (size_t) attr->mq_msgsize)
    {
      set_errno (EMSGSIZE);
      goto err;
    }
  if (attr->mq_curmsgs == 0)	/* queue is empty */
    {
      if (mqinfo->mqi_flags & O_NONBLOCK)
	{
	  set_errno (EAGAIN);
	  goto err;
	}
      /* Wait for a message to be placed onto queue */
      mqhdr->mqh_nwait++;
      while (attr->mq_curmsgs == 0)
	ipc_cond_timedwait (mqinfo->mqi_wait, mqinfo->mqi_lock, abstime);
      mqhdr->mqh_nwait--;
    }

  if ((index = mqhdr->mqh_head) == 0)
    api_fatal ("mq_receive: curmsgs = %ld; head = 0", attr->mq_curmsgs);

  msghdr = (struct msg_hdr *) &mptr[index];
  mqhdr->mqh_head = msghdr->msg_next;     /* new head of list */
  len = msghdr->msg_len;
  memcpy(ptr, msghdr + 1, len);           /* copy the message itself */
  if (priop != NULL)
    *priop = msghdr->msg_prio;

  /* Just-read message goes to front of free list */
  msghdr->msg_next = mqhdr->mqh_free;
  mqhdr->mqh_free = index;

  /* Wake up anyone blocked in mq_send waiting for room */
  if (attr->mq_curmsgs == attr->mq_maxmsg)
    ipc_cond_signal (mqinfo->mqi_wait);
  attr->mq_curmsgs--;

  ipc_mutex_unlock (mqinfo->mqi_lock);
  return len;

err:
  ipc_mutex_unlock (mqinfo->mqi_lock);
  return -1;
}

extern "C" ssize_t
mq_receive (mqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop)
{
  return _mq_receive (mqd, ptr, maxlen, priop, NULL);
}

extern "C" ssize_t
mq_timedreceive (mqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop,
		 const struct timespec *abstime)
{
  return _mq_receive (mqd, ptr, maxlen, priop, abstime);
}

extern "C" int
mq_close (mqd_t mqd)
{
  long msgsize, filesize;
  struct mq_hdr *mqhdr;
  struct mq_attr *attr;
  struct mq_info *mqinfo;

  myfault efault;
  if (efault.faulted (EBADF))
      return -1;

  mqinfo = (struct mq_info *) mqd;
  if (mqinfo->mqi_magic != MQI_MAGIC)
    {
      set_errno (EBADF);
      return -1;
    }
  mqhdr = mqinfo->mqi_hdr;
  attr = &mqhdr->mqh_attr;

  if (mq_notify (mqd, NULL))	/* unregister calling process */
    return -1;

  msgsize = MSGSIZE (attr->mq_msgsize);
  filesize = sizeof (struct mq_hdr)
	     + (attr->mq_maxmsg * (sizeof (struct msg_hdr) + msgsize));
  if (munmap (mqinfo->mqi_hdr, filesize) == -1)
    return -1;

  mqinfo->mqi_magic = 0;          /* just in case */
  ipc_cond_close (mqinfo->mqi_wait);
  ipc_mutex_close (mqinfo->mqi_lock);
  free (mqinfo);
  return 0;
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

struct sem_finfo
{
  unsigned int       value;
  unsigned long long hash;
  LUID               luid;
};

extern "C" sem_t *
sem_open (const char *name, int oflag, ...)
{
  int i, fd = -1, created;
  va_list ap;
  mode_t mode = 0;
  unsigned int value = 0;
  struct __stat64 statbuff;
  sem_t *sem = SEM_FAILED;
  sem_finfo sf;
  bool wasopen = false;
  ipc_flock file;

  size_t len = strlen (name);
  char semname[ipc_names[semaphore].prefix_len + len];

  if (!check_path (semname, semaphore, name, len))
    return SEM_FAILED;

  myfault efault;
  if (efault.faulted (EFAULT))
    return SEM_FAILED;

  created = 0;
  oflag &= (O_CREAT | O_EXCL);

again:
  if (oflag & O_CREAT)
    {
      va_start (ap, oflag);		/* init ap to final named argument */
      mode = va_arg (ap, mode_t) & ~S_IXUSR;
      value = va_arg (ap, unsigned int);
      va_end (ap);

      /* Open and specify O_EXCL and user-execute */
      fd = open (semname, oflag | O_EXCL | O_RDWR | O_CLOEXEC, mode | S_IXUSR);
      if (fd < 0)
	{
	  if (errno == EEXIST && (oflag & O_EXCL) == 0)
	    goto exists;		/* already exists, OK */
	  return SEM_FAILED;
	}
      created = 1;
      /* First one to create the file initializes it. */
      if (!AllocateLocallyUniqueId (&sf.luid))
	{
	  __seterrno ();
	  goto err;
	}
      sf.value = value;
      sf.hash = hash_path_name (0, semname);
      if (write (fd, &sf, sizeof sf) != sizeof sf)
	goto err;
      sem = semaphore::open (sf.hash, sf.luid, fd, oflag, mode, value, wasopen);
      if (sem == SEM_FAILED)
	goto err;
      /* Initialization complete, turn off user-execute bit */
      if (fchmod (fd, mode) == -1)
	goto err;
      /* Don't close (fd); */
      return sem;
    }

exists:
  /* Open the file and fetch the semaphore name. */
  if ((fd = open (semname, O_RDWR | O_CLOEXEC)) < 0)
    {
      if (errno == ENOENT && (oflag & O_CREAT))
	goto again;
      goto err;
    }
  /* Make certain initialization is complete */
  for (i = 0; i < MAX_TRIES; i++)
    {
      if (stat64 (semname, &statbuff) == -1)
	{
	  if (errno == ENOENT && (oflag & O_CREAT))
	    {
	      close (fd);
	      fd = -1;
	      goto again;
	    }
	  goto err;
	}
      if ((statbuff.st_mode & S_IXUSR) == 0)
	break;
      sleep (1);
    }
  if (i == MAX_TRIES)
    {
      set_errno (ETIMEDOUT);
      goto err;
    }
  if (file.lock (fd, sizeof sf))
    goto err;
  if (read (fd, &sf, sizeof sf) != sizeof sf)
    goto err;
  sem = semaphore::open (sf.hash, sf.luid, fd, oflag, mode, sf.value, wasopen);
  file.unlock (fd);
  if (sem == SEM_FAILED)
    goto err;
  /* If wasopen is set, the semaphore was already opened and we already have
     an open file descriptor pointing to the file.  This means, we have to
     close the file descriptor created in this call.  It won't be stored
     anywhere anyway. */
  if (wasopen)
    close (fd);
  return sem;

err:
  /* Don't let following function calls change errno */
  save_errno save;

  file.unlock (fd);
  if (created)
    unlink (semname);
  if (sem != SEM_FAILED)
    semaphore::close (sem);
  if (fd >= 0)
    close (fd);
  return SEM_FAILED;
}

int
_sem_close (sem_t *sem, bool do_close)
{
  sem_finfo sf;
  int fd, ret = -1;
  ipc_flock file;

  if (semaphore::getinternal (sem, &fd, &sf.hash, &sf.luid, &sf.value) == -1)
    return -1;
  if (!file.lock (fd, sizeof sf)
      && lseek64 (fd, 0LL, SEEK_SET) != (_off64_t) -1
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
