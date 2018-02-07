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
  WCHAR buf[MAX_PATH];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  __small_swprintf (buf, L"mqueue/mtx_%s", name);
  RtlInitUnicodeString (&uname, buf);
  InitializeObjectAttributes (&attr, &uname,
			      OBJ_INHERIT | OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
			      get_shared_parent_dir (),
			      everyone_sd (CYG_MUTANT_ACCESS));
  status = NtCreateMutant (pmtx, CYG_MUTANT_ACCESS, &attr, FALSE);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtCreateMutant: %y", status);
      return geterrno_from_win_error (RtlNtStatusToDosError (status));
    }
  return 0;
}

static int
ipc_mutex_lock (HANDLE mtx, bool eintr)
{
  switch (cygwait (mtx, cw_infinite, cw_cancel | cw_cancel_self
				     | (eintr ? cw_sig_eintr : cw_sig_restart)))
    {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED_0:
      return 0;
    case WAIT_SIGNALED:
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
ipc_cond_init (HANDLE *pevt, const char *name, char sr)
{
  WCHAR buf[MAX_PATH];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  __small_swprintf (buf, L"mqueue/evt_%s%c", name, sr);
  RtlInitUnicodeString (&uname, buf);
  InitializeObjectAttributes (&attr, &uname,
			      OBJ_INHERIT | OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
			      get_shared_parent_dir (),
			      everyone_sd (CYG_EVENT_ACCESS));
  status = NtCreateEvent (pevt, CYG_EVENT_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtCreateEvent: %y", status);
      return geterrno_from_win_error (RtlNtStatusToDosError (status));
    }
  return 0;
}

static int
ipc_cond_timedwait (HANDLE evt, HANDLE mtx, const struct timespec *abstime)
{
  HANDLE w4[4] = { evt, };
  DWORD cnt = 2;
  DWORD timer_idx = 0;
  int ret = 0;

  wait_signal_arrived here (w4[1]);
  if ((w4[cnt] = pthread::get_cancel_event ()) != NULL)
    ++cnt;
  if (abstime)
    {
      if (abstime->tv_sec < 0
	       || abstime->tv_nsec < 0
	       || abstime->tv_nsec >= NSPERSEC)
	return EINVAL;

      /* If a timeout is set, we create a waitable timer to wait for.
	 This is the easiest way to handle the absolute timeout value, given
	 that NtSetTimer also takes absolute times and given the double
	 dependency on evt *and* mtx, which requires to call WFMO twice. */
      NTSTATUS status;
      LARGE_INTEGER duetime;

      timer_idx = cnt++;
      status = NtCreateTimer (&w4[timer_idx], TIMER_ALL_ACCESS, NULL,
			      NotificationTimer);
      if (!NT_SUCCESS (status))
	return geterrno_from_nt_status (status);
      timespec_to_filetime (abstime, &duetime);
      status = NtSetTimer (w4[timer_idx], &duetime, NULL, NULL, FALSE, 0, NULL);
      if (!NT_SUCCESS (status))
	{
	  NtClose (w4[timer_idx]);
	  return geterrno_from_nt_status (status);
	}
    }
  ResetEvent (evt);
  if ((ret = ipc_mutex_unlock (mtx)) != 0)
    return ret;
  /* Everything's set up, so now wait for the event to be signalled. */
restart1:
  switch (WaitForMultipleObjects (cnt, w4, FALSE, INFINITE))
    {
    case WAIT_OBJECT_0:
      break;
    case WAIT_OBJECT_0 + 1:
      if (_my_tls.call_signal_handler ())
	goto restart1;
      ret = EINTR;
      break;
    case WAIT_OBJECT_0 + 2:
      if (timer_idx != 2)
	pthread::static_cancel_self ();
      /*FALLTHRU*/
    case WAIT_OBJECT_0 + 3:
      ret = ETIMEDOUT;
      break;
    default:
      ret = geterrno_from_win_error ();
      break;
    }
  if (ret == 0)
    {
      /* At this point we need to lock the mutex.  The wait is practically
	 the same as before, just that we now wait on the mutex instead of the
	 event. */
    restart2:
      w4[0] = mtx;
      switch (WaitForMultipleObjects (cnt, w4, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
	case WAIT_ABANDONED_0:
	  break;
	case WAIT_OBJECT_0 + 1:
	  if (_my_tls.call_signal_handler ())
	    goto restart2;
	  ret = EINTR;
	  break;
	case WAIT_OBJECT_0 + 2:
	  if (timer_idx != 2)
	    pthread_testcancel ();
	  /*FALLTHRU*/
	case WAIT_OBJECT_0 + 3:
	  ret = ETIMEDOUT;
	  break;
	default:
	  ret = geterrno_from_win_error ();
	  break;
	}
    }
  if (timer_idx)
    {
      if (ret != ETIMEDOUT)
	NtCancelTimer (w4[timer_idx], NULL);
      NtClose (w4[timer_idx]);
    }
  return ret;
}

static inline void
ipc_cond_signal (HANDLE evt)
{
  SetEvent (evt);
}

static inline void
ipc_cond_close (HANDLE evt)
{
  CloseHandle (evt);
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

/* The mq_attr structure is defined using long datatypes per POSIX.
   For interoperability reasons between 32 and 64 bit processes, we have
   to make sure to use a unified structure layout in the message queue file.
   That's what the mq_fattr is, the in-file representation of the mq_attr
   struct. */
#pragma pack (push, 4)
struct mq_fattr
{
  uint32_t mq_flags;
  uint32_t mq_maxmsg;
  uint32_t mq_msgsize;
  uint32_t mq_curmsgs;
};

struct mq_hdr
{
  struct mq_fattr mqh_attr;	 /* the queue's attributes */
  int32_t         mqh_head;	 /* index of first message */
  int32_t         mqh_free;	 /* index of first free message */
  int32_t         mqh_nwait;	 /* #threads blocked in mq_receive() */
  pid_t           mqh_pid;	 /* nonzero PID if mqh_event set */
  char            mqh_uname[36]; /* unique name used to identify synchronization
				    objects connected to this queue */
  union {
    struct sigevent mqh_event;	 /* for mq_notify() */
    /* Make sure sigevent takes the same space on 32 and 64 bit systems.
       Other than that, it doesn't need to be compatible since only
       one process can be notified at a time. */
    uint64_t        mqh_placeholder[8];
  };
  uint32_t        mqh_magic;	/* Expect MQI_MAGIC here, otherwise it's
				   an old-style message queue. */
};

struct msg_hdr
{
  int32_t         msg_next;	 /* index of next on linked list */
  int32_t         msg_len;	 /* actual length */
  unsigned int    msg_prio;	 /* priority */
};
#pragma pack (pop)

struct mq_info
{
  struct mq_hdr  *mqi_hdr;	 /* start of mmap'ed region */
  uint32_t        mqi_magic;	 /* magic number if open */
  int             mqi_flags;	 /* flags for this process */
  HANDLE          mqi_lock;	 /* mutex lock */
  HANDLE          mqi_waitsend;	 /* and condition variable for full queue */
  HANDLE          mqi_waitrecv;	 /* and condition variable for empty queue */
};

#define MQI_MAGIC	0x98765432UL

#define MSGSIZE(i)	roundup((i), sizeof(long))

#define	 MAX_TRIES	10	/* for waiting for initialization */

struct mq_attr defattr = { 0, 10, 8192, 0 };	/* Linux defaults. */

extern "C" off_t lseek64 (int, off_t, int);
extern "C" void *mmap64 (void *, size_t, int, int, int, off_t);

extern "C" mqd_t
mq_open (const char *name, int oflag, ...)
{
  int i, fd = -1, nonblock, created = 0;
  long msgsize, index;
  off_t filesize = 0;
  va_list ap;
  mode_t mode;
  int8_t *mptr = (int8_t *) MAP_FAILED;
  struct stat statbuff;
  struct mq_hdr *mqhdr;
  struct msg_hdr *msghdr;
  struct mq_attr *attr;
  struct mq_info *mqinfo = NULL;
  LUID luid;

  size_t len = strlen (name);
  char mqname[ipc_names[mqueue].prefix_len + len];

  if (!check_path (mqname, mqueue, name, len))
    return (mqd_t) -1;

  __try
    {
      oflag &= (O_CREAT | O_EXCL | O_NONBLOCK);
      nonblock = oflag & O_NONBLOCK;
      oflag &= ~O_NONBLOCK;

    again:
      if (oflag & O_CREAT)
	{
	  va_start (ap, oflag);		/* init ap to final named argument */
	  mode = va_arg (ap, mode_t) & ~S_IXUSR;
	  attr = va_arg (ap, struct mq_attr *);
	  va_end (ap);

	  /* Open and specify O_EXCL and user-execute */
	  fd = open (mqname, oflag | O_EXCL | O_RDWR | O_CLOEXEC,
		     mode | S_IXUSR);
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
	  /* Check minimum and maximum values.  The max values are pretty much
	     arbitrary, taken from the linux mq_overview man page.  However,
	     these max values make sure that the internal mq_fattr structure
	     can use 32 bit types. */
	  else if (attr->mq_maxmsg <= 0 || attr->mq_maxmsg > 32768
		   || attr->mq_msgsize <= 0 || attr->mq_msgsize > 1048576)
	    {
	      set_errno (EINVAL);
	      __leave;
	    }
	  /* Calculate and set the file size */
	  msgsize = MSGSIZE (attr->mq_msgsize);
	  filesize = sizeof (struct mq_hdr)
		     + (attr->mq_maxmsg * (sizeof (struct msg_hdr) + msgsize));
	  if (lseek64 (fd, filesize - 1, SEEK_SET) == -1)
	    __leave;
	  if (write (fd, "", 1) == -1)
	    __leave;

	  /* Memory map the file */
	  mptr = (int8_t *) mmap64 (NULL, (size_t) filesize,
				    PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, 0);
	  if (mptr == (int8_t *) MAP_FAILED)
	    __leave;

	  /* Allocate one mq_info{} for the queue */
	  if (!(mqinfo = (struct mq_info *)
			 calloc (1, sizeof (struct mq_info))))
	    __leave;
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
	  NtAllocateLocallyUniqueId (&luid);
	  __small_sprintf (mqhdr->mqh_uname, "%016X%08x%08x",
			   hash_path_name (0,mqname),
			   luid.HighPart, luid.LowPart);
	  mqhdr->mqh_head = 0;
	  mqhdr->mqh_magic = MQI_MAGIC;
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

	  /* Initialize mutex & condition variables */
	  i = ipc_mutex_init (&mqinfo->mqi_lock, mqhdr->mqh_uname);
	  if (i != 0)
	    {
	      set_errno (i);
	      __leave;
	    }
	  i = ipc_cond_init (&mqinfo->mqi_waitsend, mqhdr->mqh_uname, 'S');
	  if (i != 0)
	    {
	      set_errno (i);
	      __leave;
	    }
	  i = ipc_cond_init (&mqinfo->mqi_waitrecv, mqhdr->mqh_uname, 'R');
	  if (i != 0)
	    {
	      set_errno (i);
	      __leave;
	    }
	  /* Initialization complete, turn off user-execute bit */
	  if (fchmod (fd, mode) == -1)
	    __leave;
	  close (fd);
	  return ((mqd_t) mqinfo);
	}

    exists:
      /* Open the file then memory map */
      if ((fd = open (mqname, O_RDWR | O_CLOEXEC)) < 0)
	{
	  if (errno == ENOENT && (oflag & O_CREAT))
	    goto again;
	  __leave;
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

      filesize = statbuff.st_size;
      mptr = (int8_t *) mmap64 (NULL, (size_t) filesize, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0);
      if (mptr == (int8_t *) MAP_FAILED)
	__leave;
      close (fd);
      fd = -1;

      /* Allocate one mq_info{} for each open */
      if (!(mqinfo = (struct mq_info *) calloc (1, sizeof (struct mq_info))))
	__leave;
      mqinfo->mqi_hdr = mqhdr = (struct mq_hdr *) mptr;
      if (mqhdr->mqh_magic != MQI_MAGIC)
	{
	  system_printf (
    "Old message queue \"%s\" detected!\n"
    "This file is not usable as message queue anymore due to changes in the "
    "internal file layout.  Please remove the file and try again.", mqname);
	  set_errno (EACCES);
	  __leave;
	}
      mqinfo->mqi_magic = MQI_MAGIC;
      mqinfo->mqi_flags = nonblock;

      /* Initialize mutex & condition variable */
      i = ipc_mutex_init (&mqinfo->mqi_lock, mqhdr->mqh_uname);
      if (i != 0)
	{
	  set_errno (i);
	  __leave;
	}
      i = ipc_cond_init (&mqinfo->mqi_waitsend, mqhdr->mqh_uname, 'S');
      if (i != 0)
	{
	  set_errno (i);
	  __leave;
	}
      i = ipc_cond_init (&mqinfo->mqi_waitrecv, mqhdr->mqh_uname, 'R');
      if (i != 0)
	{
	  set_errno (i);
	  __leave;
	}
      return (mqd_t) mqinfo;
    }
  __except (EFAULT) {}
  __endtry
  /* Don't let following function calls change errno */
  save_errno save;
  if (created)
    unlink (mqname);
  if (mptr != (int8_t *) MAP_FAILED)
    munmap((void *) mptr, (size_t) filesize);
  if (mqinfo)
    {
      if (mqinfo->mqi_lock)
	ipc_mutex_close (mqinfo->mqi_lock);
      if (mqinfo->mqi_waitsend)
	ipc_cond_close (mqinfo->mqi_waitsend);
      if (mqinfo->mqi_waitrecv)
	ipc_cond_close (mqinfo->mqi_waitrecv);
      free (mqinfo);
    }
  if (fd >= 0)
    close (fd);
  return (mqd_t) -1;
}

extern "C" int
mq_getattr (mqd_t mqd, struct mq_attr *mqstat)
{
  int n;
  struct mq_hdr *mqhdr;
  struct mq_fattr *attr;
  struct mq_info *mqinfo;

  __try
    {
      mqinfo = (struct mq_info *) mqd;
      if (mqinfo->mqi_magic != MQI_MAGIC)
	{
	  set_errno (EBADF);
	  __leave;
	}
      mqhdr = mqinfo->mqi_hdr;
      attr = &mqhdr->mqh_attr;
      if ((n = ipc_mutex_lock (mqinfo->mqi_lock, false)) != 0)
	{
	  errno = n;
	  __leave;
	}
      mqstat->mq_flags = mqinfo->mqi_flags;   /* per-open */
      mqstat->mq_maxmsg = attr->mq_maxmsg;    /* remaining three per-queue */
      mqstat->mq_msgsize = attr->mq_msgsize;
      mqstat->mq_curmsgs = attr->mq_curmsgs;

      ipc_mutex_unlock (mqinfo->mqi_lock);
      return 0;
    }
  __except (EBADF) {}
  __endtry
  return -1;
}

extern "C" int
mq_setattr (mqd_t mqd, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
  int n;
  struct mq_hdr *mqhdr;
  struct mq_fattr *attr;
  struct mq_info *mqinfo;

  __try
    {
      mqinfo = (struct mq_info *) mqd;
      if (mqinfo->mqi_magic != MQI_MAGIC)
	{
	  set_errno (EBADF);
	  __leave;
	}
      mqhdr = mqinfo->mqi_hdr;
      attr = &mqhdr->mqh_attr;
      if ((n = ipc_mutex_lock (mqinfo->mqi_lock, false)) != 0)
	{
	  errno = n;
	  __leave;
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
  __except (EBADF) {}
  __endtry
  return -1;
}

extern "C" int
mq_notify (mqd_t mqd, const struct sigevent *notification)
{
  int n;
  pid_t pid;
  struct mq_hdr *mqhdr;
  struct mq_info *mqinfo;

  __try
    {
      mqinfo = (struct mq_info *) mqd;
      if (mqinfo->mqi_magic != MQI_MAGIC)
	{
	  set_errno (EBADF);
	  __leave;
	}
      mqhdr = mqinfo->mqi_hdr;
      if ((n = ipc_mutex_lock (mqinfo->mqi_lock, false)) != 0)
	{
	  errno = n;
	  __leave;
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
		  __leave;
		}
	    }
	  mqhdr->mqh_pid = pid;
	  mqhdr->mqh_event = *notification;
	}
      ipc_mutex_unlock (mqinfo->mqi_lock);
      return 0;
    }
  __except (EBADF) {}
  __endtry
  return -1;
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
  struct mq_fattr *attr;
  struct msg_hdr *msghdr, *nmsghdr, *pmsghdr;
  struct mq_info *mqinfo = NULL;
  bool ipc_mutex_locked = false;
  int ret = -1;

  pthread_testcancel ();

  __try
    {
      mqinfo = (struct mq_info *) mqd;
      if (mqinfo->mqi_magic != MQI_MAGIC)
	{
	  set_errno (EBADF);
	  __leave;
	}
      if (prio > MQ_PRIO_MAX)
	{
	  set_errno (EINVAL);
	  __leave;
	}

      mqhdr = mqinfo->mqi_hdr;        /* struct pointer */
      mptr = (int8_t *) mqhdr;        /* byte pointer */
      attr = &mqhdr->mqh_attr;
      if ((n = ipc_mutex_lock (mqinfo->mqi_lock, true)) != 0)
	{
	  errno = n;
	  __leave;
	}
      ipc_mutex_locked = true;
      if (len > (size_t) attr->mq_msgsize)
	{
	  set_errno (EMSGSIZE);
	  __leave;
	}
      if (attr->mq_curmsgs == 0)
	{
	  if (mqhdr->mqh_pid != 0 && mqhdr->mqh_nwait == 0)
	    {
	      sigev = &mqhdr->mqh_event;
	      if (sigev->sigev_notify == SIGEV_SIGNAL)
		sigqueue (mqhdr->mqh_pid, sigev->sigev_signo,
			  sigev->sigev_value);
	      mqhdr->mqh_pid = 0;             /* unregister */
	    }
	}
      else if (attr->mq_curmsgs >= attr->mq_maxmsg)
	{
	  /* Queue is full */
	  if (mqinfo->mqi_flags & O_NONBLOCK)
	    {
	      set_errno (EAGAIN);
	      __leave;
	    }
	  /* Wait for room for one message on the queue */
	  while (attr->mq_curmsgs >= attr->mq_maxmsg)
	    {
	      int ret = ipc_cond_timedwait (mqinfo->mqi_waitsend,
					    mqinfo->mqi_lock, abstime);
	      if (ret != 0)
		{
		  set_errno (ret);
		  __leave;
		}
	    }
	}

      /* nmsghdr will point to new message */
      if ((freeindex = mqhdr->mqh_free) == 0)
	api_fatal ("mq_send: curmsgs = %ld; free = 0", attr->mq_curmsgs);

      nmsghdr = (struct msg_hdr *) &mptr[freeindex];
      nmsghdr->msg_prio = prio;
      nmsghdr->msg_len = len;
      memcpy (nmsghdr + 1, ptr, len);         /* copy message from caller */
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
	ipc_cond_signal (mqinfo->mqi_waitrecv);
      attr->mq_curmsgs++;

      ipc_mutex_unlock (mqinfo->mqi_lock);
      ret = 0;
    }
  __except (EBADF) {}
  __endtry
  if (ipc_mutex_locked)
    ipc_mutex_unlock (mqinfo->mqi_lock);
  return ret;
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
  ssize_t len = -1;
  struct mq_hdr *mqhdr;
  struct mq_fattr *attr;
  struct msg_hdr *msghdr;
  struct mq_info *mqinfo = (struct mq_info *) mqd;
  bool ipc_mutex_locked = false;

  pthread_testcancel ();

  __try
    {
      if (mqinfo->mqi_magic != MQI_MAGIC)
	{
	  set_errno (EBADF);
	  __leave;
	}
      mqhdr = mqinfo->mqi_hdr;        /* struct pointer */
      mptr = (int8_t *) mqhdr;        /* byte pointer */
      attr = &mqhdr->mqh_attr;
      if ((n = ipc_mutex_lock (mqinfo->mqi_lock, true)) != 0)
	{
	  errno = n;
	  __leave;
	}
      ipc_mutex_locked = true;
      if (maxlen < (size_t) attr->mq_msgsize)
	{
	  set_errno (EMSGSIZE);
	  __leave;
	}
      if (attr->mq_curmsgs == 0)	/* queue is empty */
	{
	  if (mqinfo->mqi_flags & O_NONBLOCK)
	    {
	      set_errno (EAGAIN);
	      __leave;
	    }
	  /* Wait for a message to be placed onto queue */
	  mqhdr->mqh_nwait++;
	  while (attr->mq_curmsgs == 0)
	    {
	      int ret = ipc_cond_timedwait (mqinfo->mqi_waitrecv,
					    mqinfo->mqi_lock, abstime);
	      if (ret != 0)
		{
		  set_errno (ret);
		  __leave;
		}
	    }
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
	ipc_cond_signal (mqinfo->mqi_waitsend);
      attr->mq_curmsgs--;

      ipc_mutex_unlock (mqinfo->mqi_lock);
    }
  __except (EBADF) {}
  __endtry
  if (ipc_mutex_locked)
    ipc_mutex_unlock (mqinfo->mqi_lock);
  return len;
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
  struct mq_fattr *attr;
  struct mq_info *mqinfo;

  __try
    {
      mqinfo = (struct mq_info *) mqd;
      if (mqinfo->mqi_magic != MQI_MAGIC)
	{
	  set_errno (EBADF);
	  __leave;
	}
      mqhdr = mqinfo->mqi_hdr;
      attr = &mqhdr->mqh_attr;

      if (mq_notify (mqd, NULL))	/* unregister calling process */
	__leave;

      msgsize = MSGSIZE (attr->mq_msgsize);
      filesize = sizeof (struct mq_hdr)
		 + (attr->mq_maxmsg * (sizeof (struct msg_hdr) + msgsize));
      if (munmap (mqinfo->mqi_hdr, filesize) == -1)
	__leave;

      mqinfo->mqi_magic = 0;          /* just in case */
      ipc_cond_close (mqinfo->mqi_waitsend);
      ipc_cond_close (mqinfo->mqi_waitrecv);
      ipc_mutex_close (mqinfo->mqi_lock);
      free (mqinfo);
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
	  if (stat64 (semname, &statbuff) == -1)
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

int
_sem_close (sem_t *sem, bool do_close)
{
  sem_finfo sf;
  int fd, ret = -1;
  ipc_flock file;

  if (semaphore::getinternal (sem, &fd, &sf.hash, &sf.luid, &sf.value) == -1)
    return -1;
  if (!file.lock (fd, sizeof sf)
      && lseek64 (fd, 0LL, SEEK_SET) != (off_t) -1
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
