/* fhandler_mqueue.cc: fhandler for POSIX message queue

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
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
fhandler_mqueue::mqinfo (const char *name, int8_t *mptr, HANDLE sect,
			 size_t size, mode_t mode, int flags)
{
  set_name (name);
  mqinfo ()->mqi_hdr = (struct mq_hdr *) mptr;
  mqinfo ()->mqi_sect = sect;
  mqinfo ()->mqi_sectsize = size;
  mqinfo ()->mqi_mode = mode;
  mqinfo ()->mqi_magic = MQI_MAGIC;
  mqinfo ()->mqi_flags = flags;
  return mqinfo ();
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

      DuplicateHandle (parent, mqinfo ()->mqi_sect,
		       GetCurrentProcess (), &mqinfo ()->mqi_sect,
		       0, FALSE, DUPLICATE_SAME_ACCESS);
      status = NtMapViewOfSection (mqinfo ()->mqi_sect, NtCurrentProcess (),
				   &mptr, 0, filesize, NULL, &filesize,
				   ViewShare, 0, PAGE_READWRITE);
      if (!NT_SUCCESS (status))
	api_fatal ("Mapping message queue failed in fork\n");
      else
	mqinfo ()->mqi_hdr = (struct mq_hdr *) mptr;
      DuplicateHandle (parent, mqinfo ()->mqi_waitsend,
		       GetCurrentProcess (), &mqinfo ()->mqi_waitsend,
		       0, FALSE, DUPLICATE_SAME_ACCESS);
      DuplicateHandle (parent, mqinfo ()->mqi_waitrecv,
		       GetCurrentProcess (), &mqinfo ()->mqi_waitrecv,
		       0, FALSE, DUPLICATE_SAME_ACCESS);
      DuplicateHandle (parent, mqinfo ()->mqi_lock,
		       GetCurrentProcess (), &mqinfo ()->mqi_lock,
		       0, FALSE, DUPLICATE_SAME_ACCESS);
    }
  __except (EFAULT) {}
  __endtry
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
