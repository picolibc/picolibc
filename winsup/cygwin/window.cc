/* window.cc: hidden windows for signals/itimer support

   Copyright 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.

   Written by Sergey Okhapkin <sos@prospect.com.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/time.h>
#include <wingdi.h>
#include <winuser.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include "perprocess.h"
#include "cygtls.h"
#include "sync.h"
#include "wininfo.h"

wininfo NO_COPY winmsg;

muto NO_COPY wininfo::_lock;

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

  _lock.grab ();
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
  release ();

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

  lock ();
  if (!hwnd)
    {
      _lock.upforgrabs ();
      cygthread *h = new cygthread (::winthread, 0, this, "win");
      h->SetThreadPriority (THREAD_PRIORITY_HIGHEST);
      h->zap_h ();
      lock ();
    }
  release ();
  return hwnd;
}

void
wininfo::lock ()
{
  _lock.init ("wininfo_lock")->acquire ();
}

void
wininfo::release ()
{
  _lock.release ();
}
