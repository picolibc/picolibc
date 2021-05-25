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
#include <mqueue.h>
#include <sys/param.h>

#define MSGSIZE(i)      roundup((i), sizeof(long))

struct mq_attr defattr = { 0, 10, 8192, 0 };	/* Linux defaults. */

fhandler_mqueue::fhandler_mqueue () :
  fhandler_disk_file ()
{
  close_on_exec (true);
}

int
fhandler_mqueue::open (int flags, mode_t mode)
{
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
	  return -1;
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
  mqinfo ()->mqi_flags = flags;

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
  if (get_file_attribute (get_handle (), pc, &mode, NULL, NULL))
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

int __reg2
fhandler_mqueue::fstat (struct stat *buf)
{
  int ret = fhandler_disk_file::fstat (buf);
  if (!ret)
    buf->st_dev = FH_MQUEUE;
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
    ret = _dup (GetCurrentProcess (), fhc);
  return ret;
}

void
fhandler_mqueue::fixup_after_fork (HANDLE parent)
{
  if (_dup (parent, this))
    api_fatal ("Creating IPC object failed in fork, %E");
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
