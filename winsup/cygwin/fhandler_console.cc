/* fhandler_console.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <wingdi.h>
#include <winuser.h>
#include <wincon.h>
#include <winnls.h>	// MultiByteToWideChar () and friends
#include <ctype.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "shared_info.h"
#include "security.h"

#define CONVERT_LIMIT 4096

/* The codepages are resolved here instead of using CP_ACP and
   CP_OEMCP, so that they can later be compared for equality. */
inline UINT
cp_get_internal ()
{
  return current_codepage == ansi_cp ? GetACP() : GetOEMCP();
}

static BOOL
cp_convert (UINT destcp, char * dest, UINT srccp, const char * src, DWORD size)
{
  if (!size)
    /* no action */;
  else if (destcp == srccp)
    {
      if (dest != src)
	memcpy (dest, src, size);
    }
  else
    {
      WCHAR wbuffer[CONVERT_LIMIT]; /* same size as the maximum input, s.b. */
      if (!MultiByteToWideChar (srccp, 0, src, size, wbuffer, sizeof (wbuffer)))
	return FALSE;
      if (!WideCharToMultiByte (destcp, 0, wbuffer, size, dest, size,
				NULL, NULL))
	return FALSE;
    }
  return TRUE;
}

/* The results of GetConsoleCP() and GetConsoleOutputCP() cannot be
   cached, because a program or the user can change these values at
   any time. */
inline BOOL
con_to_str (char *d, const char *s, DWORD sz)
{
  return cp_convert (cp_get_internal (), d, GetConsoleCP (), s, sz);
}

inline BOOL
str_to_con (char *d, const char *s, DWORD sz)
{
  return cp_convert (GetConsoleOutputCP (), d, cp_get_internal (), s, sz);
}

/*
 * Scroll the screen context.
 * x1, y1 - ul corner
 * x2, y2 - dr corner
 * xn, yn - new ul corner
 * Negative values represents current screen dimensions
 */

#define srTop (info.winTop + scroll_region.Top)
#define srBottom ((scroll_region.Bottom < 0) ? info.winBottom : info.winTop + scroll_region.Bottom)

#define use_tty ISSTATE (myself, PID_USETTY)

const char * get_nonascii_key (INPUT_RECORD&, char *);

static tty_min NO_COPY *shared_console_info = NULL;

/* Allocate and initialize the shared record for the current console.
   Returns a pointer to shared_console_info. */
static __inline tty_min *
get_tty_stuff (int force = 0)
{
  if (shared_console_info && !force)
    return shared_console_info;

  shared_console_info = (tty_min *) open_shared (NULL, cygheap->console_h,
						 sizeof (*shared_console_info),
						 NULL);
  ProtectHandle (cygheap->console_h);
  shared_console_info->setntty (TTY_CONSOLE);
  shared_console_info->setsid (myself->sid);
  return shared_console_info;
}

/* Return the tty structure associated with a given tty number.  If the
   tty number is < 0, just return a dummy record. */
tty_min *
tty_list::get_tty (int n)
{
  static tty_min nada;
  if (n == TTY_CONSOLE)
    return get_tty_stuff ();
  else if (n >= 0)
    return &cygwin_shared->tty.ttys[n];
  else
    return &nada;
}


/* Determine if a console is associated with this process prior to a spawn.
   If it is, then we'll return 1.  If the console has been initialized, then
   set it into a more friendly state for non-cygwin apps. */
int __stdcall
set_console_state_for_spawn ()
{
  HANDLE h = CreateFileA ("CONIN$", GENERIC_READ, FILE_SHARE_WRITE,
			  &sec_none_nih, OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL, NULL);

  if (h == INVALID_HANDLE_VALUE || h == NULL)
    return 0;

  if (shared_console_info != NULL)
    {
#     define tc shared_console_info	/* ACK.  Temporarily define for use in TTYSETF macro */
      SetConsoleMode (h, ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
      TTYSETF (RSTCONS);
#if 0
      char ch;
      DWORD n;
      /* NOTE -- This ReadFile is apparently necessary for correct functioning on
	 Windows NT 4.0.  Without this, the next ReadFile returns garbage.  */
      (void) ReadFile (h, &ch, 0, &n, NULL);
#endif
#     undef tc
    }

  CloseHandle (h);
  return 1;
}

BOOL
fhandler_console::set_raw_win32_keyboard_mode (BOOL new_mode)
{
  BOOL old_mode = raw_win32_keyboard_mode;
  raw_win32_keyboard_mode = new_mode;
  syscall_printf ("raw keyboard mode %sabled", raw_win32_keyboard_mode ? "en" : "dis");
  return old_mode;
};

void
fhandler_console::set_cursor_maybe ()
{
  CONSOLE_SCREEN_BUFFER_INFO now;

  if (!GetConsoleScreenBufferInfo (get_output_handle (), &now))
    return;

  if (dwLastCursorPosition.X != now.dwCursorPosition.X ||
      dwLastCursorPosition.Y != now.dwCursorPosition.Y)
    {
      SetConsoleCursorPosition (get_output_handle (), now.dwCursorPosition);
      dwLastCursorPosition = now.dwCursorPosition;
    }
}

int
fhandler_console::read (void *pv, size_t buflen)
{
  if (!buflen)
    return 0;

  HANDLE h = get_io_handle ();

#define buf ((char *) pv)

  int ch;
  set_input_state ();

  int copied_chars = get_readahead_into_buffer (buf, buflen);

  if (copied_chars)
    return copied_chars;

  HANDLE w4[2];
  DWORD nwait;
  char tmp[60];

  w4[0] = h;
  if (iscygthread ())
    nwait = 1;
  else
    {
      w4[1] = signal_arrived;
      nwait = 2;
    }

  for (;;)
    {
      int bgres;
      if ((bgres = bg_check (SIGTTIN)) <= bg_eof)
	return bgres;

      set_cursor_maybe ();	/* to make cursor appear on the screen immediately */
      switch (WaitForMultipleObjects (nwait, w4, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
	  break;
	case WAIT_OBJECT_0 + 1:
	  goto sig_exit;
	default:
	  __seterrno ();
	  return -1;
	}

      DWORD nread;
      INPUT_RECORD input_rec;
      const char *toadd = NULL;

      if (!ReadConsoleInput (h, &input_rec, 1, &nread))
	{
	  __seterrno ();
	  syscall_printf ("ReadConsoleInput failed, %E");
	  return -1;		/* seems to be failure */
	}

      /* check the event that occurred */
      switch (input_rec.EventType)
	{
	case KEY_EVENT:
#define virtual_key_code (input_rec.Event.KeyEvent.wVirtualKeyCode)
#define control_key_state (input_rec.Event.KeyEvent.dwControlKeyState)

#ifdef DEBUGGING
	  /* allow manual switching to/from raw mode via ctrl-alt-scrolllock */
	  if (input_rec.Event.KeyEvent.bKeyDown &&
	      virtual_key_code == VK_SCROLL &&
	      control_key_state & (LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED) == LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED
	    )
	    {
	      set_raw_win32_keyboard_mode (!raw_win32_keyboard_mode);
	      continue;
	    }
#endif

	  if (raw_win32_keyboard_mode)
	    {
	      __small_sprintf (tmp, "\033{%u;%u;%u;%u;%u;%luK",
				    input_rec.Event.KeyEvent.bKeyDown,
				    input_rec.Event.KeyEvent.wRepeatCount,
				    input_rec.Event.KeyEvent.wVirtualKeyCode,
				    input_rec.Event.KeyEvent.wVirtualScanCode,
				    input_rec.Event.KeyEvent.uChar.UnicodeChar,
				    input_rec.Event.KeyEvent.dwControlKeyState);
	      toadd = tmp;
	      nread = strlen (toadd);
	      break;
	    }

	  if (!input_rec.Event.KeyEvent.bKeyDown)
	    continue;

#define ich (input_rec.Event.KeyEvent.uChar.AsciiChar)
#define wch (input_rec.Event.KeyEvent.uChar.UnicodeChar)

	  if (wch == 0 ||
	      /* arrow/function keys */
	      (input_rec.Event.KeyEvent.dwControlKeyState & ENHANCED_KEY))
	    {
	      toadd = get_nonascii_key (input_rec, tmp);
	      if (!toadd)
		continue;
	      nread = strlen (toadd);
	    }
	  else
	    {
	      tmp[1] = ich;
	      /* Need this check since US code page seems to have a bug when
		 converting a CTRL-U. */
	      if ((unsigned char)ich > 0x7f)
		con_to_str (tmp + 1, tmp + 1, 1);
	      /* Determine if the keystroke is modified by META. */
	      if (!(input_rec.Event.KeyEvent.dwControlKeyState & meta_mask))
		toadd = tmp + 1;
	      else
		{
		  tmp[0] = '\033';
		  tmp[1] = cyg_tolower (tmp[1]);
		  toadd = tmp;
		  nread++;
		}
	    }
#undef ich
#undef wch
	  break;

	case MOUSE_EVENT:
	  if (use_mouse)
	    {
	      MOUSE_EVENT_RECORD & mouse_event = input_rec.Event.MouseEvent;

	      /* Treat the double-click event like a regular button press */
	      if (mouse_event.dwEventFlags == DOUBLE_CLICK)
		{
		  syscall_printf ("mouse: double-click -> click");
		  mouse_event.dwEventFlags = 0;
		}

	      /* Did something other than a click occur? */
	      if (mouse_event.dwEventFlags)
		continue;

	      /* If the mouse event occurred out of the area we can handle,
		 ignore it. */
	      int x = mouse_event.dwMousePosition.X;
	      int y = mouse_event.dwMousePosition.Y;
	      if ((x + ' ' + 1 > 0xFF) || (y + ' ' + 1 > 0xFF))
		{
		  syscall_printf ("mouse: position out of range");
		  continue;
		}

	      /* Ignore unimportant mouse buttons */
	      mouse_event.dwButtonState &= 0x7;

	      /* This code assumes Windows never reports multiple button
		 events at the same time. */
	      int b = 0;
	      char sz[32];
	      if (mouse_event.dwButtonState == dwLastButtonState)
		{
		  syscall_printf ("mouse: button state unchanged");
		  continue;
		}
	      else if (mouse_event.dwButtonState < dwLastButtonState)
		{
		  b = 3;
		  strcpy (sz, "btn up");
		}
	      else if ((mouse_event.dwButtonState & 1) != (dwLastButtonState & 1))
		{
		  b = 0;
		  strcpy (sz, "btn1 down");
		}
	      else if ((mouse_event.dwButtonState & 2) != (dwLastButtonState & 2))
		{
		  b = 1;
		  strcpy (sz, "btn2 down");
		}
	      else if ((mouse_event.dwButtonState & 4) != (dwLastButtonState & 4))
		{
		  b = 2;
		  strcpy (sz, "btn3 down");
		}

	      /* Remember the current button state */
	      dwLastButtonState = mouse_event.dwButtonState;

	      /* If a button was pressed, remember the modifiers */
	      if (b != 3)
		{
		  nModifiers = 0;
		  if (mouse_event.dwControlKeyState & SHIFT_PRESSED)
		    nModifiers |= 0x4;
		  if (mouse_event.dwControlKeyState & (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED))
		    nModifiers |= 0x8;
		  if (mouse_event.dwControlKeyState & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED))
		    nModifiers |= 0x10;
		}

	      b |= nModifiers;

	      /* We can now create the code. */
	      sprintf (tmp, "\033[M%c%c%c", b + ' ', x + ' ' + 1, y + ' ' + 1);
	      syscall_printf ("mouse: %s at (%d,%d)", sz, x, y);

	      toadd = tmp;
	      nread = 6;
	    }
	  break;

	case WINDOW_BUFFER_SIZE_EVENT:
	  kill_pgrp (tc->getpgid (), SIGWINCH);
	  continue;

	default:
	  continue;
	}

      if (toadd)
	{
	  int res = line_edit (toadd, nread);
	  if (res < 0)
	    goto sig_exit;
	  else if (res)
	    break;
	}
#undef ich
    }

  while (buflen)
    if ((ch = get_readahead ()) < 0)
      break;
    else
      {
	buf[copied_chars++] = (unsigned char)(ch & 0xff);
	buflen--;
      }
#undef buf

  return copied_chars;

 sig_exit:
  set_sig_errno (EINTR);
  return -1;
}

void
fhandler_console::set_input_state ()
{
  if (TTYISSETF (RSTCONS))
    input_tcsetattr (0, &tc->ti);
}

BOOL
fhandler_console::fillin_info (void)
{
  BOOL ret;
  CONSOLE_SCREEN_BUFFER_INFO linfo;

  if ((ret = GetConsoleScreenBufferInfo (get_output_handle (), &linfo)))
    {
      info.winTop = linfo.srWindow.Top;
      info.winBottom = linfo.srWindow.Bottom;
      info.dwWinSize.Y = 1 + linfo.srWindow.Bottom - linfo.srWindow.Top;
      info.dwWinSize.X = 1 + linfo.srWindow.Right - linfo.srWindow.Left;
      info.dwBufferSize = linfo.dwSize;
      info.dwCursorPosition = linfo.dwCursorPosition;
      info.wAttributes = linfo.wAttributes;
    }
  else
    {
      memset (&info, 0, sizeof info);
      info.dwWinSize.Y = 25;
      info.dwWinSize.X = 80;
      info.winBottom = 24;
    }

  return ret;
}

void
fhandler_console::scroll_screen (int x1, int y1, int x2, int y2, int xn, int yn)
{
  SMALL_RECT sr1, sr2;
  CHAR_INFO fill;
  COORD dest;

  (void)fillin_info ();
  sr1.Left = x1 >= 0 ? x1 : info.dwWinSize.X - 1;
  if (y1 == 0)
    sr1.Top = info.winTop;
  else
    sr1.Top = y1 > 0 ? y1 : info.winBottom;
  sr1.Right = x2 >= 0 ? x2 : info.dwWinSize.X - 1;
  if (y2 == 0)
    sr1.Bottom = info.winTop;
  else
    sr1.Bottom = y2 > 0 ? y2 : info.winBottom;
  sr2.Top = srTop;
  sr2.Left = 0;
  sr2.Bottom = srBottom;
  sr2.Right = info.dwWinSize.X - 1;
  if (sr1.Bottom > sr2.Bottom && sr1.Top <= sr2.Bottom)
    sr1.Bottom = sr2.Bottom;
  dest.X = xn >= 0 ? xn : info.dwWinSize.X - 1;
  if (yn == 0)
    dest.Y = info.winTop;
  else
    dest.Y = yn > 0 ? yn : info.winBottom;
  fill.Char.AsciiChar = ' ';
  fill.Attributes = current_win32_attr;
  ScrollConsoleScreenBuffer (get_output_handle (), &sr1, &sr2, dest, &fill);

  /* ScrollConsoleScreenBuffer on Windows 95 is buggy - when scroll distance
   * is more than half of screen, filling doesn't work as expected */

  if (sr1.Top != sr1.Bottom)
    if (dest.Y <= sr1.Top)	/* forward scroll */
      clear_screen (0, 1 + dest.Y + sr1.Bottom - sr1.Top, sr2.Right, sr2.Bottom);
    else			/* reverse scroll */
      clear_screen (0, sr1.Top, sr2.Right, dest.Y - 1);
}

int
fhandler_console::open (const char *, int flags, mode_t)
{
  HANDLE h;

  tcinit (get_tty_stuff ());

  set_io_handle (INVALID_HANDLE_VALUE);
  set_output_handle (INVALID_HANDLE_VALUE);

  set_flags (flags);

  /* Open the input handle as handle_ */
  h = CreateFileA ("CONIN$", GENERIC_READ|GENERIC_WRITE,
		   FILE_SHARE_READ | FILE_SHARE_WRITE, &sec_none,
		   OPEN_EXISTING, 0, 0);

  if (h == INVALID_HANDLE_VALUE)
    {
      __seterrno ();
      return 0;
    }
  set_io_handle (h);
  set_r_no_interrupt (1);	// Handled explicitly in read code

  h = CreateFileA ("CONOUT$", GENERIC_READ|GENERIC_WRITE,
		   FILE_SHARE_READ | FILE_SHARE_WRITE, &sec_none,
		   OPEN_EXISTING, 0, 0);

  if (h == INVALID_HANDLE_VALUE)
    {
      __seterrno ();
      return 0;
    }
  set_output_handle (h);

  if (fillin_info ())
    default_color = info.wAttributes;

  set_default_attr ();

  DWORD cflags;
  if (GetConsoleMode (get_io_handle (), &cflags))
    {
      cflags |= ENABLE_PROCESSED_INPUT;
      SetConsoleMode (get_io_handle (), ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | cflags);
    }

  TTYCLEARF (RSTCONS);
  set_ctty (TTY_CONSOLE, flags);
  debug_printf ("opened conin$ %p, conout$ %p",
		get_io_handle (), get_output_handle ());

  return 1;
}

int
fhandler_console::close (void)
{
  CloseHandle (get_io_handle ());
  CloseHandle (get_output_handle ());
  set_io_handle (INVALID_HANDLE_VALUE);
  set_output_handle (INVALID_HANDLE_VALUE);
  return 0;
}

/*
 * Special console dup to duplicate input and output
 * handles.
 */

int
fhandler_console::dup (fhandler_base *child)
{
  fhandler_console *fhc = (fhandler_console *) child;

  if (!fhc->open (get_name (), get_flags (), 0))
    system_printf ("error opening console, %E");

  fhc->default_color = default_color;
  fhc->underline_color = underline_color;
  fhc->dim_color = dim_color;
  fhc->state_ = state_;
  fhc->nargs_ = nargs_;
  for (int i = 0; i < MAXARGS; i++)
    fhc->args_[i] = args_[i];
  fhc->rarg = rarg;
  fhc->saw_question_mark = saw_question_mark;

  strncpy (fhc->my_title_buf, my_title_buf, TITLESIZE + 1) ;

  fhc->current_win32_attr = current_win32_attr;
  fhc->intensity = intensity;
  fhc->underline = underline;
  fhc->blink = blink;
  fhc->reverse = reverse;
  fhc->fg = fg;
  fhc->bg = bg;

  fhc->savex = savex;
  fhc->savey = savey;

  fhc->savebufsiz = savebufsiz;
  if (savebuf)
    {
      fhc->savebuf = (PCHAR_INFO) malloc (sizeof (CHAR_INFO) *
					  savebufsiz.X * savebufsiz.Y);
      memcpy (fhc->savebuf, savebuf, sizeof (CHAR_INFO) *
				     savebufsiz.X * savebufsiz.Y);
    }

  fhc->scroll_region = scroll_region;
  fhc->dwLastCursorPosition = dwLastCursorPosition;
  fhc->dwLastButtonState = dwLastButtonState;
  fhc->nModifiers = nModifiers;

  fhc->insert_mode = insert_mode;
  fhc->use_mouse = use_mouse;
  fhc->raw_win32_keyboard_mode = raw_win32_keyboard_mode;

  return 0;
}

int
fhandler_console::ioctl (unsigned int cmd, void *buf)
{
  switch (cmd)
    {
      case TIOCGWINSZ:
	int st;

	st = fillin_info ();
	if (st)
	  {
	    /* *not* the buffer size, the actual screen size... */
	    /* based on Left Top Right Bottom of srWindow */
	    ((struct winsize *) buf)->ws_row = info.dwWinSize.Y;
	    ((struct winsize *) buf)->ws_col = info.dwWinSize.X;
	    syscall_printf ("WINSZ: (row=%d,col=%d)",
			   ((struct winsize *) buf)->ws_row,
			   ((struct winsize *) buf)->ws_col);
	    return 0;
	  }
	else
	  {
	    syscall_printf ("WINSZ failed");
	    __seterrno ();
	    return -1;
	  }
	return 0;
      case TIOCSWINSZ:
	(void) bg_check (SIGTTOU);
	return 0;
    }

  return fhandler_base::ioctl (cmd, buf);
}

int
fhandler_console::tcflush (int queue)
{
  int res = 0;
  if (queue == TCIFLUSH
      || queue == TCIOFLUSH)
    {
      if (!FlushConsoleInputBuffer (get_io_handle ()))
	{
	  __seterrno ();
	  res = -1;
	}
    }
  return res;
}

int
fhandler_console::output_tcsetattr (int, struct termios const *t)
{
  /* Ignore the optional_actions stuff, since all output is emitted
     instantly */

  /* Enable/disable LF -> CRLF conversions */
  set_w_binary ((t->c_oflag & ONLCR) ? 0 : 1);

  /* All the output bits we can ignore */

  DWORD flags = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;

  int res = SetConsoleMode (get_output_handle (), flags) ? 0 : -1;
  syscall_printf ("%d = tcsetattr (,%x) (ENABLE FLAGS %x) (lflag %x oflag %x)",
		  res, t, flags, t->c_lflag, t->c_oflag);
  return res;
}

int
fhandler_console::input_tcsetattr (int, struct termios const *t)
{
  /* Ignore the optional_actions stuff, since all output is emitted
     instantly */

  DWORD oflags;

  if (!GetConsoleMode (get_io_handle (), &oflags))
    oflags = 0;
  DWORD flags = 0;

  /* Enable/disable LF -> CRLF conversions */
  set_r_binary ((t->c_iflag & INLCR) ? 0 : 1);

  /* There's some disparity between what we need and what's
     available.  We've got ECHO and ICANON, they've
     got ENABLE_ECHO_INPUT and ENABLE_LINE_INPUT. */

  tc->ti = *t;

  if (t->c_lflag & ECHO)
    {
      flags |= ENABLE_ECHO_INPUT;
    }
  if (t->c_lflag & ICANON)
    {
      flags |= ENABLE_LINE_INPUT;
    }

  if (flags & ENABLE_ECHO_INPUT
      && !(flags & ENABLE_LINE_INPUT))
    {
      /* This is illegal, so turn off the echo here, and fake it
	 when we read the characters */

      flags &= ~ENABLE_ECHO_INPUT;
    }

  if (t->c_lflag & ISIG)
    {
      flags |= ENABLE_PROCESSED_INPUT;
    }
  /* What about ENABLE_WINDOW_INPUT
     and ENABLE_MOUSE_INPUT   ? */

  if (use_tty)
    {
      flags = 0; // ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
      tc->ti.c_iflag = 0;
      tc->ti.c_lflag = 0;
    }

  flags |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;

  int res;
  if (flags == oflags)
    res = 0;
  else
    {
      res = SetConsoleMode (get_io_handle (), flags) ? 0 : -1;
      if (res < 0)
	__seterrno ();
      syscall_printf ("%d = tcsetattr (,%x) enable flags %p, c_lflag %p iflag %p",
		      res, t, flags, t->c_lflag, t->c_iflag);
    }

  TTYCLEARF (RSTCONS);
  return res;
}

int
fhandler_console::tcsetattr (int a, struct termios const *t)
{
  int res = output_tcsetattr (a, t);
  if (res != 0)
    return res;
  return input_tcsetattr (a, t);
}

int
fhandler_console::tcgetattr (struct termios *t)
{
  int res;
  *t = tc->ti;

  t->c_cflag |= CS8;

#if 0
  if (!get_r_binary ())
    t->c_iflag |= IGNCR;
  if (!get_w_binary ())
    t->c_oflag |= ONLCR;
#endif

  DWORD flags;

  if (!GetConsoleMode (get_io_handle (), &flags))
    {
      __seterrno ();
      res = -1;
    }
  else
    {
      if (flags & ENABLE_ECHO_INPUT)
	t->c_lflag |= ECHO;

      if (flags & ENABLE_LINE_INPUT)
	t->c_lflag |= ICANON;

      if (flags & ENABLE_PROCESSED_INPUT)
	t->c_lflag |= ISIG;

      /* What about ENABLE_WINDOW_INPUT
	 and ENABLE_MOUSE_INPUT   ? */

      /* All the output bits we can ignore */
      res = 0;
    }
  syscall_printf ("%d = tcgetattr (%p) enable flags %p, t->lflag %p, t->iflag %p",
		 res, t, flags, t->c_lflag, t->c_iflag);
  return res;
}

/*
 * Constructor.
 */

fhandler_console::fhandler_console (const char *name) :
  fhandler_termios (FH_CONSOLE, name, -1)
{
  set_cb (sizeof *this);
  default_color = dim_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  underline_color = FOREGROUND_GREEN | FOREGROUND_BLUE;
  state_ = normal;
  nargs_ = 0;
  for (int i = 0; i < MAXARGS; i++) args_ [i] = 0;
  savex = savey = 0;
  savebufsiz.X = savebufsiz.Y = 0;
  savebuf = NULL;
  scroll_region.Top = 0;
  scroll_region.Bottom = -1;
  dwLastCursorPosition.X = -1;
  dwLastCursorPosition.Y = -1;
  dwLastButtonState = 0;
  nModifiers = 0;
  insert_mode = use_mouse = raw_win32_keyboard_mode = FALSE;
  /* Set the mask that determines if an input keystroke is modified by
     META.  We set this based on the keyboard layout language loaded
     for the current thread.  The left <ALT> key always generates
     META, but the right <ALT> key only generates META if we are using
     an English keyboard because many "international" keyboards
     replace common shell symbols ('[', '{', etc.) with accented
     language-specific characters (umlaut, accent grave, etc.).  On
     these keyboards right <ALT> (called AltGr) is used to produce the
     shell symbols and should not be interpreted as META. */
  meta_mask = LEFT_ALT_PRESSED;
  if (PRIMARYLANGID (LOWORD (GetKeyboardLayout (0))) == LANG_ENGLISH)
    meta_mask |= RIGHT_ALT_PRESSED;

}

#define FOREGROUND_ATTR_MASK (FOREGROUND_RED | FOREGROUND_GREEN | \
			      FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define BACKGROUND_ATTR_MASK (BACKGROUND_RED | BACKGROUND_GREEN | \
			      BACKGROUND_BLUE | BACKGROUND_INTENSITY)
void
fhandler_console::set_default_attr ()
{
  blink = underline = reverse = FALSE;
  intensity = INTENSITY_NORMAL;
  fg = default_color & FOREGROUND_ATTR_MASK;
  bg = default_color & BACKGROUND_ATTR_MASK;
  current_win32_attr = get_win32_attr ();
  SetConsoleTextAttribute (get_output_handle (), current_win32_attr);
}

WORD
fhandler_console::get_win32_attr ()
{
  WORD win_fg = fg;
  WORD win_bg = bg;
  if (reverse)
    {
      WORD save_fg = win_fg;
      win_fg = (win_bg & BACKGROUND_RED   ? FOREGROUND_RED   : 0) |
	       (win_bg & BACKGROUND_GREEN ? FOREGROUND_GREEN : 0) |
	       (win_bg & BACKGROUND_BLUE  ? FOREGROUND_BLUE  : 0) |
	       (win_fg & FOREGROUND_INTENSITY);
      win_bg = (save_fg & FOREGROUND_RED   ? BACKGROUND_RED   : 0) |
	       (save_fg & FOREGROUND_GREEN ? BACKGROUND_GREEN : 0) |
	       (save_fg & FOREGROUND_BLUE  ? BACKGROUND_BLUE  : 0) |
	       (win_bg & BACKGROUND_INTENSITY);
    }
  if (underline) win_fg = underline_color;
  /* emulate blink with bright background */
  if (blink) win_bg |= BACKGROUND_INTENSITY;
  if (intensity == INTENSITY_INVISIBLE)
    win_fg = win_bg;
  else if (intensity == INTENSITY_BOLD)
    win_fg |= FOREGROUND_INTENSITY;
  return (win_fg | win_bg);
}

/*
 * Clear the screen context from x1/y1 to x2/y2 cell.
 * Negative values represents current screen dimensions
 */
void
fhandler_console::clear_screen (int x1, int y1, int x2, int y2)
{
  COORD tlc;
  DWORD done;
  int num;

  (void)fillin_info ();

  if (x1 < 0)
    x1 = info.dwWinSize.X - 1;
  if (y1 < 0)
    y1 = info.winBottom;
  if (x2 < 0)
    x2 = info.dwWinSize.X - 1;
  if (y2 < 0)
    y2 = info.winBottom;

  num = abs (y1 - y2) * info.dwBufferSize.X + abs (x1 - x2) + 1;

  if ((y2 * info.dwBufferSize.X + x2) > (y1 * info.dwBufferSize.X + x1))
    {
      tlc.X = x1;
      tlc.Y = y1;
    }
  else
    {
      tlc.X = x2;
      tlc.Y = y2;
    }
  FillConsoleOutputCharacterA (get_output_handle (), ' ',
			       num,
			       tlc,
			       &done);
  FillConsoleOutputAttribute (get_output_handle (),
			       current_win32_attr,
			       num,
			       tlc,
			       &done);
}

void
fhandler_console::cursor_set (BOOL rel_to_top, int x, int y)
{
  COORD pos;

  (void)fillin_info ();
  if (y > info.winBottom)
    y = info.winBottom;
  else if (y < 0)
    y = 0;
  else if (rel_to_top)
    y += info.winTop;

  if (x > info.dwWinSize.X)
    x = info.dwWinSize.X - 1;
  else if (x < 0)
    x = 0;

  pos.X = x;
  pos.Y = y;
  SetConsoleCursorPosition (get_output_handle (), pos);
}

void
fhandler_console::cursor_rel (int x, int y)
{
  fillin_info ();
  x += info.dwCursorPosition.X;
  y += info.dwCursorPosition.Y;
  cursor_set (FALSE, x, y);
}

void
fhandler_console::cursor_get (int *x, int *y)
{
  fillin_info ();
  *y = info.dwCursorPosition.Y;
  *x = info.dwCursorPosition.X;
}

#define BAK 1
#define ESC 2
#define NOR 0
#define IGN 4
#if 0
#define ERR 5
#else
#define ERR NOR
#endif
#define DWN 6
#define BEL 7
#define TAB 8 /* We should't let the console deal with these */
#define CR 13
#define LF 10

static const char base_chars[256] =
{
/*00 01 02 03 04 05 06 07 */ IGN, ERR, ERR, NOR, NOR, NOR, NOR, BEL,
/*08 09 0A 0B 0C 0D 0E 0F */ BAK, TAB, DWN, ERR, ERR, CR,  ERR, IGN,
/*10 11 12 13 14 15 16 17 */ NOR, NOR, ERR, ERR, ERR, ERR, ERR, ERR,
/*18 19 1A 1B 1C 1D 1E 1F */ NOR, NOR, ERR, ESC, ERR, ERR, ERR, ERR,
/*   !  "  #  $  %  &  '  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*()  *  +  ,  -  .  /  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*0  1  2  3  4  5  6  7  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*8  9  :  ;  <  =  >  ?  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*@  A  B  C  D  E  F  G  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*H  I  J  K  L  M  N  O  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*P  Q  R  S  T  U  V  W  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*X  Y  Z  [  \  ]  ^  _  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*`  a  b  c  d  e  f  g  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*h  i  j  k  l  m  n  o  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*p  q  r  s  t  u  v  w  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*x  y  z  {  |  }  ~  7F */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*80 81 82 83 84 85 86 87 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*88 89 8A 8B 8C 8D 8E 8F */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*90 91 92 93 94 95 96 97 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*98 99 9A 9B 9C 9D 9E 9F */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*A0 A1 A2 A3 A4 A5 A6 A7 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*A8 A9 AA AB AC AD AE AF */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*B0 B1 B2 B3 B4 B5 B6 B7 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*B8 B9 BA BB BC BD BE BF */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*C0 C1 C2 C3 C4 C5 C6 C7 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*C8 C9 CA CB CC CD CE CF */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*D0 D1 D2 D3 D4 D5 D6 D7 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*D8 D9 DA DB DC DD DE DF */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*E0 E1 E2 E3 E4 E5 E6 E7 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*E8 E9 EA EB EC ED EE EF */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*F0 F1 F2 F3 F4 F5 F6 F7 */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*F8 F9 FA FB FC FD FE FF */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR };

void
fhandler_console::char_command (char c)
{
  int x, y;
  char buf[40];

  switch (c)
    {
    case 'm':   /* Set Graphics Rendition */
       int i;

       for (i = 0; i <= nargs_; i++)
	 switch (args_[i])
	   {
	     case 0:    /* normal color */
	       set_default_attr ();
	       break;
	     case 1:    /* bold */
	       intensity = INTENSITY_BOLD;
	       break;
	     case 4:
	       underline = 1;
	       break;
	     case 5:    /* blink mode */
	       blink = TRUE;
	       break;
	     case 7:    /* reverse */
	       reverse = TRUE;
	       break;
	     case 8:    /* invisible */
	       intensity = INTENSITY_INVISIBLE;
	       break;
	     case 9:    /* dim */
	       intensity = INTENSITY_DIM;
	       break;
	     case 24:
	       underline = FALSE;
	       break;
	     case 27:
	       reverse = FALSE;
	       break;
	     case 30:		/* BLACK foreground */
	       fg = 0;
	       break;
	     case 31:		/* RED foreground */
	       fg = FOREGROUND_RED;
	       break;
	     case 32:		/* GREEN foreground */
	       fg = FOREGROUND_GREEN;
	       break;
	     case 33:		/* YELLOW foreground */
	       fg = FOREGROUND_RED | FOREGROUND_GREEN;
	       break;
	     case 34:		/* BLUE foreground */
	       fg = FOREGROUND_BLUE;
	       break;
	     case 35:		/* MAGENTA foreground */
	       fg = FOREGROUND_RED | FOREGROUND_BLUE;
	       break;
	     case 36:		/* CYAN foreground */
	       fg = FOREGROUND_BLUE | FOREGROUND_GREEN;
	       break;
	     case 37:		/* WHITE foreg */
	       fg = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
	       break;
	     case 39:
	       fg = default_color & FOREGROUND_ATTR_MASK;
	       break;
	     case 40:		/* BLACK background */
	       bg = 0;
	       break;
	     case 41:		/* RED background */
	       bg = BACKGROUND_RED;
	       break;
	     case 42:		/* GREEN background */
	       bg = BACKGROUND_GREEN;
	       break;
	     case 43:		/* YELLOW background */
	       bg = BACKGROUND_RED | BACKGROUND_GREEN;
	       break;
	     case 44:		/* BLUE background */
	       bg = BACKGROUND_BLUE;
	       break;
	     case 45:		/* MAGENTA background */
	       bg = BACKGROUND_RED | BACKGROUND_BLUE;
	       break;
	     case 46:		/* CYAN background */
	       bg = BACKGROUND_BLUE | BACKGROUND_GREEN;
	       break;
	     case 47:    /* WHITE background */
	       bg = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
	       break;
	     case 49:
	       bg = default_color & BACKGROUND_ATTR_MASK;
	       break;
	   }
	 current_win32_attr = get_win32_attr ();
	 SetConsoleTextAttribute (get_output_handle (), current_win32_attr);
      break;
    case 'h':
    case 'l':
      if (!saw_question_mark)
	{
	  switch (args_[0])
	    {
	    case 4:    /* Insert mode */
	      insert_mode = (c == 'h') ? TRUE : FALSE;
	      syscall_printf ("insert mode %sabled", insert_mode ? "en" : "dis");
	      break;
	    }
	  break;
	}
      switch (args_[0])
	{
	case 47:   /* Save/Restore screen */
	  if (c == 'h') /* save */
	    {
	      CONSOLE_SCREEN_BUFFER_INFO now;
	      COORD cob = { 0, 0 };

	      if (!GetConsoleScreenBufferInfo (get_output_handle (), &now))
		break;

	      savebufsiz.X = now.srWindow.Right - now.srWindow.Left;
	      savebufsiz.Y = now.srWindow.Bottom - now.srWindow.Top;

	      if (savebuf)
		free (savebuf);
	      savebuf = (PCHAR_INFO) malloc (sizeof (CHAR_INFO) *
					     savebufsiz.X * savebufsiz.Y);

	      ReadConsoleOutputA (get_output_handle (), savebuf,
				  savebufsiz, cob, &now.srWindow);
	    }
	  else          /* restore */
	    {
	      CONSOLE_SCREEN_BUFFER_INFO now;
	      COORD cob = { 0, 0 };

	      if (!GetConsoleScreenBufferInfo (get_output_handle (), &now))
		break;

	      if (!savebuf)
		break;

	      WriteConsoleOutputA (get_output_handle (), savebuf,
				   savebufsiz, cob, &now.srWindow);

	      free (savebuf);
	      savebuf = NULL;
	      savebufsiz.X = savebufsiz.Y = 0;
	    }
	  break;

	case 1000: /* Mouse support */
	  use_mouse = (c == 'h') ? TRUE : FALSE;
	  syscall_printf ("mouse support %sabled", use_mouse ? "en" : "dis");
	  break;

	case 2000: /* Raw keyboard mode */
	  set_raw_win32_keyboard_mode ((c == 'h') ? TRUE : FALSE);
	  break;

	default: /* Ignore */
	  syscall_printf ("unknown h/l command: %d", args_[0]);
	  break;
	}
      break;
    case 'J':
      switch (args_[0])
	{
	case 0:			/* Clear to end of screen */
	  cursor_get (&x, &y);
	  clear_screen (x, y, -1, -1);
	  break;
	case 1:			/* Clear from beginning of screen to cursor */
	  cursor_get (&x, &y);
	  clear_screen (0, 0, x, y);
	  break;
	case 2:			/* Clear screen */
	  clear_screen (0, 0, -1, -1);
	  cursor_set (TRUE, 0,0);
	  break;
	default:
	  goto bad_escape;
	}
      break;

    case 'A':
      cursor_rel (0, -(args_[0] ? args_[0] : 1));
      break;
    case 'B':
      cursor_rel (0, args_[0] ? args_[0] : 1);
      break;
    case 'C':
      cursor_rel (args_[0] ? args_[0] : 1, 0);
      break;
    case 'D':
      cursor_rel (-(args_[0] ? args_[0] : 1),0);
      break;
    case 'K':
      switch (args_[0])
	{
	  case 0:		/* Clear to end of line */
	    cursor_get (&x, &y);
	    clear_screen (x, y, -1, y);
	    break;
	  case 2:		/* Clear line */
	    cursor_get (&x, &y);
	    clear_screen (0, y, -1, y);
	    break;
	  case 1:		/* Clear from bol to cursor */
	    cursor_get (&x, &y);
	    clear_screen (0, y, x, y);
	    break;
	  default:
	    goto bad_escape;
	}
      break;
    case 'H':
    case 'f':
      cursor_set (TRUE, (args_[1] ? args_[1] : 1) - 1,
			(args_[0] ? args_[0] : 1) - 1);
      break;
    case 'G':   /* hpa - position cursor at column n - 1 */
      cursor_get (&x, &y);
      cursor_set (FALSE, (args_[0] ? args_[0] - 1 : 0), y);
      break;
    case 'd':   /* vpa - position cursor at line n */
      cursor_get (&x, &y);
      cursor_set (TRUE, x, (args_[0] ? args_[0] - 1 : 0));
      break;
    case 's':   /* Save cursor position */
      cursor_get (&savex, &savey);
      break;
    case 'u':   /* Restore cursor position */
      cursor_set (FALSE, savex, savey);
      break;
    case 'I':	/* TAB */
      cursor_get (&x, &y);
      cursor_set (FALSE, 8 * (x / 8 + 1), y);
      break;
    case 'L':				/* AL - insert blank lines */
      args_[0] = args_[0] ? args_[0] : 1;
      cursor_get (&x, &y);
      scroll_screen (0, y, -1, -1, 0, y + args_[0]);
      break;
    case 'M':				/* DL - delete lines */
      args_[0] = args_[0] ? args_[0] : 1;
      cursor_get (&x, &y);
      scroll_screen (0, y + args_[0], -1, -1, 0, y);
      break;
    case '@':				/* IC - insert chars */
      args_[0] = args_[0] ? args_[0] : 1;
      cursor_get (&x, &y);
      scroll_screen (x, y, -1, y, x + args_[0], y);
      break;
    case 'P':				/* DC - delete chars */
      args_[0] = args_[0] ? args_[0] : 1;
      cursor_get (&x, &y);
      scroll_screen (x + args_[0], y, -1, y, x, y);
      break;
    case 'S':				/* SF - Scroll forward */
      args_[0] = args_[0] ? args_[0] : 1;
      scroll_screen (0, args_[0], -1, -1, 0, 0);
      break;
    case 'T':				/* SR - Scroll down */
      fillin_info ();
      args_[0] = args_[0] ? args_[0] : 1;
      scroll_screen (0, 0, -1, -1, 0, info.winTop + args_[0]);
      break;
    case 'X':				/* ec - erase chars */
      args_[0] = args_[0] ? args_[0] : 1;
      cursor_get (&x, &y);
      scroll_screen (x + args_[0], y, -1, y, x, y);
      scroll_screen (x, y, -1, y, x + args_[0], y);
      break;
    case 'Z':				/* Back tab */
      cursor_get (&x, &y);
      cursor_set (FALSE, ((8 * (x / 8 + 1)) - 8), y);
      break;
    case 'b':				/* Repeat char #1 #2 times */
      if (insert_mode)
	{
	  cursor_get (&x, &y);
	  scroll_screen (x, y, -1, y, x + args_[1], y);
	}
      while (args_[1]--)
	WriteFile (get_output_handle (), &args_[0], 1, (DWORD *) &x, 0);
      break;
    case 'c':				/* u9 - Terminal enquire string */
      strcpy (buf, "\033[?6c");
      puts_readahead (buf);
      break;
    case 'n':
      switch (args_[0])
	{
	case 6:				/* u7 - Cursor position request */
	  cursor_get (&x, &y);
	  y -= info.winTop;
	  /* x -= info.winLeft;		// not available yet */
	  __small_sprintf (buf, "\033[%d;%dR", y + 1, x + 1);
	  puts_readahead (buf);
	  break;
    default:
	  goto bad_escape;
	}
      break;
    case 'r':				/* Set Scroll region */
      scroll_region.Top = args_[0] ? args_[0] - 1 : 0;
      scroll_region.Bottom = args_[1] ? args_[1] - 1 : -1;
      cursor_set (TRUE, 0, 0);
      break;
    case 'g':				/* TAB set/clear */
      break;
    default:
bad_escape:
      break;
    }
}

const unsigned char *
fhandler_console::write_normal (const unsigned char *src,
				const unsigned char *end)
{
  /* Scan forward to see what a char which needs special treatment */
  DWORD done;
  const unsigned char *found = src;

  while (found < end)
    {
      if (base_chars[*found] != NOR)
	break;
      found++;
    }

  /* Print all the base ones out */
  if (found != src)
    {
      DWORD len = found - src;
      do
	{
	  DWORD buf_len;
	  char buf[CONVERT_LIMIT];
	  done = buf_len = min (sizeof (buf), len);
	  if (!str_to_con (buf, (const char *) src, buf_len))
	    {
	      debug_printf ("conversion error, handle %p", get_output_handle ());
	      __seterrno ();
	      return 0;
	    }

	  if (insert_mode)
	    {
	      int x, y;
	      cursor_get (&x, &y);
	      scroll_screen (x, y, -1, y, x + buf_len, y);
	    }

	  if (!WriteFile (get_output_handle (), buf, buf_len, &done, 0))
	    {
	      debug_printf ("write failed, handle %p", get_output_handle ());
	      __seterrno ();
	      return 0;
	    }
	  len -= done;
	  src += done;
	}
      while (len > 0);
    }

  if (src < end)
    {
      int x, y;
      switch (base_chars[*src])
	{
	case BEL:
	  Beep (412, 100);
	  break;
	case ESC:
	  state_ = gotesc;
	  break;
	case DWN:		/* WriteFile ("\n") always adds CR... */
	  cursor_get (&x, &y);
	  if (y >= srBottom)
	    {
	      if (y < info.winBottom || scroll_region.Top)
		{
		  scroll_screen (0, srTop + 1, -1, srBottom, 0, srTop);
		  y--;
		}
	      else
		WriteFile (get_output_handle (), "\n", 1, &done, 0);
	    }
	  if (!get_w_binary ())
	    x = 0;
	  cursor_set (FALSE, x, y + 1);
	  break;
	case BAK:
	  cursor_rel (-1, 0);
	  break;
	case IGN:
	  cursor_rel (1, 0);
	  break;
	case CR:
	  cursor_get (&x, &y);
	  cursor_set (FALSE, 0, y);
	  break;
	case ERR:
	  WriteFile (get_output_handle (), src, 1, &done, 0);
	  break;
	case TAB:
	  cursor_get (&x, &y);
	  cursor_set (FALSE, 8 * (x / 8 + 1), y);
	  break;
	}
      src ++;
    }
  return src;
}

int
fhandler_console::write (const void *vsrc, size_t len)
{
  /* Run and check for ansi sequences */
  unsigned const char *src = (unsigned char *) vsrc;
  unsigned const char *end = src + len;

  debug_printf ("%x, %d", vsrc, len);

  while (src < end)
    {
      debug_printf ("at %d(%c) state is %d", *src, isprint (*src) ? *src : ' ',
		    state_);
      switch (state_)
	{
	case normal:
	  src = write_normal (src, end);
	  if (!src) /* write_normal failed */
	    return -1;
	  break;
	case gotesc:
	  if (*src == '[')
	    {
	      state_ = gotsquare;
	      saw_question_mark = FALSE;
	      for (nargs_ = 0; nargs_ < MAXARGS; nargs_++)
		args_[nargs_] = 0;
	      nargs_ = 0;
	    }
	  else if (*src == ']')
	    {
	      rarg = 0;
	      my_title_buf[0] = '\0';
	      state_ = gotrsquare;
	    }
	  else if (*src == 'M')		/* Reverse Index */
	    {
	      fillin_info ();
	      scroll_screen (0, 0, -1, -1, 0, info.winTop + 1);
	      state_ = normal;
	    }
	  else if (*src == 'c')		/* Reset Linux terminal */
	    {
	      set_default_attr ();
	      clear_screen (0, 0, -1, -1);
	      cursor_set (TRUE, 0, 0);
	      state_ = normal;
	    }
	  else if (*src == '8')		/* Restore cursor position */
	    {
	      cursor_set (FALSE, savex, savey);
	      state_ = normal;
	    }
	  else if (*src == '7')		/* Save cursor position */
	    {
	      cursor_get (&savex, &savey);
	      state_ = normal;
	    }
	  else if (*src == 'R')
	      state_ = normal;
	  else
	    {
	      state_ = normal;
	    }
	  src++;
	  break;
	case gotarg1:
	  if (isdigit (*src))
	    {
	      args_[nargs_] = args_[nargs_] * 10 + *src - '0';
	      src++;
	    }
	  else if (*src == ';')
	    {
	      src++;
	      nargs_++;
	      if (nargs_ >= MAXARGS)
		nargs_--;
	    }
	  else
	    {
	      state_ = gotcommand;
	    }
	  break;
	case gotcommand:
	  char_command (*src++);
	  state_ = normal;
	  break;
	case gotrsquare:
	  if (isdigit (*src))
	    rarg = rarg * 10 + (*src - '0');
	  else if (*src == ';' && (rarg == 2 || rarg == 0))
	    state_ = gettitle;
	  else
	    state_ = eattitle;
	  src++;
	  break;
	case eattitle:
	case gettitle:
	  {
	    int n = strlen (my_title_buf);
	    if (*src < ' ' || *src >= '\177')
	      {
		if (*src == '\007' && state_ == gettitle)
		  {
		    if (old_title)
		      strcpy (old_title, my_title_buf);
		    set_console_title (my_title_buf);
		  }
		state_ = normal;
	      }
	    else if (n < TITLESIZE)
	      {
		my_title_buf[n++] = *src;
		my_title_buf[n] = '\0';
	      }
	    src++;
	    break;
	  }
	case gotsquare:
	  if (*src == ';')
	    {
	      state_ = gotarg1;
	      nargs_++;
	      src++;
	    }
	  else if (isalpha (*src))
	    {
	      state_ = gotcommand;
	    }
	  else if (*src != '@' && !isalpha (*src) && !isdigit (*src))
	    {
	      if (*src == '?')
		saw_question_mark = TRUE;
	      /* ignore any extra chars between [ and first arg or command */
	      src++;
	    }
	  else
	    state_ = gotarg1;
	  break;
	}
    }

  syscall_printf ("%d = write_console (,..%d)", len, len);

  return len;
}

static struct {
  int vk;
  const char *val[4];
} keytable[] = {
	       /* NORMAL */  /* SHIFT */    /* CTRL */       /* ALT */
  {VK_LEFT,	{"\033[D",	"\033[D",	"\033[D",	"\033\033[D"}},
  {VK_RIGHT,	{"\033[C",	"\033[C",	"\033[C",	"\033\033[C"}},
  {VK_UP,	{"\033[A",	"\033[A",	"\033[A",	"\033\033[A"}},
  {VK_DOWN,	{"\033[B",	"\033[B",	"\033[B",	"\033\033[B"}},
  {VK_PRIOR,	{"\033[5~",	"\033[5~",	"\033[5~",	"\033\033[5~"}},
  {VK_NEXT,	{"\033[6~",	"\033[6~",	"\033[6~",	"\033\033[6~"}},
  {VK_HOME,	{"\033[1~",	"\033[1~",	"\033[1~",	"\033\033[1~"}},
  {VK_END,	{"\033[4~",	"\033[4~",	"\033[4~",	"\033\033[4~"}},
  {VK_INSERT,	{"\033[2~",	"\033[2~",	"\033[2~",	"\033\033[2~"}},
  {VK_DELETE,	{"\033[3~",	"\033[3~",	"\033[3~",	"\033\033[3~"}},
  {VK_F1,	{"\033[[A",	"\033[23~",	NULL,		NULL}},
  {VK_F2,	{"\033[[B",	"\033[24~",	NULL,		NULL}},
  {VK_F3,	{"\033[[C",	"\033[25~",	NULL,		NULL}},
  {VK_F4,	{"\033[[D",	"\033[26~",	NULL,		NULL}},
  {VK_F5,	{"\033[[E",	"\033[28~",	NULL,		NULL}},
  {VK_F6,	{"\033[17~",	"\033[29~",	"\036",		NULL}},
  {VK_F7,	{"\033[18~",	"\033[31~",	NULL,		NULL}},
  {VK_F8,	{"\033[19~",	"\033[32~",	NULL,		NULL}},
  {VK_F9,	{"\033[20~",	"\033[33~",	NULL,		NULL}},
  {VK_F10,	{"\033[21~",	"\033[34~",	NULL,		NULL}},
  {VK_F11,	{"\033[23~",	NULL,		NULL,		NULL}},
  {VK_F12,	{"\033[24~",	NULL,		NULL,		NULL}},
  {VK_NUMPAD5,	{"\033[G",	NULL,		NULL,		NULL}},
  {VK_CLEAR,	{"\033[G",	NULL,		NULL,		NULL}},
  {'6',		{NULL,		NULL,		"\036",		NULL}},
  {0,		{"",		NULL,		NULL,		NULL}}
};

const char *
get_nonascii_key (INPUT_RECORD& input_rec, char *tmp)
{
#define NORMAL  0
#define SHIFT	1
#define CONTROL	2
#define ALT	3
  int modifier_index = NORMAL;

  if (input_rec.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
    modifier_index = SHIFT;
  else if (input_rec.Event.KeyEvent.dwControlKeyState &
		(LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
    modifier_index = CONTROL;
  else if (input_rec.Event.KeyEvent.dwControlKeyState &
		(LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
    modifier_index = ALT;

  for (int i = 0; keytable[i].vk; i++)
    if (input_rec.Event.KeyEvent.wVirtualKeyCode == keytable[i].vk)
      return keytable[i].val[modifier_index];

  if (input_rec.Event.KeyEvent.uChar.AsciiChar)
    {
      tmp[0] = input_rec.Event.KeyEvent.uChar.AsciiChar;
      tmp[1] = '\0';
      return tmp;
    }
  return NULL;
}

void
fhandler_console::init (HANDLE f, DWORD a, mode_t bin)
{
  this->fhandler_termios::init (f, bin, a);

  /* Ensure both input and output console handles are open */
  int mode = 0;

  a &= GENERIC_READ | GENERIC_WRITE;
  if (a == GENERIC_READ)
    mode = O_RDONLY;
  if (a == GENERIC_WRITE)
    mode = O_WRONLY;
  if (a == (GENERIC_READ | GENERIC_WRITE))
    mode = O_RDWR;
  open (0, mode);
  if (f != INVALID_HANDLE_VALUE)
    CloseHandle (f);	/* Reopened by open */

  output_tcsetattr (0, &tc->ti);
}

int
fhandler_console::igncr_enabled (void)
{
  return tc->ti.c_iflag & IGNCR;
}

void
fhandler_console::set_close_on_exec (int val)
{
  this->fhandler_base::set_close_on_exec (val);
  set_inheritance (output_handle, val);
}

void
fhandler_console::fixup_after_fork (HANDLE)
{
  HANDLE h = get_handle ();
  HANDLE oh = get_output_handle ();

  /* Windows does not allow duplication of console handles between processes
     so open the console explicitly. */

  if (!open (get_name (), get_flags (), 0))
    system_printf ("error opening console after fork, %E");

  if (!get_close_on_exec ())
    {
      CloseHandle (h);
      CloseHandle (oh);
    }
}

void __stdcall
set_console_title (char *title)
{
  int rc;
  char buf[257];
  strncpy (buf, title, sizeof (buf) - 1);
  buf[sizeof (buf) - 1] = '\0';
  if ((rc = WaitForSingleObject (title_mutex, 15000)) != WAIT_OBJECT_0)
    sigproc_printf ("wait for title mutex failed rc %d, %E", rc);
  SetConsoleTitle (buf);
  ReleaseMutex (title_mutex);
  debug_printf ("title '%s'", buf);
}

void
fhandler_console::fixup_after_exec (HANDLE)
{
  HANDLE h = get_handle ();
  HANDLE oh = get_output_handle ();

  if (!open (get_name (), get_flags (), 0))
    {
      int sawerr = 0;
      if (!get_io_handle ())
	{
	  system_printf ("error opening input console handle after exec, errno %d, %E", get_errno ());
	  sawerr = 1;
	}
      if (!get_output_handle ())
	{
	  system_printf ("error opening input console handle after exec, errno %d, %E", get_errno ());
	  sawerr = 1;
	}

      if (!sawerr)
	system_printf ("error opening console after exec, errno %d, %E", get_errno ());
    }

  CloseHandle (h);
  CloseHandle (oh);
  return;
}
