/* strace.cc: system/windows tracing

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <time.h>
#include <wingdi.h>
#include <winuser.h>
#include <ctype.h>
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"

#define PROTECT(x) x[sizeof(x)-1] = 0
#define CHECK(x) if (x[sizeof(x)-1] != 0) { small_printf("array bound exceeded %d\n", __LINE__); ExitProcess(1); }

class strace NO_COPY strace;

/* 'twould be nice to declare this in winsup.h but winsup.h doesn't require
   stdarg.h, so we declare it here instead. */

#ifndef NOSTRACE

int
strace::microseconds()
{
  static int first_microsec = 0;
  static long long hires_frequency = 0;
  static int hires_initted = 0;

  int microsec;

  if (!hires_initted)
    {
      hires_initted = 1;
      QueryPerformanceFrequency ((LARGE_INTEGER *) &hires_frequency);
      if (hires_frequency == 0)
	  hires_initted = 2;
    }
  if (hires_initted == 2)
    {
      int count = GetTickCount ();
      microsec = count * 1000;
    }
  else
    {
      long long thiscount;
      QueryPerformanceCounter ((LARGE_INTEGER *) &thiscount);
      thiscount = (long long) (((double) thiscount/(double) hires_frequency)
			       * 1000000.0);
      microsec = thiscount;
    }
  if (first_microsec == 0)
    first_microsec = microsec;
  return microsec - first_microsec;
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
    else if (isspace((int)pe[-1]))
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

extern "C" char *__progname;

/* sprintf analog for use by output routines. */
int
strace::vsprntf (char *buf, const char *func, const char *infmt, va_list ap)
{
  int count;
  char fmt[80];
  static int nonewline = FALSE;
  DWORD err = GetLastError ();
  const char *tn = threadname (0);
  char *pn = __progname ?: myself->progname;

  int microsec = microseconds ();
  lmicrosec = microsec;

  __small_sprintf (fmt, "%7d [%s] %s ", microsec, tn, "%s %d%s");

  SetLastError (err);

  if (nonewline)
    count = 0;
  else
    {
      char *p, progname[MAX_PATH + 1];
      if ((p = strrchr (pn, '\\')) != NULL)
	p++;
      else if ((p = strrchr (pn, '/')) != NULL)
	p++;
      else
	p = pn;
      strcpy (progname, p);
      if ((p = strrchr (progname, '.')) != NULL)
	*p = '\000';
      p = progname;
      count = __small_sprintf (buf, fmt, p && *p ? p : "?",
			       myself->pid, hExeced ? "!" : "");
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
	   nonewline = TRUE;
	  goto done;
	default:
	  goto addnl;
      }

addnl:
  *p++ = '\n';
  *p = '\0';
  nonewline = FALSE;

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
}

/* Printf function used when tracing system calls.
   Warning: DO NOT SET ERRNO HERE! */

void
strace::prntf (unsigned category, const char *func, const char *fmt, ...)
{
  DWORD err = GetLastError ();
  int count;
  char buf[10000];
  va_list ap;

  PROTECT(buf);
  SetLastError (err);

  va_start (ap, fmt);
  count = this->vsprntf (buf, func, fmt, ap);
  CHECK(buf);
  if (category & _STRACE_SYSTEM)
    {
      DWORD done;
      WriteFile (GetStdHandle (STD_ERROR_HANDLE), buf, count, &done, 0);
      FlushFileBuffers (GetStdHandle (STD_ERROR_HANDLE));
    }

#ifndef NOSTRACE
  if (active)
    this->write (category, buf, count);
#endif
  SetLastError (err);
}

static const struct tab
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
  if (active)
    {
      int i;

      for (i = 0; ta[i].n; i++)
	{
	  if (ta[i].v == message)
	    {
	      this->prntf (_STRACE_WM, NULL, "wndproc %d %s %d %d", message, ta[i].n, word, lon);
	      return;
	    }
	}
      this->prntf (_STRACE_WM, NULL, "wndproc %d unknown  %d %d", message, word, lon);
    }
}
#endif /*NOSTRACE*/
