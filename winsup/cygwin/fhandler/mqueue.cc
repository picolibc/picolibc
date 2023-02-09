/* fhandler_mqueue.cc: fhandler for POSIX message queue

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "shared_info.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "clock.h"
#include <stdio.h>
#include <mqueue.h>
#include <sys/param.h>

#define MSGSIZE(i)      roundup((i), sizeof(long))

#define FILESIZE	80

struct mq_attr defattr = { 0, 10, 8192, 0 };	/* Linux defaults. */

fhandler_mqueue::fhandler_mqueue () :
  fhandler_disk_file ()
{
  filebuf = (char *) ccalloc_abort (HEAP_BUF, 1, FILESIZE);
}

fhandler_mqueue::~fhandler_mqueue ()
{
  cfree (filebuf);
}

bool
fhandler_mqueue::valid_path ()
{
  const char *posix_basename = get_name () + MQ_LEN;
  size_t len = strlen (posix_basename);
  if (len > 0 && len <= NAME_MAX && !strpbrk (posix_basename, "/\\"))
    return true;
  return false;
}

int
fhandler_mqueue::open (int flags, mode_t mode)
{
  if (!valid_path ())
    {
      set_errno (EINVAL);
      return 0;
    }
  /* FIXME: reopen by handle semantics missing yet */
  flags &= ~(O_NOCTTY | O_PATH | O_BINARY | O_TEXT);
  return mq_open (flags, mode, NULL);
}

int
fhandler_mqueue::mq_open (int oflags, mode_t mode, struct mq_attr *attr)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  PUNICODE_STRING mqstream;
  OBJECT_ATTRIBUTES oa;
  struct mq_info *mqinfo = NULL;
  bool created = false;

  if ((oflags & ~(O_ACCMODE | O_CLOEXEC | O_CREAT | O_EXCL | O_NONBLOCK))
      || (oflags & O_ACCMODE) == O_ACCMODE)
    {
      set_errno (EINVAL);
      return 0;
    }

  /* attach a stream suffix to the NT filename, thus creating a stream. */
  mqstream = pc.get_nt_native_path (&ro_u_mq_suffix);
  pc.get_object_attr (oa, sec_none_nih);

again:
  if (oflags & O_CREAT)
    {
      /* Create and disallow sharing */
      status = NtCreateFile (&get_handle (),
			     GENERIC_READ | GENERIC_WRITE | DELETE
			     | SYNCHRONIZE, &oa, &io, NULL,
			     FILE_ATTRIBUTE_NORMAL, FILE_SHARE_DELETE,
			     FILE_CREATE,
			     FILE_OPEN_FOR_BACKUP_INTENT
			     | FILE_SYNCHRONOUS_IO_NONALERT,
			     NULL, 0);
      if (!NT_SUCCESS (status))
	{
	  if (status == STATUS_OBJECT_NAME_COLLISION && (oflags & O_EXCL) == 0)
	    goto exists;
	  __seterrno_from_nt_status (status);
	  return 0;
	}
      if (pc.has_acls ())
	set_created_file_access (get_handle (), pc, mode);
      created = true;
      goto out;
    }
exists:
  /* Open the file, and loop while detecting a sharing violation. */
  while (true)
    {
      status = NtOpenFile (&get_handle (),
			   GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
			   &oa, &io, FILE_SHARE_VALID_FLAGS,
			   FILE_OPEN_FOR_BACKUP_INTENT
			   | FILE_SYNCHRONOUS_IO_NONALERT);
      if (NT_SUCCESS (status))
	break;
      if (status == STATUS_OBJECT_NAME_NOT_FOUND && (oflags & O_CREAT))
	goto again;
      if (status != STATUS_SHARING_VIOLATION)
	{
	  __seterrno_from_nt_status (status);
	  return 0;
	}
      Sleep (100L);
    }
out:
  /* We need the filename without STREAM_SUFFIX later on */
  mqstream->Length -= ro_u_mq_suffix.Length;
  mqstream->Buffer[mqstream->Length / sizeof (WCHAR)] = L'\0';

  if (created)
    {
      if (attr == NULL)
	attr = &defattr;
      /* Check minimum and maximum values.  The max values are pretty much
	 arbitrary, taken from the linux mq_overview man page, up to Linux
	 3.4.  These max values make sure that the internal mq_fattr
	 structure can use 32 bit types. */
      if (attr->mq_maxmsg <= 0 || attr->mq_maxmsg > 32768
	       || attr->mq_msgsize <= 0 || attr->mq_msgsize > 1048576)
	set_errno (EINVAL);
      else
	mqinfo = mqinfo_create (attr, mode, oflags & O_NONBLOCK);
    }
  else
    mqinfo = mqinfo_open (oflags & O_NONBLOCK);
  mq_open_finish (mqinfo != NULL, created);
  /* Set fhandler open flags */
  if (mqinfo)
    {
      set_access (GENERIC_READ | SYNCHRONIZE);
      close_on_exec (true);
      set_flags (oflags | O_CLOEXEC, O_BINARY);
      set_open_status ();
    }
  return mqinfo ? 1 : 0;
}

struct mq_info *
fhandler_mqueue::_mqinfo (SIZE_T filesize, mode_t mode, int flags,
			  bool just_open)
{
  WCHAR buf[NAME_MAX + sizeof ("mqueue/XXX")];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES oa;
  NTSTATUS status;
  LARGE_INTEGER fsiz = { QuadPart: (LONGLONG) filesize };
  PVOID mptr = NULL;

  /* Set sectsize prior to using filesize in NtMapViewOfSection.  It will
     get pagesize aligned, which breaks the next NtMapViewOfSection in fork. */
  mqinfo ()->mqi_sectsize = filesize;
  mqinfo ()->mqi_mode = mode;
  set_nonblocking (flags & O_NONBLOCK);

  __small_swprintf (buf, L"mqueue/mtx%s", get_name ());
  RtlInitUnicodeString (&uname, buf);
  InitializeObjectAttributes (&oa, &uname, OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                              get_shared_parent_dir (),
                              everyone_sd (CYG_MUTANT_ACCESS));
  status = NtCreateMutant (&mqinfo ()->mqi_lock, CYG_MUTANT_ACCESS, &oa,
			   FALSE);
  if (!NT_SUCCESS (status))
    goto err;

  wcsncpy (buf + 7, L"snd", 3);
  /* same length, no RtlInitUnicodeString required */
  InitializeObjectAttributes (&oa, &uname, OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                              get_shared_parent_dir (),
                              everyone_sd (CYG_EVENT_ACCESS));
  status = NtCreateEvent (&mqinfo ()->mqi_waitsend, CYG_EVENT_ACCESS, &oa,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    goto err;
  wcsncpy (buf + 7, L"rcv", 3);
  /* same length, same attributes, no more init required */
  status = NtCreateEvent (&mqinfo ()->mqi_waitrecv, CYG_EVENT_ACCESS, &oa,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    goto err;

  InitializeObjectAttributes (&oa, NULL, 0, NULL, NULL);
  status = NtCreateSection (&mqinfo ()->mqi_sect, SECTION_ALL_ACCESS, &oa,
			    &fsiz, PAGE_READWRITE, SEC_COMMIT, get_handle ());
  if (!NT_SUCCESS (status))
    goto err;

  status = NtMapViewOfSection (mqinfo ()->mqi_sect, NtCurrentProcess (),
			       &mptr, 0, filesize, NULL, &filesize,
			       ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    goto err;

  mqinfo ()->mqi_hdr = (struct mq_hdr *) mptr;

  /* Special problem on Cygwin.  /dev/mqueue is just a simple dir,
     so there's a chance normal files are created in there. */
  if (just_open && mqinfo ()->mqi_hdr->mqh_magic != MQI_MAGIC)
    {
      status = STATUS_ACCESS_DENIED;
      goto err;
    }

  mqinfo ()->mqi_magic = MQI_MAGIC;
  return mqinfo ();

err:
  if (mqinfo ()->mqi_sect)
    NtClose (mqinfo ()->mqi_sect);
  if (mqinfo ()->mqi_waitrecv)
    NtClose (mqinfo ()->mqi_waitrecv);
  if (mqinfo ()->mqi_waitsend)
    NtClose (mqinfo ()->mqi_waitsend);
  if (mqinfo ()->mqi_lock)
    NtClose (mqinfo ()->mqi_lock);
  __seterrno_from_nt_status (status);
  return NULL;
}

struct mq_info *
fhandler_mqueue::mqinfo_open (int flags)
{
  FILE_STANDARD_INFORMATION fsi;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  mode_t mode;

  fsi.EndOfFile.QuadPart = 0;
  status = NtQueryInformationFile (get_handle (), &io, &fsi, sizeof fsi,
				   FileStandardInformation);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  if (get_file_attribute (get_handle (), pc, mode, NULL, NULL))
    mode = STD_RBITS | STD_WBITS;

  return _mqinfo (fsi.EndOfFile.QuadPart, mode, flags, true);
}

struct mq_info *
fhandler_mqueue::mqinfo_create (struct mq_attr *attr, mode_t mode, int flags)
{
  long msgsize;
  off_t filesize = 0;
  FILE_END_OF_FILE_INFORMATION feofi;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  struct mq_info *mqinfo = NULL;

  msgsize = MSGSIZE (attr->mq_msgsize);
  filesize = sizeof (struct mq_hdr)
	     + (attr->mq_maxmsg * (sizeof (struct msg_hdr) + msgsize));
  feofi.EndOfFile.QuadPart = filesize;
  status = NtSetInformationFile (get_handle (), &io, &feofi, sizeof feofi,
				 FileEndOfFileInformation);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }

  mqinfo = _mqinfo (filesize, mode, flags, false);

  if (mqinfo)
    {
      /* Initialize header at beginning of file */
      /* Create free list with all messages on it */
      int8_t *mptr;
      struct mq_hdr *mqhdr;
      struct msg_hdr *msghdr;

      mptr = (int8_t *) mqinfo->mqi_hdr;
      mqhdr = mqinfo->mqi_hdr;
      mqhdr->mqh_attr.mq_flags = 0;
      mqhdr->mqh_attr.mq_maxmsg = attr->mq_maxmsg;
      mqhdr->mqh_attr.mq_msgsize = attr->mq_msgsize;
      mqhdr->mqh_attr.mq_curmsgs = 0;
      mqhdr->mqh_nwait = 0;
      mqhdr->mqh_pid = 0;
      mqhdr->mqh_head = 0;
      mqhdr->mqh_magic = MQI_MAGIC;
      long index = sizeof (struct mq_hdr);
      mqhdr->mqh_free = index;
      for (int i = 0; i < attr->mq_maxmsg - 1; i++)
	{
	  msghdr = (struct msg_hdr *) &mptr[index];
	  index += sizeof (struct msg_hdr) + msgsize;
	  msghdr->msg_next = index;
	}
      msghdr = (struct msg_hdr *) &mptr[index];
      msghdr->msg_next = 0;         /* end of free list */
    }

  return mqinfo;
}

void
fhandler_mqueue::mq_open_finish (bool success, bool created)
{
  NTSTATUS status;
  HANDLE def_stream;
  OBJECT_ATTRIBUTES oa;
  IO_STATUS_BLOCK io;

  if (get_handle ())
    {
      /* If we have an open queue stream handle, close it and set it to NULL */
      HANDLE queue_stream = get_handle ();
      set_handle (NULL);
      if (success)
	{
	  /* In case of success, open the default stream for reading.  This
	     can be used to implement various IO functions without exposing
	     the actual message queue. */
	  pc.get_object_attr (oa, sec_none_nih);
	  status = NtOpenFile (&def_stream, GENERIC_READ | SYNCHRONIZE,
			       &oa, &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT
			       | FILE_SYNCHRONOUS_IO_NONALERT);
	  if (NT_SUCCESS (status))
	    set_handle (def_stream);
	  else /* Note that we don't treat this as an error! */
	    {
	      debug_printf ("Opening default stream failed: status %y", status);
	      nohandle (true);
	    }
	}
      else if (created)
	{
	  /* In case of error at creation time, delete the file */
	  FILE_DISPOSITION_INFORMATION disp = { TRUE };

	  NtSetInformationFile (queue_stream, &io, &disp, sizeof disp,
				FileDispositionInformation);
	  /* We also have to set the delete disposition on the default stream,
	     otherwise only the queue stream will get deleted */
	  pc.get_object_attr (oa, sec_none_nih);
	  status = NtOpenFile (&def_stream, DELETE, &oa, &io,
			       FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT);
	  if (NT_SUCCESS (status))
	    {
	      NtSetInformationFile (def_stream, &io, &disp, sizeof disp,
				    FileDispositionInformation);
	      NtClose (def_stream);
	    }
	}
      NtClose (queue_stream);
    }
}

char *
fhandler_mqueue::get_proc_fd_name (char *buf)
{
  return strcpy (buf, strrchr (get_name (), '/'));
}

int
fhandler_mqueue::fcntl (int cmd, intptr_t arg)
{
  int res;

  switch (cmd)
    {
    case F_GETFD:
      res = close_on_exec () ? FD_CLOEXEC : 0;
      break;
    case F_GETFL:
      res = get_flags ();
      debug_printf ("GETFL: %y", res);
      break;
    default:
      set_errno (EINVAL);
      res = -1;
      break;
    }
  return res;
}

/* Do what fhandler_virtual does for read/lseek */
bool
fhandler_mqueue::fill_filebuf ()
{
  unsigned long qsize = 0;
  int notify = 0;
  int signo = 0;
  int notify_pid = 0;

  if (mutex_lock (mqinfo ()->mqi_lock, true) == 0)
    {
      struct mq_hdr *mqhdr = mqinfo ()->mqi_hdr;
      int8_t *mptr = (int8_t *) mqhdr;
      struct msg_hdr *msghdr;
      for (long index = mqhdr->mqh_head; index; index = msghdr->msg_next)
	{
	  msghdr = (struct msg_hdr *) &mptr[index];
	  qsize += msghdr->msg_len;
	}
      if (mqhdr->mqh_pid)
	{
	  notify = mqhdr->mqh_event.sigev_notify;
	  if (notify == SIGEV_SIGNAL)
	    signo = mqhdr->mqh_event.sigev_signo;
	  notify_pid = mqhdr->mqh_pid;
	}
      mutex_unlock (mqinfo ()->mqi_lock);
    }
  /* QSIZE:      bytes of all current msgs
     NOTIFY:     sigev_notify if there's a notifier
     SIGNO:      signal number if NOTIFY && sigev_notify == SIGEV_SIGNAL
     NOTIFY_PID: if NOTIFY pid */
  snprintf (filebuf, FILESIZE,
	    "QSIZE:%-10lu NOTIFY:%-5d SIGNO:%-5d NOTIFY_PID:%-6d\n",
	    qsize, notify, signo, notify_pid);
  filesize = strlen (filebuf);
  return true;
}

void
fhandler_mqueue::read (void *in_ptr, size_t& len)
{
  if (len == 0)
    return;
  if (!filebuf[0] && !fill_filebuf ())
    {
      len = (size_t) -1;
      return;
    }
  if ((ssize_t) len > filesize - position)
    len = (size_t) (filesize - position);
  if ((ssize_t) len < 0)
    len = 0;
  else
    memcpy (in_ptr, filebuf + position, len);
  position += len;
}

off_t
fhandler_mqueue::lseek (off_t offset, int whence)
{
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
fhandler_mqueue::fstat (struct stat *buf)
{
  int ret = fhandler_disk_file::fstat (buf);
  if (!ret)
    {
      buf->st_size = FILESIZE;
      buf->st_dev = FH_MQUEUE;
    }
  return ret;
}

int
fhandler_mqueue::_dup (HANDLE parent, fhandler_mqueue *fhc)
{
  __try
    {
      PVOID mptr = NULL;
      SIZE_T filesize = mqinfo ()->mqi_sectsize;
      NTSTATUS status;

      if (!DuplicateHandle (parent, mqinfo ()->mqi_sect,
			    GetCurrentProcess (), &fhc->mqinfo ()->mqi_sect,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      status = NtMapViewOfSection (mqinfo ()->mqi_sect, NtCurrentProcess (),
				   &mptr, 0, filesize, NULL, &filesize,
				   ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);
      if (!NT_SUCCESS (status))
	api_fatal ("Mapping message queue failed in fork, status 0x%x\n",
		   status);

      fhc->mqinfo ()->mqi_hdr = (struct mq_hdr *) mptr;
      if (!DuplicateHandle (parent, mqinfo ()->mqi_waitsend,
			    GetCurrentProcess (), &fhc->mqinfo ()->mqi_waitsend,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      if (!DuplicateHandle (parent, mqinfo ()->mqi_waitrecv,
			    GetCurrentProcess (), &fhc->mqinfo ()->mqi_waitrecv,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      if (!DuplicateHandle (parent, mqinfo ()->mqi_lock,
			    GetCurrentProcess (), &fhc->mqinfo ()->mqi_lock,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      return 0;
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

int
fhandler_mqueue::dup (fhandler_base *child, int flags)
{
  fhandler_mqueue *fhc = (fhandler_mqueue *) child;

  int ret = fhandler_disk_file::dup (child, flags);
  if (!ret)
    {
      memcpy (fhc->filebuf, filebuf, FILESIZE);
      ret = _dup (GetCurrentProcess (), fhc);
    }
  return ret;
}

void
fhandler_mqueue::fixup_after_fork (HANDLE parent)
{
  if (_dup (parent, this))
    api_fatal ("Creating IPC object failed in fork, %E");
}

int
fhandler_mqueue::ioctl (unsigned int cmd, void *buf)
{
  return fhandler_base::ioctl (cmd, buf);
}

int
fhandler_mqueue::close ()
{
  __try
    {
      mqinfo ()->mqi_magic = 0;          /* just in case */
      NtUnmapViewOfSection (NtCurrentProcess (), mqinfo ()->mqi_hdr);
      NtClose (mqinfo ()->mqi_sect);
      NtClose (mqinfo ()->mqi_waitsend);
      NtClose (mqinfo ()->mqi_waitrecv);
      NtClose (mqinfo ()->mqi_lock);
    }
  __except (0) {}
  __endtry
  return 0;
}

int
fhandler_mqueue::mutex_lock (HANDLE mtx, bool eintr)
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

int
fhandler_mqueue::mutex_unlock (HANDLE mtx)
{
  return ReleaseMutex (mtx) ? 0 : geterrno_from_win_error ();
}

int
fhandler_mqueue::cond_timedwait (HANDLE evt, HANDLE mtx,
				 const struct timespec *abstime)
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
      if (!valid_timespec (*abstime))
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
  if ((ret = mutex_unlock (mtx)) != 0)
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
      fallthrough;
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
	  fallthrough;
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

void
fhandler_mqueue::cond_signal (HANDLE evt)
{
  SetEvent (evt);
}

int
fhandler_mqueue::mq_getattr (struct mq_attr *mqstat)
{
  int n;
  struct mq_hdr *mqhdr;
  struct mq_fattr *attr;

  __try
    {
      mqhdr = mqinfo ()->mqi_hdr;
      attr = &mqhdr->mqh_attr;
      if ((n = mutex_lock (mqinfo ()->mqi_lock, false)) != 0)
	{
	  errno = n;
	  __leave;
	}
      mqstat->mq_flags = is_nonblocking () ? O_NONBLOCK : 0;   /* per-open */
      mqstat->mq_maxmsg = attr->mq_maxmsg;    /* remaining three per-queue */
      mqstat->mq_msgsize = attr->mq_msgsize;
      mqstat->mq_curmsgs = attr->mq_curmsgs;

      mutex_unlock (mqinfo ()->mqi_lock);
      return 0;
    }
  __except (EBADF) {}
  __endtry
  return -1;
}

int
fhandler_mqueue::mq_setattr (const struct mq_attr *mqstat,
			     struct mq_attr *omqstat)
{
  int n;
  struct mq_hdr *mqhdr;
  struct mq_fattr *attr;

  __try
    {
      mqhdr = mqinfo ()->mqi_hdr;
      attr = &mqhdr->mqh_attr;
      if ((n = mutex_lock (mqinfo ()->mqi_lock, false)) != 0)
	{
	  errno = n;
	  __leave;
	}

      if (omqstat != NULL)
	{
	  omqstat->mq_flags = is_nonblocking () ? O_NONBLOCK : 0;
	  omqstat->mq_maxmsg = attr->mq_maxmsg;
	  omqstat->mq_msgsize = attr->mq_msgsize;
	  omqstat->mq_curmsgs = attr->mq_curmsgs; /* and current status */
	}

      set_nonblocking (mqstat->mq_flags & O_NONBLOCK);

      mutex_unlock (mqinfo ()->mqi_lock);
      return 0;
    }
  __except (EBADF) {}
  __endtry
  return -1;
}

int
fhandler_mqueue::mq_notify (const struct sigevent *notification)
{
  int n;
  pid_t pid;
  struct mq_hdr *mqhdr;

  __try
    {
      mqhdr = mqinfo ()->mqi_hdr;
      if ((n = mutex_lock (mqinfo ()->mqi_lock, false)) != 0)
	{
	  errno = n;
	  __leave;
	}

      pid = myself->pid;
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
		  mutex_unlock (mqinfo ()->mqi_lock);
		  __leave;
		}
	    }
	  mqhdr->mqh_pid = pid;
	  mqhdr->mqh_event = *notification;
	}
      mutex_unlock (mqinfo ()->mqi_lock);
      return 0;
    }
  __except (EBADF) {}
  __endtry
  return -1;
}

int
fhandler_mqueue::mq_timedsend (const char *ptr, size_t len, unsigned int prio,
			       const struct timespec *abstime)
{
  int n;
  long index, freeindex;
  int8_t *mptr;
  struct sigevent *sigev;
  struct mq_hdr *mqhdr;
  struct mq_fattr *attr;
  struct msg_hdr *msghdr, *nmsghdr, *pmsghdr;
  bool mutex_locked = false;
  int ret = -1;

  pthread_testcancel ();

  __try
    {
      if (prio >= MQ_PRIO_MAX)
	{
	  set_errno (EINVAL);
	  __leave;
	}

      mqhdr = mqinfo ()->mqi_hdr;     /* struct pointer */
      mptr = (int8_t *) mqhdr;        /* byte pointer */
      attr = &mqhdr->mqh_attr;
      if ((n = mutex_lock (mqinfo ()->mqi_lock, true)) != 0)
	{
	  errno = n;
	  __leave;
	}
      mutex_locked = true;
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
	  if (is_nonblocking ())
	    {
	      set_errno (EAGAIN);
	      __leave;
	    }
	  /* Wait for room for one message on the queue */
	  while (attr->mq_curmsgs >= attr->mq_maxmsg)
	    {
	      int ret = cond_timedwait (mqinfo ()->mqi_waitsend,
					mqinfo ()->mqi_lock, abstime);
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
	cond_signal (mqinfo ()->mqi_waitrecv);
      attr->mq_curmsgs++;

      ret = 0;
    }
  __except (EBADF) {}
  __endtry
  if (mutex_locked)
    mutex_unlock (mqinfo ()->mqi_lock);
  return ret;
}

ssize_t
fhandler_mqueue::mq_timedrecv (char *ptr, size_t maxlen, unsigned int *priop,
			       const struct timespec *abstime)
{
  int n;
  long index;
  int8_t *mptr;
  ssize_t len = -1;
  struct mq_hdr *mqhdr;
  struct mq_fattr *attr;
  struct msg_hdr *msghdr;
  bool mutex_locked = false;

  pthread_testcancel ();

  __try
    {
      mqhdr = mqinfo ()->mqi_hdr;     /* struct pointer */
      mptr = (int8_t *) mqhdr;        /* byte pointer */
      attr = &mqhdr->mqh_attr;
      if ((n = mutex_lock (mqinfo ()->mqi_lock, true)) != 0)
	{
	  errno = n;
	  __leave;
	}
      mutex_locked = true;
      if (maxlen < (size_t) attr->mq_msgsize)
	{
	  set_errno (EMSGSIZE);
	  __leave;
	}
      if (attr->mq_curmsgs == 0)	/* queue is empty */
	{
	  if (is_nonblocking ())
	    {
	      set_errno (EAGAIN);
	      __leave;
	    }
	  /* Wait for a message to be placed onto queue */
	  mqhdr->mqh_nwait++;
	  while (attr->mq_curmsgs == 0)
	    {
	      int ret = cond_timedwait (mqinfo ()->mqi_waitrecv,
					mqinfo ()->mqi_lock, abstime);
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
	cond_signal (mqinfo ()->mqi_waitsend);
      attr->mq_curmsgs--;
    }
  __except (EBADF) {}
  __endtry
  if (mutex_locked)
    mutex_unlock (mqinfo ()->mqi_lock);
  return len;
}
