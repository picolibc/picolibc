/* window.cc: hidden windows for signals/itimer support

   Copyright 1997, 1998, 2000 Cygnus Solutions.

   Written by Sergey Okhapkin <sos@prospect.com.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "winsup.h"

static NO_COPY UINT timer_active = 0;
static NO_COPY struct itimerval itv;
static NO_COPY DWORD start_time;
static NO_COPY HWND ourhwnd = NULL;

static LRESULT CALLBACK
WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	    {
	      timer_active = 0;
	    }
	  else
	    {
	      timer_active = SetTimer (hwnd, 1, elapse, NULL);
	      start_time = GetTickCount ();
	      itv.it_value = itv.it_interval;
	    }
	  raise(SIGALRM);
	}
      return 0;
    case WM_ASYNCIO:
      raise (SIGIO);
      return 0;
    default:
      return DefWindowProc (hwnd, uMsg, wParam, lParam);
    }
}

static HANDLE window_started;

static DWORD WINAPI
Winmain (VOID *)
{
  MSG msg;
  WNDCLASS wc;
  static char classname[] = "CygwinWndClass";

  /* Register the window class for the main window. */

  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC) WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = user_data->hmodule;
  wc.hIcon = NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = classname;

  if (!RegisterClass (&wc))
    {
      system_printf ("Cannot register window class");
      return FALSE;
    }

  /* Create hidden window. */
  ourhwnd = CreateWindow (classname, classname,
	WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
	CW_USEDEFAULT, CW_USEDEFAULT, (HWND) NULL,
	(HMENU) NULL, user_data->hmodule, (LPVOID) NULL);

  SetEvent (window_started);

  if (!ourhwnd)
    {
      system_printf ("Cannot create window");
      return FALSE;
    }

  /* Start the message loop. */

  while (GetMessage (&msg, ourhwnd, 0, 0) == TRUE)
    {
      DispatchMessage (&msg);
    }

  return msg.wParam;
}

HWND __stdcall
gethwnd ()
{
  if (ourhwnd != NULL)
    return ourhwnd;

  HANDLE hThread;

  window_started = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  hThread = makethread (Winmain, NULL, 0, "win");
  if (!hThread)
    {
      system_printf ("Cannot start window thread");
    }
  else
    {
      SetThreadPriority (hThread, THREAD_PRIORITY_HIGHEST);
      CloseHandle (hThread);
    }
  WaitForSingleObject (window_started, INFINITE);
  CloseHandle (window_started);
  return ourhwnd;
}

void __stdcall
window_terminate ()
{
  if (ourhwnd)
    SendMessage (ourhwnd, WM_DESTROY, 0, 0);
}

extern "C"
int
setitimer (int which, const struct itimerval *value, struct itimerval *oldvalue)
{
  UINT elapse;

  if (which != ITIMER_REAL)
    {
      set_errno (EINVAL);
      return -1;
    }
  /* Check if we will wrap */
  if (itv.it_value.tv_sec >= (long) (UINT_MAX / 1000))
    {
      set_errno (EINVAL);
      return -1;
    }
  if (timer_active)
    {
      KillTimer (gethwnd(), timer_active);
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
  elapse = itv.it_value.tv_sec * 1000 + itv.it_value.tv_usec / 1000;
  if (elapse == 0)
    return 0;
  if (!(timer_active = SetTimer (gethwnd(), 1, elapse, NULL)))
    {
      __seterrno ();
      return -1;
    }
  start_time = GetTickCount ();
  return 0;
}

extern "C"
int
getitimer (int which, struct itimerval *value)
{
  UINT elapse, val;

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
  *value = itv;
  if (!timer_active)
    {
      value->it_value.tv_sec = 0;
      value->it_value.tv_usec = 0;
      return 0;
    }
  elapse = GetTickCount () - start_time;
  val = itv.it_value.tv_sec * 1000 + itv.it_value.tv_usec / 1000;
  val -= elapse;
  value->it_value.tv_sec = val/1000;
  value->it_value.tv_usec = val%1000;
  return 0;
}

extern "C"
unsigned int
alarm (unsigned int seconds)
{
  int ret;
  struct itimerval newt, oldt;

  getitimer (ITIMER_REAL, &oldt);

  newt.it_value.tv_sec = seconds;
  newt.it_value.tv_usec = 0;
  newt.it_interval.tv_sec = 0;
  newt.it_interval.tv_usec = 0;
  setitimer (ITIMER_REAL, &newt, NULL);
  ret = oldt.it_value.tv_sec;
  if (ret == 0 && oldt.it_value.tv_usec)
    ret = 1;
  return ret;
}
