/* fhandler_windows.cc: code to access windows message queues.

   Copyright 1998 Cygnus Solutions.

   Written by Sergey S. Okhapkin (sos@prospect.com.ru).
   Feedback and testing by Andy Piper (andyp@parallax.co.uk).

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <wingdi.h>
#include <winuser.h>
#include "cygerrno.h"

/*
The following unix-style calls are supported:

	open ("/dev/windows", flags, mode=0)
		- create a unix fd for message queue.
		  O_NONBLOCK flag controls the read() call behavior.

	read (fd, buf, len)
		- return next message from queue. buf must point to MSG
		  structure, len must be >= sizeof (MSG). If read is set to
		  non-blocking and the queue is empty, read call returns -1
		  immediately with errno set to EAGAIN, otherwise it blocks
		  untill the message will be received.

	write (fd, buf, len)
		- send a message pointed by buf. len argument ignored.

	ioctl (fd, command, *param)
		- control read()/write() behavior.
		ioctl (fd, WINDOWS_POST, NULL): write() will PostMessage();
		ioctl (fd, WINDOWS_SEND, NULL): write() will SendMessage();
		ioctl (fd, WINDOWS_HWND, &hWnd): read() messages for
			hWnd window.

	select () call marks read fd when any message posted to queue.
*/

fhandler_windows::fhandler_windows (const char *name) :
	fhandler_base (FH_WINDOWS, name)
{
  set_cb (sizeof *this);
  hWnd_ = NULL;
  method_ = WINDOWS_POST;
}

int
fhandler_windows::open (const char *, int flags, mode_t)
{
  set_flags (flags);
  set_close_on_exec_flag (1);
  return 1;
}

int
fhandler_windows::write (const void *buf, size_t)
{
  MSG *ptr = (MSG *) buf;

  if (method_ == WINDOWS_POST)
    {
      if (!PostMessage (ptr->hwnd, ptr->message, ptr->wParam, ptr->lParam))
	{
	  __seterrno ();
	  return -1;
	}
      else
	return sizeof (MSG);
    }
  else
    return SendMessage (ptr->hwnd, ptr->message, ptr->wParam, ptr->lParam);
}

int
fhandler_windows::read (void *buf, size_t len)
{
  MSG *ptr = (MSG *) buf;
  int ret;

  if (len < sizeof (MSG))
    {
      set_errno (EINVAL);
      return -1;
    }

  ret = GetMessage (ptr, hWnd_, 0, 0);

  if (ret == -1)
    {
      __seterrno ();
    }
  set_errno (0);
  return ret;
}

int
fhandler_windows::ioctl (unsigned int cmd, void *val)
{
  switch (cmd)
    {
    case WINDOWS_POST:
    case WINDOWS_SEND:
      method_ = cmd;
      break;
    case WINDOWS_HWND:
      if (val == NULL)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      hWnd_ = * ((HWND *) val);
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }
  return 0;
}

void
fhandler_windows::set_close_on_exec (int val)
{
  if (get_handle ())
    this->fhandler_base::set_close_on_exec (val);
  else
    this->fhandler_base::set_close_on_exec_flag (val);
  void *h = hWnd_;
  if (h)
    set_inheritance (h, val);
}

void
fhandler_windows::fixup_after_fork (HANDLE parent)
{
  if (get_handle ())
    this->fhandler_base::fixup_after_fork (parent);
  void *h = hWnd_;
  if (h)
    fork_fixup (parent, h, "hWnd_");
}
