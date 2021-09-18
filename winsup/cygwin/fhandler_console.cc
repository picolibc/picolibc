/* fhandler_console.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/cygwin.h>
#include <cygwin/kd.h>
#include <unistd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "pinfo.h"
#include "shared_info.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "registry.h"
#include <asm/socket.h>
#include "sync.h"
#include "child_info.h"
#include "cygwait.h"

/* Don't make this bigger than NT_MAX_PATH as long as the temporary buffer
   is allocated using tmp_pathbuf!!! */
#define CONVERT_LIMIT NT_MAX_PATH

#define ALT_PRESSED (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)
#define CTRL_PRESSED (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)

#define con (shared_console_info->con)
#define srTop (con.b.srWindow.Top + con.scroll_region.Top)
#define srBottom ((con.scroll_region.Bottom < 0) ? \
		  con.b.srWindow.Bottom : \
		  con.b.srWindow.Top + con.scroll_region.Bottom)
#define con_is_legacy (shared_console_info && con.is_legacy)

#define CONS_THREAD_SYNC "cygcons.thread_sync"
static bool NO_COPY master_thread_started = false;

const unsigned fhandler_console::MAX_WRITE_CHARS = 16384;

fhandler_console::console_state NO_COPY *fhandler_console::shared_console_info;

bool NO_COPY fhandler_console::invisible_console;

/* Mutex for AttachConsole()/FreeConsole() in fhandler_tty.cc */
HANDLE NO_COPY attach_mutex;

static inline void
acquire_attach_mutex (DWORD t)
{
  if (attach_mutex)
    WaitForSingleObject (attach_mutex, t);
}

static inline void
release_attach_mutex ()
{
  if (attach_mutex)
    ReleaseMutex (attach_mutex);
}

/* con_ra is shared in the same process.
   Only one console can exist in a process, therefore, static is suitable. */
static struct fhandler_base::rabuf_t con_ra;

/* Write pending buffer for ESC sequence handling
   in xterm compatible mode */
static wchar_t last_char;

/* simple helper class to accumulate output in a buffer
   and send that to the console on request: */
static class write_pending_buffer
{
private:
  static const size_t WPBUF_LEN = 256u;
  char buf[WPBUF_LEN];
  size_t ixput;
public:
  inline void put (char x)
  {
    if (ixput < WPBUF_LEN)
      buf[ixput++] = x;
  }
  inline void empty () { ixput = 0u; }
  inline void send (HANDLE &handle)
  {
    wchar_t bufw[WPBUF_LEN];
    DWORD len = sys_mbstowcs (bufw, WPBUF_LEN, buf, ixput);
    WriteConsoleW (handle, bufw, len, NULL, 0);
  }
} wpbuf;

static void
beep ()
{
  const WCHAR ding[] = L"\\media\\ding.wav";
  reg_key r (HKEY_CURRENT_USER, KEY_ALL_ACCESS, L"AppEvents", L"Schemes",
	     L"Apps", L".Default", L".Default", L".Current", NULL);
  if (r.created ())
    {
      tmp_pathbuf tp;

      PWCHAR ding_path = tp.w_get ();
      wcpcpy (wcpcpy (ding_path, windows_directory), ding);
      r.set_string (L"", ding_path);
    }
  MessageBeep (MB_OK);
}

fhandler_console::console_state *
fhandler_console::open_shared_console (HWND hw, HANDLE& h, bool& create)
{
  wchar_t namebuf[(sizeof "XXXXXXXXXXXXXXXXXX-consNNNNNNNNNN")];
  __small_swprintf (namebuf, L"%S-cons%p", &cygheap->installation_key, hw);

  shared_locations m = create ? SH_SHARED_CONSOLE : SH_JUSTOPEN;
  console_state *res = (console_state *)
    open_shared (namebuf, 0, h, sizeof (*shared_console_info), &m);
  create = m != SH_JUSTOPEN;
  return res;
}

class console_unit
{
  int n;
  unsigned long bitmask;
  HWND me;

public:
  operator int () const {return n;}
  console_unit (HWND);
  friend BOOL CALLBACK enum_windows (HWND, LPARAM);
};

BOOL CALLBACK
enum_windows (HWND hw, LPARAM lp)
{
  console_unit *this1 = (console_unit *) lp;
  if (hw == this1->me)
    return TRUE;
  HANDLE h = NULL;
  fhandler_console::console_state *cs;
  if ((cs = fhandler_console::open_shared_console (hw, h)))
    {
      this1->bitmask ^= 1 << cs->tty_min_state.getntty ();
      UnmapViewOfFile ((void *) cs);
      CloseHandle (h);
    }
  return TRUE;
}

console_unit::console_unit (HWND me0):
  bitmask (0xffffffff), me (me0)
{
  EnumWindows (enum_windows, (LPARAM) this);
  n = (_minor_t) ffs (bitmask) - 1;
  if (n < 0)
    api_fatal ("console device allocation failure - too many consoles in use, max consoles is 32");
}

static DWORD WINAPI
cons_master_thread (VOID *arg)
{
  fhandler_console *fh = (fhandler_console *) arg;
  tty *ttyp = (tty *) fh->tc ();
  fhandler_console::handle_set_t handle_set;
  fh->get_duplicated_handle_set (&handle_set);
  HANDLE thread_sync_event;
  DuplicateHandle (GetCurrentProcess (), fh->thread_sync_event,
		   GetCurrentProcess (), &thread_sync_event,
		   0, FALSE, DUPLICATE_SAME_ACCESS);
  SetEvent (thread_sync_event);
  master_thread_started = true;
  /* Do not touch class members after here because the class instance
     may have been destroyed. */
  fhandler_console::cons_master_thread (&handle_set, ttyp);
  fhandler_console::close_handle_set (&handle_set);
  SetEvent (thread_sync_event);
  CloseHandle (thread_sync_event);
  return 0;
}

/* This thread processes signals derived from input messages.
   Without this thread, those signals can be handled only when
   the process calls read() or select(). This thread reads input
   records, processes signals and removes corresponding record.
   The other input records are kept back for read() or select(). */
void
fhandler_console::cons_master_thread (handle_set_t *p, tty *ttyp)
{
  DWORD output_stopped_at = 0;
  while (con.owner == myself->pid)
    {
      DWORD total_read, n, i;
      INPUT_RECORD input_rec[INREC_SIZE];

      WaitForSingleObject (p->input_mutex, INFINITE);
      total_read = 0;
      switch (cygwait (p->input_handle, (DWORD) 0))
	{
	case WAIT_OBJECT_0:
	  ReadConsoleInputW (p->input_handle,
			     input_rec, INREC_SIZE, &total_read);
	  break;
	case WAIT_TIMEOUT:
	case WAIT_SIGNALED:
	case WAIT_CANCELED:
	  break;
	default: /* Error */
	  ReleaseMutex (p->input_mutex);
	  return;
	}
      /* If ENABLE_VIRTUAL_TERMINAL_INPUT is not set, changing
	 window height does not generate WINDOW_BUFFER_SIZE_EVENT.
	 Therefore, check windows size every time here. */
      if (!wincap.has_con_24bit_colors () || con_is_legacy)
	{
	  SHORT y = con.dwWinSize.Y;
	  SHORT x = con.dwWinSize.X;
	  con.fillin (p->output_handle);
	  if (y != con.dwWinSize.Y || x != con.dwWinSize.X)
	    {
	      con.scroll_region.Top = 0;
	      con.scroll_region.Bottom = -1;
	      ttyp->kill_pgrp (SIGWINCH);
	    }
	}
      for (i = 0; i < total_read; i++)
	{
	  const wchar_t wc = input_rec[i].Event.KeyEvent.uChar.UnicodeChar;
	  if ((wint_t) wc >= 0x80)
	    continue;
	  char c = (char) wc;
	  bool processed = false;
	  termios &ti = ttyp->ti;
	  switch (input_rec[i].EventType)
	    {
	    case KEY_EVENT:
	      if (ti.c_lflag & ISIG)
		{
		  int sig = 0;
		  if (CCEQ (ti.c_cc[VINTR], c))
		    sig = SIGINT;
		  else if (CCEQ (ti.c_cc[VQUIT], c))
		    sig = SIGQUIT;
		  else if (CCEQ (ti.c_cc[VSUSP], c))
		    sig = SIGTSTP;
		  if (sig && input_rec[i].Event.KeyEvent.bKeyDown)
		    {
		      ttyp->kill_pgrp (sig);
		      ttyp->output_stopped = false;
		      ti.c_lflag &= ~FLUSHO;
		      /* Discard type ahead input */
		      goto skip_writeback;
		    }
		}
	      if (ti.c_iflag & IXON)
		{
		  if (CCEQ (ti.c_cc[VSTOP], c))
		    {
		      if (!ttyp->output_stopped
			  && input_rec[i].Event.KeyEvent.bKeyDown)
			{
			  ttyp->output_stopped = true;
			  output_stopped_at = i;
			}
		      processed = true;
		    }
		  else if (CCEQ (ti.c_cc[VSTART], c))
		    {
		restart_output:
		      if (input_rec[i].Event.KeyEvent.bKeyDown)
			ttyp->output_stopped = false;
		      processed = true;
		    }
		  else if ((ti.c_iflag & IXANY) && ttyp->output_stopped
			   && c && i >= output_stopped_at)
		    goto restart_output;
		}
	      if ((ti.c_lflag & ICANON) && (ti.c_lflag & IEXTEN)
		  && CCEQ (ti.c_cc[VDISCARD], c))
		{
		  if (input_rec[i].Event.KeyEvent.bKeyDown)
		    ti.c_lflag ^= FLUSHO;
		  processed = true;
		}
	      break;
	    case WINDOW_BUFFER_SIZE_EVENT:
	      SHORT y = con.dwWinSize.Y;
	      SHORT x = con.dwWinSize.X;
	      con.fillin (p->output_handle);
	      if (y != con.dwWinSize.Y || x != con.dwWinSize.X)
		{
		  con.scroll_region.Top = 0;
		  con.scroll_region.Bottom = -1;
		  if (wincap.has_con_24bit_colors () && !con_is_legacy)
		    { /* Fix tab position */
		      /* Re-setting ENABLE_VIRTUAL_TERMINAL_PROCESSING
			 fixes the tab position. */
		      set_output_mode (tty::restore, &ti, p);
		      set_output_mode (tty::cygwin, &ti, p);
		    }
		  ttyp->kill_pgrp (SIGWINCH);
		}
	      processed = true;
	      break;
	    }
	  if (processed)
	    { /* Remove corresponding record. */
	      memmove (input_rec + i, input_rec + i + 1,
		       sizeof (INPUT_RECORD) * (total_read - i - 1));
	      total_read--;
	      i--;
	    }
	}
      if (total_read)
	/* Write back input records other than interrupt. */
	WriteConsoleInputW (p->input_handle, input_rec, total_read, &n);
skip_writeback:
      ReleaseMutex (p->input_mutex);
      cygwait (40);
    }
}

bool
fhandler_console::set_unit ()
{
  bool created;
  fh_devices devset;
  lock_ttys here;
  HWND me;
  fh_devices this_unit = dev ();
  bool generic_console = this_unit == FH_CONIN || this_unit == FH_CONOUT;
  if (shared_console_info)
    {
      fh_devices shared_unit =
	(fh_devices) shared_console_info->tty_min_state.getntty ();
      devset = (shared_unit == this_unit || this_unit == FH_CONSOLE
		|| generic_console
		|| this_unit == FH_TTY) ?
		shared_unit : FH_ERROR;
      created = false;
    }
  else if ((!generic_console &&
	    (myself->ctty != -1 && !iscons_dev (myself->ctty)))
	   || !(me = GetConsoleWindow ()))
    devset = FH_ERROR;
  else
    {
      created = true;
      shared_console_info =
	open_shared_console (me, cygheap->console_h, created);
      ProtectHandleINH (cygheap->console_h);
      if (created)
	shared_console_info->
	  tty_min_state.setntty (DEV_CONS_MAJOR, console_unit (me));
      devset = (fh_devices) shared_console_info->tty_min_state.getntty ();
      if (created)
	con.owner = myself->pid;
    }
  if (!created && shared_console_info)
    {
      while (con.owner > MAX_PID)
	Sleep (1);
      pinfo p (con.owner);
      if (!p)
	con.owner = myself->pid;
    }

  dev ().parse (devset);
  if (devset != FH_ERROR)
    pc.file_attributes (FILE_ATTRIBUTE_NORMAL);
  else
    {
      set_handle (NULL);
      set_output_handle (NULL);
      created = false;
    }
  return created;
}

/* Allocate and initialize the shared record for the current console. */
void
fhandler_console::setup ()
{
  if (set_unit ())
    {
      con.scroll_region.Bottom = -1;
      con.dwLastCursorPosition.X = -1;
      con.dwLastCursorPosition.Y = -1;
      con.dwLastMousePosition.X = -1;
      con.dwLastMousePosition.Y = -1;
      con.savex = con.savey = -1;
      con.screen_alternated = false;
      con.dwLastButtonState = 0;	/* none pressed */
      con.last_button_code = 3;	/* released */
      con.underline_color = FOREGROUND_GREEN | FOREGROUND_BLUE;
      con.dim_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
      con.meta_mask = LEFT_ALT_PRESSED;
      /* Set the mask that determines if an input keystroke is modified by
	 META.  We set this based on the keyboard layout language loaded
	 for the current thread.  The left <ALT> key always generates
	 META, but the right <ALT> key only generates META if we are using
	 an English keyboard because many "international" keyboards
	 replace common shell symbols ('[', '{', etc.) with accented
	 language-specific characters (umlaut, accent grave, etc.).  On
	 these keyboards right <ALT> (called AltGr) is used to produce the
	 shell symbols and should not be interpreted as META. */
      if (PRIMARYLANGID (LOWORD (GetKeyboardLayout (0))) == LANG_ENGLISH)
	con.meta_mask |= RIGHT_ALT_PRESSED;
      con.set_default_attr ();
      con.backspace_keycode = CERASE;
      con.cons_rapoi = NULL;
      shared_console_info->tty_min_state.is_console = true;
      con.cursor_key_app_mode = false;
    }
}

char *&
fhandler_console::rabuf ()
{
  return con_ra.rabuf;
}

size_t &
fhandler_console::ralen ()
{
  return con_ra.ralen;
}

size_t &
fhandler_console::raixget ()
{
  return con_ra.raixget;
}

size_t &
fhandler_console::raixput ()
{
  return con_ra.raixput;
}

size_t &
fhandler_console::rabuflen ()
{
  return con_ra.rabuflen;
}

/* The function set_{in,out}put_mode() should be static so that they
   can be called even after the fhandler_console instance is deleted. */
void
fhandler_console::set_input_mode (tty::cons_mode m, const termios *t,
				  const handle_set_t *p)
{
  DWORD flags = 0, oflags;
  WaitForSingleObject (p->input_mutex, INFINITE);
  GetConsoleMode (p->input_handle, &oflags);
  switch (m)
    {
    case tty::restore:
      flags = ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT;
      break;
    case tty::cygwin:
      flags = ENABLE_WINDOW_INPUT;
      if (wincap.has_con_24bit_colors () && !con_is_legacy)
	flags |= ENABLE_VIRTUAL_TERMINAL_INPUT;
      else
	flags |= ENABLE_MOUSE_INPUT;
      break;
    case tty::native:
      if (t->c_lflag & ECHO)
	flags |= ENABLE_ECHO_INPUT;
      if (t->c_lflag & ICANON)
	flags |= ENABLE_LINE_INPUT;
      if (flags & ENABLE_ECHO_INPUT && !(flags & ENABLE_LINE_INPUT))
	/* This is illegal, so turn off the echo here, and fake it
	   when we read the characters */
	flags &= ~ENABLE_ECHO_INPUT;
      if (t->c_lflag & ISIG)
	flags |= ENABLE_PROCESSED_INPUT;
      break;
    }
  SetConsoleMode (p->input_handle, flags);
  if (!(oflags & ENABLE_VIRTUAL_TERMINAL_INPUT)
      && (flags & ENABLE_VIRTUAL_TERMINAL_INPUT)
      && con.cursor_key_app_mode)
    { /* Restore DECCKM */
      set_output_mode (tty::cygwin, t, p);
      WriteConsoleW (p->output_handle, L"\033[?1h", 5, NULL, 0);
    }
  ReleaseMutex (p->input_mutex);
}

void
fhandler_console::set_output_mode (tty::cons_mode m, const termios *t,
				   const handle_set_t *p)
{
  DWORD flags = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
  WaitForSingleObject (p->output_mutex, INFINITE);
  switch (m)
    {
    case tty::restore:
      break;
    case tty::cygwin:
      if (wincap.has_con_24bit_colors () && !con_is_legacy)
	flags |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      fallthrough;
    case tty::native:
      if (wincap.has_con_24bit_colors () && !con_is_legacy
	  && (!(t->c_oflag & OPOST) || !(t->c_oflag & ONLCR)))
	flags |= DISABLE_NEWLINE_AUTO_RETURN;
      break;
    }
  SetConsoleMode (p->output_handle, flags);
  ReleaseMutex (p->output_mutex);
}

/* Return the tty structure associated with a given tty number.  If the
   tty number is < 0, just return a dummy record. */
tty_min *
tty_list::get_cttyp ()
{
  dev_t n = myself->ctty;
  if (iscons_dev (n))
    return fhandler_console::shared_console_info ?
      &fhandler_console::shared_console_info->tty_min_state : NULL;
  else if (istty_slave_dev (n))
    return &ttys[device::minor (n)];
  else
    return NULL;
}

void
fhandler_console::setup_io_mutex (void)
{
  char buf[MAX_PATH];
  DWORD res;

  res = WAIT_FAILED;
  if (!input_mutex || WAIT_FAILED == (res = acquire_input_mutex (0)))
    {
      shared_name (buf, "cygcons.input.mutex", get_minor ());
      input_mutex = OpenMutex (MAXIMUM_ALLOWED, TRUE, buf);
      if (!input_mutex)
	input_mutex = CreateMutex (&sec_none, FALSE, buf);
      if (!input_mutex)
	{
	  __seterrno ();
	  return;
	}
    }
  if (res == WAIT_OBJECT_0)
    release_input_mutex ();

  res = WAIT_FAILED;
  if (!output_mutex || WAIT_FAILED == (res = acquire_output_mutex (0)))
    {
      shared_name (buf, "cygcons.output.mutex", get_minor ());
      output_mutex = OpenMutex (MAXIMUM_ALLOWED, TRUE, buf);
      if (!output_mutex)
	output_mutex = CreateMutex (&sec_none, FALSE, buf);
      if (!output_mutex)
	{
	  __seterrno ();
	  return;
	}
    }
  if (res == WAIT_OBJECT_0)
    release_output_mutex ();
}

inline DWORD
dev_console::con_to_str (char *d, int dlen, WCHAR w)
{
  return sys_wcstombs (d, dlen, &w, 1);
}

inline UINT
dev_console::get_console_cp ()
{
  /* The alternate charset is always 437, just as in the Linux console. */
  return alternate_charset_active ? 437 : 0;
}

inline DWORD
dev_console::str_to_con (mbtowc_p f_mbtowc, PWCHAR d, const char *s, DWORD sz)
{
  return _sys_mbstowcs (f_mbtowc, d, CONVERT_LIMIT, s, sz);
}

bool
fhandler_console::set_raw_win32_keyboard_mode (bool new_mode)
{
  bool old_mode = con.raw_win32_keyboard_mode;
  con.raw_win32_keyboard_mode = new_mode;
  syscall_printf ("raw keyboard mode %sabled",
		  con.raw_win32_keyboard_mode ? "en" : "dis");
  return old_mode;
};

void
fhandler_console::set_cursor_maybe ()
{
  acquire_attach_mutex (INFINITE);
  con.fillin (get_output_handle ());
  release_attach_mutex ();
  /* Nothing to do for xterm compatible mode. */
  if (wincap.has_con_24bit_colors () && !con_is_legacy)
    return;
  if (con.dwLastCursorPosition.X != con.b.dwCursorPosition.X ||
      con.dwLastCursorPosition.Y != con.b.dwCursorPosition.Y)
    {
      acquire_attach_mutex (INFINITE);
      SetConsoleCursorPosition (get_output_handle (), con.b.dwCursorPosition);
      release_attach_mutex ();
      con.dwLastCursorPosition = con.b.dwCursorPosition;
    }
}

/* Workaround for a bug of windows xterm compatible mode. */
/* The horizontal tab positions are broken after resize. */
void
fhandler_console::fix_tab_position (void)
{
  /* Re-setting ENABLE_VIRTUAL_TERMINAL_PROCESSING
     fixes the tab position. */
  set_output_mode (tty::restore, &get_ttyp ()->ti, &handle_set);
  set_output_mode (tty::cygwin, &get_ttyp ()->ti, &handle_set);
}

bool
fhandler_console::send_winch_maybe ()
{
  SHORT y = con.dwWinSize.Y;
  SHORT x = con.dwWinSize.X;
  con.fillin (get_output_handle ());

  if (y != con.dwWinSize.Y || x != con.dwWinSize.X)
    {
      con.scroll_region.Top = 0;
      con.scroll_region.Bottom = -1;
      if (wincap.has_con_24bit_colors () && !con_is_legacy)
	fix_tab_position ();
      get_ttyp ()->kill_pgrp (SIGWINCH);
      return true;
    }
  return false;
}

/* Check whether a mouse event is to be reported as an escape sequence */
bool
fhandler_console::mouse_aware (MOUSE_EVENT_RECORD& mouse_event)
{
  if (!con.use_mouse)
    return 0;

  /* Adjust mouse position by window scroll buffer offset
     and remember adjusted position in state for use by read() */
  CONSOLE_SCREEN_BUFFER_INFO now;
  if (!GetConsoleScreenBufferInfo (get_output_handle (), &now))
    /* Cannot adjust position by window scroll buffer offset */
    return 0;

  con.dwMousePosition.X = mouse_event.dwMousePosition.X - now.srWindow.Left;
  con.dwMousePosition.Y = mouse_event.dwMousePosition.Y - now.srWindow.Top;

  return ((mouse_event.dwEventFlags == 0
	   || mouse_event.dwEventFlags == DOUBLE_CLICK)
	  && mouse_event.dwButtonState != con.dwLastButtonState)
	 || mouse_event.dwEventFlags == MOUSE_WHEELED
	 || (mouse_event.dwEventFlags == MOUSE_MOVED
	     && (con.dwMousePosition.X != con.dwLastMousePosition.X
		 || con.dwMousePosition.Y != con.dwLastMousePosition.Y)
	     && ((con.use_mouse >= 2 && mouse_event.dwButtonState)
		 || con.use_mouse >= 3));
}

void __reg3
fhandler_console::read (void *pv, size_t& buflen)
{
  termios_printf ("read(%p,%d)", pv, buflen);

  push_process_state process_state (PID_TTYIN);

  int copied_chars = 0;

  DWORD timeout = is_nonblocking () ? 0 : INFINITE;

  set_input_mode (tty::cygwin, &get_ttyp ()->ti, &handle_set);

  while (!input_ready && !get_cons_readahead_valid ())
    {
      int bgres;
      if ((bgres = bg_check (SIGTTIN)) <= bg_eof)
	{
	  buflen = bgres;
	  return;
	}

      set_cursor_maybe (); /* to make cursor appear on the screen immediately */
wait_retry:
      switch (cygwait (get_handle (), timeout))
	{
	case WAIT_OBJECT_0:
	  break;
	case WAIT_SIGNALED:
	  goto sig_exit;
	case WAIT_CANCELED:
	  process_state.pop ();
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	case WAIT_TIMEOUT:
	  set_sig_errno (EAGAIN);
	  buflen = (size_t) -1;
	  return;
	default:
	  if (GetLastError () == ERROR_INVALID_HANDLE)
	    { /* Confirm the handle is still valid */
	      DWORD mode;
	      acquire_attach_mutex (INFINITE);
	      BOOL res = GetConsoleMode (get_handle (), &mode);
	      release_attach_mutex ();
	      if (res)
		goto wait_retry;
	    }
	  goto err;
	}

#define buf ((char *) pv)

      int ret;
      acquire_input_mutex (INFINITE);
      acquire_attach_mutex (INFINITE);
      ret = process_input_message ();
      release_attach_mutex ();
      switch (ret)
	{
	case input_error:
	  release_input_mutex ();
	  goto err;
	case input_processing:
	  release_input_mutex ();
	  continue;
	case input_ok: /* input ready */
	  break;
	case input_signalled: /* signalled */
	case input_winch:
	  release_input_mutex ();
	  if (global_sigs[get_ttyp ()->last_sig].sa_flags & SA_RESTART)
	    continue;
	  goto sig_exit;
	default:
	  /* Should not come here */
	  release_input_mutex ();
	  goto err;
	}
    }

  /* Check console read-ahead buffer filled from terminal requests */
  while (con.cons_rapoi && *con.cons_rapoi && buflen)
    {
      buf[copied_chars++] = *con.cons_rapoi++;
      buflen --;
    }

  copied_chars +=
    get_readahead_into_buffer (buf + copied_chars, buflen);

  if (!con_ra.ralen)
    input_ready = false;
  release_input_mutex ();

#undef buf

  buflen = copied_chars;
  return;

err:
  __seterrno ();
  buflen = (size_t) -1;
  return;

sig_exit:
  set_sig_errno (EINTR);
  buflen = (size_t) -1;
}

fhandler_console::input_states
fhandler_console::process_input_message (void)
{
  char tmp[60];

  if (!shared_console_info)
    return input_error;

  termios *ti = &(get_ttyp ()->ti);

  fhandler_console::input_states stat = input_processing;
  DWORD total_read, i;
  INPUT_RECORD input_rec[INREC_SIZE];

  if (!PeekConsoleInputW (get_handle (), input_rec, INREC_SIZE, &total_read))
    {
      termios_printf ("PeekConsoleInput failed, %E");
      return input_error;
    }

  for (i = 0; i < total_read; i ++)
    {
      DWORD nread = 1;
      const char *toadd = NULL;

      const WCHAR &unicode_char =
	input_rec[i].Event.KeyEvent.uChar.UnicodeChar;
      const DWORD &ctrl_key_state =
	input_rec[i].Event.KeyEvent.dwControlKeyState;

      /* check the event that occurred */
      switch (input_rec[i].EventType)
	{
	case KEY_EVENT:

	  con.nModifiers = 0;

#ifdef DEBUGGING
	  /* allow manual switching to/from raw mode via ctrl-alt-scrolllock */
	  if (input_rec[i].Event.KeyEvent.bKeyDown
	      && input_rec[i].Event.KeyEvent.wVirtualKeyCode == VK_SCROLL
	      && (ctrl_key_state & (LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED))
	      == (LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED))
	    {
	      set_raw_win32_keyboard_mode (!con.raw_win32_keyboard_mode);
	      continue;
	    }
#endif

	  if (con.raw_win32_keyboard_mode)
	    {
	      __small_sprintf (tmp, "\033{%u;%u;%u;%u;%u;%luK",
			       input_rec[i].Event.KeyEvent.bKeyDown,
			       input_rec[i].Event.KeyEvent.wRepeatCount,
			       input_rec[i].Event.KeyEvent.wVirtualKeyCode,
			       input_rec[i].Event.KeyEvent.wVirtualScanCode,
			       input_rec[i].Event.KeyEvent.uChar.UnicodeChar,
			       input_rec[i].Event.KeyEvent.dwControlKeyState);
	      toadd = tmp;
	      nread = strlen (toadd);
	      break;
	    }

	  /* Ignore key up events, except for Alt+Numpad events. */
	  if (!input_rec[i].Event.KeyEvent.bKeyDown &&
	      !is_alt_numpad_event (&input_rec[i]))
	    continue;
	  /* Ignore Alt+Numpad keys.  They are eventually handled below after
	     releasing the Alt key. */
	  if (input_rec[i].Event.KeyEvent.bKeyDown
	      && is_alt_numpad_key (&input_rec[i]))
	    continue;

	  if (ctrl_key_state & SHIFT_PRESSED)
	    con.nModifiers |= 1;
	  if (ctrl_key_state & RIGHT_ALT_PRESSED)
	    con.nModifiers |= 2;
	  if (ctrl_key_state & CTRL_PRESSED)
	    con.nModifiers |= 4;
	  if (ctrl_key_state & LEFT_ALT_PRESSED)
	    con.nModifiers |= 8;

	  /* Allow Backspace to emit ^? and escape sequences. */
	  if (input_rec[i].Event.KeyEvent.wVirtualKeyCode == VK_BACK)
	    {
	      char c = con.backspace_keycode;
	      nread = 0;
	      if (ctrl_key_state & ALT_PRESSED)
		{
		  if (con.metabit)
		    c |= 0x80;
		  else
		    tmp[nread++] = '\e';
		}
	      tmp[nread++] = c;
	      tmp[nread] = 0;
	      toadd = tmp;
	    }
	  /* Allow Ctrl-Space to emit ^@ */
	  else if (input_rec[i].Event.KeyEvent.wVirtualKeyCode
		   == ((wincap.has_con_24bit_colors () && !con_is_legacy) ?
		       '2' : VK_SPACE)
		   && (ctrl_key_state & CTRL_PRESSED)
		   && !(ctrl_key_state & ALT_PRESSED))
	    toadd = "";
	  else if (unicode_char == 0
		   /* arrow/function keys */
		   || (input_rec[i].Event.KeyEvent.dwControlKeyState
		       & ENHANCED_KEY))
	    {
	      toadd = get_nonascii_key (input_rec[i], tmp);
	      if (!toadd)
		{
		  con.nModifiers = 0;
		  continue;
		}
	      nread = strlen (toadd);
	    }
	  else
	    {
	      nread = con.con_to_str (tmp + 1, 59, unicode_char);
	      /* Determine if the keystroke is modified by META.  The tricky
		 part is to distinguish whether the right Alt key should be
		 recognized as Alt, or as AltGr. */
	      bool meta =
		/* Alt but not AltGr (= left ctrl + right alt)? */
		(ctrl_key_state & ALT_PRESSED) != 0
		&& ((ctrl_key_state & CTRL_PRESSED) == 0
		    /* but also allow Alt-AltGr: */
		    || (ctrl_key_state & ALT_PRESSED) == ALT_PRESSED
		    || (unicode_char <= 0x1f || unicode_char == 0x7f));
	      if (!meta)
		{
		  /* Determine if the character is in the current multibyte
		     charset.  The test is easy.  If the multibyte sequence
		     is > 1 and the first byte is ASCII CAN, the character
		     has been translated into the ASCII CAN + UTF-8 replacement
		     sequence.  If so, just ignore the keypress.
		     FIXME: Is there a better solution? */
		  if (nread > 1 && tmp[1] == 0x18)
		    beep ();
		  else
		    toadd = tmp + 1;
		}
	      else if (con.metabit)
		{
		  tmp[1] |= 0x80;
		  toadd = tmp + 1;
		}
	      else
		{
		  tmp[0] = '\033';
		  tmp[1] = cyg_tolower (tmp[1]);
		  toadd = tmp;
		  nread++;
		  con.nModifiers &= ~4;
		}
	    }
	  break;

	case MOUSE_EVENT:
	  send_winch_maybe ();
	    {
	      MOUSE_EVENT_RECORD& mouse_event = input_rec[i].Event.MouseEvent;
	      /* As a unique guard for mouse report generation,
		 call mouse_aware() which is common with select(), so the result
		 of select() and the actual read() will be consistent on the
		 issue of whether input (i.e. a mouse escape sequence) will
		 be available or not */
	      if (mouse_aware (mouse_event))
		{
		  /* Note: Reported mouse position was already retrieved by
		     mouse_aware() and adjusted by window scroll buffer offset */

		  /* Treat the double-click event like a regular button press */
		  if (mouse_event.dwEventFlags == DOUBLE_CLICK)
		    {
		      syscall_printf ("mouse: double-click -> click");
		      mouse_event.dwEventFlags = 0;
		    }

		  /* This code assumes Windows never reports multiple button
		     events at the same time. */
		  int b = 0;
		  char sz[32];
		  char mode6_term = 'M';

		  if (mouse_event.dwEventFlags == MOUSE_WHEELED)
		    {
		      if (mouse_event.dwButtonState & 0xFF800000)
			{
			  b = 0x41;
			  strcpy (sz, "wheel down");
			}
		      else
			{
			  b = 0x40;
			  strcpy (sz, "wheel up");
			}
		    }
		  else
		    {
		      /* Ignore unimportant mouse buttons */
		      mouse_event.dwButtonState &= 0x7;

		      if (mouse_event.dwEventFlags == MOUSE_MOVED)
			{
			  b = con.last_button_code;
			}
		      else if (mouse_event.dwButtonState < con.dwLastButtonState
			       && !con.ext_mouse_mode6)
			{
			  b = 3;
			  strcpy (sz, "btn up");
			}
		      else if ((mouse_event.dwButtonState & 1)
			       != (con.dwLastButtonState & 1))
			{
			  b = 0;
			  strcpy (sz, "btn1 down");
			}
		      else if ((mouse_event.dwButtonState & 2)
			       != (con.dwLastButtonState & 2))
			{
			  b = 2;
			  strcpy (sz, "btn2 down");
			}
		      else if ((mouse_event.dwButtonState & 4)
			       != (con.dwLastButtonState & 4))
			{
			  b = 1;
			  strcpy (sz, "btn3 down");
			}

		      if (con.ext_mouse_mode6 /* distinguish release */
			  && mouse_event.dwButtonState < con.dwLastButtonState)
			mode6_term = 'm';

		      con.last_button_code = b;

		      if (mouse_event.dwEventFlags == MOUSE_MOVED)
			{
			  b += 32;
			  strcpy (sz, "move");
			}
		      else
			{
			  /* Remember the modified button state */
			  con.dwLastButtonState = mouse_event.dwButtonState;
			}
		    }

		  /* Remember mouse position */
		  con.dwLastMousePosition.X = con.dwMousePosition.X;
		  con.dwLastMousePosition.Y = con.dwMousePosition.Y;

		  /* Remember the modifiers */
		  con.nModifiers = 0;
		  if (mouse_event.dwControlKeyState & SHIFT_PRESSED)
		    con.nModifiers |= 0x4;
		  if (mouse_event.dwControlKeyState & ALT_PRESSED)
		    con.nModifiers |= 0x8;
		  if (mouse_event.dwControlKeyState & CTRL_PRESSED)
		    con.nModifiers |= 0x10;

		  /* Indicate the modifiers */
		  b |= con.nModifiers;

		  /* We can now create the code. */
		  if (con.ext_mouse_mode6)
		    {
		      __small_sprintf (tmp, "\033[<%d;%d;%d%c", b,
				       con.dwMousePosition.X + 1,
				       con.dwMousePosition.Y + 1,
				       mode6_term);
		      nread = strlen (tmp);
		    }
		  else if (con.ext_mouse_mode15)
		    {
		      __small_sprintf (tmp, "\033[%d;%d;%dM", b + 32,
				       con.dwMousePosition.X + 1,
				       con.dwMousePosition.Y + 1);
		      nread = strlen (tmp);
		    }
		  else if (con.ext_mouse_mode5)
		    {
		      unsigned int xcode = con.dwMousePosition.X + ' ' + 1;
		      unsigned int ycode = con.dwMousePosition.Y + ' ' + 1;

		      __small_sprintf (tmp, "\033[M%c", b + ' ');
		      nread = 4;
		      /* the neat nested encoding function of mintty
			 does not compile in g++, so let's unfold it: */
		      if (xcode < 0x80)
			tmp [nread++] = xcode;
		      else if (xcode < 0x800)
			{
			  tmp [nread++] = 0xC0 + (xcode >> 6);
			  tmp [nread++] = 0x80 + (xcode & 0x3F);
			}
		      else
			tmp [nread++] = 0;
		      if (ycode < 0x80)
			tmp [nread++] = ycode;
		      else if (ycode < 0x800)
			{
			  tmp [nread++] = 0xC0 + (ycode >> 6);
			  tmp [nread++] = 0x80 + (ycode & 0x3F);
			}
		      else
			tmp [nread++] = 0;
		    }
		  else
		    {
		      unsigned int xcode = con.dwMousePosition.X + ' ' + 1;
		      unsigned int ycode = con.dwMousePosition.Y + ' ' + 1;
		      if (xcode >= 256)
			xcode = 0;
		      if (ycode >= 256)
			ycode = 0;
		      __small_sprintf (tmp, "\033[M%c%c%c", b + ' ',
				       xcode, ycode);
		      nread = 6;	/* tmp may contain NUL bytes */
		    }
		  syscall_printf ("mouse: %s at (%d,%d)", sz,
				  con.dwMousePosition.X,
				  con.dwMousePosition.Y);

		  toadd = tmp;
		}
	    }
	  break;

	case FOCUS_EVENT:
	  if (con.use_focus)
	    {
	      if (input_rec[i].Event.FocusEvent.bSetFocus)
		__small_sprintf (tmp, "\033[I");
	      else
		__small_sprintf (tmp, "\033[O");

	      toadd = tmp;
	      nread = 3;
	    }
	  break;

	case WINDOW_BUFFER_SIZE_EVENT:
	  if (send_winch_maybe ())
	    {
	      stat = input_winch;
	      goto out;
	    }
	  /* fall through */
	default:
	  continue;
	}

      if (toadd)
	{
	  ssize_t ret;
	  line_edit_status res = line_edit (toadd, nread, *ti, &ret);
	  if (res == line_edit_signalled)
	    {
	      stat = input_signalled;
	      goto out;
	    }
	  else if (res == line_edit_input_done)
	    {
	      input_ready = true;
	      stat = input_ok;
	      if (ti->c_lflag & ICANON)
		goto out;
	    }
	}
    }
out:
  /* Discard processed recored. */
  DWORD dummy;
  DWORD discard_len = min (total_read, i + 1);
  if (discard_len)
    ReadConsoleInputW (get_handle (), input_rec, discard_len, &dummy);
  return stat;
}

bool
dev_console::fillin (HANDLE h)
{
  bool ret;

  if ((ret = GetConsoleScreenBufferInfo (h, &b)))
    {
      dwWinSize.Y = 1 + b.srWindow.Bottom - b.srWindow.Top;
      dwWinSize.X = 1 + b.srWindow.Right - b.srWindow.Left;
      if (b.dwCursorPosition.Y > dwEnd.Y
	  || (b.dwCursorPosition.Y >= dwEnd.Y
	      && b.dwCursorPosition.X > dwEnd.X))
	dwEnd = b.dwCursorPosition;
    }
  else
    {
      memset (&b, 0, sizeof (b));
      dwWinSize.Y = 25;
      dwWinSize.X = 80;
      b.srWindow.Bottom = 24;
      b.srWindow.Right = 79;
    }

  return ret;
}

void __reg3
dev_console::scroll_buffer (HANDLE h, int x1, int y1, int x2, int y2,
			    int xn, int yn)
{
/* Scroll the screen context.
   x1, y1 - ul corner
   x2, y2 - dr corner
   xn, yn - new ul corner
   Negative values represents current screen dimensions
*/
  SMALL_RECT sr1, sr2;
  CHAR_INFO fill;
  COORD dest;
  fill.Char.UnicodeChar = L' ';
  fill.Attributes = current_win32_attr;

  fillin (h);
  sr1.Left = x1 >= 0 ? x1 : dwWinSize.X - 1;
  sr1.Top = y1 >= 0 ? y1 : b.srWindow.Bottom;
  sr1.Right = x2 >= 0 ? x2 : dwWinSize.X - 1;
  sr1.Bottom = y2 >= 0 ? y2 : b.srWindow.Bottom;
  sr2.Top = b.srWindow.Top + scroll_region.Top;
  sr2.Left = 0;
  sr2.Bottom = (scroll_region.Bottom < 0) ?
    b.srWindow.Bottom : b.srWindow.Top + scroll_region.Bottom;
  sr2.Right = dwWinSize.X - 1;
  if (sr1.Bottom > sr2.Bottom && sr1.Top <= sr2.Bottom)
    sr1.Bottom = sr2.Bottom;
  dest.X = xn >= 0 ? xn : dwWinSize.X - 1;
  dest.Y = yn >= 0 ? yn : b.srWindow.Bottom;
  ScrollConsoleScreenBufferW (h, &sr1, &sr2, dest, &fill);
}

inline void
fhandler_console::scroll_buffer (int x1, int y1, int x2, int y2,
				 int xn, int yn)
{
  con.scroll_buffer (get_output_handle (), x1, y1, x2, y2, xn, yn);
}

inline void
fhandler_console::scroll_buffer_screen (int x1, int y1, int x2, int y2,
					int xn, int yn)
{
  if (y1 >= 0)
    y1 += con.b.srWindow.Top;
  if (y2 >= 0)
    y2 += con.b.srWindow.Top;
  if (yn >= 0)
    yn += con.b.srWindow.Top;
  con.scroll_buffer (get_output_handle (), x1, y1, x2, y2, xn, yn);
}

int
fhandler_console::dup (fhandler_base *child, int flags)
{
  /* See comments in fhandler_pty_slave::dup */
  if (myself->ctty != -2)
    myself->set_ctty (this, flags);
  return 0;
}

int
fhandler_console::open (int flags, mode_t)
{
  HANDLE h;

  if (dev () == FH_ERROR)
    {
      set_errno (EPERM);	/* constructor found an error */
      return 0;
    }

  tcinit (false);

  set_handle (NULL);
  set_output_handle (NULL);

  /* Open the input handle as handle_ */
  h = CreateFileW (L"CONIN$", GENERIC_READ | GENERIC_WRITE,
		  FILE_SHARE_READ | FILE_SHARE_WRITE, &sec_none,
		  OPEN_EXISTING, 0, 0);

  if (h == INVALID_HANDLE_VALUE)
    {
      __seterrno ();
      return 0;
    }
  set_handle (h);
  handle_set.input_handle = h;

  h = CreateFileW (L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
		  FILE_SHARE_READ | FILE_SHARE_WRITE, &sec_none,
		  OPEN_EXISTING, 0, 0);

  if (h == INVALID_HANDLE_VALUE)
    {
      __seterrno ();
      return 0;
    }
  set_output_handle (h);
  handle_set.output_handle = h;

  setup_io_mutex ();
  handle_set.input_mutex = input_mutex;
  handle_set.output_mutex = output_mutex;

  if (con.fillin (get_output_handle ()))
    {
      con.current_win32_attr = con.b.wAttributes;
      if (!con.default_color)
	con.default_color = con.b.wAttributes;
      con.set_default_attr ();
    }

  set_open_status ();

  if (myself->pid == con.owner && wincap.has_con_24bit_colors ())
    {
      bool is_legacy = false;
      DWORD dwMode;
      /* Check xterm compatible mode in output */
      GetConsoleMode (get_output_handle (), &dwMode);
      if (!SetConsoleMode (get_output_handle (),
			   dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
	is_legacy = true;
      SetConsoleMode (get_output_handle (), dwMode);
      /* Check xterm compatible mode in input */
      GetConsoleMode (get_handle (), &dwMode);
      if (!SetConsoleMode (get_handle (),
			   dwMode | ENABLE_VIRTUAL_TERMINAL_INPUT))
	is_legacy = true;
      SetConsoleMode (get_handle (), dwMode);
      con.is_legacy = is_legacy;
      extern int sawTERM;
      if (con_is_legacy && !sawTERM)
	setenv ("TERM", "cygwin", 1);
    }

  set_input_mode (tty::cygwin, &get_ttyp ()->ti, &handle_set);
  set_output_mode (tty::cygwin, &get_ttyp ()->ti, &handle_set);

  debug_printf ("opened conin$ %p, conout$ %p", get_handle (),
		get_output_handle ());

  if (myself->pid == con.owner)
    {
      char name[MAX_PATH];
      shared_name (name, CONS_THREAD_SYNC, get_minor ());
      thread_sync_event = CreateEvent(NULL, FALSE, FALSE, name);
      new cygthread (::cons_master_thread, this, "consm");
      WaitForSingleObject (thread_sync_event, INFINITE);
      CloseHandle (thread_sync_event);
    }
  return 1;
}

bool
fhandler_console::open_setup (int flags)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  if (myself->set_ctty (this, flags) && !myself->cygstarted)
    init_console_handler (true);
  return fhandler_base::open_setup (flags);
}

int
fhandler_console::close ()
{
  debug_printf ("closing: %p, %p", get_handle (), get_output_handle ());

  acquire_output_mutex (INFINITE);

  if (shared_console_info)
    {
      /* Restore console mode if this is the last closure. */
      OBJECT_BASIC_INFORMATION obi;
      NTSTATUS status;
      status = NtQueryObject (get_handle (), ObjectBasicInformation,
			      &obi, sizeof obi, NULL);
      if ((NT_SUCCESS (status) && obi.HandleCount == 1)
	  || myself->pid == con.owner)
	{
	  set_output_mode (tty::restore, &get_ttyp ()->ti, &handle_set);
	  set_input_mode (tty::restore, &get_ttyp ()->ti, &handle_set);
	}
    }

  release_output_mutex ();

  if (shared_console_info && con.owner == myself->pid
      && master_thread_started)
    {
      char name[MAX_PATH];
      shared_name (name, CONS_THREAD_SYNC, get_minor ());
      thread_sync_event = OpenEvent (MAXIMUM_ALLOWED, FALSE, name);
      con.owner = MAX_PID + 1;
      WaitForSingleObject (thread_sync_event, INFINITE);
      CloseHandle (thread_sync_event);
      con.owner = 0;
    }

  CloseHandle (input_mutex);
  input_mutex = NULL;
  CloseHandle (output_mutex);
  output_mutex = NULL;

  CloseHandle (get_handle ());
  CloseHandle (get_output_handle ());

  if (con_ra.rabuf)
    free (con_ra.rabuf);

  if (!have_execed)
    free_console ();
  return 0;
}

int
fhandler_console::ioctl (unsigned int cmd, void *arg)
{
  int res = fhandler_termios::ioctl (cmd, arg);
  if (res <= 0)
    return res;
  acquire_output_mutex (INFINITE);
  switch (cmd)
    {
      case TIOCGWINSZ:
	int st;

	acquire_attach_mutex (INFINITE);
	st = con.fillin (get_output_handle ());
	release_attach_mutex ();
	if (st)
	  {
	    /* *not* the buffer size, the actual screen size... */
	    /* based on Left Top Right Bottom of srWindow */
	    ((struct winsize *) arg)->ws_row = con.dwWinSize.Y;
	    ((struct winsize *) arg)->ws_col = con.dwWinSize.X;
	    syscall_printf ("WINSZ: (row=%d,col=%d)",
			   ((struct winsize *) arg)->ws_row,
			   ((struct winsize *) arg)->ws_col);
	    release_output_mutex ();
	    return 0;
	  }
	else
	  {
	    syscall_printf ("WINSZ failed");
	    __seterrno ();
	    release_output_mutex ();
	    return -1;
	  }
	release_output_mutex ();
	return 0;
      case TIOCSWINSZ:
	bg_check (SIGTTOU);
	release_output_mutex ();
	return 0;
      case KDGKBMETA:
	*(int *) arg = (con.metabit) ? K_METABIT : K_ESCPREFIX;
	release_output_mutex ();
	return 0;
      case KDSKBMETA:
	if ((intptr_t) arg == K_METABIT)
	  con.metabit = TRUE;
	else if ((intptr_t) arg == K_ESCPREFIX)
	  con.metabit = FALSE;
	else
	  {
	    set_errno (EINVAL);
	    release_output_mutex ();
	    return -1;
	  }
	release_output_mutex ();
	return 0;
      case TIOCLINUX:
	if (*(unsigned char *) arg == 6)
	  {
	    *(unsigned char *) arg = (unsigned char) con.nModifiers;
	    release_output_mutex ();
	    return 0;
	  }
	set_errno (EINVAL);
	release_output_mutex ();
	return -1;
      case FIONREAD:
	{
	  DWORD n;
	  int ret = 0;
	  INPUT_RECORD inp[INREC_SIZE];
	  acquire_attach_mutex (INFINITE);
	  if (!PeekConsoleInputW (get_handle (), inp, INREC_SIZE, &n))
	    {
	      set_errno (EINVAL);
	      release_attach_mutex ();
	      release_output_mutex ();
	      return -1;
	    }
	  release_attach_mutex ();
	  bool saw_eol = false;
	  for (DWORD i=0; i<n; i++)
	    if (inp[i].EventType == KEY_EVENT &&
		inp[i].Event.KeyEvent.bKeyDown &&
		inp[i].Event.KeyEvent.uChar.UnicodeChar)
	      {
		WCHAR wc = inp[i].Event.KeyEvent.uChar.UnicodeChar;
		char mbs[8];
		int len = con.con_to_str (mbs, sizeof (mbs), wc);
		if ((get_ttyp ()->ti.c_lflag & ICANON) &&
		    len == 1 && CCEQ (get_ttyp ()->ti.c_cc[VEOF], mbs[0]))
		  {
		    saw_eol = true;
		    break;
		  }
		ret += len;
		const char eols[] = {
		  '\n',
		  '\r',
		  (char) get_ttyp ()->ti.c_cc[VEOL],
		  (char) get_ttyp ()->ti.c_cc[VEOL2]
		};
		if ((get_ttyp ()->ti.c_lflag & ICANON) &&
		    len == 1 && memchr (eols, mbs[0], sizeof (eols)))
		  {
		    saw_eol = true;
		    break;
		  }
	      }
	  if ((get_ttyp ()->ti.c_lflag & ICANON) && !saw_eol)
	    *(int *) arg = 0;
	  else
	    *(int *) arg = ret;
	  release_output_mutex ();
	  return 0;
	}
	break;
    }

  release_output_mutex ();
  return fhandler_base::ioctl (cmd, arg);
}

int
fhandler_console::tcflush (int queue)
{
  int res = 0;
  if (queue == TCIFLUSH
      || queue == TCIOFLUSH)
    {
      acquire_attach_mutex (INFINITE);
      if (!FlushConsoleInputBuffer (get_handle ()))
	{
	  __seterrno ();
	  res = -1;
	}
      release_attach_mutex ();
    }
  return res;
}

int
fhandler_console::tcsetattr (int a, struct termios const *t)
{
  get_ttyp ()->ti = *t;
  return 0;
}

int
fhandler_console::tcgetattr (struct termios *t)
{
  *t = get_ttyp ()->ti;
  t->c_cflag |= CS8;
  return 0;
}

fhandler_console::fhandler_console (fh_devices unit) :
  fhandler_termios (), input_ready (false), thread_sync_event (NULL),
  input_mutex (NULL), output_mutex (NULL)
{
  if (unit > 0)
    dev ().parse (unit);
  setup ();
  trunc_buf.len = 0;
  _tc = &(shared_console_info->tty_min_state);
}

void
dev_console::set_color (HANDLE h)
{
  WORD win_fg = fg;
  WORD win_bg = bg;
  if (reverse)
    {
      WORD save_fg = win_fg;
      win_fg = (win_bg & BACKGROUND_RED   ? FOREGROUND_RED   : 0) |
	       (win_bg & BACKGROUND_GREEN ? FOREGROUND_GREEN : 0) |
	       (win_bg & BACKGROUND_BLUE  ? FOREGROUND_BLUE  : 0) |
	       (win_bg & BACKGROUND_INTENSITY ? FOREGROUND_INTENSITY : 0);
      win_bg = (save_fg & FOREGROUND_RED   ? BACKGROUND_RED   : 0) |
	       (save_fg & FOREGROUND_GREEN ? BACKGROUND_GREEN : 0) |
	       (save_fg & FOREGROUND_BLUE  ? BACKGROUND_BLUE  : 0) |
	       (save_fg & FOREGROUND_INTENSITY ? BACKGROUND_INTENSITY : 0);
    }

  /* apply attributes */
  if (underline)
    win_fg = underline_color;
  /* emulate blink with bright background */
  if (blink)
    win_bg |= BACKGROUND_INTENSITY;
  if (intensity == INTENSITY_INVISIBLE)
    win_fg = win_bg;
  else if (intensity != INTENSITY_BOLD)
    /* nothing to do */;
    /* apply foreground intensity only in non-reverse mode! */
  else if (reverse)
    win_bg |= BACKGROUND_INTENSITY;
  else
    win_fg |= FOREGROUND_INTENSITY;

  current_win32_attr = win_fg | win_bg;
  if (h)
    SetConsoleTextAttribute (h, current_win32_attr);
}

#define FOREGROUND_ATTR_MASK (FOREGROUND_RED | FOREGROUND_GREEN | \
			      FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define BACKGROUND_ATTR_MASK (BACKGROUND_RED | BACKGROUND_GREEN | \
			      BACKGROUND_BLUE | BACKGROUND_INTENSITY)
void
dev_console::set_default_attr ()
{
  blink = underline = reverse = false;
  intensity = INTENSITY_NORMAL;
  fg = default_color & FOREGROUND_ATTR_MASK;
  bg = default_color & BACKGROUND_ATTR_MASK;
  set_color (NULL);
}

int
dev_console::set_cl_x (cltype x)
{
  if (x == cl_disp_beg || x == cl_buf_beg)
    return 0;
  if (x == cl_disp_end)
    return dwWinSize.X - 1;
  if (x == cl_buf_end)
    return b.dwSize.X - 1;
  return b.dwCursorPosition.X;
}

int
dev_console::set_cl_y (cltype y)
{
  if (y == cl_buf_beg)
    return 0;
  if (y == cl_disp_beg)
    return b.srWindow.Top;
  if (y == cl_disp_end)
    return b.srWindow.Bottom;
  if (y == cl_buf_end)
    return b.dwSize.Y - 1;
  return b.dwCursorPosition.Y;
}

bool
dev_console::scroll_window (HANDLE h, int x1, int y1, int x2, int y2)
{
  if (save_buf || x1 != 0 || x2 != dwWinSize.X - 1 || y1 != b.srWindow.Top
      || y2 != b.srWindow.Bottom || b.dwSize.Y <= dwWinSize.Y)
    return false;

  SMALL_RECT sr;
  int toscroll = dwEnd.Y - b.srWindow.Top + 1;
  sr.Left = sr.Right = dwEnd.X = 0;

  if (b.srWindow.Bottom + toscroll >= b.dwSize.Y)
    {
      /* So we're at the end of the buffer and scrolling the console window
	 would move us beyond the buffer.  What we do here is to scroll the
	 console buffer upward by just as much so that the current last line
	 becomes the last line just prior to the first window line.  That
	 keeps the end of the console buffer intact, as desired. */
      SMALL_RECT br;
      COORD dest;
      CHAR_INFO fill;

      br.Left = 0;
      br.Top = (b.srWindow.Bottom - b.srWindow.Top) + 1
	       - (b.dwSize.Y - dwEnd.Y - 1);
      br.Right = b.dwSize.X - 1;
      br.Bottom = b.dwSize.Y - 1;
      dest.X = dest.Y = 0;
      fill.Char.UnicodeChar = L' ';
      fill.Attributes = current_win32_attr;
      ScrollConsoleScreenBufferW (h, &br, NULL, dest, &fill);
      /* Since we're moving the console buffer under the console window
	 we only have to move the console window if the user scrolled the
	 window upwards.  The number of lines is the distance to the
	 buffer bottom. */
      toscroll = b.dwSize.Y - b.srWindow.Bottom - 1;
      /* Fix dwEnd to reflect the new cursor line.  Take the above scrolling
	 into account and subtract 1 to account for the increment below. */
      dwEnd.Y = b.dwCursorPosition.Y + toscroll - 1;
    }
  if (toscroll)
    {
      /* FIXME: For some reason SetConsoleWindowInfo does not correctly
	 set the scrollbars.  Calling SetConsoleCursorPosition here is
	 just a workaround which doesn't cover all cases.  In some scenarios
	 the scrollbars are still off by one console window size. */

      /* The reminder of the console buffer is big enough to simply move
         the console window.  We have to set the cursor first, otherwise
	 the scroll bars will not be corrected.  */
      SetConsoleCursorPosition (h, dwEnd);
      /* If the user scolled manually, setting the cursor position might scroll
         the console window so that the cursor is not at the top.  Correct
	 the action by moving the window down again so the cursor is one line
	 above the new window position. */
      GetConsoleScreenBufferInfo (h, &b);
      if (b.dwCursorPosition.Y >= b.srWindow.Top)
	toscroll = b.dwCursorPosition.Y - b.srWindow.Top + 1;
      /* Move the window accordingly. */
      sr.Top = sr.Bottom = toscroll;
      SetConsoleWindowInfo (h, FALSE, &sr);
    }
  /* Eventually set cursor to new end position at the top of the window. */
  dwEnd.Y++;
  SetConsoleCursorPosition (h, dwEnd);
  /* Fix up console buffer info. */
  fillin (h);
  return true;
}

/*
 * Clear the screen context from x1/y1 to x2/y2 cell.
 * Negative values represents current screen dimensions
 */
void __reg3
fhandler_console::clear_screen (cltype xc1, cltype yc1, cltype xc2, cltype yc2)
{
  HANDLE h = get_output_handle ();
  SHORT oldEndY = con.dwEnd.Y;

  con.fillin (h);

  int x1 = con.set_cl_x (xc1);
  int y1 = con.set_cl_y (yc1);
  int x2 = con.set_cl_x (xc2);
  int y2 = con.set_cl_y (yc2);

  /* Make correction for the following situation:  The console buffer
     is only partially used and the user scrolled down into the as yet
     unused area so far that the cursor is outside the window buffer. */
  if (oldEndY < con.dwEnd.Y && oldEndY < con.b.srWindow.Top)
    {
      con.dwEnd.Y = con.b.dwCursorPosition.Y = oldEndY;
      y1 = con.b.srWindow.Top;
    }

  /* Detect special case - scroll the screen if we have a buffer in order to
     preserve the buffer. */
  if (!con.scroll_window (h, x1, y1, x2, y2))
    con.clear_screen (h, x1, y1, x2, y2);
}

void __reg3
dev_console::clear_screen (HANDLE h, int x1, int y1, int x2, int y2)
{
  COORD tlc;
  DWORD done;
  int num;

  num = abs (y1 - y2) * b.dwSize.X + abs (x1 - x2) + 1;

  if ((y2 * b.dwSize.X + x2) > (y1 * b.dwSize.X + x1))
    {
      tlc.X = x1;
      tlc.Y = y1;
    }
  else
    {
      tlc.X = x2;
      tlc.Y = y2;
    }
  FillConsoleOutputCharacterW (h, L' ', num, tlc, &done);
  FillConsoleOutputAttribute (h, current_win32_attr, num, tlc, &done);
}

void __reg3
fhandler_console::cursor_set (bool rel_to_top, int x, int y)
{
  COORD pos;

  con.fillin (get_output_handle ());
#if 0
  /* Setting y to the current b.srWindow.Bottom here is the reason that the window
     isn't scrolled back to the current cursor position like it's done in
     any other terminal.  Rather, the curser is forced to the bottom of the
     currently scrolled region.  This breaks the console buffer content if
     output is generated while the user had the window scrolled back.  This
     behaviour is very old, it has no matching ChangeLog entry.
     Just disable for now but keep the code in for future reference. */
  if (y > con.b.srWindow.Bottom)
    y = con.b.srWindow.Bottom;
  else
#endif
  if (y < 0)
    y = 0;
  else if (rel_to_top)
    y += con.b.srWindow.Top;

  if (x > con.dwWinSize.X)
    x = con.dwWinSize.X - 1;
  else if (x < 0)
    x = 0;

  pos.X = x;
  pos.Y = y;
  SetConsoleCursorPosition (get_output_handle (), pos);
}

void __reg3
fhandler_console::cursor_rel (int x, int y)
{
  con.fillin (get_output_handle ());
  x += con.b.dwCursorPosition.X;
  y += con.b.dwCursorPosition.Y;
  cursor_set (false, x, y);
}

void __reg3
fhandler_console::cursor_get (int *x, int *y)
{
  con.fillin (get_output_handle ());
  *y = con.b.dwCursorPosition.Y;
  *x = con.b.dwCursorPosition.X;
}

/* VT100 line drawing graphics mode maps `abcdefghijklmnopqrstuvwxyz{|}~ to
   graphical characters */
static const wchar_t __vt100_conv[31] = {
	0x25C6, /* Black Diamond */
	0x2592, /* Medium Shade */
	0x2409, /* Symbol for Horizontal Tabulation */
	0x240C, /* Symbol for Form Feed */
	0x240D, /* Symbol for Carriage Return */
	0x240A, /* Symbol for Line Feed */
	0x00B0, /* Degree Sign */
	0x00B1, /* Plus-Minus Sign */
	0x2424, /* Symbol for Newline */
	0x240B, /* Symbol for Vertical Tabulation */
	0x2518, /* Box Drawings Light Up And Left */
	0x2510, /* Box Drawings Light Down And Left */
	0x250C, /* Box Drawings Light Down And Right */
	0x2514, /* Box Drawings Light Up And Right */
	0x253C, /* Box Drawings Light Vertical And Horizontal */
	0x23BA, /* Horizontal Scan Line-1 */
	0x23BB, /* Horizontal Scan Line-3 */
	0x2500, /* Box Drawings Light Horizontal */
	0x23BC, /* Horizontal Scan Line-7 */
	0x23BD, /* Horizontal Scan Line-9 */
	0x251C, /* Box Drawings Light Vertical And Right */
	0x2524, /* Box Drawings Light Vertical And Left */
	0x2534, /* Box Drawings Light Up And Horizontal */
	0x252C, /* Box Drawings Light Down And Horizontal */
	0x2502, /* Box Drawings Light Vertical */
	0x2264, /* Less-Than Or Equal To */
	0x2265, /* Greater-Than Or Equal To */
	0x03C0, /* Greek Small Letter Pi */
	0x2260, /* Not Equal To */
	0x00A3, /* Pound Sign */
	0x00B7, /* Middle Dot */
};

inline bool
fhandler_console::write_console (PWCHAR buf, DWORD len, DWORD& done)
{
  if (con.iso_2022_G1
	? con.vt100_graphics_mode_G1
	: con.vt100_graphics_mode_G0)
    for (DWORD i = 0; i < len; i ++)
      if (buf[i] >= (unsigned char) '`' && buf[i] <= (unsigned char) '~')
	buf[i] = __vt100_conv[buf[i] - (unsigned char) '`'];

  if (len > 0)
    last_char = buf[len-1];

  while (len > 0)
    {
      DWORD nbytes = len > MAX_WRITE_CHARS ? MAX_WRITE_CHARS : len;
      if (!WriteConsoleW (get_output_handle (), buf, nbytes, &done, 0))
	{
	  __seterrno ();
	  return false;
	}
      len -= done;
      buf += done;
    }
  return true;
}

/* The following three functions were adapted (i.e., mildly modified) from
   http://stackoverflow.com/questions/14699043/replacement-to-systemcolor */

/* Split a rectangular region into two smaller rectangles based on the
   largest dimension. */
static void
region_split (PCHAR_INFO& buf, COORD& bufsiz, SMALL_RECT& region,
	      PCHAR_INFO& buf_b, COORD& bufsiz_b, SMALL_RECT& region_b)
{
  region_b = region;
  bufsiz_b = bufsiz;

  SHORT half = (1 + region.Bottom - region.Top) / 2;
  region_b.Top += half;
  region.Bottom = (bufsiz.Y = region_b.Top) - 1;
  buf_b = buf + (half * (1 + region.Right));
  bufsiz_b.Y = region_b.Bottom - region_b.Top;
}

/* Utility function to figure out the distance between two points. */
static SHORT
delta (SHORT first, SHORT second)
{
  return (second >= first) ? (second - first + 1) : 0;
}

/* Subdivide the ReadConsoleInput operation into smaller and smaller chunks as
   needed until it succeeds in reading the entire screen buffer. */
static BOOL
ReadConsoleOutputWrapper (HANDLE h, PCHAR_INFO buf, COORD bufsiz,
			  SMALL_RECT region)
{
  COORD coord = {};
  SHORT width = delta (region.Left, region.Right);
  SHORT height = delta (region.Top, region.Bottom);

  if ((width == 0) || (height == 0))
    return TRUE;

  BOOL success = ReadConsoleOutputW (h, buf, bufsiz, coord, &region);
  if (success)
    /* it worked */;
  else if (GetLastError () == ERROR_NOT_ENOUGH_MEMORY && (width * height) > 1)
    {
      PCHAR_INFO buf_b;
      COORD bufsiz_b;
      SMALL_RECT region_b;
      region_split (buf, bufsiz, region, buf_b, bufsiz_b, region_b);
      success = ReadConsoleOutputWrapper (h, buf, bufsiz, region)
		&& ReadConsoleOutputWrapper (h, buf_b, bufsiz_b, region_b);
    }
  return success;
}

void
dev_console::save_restore (HANDLE h, char c)
{
  if (c == 'h') /* save */
    {
      fillin (h);
      save_bufsize.X = b.dwSize.X;
      if ((save_bufsize.Y = dwEnd.Y + 1) > b.dwSize.Y)
	save_bufsize.X = b.dwSize.Y;

      if (save_buf)
	cfree (save_buf);
      size_t screen_size = sizeof (CHAR_INFO) * save_bufsize.X * save_bufsize.Y;
      save_buf = (PCHAR_INFO) cmalloc_abort (HEAP_1_BUF, screen_size);

      save_cursor = b.dwCursorPosition;	/* Remember where we were. */
      save_top = b.srWindow.Top;

      SMALL_RECT now = {};		/* Read the whole buffer */
      now.Bottom = save_bufsize.Y - 1;
      now.Right = save_bufsize.X - 1;
      if (!ReadConsoleOutputWrapper (h, save_buf, save_bufsize, now))
	debug_printf ("ReadConsoleOutputWrapper(h, ...) failed during save, %E");

      /* Position at top of buffer */
      COORD cob = {};
      if (!SetConsoleCursorPosition (h, cob))
	debug_printf ("SetConsoleCursorInfo(%p, ...) failed during save, %E", h);

      /* Clear entire buffer */
      clear_screen (h, 0, 0, now.Right, now.Bottom);
      b.dwCursorPosition.X = b.dwCursorPosition.Y = dwEnd.X = dwEnd.Y = 0;
    }
  else if (save_buf)
    {
      COORD cob = {};
      SMALL_RECT now = {};
      now.Bottom = save_bufsize.Y - 1;
      now.Right = save_bufsize.X - 1;
      /* Restore whole buffer */
      clear_screen (h, 0, 0, b.dwSize.X - 1, b.dwSize.Y - 1);
      BOOL res = WriteConsoleOutputW (h, save_buf, save_bufsize, cob, &now);
      if (!res)
	debug_printf ("WriteConsoleOutputW failed, %E");

      cfree (save_buf);
      save_buf = NULL;

      cob.X = 0;
      cob.Y = save_top;
      /* CGF: NOOP?  Doesn't seem to position screen as expected */
      /* Temporarily position at top of screen */
      if (!SetConsoleCursorPosition (h, cob))
	debug_printf ("SetConsoleCursorInfo(%p, cob) failed during restore, %E", h);
      /* Position where we were previously */
      if (!SetConsoleCursorPosition (h, save_cursor))
	debug_printf ("SetConsoleCursorInfo(%p, save_cursor) failed during restore, %E", h);
      /* Get back correct version of buffer information */
      dwEnd.X = dwEnd.Y = 0;
      fillin (h);
    }
}

#define BAK 1
#define ESC 2
#define NOR 0
#define IGN 4
#if 1
#define ERR 5
#else
#define ERR NOR
#endif
#define DWN 6
#define BEL 7
#define TAB 8 /* We should't let the console deal with these */
#define CR 13
#define LF 10
#define SO 14
#define SI 15

static const char base_chars[256] =
{
/*00 01 02 03 04 05 06 07 */ IGN, ERR, ERR, NOR, NOR, NOR, NOR, BEL,
/*08 09 0A 0B 0C 0D 0E 0F */ BAK, TAB, DWN, ERR, ERR, CR,  SO,  SI,
/*10 11 12 13 14 15 16 17 */ NOR, NOR, ERR, ERR, ERR, ERR, ERR, ERR,
/*18 19 1A 1B 1C 1D 1E 1F */ NOR, NOR, ERR, ESC, ERR, ERR, ERR, ERR,
/*   !  "  #  $  %  &  '  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
/*(  )  *  +  ,  -  .  /  */ NOR, NOR, NOR, NOR, NOR, NOR, NOR, NOR,
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

static const char table256[256] =
{
   0, 4, 2, 6, 1, 5, 3, 7, 8,12,10,14, 9,13,11,15,
   0, 1, 1, 1, 9, 9, 2, 3, 3, 3, 3, 9, 2, 3, 3, 3,
   3,11, 2, 3, 3, 3,11,11,10, 3, 3,11,11,11,10,10,
  11,11,11,11, 4, 5, 5, 5, 5, 9, 6, 8, 8, 8, 8, 9,
   6, 8, 8, 8, 8, 7, 6, 8, 8, 8, 7, 7, 6, 8, 8, 7,
   7,11,10,10, 7, 7,11,11, 4, 5, 5, 5, 5,13, 6, 8,
   8, 8, 8, 7, 6, 8, 8, 8, 7, 7, 6, 8, 8, 7, 7, 7,
   6, 8, 7, 7, 7, 7,14, 7, 7, 7, 7, 7, 4, 5, 5, 5,
  13,13, 6, 8, 8, 8, 7, 7, 6, 8, 8, 7, 7, 7, 6, 8,
   7, 7, 7, 7,14, 7, 7, 7, 7, 7,14, 7, 7, 7, 7,15,
  12, 5, 5,13,13,13, 6, 8, 8, 7, 7,13, 6, 8, 7, 7,
   7, 7,14, 7, 7, 7, 7, 7,14, 7, 7, 7, 7,15,14,14,
   7, 7,15,15,12,12,13,13,13,13,12,12, 7, 7,13,13,
  14, 7, 7, 7, 7, 7,14, 7, 7, 7, 7,15,14,14, 7, 7,
  15,15,14,14, 7,15,15,15, 0, 0, 0, 0, 0, 0, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7,15,15
};

void
fhandler_console::char_command (char c)
{
  int x, y, n;
  char buf[40];
  wchar_t bufw[40];
  int r, g, b;

  if (wincap.has_con_24bit_colors () && !con_is_legacy)
    {
      /* For xterm compatible mode */
      switch (c)
	{
#if 0 /* These sequences, which are supported by real xterm, are
	 not supported by xterm compatible mode. Therefore they
	 were implemented once. However, these are not declared
	 in terminfo of xterm-256color, therefore, do not appear
	 to be necessary. */
	case '`': /* HPA */
	  if (con.args[0] == 0)
	    con.args[0] = 1;
	  cursor_get (&x, &y);
	  cursor_set (false, con.args[0]-1, y);
	  break;
	case 'a': /* HPR */
	  if (con.args[0] == 0)
	    con.args[0] = 1;
	  cursor_rel (con.args[0], 0);
	  break;
	case 'e': /* VPR */
	  if (con.args[0] == 0)
	    con.args[0] = 1;
	  cursor_rel (0, con.args[0]);
	  break;
#endif
	case 'b': /* REP */
	  wpbuf.put (c);
	  if (wincap.has_con_esc_rep ())
	    /* Just send the sequence */
	    wpbuf.send (get_output_handle ());
	  else if (last_char && last_char != L'\n')
	    for (int i = 0; i < con.args[0]; i++)
	      WriteConsoleW (get_output_handle (), &last_char, 1, 0, 0);
	  break;
	case 'r': /* DECSTBM */
	  con.scroll_region.Top = con.args[0] ? con.args[0] - 1 : 0;
	  con.scroll_region.Bottom = con.args[1] ? con.args[1] - 1 : -1;
	  wpbuf.put (c);
	  /* Just send the sequence */
	  wpbuf.send (get_output_handle ());
	  break;
	case 'L': /* IL */
	  if (wincap.has_con_broken_il_dl ())
	    {
	      /* Use "CSI Ps T" instead */
	      cursor_get (&x, &y);
	      if (y < srTop || y > srBottom)
		break;
	      if (y == con.b.srWindow.Top
		  && srBottom == con.b.srWindow.Bottom)
		{
		  /* Erase scroll down area */
		  n = con.args[0] ? : 1;
		  __small_swprintf (bufw, L"\033[%d;1H\033[J\033[%d;%dH",
				    srBottom - (n-1) - con.b.srWindow.Top + 1,
				    y + 1 - con.b.srWindow.Top, x + 1);
		  WriteConsoleW (get_output_handle (),
				 bufw, wcslen (bufw), 0, 0);
		}
	      __small_swprintf (bufw, L"\033[%d;%dr",
				y + 1 - con.b.srWindow.Top,
				srBottom + 1 - con.b.srWindow.Top);
	      WriteConsoleW (get_output_handle (), bufw, wcslen (bufw), 0, 0);
	      wpbuf.put ('T');
	      wpbuf.send (get_output_handle ());
	      __small_swprintf (bufw, L"\033[%d;%dr",
				srTop + 1 - con.b.srWindow.Top,
				srBottom + 1 - con.b.srWindow.Top);
	      WriteConsoleW (get_output_handle (), bufw, wcslen (bufw), 0, 0);
	      __small_swprintf (bufw, L"\033[%d;%dH",
				y + 1 - con.b.srWindow.Top, x + 1);
	      WriteConsoleW (get_output_handle (), bufw, wcslen (bufw), 0, 0);
	    }
	  else
	    {
	      wpbuf.put (c);
	      /* Just send the sequence */
	      wpbuf.send (get_output_handle ());
	    }
	  break;
	case 'M': /* DL */
	  if (wincap.has_con_broken_il_dl ())
	    {
	      /* Use "CSI Ps S" instead */
	      cursor_get (&x, &y);
	      if (y < srTop || y > srBottom)
		break;
	      __small_swprintf (bufw, L"\033[%d;%dr",
				y + 1 - con.b.srWindow.Top,
				srBottom + 1 - con.b.srWindow.Top);
	      WriteConsoleW (get_output_handle (), bufw, wcslen (bufw), 0, 0);
	      wpbuf.put ('S');
	      wpbuf.send (get_output_handle ());
	      __small_swprintf (bufw, L"\033[%d;%dr",
				srTop + 1 - con.b.srWindow.Top,
				srBottom + 1 - con.b.srWindow.Top);
	      WriteConsoleW (get_output_handle (), bufw, wcslen (bufw), 0, 0);
	      __small_swprintf (bufw, L"\033[%d;%dH",
				y + 1 - con.b.srWindow.Top, x + 1);
	      WriteConsoleW (get_output_handle (), bufw, wcslen (bufw), 0, 0);
	    }
	  else
	    {
	      wpbuf.put (c);
	      /* Just send the sequence */
	      wpbuf.send (get_output_handle ());
	    }
	  break;
	case 'J': /* ED */
	  wpbuf.put (c);
	  if (con.args[0] == 3 && con.savey >= 0)
	    {
	      con.fillin (get_output_handle ());
	      con.savey -= con.b.srWindow.Top;
	    }
	  if (con.args[0] == 3 && wincap.has_con_broken_csi3j ())
	    { /* Workaround for broken CSI3J in Win10 1809 */
	      CONSOLE_SCREEN_BUFFER_INFO sbi;
	      GetConsoleScreenBufferInfo (get_output_handle (), &sbi);
	      SMALL_RECT r = {0, sbi.srWindow.Top,
		(SHORT) (sbi.dwSize.X - 1), (SHORT) (sbi.dwSize.Y - 1)};
	      CHAR_INFO f = {' ', sbi.wAttributes};
	      COORD d = {0, 0};
	      ScrollConsoleScreenBufferA (get_output_handle (),
					  &r, NULL, d, &f);
	      SetConsoleCursorPosition (get_output_handle (), d);
	      d = sbi.dwCursorPosition;
	      d.Y -= sbi.srWindow.Top;
	      SetConsoleCursorPosition (get_output_handle (), d);
	    }
	  else
	    /* Just send the sequence */
	    wpbuf.send (get_output_handle ());
	  break;
	case 'h': /* DECSET */
	case 'l': /* DECRST */
	  wpbuf.put (c);
	  /* Just send the sequence */
	  wpbuf.send (get_output_handle ());
	  if (con.saw_question_mark)
	    {
	      bool need_fix_tab_position = false;
	      for (int i = 0; i < con.nargs; i++)
		{
		  if (con.args[i] == 1049)
		    {
		      con.screen_alternated = (c == 'h');
		      need_fix_tab_position = true;
		    }
		  if (con.args[i] == 1) /* DECCKM */
		    con.cursor_key_app_mode = (c == 'h');
		}
	      /* Call fix_tab_position() if screen has been alternated. */
	      if (need_fix_tab_position)
		fix_tab_position ();
	    }
	  break;
	case 'p':
	  if (con.saw_exclamation_mark) /* DECSTR Soft reset */
	    {
	      con.scroll_region.Top = 0;
	      con.scroll_region.Bottom = -1;
	      con.savex = con.savey = -1;
	      con.cursor_key_app_mode = false;
	    }
	  wpbuf.put (c);
	  /* Just send the sequence */
	  wpbuf.send (get_output_handle ());
	  break;
	case 'm':
	  if (con.saw_greater_than_sign)
	    break; /* Ignore unsupported CSI > Pm m */
	  /* Text attribute settings */
	  wpbuf.put (c);
	  /* Just send the sequence */
	  wpbuf.send (get_output_handle ());
	  break;
	default:
	  /* Other escape sequences */
	  wpbuf.put (c);
	  /* Just send the sequence */
	  wpbuf.send (get_output_handle ());
	  break;
	}
      return;
    }

  /* For legacy cygwin treminal */
  switch (c)
    {
    case 'm':   /* Set Graphics Rendition */
       for (int i = 0; i < con.nargs; i++)
	 switch (con.args[i])
	   {
	     case 0:    /* normal color */
	       con.set_default_attr ();
	       break;
	     case 1:    /* bold */
	       con.intensity = INTENSITY_BOLD;
	       break;
	     case 2:	/* dim */
	       con.intensity = INTENSITY_DIM;
	       break;
	     case 4:	/* underlined */
	       con.underline = 1;
	       break;
	     case 5:    /* blink mode */
	       con.blink = true;
	       break;
	     case 7:    /* reverse */
	       con.reverse = true;
	       break;
	     case 8:    /* invisible */
	       con.intensity = INTENSITY_INVISIBLE;
	       break;
	     case 10:   /* end alternate charset */
	       con.alternate_charset_active = false;
	       break;
	     case 11:   /* start alternate charset */
	       con.alternate_charset_active = true;
	       break;
	     case 22:
	     case 28:
	       con.intensity = INTENSITY_NORMAL;
	       break;
	     case 24:
	       con.underline = false;
	       break;
	     case 25:
	       con.blink = false;
	       break;
	     case 27:
	       con.reverse = false;
	       break;
	     case 30:		/* BLACK foreground */
	       con.fg = 0;
	       break;
	     case 31:		/* RED foreground */
	       con.fg = FOREGROUND_RED;
	       break;
	     case 32:		/* GREEN foreground */
	       con.fg = FOREGROUND_GREEN;
	       break;
	     case 33:		/* YELLOW foreground */
	       con.fg = FOREGROUND_RED | FOREGROUND_GREEN;
	       break;
	     case 34:		/* BLUE foreground */
	       con.fg = FOREGROUND_BLUE;
	       break;
	     case 35:		/* MAGENTA foreground */
	       con.fg = FOREGROUND_RED | FOREGROUND_BLUE;
	       break;
	     case 36:		/* CYAN foreground */
	       con.fg = FOREGROUND_BLUE | FOREGROUND_GREEN;
	       break;
	     case 37:		/* WHITE foreg */
	       con.fg = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
	       break;
	     case 38:
	       if (con.nargs < i + 2)
		 /* Sequence error (abort) */
		 break;
	       switch (con.args[i + 1])
		 {
		 case 2:
		   if (con.nargs < i + 5)
		     /* Sequence error (abort) */
		     break;
		   r = con.args[i + 2];
		   g = con.args[i + 3];
		   b = con.args[i + 4];
		   r = r < (95 + 1) / 2 ? 0 : r > 255 ? 5 : (r - 55 + 20) / 40;
		   g = g < (95 + 1) / 2 ? 0 : g > 255 ? 5 : (g - 55 + 20) / 40;
		   b = b < (95 + 1) / 2 ? 0 : b > 255 ? 5 : (b - 55 + 20) / 40;
		   con.fg = table256[16 + r*36 + g*6 + b];
		   i += 4;
		   break;
		 case 5:
		   if (con.nargs < i + 3)
		     /* Sequence error (abort) */
		     break;
		   {
		     int idx = con.args[i + 2];
		     if (idx < 0)
		       idx = 0;
		     if (idx > 255)
		       idx = 255;
		     con.fg = table256[idx];
		     i += 2;
		   }
		   break;
		 }
	       break;
	     case 39:
	       con.fg = con.default_color & FOREGROUND_ATTR_MASK;
	       break;
	     case 40:		/* BLACK background */
	       con.bg = 0;
	       break;
	     case 41:		/* RED background */
	       con.bg = BACKGROUND_RED;
	       break;
	     case 42:		/* GREEN background */
	       con.bg = BACKGROUND_GREEN;
	       break;
	     case 43:		/* YELLOW background */
	       con.bg = BACKGROUND_RED | BACKGROUND_GREEN;
	       break;
	     case 44:		/* BLUE background */
	       con.bg = BACKGROUND_BLUE;
	       break;
	     case 45:		/* MAGENTA background */
	       con.bg = BACKGROUND_RED | BACKGROUND_BLUE;
	       break;
	     case 46:		/* CYAN background */
	       con.bg = BACKGROUND_BLUE | BACKGROUND_GREEN;
	       break;
	     case 47:    /* WHITE background */
	       con.bg = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
	       break;
	     case 48:
	       if (con.nargs < i + 2)
		 /* Sequence error (abort) */
		 break;
	       switch (con.args[i + 1])
		 {
		 case 2:
		   if (con.nargs < i + 5)
		     /* Sequence error (abort) */
		     break;
		   r = con.args[i + 2];
		   g = con.args[i + 3];
		   b = con.args[i + 4];
		   r = r < (95 + 1) / 2 ? 0 : r > 255 ? 5 : (r - 55 + 20) / 40;
		   g = g < (95 + 1) / 2 ? 0 : g > 255 ? 5 : (g - 55 + 20) / 40;
		   b = b < (95 + 1) / 2 ? 0 : b > 255 ? 5 : (b - 55 + 20) / 40;
		   con.bg = table256[16 + r*36 + g*6 + b] << 4;
		   i += 4;
		   break;
		 case 5:
		   if (con.nargs < i + 3)
		     /* Sequence error (abort) */
		     break;
		   {
		     int idx = con.args[i + 2];
		     if (idx < 0)
		       idx = 0;
		     if (idx > 255)
		       idx = 255;
		     con.bg = table256[idx] << 4;
		     i += 2;
		   }
		   break;
		 }
	       break;
	     case 49:
	       con.bg = con.default_color & BACKGROUND_ATTR_MASK;
	       break;
	   }
       con.set_color (get_output_handle ());
      break;
    case 'q': /* Set cursor style (DECSCUSR) */
      if (con.saw_space)
	{
	    CONSOLE_CURSOR_INFO console_cursor_info;
	    GetConsoleCursorInfo (get_output_handle (), &console_cursor_info);
	    switch (con.args[0])
	      {
		case 0: /* blinking block */
		case 1: /* blinking block (default) */
		case 2: /* steady block */
		  console_cursor_info.dwSize = 100;
		  SetConsoleCursorInfo (get_output_handle (),
					&console_cursor_info);
		  break;
		case 3: /* blinking underline */
		case 4: /* steady underline */
		  console_cursor_info.dwSize = 10; /* or Windows default 25? */
		  SetConsoleCursorInfo (get_output_handle (),
					&console_cursor_info);
		  break;
		default: /* use value as percentage */
		  console_cursor_info.dwSize = con.args[0];
		  SetConsoleCursorInfo (get_output_handle (),
					&console_cursor_info);
		  break;
	      }
	}
      break;
    case 'h':
    case 'l':
      if (!con.saw_question_mark)
	{
	  switch (con.args[0])
	    {
	    case 4:    /* Insert mode */
	      con.insert_mode = (c == 'h') ? true : false;
	      syscall_printf ("insert mode %sabled",
			      con.insert_mode ? "en" : "dis");
	      break;
	    }
	  break;
	}
      switch (con.args[0])
	{
	case 25: /* Show/Hide Cursor (DECTCEM) */
	  {
	    CONSOLE_CURSOR_INFO console_cursor_info;
	    GetConsoleCursorInfo (get_output_handle (), & console_cursor_info);
	    if (c == 'h')
	      console_cursor_info.bVisible = TRUE;
	    else
	      console_cursor_info.bVisible = FALSE;
	    SetConsoleCursorInfo (get_output_handle (), & console_cursor_info);
	    break;
	  }
	case 47:   /* Save/Restore screen */
	  con.save_restore (get_output_handle (), c);
	  break;

	case 67: /* DECBKM ("DEC Backarrow Key Mode") */
	  con.backspace_keycode = (c == 'h' ? CTRL('H') : CERASE);
	  break;

	case 1000: /* Mouse tracking */
	  con.use_mouse = (c == 'h') ? 1 : 0;
	  break;

	case 1002: /* Mouse button event tracking */
	  con.use_mouse = (c == 'h') ? 2 : 0;
	  break;

	case 1003: /* Mouse any event tracking */
	  con.use_mouse = (c == 'h') ? 3 : 0;
	  break;

	case 1004: /* Focus in/out event reporting */
	  con.use_focus = (c == 'h') ? true : false;
	  break;

	case 1005: /* Extended mouse mode */
	  con.ext_mouse_mode5 = c == 'h';
	  break;

	case 1006: /* SGR extended mouse mode */
	  con.ext_mouse_mode6 = c == 'h';
	  break;

	case 1015: /* Urxvt extended mouse mode */
	  con.ext_mouse_mode15 = c == 'h';
	  break;

	case 2000: /* Raw keyboard mode */
	  set_raw_win32_keyboard_mode ((c == 'h') ? true : false);
	  break;

	default: /* Ignore */
	  syscall_printf ("unknown h/l command: %d", con.args[0]);
	  break;
	}
      break;
    case 'J':
      switch (con.args[0])
	{
	case 0:			/* Clear to end of screen */
	  clear_screen (cl_curr_pos, cl_curr_pos, cl_disp_end, cl_disp_end);
	  break;
	case 1:			/* Clear from beginning of screen to cursor */
	  clear_screen (cl_disp_beg, cl_disp_beg, cl_curr_pos, cl_curr_pos);
	  break;
	case 2:			/* Clear screen */
	  cursor_get (&x, &y);
	  clear_screen (cl_disp_beg, cl_disp_beg, cl_disp_end, cl_disp_end);
	  cursor_set (false, x, y);
	  break;
	default:
	  goto bad_escape;
	}
      break;

    case 'A':
      cursor_rel (0, -(con.args[0] ?: 1));
      break;
    case 'B':
      cursor_rel (0, con.args[0] ?: 1);
      break;
    case 'C':
      cursor_rel (con.args[0] ?: 1, 0);
      break;
    case 'D':
      cursor_rel (-(con.args[0] ?: 1),0);
      break;
    case 'K':
      switch (con.args[0])
	{
	  case 0:		/* Clear to end of line */
	    clear_screen (cl_curr_pos, cl_curr_pos, cl_disp_end, cl_curr_pos);
	    break;
	  case 2:		/* Clear line */
	    clear_screen (cl_disp_beg, cl_curr_pos, cl_disp_end, cl_curr_pos);
	    break;
	  case 1:		/* Clear from bol to cursor */
	    clear_screen (cl_disp_beg, cl_curr_pos, cl_curr_pos, cl_curr_pos);
	    break;
	  default:
	    goto bad_escape;
	}
      break;
    case 'H':
    case 'f':
      cursor_set (true, (con.args[1] ?: 1) - 1,
			(con.args[0] ?: 1) - 1);
      break;
    case 'G':   /* hpa - position cursor at column n - 1 */
      cursor_get (&x, &y);
      cursor_set (false, (con.args[0] ? con.args[0] - 1 : 0), y);
      break;
    case 'd':   /* vpa - position cursor at line n */
      cursor_get (&x, &y);
      cursor_set (true, x, (con.args[0] ? con.args[0] - 1 : 0));
      break;
    case 's':   /* Save cursor position */
      cursor_get (&con.savex, &con.savey);
      con.savey -= con.b.srWindow.Top;
      break;
    case 'u':   /* Restore cursor position */
      cursor_set (true, con.savex, con.savey);
      break;
    case 'I':	/* TAB */
      cursor_get (&x, &y);
      cursor_set (false, 8 * (x / 8 + 1), y);
      break;
    case 'L':				/* AL - insert blank lines */
      n = con.args[0] ?: 1;
      cursor_get (&x, &y);
      scroll_buffer (0, y, -1, -1, 0, y + n);
      break;
    case 'M':				/* DL - delete lines */
      n = con.args[0] ?: 1;
      cursor_get (&x, &y);
      scroll_buffer (0, y + n, -1, -1, 0, y);
      break;
    case '@':				/* IC - insert chars */
      n = con.args[0] ?: 1;
      cursor_get (&x, &y);
      scroll_buffer (x, y, -1, y, x + n, y);
      break;
    case 'P':				/* DC - delete chars */
      n = con.args[0] ?: 1;
      cursor_get (&x, &y);
      scroll_buffer (x + n, y, -1, y, x, y);
      break;
    case 'S':				/* SF - Scroll forward */
      n = con.args[0] ?: 1;
      scroll_buffer_screen (0, n, -1, -1, 0, 0);
      break;
    case 'T':				/* SR - Scroll down */
      con.fillin (get_output_handle ());
      n = con.b.srWindow.Top + con.args[0] ?: 1;
      scroll_buffer_screen (0, 0, -1, -1, 0, n);
      break;
    case 'X':				/* ec - erase chars */
      n = con.args[0] ?: 1;
      cursor_get (&x, &y);
      scroll_buffer (x + n, y, -1, y, x, y);
      scroll_buffer (x, y, -1, y, x + n, y);
      break;
    case 'Z':				/* Back tab */
      cursor_get (&x, &y);
      cursor_set (false, ((8 * (x / 8 + 1)) - 8), y);
      break;
    case 'b':				/* Repeat char #1 #2 times */
      if (con.insert_mode)
	{
	  cursor_get (&x, &y);
	  scroll_buffer (x, y, -1, y, x + con.args[1], y);
	}
      while (con.args[1]--)
	WriteFile (get_output_handle (), &con.args[0], 1, (DWORD *) &x, 0);
      break;
    case 'c':				/* u9 - Terminal enquire string */
      if (con.saw_greater_than_sign)
	/* Generate Secondary Device Attribute report, using 67 = ASCII 'C'
	   to indicate Cygwin (convention used by Rxvt, Urxvt, Screen, Mintty),
	   and cygwin version for terminal version. */
	__small_sprintf (buf, "\033[>67;%d%02d;0c",
			 CYGWIN_VERSION_DLL_MAJOR, CYGWIN_VERSION_DLL_MINOR);
      else
	strcpy (buf, "\033[?6c");
      /* The generated report needs to be injected for read-ahead into the
	 fhandler_console object associated with standard input.
	 So puts_readahead does not work.
	 Use a common console read-ahead buffer instead. */
      acquire_input_mutex (INFINITE);
      con.cons_rapoi = NULL;
      strcpy (con.cons_rabuf, buf);
      con.cons_rapoi = con.cons_rabuf;
      release_input_mutex ();
      /* Wake up read() or select() by sending a message
	 which has no effect */
      PostMessageW (GetConsoleWindow (), WM_SETFOCUS, 0, 0);
      break;
    case 'n':
      switch (con.args[0])
	{
	case 6:				/* u7 - Cursor position request */
	  cursor_get (&x, &y);
	  y -= con.b.srWindow.Top;
	  /* x -= con.b.srWindow.Left;		// not available yet */
	  __small_sprintf (buf, "\033[%d;%dR", y + 1, x + 1);
	  acquire_input_mutex (INFINITE);
	  con.cons_rapoi = NULL;
	  strcpy (con.cons_rabuf, buf);
	  con.cons_rapoi = con.cons_rabuf;
	  release_input_mutex ();
	  /* Wake up read() or select() by sending a message
	     which has no effect */
	  PostMessageW (GetConsoleWindow (), WM_SETFOCUS, 0, 0);
	  break;
	default:
	  goto bad_escape;
	}
      break;
    case 'r':				/* Set Scroll region */
      con.scroll_region.Top = con.args[0] ? con.args[0] - 1 : 0;
      con.scroll_region.Bottom = con.args[1] ? con.args[1] - 1 : -1;
      cursor_set (true, 0, 0);
      break;
    case 'g':				/* TAB set/clear */
      break;
    default:
bad_escape:
      break;
    }
}

#define NUM_REPLACEMENT_CHARS	3

static const wchar_t replacement_char[NUM_REPLACEMENT_CHARS] =
{
  0xfffd, /* REPLACEMENT CHARACTER */
  0x25a1, /* WHITE SQUARE */
  0x2592  /* MEDIUM SHADE */
};
/* nFont member is always 0 so we have to use the facename. */
static WCHAR cons_facename[LF_FACESIZE];
static WCHAR rp_char;
static NO_COPY HDC cdc;

static int CALLBACK
enum_proc (const LOGFONTW *lf, const TEXTMETRICW *tm,
	   DWORD FontType, LPARAM lParam)
{
  int *done = (int *) lParam;
  *done = 1;
  return 0;
}

static void
check_font (HANDLE hdl)
{
  CONSOLE_FONT_INFOEX cfi;
  LOGFONTW lf;

  cfi.cbSize = sizeof cfi;
  if (!GetCurrentConsoleFontEx (hdl, 0, &cfi))
    return;
  /* Switched font? */
  if (wcscmp (cons_facename, cfi.FaceName) == 0)
    return;
  if (!cdc && !(cdc = GetDC (GetConsoleWindow ())))
    return;
  /* Some FaceNames like DejaVu Sans Mono are sometimes returned with stray
     trailing chars.  Fix it. */
  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
  wchar_t *cp = wcpcpy (lf.lfFaceName, cfi.FaceName) - 1;
  int done = 0;
  do
    {
      EnumFontFamiliesExW (cdc, &lf, enum_proc, (LPARAM) &done, 0);
      if (!done)
	*cp-- = L'\0';
    }
  while (!done && cp >= lf.lfFaceName);
  /* What, really?  No recognizable font? */
  if (!done)
    {
      rp_char = L'?';
      return;
    }
  /* Yes.  Check for the best replacement char. */
  HFONT f = CreateFontW (0, 0, 0, 0,
			 cfi.FontWeight, FALSE, FALSE, FALSE,
			 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			 FIXED_PITCH | FF_DONTCARE, lf.lfFaceName);
  if (!f)
    return;

  HFONT old_f = (HFONT) SelectObject(cdc, f);
  if (old_f)
    {
      WORD glyph_idx[NUM_REPLACEMENT_CHARS];

      if (GetGlyphIndicesW (cdc, replacement_char,
			    NUM_REPLACEMENT_CHARS, glyph_idx,
			    GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR)
	{
	  int i;

	  for (i = 0; i < NUM_REPLACEMENT_CHARS; ++i)
	    if (glyph_idx[i] != 0xffff)
	      break;
	  if (i == NUM_REPLACEMENT_CHARS)
	    i = 0;
	  rp_char = replacement_char[i];
	  /* Note that we copy the original name returned by
	     GetCurrentConsoleFontEx, even if it was broken.
	     This allows an early return, rather than to store
	     the fixed name and then having to enum font families
	     all over again. */
	  wcscpy (cons_facename, cfi.FaceName);
	}
      SelectObject (cdc, old_f);
    }
  DeleteObject (f);
}

/* This gets called when we found an invalid input character.
   Print one of the above Unicode chars as replacement char. */
inline void
fhandler_console::write_replacement_char ()
{
  check_font (get_output_handle ());

  DWORD done;
  WriteConsoleW (get_output_handle (), &rp_char, 1, &done, 0);
}

const unsigned char *
fhandler_console::write_normal (const unsigned char *src,
				const unsigned char *end)
{
  /* Scan forward to see what a char which needs special treatment */
  DWORD done;
  DWORD buf_len;
  const unsigned char *found = src;
  int ret;
  mbstate_t ps;
  mbtowc_p f_mbtowc;

  /* The alternate charset is always 437, just as in the Linux console. */
  f_mbtowc = con.get_console_cp () ? __cp_mbtowc (437) : __MBTOWC;
  if (f_mbtowc == __ascii_mbtowc)
    f_mbtowc = __utf8_mbtowc;

  /* First check if we have cached lead bytes of a former try to write
     a truncated multibyte sequence.  If so, process it. */
  if (trunc_buf.len)
    {
      const unsigned char *nfound;
      int cp_len = MIN (end - src, 4 - trunc_buf.len);
      memcpy (trunc_buf.buf + trunc_buf.len, src, cp_len);
      memset (&ps, 0, sizeof ps);
      switch (ret = f_mbtowc (_REENT, NULL, (const char *) trunc_buf.buf,
			       trunc_buf.len + cp_len, &ps))
	{
	case -2:
	  /* Still truncated multibyte sequence?  Keep in trunc_buf. */
	  trunc_buf.len += cp_len;
	  return end;
	case -1:
	  /* Give up, print replacement chars for trunc_buf... */
	  for (int i = 0; i < trunc_buf.len; ++i)
	    write_replacement_char ();
	  /* ... mark trunc_buf as unused... */
	  trunc_buf.len = 0;
	  /* ... and proceed. */
	  nfound = NULL;
	  break;
	case 0:
	  nfound = trunc_buf.buf + 1;
	  break;
	default:
	  nfound = trunc_buf.buf + ret;
	  break;
	}
      /* Valid multibyte sequence?  Process. */
      if (nfound)
	{
	  buf_len = con.str_to_con (f_mbtowc, write_buf,
				    (const char *) trunc_buf.buf,
				    nfound - trunc_buf.buf);
	  if (!write_console (write_buf, buf_len, done))
	    {
	      debug_printf ("multibyte sequence write failed, handle %p",
			    get_output_handle ());
	      return 0;
	    }
	  found = src + (nfound - trunc_buf.buf - trunc_buf.len);
	  trunc_buf.len = 0;
	  return found;
	}
    }

  /* Loop over src buffer as long as we have just simple characters.  Stop
     as soon as we reach the conversion limit, or if we encounter a control
     character or a truncated or invalid mutibyte sequence. */
  /* If system has 24 bit color capability, just write all control
     sequences to console since xterm compatible mode is enabled. */
  memset (&ps, 0, sizeof ps);
  while (found < end
	 && found - src < CONVERT_LIMIT
	 && base_chars[*found] != IGN
	 && base_chars[*found] != ESC
	 && ((wincap.has_con_24bit_colors () && !con_is_legacy)
	     || base_chars[*found] == NOR))
    {
      switch (ret = f_mbtowc (_REENT, NULL, (const char *) found,
			       end - found, &ps))
	{
	case -2: /* Truncated multibyte sequence.  Store for next write. */
	  trunc_buf.len = end - found;
	  memcpy (trunc_buf.buf, found, trunc_buf.len);
	  goto do_print;
	case -1: /* Invalid multibyte sequence. Handled below. */
	  goto do_print;
	case 0:
	  found++;
	  break;
	default:
	  found += ret;
	  break;
	}
    }

do_print:

  /* Print all the base characters out */
  if (found != src)
    {
      DWORD len = found - src;
      buf_len = con.str_to_con (f_mbtowc, write_buf, (const char *) src, len);
      if (!buf_len)
	{
	  debug_printf ("conversion error, handle %p",
			get_output_handle ());
	  __seterrno ();
	  return 0;
	}

      if (con.insert_mode)
	{
	  int x, y;
	  cursor_get (&x, &y);
	  scroll_buffer (x, y, -1, y, x + buf_len, y);
	}

      if (!write_console (write_buf, buf_len, done))
	{
	  debug_printf ("write failed, handle %p", get_output_handle ());
	  return 0;
	}
      /* Stop here if we reached the conversion limit. */
      if (len >= CONVERT_LIMIT)
	return found + trunc_buf.len;
    }
  /* If there's still something in the src buffer, but it's not a truncated
     multibyte sequence, then we stumbled over a control character or an
     invalid multibyte sequence.  Print it. */
  if (found < end && trunc_buf.len == 0)
    {
      int x, y;
      switch (base_chars[*found])
	{
	case SO:	/* Shift Out: Invoke G1 character set (ISO 2022) */
	  con.iso_2022_G1 = true;
	  break;
	case SI:	/* Shift In: Invoke G0 character set (ISO 2022) */
	  con.iso_2022_G1 = false;
	  break;
	case BEL:
	  beep ();
	  break;
	case ESC:
	  con.state = gotesc;
	  wpbuf.put (*found);
	  break;
	case DWN:
	  cursor_get (&x, &y);
	  if (y >= srBottom)
	    {
	      if (y >= con.b.srWindow.Bottom && !con.scroll_region.Top)
		WriteConsoleW (get_output_handle (), L"\n", 1, &done, 0);
	      else
		{
		  scroll_buffer (0, srTop + 1, -1, srBottom, 0, srTop);
		  y--;
		}
	    }
	  cursor_set (false,
		      ((get_ttyp ()->ti.c_oflag & ONLCR) ? 0 : x), y + 1);
	  break;
	case BAK:
	  cursor_rel (-1, 0);
	  break;
	case IGN:
	 /* Up to release 3.1.3 we called cursor_rel (1, 0); to move the cursor
	    one step to the right.  However, that neither matches the terminfo
	    for the cygwin terminal, nor the one for the xterm terminal. */
	  break;
	case CR:
	  cursor_get (&x, &y);
	  cursor_set (false, 0, y);
	  break;
	case ERR:
	  /* Don't print chars marked as ERR chars, except for a ASCII CAN
	     sequence which is printed as singlebyte chars from the UTF
	     Basic Latin and Latin 1 Supplement plains. */
	  if (*found == 0x18)
	    {
	      write_replacement_char ();
	      if (found + 1 < end)
		{
		  ret = __utf8_mbtowc (_REENT, NULL, (const char *) found + 1,
				       end - found - 1, &ps);
		  if (ret != -1)
		    while (ret-- > 0)
		      {
			WCHAR w = *(found + 1);
			WriteConsoleW (get_output_handle (), &w, 1, &done, 0);
			found++;
		      }
		}
	    }
	  break;
	case TAB:
	  cursor_get (&x, &y);
	  cursor_set (false, 8 * (x / 8 + 1), y);
	  break;
	case NOR:
	  write_replacement_char ();
	  break;
	}
      found++;
    }
  return found + trunc_buf.len;
}

ssize_t __stdcall
fhandler_console::write (const void *vsrc, size_t len)
{
  bg_check_types bg = bg_check (SIGTTOU);
  if (bg <= bg_eof)
    return (ssize_t) bg;

  if (get_ttyp ()->ti.c_lflag & FLUSHO)
    return len; /* Discard write data */

  if (get_ttyp ()->output_stopped && is_nonblocking ())
    {
      set_errno (EAGAIN);
      return -1;
    }
  while (get_ttyp ()->output_stopped)
    cygwait (10);

  acquire_attach_mutex (INFINITE);
  push_process_state process_state (PID_TTYOU);

  set_output_mode (tty::cygwin, &get_ttyp ()->ti, &handle_set);

  acquire_output_mutex (INFINITE);

  /* Run and check for ansi sequences */
  unsigned const char *src = (unsigned char *) vsrc;
  unsigned const char *end = src + len;
  /* This might look a bit far fetched, but using the TLS path buffer allows
     to allocate a big buffer without using the stack too much.  Doing it here
     in write instead of in write_normal should be faster, too. */
  tmp_pathbuf tp;
  write_buf = tp.w_get ();

  debug_printf ("%p, %ld", vsrc, len);

  while (src < end)
    {
      paranoid_printf ("char %0c state is %d", *src, con.state);
      switch (con.state)
	{
	case normal:
	  src = write_normal (src, end);
	  if (!src) /* write_normal failed */
	    {
	      release_output_mutex ();
	      return -1;
	    }
	  break;
	case gotesc:
	  if (*src == '[')		/* CSI Control Sequence Introducer */
	    {
	      wpbuf.put (*src);
	      con.state = gotsquare;
	      memset (con.args, 0, sizeof con.args);
	      con.nargs = 0;
	      con.saw_question_mark = false;
	      con.saw_greater_than_sign = false;
	      con.saw_space = false;
	      con.saw_exclamation_mark = false;
	    }
	  else if (*src == '8')		/* DECRC Restore cursor position */
	    {
	      if (con.screen_alternated)
		{
		  /* For xterm mode only */
		  /* Just send the sequence */
		  wpbuf.put (*src);
		  wpbuf.send (get_output_handle ());
		}
	      else if (con.savex >= 0 && con.savey >= 0)
		cursor_set (false, con.savex, con.savey);
	      con.state = normal;
	      wpbuf.empty();
	    }
	  else if (*src == '7')		/* DECSC Save cursor position */
	    {
	      if (con.screen_alternated)
		{
		  /* For xterm mode only */
		  /* Just send the sequence */
		  wpbuf.put (*src);
		  wpbuf.send (get_output_handle ());
		}
	      else
		cursor_get (&con.savex, &con.savey);
	      con.state = normal;
	      wpbuf.empty();
	    }
	  else if (wincap.has_con_24bit_colors () && !con_is_legacy
		   && wincap.has_con_broken_il_dl () && *src == 'M')
	    { /* Reverse Index (scroll down) */
	      int x, y;
	      cursor_get (&x, &y);
	      if (y == srTop)
		{
		  if (y == con.b.srWindow.Top
		      && srBottom == con.b.srWindow.Bottom)
		    {
		      /* Erase scroll down area */
		      wchar_t buf[] = L"\033[32768;1H\033[J\033[32768;32768";
		      __small_swprintf (buf, L"\033[%d;1H\033[J\033[%d;%dH",
					srBottom - con.b.srWindow.Top + 1,
					y + 1 - con.b.srWindow.Top, x + 1);
		      WriteConsoleW (get_output_handle (),
				     buf, wcslen (buf), 0, 0);
		    }
		  /* Substitute "CSI Ps T" */
		  wpbuf.put ('[');
		  wpbuf.put ('T');
		}
	      else
		wpbuf.put (*src);
	      wpbuf.send (get_output_handle ());
	      con.state = normal;
	      wpbuf.empty();
	    }
	  else if (*src == ']')		/* OSC Operating System Command */
	    {
	      wpbuf.put (*src);
	      con.rarg = 0;
	      con.my_title_buf[0] = '\0';
	      con.state = gotrsquare;
	    }
	  else if (wincap.has_con_24bit_colors () && !con_is_legacy)
	    {
	      if (*src == 'c') /* RIS Full reset */
		{
		  con.scroll_region.Top = 0;
		  con.scroll_region.Bottom = -1;
		  con.savex = con.savey = -1;
		  con.cursor_key_app_mode = false;
		}
	      /* ESC sequences below (e.g. OSC, etc) are left to xterm
		 emulation in xterm compatible mode, therefore, are not
		 handled and just sent them. */
	      wpbuf.put (*src);
	      /* Just send the sequence */
	      wpbuf.send (get_output_handle ());
	      con.state = normal;
	      wpbuf.empty();
	    }
	  else if (*src == '(')		/* Designate G0 character set */
	    {
	      wpbuf.put (*src);
	      con.state = gotparen;
	    }
	  else if (*src == ')')		/* Designate G1 character set */
	    {
	      wpbuf.put (*src);
	      con.state = gotrparen;
	    }
	  else if (*src == 'M')		/* Reverse Index (scroll down) */
	    {
	      con.fillin (get_output_handle ());
	      scroll_buffer_screen (0, 0, -1, -1, 0, 1);
	      con.state = normal;
	      wpbuf.empty();
	    }
	  else if (*src == 'c')		/* RIS Full Reset */
	    {
	      con.set_default_attr ();
	      con.vt100_graphics_mode_G0 = false;
	      con.vt100_graphics_mode_G1 = false;
	      con.iso_2022_G1 = false;
	      cursor_set (false, 0, 0);
	      clear_screen (cl_buf_beg, cl_buf_beg, cl_buf_end, cl_buf_end);
	      con.state = normal;
	      wpbuf.empty();
	    }
	  else if (*src == 'R')		/* ? */
	    {
	      con.state = normal;
	      wpbuf.empty();
	    }
	  else
	    {
	      con.state = normal;
	      wpbuf.empty();
	    }
	  src++;
	  break;
	case gotarg1:
	  if (isdigit (*src))
	    {
	      if (con.nargs < MAXARGS)
		con.args[con.nargs] = con.args[con.nargs] * 10 + *src - '0';
	      wpbuf.put (*src);
	      src++;
	    }
	  else if (*src == ';')
	    {
	      wpbuf.put (*src);
	      src++;
	      if (con.nargs < MAXARGS)
		con.nargs++;
	    }
	  else if (*src == ' ')
	    {
	      wpbuf.put (*src);
	      src++;
	      con.saw_space = true;
	      con.state = gotcommand;
	    }
	  else
	    con.state = gotcommand;
	  break;
	case gotcommand:
	  if (con.nargs < MAXARGS)
	    con.nargs++;
	  char_command (*src++);
	  con.state = normal;
	  wpbuf.empty();
	  break;
	case gotrsquare:
	  if (isdigit (*src))
	    con.rarg = con.rarg * 10 + (*src - '0');
	  else if (*src == ';' && (con.rarg == 2 || con.rarg == 0))
	    con.state = gettitle;
	  else if (*src == ';' && (con.rarg == 4 || con.rarg == 104
				   || (con.rarg >= 10 && con.rarg <= 19)))
	    con.state = eatpalette;
	  else
	    con.state = eattitle;
	  wpbuf.put (*src);
	  src++;
	  break;
	case eattitle:
	case gettitle:
	  {
	    wpbuf.put (*src);
	    int n = strlen (con.my_title_buf);
	    if (*src < ' ')
	      {
		if (wincap.has_con_24bit_colors () && !con_is_legacy)
		  wpbuf.send (get_output_handle ());
		else if (*src == '\007' && con.state == gettitle)
		  set_console_title (con.my_title_buf);
		con.state = normal;
		wpbuf.empty();
	      }
	    else if (n < TITLESIZE)
	      {
		con.my_title_buf[n++] = *src;
		con.my_title_buf[n] = '\0';
	      }
	    src++;
	    break;
	  }
	case eatpalette:
	  wpbuf.put (*src);
	  if (*src == '?')
	    con.saw_question_mark = true;
	  else if (*src == '\033')
	    con.state = endpalette;
	  else if (*src == '\a')
	    {
	      /* Send OSC Ps; Pt BEL other than OSC Ps; ? BEL */
	      if (wincap.has_con_24bit_colors () && !con_is_legacy
		  && !con.saw_question_mark)
		wpbuf.send (get_output_handle ());
	      con.state = normal;
	      wpbuf.empty();
	    }
	  src++;
	  break;
	case endpalette:
	  wpbuf.put (*src);
	  if (*src == '\\')
	    {
	      /* Send OSC Ps; Pt ST other than OSC Ps; ? ST */
	      if (wincap.has_con_24bit_colors () && !con_is_legacy
		  && !con.saw_question_mark)
		wpbuf.send (get_output_handle ());
	      con.state = normal;
	    }
	  else
	    /* Sequence error (abort) */
	    con.state = normal;
	  wpbuf.empty();
	  src++;
	  break;
	case gotsquare:
	  if (*src == ';')
	    {
	      con.state = gotarg1;
	      wpbuf.put (*src);
	      if (con.nargs < MAXARGS)
		con.nargs++;
	      src++;
	    }
	  else if (isalpha (*src))
	    con.state = gotcommand;
	  else if (*src != '@' && !isalpha (*src) && !isdigit (*src))
	    {
	      if (*src == '?')
		con.saw_question_mark = true;
	      else if (*src == '>')
		con.saw_greater_than_sign = true;
	      else if (*src == '!')
		con.saw_exclamation_mark = true;
	      wpbuf.put (*src);
	      /* ignore any extra chars between [ and first arg or command */
	      src++;
	    }
	  else
	    con.state = gotarg1;
	  break;
	case gotparen:	/* Designate G0 Character Set (ISO 2022) */
	  if (*src == '0')
	    con.vt100_graphics_mode_G0 = true;
	  else
	    con.vt100_graphics_mode_G0 = false;
	  con.state = normal;
	  wpbuf.empty();
	  src++;
	  break;
	case gotrparen:	/* Designate G1 Character Set (ISO 2022) */
	  if (*src == '0')
	    con.vt100_graphics_mode_G1 = true;
	  else
	    con.vt100_graphics_mode_G1 = false;
	  con.state = normal;
	  wpbuf.empty();
	  src++;
	  break;
	}
    }
  release_output_mutex ();

  syscall_printf ("%ld = fhandler_console::write(...)", len);

  release_attach_mutex ();
  return len;
}

void
fhandler_console::doecho (const void *str, DWORD len)
{
  bool stopped = get_ttyp ()->output_stopped;
  get_ttyp ()->output_stopped = false;
  write (str, len);
  get_ttyp ()->output_stopped = stopped;
}

static const struct {
  int vk;
  const char *val[4];
} keytable[] = {
	       /* NORMAL */    /* SHIFT */     /* CTRL */     /* CTRL-SHIFT */
  /* Unmodified and Alt-modified keypad keys comply with linux console
     SHIFT, CTRL, CTRL-SHIFT modifiers comply with xterm modifier usage */
  {VK_NUMPAD5,	{"\033[G",	"\033[1;2G",	"\033[1;5G",	"\033[1;6G"}},
  {VK_CLEAR,	{"\033[G",	"\033[1;2G",	"\033[1;5G",	"\033[1;6G"}},
  {VK_LEFT,	{"\033[D",	"\033[1;2D",	"\033[1;5D",	"\033[1;6D"}},
  {VK_RIGHT,	{"\033[C",	"\033[1;2C",	"\033[1;5C",	"\033[1;6C"}},
  {VK_UP,	{"\033[A",	"\033[1;2A",	"\033[1;5A",	"\033[1;6A"}},
  {VK_DOWN,	{"\033[B",	"\033[1;2B",	"\033[1;5B",	"\033[1;6B"}},
  {VK_PRIOR,	{"\033[5~",	"\033[5;2~",	"\033[5;5~",	"\033[5;6~"}},
  {VK_NEXT,	{"\033[6~",	"\033[6;2~",	"\033[6;5~",	"\033[6;6~"}},
  {VK_HOME,	{"\033[1~",	"\033[1;2~",	"\033[1;5~",	"\033[1;6~"}},
  {VK_END,	{"\033[4~",	"\033[4;2~",	"\033[4;5~",	"\033[4;6~"}},
  {VK_INSERT,	{"\033[2~",	"\033[2;2~",	"\033[2;5~",	"\033[2;6~"}},
  {VK_DELETE,	{"\033[3~",	"\033[3;2~",	"\033[3;5~",	"\033[3;6~"}},
  /* F1...F12, SHIFT-F1...SHIFT-F10 comply with linux console
     F6...F12, and all modified F-keys comply with rxvt (compatible extension) */
  {VK_F1,	{"\033[[A",	"\033[23~",	"\033[11^",	"\033[23^"}},
  {VK_F2,	{"\033[[B",	"\033[24~",	"\033[12^",	"\033[24^"}},
  {VK_F3,	{"\033[[C",	"\033[25~",	"\033[13^",	"\033[25^"}},
  {VK_F4,	{"\033[[D",	"\033[26~",	"\033[14^",	"\033[26^"}},
  {VK_F5,	{"\033[[E",	"\033[28~",	"\033[15^",	"\033[28^"}},
  {VK_F6,	{"\033[17~",	"\033[29~",	"\033[17^",	"\033[29^"}},
  {VK_F7,	{"\033[18~",	"\033[31~",	"\033[18^",	"\033[31^"}},
  {VK_F8,	{"\033[19~",	"\033[32~",	"\033[19^",	"\033[32^"}},
  {VK_F9,	{"\033[20~",	"\033[33~",	"\033[20^",	"\033[33^"}},
  {VK_F10,	{"\033[21~",	"\033[34~",	"\033[21^",	"\033[34^"}},
  {VK_F11,	{"\033[23~",	"\033[23$",	"\033[23^",	"\033[23@"}},
  {VK_F12,	{"\033[24~",	"\033[24$",	"\033[24^",	"\033[24@"}},
  /* CTRL-6 complies with Windows cmd console but should be fixed */
  {'6',		{NULL,		NULL,		"\036",		NULL}},
  /* Table end marker */
  {0}
};

const char *
fhandler_console::get_nonascii_key (INPUT_RECORD& input_rec, char *tmp)
{
#define NORMAL  0
#define SHIFT	1
#define CONTROL	2
/*#define CONTROLSHIFT	3*/

  int modifier_index = NORMAL;
  if (input_rec.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
    modifier_index = SHIFT;
  if (input_rec.Event.KeyEvent.dwControlKeyState & CTRL_PRESSED)
    modifier_index += CONTROL;

  for (int i = 0; keytable[i].vk; i++)
    if (input_rec.Event.KeyEvent.wVirtualKeyCode == keytable[i].vk)
      {
	if ((input_rec.Event.KeyEvent.dwControlKeyState & ALT_PRESSED)
	    && keytable[i].val[modifier_index] != NULL)
	  { /* Generic ESC prefixing if Alt is pressed */
	    tmp[0] = '\033';
	    strcpy (tmp + 1, keytable[i].val[modifier_index]);
	    return tmp;
	  }
	else
	  return keytable[i].val[modifier_index];
      }

  if (input_rec.Event.KeyEvent.uChar.AsciiChar)
    {
      tmp[0] = input_rec.Event.KeyEvent.uChar.AsciiChar;
      tmp[1] = '\0';
      return tmp;
    }
  return NULL;
}

int
fhandler_console::init (HANDLE h, DWORD a, mode_t bin)
{
  // this->fhandler_termios::init (h, mode, bin);
  /* Ensure both input and output console handles are open */
  int flags = 0;

  a &= GENERIC_READ | GENERIC_WRITE;
  if (a == GENERIC_READ)
    flags = O_RDONLY;
  if (a == GENERIC_WRITE)
    flags = O_WRONLY;
  if (a == (GENERIC_READ | GENERIC_WRITE))
    flags = O_RDWR;
  open_with_arch (flags | O_BINARY | (h ? 0 : O_NOCTTY));

  return !tcsetattr (0, &get_ttyp ()->ti);
}

int
fhandler_console::igncr_enabled ()
{
  return get_ttyp ()->ti.c_iflag & IGNCR;
}

void
fhandler_console::set_close_on_exec (bool val)
{
  close_on_exec (val);
}

void __stdcall
set_console_title (char *title)
{
  wchar_t buf[TITLESIZE + 1];
  sys_mbstowcs (buf, TITLESIZE + 1, title);
  lock_ttys here (15000);
  SetConsoleTitleW (buf);
  debug_printf ("title '%W'", buf);
}

void
fhandler_console::fixup_after_fork_exec (bool execing)
{
  set_unit ();
  setup_io_mutex ();
}

// #define WINSTA_ACCESS (WINSTA_READATTRIBUTES | STANDARD_RIGHTS_READ | STANDARD_RIGHTS_WRITE | WINSTA_CREATEDESKTOP | WINSTA_EXITWINDOWS)
#define WINSTA_ACCESS WINSTA_ALL_ACCESS

/* Create a console in an invisible window station.  This should work
   in all versions of Windows NT except Windows 7 (so far). */
bool
fhandler_console::create_invisible_console (HWINSTA horig)
{
  HWINSTA h = CreateWindowStationW (NULL, 0, WINSTA_ACCESS, NULL);
  termios_printf ("%p = CreateWindowStation(NULL), %E", h);
  BOOL b;
  if (h)
    {
      b = SetProcessWindowStation (h);
      termios_printf ("SetProcessWindowStation %d, %E", b);
    }
  b = AllocConsole ();	/* will cause flashing if CreateWindowStation
			   failed */
  if (!h)
    SetParent (GetConsoleWindow (), HWND_MESSAGE);
  if (horig && h && h != horig && SetProcessWindowStation (horig))
    CloseWindowStation (h);
  termios_printf ("%d = AllocConsole (), %E", b);
  invisible_console = true;
  return b;
}

/* Ugly workaround for Windows 7 and later.

   First try to just attach to any console which may have started this
   app.  If that works use this as our "invisible console".

   This will fail if not started from the command prompt.  In that case, start
   a dummy console application in a hidden state so that we can use its console
   as our invisible console.  This probably works everywhere but process
   creation is slow and to be avoided if possible so the window station method
   is vastly preferred.

   FIXME: This is not completely thread-safe since it creates two inheritable
   handles which are known only to this function.  If another thread starts
   a process the new process will inherit these handles.  However, since this
   function is currently only called at startup and during exec, it shouldn't
   be a big deal.  */
bool
fhandler_console::create_invisible_console_workaround (bool force)
{
  /* If force is set, avoid to reattach to existing console. */
  if (force || !AttachConsole (-1))
    {
      bool taskbar;
      DWORD err = force ? 0 : GetLastError ();
      path_conv helper ("/bin/cygwin-console-helper.exe");
      HANDLE hello = NULL;
      HANDLE goodbye = NULL;
      /* If err == ERROR_PROC_FOUND then this method won't work.  But that's
	 ok.  The window station method should work ok when AttachConsole doesn't
	 work.

	 If the helper doesn't exist or we can't create event handles then we
	 can't use this method. */
      if (err == ERROR_PROC_NOT_FOUND || !helper.exists ()
	  || !(hello = CreateEvent (&sec_none, true, false, NULL))
	  || !(goodbye = CreateEvent (&sec_none, true, false, NULL)))
	{
	  AllocConsole ();	/* This is just sanity check code.  We should
				   never actually hit here unless we're running
				   in an environment which lacks the helper
				   app. */
	  taskbar = true;
	}
      else
	{
	  STARTUPINFOW si = {};
	  PROCESS_INFORMATION pi;
	  size_t len = helper.get_wide_win32_path_len ();
	  WCHAR cmd[len + 1];
	  WCHAR args[len + 1 + (2 * sizeof (" 0xffffffffffffffff")) + 1];
	  WCHAR title[] = L"invisible cygwin console";

	  /* Create a new hidden process.  Use the two event handles as
	     argv[1] and argv[2]. */

	  helper.get_wide_win32_path (cmd);
	  __small_swprintf (args, L"\"%W\" %p %p", cmd, hello, goodbye);

	  si.cb = sizeof (si);
	  si.dwFlags = STARTF_USESHOWWINDOW;
	  si.wShowWindow = SW_HIDE;
	  si.lpTitle = title;

	  BOOL x = CreateProcessW (cmd, args,
				   &sec_none_nih, &sec_none_nih, true,
				   CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	  if (x)
	    {
	      CloseHandle (pi.hProcess);	/* Don't need */
	      CloseHandle (pi.hThread);		/*  these.    */
	    }
	  taskbar = false;
	  /* Wait for subprocess to indicate that it is live.  This may not
	     actually be needed but it's hard to say since it is possible that
	     there will be no console for a brief time after the process
	     returns and there is no easy way to determine if/when this happens
	     in Windows.  So play it safe. */
	  if (!x || (WaitForSingleObject (hello, 10000) != WAIT_OBJECT_0)
	      || !AttachConsole (pi.dwProcessId))
	    AllocConsole ();	/* Oh well.  Watch the flash. */
	}

      if (!taskbar)
	/* Setting the owner of the console window to HWND_MESSAGE seems to
	   hide it from the taskbar.  Don't know if this method is faster than
	   calling ShowWindowAsync but it should guarantee no taskbar presence
	   for the hidden console. */
	SetParent (GetConsoleWindow (), HWND_MESSAGE);
      if (hello)
	CloseHandle (hello);
      if (goodbye)
	{
	  SetEvent (goodbye);	/* Tell helper process it's ok to exit. */
	  CloseHandle (goodbye);
	}
    }
  return invisible_console = true;
}

void
fhandler_console::free_console ()
{
  BOOL res = FreeConsole ();
  debug_printf ("freed console, res %d", res);
  init_console_handler (false);
}

bool
fhandler_console::need_invisible (bool force)
{
  BOOL b = false;
  /* If force is set, forcibly create a new invisible console
     even if a console device already exists. */
  if (exists () && !force)
    invisible_console = false;
  else
    {
      HWINSTA h;
      /* The intent here is to allocate an "invisible" console if we have no
	 controlling tty or to reuse the existing console if we already have
	 a tty.  So, first get the old window station.  If there is no controlling
	 terminal, create a new window station and then set it as the current
	 window station.  The subsequent AllocConsole will then be allocated
	 invisibly.  But, after doing that we have to restore any existing windows
	 station or, strangely, characters will not be displayed in any windows
	 drawn on the current screen.  We only do this if we have changed to
	 a new window station and if we had an existing windows station previously.
	 We also close the previously opened window station even though AllocConsole
	 is now "using" it.  This doesn't seem to cause any problems.

	 Things to watch out for if you make changes in this code:

	 - Flashing, black consoles showing up when you start, e.g., ssh in
	   an xterm.
	 - Non-displaying of characters in rxvt or xemacs if you start a
	   process using setsid: bash -lc "setsid rxvt".  */

      h = GetProcessWindowStation ();

      USEROBJECTFLAGS oi;
      DWORD len;
      if (!h
	  || !GetUserObjectInformationW (h, UOI_FLAGS, &oi, sizeof (oi), &len)
	  || !(oi.dwFlags & WSF_VISIBLE))
	{
	  b = true;
	  debug_printf ("window station is not visible");
	  AllocConsole ();
	  invisible_console = true;
	}
      else if (wincap.has_broken_alloc_console ())
	b = create_invisible_console_workaround (force);
      else
	b = create_invisible_console (h);
    }

  debug_printf ("invisible_console %d", invisible_console);
  return b;
}

DWORD
fhandler_console::__acquire_input_mutex (const char *fn, int ln, DWORD ms)
{
#ifdef DEBUGGING
  strace.prntf (_STRACE_TERMIOS, fn, "(%d): trying to get input_mutex", ln);
#endif
  DWORD res = WaitForSingleObject (input_mutex, ms);
  if (res != WAIT_OBJECT_0)
    strace.prntf (_STRACE_TERMIOS, fn,
		  "(%d): Failed to acquire input_mutex %08x",
		  ln, GetLastError ());
#ifdef DEBUGGING
  else
    strace.prntf (_STRACE_TERMIOS, fn, "(%d): got input_mutex", ln);
#endif
  return res;
}

void
fhandler_console::__release_input_mutex (const char *fn, int ln)
{
  ReleaseMutex (input_mutex);
#ifdef DEBUGGING
  strace.prntf (_STRACE_TERMIOS, fn, "(%d): release input_mutex", ln);
#endif
}

DWORD
fhandler_console::__acquire_output_mutex (const char *fn, int ln, DWORD ms)
{
#ifdef DEBUGGING
  strace.prntf (_STRACE_TERMIOS, fn, "(%d): trying to get output_mutex", ln);
#endif
  DWORD res = WaitForSingleObject (output_mutex, ms);
  if (res != WAIT_OBJECT_0)
    strace.prntf (_STRACE_TERMIOS, fn,
		  "(%d): Failed to acquire output_mutex %08x",
		  ln, GetLastError ());
#ifdef DEBUGGING
  else
    strace.prntf (_STRACE_TERMIOS, fn, "(%d): got output_mutex", ln);
#endif
  return res;
}

void
fhandler_console::__release_output_mutex (const char *fn, int ln)
{
  ReleaseMutex (output_mutex);
#ifdef DEBUGGING
  strace.prntf (_STRACE_TERMIOS, fn, "(%d): release output_mutex", ln);
#endif
}

void
fhandler_console::get_duplicated_handle_set (handle_set_t *p)
{
  DuplicateHandle (GetCurrentProcess (), get_handle (),
		   GetCurrentProcess (), &p->input_handle,
		   0, FALSE, DUPLICATE_SAME_ACCESS);
  DuplicateHandle (GetCurrentProcess (), get_output_handle (),
		   GetCurrentProcess (), &p->output_handle,
		   0, FALSE, DUPLICATE_SAME_ACCESS);
  DuplicateHandle (GetCurrentProcess (), input_mutex,
		   GetCurrentProcess (), &p->input_mutex,
		   0, FALSE, DUPLICATE_SAME_ACCESS);
  DuplicateHandle (GetCurrentProcess (), output_mutex,
		   GetCurrentProcess (), &p->output_mutex,
		   0, FALSE, DUPLICATE_SAME_ACCESS);
}

/* The function close_handle_set() should be static so that they can
   be called even after the fhandler_console instance is deleted. */
void
fhandler_console::close_handle_set (handle_set_t *p)
{
  CloseHandle (p->input_handle);
  p->input_handle = NULL;
  CloseHandle (p->output_handle);
  p->output_handle = NULL;
  CloseHandle (p->input_mutex);
  p->input_mutex = NULL;
  CloseHandle (p->output_mutex);
  p->output_mutex = NULL;
}
