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

fhandler_mqueue::fhandler_mqueue () :
  fhandler_base ()
{
  nohandle (true);
  close_on_exec (true);
}

struct mq_info *
fhandler_mqueue::_mqinfo (HANDLE fh, SIZE_T filesize, mode_t mode, int flags,
			  bool just_open)
{
  WCHAR buf[NAME_MAX + sizeof ("mqueue/XXX")];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
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
  InitializeObjectAttributes (&attr, &uname, OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                              get_shared_parent_dir (),
                              everyone_sd (CYG_MUTANT_ACCESS));
  status = NtCreateMutant (&mqinfo ()->mqi_lock, CYG_MUTANT_ACCESS, &attr,
			   FALSE);
  if (!NT_SUCCESS (status))
    goto err;

  wcsncpy (buf + 7, L"snd", 3);
  /* same length, no RtlInitUnicodeString required */
  InitializeObjectAttributes (&attr, &uname, OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                              get_shared_parent_dir (),
                              everyone_sd (CYG_EVENT_ACCESS));
  status = NtCreateEvent (&mqinfo ()->mqi_waitsend, CYG_EVENT_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    goto err;
  wcsncpy (buf + 7, L"rcv", 3);
  /* same length, same attributes, no more init required */
  status = NtCreateEvent (&mqinfo ()->mqi_waitrecv, CYG_EVENT_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    goto err;

  InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
  status = NtCreateSection (&mqinfo ()->mqi_sect, SECTION_ALL_ACCESS, &attr,
			    &fsiz, PAGE_READWRITE, SEC_COMMIT, fh);
  if (!NT_SUCCESS (status))
    goto err;

  status = NtMapViewOfSection (mqinfo ()->mqi_sect, NtCurrentProcess (),
			       &mptr, 0, filesize, NULL, &filesize,
			       ViewShare, 0, PAGE_READWRITE);
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

char *
fhandler_mqueue::get_proc_fd_name (char *buf)
{
  return strcpy (buf, get_name ());
}

int __reg2
fhandler_mqueue::fstat (struct stat *buf)
{
  int ret = fhandler_base::fstat (buf);
  if (!ret)
    {
      buf->st_mode = S_IFREG | mqinfo ()->mqi_mode;
      buf->st_dev = FH_MQUEUE;
      buf->st_ino = hash_path_name (0, get_name ());
    }
  return ret;
}

int
fhandler_mqueue::dup (fhandler_base *child, int flags)
{
  /* FIXME */
  set_errno (EBADF);
  return -1;
}

void
fhandler_mqueue::fixup_after_fork (HANDLE parent)
{
  __try
    {
      PVOID mptr = NULL;
      SIZE_T filesize = mqinfo ()->mqi_sectsize;
      NTSTATUS status;

      if (!DuplicateHandle (parent, mqinfo ()->mqi_sect,
			    GetCurrentProcess (), &mqinfo ()->mqi_sect,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      status = NtMapViewOfSection (mqinfo ()->mqi_sect, NtCurrentProcess (),
				   &mptr, 0, filesize, NULL, &filesize,
				   ViewShare, 0, PAGE_READWRITE);
      if (!NT_SUCCESS (status))
	api_fatal ("Mapping message queue failed in fork, status 0x%x\n",
		   status);

      mqinfo ()->mqi_hdr = (struct mq_hdr *) mptr;
      if (!DuplicateHandle (parent, mqinfo ()->mqi_waitsend,
			    GetCurrentProcess (), &mqinfo ()->mqi_waitsend,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      if (!DuplicateHandle (parent, mqinfo ()->mqi_waitrecv,
			    GetCurrentProcess (), &mqinfo ()->mqi_waitrecv,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      if (!DuplicateHandle (parent, mqinfo ()->mqi_lock,
			    GetCurrentProcess (), &mqinfo ()->mqi_lock,
			    0, FALSE, DUPLICATE_SAME_ACCESS))
	__leave;
      return;
    }
  __except (EFAULT) {}
  __endtry
  api_fatal ("Creating IPC object failed in fork, %E");
}

int
fhandler_mqueue::close ()
{
  int ret = -1;

  __try
    {
      mqinfo ()->mqi_magic = 0;          /* just in case */
      NtUnmapViewOfSection (NtCurrentProcess (), mqinfo ()->mqi_hdr);
      NtClose (mqinfo ()->mqi_sect);
      NtClose (mqinfo ()->mqi_waitsend);
      NtClose (mqinfo ()->mqi_waitrecv);
      NtClose (mqinfo ()->mqi_lock);
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}
