/* window.cc: hidden windows for signals/itimer support

   Copyright 1997, 1998, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

   Written by Sergey Okhapkin <sos@prospect.com.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <wingdi.h>
#include <winuser.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include <unistd.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "thread.h"
#include "cygtls.h"
#include "sync.h"
#include "wininfo.h"

wininfo NO_COPY winmsg;

muto NO_COPY *wininfo::lock;

wininfo::wininfo ()
{
  new_muto_name (lock, "!winlock");
}

int __stdcall
wininfo::process (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef NOSTRACE
  strace.wm (uMsg, wParam, lParam);
#endif
  switch (uMsg)
    {
    case WM_PAINT:
      return 0;
    case WM_DESTROY:
      PostQuitMessage (0);
      return 0;
    case WM_TIMER:
      if (wParam == timer_active)
	{
	  UINT elapse = itv.it_interval.tv_sec * 1000 +
			itv.it_interval.tv_usec / 1000;
	  KillTimer (hwnd, timer_active);
	  if (!elapse)
	    timer_active = 0;
	  else
	    {
	      timer_active = SetTimer (hwnd, 1, elapse, NULL);
	      start_time = GetTickCount ();
	      itv.it_value = itv.it_interval;
	    }
	  raise (SIGALRM);
	}
      return 0;
    case WM_ASYNCIO:
      if (WSAGETSELECTEVENT (lParam) == FD_OOB)
	raise (SIGURG);
      else
	raise (SIGIO);
      return 0;
    default:
      return DefWindowProc (hwnd, uMsg, wParam, lParam);
    }
}

static LRESULT CALLBACK
process_window_events (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return winmsg.process (hwnd, uMsg, wParam, lParam);
}

/* Handle windows events.  Inherits ownership of the wininfo lock */
DWORD WINAPI
wininfo::winthread ()
{
  MSG msg;
  WNDCLASS wc;
  static NO_COPY char classname[] = "CygwinWndClass";

  lock->grab ();
  /* Register the window class for the main window. */

  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC) process_window_events;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = user_data->hmodule;
  wc.hIcon = NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = classname;

  if (!RegisterClass (&wc))
    api_fatal ("cannot register window class, %E");

  /* Create hidden window. */
  hwnd = CreateWindow (classname, classname, WS_POPUP, CW_USEDEFAULT,
			   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			   (HWND) NULL, (HMENU) NULL, user_data->hmodule,
			   (LPVOID) NULL);
  if (!hwnd)
    api_fatal ("couldn't create window, %E");
  lock->release ();

  while (GetMessage (&msg, hwnd, 0, 0) == TRUE)
    DispatchMessage (&msg);

  return 0;
}

static DWORD WINAPI
winthread (VOID *arg)
{
  return  ((wininfo *) arg)->winthread ();
}

wininfo::operator
HWND ()
{
  if (hwnd)
    return hwnd;

  lock->acquire ();
  if (!hwnd)
    {
      lock->upforgrabs ();
      cygthread *h = new cygthread (::winthread, this, "win");
      h->SetThreadPriority (THREAD_PRIORITY_HIGHEST);
      h->zap_h ();
      lock->acquire ();
    }
  lock->release ();
  return hwnd;
}

extern "C" int
setitimer (int which, const struct itimerval *value, struct itimerval *oldvalue)
{
  if (which != ITIMER_REAL)
    {
      set_errno (ENOSYS);
      return -1;
    }
  return winmsg.setitimer (value, oldvalue);
}

/* FIXME: Very racy */
int __stdcall
wininfo::setitimer (const struct itimerval *value, struct itimerval *oldvalue)
{
  /* Check if we will wrap */
  if (itv.it_value.tv_sec >= (long) (UINT_MAX / 1000))
    {
      set_errno (EINVAL);
      return -1;
    }
  if (timer_active)
    {
      KillTimer (winmsg, timer_active);
      timer_active = 0;
    }
  if (oldvalue)
    *oldvalue = itv;
  if (value == NULL)
    {
      set_errno (EFAULT);
      return -1;
    }
  itv = *value;
  UINT elapse = itv.it_value.tv_sec * 1000 + itv.it_value.tv_usec / 1000;
  if (elapse == 0)
    if (itv.it_value.tv_usec)
      elapse = 1;
    else
      return 0;
  if (!(timer_active = SetTimer (winmsg, 1, elapse, NULL)))
    {
      __seterrno ();
      return -1;
    }
  start_time = GetTickCount ();
  return 0;
}

extern "C" int
getitimer (int which, struct itimerval *value)
{
  if (which != ITIMER_REAL)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (value == NULL)
    {
      set_errno (EFAULT);
      return -1;
    }
  return winmsg.getitimer (value);
}

/* FIXME: racy */
int __stdcall
wininfo::getitimer (struct itimerval *value)
{
  *value = itv;
  if (!timer_active)
    {
      value->it_value.tv_sec = 0;
      value->it_value.tv_usec = 0;
      return 0;
    }

  UINT elapse, val;

  elapse = GetTickCount () - start_time;
  val = itv.it_value.tv_sec * 1000 + itv.it_value.tv_usec / 1000;
  val -= elapse;
  value->it_value.tv_sec = val / 1000;
  value->it_value.tv_usec = val % 1000;
  return 0;
}

extern "C" unsigned int
alarm (unsigned int seconds)
{
  int ret;
  struct itimerval newt, oldt;

  newt.it_value.tv_sec = seconds;
  newt.it_value.tv_usec = 0;
  newt.it_interval.tv_sec = 0;
  newt.it_interval.tv_usec = 0;
  setitimer (ITIMER_REAL, &newt, &oldt);
  ret = oldt.it_value.tv_sec;
  if (ret == 0 && oldt.it_value.tv_usec)
    ret = 1;
  return ret;
}

extern "C" useconds_t
ualarm (useconds_t value, useconds_t interval)
{
  struct itimerval timer, otimer;

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = value;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = interval;

  if (setitimer (ITIMER_REAL, &timer, &otimer) < 0)
    return (u_int)-1;

  return (otimer.it_value.tv_sec * 1000000) + otimer.it_value.tv_usec;
}

bool
has_visible_window_station (void)
{
  HWINSTA station_hdl;
  USEROBJECTFLAGS uof;
  DWORD len;

  /* Check if the process is associated with a visible window station.
     These are processes running on the local desktop as well as processes
     running in terminal server sessions.
     Processes running in a service session not explicitely associated
     with the desktop (using the "Allow service to interact with desktop"
     property) are running in an invisible window station. */
  if ((station_hdl = GetProcessWindowStation ())
      && GetUserObjectInformationA (station_hdl, UOI_FLAGS, &uof,
				    sizeof uof, &len)
      && (uof.dwFlags & WSF_VISIBLE))
    return true;
  return false;
}
