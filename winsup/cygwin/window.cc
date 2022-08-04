/* window.cc: hidden windows for signals/itimer support

   Written by Sergey Okhapkin <sos@prospect.com.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/time.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include "perprocess.h"
#include "cygtls.h"
#include "sync.h"
#include "wininfo.h"

wininfo NO_COPY winmsg;

muto NO_COPY wininfo::_lock;

int
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
      return DefWindowProcW (hwnd, uMsg, wParam, lParam);
    }
}

static LRESULT CALLBACK
process_window_events (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return winmsg.process (hwnd, uMsg, wParam, lParam);
}

/* Handle windows events.  Inherits ownership of the wininfo lock */
DWORD
wininfo::winthread ()
{
  MSG msg;
  WNDCLASSW wc;
  static NO_COPY WCHAR classname[] = L"CygwinWndClass";

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

  if (!RegisterClassW (&wc))
    api_fatal ("cannot register window class, %E");

  /* Create hidden window. */
  hwnd = CreateWindowExW (0, classname, classname, WS_POPUP, CW_USEDEFAULT,
			  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			  (HWND) NULL, (HMENU) NULL, user_data->hmodule,
			  (LPVOID) NULL);
  if (!hwnd)
    api_fatal ("couldn't create window, %E");
  release ();

  int ret;
  while ((ret = (int) GetMessageW (&msg, hwnd, 0, 0)) > 0)
    DispatchMessageW (&msg);

  return 0;
}

static DWORD
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
      cygthread *h = new cygthread (::winthread, this, "win");
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
