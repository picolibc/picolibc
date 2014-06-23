/* strace.cc: system/windows tracing

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <ctype.h>
#include "cygerrno.h"
#include "pinfo.h"
#include "perprocess.h"
#include "cygwin_version.h"
#include "cygthread.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"

#define PROTECT(x) x[sizeof (x)-1] = 0
#define CHECK(x) if (x[sizeof (x)-1] != 0) { small_printf ("array bound exceeded %d\n", __LINE__); ExitProcess (1); }

class strace NO_COPY strace;

#ifndef NOSTRACE

void
strace::activate (bool isfork)
{
  if (!_active && being_debugged ())
    {
      char buf[30];
      __small_sprintf (buf, "cYg%8x %lx %d", _STRACE_INTERFACE_ACTIVATE_ADDR, &_active, isfork);
      OutputDebugString (buf);
      if (_active)
	{
	  char pidbuf[80];
	  WCHAR progname_buf[NT_MAX_PATH - 512];
	  WCHAR *progname;
	  if (myself)
	    {
	      __small_sprintf (pidbuf, "(pid %d, ppid %d, windows pid %u)", myself->pid,
			       myself->ppid ?: 1, GetCurrentProcessId ());
	      progname = myself->progname;
	    }
	  else
	    {
	      GetModuleFileNameW (NULL, progname_buf,
	      			  sizeof progname_buf / sizeof (WCHAR));
	      __small_sprintf (pidbuf, "(windows pid %u)", GetCurrentProcessId ());
	      progname = progname_buf;
	    }
	  prntf (1, NULL, "**********************************************");
	  prntf (1, NULL, "Program name: %W %s", progname, pidbuf);
	  prntf (1, NULL, "OS version:   Windows %s", wincap.osname ());
	  prntf (1, NULL, "**********************************************");
	}
    }
}

void
strace::dll_info ()
{
  if (active ())
    {
      prntf (1, NULL, "App version:  %d.%d, api: %d.%d",
	     user_data->dll_major, user_data->dll_minor,
	     user_data->api_major, user_data->api_minor);
      prntf (1, NULL, "DLL version:  %d.%d, api: %d.%d",
	     cygwin_version.dll_major, cygwin_version.dll_minor,
	     cygwin_version.api_major, cygwin_version.api_minor);
      prntf (1, NULL, "DLL build:    %s", cygwin_version.dll_build_date);
    }
}

int
strace::microseconds ()
{
  static hires_ns now;
  return (int) now.usecs ();
}

static int __stdcall
getfunc (char *in_dst, const char *func)
{
  const char *p;
  const char *pe;
  char *dst = in_dst;
  for (p = func; (pe = strchr (p, '(')); p = pe + 1)
    if (isalnum ((int)pe[-1]) || pe[-1] == '_')
      break;
    else if (isspace ((int)pe[-1]))
      {
	pe--;
	break;
      }
  if (!pe)
    pe = strchr (func, '\0');
  for (p = pe; p > func; p--)
    if (p != pe && *p == ' ')
      {
	p++;
	break;
      }
  if (*p == '*')
    p++;
  while (p < pe)
    *dst++ = *p++;

  *dst++ = ':';
  *dst++ = ' ';
  *dst = '\0';

  return dst - in_dst;
}

static char *
mypid (char *buf)
{
  if (myself && myself->pid)
    __small_sprintf (buf, "%d", myself->pid);
  else
    __small_sprintf (buf, "(%d)", GetCurrentProcessId ());
  return buf;
}

/* sprintf analog for use by output routines. */
int
strace::vsprntf (char *buf, const char *func, const char *infmt, va_list ap)
{
  int count;
  char fmt[80];
  static NO_COPY bool nonewline = false;
  DWORD err = GetLastError ();
  const char *tn = cygthread::name ();

  int microsec = microseconds ();
  lmicrosec = microsec;

  __small_sprintf (fmt, "%7d [%s] %s ", microsec, tn, "%W %s%s");

  SetLastError (err);

  if (nonewline)
    count = 0;
  else
    {
      PWCHAR pn = NULL;
      WCHAR progname[NT_MAX_PATH];
      if (!cygwin_finished_initializing)
	pn = (myself) ? myself->progname : NULL;
      else if (__progname)
	sys_mbstowcs(pn = progname, NT_MAX_PATH, __progname);

      WCHAR empty[1] = {};
      PWCHAR p;
      if (!pn)
	GetModuleFileNameW (NULL, pn = progname, sizeof (progname));
      if (!pn)
	p = empty;
      else if ((p = wcsrchr (pn, L'\\')) != NULL)
	p++;
      else if ((p = wcsrchr (pn, L'/')) != NULL)
	p++;
      else
	p = pn;
      if (p != progname)
	wcscpy (progname, p);
      if ((p = wcsrchr (progname, '.')) != NULL
	  && !wcscasecmp (p, L".exe"))
	*p = '\000';
      p = progname;
      char tmpbuf[20];
      count = __small_sprintf (buf, fmt, *p ? p : L"?", mypid (tmpbuf),
			       execing ? "!" : "");
      if (func)
	count += getfunc (buf + count, func);
    }

  count += __small_vsprintf (buf + count, infmt, ap);
  char *p;
  for (p = buf + count; p > buf; p--)
    switch (p[-1])
      {
	case '\n':
	  p[-1] = '\0';
	  break;
	case '\b':
	  *--p = '\0';
	   nonewline = true;
	  goto done;
	default:
	  goto addnl;
      }

addnl:
  *p++ = '\n';
  *p = '\0';
  nonewline = false;

done:
  return p - buf;
}

/* Write to strace file or strace queue. */
void
strace::write (unsigned category, const char *buf, int count)
{
# define PREFIX (3 + 8 + 1 + 8 + 1)
  char outbuf[PREFIX + 1 + count + 1];
# define outstuff (outbuf + 12)
  __small_sprintf (outstuff, "%x %s", category, buf);
  __small_sprintf (outbuf, "cYg%08x", strlen (outstuff) + 1);
  outstuff[-1] = ' ';
  OutputDebugString (outbuf);
#undef outstuff
#undef PREFIX
}

void
strace::write_childpid (pid_t pid)
{
  char buf[30];

  if (!attached () || !being_debugged ())
    return;
  __small_sprintf (buf, "cYg%8x %x", _STRACE_CHILD_PID, pid);
  OutputDebugString (buf);
}

/* Printf function used when tracing system calls.
   Warning: DO NOT SET ERRNO HERE! */

void
strace::vprntf (unsigned category, const char *func, const char *fmt, va_list ap)
{
  DWORD err = GetLastError ();
  int len;
  char buf[NT_MAX_PATH];

  PROTECT (buf);
  SetLastError (err);

  len = vsprntf (buf, func, fmt, ap);
  CHECK (buf);
  if (category & _STRACE_SYSTEM)
    {
      DWORD done;
      WriteFile (GetStdHandle (STD_ERROR_HANDLE), buf, len, &done, 0);
      FlushFileBuffers (GetStdHandle (STD_ERROR_HANDLE));
      /* Make sure that the message shows up on the screen, too, since this is
	 a serious error. */
      if (GetFileType (GetStdHandle (STD_ERROR_HANDLE)) != FILE_TYPE_CHAR)
	{
	  HANDLE h = CreateFile ("CONOUT$", GENERIC_READ | GENERIC_WRITE,
				 FILE_SHARE_READ | FILE_SHARE_WRITE,
				 &sec_none, OPEN_EXISTING, 0, 0);
	  if (h != INVALID_HANDLE_VALUE)
	    {
	      WriteFile (h, buf, len, &done, 0);
	      CloseHandle (h);
	    }
	}
    }

#ifndef NOSTRACE
  if (active ())
    write (category, buf, len);
#endif
  SetLastError (err);
}

void
strace::prntf (unsigned category, const char *func, const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vprntf (category, func, fmt, ap);
  va_end (ap);
}

extern "C" void
strace_printf (unsigned category, const char *func, const char *fmt, ...)
{
  va_list ap;

  if ((category & _STRACE_SYSTEM) || strace.active ())
    {
      va_start (ap, fmt);
      strace.vprntf (category, func, fmt, ap);
      va_end (ap);
    }
}

static NO_COPY struct tab
{
  int v;
  const char *n;
}
ta[] =
{
  {  WM_NULL, "WM_NULL"  },
  {  WM_CREATE, "WM_CREATE"  },
  {  WM_DESTROY, "WM_DESTROY"  },
  {  WM_MOVE, "WM_MOVE"  },
  {  WM_SIZE, "WM_SIZE"  },
  {  WM_ACTIVATE, "WM_ACTIVATE"  },
  {  WM_SETFOCUS, "WM_SETFOCUS"  },
  {  WM_KILLFOCUS, "WM_KILLFOCUS"  },
  {  WM_ENABLE, "WM_ENABLE"  },
  {  WM_SETREDRAW, "WM_SETREDRAW"  },
  {  WM_SETTEXT, "WM_SETTEXT"  },
  {  WM_GETTEXT, "WM_GETTEXT"  },
  {  WM_GETTEXTLENGTH, "WM_GETTEXTLENGTH"  },
  {  WM_PAINT, "WM_PAINT"  },
  {  WM_CLOSE, "WM_CLOSE"  },
  {  WM_QUERYENDSESSION, "WM_QUERYENDSESSION"  },
  {  WM_QUIT, "WM_QUIT"  },
  {  WM_QUERYOPEN, "WM_QUERYOPEN"  },
  {  WM_ERASEBKGND, "WM_ERASEBKGND"  },
  {  WM_SYSCOLORCHANGE, "WM_SYSCOLORCHANGE"  },
  {  WM_ENDSESSION, "WM_ENDSESSION"  },
  {  WM_SHOWWINDOW, "WM_SHOWWINDOW"  },
  {  WM_WININICHANGE, "WM_WININICHANGE"  },
  {  WM_DEVMODECHANGE, "WM_DEVMODECHANGE"  },
  {  WM_ACTIVATEAPP, "WM_ACTIVATEAPP"  },
  {  WM_FONTCHANGE, "WM_FONTCHANGE"  },
  {  WM_TIMECHANGE, "WM_TIMECHANGE"  },
  {  WM_CANCELMODE, "WM_CANCELMODE"  },
  {  WM_SETCURSOR, "WM_SETCURSOR"  },
  {  WM_MOUSEACTIVATE, "WM_MOUSEACTIVATE"  },
  {  WM_CHILDACTIVATE, "WM_CHILDACTIVATE"  },
  {  WM_QUEUESYNC, "WM_QUEUESYNC"  },
  {  WM_GETMINMAXINFO, "WM_GETMINMAXINFO"  },
  {  WM_PAINTICON, "WM_PAINTICON"  },
  {  WM_ICONERASEBKGND, "WM_ICONERASEBKGND"  },
  {  WM_NEXTDLGCTL, "WM_NEXTDLGCTL"  },
  {  WM_SPOOLERSTATUS, "WM_SPOOLERSTATUS"  },
  {  WM_DRAWITEM, "WM_DRAWITEM"  },
  {  WM_MEASUREITEM, "WM_MEASUREITEM"  },
  {  WM_DELETEITEM, "WM_DELETEITEM"  },
  {  WM_VKEYTOITEM, "WM_VKEYTOITEM"  },
  {  WM_CHARTOITEM, "WM_CHARTOITEM"  },
  {  WM_SETFONT, "WM_SETFONT"  },
  {  WM_GETFONT, "WM_GETFONT"  },
  {  WM_SETHOTKEY, "WM_SETHOTKEY"  },
  {  WM_GETHOTKEY, "WM_GETHOTKEY"  },
  {  WM_QUERYDRAGICON, "WM_QUERYDRAGICON"  },
  {  WM_COMPAREITEM, "WM_COMPAREITEM"  },
  {  WM_COMPACTING, "WM_COMPACTING"  },
  {  WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING"  },
  {  WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED"  },
  {  WM_POWER, "WM_POWER"  },
  {  WM_COPYDATA, "WM_COPYDATA"  },
  {  WM_CANCELJOURNAL, "WM_CANCELJOURNAL"  },
  {  WM_NCCREATE, "WM_NCCREATE"  },
  {  WM_NCDESTROY, "WM_NCDESTROY"  },
  {  WM_NCCALCSIZE, "WM_NCCALCSIZE"  },
  {  WM_NCHITTEST, "WM_NCHITTEST"  },
  {  WM_NCPAINT, "WM_NCPAINT"  },
  {  WM_NCACTIVATE, "WM_NCACTIVATE"  },
  {  WM_GETDLGCODE, "WM_GETDLGCODE"  },
  {  WM_NCMOUSEMOVE, "WM_NCMOUSEMOVE"  },
  {  WM_NCLBUTTONDOWN, "WM_NCLBUTTONDOWN"  },
  {  WM_NCLBUTTONUP, "WM_NCLBUTTONUP"  },
  {  WM_NCLBUTTONDBLCLK, "WM_NCLBUTTONDBLCLK"  },
  {  WM_NCRBUTTONDOWN, "WM_NCRBUTTONDOWN"  },
  {  WM_NCRBUTTONUP, "WM_NCRBUTTONUP"  },
  {  WM_NCRBUTTONDBLCLK, "WM_NCRBUTTONDBLCLK"  },
  {  WM_NCMBUTTONDOWN, "WM_NCMBUTTONDOWN"  },
  {  WM_NCMBUTTONUP, "WM_NCMBUTTONUP"  },
  {  WM_NCMBUTTONDBLCLK, "WM_NCMBUTTONDBLCLK"  },
  {  WM_KEYFIRST, "WM_KEYFIRST"  },
  {  WM_KEYDOWN, "WM_KEYDOWN"  },
  {  WM_KEYUP, "WM_KEYUP"  },
  {  WM_CHAR, "WM_CHAR"  },
  {  WM_DEADCHAR, "WM_DEADCHAR"  },
  {  WM_SYSKEYDOWN, "WM_SYSKEYDOWN"  },
  {  WM_SYSKEYUP, "WM_SYSKEYUP"  },
  {  WM_SYSCHAR, "WM_SYSCHAR"  },
  {  WM_SYSDEADCHAR, "WM_SYSDEADCHAR"  },
  {  WM_KEYLAST, "WM_KEYLAST"  },
  {  WM_INITDIALOG, "WM_INITDIALOG"  },
  {  WM_COMMAND, "WM_COMMAND"  },
  {  WM_SYSCOMMAND, "WM_SYSCOMMAND"  },
  {  WM_TIMER, "WM_TIMER"  },
  {  WM_HSCROLL, "WM_HSCROLL"  },
  {  WM_VSCROLL, "WM_VSCROLL"  },
  {  WM_INITMENU, "WM_INITMENU"  },
  {  WM_INITMENUPOPUP, "WM_INITMENUPOPUP"  },
  {  WM_MENUSELECT, "WM_MENUSELECT"  },
  {  WM_MENUCHAR, "WM_MENUCHAR"  },
  {  WM_ENTERIDLE, "WM_ENTERIDLE"  },
  {  WM_CTLCOLORMSGBOX, "WM_CTLCOLORMSGBOX"  },
  {  WM_CTLCOLOREDIT, "WM_CTLCOLOREDIT"  },
  {  WM_CTLCOLORLISTBOX, "WM_CTLCOLORLISTBOX"  },
  {  WM_CTLCOLORBTN, "WM_CTLCOLORBTN"  },
  {  WM_CTLCOLORDLG, "WM_CTLCOLORDLG"  },
  {  WM_CTLCOLORSCROLLBAR, "WM_CTLCOLORSCROLLBAR"  },
  {  WM_CTLCOLORSTATIC, "WM_CTLCOLORSTATIC"  },
  {  WM_MOUSEFIRST, "WM_MOUSEFIRST"  },
  {  WM_MOUSEMOVE, "WM_MOUSEMOVE"  },
  {  WM_LBUTTONDOWN, "WM_LBUTTONDOWN"  },
  {  WM_LBUTTONUP, "WM_LBUTTONUP"  },
  {  WM_LBUTTONDBLCLK, "WM_LBUTTONDBLCLK"  },
  {  WM_RBUTTONDOWN, "WM_RBUTTONDOWN"  },
  {  WM_RBUTTONUP, "WM_RBUTTONUP"  },
  {  WM_RBUTTONDBLCLK, "WM_RBUTTONDBLCLK"  },
  {  WM_MBUTTONDOWN, "WM_MBUTTONDOWN"  },
  {  WM_MBUTTONUP, "WM_MBUTTONUP"  },
  {  WM_MBUTTONDBLCLK, "WM_MBUTTONDBLCLK"  },
  {  WM_MOUSELAST, "WM_MOUSELAST"  },
  {  WM_PARENTNOTIFY, "WM_PARENTNOTIFY"  },
  {  WM_ENTERMENULOOP, "WM_ENTERMENULOOP"  },
  {  WM_EXITMENULOOP, "WM_EXITMENULOOP"  },
  {  WM_MDICREATE, "WM_MDICREATE"  },
  {  WM_MDIDESTROY, "WM_MDIDESTROY"  },
  {  WM_MDIACTIVATE, "WM_MDIACTIVATE"  },
  {  WM_MDIRESTORE, "WM_MDIRESTORE"  },
  {  WM_MDINEXT, "WM_MDINEXT"  },
  {  WM_MDIMAXIMIZE, "WM_MDIMAXIMIZE"  },
  {  WM_MDITILE, "WM_MDITILE"  },
  {  WM_MDICASCADE, "WM_MDICASCADE"  },
  {  WM_MDIICONARRANGE, "WM_MDIICONARRANGE"  },
  {  WM_MDIGETACTIVE, "WM_MDIGETACTIVE"  },
  {  WM_MDISETMENU, "WM_MDISETMENU"  },
  {  WM_DROPFILES, "WM_DROPFILES"  },
  {  WM_MDIREFRESHMENU, "WM_MDIREFRESHMENU"  },
  {  WM_CUT, "WM_CUT"  },
  {  WM_COPY, "WM_COPY"  },
  {  WM_PASTE, "WM_PASTE"  },
  {  WM_CLEAR, "WM_CLEAR"  },
  {  WM_UNDO, "WM_UNDO"  },
  {  WM_RENDERFORMAT, "WM_RENDERFORMAT"  },
  {  WM_RENDERALLFORMATS, "WM_RENDERALLFORMATS"  },
  {  WM_DESTROYCLIPBOARD, "WM_DESTROYCLIPBOARD"  },
  {  WM_DRAWCLIPBOARD, "WM_DRAWCLIPBOARD"  },
  {  WM_PAINTCLIPBOARD, "WM_PAINTCLIPBOARD"  },
  {  WM_VSCROLLCLIPBOARD, "WM_VSCROLLCLIPBOARD"  },
  {  WM_SIZECLIPBOARD, "WM_SIZECLIPBOARD"  },
  {  WM_ASKCBFORMATNAME, "WM_ASKCBFORMATNAME"  },
  {  WM_CHANGECBCHAIN, "WM_CHANGECBCHAIN"  },
  {  WM_HSCROLLCLIPBOARD, "WM_HSCROLLCLIPBOARD"  },
  {  WM_QUERYNEWPALETTE, "WM_QUERYNEWPALETTE"  },
  {  WM_PALETTEISCHANGING, "WM_PALETTEISCHANGING"  },
  {  WM_PALETTECHANGED, "WM_PALETTECHANGED"  },
  {  WM_HOTKEY, "WM_HOTKEY"  },
  {  WM_PENWINFIRST, "WM_PENWINFIRST"  },
  {  WM_PENWINLAST, "WM_PENWINLAST"  },
  {  WM_ASYNCIO, "ASYNCIO"  },
  {  0, 0  }};

void
strace::wm (int message, int word, int lon)
{
  if (active ())
    {
      int i;

      for (i = 0; ta[i].n; i++)
	{
	  if (ta[i].v == message)
	    {
	      prntf (_STRACE_WM, NULL, "wndproc %d %s %d %d", message, ta[i].n, word, lon);
	      return;
	    }
	}
      prntf (_STRACE_WM, NULL, "wndproc %d unknown  %d %d", message, word, lon);
    }
}
#endif /*NOSTRACE*/
