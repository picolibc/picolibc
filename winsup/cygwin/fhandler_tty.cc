/* fhandler_tty.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/param.h>
#include <cygwin/acl.h>
#include <cygwin/kd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "sigproc.h"
#include "pinfo.h"
#include "ntdll.h"
#include "cygheap.h"
#include "shared_info.h"
#include "cygthread.h"
#include "child_info.h"
#include <asm/socket.h>
#include "cygwait.h"
#include "tls_pbuf.h"
#include "registry.h"

#define ALWAYS_USE_PCON false
#define USE_API_HOOK true

#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE 0x00020016
#endif /* PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE */

extern "C" int sscanf (const char *, const char *, ...);
extern "C" int ttyname_r (int, char*, size_t);

extern "C" {
  HRESULT WINAPI CreatePseudoConsole (COORD, HANDLE, HANDLE, DWORD, HPCON *);
  HRESULT WINAPI ResizePseudoConsole (HPCON, COORD);
  VOID WINAPI ClosePseudoConsole (HPCON);
}

#define close_maybe(h) \
  do { \
    if (h && h != INVALID_HANDLE_VALUE) \
      CloseHandle (h); \
  } while (0)

/* pty master control pipe messages */
struct pipe_request {
  DWORD pid;
};

struct pipe_reply {
  HANDLE from_master;
  HANDLE from_master_cyg;
  HANDLE to_master;
  HANDLE to_master_cyg;
  DWORD error;
};

static int pcon_attached_to = -1;
static bool isHybrid;
static bool do_not_reset_switch_to_pcon;
static bool freeconsole_on_close = true;

#if USE_API_HOOK
static void
set_switch_to_pcon (void)
{
  cygheap_fdenum cfd (false);
  int fd;
  while ((fd = cfd.next ()) >= 0)
    if (cfd->get_major () == DEV_PTYS_MAJOR)
      {
	fhandler_base *fh = cfd;
	fhandler_pty_slave *ptys = (fhandler_pty_slave *) fh;
	if (ptys->get_pseudo_console ())
	  ptys->set_switch_to_pcon (fd);
      }
}

static void
force_attach_to_pcon (HANDLE h)
{
  bool attach_done = false;
  for (int n = 0; n < 2; n ++)
    {
      /* First time, attach to the pty whose handle value is match.
	 Second time, try to attach to any pty. */
      cygheap_fdenum cfd (false);
      while (cfd.next () >= 0)
	if (cfd->get_major () == DEV_PTYS_MAJOR)
	  {
	    fhandler_base *fh = cfd;
	    fhandler_pty_slave *ptys = (fhandler_pty_slave *) fh;
	    if (!ptys->get_pseudo_console ())
	      continue;
	    if (n != 0
		|| h == ptys->get_handle ()
		|| h == ptys->get_output_handle ())
	      {
		if (fhandler_console::get_console_process_id
				  (ptys->get_helper_process_id (), true))
		  attach_done = true;
		else
		  {
		    FreeConsole ();
		    if (AttachConsole (ptys->get_helper_process_id ()))
		      {
			pcon_attached_to = ptys->get_minor ();
			attach_done = true;
		      }
		    else
		      pcon_attached_to = -1;
		  }
		break;
	      }
	  }
	else if (cfd->get_major () == DEV_CONS_MAJOR)
	  {
	    fhandler_base *fh = cfd;
	    fhandler_console *cons = (fhandler_console *) fh;
	    if (n != 0
		|| h == cons->get_handle ()
		|| h == cons->get_output_handle ())
	      {
		/* If the process is running on a console,
		   the parent process should be attached
		   to the same console. */
		DWORD attach_wpid;
		if (myself->ppid == 1)
		  attach_wpid = ATTACH_PARENT_PROCESS;
		else
		  {
		    pinfo p (myself->ppid);
		    attach_wpid = p->dwProcessId;
		  }
		FreeConsole ();
		if (AttachConsole (attach_wpid))
		  {
		    pcon_attached_to = -1;
		    attach_done = true;
		  }
		else
		  pcon_attached_to = -1;
		break;
	      }
	  }
      if (attach_done)
	break;
    }
}

void
set_ishybrid_and_switch_to_pcon (HANDLE h)
{
  if (GetFileType (h) == FILE_TYPE_CHAR)
    {
      force_attach_to_pcon (h);
      DWORD dummy;
      if (!isHybrid && (GetConsoleMode (h, &dummy)
			|| GetLastError () != ERROR_INVALID_HANDLE))
	{
	  isHybrid = true;
	  set_switch_to_pcon ();
	}
    }
}

#define DEF_HOOK(name) static __typeof__ (name) *name##_Orig
DEF_HOOK (WriteFile);
DEF_HOOK (WriteConsoleA);
DEF_HOOK (WriteConsoleW);
DEF_HOOK (ReadFile);
DEF_HOOK (ReadConsoleA);
DEF_HOOK (ReadConsoleW);
DEF_HOOK (WriteConsoleOutputA);
DEF_HOOK (WriteConsoleOutputW);
DEF_HOOK (WriteConsoleOutputCharacterA);
DEF_HOOK (WriteConsoleOutputCharacterW);
DEF_HOOK (WriteConsoleOutputAttribute);
DEF_HOOK (SetConsoleCursorPosition);
DEF_HOOK (SetConsoleTextAttribute);
DEF_HOOK (WriteConsoleInputA);
DEF_HOOK (WriteConsoleInputW);
DEF_HOOK (ReadConsoleInputA);
DEF_HOOK (ReadConsoleInputW);
DEF_HOOK (PeekConsoleInputA);
DEF_HOOK (PeekConsoleInputW);
/* CreateProcess() is hooked for GDB etc. */
DEF_HOOK (CreateProcessA);
DEF_HOOK (CreateProcessW);

static BOOL WINAPI
WriteFile_Hooked
     (HANDLE h, LPCVOID p, DWORD l, LPDWORD n, LPOVERLAPPED o)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteFile_Orig (h, p, l, n, o);
}
static BOOL WINAPI
WriteConsoleA_Hooked
     (HANDLE h, LPCVOID p, DWORD l, LPDWORD n, LPVOID o)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleA_Orig (h, p, l, n, o);
}
static BOOL WINAPI
WriteConsoleW_Hooked
     (HANDLE h, LPCVOID p, DWORD l, LPDWORD n, LPVOID o)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleW_Orig (h, p, l, n, o);
}
static BOOL WINAPI
ReadFile_Hooked
     (HANDLE h, LPVOID p, DWORD l, LPDWORD n, LPOVERLAPPED o)
{
  set_ishybrid_and_switch_to_pcon (h);
  return ReadFile_Orig (h, p, l, n, o);
}
static BOOL WINAPI
ReadConsoleA_Hooked
     (HANDLE h, LPVOID p, DWORD l, LPDWORD n, LPVOID o)
{
  set_ishybrid_and_switch_to_pcon (h);
  return ReadConsoleA_Orig (h, p, l, n, o);
}
static BOOL WINAPI
ReadConsoleW_Hooked
     (HANDLE h, LPVOID p, DWORD l, LPDWORD n, LPVOID o)
{
  set_ishybrid_and_switch_to_pcon (h);
  return ReadConsoleW_Orig (h, p, l, n, o);
}
static BOOL WINAPI
WriteConsoleOutputA_Hooked
     (HANDLE h, CONST CHAR_INFO *p, COORD s, COORD c, PSMALL_RECT r)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleOutputA_Orig (h, p, s, c, r);
}
static BOOL WINAPI
WriteConsoleOutputW_Hooked
     (HANDLE h, CONST CHAR_INFO *p, COORD s, COORD c, PSMALL_RECT r)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleOutputW_Orig (h, p, s, c, r);
}
static BOOL WINAPI
WriteConsoleOutputCharacterA_Hooked
     (HANDLE h, LPCSTR p, DWORD l, COORD c, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleOutputCharacterA_Orig (h, p, l, c, n);
}
static BOOL WINAPI
WriteConsoleOutputCharacterW_Hooked
     (HANDLE h, LPCWSTR p, DWORD l, COORD c, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleOutputCharacterW_Orig (h, p, l, c, n);
}
static BOOL WINAPI
WriteConsoleOutputAttribute_Hooked
     (HANDLE h, CONST WORD *a, DWORD l, COORD c, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleOutputAttribute_Orig (h, a, l, c, n);
}
static BOOL WINAPI
SetConsoleCursorPosition_Hooked
     (HANDLE h, COORD c)
{
  set_ishybrid_and_switch_to_pcon (h);
  return SetConsoleCursorPosition_Orig (h, c);
}
static BOOL WINAPI
SetConsoleTextAttribute_Hooked
     (HANDLE h, WORD a)
{
  set_ishybrid_and_switch_to_pcon (h);
  return SetConsoleTextAttribute_Orig (h, a);
}
static BOOL WINAPI
WriteConsoleInputA_Hooked
     (HANDLE h, CONST INPUT_RECORD *r, DWORD l, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleInputA_Orig (h, r, l, n);
}
static BOOL WINAPI
WriteConsoleInputW_Hooked
     (HANDLE h, CONST INPUT_RECORD *r, DWORD l, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return WriteConsoleInputW_Orig (h, r, l, n);
}
static BOOL WINAPI
ReadConsoleInputA_Hooked
     (HANDLE h, PINPUT_RECORD r, DWORD l, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return ReadConsoleInputA_Orig (h, r, l, n);
}
static BOOL WINAPI
ReadConsoleInputW_Hooked
     (HANDLE h, PINPUT_RECORD r, DWORD l, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return ReadConsoleInputW_Orig (h, r, l, n);
}
static BOOL WINAPI
PeekConsoleInputA_Hooked
     (HANDLE h, PINPUT_RECORD r, DWORD l, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return PeekConsoleInputA_Orig (h, r, l, n);
}
static BOOL WINAPI
PeekConsoleInputW_Hooked
     (HANDLE h, PINPUT_RECORD r, DWORD l, LPDWORD n)
{
  set_ishybrid_and_switch_to_pcon (h);
  return PeekConsoleInputW_Orig (h, r, l, n);
}
/* CreateProcess() is hooked for GDB etc. */
static BOOL WINAPI
CreateProcessA_Hooked
     (LPCSTR n, LPSTR c, LPSECURITY_ATTRIBUTES pa, LPSECURITY_ATTRIBUTES ta,
      BOOL inh, DWORD f, LPVOID e, LPCSTR d,
      LPSTARTUPINFOA si, LPPROCESS_INFORMATION pi)
{
  HANDLE h;
  if (si->dwFlags & STARTF_USESTDHANDLES)
    h = si->hStdOutput;
  else
    h = GetStdHandle (STD_OUTPUT_HANDLE);
  set_ishybrid_and_switch_to_pcon (h);
  return CreateProcessA_Orig (n, c, pa, ta, inh, f, e, d, si, pi);
}
static BOOL WINAPI
CreateProcessW_Hooked
     (LPCWSTR n, LPWSTR c, LPSECURITY_ATTRIBUTES pa, LPSECURITY_ATTRIBUTES ta,
      BOOL inh, DWORD f, LPVOID e, LPCWSTR d,
      LPSTARTUPINFOW si, LPPROCESS_INFORMATION pi)
{
  HANDLE h;
  if (si->dwFlags & STARTF_USESTDHANDLES)
    h = si->hStdOutput;
  else
    h = GetStdHandle (STD_OUTPUT_HANDLE);
  set_ishybrid_and_switch_to_pcon (h);
  return CreateProcessW_Orig (n, c, pa, ta, inh, f, e, d, si, pi);
}
#else /* USE_API_HOOK */
#define WriteFile_Orig 0
#define ReadFile_Orig 0
#define PeekConsoleInputA_Orig 0
void set_ishybrid_and_switch_to_pcon (HANDLE) {}
#endif /* USE_API_HOOK */

static char *
convert_mb_str (UINT cp_to, size_t *len_to,
		UINT cp_from, const char *ptr_from, size_t len_from)
{
  char *buf;
  size_t nlen;
  if (cp_to != cp_from)
    {
      size_t wlen =
	MultiByteToWideChar (cp_from, 0, ptr_from, len_from, NULL, 0);
      wchar_t *wbuf = (wchar_t *)
	HeapAlloc (GetProcessHeap (), 0, wlen * sizeof (wchar_t));
      wlen =
	MultiByteToWideChar (cp_from, 0, ptr_from, len_from, wbuf, wlen);
      nlen = WideCharToMultiByte (cp_to, 0, wbuf, wlen, NULL, 0, NULL, NULL);
      buf = (char *) HeapAlloc (GetProcessHeap (), 0, nlen);
      nlen = WideCharToMultiByte (cp_to, 0, wbuf, wlen, buf, nlen, NULL, NULL);
      HeapFree (GetProcessHeap (), 0, wbuf);
    }
  else
    {
      /* Just copy */
      buf = (char *) HeapAlloc (GetProcessHeap (), 0, len_from);
      memcpy (buf, ptr_from, len_from);
      nlen = len_from;
    }
  *len_to = nlen;
  return buf;
}

static void
mb_str_free (char *ptr)
{
  HeapFree (GetProcessHeap (), 0, ptr);
}

static bool
bytes_available (DWORD& n, HANDLE h)
{
  DWORD navail, nleft;
  navail = nleft = 0;
  bool succeeded = PeekNamedPipe (h, NULL, 0, NULL, &navail, &nleft);
  if (succeeded)
    /* nleft should always be the right choice unless something has written 0
       bytes to the pipe.  In that pathological case we return the actual number
       of bytes available in the pipe. See cgf-000008 for more details.  */
    n = nleft ?: navail;
  else
    {
      termios_printf ("PeekNamedPipe(%p) failed, %E", h);
      n = 0;
    }
  debug_only_printf ("n %u, nleft %u, navail %u", n, nleft, navail);
  return succeeded;
}

bool
fhandler_pty_common::bytes_available (DWORD &n)
{
  return ::bytes_available (n, get_handle_cyg ());
}

#ifdef DEBUGGING
static class mutex_stack
{
public:
  const char *fn;
  int ln;
  const char *tname;
} ostack[100];

static int osi;
#endif /*DEBUGGING*/

void
fhandler_pty_master::flush_to_slave ()
{
  if (get_readahead_valid () && !(get_ttyp ()->ti.c_lflag & ICANON))
    accept_input ();
}

DWORD
fhandler_pty_common::__acquire_output_mutex (const char *fn, int ln,
					     DWORD ms)
{
  if (strace.active ())
    strace.prntf (_STRACE_TERMIOS, fn, "(%d): pty output_mutex (%p): waiting %d ms", ln, output_mutex, ms);
  if (ms == INFINITE)
    ms = 100;
  DWORD res = WaitForSingleObject (output_mutex, ms);
  if (res == WAIT_OBJECT_0)
    {
#ifndef DEBUGGING
      if (strace.active ())
	strace.prntf (_STRACE_TERMIOS, fn, "(%d): pty output_mutex: acquired", ln, res);
#else
      ostack[osi].fn = fn;
      ostack[osi].ln = ln;
      ostack[osi].tname = mythreadname ();
      termios_printf ("acquired for %s:%d, osi %d", fn, ln, osi);
      osi++;
#endif
    }
  return res;
}

void
fhandler_pty_common::__release_output_mutex (const char *fn, int ln)
{
  if (ReleaseMutex (output_mutex))
    {
#ifndef DEBUGGING
      if (strace.active ())
	strace.prntf (_STRACE_TERMIOS, fn, "(%d): pty output_mutex(%p) released", ln, output_mutex);
#else
      if (osi > 0)
	osi--;
      termios_printf ("released(%p) at %s:%d, osi %d", output_mutex, fn, ln, osi);
      termios_printf ("  for %s:%d (%s)", ostack[osi].fn, ostack[osi].ln, ostack[osi].tname);
      ostack[osi].ln = -ln;
#endif
    }
#ifdef DEBUGGING
  else if (osi > 0)
    {
      system_printf ("couldn't release output mutex but we seem to own it, %E");
      try_to_debug ();
    }
#endif
}

/* Process pty input. */

void
fhandler_pty_master::doecho (const void *str, DWORD len)
{
  ssize_t towrite = len;
  acquire_output_mutex (INFINITE);
  if (!process_opost_output (echo_w, str, towrite, true))
    termios_printf ("Write to echo pipe failed, %E");
  release_output_mutex ();
}

int
fhandler_pty_master::accept_input ()
{
  DWORD bytes_left;
  int ret = 1;

  WaitForSingleObject (input_mutex, INFINITE);

  bytes_left = eat_readahead (-1);

  if (!bytes_left)
    {
      termios_printf ("sending EOF to slave");
      get_ttyp ()->read_retval = 0;
    }
  else
    {
      char *p = rabuf;
      DWORD rc;
      DWORD written = 0;

      paranoid_printf ("about to write %u chars to slave", bytes_left);
      rc = WriteFile (get_output_handle (), p, bytes_left, &written, NULL);
      if (!rc)
	{
	  debug_printf ("error writing to pipe %p %E", get_output_handle ());
	  get_ttyp ()->read_retval = -1;
	  ret = -1;
	}
      else
	{
	  get_ttyp ()->read_retval = 1;
	  p += written;
	  bytes_left -= written;
	  if (bytes_left > 0)
	    {
	      debug_printf ("to_slave pipe is full");
	      puts_readahead (p, bytes_left);
	      ret = 0;
	    }
	}
    }

  SetEvent (input_available_event);
  ReleaseMutex (input_mutex);
  return ret;
}

bool
fhandler_pty_master::hit_eof ()
{
  if (get_ttyp ()->was_opened && !get_ttyp ()->slave_alive ())
    {
      /* We have the only remaining open handle to this pty, and
	 the slave pty has been opened at least once.  We treat
	 this as EOF.  */
      termios_printf ("all other handles closed");
      return 1;
    }
  return 0;
}

/* Process pty output requests */

int
fhandler_pty_master::process_slave_output (char *buf, size_t len, int pktmode_on)
{
  size_t rlen;
  char outbuf[OUT_BUFFER_SIZE];
  DWORD n;
  DWORD echo_cnt;
  int rc = 0;

  flush_to_slave ();

  if (len == 0)
    goto out;

  for (;;)
    {
      n = echo_cnt = 0;
      for (;;)
	{
	  /* Check echo pipe first. */
	  if (::bytes_available (echo_cnt, echo_r) && echo_cnt > 0)
	    break;
	  if (!bytes_available (n))
	    goto err;
	  if (n)
	    break;
	  if (hit_eof ())
	    {
	      set_errno (EIO);
	      rc = -1;
	      goto out;
	    }
	  /* DISCARD (FLUSHO) and tcflush can finish here. */
	  if ((get_ttyp ()->ti.c_lflag & FLUSHO || !buf))
	    goto out;

	  if (is_nonblocking ())
	    {
	      set_errno (EAGAIN);
	      rc = -1;
	      goto out;
	    }
	  pthread_testcancel ();
	  if (cygwait (NULL, 10, cw_sig_eintr) == WAIT_SIGNALED
	      && !_my_tls.call_signal_handler ())
	    {
	      set_errno (EINTR);
	      rc = -1;
	      goto out;
	    }
	  flush_to_slave ();
	}

      /* Set RLEN to the number of bytes to read from the pipe.  */
      rlen = len;

      char *optr;
      optr = buf;
      if (pktmode_on && buf)
	{
	  *optr++ = TIOCPKT_DATA;
	  rlen -= 1;
	}

      if (rlen == 0)
	{
	  rc = optr - buf;
	  goto out;
	}

      if (rlen > sizeof outbuf)
	rlen = sizeof outbuf;

      /* If echo pipe has data (something has been typed or pasted), prefer
         it over slave output. */
      if (echo_cnt > 0)
	{
	  if (!ReadFile (echo_r, outbuf, rlen, &n, NULL))
	    {
	      termios_printf ("ReadFile on echo pipe failed, %E");
	      goto err;
	    }
	}
      else if (!ReadFile (get_handle (), outbuf, rlen, &n, NULL))
	{
	  termios_printf ("ReadFile failed, %E");
	  goto err;
	}

      termios_printf ("bytes read %u", n);

      if (get_ttyp ()->ti.c_lflag & FLUSHO || !buf)
	continue;

      memcpy (optr, outbuf, n);
      optr += n;
      rc = optr - buf;
      break;

    err:
      if (GetLastError () == ERROR_BROKEN_PIPE)
	rc = 0;
      else
	{
	  __seterrno ();
	  rc = -1;
	}
      break;
    }

out:
  termios_printf ("returning %d", rc);
  return rc;
}

/* pty slave stuff */

fhandler_pty_slave::fhandler_pty_slave (int unit)
  : fhandler_pty_common (), inuse (NULL), output_handle_cyg (NULL),
  io_handle_cyg (NULL), pid_restore (0), fd (-1)
{
  if (unit >= 0)
    dev ().parse (DEV_PTYS_MAJOR, unit);
}

fhandler_pty_slave::~fhandler_pty_slave ()
{
  if (!get_ttyp ())
    /* Why comes here? Who clears _tc? */
    return;
  if (get_pseudo_console ())
    {
      int used = 0;
      int attached = 0;
      cygheap_fdenum cfd (false);
      while (cfd.next () >= 0)
	{
	  if (cfd->get_major () == DEV_PTYS_MAJOR ||
	      cfd->get_major () == DEV_CONS_MAJOR)
	    used ++;
	  if (cfd->get_major () == DEV_PTYS_MAJOR &&
	      cfd->get_minor () == pcon_attached_to)
	    attached ++;
	}

      /* Call FreeConsole() if no tty is opened and the process
	 is attached to console corresponding to tty. This is
	 needed to make GNU screen and tmux work in Windows 10
	 1903. */
      if (attached == 0)
	pcon_attached_to = -1;
      if (used == 0)
	{
	  init_console_handler (false);
	  if (freeconsole_on_close)
	    FreeConsole ();
	}
    }
}

int
fhandler_pty_slave::open (int flags, mode_t)
{
  HANDLE pty_owner;
  HANDLE from_master_local, from_master_cyg_local;
  HANDLE to_master_local, to_master_cyg_local;
  HANDLE *handles[] =
  {
    &from_master_local, &input_available_event, &input_mutex, &inuse,
    &output_mutex, &to_master_local, &pty_owner, &to_master_cyg_local,
    &from_master_cyg_local,
    NULL
  };

  for (HANDLE **h = handles; *h; h++)
    **h = NULL;

  _tc = cygwin_shared->tty[get_minor ()];

  tcinit (false);

  cygwin_shared->tty.attach (get_minor ());

  /* Create synchronisation events */
  char buf[MAX_PATH];

  const char *errmsg = NULL;

  if (!(output_mutex = get_ttyp ()->open_output_mutex (MAXIMUM_ALLOWED)))
    {
      errmsg = "open output mutex failed, %E";
      goto err;
    }
  if (!(input_mutex = get_ttyp ()->open_input_mutex (MAXIMUM_ALLOWED)))
    {
      errmsg = "open input mutex failed, %E";
      goto err;
    }
  shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
  if (!(input_available_event = OpenEvent (MAXIMUM_ALLOWED, TRUE, buf)))
    {
      errmsg = "open input event failed, %E";
      goto err;
    }

  /* FIXME: Needs a method to eliminate tty races */
  {
    /* Create security attribute.  Default permissions are 0620. */
    security_descriptor sd;
    sd.malloc (sizeof (SECURITY_DESCRIPTOR));
    RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
    SECURITY_ATTRIBUTES sa = { sizeof (SECURITY_ATTRIBUTES), NULL, TRUE };
    if (!create_object_sd_from_attribute (myself->uid, myself->gid,
					  S_IFCHR | S_IRUSR | S_IWUSR | S_IWGRP,
					  sd))
      sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) sd;
    acquire_output_mutex (INFINITE);
    inuse = get_ttyp ()->create_inuse (&sa);
    get_ttyp ()->was_opened = true;
    release_output_mutex ();
  }

  if (!get_ttyp ()->from_master () || !get_ttyp ()->from_master_cyg () ||
      !get_ttyp ()->to_master () || !get_ttyp ()->to_master_cyg ())
    {
      errmsg = "pty handles have been closed";
      set_errno (EACCES);
      goto err_no_errno;
    }

  /* Three case for duplicating the pipe handles:
     - Either we're the master.  In this case, just duplicate the handles.
     - Or, we have the right to open the master process for handle duplication.
       In this case, just duplicate the handles.
     - Or, we have to ask the master process itself.  In this case, send our
       pid to the master process and check the reply.  The reply contains
       either the handles, or an error code which tells us why we didn't
       get the handles. */
  if (myself->pid == get_ttyp ()->master_pid)
    {
      /* This is the most common case, just calling openpty. */
      termios_printf ("dup handles within myself.");
      pty_owner = GetCurrentProcess ();
    }
  else
    {
      pinfo p (get_ttyp ()->master_pid);
      if (!p)
	termios_printf ("*** couldn't find pty master");
      else
	{
	  pty_owner = OpenProcess (PROCESS_DUP_HANDLE, FALSE, p->dwProcessId);
	  if (pty_owner)
	    termios_printf ("dup handles directly since I'm the owner");
	}
    }
  if (pty_owner)
    {
      if (!DuplicateHandle (pty_owner, get_ttyp ()->from_master (),
			    GetCurrentProcess (), &from_master_local, 0, TRUE,
			    DUPLICATE_SAME_ACCESS))
	{
	  termios_printf ("can't duplicate input from %u/%p, %E",
			  get_ttyp ()->master_pid, get_ttyp ()->from_master ());
	  __seterrno ();
	  goto err_no_msg;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->from_master_cyg (),
			    GetCurrentProcess (), &from_master_cyg_local, 0, TRUE,
			    DUPLICATE_SAME_ACCESS))
	{
	  termios_printf ("can't duplicate input from %u/%p, %E",
			  get_ttyp ()->master_pid, get_ttyp ()->from_master_cyg ());
	  __seterrno ();
	  goto err_no_msg;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->to_master (),
			  GetCurrentProcess (), &to_master_local, 0, TRUE,
			  DUPLICATE_SAME_ACCESS))
	{
	  errmsg = "can't duplicate output, %E";
	  goto err;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->to_master_cyg (),
			  GetCurrentProcess (), &to_master_cyg_local, 0, TRUE,
			  DUPLICATE_SAME_ACCESS))
	{
	  errmsg = "can't duplicate output for cygwin, %E";
	  goto err;
	}
      if (pty_owner != GetCurrentProcess ())
	CloseHandle (pty_owner);
    }
  else
    {
      pipe_request req = { GetCurrentProcessId () };
      pipe_reply repl;
      DWORD len;

      __small_sprintf (buf, "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
		       &cygheap->installation_key, get_minor ());
      termios_printf ("dup handles via master control pipe %s", buf);
      if (!CallNamedPipe (buf, &req, sizeof req, &repl, sizeof repl,
			  &len, 500))
	{
	  errmsg = "can't call master, %E";
	  goto err;
	}
      from_master_local = repl.from_master;
      from_master_cyg_local = repl.from_master_cyg;
      to_master_local = repl.to_master;
      to_master_cyg_local = repl.to_master_cyg;
      if (!from_master_local || !from_master_cyg_local ||
	  !to_master_local || !to_master_cyg_local)
	{
	  SetLastError (repl.error);
	  errmsg = "error duplicating pipes, %E";
	  goto err;
	}
    }
  VerifyHandle (from_master_local);
  VerifyHandle (from_master_cyg_local);
  VerifyHandle (to_master_local);
  VerifyHandle (to_master_cyg_local);

  termios_printf ("duplicated from_master %p->%p from pty_owner",
		  get_ttyp ()->from_master (), from_master_local);
  termios_printf ("duplicated from_master_cyg %p->%p from pty_owner",
		  get_ttyp ()->from_master_cyg (), from_master_cyg_local);
  termios_printf ("duplicated to_master %p->%p from pty_owner",
		  get_ttyp ()->to_master (), to_master_local);
  termios_printf ("duplicated to_master_cyg %p->%p from pty_owner",
		  get_ttyp ()->to_master_cyg (), to_master_cyg_local);

  set_handle (from_master_local);
  set_handle_cyg (from_master_cyg_local);
  set_output_handle (to_master_local);
  set_output_handle_cyg (to_master_cyg_local);

  if (!get_pseudo_console ())
    {
      fhandler_console::need_invisible ();
      pcon_attached_to = -1;
    }
  else if (!fhandler_console::get_console_process_id
			       (GetCurrentProcessId (), true))
    {
      fhandler_console::need_invisible ();
      pcon_attached_to = -1;
    }
  else if (fhandler_console::get_console_process_id
			       (get_helper_process_id (), true))
    /* Attached to pcon of this pty */
    {
      pcon_attached_to = get_minor ();
      init_console_handler (true);
    }

  set_open_status ();
  return 1;

err:
  if (GetLastError () == ERROR_FILE_NOT_FOUND)
    set_errno (ENXIO);
  else
    __seterrno ();
err_no_errno:
  termios_printf (errmsg);
err_no_msg:
  for (HANDLE **h = handles; *h; h++)
    if (**h && **h != INVALID_HANDLE_VALUE)
      CloseHandle (**h);
  return 0;
}

void
fhandler_pty_slave::open_setup (int flags)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  myself->set_ctty (this, flags);
  report_tty_counts (this, "opened", "");
  fhandler_base::open_setup (flags);
}

void
fhandler_pty_slave::cleanup ()
{
  /* This used to always call fhandler_pty_common::close when we were execing
     but that caused multiple closes of the handles associated with this pty.
     Since close_all_files is not called until after the cygwin process has
     synced or before a non-cygwin process has exited, it should be safe to
     just close this normally.  cgf 2006-05-20 */
  report_tty_counts (this, "closed", "");
  fhandler_base::cleanup ();
}

int
fhandler_pty_slave::close ()
{
  termios_printf ("closing last open %s handle", ttyname ());
  if (inuse && !CloseHandle (inuse))
    termios_printf ("CloseHandle (inuse), %E");
  if (!ForceCloseHandle (input_available_event))
    termios_printf ("CloseHandle (input_available_event<%p>), %E", input_available_event);
  if (!ForceCloseHandle (get_output_handle_cyg ()))
    termios_printf ("CloseHandle (get_output_handle_cyg ()<%p>), %E",
	get_output_handle_cyg ());
  if (!ForceCloseHandle (get_handle_cyg ()))
    termios_printf ("CloseHandle (get_handle_cyg ()<%p>), %E",
	get_handle_cyg ());
  if (!get_pseudo_console () &&
      (unsigned) myself->ctty == FHDEV (DEV_PTYS_MAJOR, get_minor ()))
    fhandler_console::free_console ();	/* assumes that we are the last pty closer */
  fhandler_pty_common::close ();
  if (!ForceCloseHandle (output_mutex))
    termios_printf ("CloseHandle (output_mutex<%p>), %E", output_mutex);
  if (pcon_attached_to == get_minor ())
    get_ttyp ()->num_pcon_attached_slaves --;
  get_ttyp ()->mask_switch_to_pcon_in = false;
  return 0;
}

int
fhandler_pty_slave::init (HANDLE h, DWORD a, mode_t)
{
  int flags = 0;

  a &= GENERIC_READ | GENERIC_WRITE;
  if (a == GENERIC_READ)
    flags = O_RDONLY;
  if (a == GENERIC_WRITE)
    flags = O_WRONLY;
  if (a == (GENERIC_READ | GENERIC_WRITE))
    flags = O_RDWR;

  int ret = open_with_arch (flags);

  if (ret && !cygwin_finished_initializing && !being_debugged ())
    {
      /* This only occurs when called from dtable::init_std_file_from_handle
	 We have been started from a non-Cygwin process.  So we should become
	 pty process group leader.
	 TODO: Investigate how SIGTTIN should be handled with pure-windows
	 programs. */
      pinfo p (tc ()->getpgid ());
      /* We should only grab this when the process group owner for this
	 pty is a non-cygwin process or we've been started directly
	 from a non-Cygwin process with no Cygwin ancestry.  */
      if (!p || ISSTATE (p, PID_NOTCYGWIN))
	{
	  termios_printf ("Setting process group leader to %d since %W(%d) is not a cygwin process",
			  myself->pgid, p->progname, p->pid);
	  tc ()->setpgid (myself->pgid);
	}
    }

  if (h != INVALID_HANDLE_VALUE)
    CloseHandle (h);	/* Reopened by open */

  return ret;
}

bool
fhandler_pty_slave::try_reattach_pcon (void)
{
  pid_restore = 0;

  /* Do not detach from the console because re-attaching will
     fail if helper process is running as service account. */
  if (get_ttyp()->attach_pcon_in_fork)
    return false;
  if (pcon_attached_to >= 0 &&
      cygwin_shared->tty[pcon_attached_to]->attach_pcon_in_fork)
    return false;

  pid_restore =
    fhandler_console::get_console_process_id (GetCurrentProcessId (),
					      false);
  /* If pid_restore is not set, give up. */
  if (!pid_restore)
    return false;

  FreeConsole ();
  if (!AttachConsole (get_helper_process_id ()))
    {
      system_printf ("pty%d: AttachConsole(helper=%d) failed. 0x%08lx",
		     get_minor (), get_helper_process_id (), GetLastError ());
      return false;
    }
  return true;
}

void
fhandler_pty_slave::restore_reattach_pcon (void)
{
  if (pid_restore)
    {
      FreeConsole ();
      if (!AttachConsole (pid_restore))
	{
	  system_printf ("pty%d: AttachConsole(restore=%d) failed. 0x%08lx",
			 get_minor (), pid_restore, GetLastError ());
	  pcon_attached_to = -1;
	}
    }
  pid_restore = 0;
}

void
fhandler_pty_slave::set_switch_to_pcon (int fd_set)
{
  if (fd < 0)
    fd = fd_set;
  if (!isHybrid)
    {
      reset_switch_to_pcon ();
      return;
    }
  if (fd == 0 && !get_ttyp ()->switch_to_pcon_in)
    {
      pid_restore = 0;
      if (pcon_attached_to != get_minor ())
	if (!try_reattach_pcon ())
	  goto skip_console_setting;
      FlushConsoleInputBuffer (get_handle ());
      DWORD mode;
      mode = ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
      SetConsoleMode (get_handle (), mode);
skip_console_setting:
      restore_reattach_pcon ();
      if (get_ttyp ()->pcon_pid == 0 ||
	  kill (get_ttyp ()->pcon_pid, 0) != 0)
	get_ttyp ()->pcon_pid = myself->pid;
      get_ttyp ()->switch_to_pcon_in = true;
    }
  else if ((fd == 1 || fd == 2) && !get_ttyp ()->switch_to_pcon_out)
    {
      cygwait (get_ttyp ()->fwd_done, INFINITE);
      if (get_ttyp ()->pcon_pid == 0 ||
	  kill (get_ttyp ()->pcon_pid, 0) != 0)
	get_ttyp ()->pcon_pid = myself->pid;
      get_ttyp ()->switch_to_pcon_out = true;
    }
}

void
fhandler_pty_slave::reset_switch_to_pcon (void)
{
  if (isHybrid)
    this->set_switch_to_pcon (fd);
  if (get_ttyp ()->pcon_pid &&
      get_ttyp ()->pcon_pid != myself->pid &&
      kill (get_ttyp ()->pcon_pid, 0) == 0)
    /* There is a process which is grabbing pseudo console. */
    return;
  if (isHybrid)
    {
      if (ALWAYS_USE_PCON)
	{
	  DWORD mode;
	  GetConsoleMode (get_handle (), &mode);
	  mode |= ENABLE_ECHO_INPUT;
	  mode |= ENABLE_LINE_INPUT;
	  mode &= ~ENABLE_PROCESSED_INPUT;
	  SetConsoleMode (get_handle (), mode);
	}
      get_ttyp ()->pcon_pid = 0;
      init_console_handler (true);
      return;
    }
  if (do_not_reset_switch_to_pcon)
    return;
  if (get_ttyp ()->switch_to_pcon_in)
    {
      DWORD mode;
      GetConsoleMode (get_handle (), &mode);
      SetConsoleMode (get_handle (), mode & ~ENABLE_ECHO_INPUT);
    }
  if (get_ttyp ()->switch_to_pcon_out)
    /* Wait for pty_master_fwd_thread() */
    cygwait (get_ttyp ()->fwd_done, INFINITE);
  get_ttyp ()->pcon_pid = 0;
  get_ttyp ()->switch_to_pcon_in = false;
  get_ttyp ()->switch_to_pcon_out = false;
  init_console_handler (true);
}

void
fhandler_pty_slave::push_to_pcon_screenbuffer (const char *ptr, size_t len)
{
  bool attached =
    !!fhandler_console::get_console_process_id (get_helper_process_id (), true);
  if (!attached && pcon_attached_to == get_minor ())
    {
      for (DWORD t0 = GetTickCount (); GetTickCount () - t0 < 100; )
	{
	  Sleep (1);
	  attached = fhandler_console::get_console_process_id
				      (get_helper_process_id (), true);
	  if (attached)
	    break;
	}
      if (!attached)
	{
	  system_printf ("pty%d: pcon_attach_to mismatch??????", get_minor ());
	  return;
	}
    }

  /* If not attached to this pseudo console, try to attach temporarily. */
  pid_restore = 0;
  if (pcon_attached_to != get_minor ())
    if (!try_reattach_pcon ())
      goto detach;

  char *buf;
  size_t nlen;
  DWORD origCP;
  origCP = GetConsoleOutputCP ();
  SetConsoleOutputCP (get_ttyp ()->term_code_page);
  /* Just copy */
  buf = (char *) HeapAlloc (GetProcessHeap (), 0, len);
  memcpy (buf, (char *)ptr, len);
  nlen = len;
  char *p0, *p1;
  p0 = p1 = buf;
  /* Remove alternate screen buffer drawing */
  while (p0 && p1)
    {
      if (!get_ttyp ()->screen_alternated)
	{
	  /* Check switching to alternate screen buffer */
	  p0 = (char *) memmem (p1, nlen - (p1-buf), "\033[?1049h", 8);
	  if (p0)
	    {
	      //p0 += 8;
	      get_ttyp ()->screen_alternated = true;
	      if (get_ttyp ()->switch_to_pcon_out)
		do_not_reset_switch_to_pcon = true;
	    }
	}
      if (get_ttyp ()->screen_alternated)
	{
	  /* Check switching to main screen buffer */
	  p1 = (char *) memmem (p0, nlen - (p0-buf), "\033[?1049l", 8);
	  if (p1)
	    {
	      p1 += 8;
	      get_ttyp ()->screen_alternated = false;
	      do_not_reset_switch_to_pcon = false;
	      memmove (p0, p1, buf+nlen - p1);
	      nlen -= p1 - p0;
	    }
	  else
	    nlen = p0 - buf;
	}
    }
  if (!nlen) /* Nothing to be synchronized */
    goto cleanup;
  if (get_ttyp ()->switch_to_pcon_out)
    goto cleanup;
  /* Remove ESC sequence which returns results to console
     input buffer. Without this, cursor position report
     is put into the input buffer as a garbage. */
  /* Remove ESC sequence to report cursor position. */
  while ((p0 = (char *) memmem (buf, nlen, "\033[6n", 4)))
    {
      memmove (p0, p0+4, nlen - (p0+4 - buf));
      nlen -= 4;
    }
  /* Remove ESC sequence to report terminal identity. */
  while ((p0 = (char *) memmem (buf, nlen, "\033[0c", 4)))
    {
      memmove (p0, p0+4, nlen - (p0+4 - buf));
      nlen -= 4;
    }

  /* If the ESC sequence ESC[?3h or ESC[?3l which clears console screen
     buffer is pushed, set need_redraw_screen to trigger redraw screen. */
  p0 = buf;
  while ((p0 = (char *) memmem (p0, nlen - (p0 - buf), "\033[?", 3)))
    {
      p0 += 3;
      bool exist_arg_3 = false;
      while (p0 < buf + nlen && (isdigit (*p0) || *p0 == ';'))
	{
	  int arg = 0;
	  while (p0 < buf + nlen && isdigit (*p0))
	    arg = arg * 10 + (*p0 ++) - '0';
	  if (arg == 3)
	    exist_arg_3 = true;
	  if (p0 < buf + nlen && *p0 == ';')
	    p0 ++;
	}
      if (p0 < buf + nlen && exist_arg_3 && (*p0 == 'h' || *p0 == 'l'))
	get_ttyp ()->need_redraw_screen = true;
      p0 ++;
      if (p0 >= buf + nlen)
	break;
    }

  int retry_count;
  retry_count = 0;
  DWORD dwMode, flags;
  flags = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  while (!GetConsoleMode (get_output_handle (), &dwMode))
    {
      termios_printf ("GetConsoleMode failed, %E");
      /* Re-open handles */
      this->close ();
      this->open (0, 0);
      /* Fix pseudo console window size */
      this->ioctl (TIOCSWINSZ, &get_ttyp ()->winsize);
      if (++retry_count > 3)
	goto cleanup;
    }
  if (!(get_ttyp ()->ti.c_oflag & OPOST) ||
      !(get_ttyp ()->ti.c_oflag & ONLCR))
    flags |= DISABLE_NEWLINE_AUTO_RETURN;
  SetConsoleMode (get_output_handle (), dwMode | flags);
  char *p;
  p = buf;
  DWORD wLen, written;
  written = 0;
  BOOL (WINAPI *WriteFunc)
    (HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
  WriteFunc = WriteFile_Orig ? WriteFile_Orig : WriteFile;
  while (written <  nlen)
    {
      if (!WriteFunc (get_output_handle (), p, nlen - written, &wLen, NULL))
	{
	  termios_printf ("WriteFile failed, %E");
	  break;
	}
      else
	{
	  written += wLen;
	  p += wLen;
	}
    }
  /* Detach from pseudo console and resume. */
  flags = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode (get_output_handle (), dwMode | flags);
cleanup:
  SetConsoleOutputCP (origCP);
  HeapFree (GetProcessHeap (), 0, buf);
detach:
  restore_reattach_pcon ();
}

ssize_t __stdcall
fhandler_pty_slave::write (const void *ptr, size_t len)
{
  ssize_t towrite = len;

  bg_check_types bg = bg_check (SIGTTOU);
  if (bg <= bg_eof)
    return (ssize_t) bg;

  termios_printf ("pty%d, write(%p, %lu)", get_minor (), ptr, len);

  push_process_state process_state (PID_TTYOU);

  reset_switch_to_pcon ();

  UINT target_code_page = get_ttyp ()->switch_to_pcon_out ?
    GetConsoleOutputCP () : get_ttyp ()->term_code_page;
  ssize_t nlen;
  char *buf = convert_mb_str (target_code_page, (size_t *) &nlen,
		 get_ttyp ()->term_code_page, (const char *) ptr, len);

  /* If not attached to this pseudo console, try to attach temporarily. */
  pid_restore = 0;
  bool fallback = false;
  if (get_ttyp ()->switch_to_pcon_out && pcon_attached_to != get_minor ())
    if (!try_reattach_pcon ())
      fallback = true;

  DWORD dwMode, flags;
  flags = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!(get_ttyp ()->ti.c_oflag & OPOST) ||
      !(get_ttyp ()->ti.c_oflag & ONLCR))
    flags |= DISABLE_NEWLINE_AUTO_RETURN;
  if (get_ttyp ()->switch_to_pcon_out && !fallback)
    {
      GetConsoleMode (get_output_handle (), &dwMode);
      SetConsoleMode (get_output_handle (), dwMode | flags);
    }
  HANDLE to = (get_ttyp ()->switch_to_pcon_out && !fallback) ?
    get_output_handle () : get_output_handle_cyg ();
  acquire_output_mutex (INFINITE);
  if (!process_opost_output (to, buf, nlen, false))
    {
      DWORD err = GetLastError ();
      termios_printf ("WriteFile failed, %E");
      switch (err)
	{
	case ERROR_NO_DATA:
	  err = ERROR_IO_DEVICE;
	  /*FALLTHRU*/
	default:
	  __seterrno_from_win_error (err);
	}
      towrite = -1;
    }
  release_output_mutex ();
  mb_str_free (buf);
  flags = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (get_ttyp ()->switch_to_pcon_out && !fallback)
    SetConsoleMode (get_output_handle (), dwMode | flags);

  restore_reattach_pcon ();

  /* Push slave output to pseudo console screen buffer */
  if (get_pseudo_console ())
    {
      acquire_output_mutex (INFINITE);
      push_to_pcon_screenbuffer ((char *)ptr, len);
      release_output_mutex ();
    }

  return towrite;
}

bool
fhandler_pty_common::to_be_read_from_pcon (void)
{
  return get_ttyp ()->switch_to_pcon_in &&
    (!get_ttyp ()->mask_switch_to_pcon_in || ALWAYS_USE_PCON);
}

void __reg3
fhandler_pty_slave::read (void *ptr, size_t& len)
{
  char *ptr0 = (char *)ptr;
  ssize_t totalread = 0;
  int vmin = 0;
  int vtime = 0;	/* Initialized to prevent -Wuninitialized warning */
  size_t readlen;
  DWORD bytes_in_pipe;
  char buf[INP_BUFFER_SIZE];
  DWORD time_to_wait;

  bg_check_types bg = bg_check (SIGTTIN);
  if (bg <= bg_eof)
    {
      len = (size_t) bg;
      return;
    }

  termios_printf ("read(%p, %lu) handle %p", ptr, len, get_handle_cyg ());

  push_process_state process_state (PID_TTYIN);

  if (ptr) /* Indicating not tcflush(). */
    {
      reset_switch_to_pcon ();
      mask_switch_to_pcon_in (true);
    }

  if (is_nonblocking () || !ptr) /* Indicating tcflush(). */
    time_to_wait = 0;
  else if ((get_ttyp ()->ti.c_lflag & ICANON))
    time_to_wait = INFINITE;
  else
    {
      vmin = get_ttyp ()->ti.c_cc[VMIN];
      if (vmin > INP_BUFFER_SIZE)
	vmin = INP_BUFFER_SIZE;
      vtime = get_ttyp ()->ti.c_cc[VTIME];
      if (vmin < 0)
	vmin = 0;
      if (vtime < 0)
	vtime = 0;
      if (!vmin && !vtime)
	time_to_wait = 0;
      else
	time_to_wait = !vtime ? INFINITE : 100 * vtime;
    }

  while (len)
    {
      switch (cygwait (input_available_event, time_to_wait))
	{
	case WAIT_OBJECT_0:
	  break;
	case WAIT_SIGNALED:
	  if (totalread > 0)
	    goto out;
	  termios_printf ("wait catched signal");
	  set_sig_errno (EINTR);
	  totalread = -1;
	  goto out;
	case WAIT_CANCELED:
	  process_state.pop ();
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	case WAIT_TIMEOUT:
	  termios_printf ("wait timed out, time_to_wait %u", time_to_wait);
	  /* No error condition when called from tcflush. */
	  if (!totalread && ptr)
	    {
	      set_sig_errno (EAGAIN);
	      totalread = -1;
	    }
	  goto out;
	default:
	  termios_printf ("wait for input event failed, %E");
	  if (!totalread)
	    {
	      __seterrno ();
	      totalread = -1;
	    }
	  goto out;
	}
      /* Now that we know that input is available we have to grab the
	 input mutex. */
      switch (cygwait (input_mutex, 1000))
	{
	case WAIT_OBJECT_0:
	case WAIT_ABANDONED_0:
	  break;
	case WAIT_SIGNALED:
	  if (totalread > 0)
	    goto out;
	  termios_printf ("wait for mutex caught signal");
	  set_sig_errno (EINTR);
	  totalread = -1;
	  goto out;
	case WAIT_CANCELED:
	  process_state.pop ();
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	case WAIT_TIMEOUT:
	  termios_printf ("failed to acquire input mutex after input event "
			  "arrived");
	  /* If we have a timeout, we can simply handle this failure to
	     grab the mutex as an EAGAIN situation.  Otherwise, if this
	     is an infinitely blocking read, restart the loop. */
	  if (time_to_wait != INFINITE)
	    {
	      if (!totalread)
		{
		  set_sig_errno (EAGAIN);
		  totalread = -1;
		}
	      goto out;
	    }
	  continue;
	default:
	  termios_printf ("wait for input mutex failed, %E");
	  if (!totalread)
	    {
	      __seterrno ();
	      totalread = -1;
	    }
	  goto out;
	}
      if (to_be_read_from_pcon ())
	{
	  if (!try_reattach_pcon ())
	    {
	      restore_reattach_pcon ();
	      goto do_read_cyg;
	    }

	  DWORD dwMode;
	  GetConsoleMode (get_handle (), &dwMode);
	  DWORD flags = ENABLE_VIRTUAL_TERMINAL_INPUT;
	  if (get_ttyp ()->ti.c_lflag & ECHO)
	    flags |= ENABLE_ECHO_INPUT;
	  if (get_ttyp ()->ti.c_lflag & ICANON)
	    flags |= ENABLE_LINE_INPUT;
	  if (flags & ENABLE_ECHO_INPUT && !(flags & ENABLE_LINE_INPUT))
	    flags &= ~ENABLE_ECHO_INPUT;
	  if ((get_ttyp ()->ti.c_lflag & ISIG) &&
	      !(get_ttyp ()->ti.c_iflag & IGNBRK))
	    flags |= ALWAYS_USE_PCON ? 0 : ENABLE_PROCESSED_INPUT;
	  if (dwMode != flags)
	    SetConsoleMode (get_handle (), flags);
	  /* Read get_handle() instad of get_handle_cyg() */
	  BOOL (WINAPI *ReadFunc)
	    (HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
	  ReadFunc = ReadFile_Orig ? ReadFile_Orig : ReadFile;
	  DWORD rlen;
	  if (!ReadFunc (get_handle (), ptr, len, &rlen, NULL))
	    {
	      termios_printf ("read failed, %E");
	      ReleaseMutex (input_mutex);
	      set_errno (EIO);
	      totalread = -1;
	      goto out;
	    }
	  INPUT_RECORD inp[128];
	  DWORD n;
	  BOOL (WINAPI *PeekFunc)
	    (HANDLE, PINPUT_RECORD, DWORD, LPDWORD);
	  PeekFunc =
	    PeekConsoleInputA_Orig ? PeekConsoleInputA_Orig : PeekConsoleInput;
	  PeekFunc (get_handle (), inp, 128, &n);
	  bool pipe_empty = true;
	  while (n-- > 0)
	    if (inp[n].EventType == KEY_EVENT && inp[n].Event.KeyEvent.bKeyDown)
	      pipe_empty = false;
	  if (pipe_empty)
	    ResetEvent (input_available_event);
	  ReleaseMutex (input_mutex);
	  len = rlen;

	  restore_reattach_pcon ();
	  mask_switch_to_pcon_in (false);
	  return;
	}

do_read_cyg:
      if (!bytes_available (bytes_in_pipe))
	{
	  ReleaseMutex (input_mutex);
	  set_errno (EIO);
	  totalread = -1;
	  goto out;
	}

      if (ptr && !bytes_in_pipe && !vmin && !time_to_wait)
	{
	  ReleaseMutex (input_mutex);
	  mask_switch_to_pcon_in (false);
	  len = (size_t) bytes_in_pipe;
	  return;
	}

      readlen = bytes_in_pipe ? MIN (len, sizeof (buf)) : 0;
      if (get_ttyp ()->ti.c_lflag & ICANON && ptr)
	readlen = MIN (bytes_in_pipe, readlen);

#if 0
      /* Why on earth is the read length reduced to vmin, even if more bytes
	 are available *and* len is bigger *and* the local buf is big enough?
	 Disable this code for now, it looks like a remnant of old. */
      if (ptr && vmin && readlen > (unsigned) vmin)
	readlen = vmin;
#endif

      DWORD n = 0;
      if (readlen)
	{
	  termios_printf ("reading %lu bytes (vtime %d)", readlen, vtime);
	  if (!ReadFile (get_handle_cyg (), buf, readlen, &n, NULL))
	    {
	      termios_printf ("read failed, %E");
	      ReleaseMutex (input_mutex);
	      set_errno (EIO);
	      totalread = -1;
	      goto out;
	    }
	  else
	    {
	      /* MSDN states that 5th prameter can be used to determine total
		 number of bytes in pipe, but for some reason this number doesn't
		 change after successful read. So we have to peek into the pipe
		 again to see if input is still available */
	      if (!bytes_available (bytes_in_pipe))
		{
		  ReleaseMutex (input_mutex);
		  set_errno (EIO);
		  totalread = -1;
		  goto out;
		}
	      if (n)
		{
		  if (!(!ptr && len == UINT_MAX)) /* not tcflush() */
		    len -= n;
		  totalread += n;
		  if (ptr)
		    {
		      memcpy (ptr, buf, n);
		      ptr = (char *) ptr + n;
		    }
		}
	    }
	}

      if (!bytes_in_pipe)
	ResetEvent (input_available_event);

      ReleaseMutex (input_mutex);

      if (!ptr)
	{
	  if (!bytes_in_pipe)
	    break;
	  continue;
	}

      if (get_ttyp ()->read_retval < 0)	// read error
	{
	  set_errno (-get_ttyp ()->read_retval);
	  totalread = -1;
	  break;
	}
      if (get_ttyp ()->read_retval == 0)	//EOF
	{
	  termios_printf ("saw EOF");
	  break;
	}
      if (get_ttyp ()->ti.c_lflag & ICANON || is_nonblocking ())
	break;
      if (vmin && totalread >= vmin)
	break;

      /* vmin == 0 && vtime == 0:
       *   we've already read all input, if any, so return immediately
       * vmin == 0 && vtime > 0:
       *   we've waited for input 10*vtime ms in WFSO(input_available_event),
       *   no matter whether any input arrived, we shouldn't wait any longer,
       *   so return immediately
       * vmin > 0 && vtime == 0:
       *   here, totalread < vmin, so continue waiting until more data
       *   arrive
       * vmin > 0 && vtime > 0:
       *   similar to the previous here, totalread < vmin, and timer
       *   hadn't expired -- WFSO(input_available_event) != WAIT_TIMEOUT,
       *   so "restart timer" and wait until more data arrive
       */

      if (vmin == 0)
	break;
    }
out:
  termios_printf ("%d = read(%p, %lu)", totalread, ptr, len);
  len = (size_t) totalread;
  /* Push slave read as echo to pseudo console screen buffer. */
  if (get_pseudo_console () && ptr0 && (get_ttyp ()->ti.c_lflag & ECHO))
    {
      acquire_output_mutex (INFINITE);
      push_to_pcon_screenbuffer (ptr0, len);
      release_output_mutex ();
    }
  mask_switch_to_pcon_in (false);
}

int
fhandler_pty_slave::dup (fhandler_base *child, int flags)
{
  /* This code was added in Oct 2001 for some undisclosed reason.
     However, setting the controlling tty on a dup causes rxvt to
     hang when the parent does a dup since the controlling pgid changes.
     Specifically testing for -2 (ctty has been setsid'ed) works around
     this problem.  However, it's difficult to see scenarios in which you
     have a dup'able fd, no controlling tty, and not having run setsid.
     So, we might want to consider getting rid of the set_ctty in tty-like dup
     methods entirely at some point */
  if (myself->ctty != -2)
    myself->set_ctty (this, flags);
  report_tty_counts (child, "duped slave", "");
  return 0;
}

int
fhandler_pty_master::dup (fhandler_base *child, int)
{
  report_tty_counts (child, "duped master", "");
  return 0;
}

int
fhandler_pty_slave::tcgetattr (struct termios *t)
{
  reset_switch_to_pcon ();
  *t = get_ttyp ()->ti;
  return 0;
}

int
fhandler_pty_slave::tcsetattr (int, const struct termios *t)
{
  reset_switch_to_pcon ();
  acquire_output_mutex (INFINITE);
  get_ttyp ()->ti = *t;
  release_output_mutex ();
  return 0;
}

int
fhandler_pty_slave::tcflush (int queue)
{
  int ret = 0;

  termios_printf ("tcflush(%d) handle %p", queue, get_handle_cyg ());

  reset_switch_to_pcon ();

  if (queue == TCIFLUSH || queue == TCIOFLUSH)
    {
      size_t len = UINT_MAX;
      read (NULL, len);
      ret = ((int) len) >= 0 ? 0 : -1;
    }
  if (queue == TCOFLUSH || queue == TCIOFLUSH)
    {
      /* do nothing for now. */
    }

  termios_printf ("%d=tcflush(%d)", ret, queue);
  return ret;
}

int
fhandler_pty_slave::ioctl (unsigned int cmd, void *arg)
{
  termios_printf ("ioctl (%x)", cmd);
  reset_switch_to_pcon ();
  int res = fhandler_termios::ioctl (cmd, arg);
  if (res <= 0)
    return res;

  if (myself->pgid && get_ttyp ()->getpgid () != myself->pgid
      && (unsigned) myself->ctty == FHDEV (DEV_PTYS_MAJOR, get_minor ())
      && (get_ttyp ()->ti.c_lflag & TOSTOP))
    {
      /* background process */
      termios_printf ("bg ioctl pgid %d, tpgid %d, %s", myself->pgid,
		      get_ttyp ()->getpgid (), myctty ());
      raise (SIGTTOU);
    }

  int retval;
  switch (cmd)
    {
    case TIOCGWINSZ:
    case TIOCSWINSZ:
      break;
    case TIOCGPGRP:
      {
	pid_t pid = this->tcgetpgrp ();
	if (pid < 0)
	  retval = -1;
	else
	  {
	    *((pid_t *) arg) = pid;
	    retval = 0;
	  }
      }
      goto out;
    case TIOCSPGRP:
      retval = this->tcsetpgrp ((pid_t) (intptr_t) arg);
      goto out;
    case FIONREAD:
      {
	DWORD n;
	if (!bytes_available (n))
	  {
	    set_errno (EINVAL);
	    retval = -1;
	  }
	else
	  {
	    *(int *) arg = (int) n;
	    retval = 0;
	  }
      }
      goto out;
    default:
      return fhandler_base::ioctl (cmd, arg);
    }

  acquire_output_mutex (INFINITE);

  get_ttyp ()->cmd = cmd;
  get_ttyp ()->ioctl_retval = 0;
  switch (cmd)
    {
    case TIOCGWINSZ:
      get_ttyp ()->arg.winsize = get_ttyp ()->winsize;
      *(struct winsize *) arg = get_ttyp ()->arg.winsize;
      get_ttyp ()->winsize = get_ttyp ()->arg.winsize;
      break;
    case TIOCSWINSZ:
      if (get_pseudo_console ())
	{
	  /* If not attached to this pseudo console,
	     try to attach temporarily. */
	  pid_restore = 0;
	  if (pcon_attached_to != get_minor ())
	    if (!try_reattach_pcon ())
	      goto cleanup;

	  COORD size;
	  size.X = ((struct winsize *) arg)->ws_col;
	  size.Y = ((struct winsize *) arg)->ws_row;
	  CONSOLE_SCREEN_BUFFER_INFO csbi;
	  if (GetConsoleScreenBufferInfo (get_output_handle (), &csbi))
	    if (size.X == csbi.srWindow.Right - csbi.srWindow.Left + 1 &&
		size.Y == csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
	      goto cleanup;
	  if (!SetConsoleScreenBufferSize (get_output_handle (), size))
	    goto cleanup;
	  SMALL_RECT rect;
	  rect.Left = 0;
	  rect.Top = 0;
	  rect.Right = size.X-1;
	  rect.Bottom = size.Y-1;
	  SetConsoleWindowInfo (get_output_handle (), TRUE, &rect);
cleanup:
	  restore_reattach_pcon ();
	}

      if (get_ttyp ()->winsize.ws_row != ((struct winsize *) arg)->ws_row
	  || get_ttyp ()->winsize.ws_col != ((struct winsize *) arg)->ws_col)
	{
	  get_ttyp ()->arg.winsize = *(struct winsize *) arg;
	  get_ttyp ()->winsize = *(struct winsize *) arg;
	  get_ttyp ()->kill_pgrp (SIGWINCH);
	}
      break;
    }

  release_output_mutex ();
  retval = get_ttyp ()->ioctl_retval;
  if (retval < 0)
    {
      set_errno (-retval);
      retval = -1;
    }

out:
  termios_printf ("%d = ioctl(%x)", retval, cmd);
  return retval;
}

int __reg2
fhandler_pty_slave::fstat (struct stat *st)
{
  fhandler_base::fstat (st);

  bool to_close = false;
  if (!input_available_event)
    {
      char buf[MAX_PATH];
      shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
      input_available_event = OpenEvent (READ_CONTROL, TRUE, buf);
      if (input_available_event)
	to_close = true;
    }
  st->st_mode = S_IFCHR;
  if (!input_available_event
      || get_object_attribute (input_available_event, &st->st_uid, &st->st_gid,
			       &st->st_mode))
    {
      /* If we can't access the ACL, or if the tty doesn't actually exist,
	 then fake uid and gid to strict, system-like values. */
      st->st_mode = S_IFCHR | S_IRUSR | S_IWUSR;
      st->st_uid = 18;
      st->st_gid = 544;
    }
  if (to_close)
    CloseHandle (input_available_event);
  return 0;
}

int __reg3
fhandler_pty_slave::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  int res = -1;
  bool to_close = false;
  security_descriptor sd;
  mode_t attr = S_IFCHR;

  switch (cmd)
    {
      case SETACL:
	if (!aclsort32 (nentries, 0, aclbufp))
	  set_errno (ENOTSUP);
	break;
      case GETACL:
	if (!aclbufp)
	  {
	    set_errno (EFAULT);
	    break;
	  }
	/*FALLTHRU*/
      case GETACLCNT:
	if (!input_available_event)
	  {
	    char buf[MAX_PATH];
	    shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
	    input_available_event = OpenEvent (READ_CONTROL, TRUE, buf);
	    if (input_available_event)
	      to_close = true;
	  }
	if (!input_available_event
	    || get_object_sd (input_available_event, sd))
	  {
	    res = get_posix_access (NULL, &attr, NULL, NULL, aclbufp, nentries);
	    if (aclbufp && res == MIN_ACL_ENTRIES)
	      {
		aclbufp[0].a_perm = S_IROTH | S_IWOTH;
		aclbufp[0].a_id = 18;
		aclbufp[1].a_id = 544;
	      }
	    break;
	  }
	if (cmd == GETACL)
	  res = get_posix_access (sd, &attr, NULL, NULL, aclbufp, nentries);
	else
	  res = get_posix_access (sd, &attr, NULL, NULL, NULL, 0);
	break;
      default:
	set_errno (EINVAL);
	break;
    }
  if (to_close)
    CloseHandle (input_available_event);
  return res;
}

/* Helper function for fchmod and fchown, which just opens all handles
   and signals success via bool return. */
bool
fhandler_pty_slave::fch_open_handles (bool chown)
{
  char buf[MAX_PATH];
  DWORD write_access = WRITE_DAC | (chown ? WRITE_OWNER : 0);

  _tc = cygwin_shared->tty[get_minor ()];
  shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
  input_available_event = OpenEvent (READ_CONTROL | write_access,
				     TRUE, buf);
  output_mutex = get_ttyp ()->open_output_mutex (write_access);
  input_mutex = get_ttyp ()->open_input_mutex (write_access);
  inuse = get_ttyp ()->open_inuse (write_access);
  if (!input_available_event || !output_mutex || !input_mutex || !inuse)
    {
      __seterrno ();
      return false;
    }
  return true;
}

/* Helper function for fchmod and fchown, which sets the new security
   descriptor on all objects representing the pty. */
int
fhandler_pty_slave::fch_set_sd (security_descriptor &sd, bool chown)
{
  security_descriptor sd_old;

  get_object_sd (input_available_event, sd_old);
  if (!set_object_sd (input_available_event, sd, chown)
      && !set_object_sd (output_mutex, sd, chown)
      && !set_object_sd (input_mutex, sd, chown)
      && !set_object_sd (inuse, sd, chown))
    return 0;
  set_object_sd (input_available_event, sd_old, chown);
  set_object_sd (output_mutex, sd_old, chown);
  set_object_sd (input_mutex, sd_old, chown);
  set_object_sd (inuse, sd_old, chown);
  return -1;
}

/* Helper function for fchmod and fchown, which closes all object handles in
   the pty. */
void
fhandler_pty_slave::fch_close_handles ()
{
  close_maybe (input_available_event);
  close_maybe (output_mutex);
  close_maybe (input_mutex);
  close_maybe (inuse);
}

int __reg1
fhandler_pty_slave::fchmod (mode_t mode)
{
  int ret = -1;
  bool to_close = false;
  security_descriptor sd;
  uid_t uid;
  gid_t gid;
  mode_t orig_mode = S_IFCHR;

  if (!input_available_event)
    {
      to_close = true;
      if (!fch_open_handles (false))
	goto errout;
    }
  sd.malloc (sizeof (SECURITY_DESCRIPTOR));
  RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
  if (!get_object_attribute (input_available_event, &uid, &gid, &orig_mode)
      && !create_object_sd_from_attribute (uid, gid, S_IFCHR | mode, sd))
    ret = fch_set_sd (sd, false);
errout:
  if (to_close)
    fch_close_handles ();
  return ret;
}

int __reg2
fhandler_pty_slave::fchown (uid_t uid, gid_t gid)
{
  int ret = -1;
  bool to_close = false;
  security_descriptor sd;
  uid_t o_uid;
  gid_t o_gid;
  mode_t mode = S_IFCHR;

  if (uid == ILLEGAL_UID && gid == ILLEGAL_GID)
    return 0;
  if (!input_available_event)
    {
      to_close = true;
      if (!fch_open_handles (true))
	goto errout;
    }
  sd.malloc (sizeof (SECURITY_DESCRIPTOR));
  RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
  if (!get_object_attribute (input_available_event, &o_uid, &o_gid, &mode))
    {
      if (uid == ILLEGAL_UID)
	uid = o_uid;
      if (gid == ILLEGAL_GID)
	gid = o_gid;
      if (uid == o_uid && gid == o_gid)
	ret = 0;
      else if (!create_object_sd_from_attribute (uid, gid, mode, sd))
	ret = fch_set_sd (sd, true);
    }
errout:
  if (to_close)
    fch_close_handles ();
  return ret;
}

/*******************************************************
 fhandler_pty_master
*/
fhandler_pty_master::fhandler_pty_master (int unit)
  : fhandler_pty_common (), pktmode (0), master_ctl (NULL),
    master_thread (NULL), from_master (NULL), to_master (NULL),
    from_slave (NULL), to_slave (NULL), echo_r (NULL), echo_w (NULL),
    dwProcessId (0), to_master_cyg (NULL), from_master_cyg (NULL),
    master_fwd_thread (NULL)
{
  if (unit >= 0)
    dev ().parse (DEV_PTYM_MAJOR, unit);
  else if (!setup ())
    {
      dev ().parse (FH_ERROR);
      return;
    }
  set_name ("/dev/ptmx");
}

int
fhandler_pty_master::open (int flags, mode_t)
{
  set_open_status ();
  dwProcessId = GetCurrentProcessId ();
  return 1;
}

void
fhandler_pty_master::open_setup (int flags)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  char buf[sizeof ("opened pty master for ptyNNNNNNNNNNN")];
  __small_sprintf (buf, "opened pty master for pty%d", get_minor ());
  report_tty_counts (this, buf, "");
  fhandler_base::open_setup (flags);
}

off_t
fhandler_pty_common::lseek (off_t, int)
{
  set_errno (ESPIPE);
  return -1;
}

int
fhandler_pty_common::close ()
{
  termios_printf ("pty%d <%p,%p> closing",
		  get_minor (), get_handle (), get_output_handle ());
  if (!ForceCloseHandle (input_mutex))
    termios_printf ("CloseHandle (input_mutex<%p>), %E", input_mutex);
  if (!ForceCloseHandle1 (get_handle (), from_pty))
    termios_printf ("CloseHandle (get_handle ()<%p>), %E", get_handle ());
  if (!ForceCloseHandle1 (get_output_handle (), to_pty))
    termios_printf ("CloseHandle (get_output_handle ()<%p>), %E",
		    get_output_handle ());

  return 0;
}

void
fhandler_pty_master::cleanup ()
{
  report_tty_counts (this, "closing master", "");
  if (archetype)
    from_master = from_master_cyg =
      to_master = to_master_cyg = from_slave = to_slave = NULL;
  fhandler_base::cleanup ();
}

int
fhandler_pty_master::close ()
{
  OBJECT_BASIC_INFORMATION obi;
  NTSTATUS status;
  pid_t master_pid_tmp = get_ttyp ()->master_pid;

  termios_printf ("closing from_master(%p)/from_master_cyg(%p)/to_master(%p)/to_master_cyg(%p) since we own them(%u)",
		  from_master, from_master_cyg,
		  to_master, to_master_cyg, dwProcessId);
  if (cygwin_finished_initializing)
    {
      if (master_ctl && get_ttyp ()->master_pid == myself->pid)
	{
	  char buf[MAX_PATH];
	  pipe_request req = { (DWORD) -1 };
	  pipe_reply repl;
	  DWORD len;

	  __small_sprintf (buf, "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
			   &cygheap->installation_key, get_minor ());
	  acquire_output_mutex (INFINITE);
	  if (master_ctl)
	    {
	      CallNamedPipe (buf, &req, sizeof req, &repl, sizeof repl, &len,
			     500);
	      CloseHandle (master_ctl);
	      master_thread->detach ();
	      get_ttyp ()->set_master_ctl_closed ();
	      master_ctl = NULL;
	    }
	  release_output_mutex ();
	  master_fwd_thread->terminate_thread ();
	  CloseHandle (get_ttyp ()->fwd_done);
	  get_ttyp ()->fwd_done = NULL;
	}
    }

  /* Check if the last master handle has been closed.  If so, set
     input_available_event to wake up potentially waiting slaves. */
  acquire_output_mutex (INFINITE);
  status = NtQueryObject (get_output_handle (), ObjectBasicInformation,
			  &obi, sizeof obi, NULL);
  fhandler_pty_common::close ();
  release_output_mutex ();
  if (!ForceCloseHandle (output_mutex))
    termios_printf ("CloseHandle (output_mutex<%p>), %E", output_mutex);
  if (!NT_SUCCESS (status))
    debug_printf ("NtQueryObject: %y", status);
  else if (obi.HandleCount == 1)
    {
      termios_printf ("Closing last master of pty%d", get_minor ());
      /* Close Pseudo Console */
      if (get_pseudo_console ())
	{
	  /* Terminate helper process */
	  SetEvent (get_ttyp ()->h_helper_goodbye);
	  WaitForSingleObject (get_ttyp ()->h_helper_process, INFINITE);
	  CloseHandle (get_ttyp ()->h_helper_goodbye);
	  CloseHandle (get_ttyp ()->h_helper_process);
	  /* FIXME: Pseudo console can be accessed via its handle
	     only in the process which created it. What else can we do? */
	  if (master_pid_tmp == myself->pid)
	    {
	      /* ClosePseudoConsole() seems to have a bug that one
		 internal handle remains opened. This causes handle leak.
		 This is a workaround for this problem. */
	      HPCON_INTERNAL *hp = (HPCON_INTERNAL *) get_pseudo_console ();
	      HANDLE tmp = hp->hConHostProcess;
	      /* Release pseudo console */
	      ClosePseudoConsole (get_pseudo_console ());
	      CloseHandle (tmp);
	    }
	  get_ttyp ()->switch_to_pcon_in = false;
	  get_ttyp ()->switch_to_pcon_out = false;
	}
      if (get_ttyp ()->getsid () > 0)
	kill (get_ttyp ()->getsid (), SIGHUP);
      SetEvent (input_available_event);
    }

  if (!ForceCloseHandle (from_master))
    termios_printf ("error closing from_master %p, %E", from_master);
  if (from_master_cyg != from_master) /* Avoid double close. */
    if (!ForceCloseHandle (from_master_cyg))
      termios_printf ("error closing from_master_cyg %p, %E", from_master_cyg);
  if (!ForceCloseHandle (to_master))
    termios_printf ("error closing to_master %p, %E", to_master);
  from_master = to_master = NULL;
  if (!ForceCloseHandle (from_slave))
    termios_printf ("error closing from_slave %p, %E", from_slave);
  from_slave = NULL;
  if (!ForceCloseHandle (to_master_cyg))
    termios_printf ("error closing to_master_cyg %p, %E", to_master_cyg);
  to_master_cyg = from_master_cyg = NULL;
  ForceCloseHandle (echo_r);
  ForceCloseHandle (echo_w);
  echo_r = echo_w = NULL;
  if (to_slave)
    ForceCloseHandle (to_slave);
  to_slave = NULL;

  if (have_execed || get_ttyp ()->master_pid != myself->pid)
    termios_printf ("not clearing: %d, master_pid %d",
		    have_execed, get_ttyp ()->master_pid);
  if (!ForceCloseHandle (input_available_event))
    termios_printf ("CloseHandle (input_available_event<%p>), %E",
		    input_available_event);

  return 0;
}

ssize_t __stdcall
fhandler_pty_master::write (const void *ptr, size_t len)
{
  ssize_t ret;
  char *p = (char *) ptr;
  termios ti = tc ()->ti;

  bg_check_types bg = bg_check (SIGTTOU);
  if (bg <= bg_eof)
    return (ssize_t) bg;

  push_process_state process_state (PID_TTYOU);

  /* Write terminal input to to_slave pipe instead of output_handle
     if current application is native console application. */
  if (to_be_read_from_pcon ())
    {
      size_t nlen;
      char *buf = convert_mb_str
	(CP_UTF8, &nlen, get_ttyp ()->term_code_page, (const char *) ptr, len);

      DWORD wLen;
      WriteFile (to_slave, buf, nlen, &wLen, NULL);

      if (ALWAYS_USE_PCON &&
	  (ti.c_lflag & ISIG) && memchr (p, ti.c_cc[VINTR], len))
	get_ttyp ()->kill_pgrp (SIGINT);

      if (ti.c_lflag & ICANON)
	{
	  if (memchr (buf, '\r', nlen))
	    SetEvent (input_available_event);
	}
      else
	SetEvent (input_available_event);

      mb_str_free (buf);
      return len;
    }

  if (get_ttyp ()->switch_to_pcon_in &&
      (ti.c_lflag & ISIG) &&
      memchr (p, ti.c_cc[VINTR], len) &&
      get_ttyp ()->getpgid () == get_ttyp ()->pcon_pid)
    {
      DWORD n;
      /* Send ^C to pseudo console as well */
      WriteFile (to_slave, "\003", 1, &n, 0);
    }

  line_edit_status status = line_edit (p++, len, ti, &ret);
  if (status > line_edit_signalled && status != line_edit_pipe_full)
    ret = -1;
  return ret;
}

void __reg3
fhandler_pty_master::read (void *ptr, size_t& len)
{
  bg_check_types bg = bg_check (SIGTTIN);
  if (bg <= bg_eof)
    {
      len = (size_t) bg;
      return;
    }
  push_process_state process_state (PID_TTYIN);
  len = (size_t) process_slave_output ((char *) ptr, len, pktmode);
}

int
fhandler_pty_master::tcgetattr (struct termios *t)
{
  *t = cygwin_shared->tty[get_minor ()]->ti;
  return 0;
}

int
fhandler_pty_master::tcsetattr (int, const struct termios *t)
{
  cygwin_shared->tty[get_minor ()]->ti = *t;
  return 0;
}

int
fhandler_pty_master::tcflush (int queue)
{
  int ret = 0;

  termios_printf ("tcflush(%d) handle %p", queue, get_handle ());

  if (queue == TCIFLUSH || queue == TCIOFLUSH)
    ret = process_slave_output (NULL, OUT_BUFFER_SIZE, 0);
  else if (queue == TCIFLUSH || queue == TCIOFLUSH)
    {
      /* do nothing for now. */
    }

  termios_printf ("%d=tcflush(%d)", ret, queue);
  return ret;
}

int
fhandler_pty_master::ioctl (unsigned int cmd, void *arg)
{
  int res = fhandler_termios::ioctl (cmd, arg);
  if (res <= 0)
    return res;

  switch (cmd)
    {
    case TIOCPKT:
      pktmode = *(int *) arg;
      break;
    case TIOCGWINSZ:
      *(struct winsize *) arg = get_ttyp ()->winsize;
      break;
    case TIOCSWINSZ:
      /* FIXME: Pseudo console can be accessed via its handle
	 only in the process which created it. What else can we do? */
      if (get_pseudo_console () && get_ttyp ()->master_pid == myself->pid)
	{
	  COORD size;
	  size.X = ((struct winsize *) arg)->ws_col;
	  size.Y = ((struct winsize *) arg)->ws_row;
	  ResizePseudoConsole (get_pseudo_console (), size);
	}
      if (get_ttyp ()->winsize.ws_row != ((struct winsize *) arg)->ws_row
	  || get_ttyp ()->winsize.ws_col != ((struct winsize *) arg)->ws_col)
	{
	  get_ttyp ()->winsize = *(struct winsize *) arg;
	  get_ttyp ()->kill_pgrp (SIGWINCH);
	}
      break;
    case TIOCGPGRP:
      *((pid_t *) arg) = this->tcgetpgrp ();
      break;
    case TIOCSPGRP:
      return this->tcsetpgrp ((pid_t) (intptr_t) arg);
    case FIONREAD:
      {
	DWORD n;
	if (!bytes_available (n))
	  {
	    set_errno (EINVAL);
	    return -1;
	  }
	*(int *) arg = (int) n;
      }
      break;
    default:
      return fhandler_base::ioctl (cmd, arg);
    }
  return 0;
}

int
fhandler_pty_master::ptsname_r (char *buf, size_t buflen)
{
  char tmpbuf[TTY_NAME_MAX];

  __ptsname (tmpbuf, get_minor ());
  if (buflen <= strlen (tmpbuf))
    {
      set_errno (ERANGE);
      return ERANGE;
    }
  strcpy (buf, tmpbuf);
  return 0;
}

void
fhandler_pty_common::set_close_on_exec (bool val)
{
  // Cygwin processes will handle this specially on exec.
  close_on_exec (val);
}

/* This table is borrowed from mintty: charset.c */
static const struct {
  UINT cp;
  const char *name;
}
cs_names[] = {
  { CP_UTF8, "UTF-8"},
  { CP_UTF8, "UTF8"},
  {   20127, "ASCII"},
  {   20127, "US-ASCII"},
  {   20127, "ANSI_X3.4-1968"},
  {   20866, "KOI8-R"},
  {   20866, "KOI8R"},
  {   20866, "KOI8"},
  {   21866, "KOI8-U"},
  {   21866, "KOI8U"},
  {   20932, "EUCJP"},
  {   20932, "EUC-JP"},
  {     874, "TIS620"},
  {     874, "TIS-620"},
  {     932, "SJIS"},
  {     936, "GBK"},
  {     936, "GB2312"},
  {     936, "EUCCN"},
  {     936, "EUC-CN"},
  {     949, "EUCKR"},
  {     949, "EUC-KR"},
  {     950, "BIG5"},
  {       0, "NULL"}
};

static void
get_locale_from_env (char *locale)
{
  const char *env = NULL;
  char lang[ENCODING_LEN + 1] = {0, }, country[ENCODING_LEN + 1] = {0, };
  env = getenv ("LC_ALL");
  if (env == NULL || !*env)
    env = getenv ("LC_CTYPE");
  if (env == NULL || !*env)
    env = getenv ("LANG");
  if (env == NULL || !*env)
    {
      if (GetLocaleInfo (LOCALE_CUSTOM_UI_DEFAULT,
			  LOCALE_SISO639LANGNAME,
			  lang, sizeof (lang)))
	GetLocaleInfo (LOCALE_CUSTOM_UI_DEFAULT,
		       LOCALE_SISO3166CTRYNAME,
		       country, sizeof (country));
      else if (GetLocaleInfo (LOCALE_CUSTOM_DEFAULT,
			      LOCALE_SISO639LANGNAME,
			      lang, sizeof (lang)))
	  GetLocaleInfo (LOCALE_CUSTOM_DEFAULT,
			 LOCALE_SISO3166CTRYNAME,
			 country, sizeof (country));
      else if (GetLocaleInfo (LOCALE_USER_DEFAULT,
			      LOCALE_SISO639LANGNAME,
			      lang, sizeof (lang)))
	  GetLocaleInfo (LOCALE_USER_DEFAULT,
			 LOCALE_SISO3166CTRYNAME,
			 country, sizeof (country));
      else if (GetLocaleInfo (LOCALE_SYSTEM_DEFAULT,
			      LOCALE_SISO639LANGNAME,
			      lang, sizeof (lang)))
	  GetLocaleInfo (LOCALE_SYSTEM_DEFAULT,
			 LOCALE_SISO3166CTRYNAME,
			 country, sizeof (country));
      if (strlen (lang) && strlen (country))
	__small_sprintf (lang + strlen (lang), "_%s.UTF-8", country);
      else
	strcpy (lang , "C.UTF-8");
      env = lang;
    }
  strcpy (locale, env);
}

static LCID
get_langinfo (char *locale_out, char *charset_out)
{
  /* Get locale from environment */
  char new_locale[ENCODING_LEN + 1];
  get_locale_from_env (new_locale);

  __locale_t loc;
  memset (&loc, 0, sizeof (loc));
  const char *locale = __loadlocale (&loc, LC_CTYPE, new_locale);
  if (!locale)
    locale = "C";

  char tmp_locale[ENCODING_LEN + 1];
  char *ret = __set_locale_from_locale_alias (locale, tmp_locale);
  if (ret)
    locale = tmp_locale;

  const char *charset;
  struct lc_ctype_T *lc_ctype = (struct lc_ctype_T *) loc.lc_cat[LC_CTYPE].ptr;
  if (!lc_ctype)
    charset = "ASCII";
  else
    charset = lc_ctype->codeset;

  /* The following code is borrowed from nl_langinfo()
     in newlib/libc/locale/nl_langinfo.c */
  /* Convert charset to Linux compatible codeset string. */
  if (charset[0] == 'A'/*SCII*/)
    charset = "ANSI_X3.4-1968";
  else if (charset[0] == 'E')
    {
      if (strcmp (charset, "EUCJP") == 0)
	charset = "EUC-JP";
      else if (strcmp (charset, "EUCKR") == 0)
	charset = "EUC-KR";
      else if (strcmp (charset, "EUCCN") == 0)
	charset = "GB2312";
    }
  else if (charset[0] == 'C'/*Pxxxx*/)
    {
      if (strcmp (charset + 2, "874") == 0)
	charset = "TIS-620";
      else if (strcmp (charset + 2, "20866") == 0)
	charset = "KOI8-R";
      else if (strcmp (charset + 2, "21866") == 0)
	charset = "KOI8-U";
      else if (strcmp (charset + 2, "101") == 0)
	charset = "GEORGIAN-PS";
      else if (strcmp (charset + 2, "102") == 0)
	charset = "PT154";
    }
  else if (charset[0] == 'S'/*JIS*/)
    {
      /* Cygwin uses MSFT's implementation of SJIS, which differs
	 in some codepoints from the real thing, especially
	 0x5c: yen sign instead of backslash,
	 0x7e: overline instead of tilde.
	 We can't use the real SJIS since otherwise Win32
	 pathnames would become invalid.  OTOH, if we return
	 "SJIS" here, then libiconv will do mb<->wc conversion
	 differently to our internal functions.  Therefore we
	 return what we really implement, CP932.  This is handled
	 fine by libiconv. */
      charset = "CP932";
    }

  wchar_t lc[ENCODING_LEN + 1];
  wchar_t *p;
  mbstowcs (lc, locale, ENCODING_LEN);
  p = wcschr (lc, L'.');
  if (p)
    *p = L'\0';
  p = wcschr (lc, L'_');
  if (p)
    *p = L'-';
  LCID lcid = LocaleNameToLCID (lc, 0);
  if (!lcid && !strcmp (charset, "ASCII"))
    return 0;

  /* Set results */
  strcpy (locale_out, new_locale);
  strcpy (charset_out, charset);
  return lcid;
}

void
fhandler_pty_slave::setup_locale (void)
{
  char locale[ENCODING_LEN + 1] = "C";
  char charset[ENCODING_LEN + 1] = "ASCII";
  LCID lcid = get_langinfo (locale, charset);

  /* Set console code page form locale */
  if (get_pseudo_console ())
    {
      UINT code_page;
      if (lcid == 0 || lcid == (LCID) -1)
	code_page = 20127; /* ASCII */
      else if (!GetLocaleInfo (lcid,
			       LOCALE_IDEFAULTCODEPAGE | LOCALE_RETURN_NUMBER,
			       (char *) &code_page, sizeof (code_page)))
	code_page = 20127; /* ASCII */
      SetConsoleCP (code_page);
      SetConsoleOutputCP (code_page);
    }

  /* Set terminal code page from locale */
  /* This code is borrowed from mintty: charset.c */
  get_ttyp ()->term_code_page = 20127; /* Default ASCII */
  char charset_u[ENCODING_LEN + 1] = {0, };
  for (int i=0; charset[i] && i<ENCODING_LEN; i++)
    charset_u[i] = toupper (charset[i]);
  unsigned int iso;
  UINT cp = 20127; /* Default for fallback */
  if (sscanf (charset_u, "ISO-8859-%u", &iso) == 1 ||
      sscanf (charset_u, "ISO8859-%u", &iso) == 1 ||
      sscanf (charset_u, "ISO8859%u", &iso) == 1)
    {
      if (iso && iso <= 16 && iso !=12)
	get_ttyp ()->term_code_page = 28590 + iso;
    }
  else if (sscanf (charset_u, "CP%u", &cp) == 1)
    get_ttyp ()->term_code_page = cp;
  else
    for (int i=0; cs_names[i].cp; i++)
      if (strcasecmp (charset_u, cs_names[i].name) == 0)
	{
	  get_ttyp ()->term_code_page = cs_names[i].cp;
	  break;
	}
}

void
fhandler_pty_slave::set_freeconsole_on_close (bool val)
{
  freeconsole_on_close = val;
}

void
fhandler_pty_slave::fixup_after_attach (bool native_maybe, int fd_set)
{
  if (fd < 0)
    fd = fd_set;
  if (get_pseudo_console ())
    {
      if (fhandler_console::get_console_process_id (get_helper_process_id (),
						    true))
	{
	  if (pcon_attached_to != get_minor ())
	    {
	      pcon_attached_to = get_minor ();
	      init_console_handler (true);
	    }
	  /* Clear screen to synchronize pseudo console screen buffer
	     with real terminal. This is necessary because pseudo
	     console screen buffer is empty at start. */
	  if (get_ttyp ()->num_pcon_attached_slaves == 0
	      && !ALWAYS_USE_PCON)
	    /* Assume this is the first process using this pty slave. */
	    get_ttyp ()->need_redraw_screen = true;

	  get_ttyp ()->num_pcon_attached_slaves ++;
	}

      if (ALWAYS_USE_PCON && !isHybrid && pcon_attached_to == get_minor ())
	set_ishybrid_and_switch_to_pcon (get_output_handle ());

      if (pcon_attached_to == get_minor () && native_maybe)
	{
	  if (fd == 0)
	    {
	      FlushConsoleInputBuffer (get_handle ());
	      DWORD mode =
		ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
	      SetConsoleMode (get_handle (), mode);
	      if (get_ttyp ()->pcon_pid == 0 ||
		  kill (get_ttyp ()->pcon_pid, 0) != 0)
		get_ttyp ()->pcon_pid = myself->pid;
	      get_ttyp ()->switch_to_pcon_in = true;
	    }
	  else if (fd == 1 || fd == 2)
	    {
	      DWORD mode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
	      SetConsoleMode (get_output_handle (), mode);
	      if (!get_ttyp ()->switch_to_pcon_out)
		cygwait (get_ttyp ()->fwd_done, INFINITE);
	      if (get_ttyp ()->pcon_pid == 0 ||
		  kill (get_ttyp ()->pcon_pid, 0) != 0)
		get_ttyp ()->pcon_pid = myself->pid;
	      get_ttyp ()->switch_to_pcon_out = true;

	      if (get_ttyp ()->need_redraw_screen)
		{
		  /* Forcibly redraw screen based on console screen buffer. */
		  /* The following code triggers redrawing the screen. */
		  CONSOLE_SCREEN_BUFFER_INFO sbi;
		  GetConsoleScreenBufferInfo (get_output_handle (), &sbi);
		  SMALL_RECT rect = {0, 0,
		    (SHORT) (sbi.dwSize.X -1), (SHORT) (sbi.dwSize.Y - 1)};
		  COORD dest = {0, 0};
		  CHAR_INFO fill = {' ', 0};
		  ScrollConsoleScreenBuffer (get_output_handle (),
					     &rect, NULL, dest, &fill);
		  get_ttyp ()->need_redraw_screen = false;
		}
	    }
	  init_console_handler (false);
	}
      else if (fd == 0 && native_maybe)
	/* Read from unattached pseudo console cause freeze,
	   therefore, fallback to legacy pty. */
	set_handle (get_handle_cyg ());
    }
}

void
fhandler_pty_slave::fixup_after_fork (HANDLE parent)
{
  fixup_after_attach (false, -1);
  // fork_fixup (parent, inuse, "inuse");
  // fhandler_pty_common::fixup_after_fork (parent);
  report_tty_counts (this, "inherited", "");
}

void
fhandler_pty_slave::fixup_after_exec ()
{
  /* Native windows program does not reset event on read.
     Therefore, reset here if no input is available. */
  DWORD bytes_in_pipe;
  if (bytes_available (bytes_in_pipe) && !bytes_in_pipe)
    ResetEvent (input_available_event);

  reset_switch_to_pcon ();

  if (!close_on_exec ())
    fixup_after_fork (NULL);
  else if (get_pseudo_console ())
    {
      int used = 0;
      int attached = 0;
      cygheap_fdenum cfd (false);
      while (cfd.next () >= 0)
	{
	  if (cfd->get_major () == DEV_PTYS_MAJOR ||
	      cfd->get_major () == DEV_CONS_MAJOR)
	    used ++;
	  if (cfd->get_major () == DEV_PTYS_MAJOR &&
	      cfd->get_minor () == pcon_attached_to)
	    attached ++;
	}

      /* Call FreeConsole() if no tty is opened and the process
	 is attached to console corresponding to tty. This is
	 needed to make GNU screen and tmux work in Windows 10
	 1903. */
      if (attached == 1 && get_minor () == pcon_attached_to)
	pcon_attached_to = -1;
      if (used == 1 /* About to close this tty */)
	{
	  init_console_handler (false);
	  if (freeconsole_on_close)
	    FreeConsole ();
	}
    }

  /* Set locale */
  if (get_ttyp ()->term_code_page == 0)
    setup_locale ();

#if USE_API_HOOK
  /* Hook Console API */
  if (get_pseudo_console ())
    {
#define DO_HOOK(module, name) \
      if (!name##_Orig) \
	{ \
	  void *api = hook_api (module, #name, (void *) name##_Hooked); \
	  name##_Orig = (__typeof__ (name) *) api; \
	  /*if (api) system_printf (#name " hooked.");*/ \
	}
      DO_HOOK (NULL, WriteFile);
      DO_HOOK (NULL, WriteConsoleA);
      DO_HOOK (NULL, WriteConsoleW);
      DO_HOOK (NULL, ReadFile);
      DO_HOOK (NULL, ReadConsoleA);
      DO_HOOK (NULL, ReadConsoleW);
      DO_HOOK (NULL, WriteConsoleOutputA);
      DO_HOOK (NULL, WriteConsoleOutputW);
      DO_HOOK (NULL, WriteConsoleOutputCharacterA);
      DO_HOOK (NULL, WriteConsoleOutputCharacterW);
      DO_HOOK (NULL, WriteConsoleOutputAttribute);
      DO_HOOK (NULL, SetConsoleCursorPosition);
      DO_HOOK (NULL, SetConsoleTextAttribute);
      DO_HOOK (NULL, WriteConsoleInputA);
      DO_HOOK (NULL, WriteConsoleInputW);
      DO_HOOK (NULL, ReadConsoleInputA);
      DO_HOOK (NULL, ReadConsoleInputW);
      DO_HOOK (NULL, PeekConsoleInputA);
      DO_HOOK (NULL, PeekConsoleInputW);
      /* CreateProcess() is hooked for GDB etc. */
      DO_HOOK (NULL, CreateProcessA);
      DO_HOOK (NULL, CreateProcessW);
    }
#endif /* USE_API_HOOK */
}

/* This thread function handles the master control pipe.  It waits for a
   client to connect.  Then it checks if the client process has permissions
   to access the tty handles.  If so, it opens the client process and
   duplicates the handles into that process.  If that fails, it sends a reply
   with at least one handle set to NULL and an error code.  Last but not
   least, the client is disconnected and the thread waits for the next client.

   A special case is when the master side of the tty is about to be closed.
   The client side is the fhandler_pty_master::close function and it sends
   a PID -1 in that case.  A check is performed that the request to leave
   really comes from the master process itself.

   Since there's always only one pipe instance, there's a chance that clients
   have to wait to connect to the master control pipe.  Therefore the client
   calls to CallNamedPipe should have a big enough timeout value.  For now this
   is 500ms.  Hope that's enough. */

DWORD
fhandler_pty_master::pty_master_thread ()
{
  bool exit = false;
  GENERIC_MAPPING map = { EVENT_QUERY_STATE, EVENT_MODIFY_STATE, 0,
			  EVENT_QUERY_STATE | EVENT_MODIFY_STATE };
  pipe_request req;
  DWORD len;
  security_descriptor sd;
  HANDLE token;
  PRIVILEGE_SET ps;
  DWORD pid;
  NTSTATUS status;

  termios_printf ("Entered");
  while (!exit && (ConnectNamedPipe (master_ctl, NULL)
		   || GetLastError () == ERROR_PIPE_CONNECTED))
    {
      pipe_reply repl = { NULL, NULL, NULL, 0 };
      bool deimp = false;
      NTSTATUS allow = STATUS_ACCESS_DENIED;
      ACCESS_MASK access = EVENT_MODIFY_STATE;
      HANDLE client = NULL;

      if (!ReadFile (master_ctl, &req, sizeof req, &len, NULL))
	{
	  termios_printf ("ReadFile, %E");
	  goto reply;
	}
      if (!GetNamedPipeClientProcessId (master_ctl, &pid))
	pid = req.pid;
      if (get_object_sd (input_available_event, sd))
	{
	  termios_printf ("get_object_sd, %E");
	  goto reply;
	}
      cygheap->user.deimpersonate ();
      deimp = true;
      if (!ImpersonateNamedPipeClient (master_ctl))
	{
	  termios_printf ("ImpersonateNamedPipeClient, %E");
	  goto reply;
	}
      status = NtOpenThreadToken (GetCurrentThread (), TOKEN_QUERY, TRUE,
				  &token);
      if (!NT_SUCCESS (status))
	{
	  termios_printf ("NtOpenThreadToken, %y", status);
	  SetLastError (RtlNtStatusToDosError (status));
	  goto reply;
	}
      len = sizeof ps;
      status = NtAccessCheck (sd, token, access, &map, &ps, &len, &access,
			      &allow);
      NtClose (token);
      if (!NT_SUCCESS (status))
	{
	  termios_printf ("NtAccessCheck, %y", status);
	  SetLastError (RtlNtStatusToDosError (status));
	  goto reply;
	}
      if (!RevertToSelf ())
	{
	  termios_printf ("RevertToSelf, %E");
	  goto reply;
	}
      if (req.pid == (DWORD) -1)	/* Request to finish thread. */
	{
	  /* Check if the requesting process is the master process itself. */
	  if (pid == GetCurrentProcessId ())
	    exit = true;
	  goto reply;
	}
      if (NT_SUCCESS (allow))
	{
	  client = OpenProcess (PROCESS_DUP_HANDLE, FALSE, pid);
	  if (!client)
	    {
	      termios_printf ("OpenProcess, %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), from_master,
			       client, &repl.from_master,
			       0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (from_master), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), from_master_cyg,
			       client, &repl.from_master_cyg,
			       0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (from_master_cyg), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), to_master,
				client, &repl.to_master,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_master), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), to_master_cyg,
				client, &repl.to_master_cyg,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_master_cyg), %E");
	      goto reply;
	    }
	}
reply:
      repl.error = GetLastError ();
      if (client)
	CloseHandle (client);
      if (deimp)
	cygheap->user.reimpersonate ();
      sd.free ();
      termios_printf ("Reply: from %p, to %p, error %u",
		      repl.from_master, repl.to_master, repl.error );
      if (!WriteFile (master_ctl, &repl, sizeof repl, &len, NULL))
	termios_printf ("WriteFile, %E");
      if (!DisconnectNamedPipe (master_ctl))
	termios_printf ("DisconnectNamedPipe, %E");
    }
  termios_printf ("Leaving");
  return 0;
}

static DWORD WINAPI
pty_master_thread (VOID *arg)
{
  return ((fhandler_pty_master *) arg)->pty_master_thread ();
}

DWORD
fhandler_pty_master::pty_master_fwd_thread ()
{
  DWORD rlen;
  char outbuf[65536];

  termios_printf ("Started.");
  for (;;)
    {
      if (::bytes_available (rlen, from_slave) && rlen == 0)
	SetEvent (get_ttyp ()->fwd_done);
      if (!ReadFile (from_slave, outbuf, sizeof outbuf, &rlen, NULL))
	{
	  termios_printf ("ReadFile for forwarding failed, %E");
	  break;
	}
      ResetEvent (get_ttyp ()->fwd_done);
      ssize_t wlen = rlen;
      char *ptr = outbuf;
      if (get_pseudo_console ())
	{
	  /* Avoid duplicating slave output which is already sent to
	     to_master_cyg */
	  if (!get_ttyp ()->switch_to_pcon_out)
	    continue;

	  /* Avoid setting window title to "cygwin-console-helper.exe" */
	  int state = 0;
	  int start_at = 0;
	  for (DWORD i=0; i<rlen; i++)
	    if (outbuf[i] == '\033')
	      {
		start_at = i;
		state = 1;
		continue;
	      }
	    else if ((state == 1 && outbuf[i] == ']') ||
		     (state == 2 && outbuf[i] == '0') ||
		     (state == 3 && outbuf[i] == ';'))
	      {
		state ++;
		continue;
	      }
	    else if (state == 4 && outbuf[i] == '\a')
	      {
		memmove (&outbuf[start_at], &outbuf[i+1], rlen-i-1);
		state = 0;
		rlen = wlen = start_at + rlen - i - 1;
		continue;
	      }
	    else if (outbuf[i] == '\a')
	      {
		state = 0;
		continue;
	      }

	  /* Remove ESC sequence which returns results to console
	     input buffer. Without this, cursor position report
	     is put into the input buffer as a garbage. */
	  /* Remove ESC sequence to report cursor position. */
	  char *p0;
	  while ((p0 = (char *) memmem (outbuf, rlen, "\033[6n", 4)))
	    {
	      memmove (p0, p0+4, rlen - (p0+4 - outbuf));
	      rlen -= 4;
	    }
	  /* Remove ESC sequence to report terminal identity. */
	  while ((p0 = (char *) memmem (outbuf, rlen, "\033[0c", 4)))
	    {
	      memmove (p0, p0+4, rlen - (p0+4 - outbuf));
	      rlen -= 4;
	    }
	  wlen = rlen;

	  size_t nlen;
	  char *buf = convert_mb_str
	    (get_ttyp ()->term_code_page, &nlen, CP_UTF8, ptr, wlen);

	  ptr = buf;
	  wlen = rlen = nlen;

	  /* OPOST processing was already done in pseudo console,
	     so just write it to to_master_cyg. */
	  DWORD written;
	  acquire_output_mutex (INFINITE);
	  while (rlen>0)
	    {
	      if (!WriteFile (to_master_cyg, ptr, wlen, &written, NULL))
		{
		  termios_printf ("WriteFile for forwarding failed, %E");
		  break;
		}
	      ptr += written;
	      wlen = (rlen -= written);
	    }
	  release_output_mutex ();
	  mb_str_free (buf);
	  continue;
	}
      size_t nlen;
      char *buf = convert_mb_str
	(get_ttyp ()->term_code_page, &nlen, GetConsoleOutputCP (), ptr, wlen);

      ptr = buf;
      wlen = rlen = nlen;
      acquire_output_mutex (INFINITE);
      while (rlen>0)
	{
	  if (!process_opost_output (to_master_cyg, ptr, wlen, false))
	    {
	      termios_printf ("WriteFile for forwarding failed, %E");
	      break;
	    }
	  ptr += wlen;
	  wlen = (rlen -= wlen);
	}
      release_output_mutex ();
      mb_str_free (buf);
    }
  return 0;
}

static DWORD WINAPI
pty_master_fwd_thread (VOID *arg)
{
  return ((fhandler_pty_master *) arg)->pty_master_fwd_thread ();
}

/* If master process is running as service, attaching to
   pseudo console should be done in fork. If attaching
   is done in spawn for inetd or sshd, it fails because
   the helper process is running as privileged user while
   slave process is not. This function is used to determine
   if the process is running as a srvice or not. */
inline static bool
is_running_as_service (void)
{
  return check_token_membership (well_known_service_sid)
    || cygheap->user.saved_sid () == well_known_system_sid;
}

bool
fhandler_pty_master::setup_pseudoconsole ()
{
  if (disable_pcon)
    return false;
  /* If the legacy console mode is enabled, pseudo console seems
     not to work as expected. To determine console mode, registry
     key ForceV2 in HKEY_CURRENT_USER\Console is checked. */
  reg_key reg (HKEY_CURRENT_USER, KEY_READ, L"Console", NULL);
  if (reg.error ())
    return false;
  if (reg.get_dword (L"ForceV2", 1) == 0)
    {
      termios_printf ("Pseudo console is disabled "
		      "because the legacy console mode is enabled.");
      return false;
    }

  /* Pseudo console supprot is realized using a tricky technic.
     PTY need the pseudo console handles, however, they cannot
     be retrieved by normal procedure. Therefore, run a helper
     process in a pseudo console and get them from the helper.
     Slave process will attach to the pseudo console in the
     helper process using AttachConsole(). */
  COORD size = {80, 25};
  CreatePipe (&from_master, &to_slave, &sec_none, 0);
  SetLastError (ERROR_SUCCESS);
  HRESULT res = CreatePseudoConsole (size, from_master, to_master,
				     0, &get_ttyp ()->h_pseudo_console);
  if (res != S_OK || GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      if (res != S_OK)
	system_printf ("CreatePseudoConsole() failed. %08x\n",
		       GetLastError ());
      CloseHandle (from_master);
      CloseHandle (to_slave);
      from_master = from_master_cyg;
      to_slave = NULL;
      get_ttyp ()->h_pseudo_console = NULL;
      return false;
    }

  /* If master process is running as service, attaching to
     pseudo console should be done in fork. If attaching
     is done in spawn for inetd or sshd, it fails because
     the helper process is running as privileged user while
     slave process is not. */
  if (is_running_as_service ())
    get_ttyp ()->attach_pcon_in_fork = true;

  SIZE_T bytesRequired;
  InitializeProcThreadAttributeList (NULL, 2, 0, &bytesRequired);
  STARTUPINFOEXW si_helper;
  ZeroMemory (&si_helper, sizeof (si_helper));
  si_helper.StartupInfo.cb = sizeof (STARTUPINFOEXW);
  si_helper.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)
    HeapAlloc (GetProcessHeap (), 0, bytesRequired);
  InitializeProcThreadAttributeList (si_helper.lpAttributeList,
				     2, 0, &bytesRequired);
  UpdateProcThreadAttribute (si_helper.lpAttributeList,
			     0,
			     PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
			     get_ttyp ()->h_pseudo_console,
			     sizeof (get_ttyp ()->h_pseudo_console),
			     NULL, NULL);
  HANDLE hello = CreateEvent (&sec_none, true, false, NULL);
  HANDLE goodbye = CreateEvent (&sec_none, true, false, NULL);
  /* Create a pipe for receiving pseudo console handles */
  HANDLE hr, hw;
  CreatePipe (&hr, &hw, &sec_none, 0);
  /* Inherit only handles which are needed by helper. */
  HANDLE handles_to_inherit[] = {hello, goodbye, hw};
  UpdateProcThreadAttribute (si_helper.lpAttributeList,
			     0,
			     PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
			     handles_to_inherit,
			     sizeof (handles_to_inherit),
			     NULL, NULL);
  /* Create helper process */
  WCHAR cmd[MAX_PATH];
  path_conv helper ("/bin/cygwin-console-helper.exe");
  size_t len = helper.get_wide_win32_path_len ();
  helper.get_wide_win32_path (cmd);
  __small_swprintf (cmd + len, L" %p %p %p", hello, goodbye, hw);
  si_helper.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
  si_helper.StartupInfo.hStdInput = NULL;
  si_helper.StartupInfo.hStdOutput = NULL;
  si_helper.StartupInfo.hStdError = NULL;
  PROCESS_INFORMATION pi_helper;
  CreateProcessW (NULL, cmd, &sec_none, &sec_none,
		  TRUE, EXTENDED_STARTUPINFO_PRESENT,
		  NULL, NULL, &si_helper.StartupInfo, &pi_helper);
  WaitForSingleObject (hello, INFINITE);
  CloseHandle (hello);
  CloseHandle (pi_helper.hThread);
  /* Retrieve pseudo console handles */
  DWORD rLen;
  char buf[64];
  ReadFile (hr, buf, sizeof (buf), &rLen, NULL);
  buf[rLen] = '\0';
  HANDLE hpConIn, hpConOut;
  sscanf (buf, "StdHandles=%p,%p", &hpConIn, &hpConOut);
  DuplicateHandle (pi_helper.hProcess, hpConIn,
		   GetCurrentProcess (), &hpConIn, 0,
		   TRUE, DUPLICATE_SAME_ACCESS);
  DuplicateHandle (pi_helper.hProcess, hpConOut,
		   GetCurrentProcess (), &hpConOut, 0,
		   TRUE, DUPLICATE_SAME_ACCESS);
  CloseHandle (hr);
  CloseHandle (hw);
  /* Clean up */
  DeleteProcThreadAttributeList (si_helper.lpAttributeList);
  HeapFree (GetProcessHeap (), 0, si_helper.lpAttributeList);
  /* Setting information of stuffs regarding pseudo console */
  get_ttyp ()->h_helper_goodbye = goodbye;
  get_ttyp ()->h_helper_process = pi_helper.hProcess;
  get_ttyp ()->helper_process_id = pi_helper.dwProcessId;
  CloseHandle (from_master);
  CloseHandle (to_master);
  from_master = hpConIn;
  to_master = hpConOut;
  return true;
}

bool
fhandler_pty_master::setup ()
{
  int res;
  security_descriptor sd;
  SECURITY_ATTRIBUTES sa = { sizeof (SECURITY_ATTRIBUTES), NULL, TRUE };

  /* Find an unallocated pty to use. */
  int unit = cygwin_shared->tty.allocate (from_master_cyg, get_output_handle ());
  if (unit < 0)
    return false;

  /* from_master should be used for pseudo console.
     Just copy from_master_cyg here for the case that
     pseudo console is not available. */
  from_master = from_master_cyg;

  ProtectHandle1 (get_output_handle (), to_pty);

  tty& t = *cygwin_shared->tty[unit];
  _tc = (tty_min *) &t;

  tcinit (true);		/* Set termios information.  Force initialization. */

  const char *errstr = NULL;
  DWORD pipe_mode = PIPE_NOWAIT;

  if (!SetNamedPipeHandleState (get_output_handle (), &pipe_mode, NULL, NULL))
    termios_printf ("can't set output_handle(%p) to non-blocking mode",
		    get_output_handle ());

  char pipename[sizeof ("ptyNNNN-to-master-cyg")];
  __small_sprintf (pipename, "pty%d-to-master", unit);
  res = fhandler_pipe::create (&sec_none, &from_slave, &to_master,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "output pipe";
      goto err;
    }

  __small_sprintf (pipename, "pty%d-to-master-cyg", unit);
  res = fhandler_pipe::create (&sec_none, &get_handle (), &to_master_cyg,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "output pipe for cygwin";
      goto err;
    }

  ProtectHandle1 (from_slave, from_pty);

  __small_sprintf (pipename, "pty%d-echoloop", unit);
  res = fhandler_pipe::create (&sec_none, &echo_r, &echo_w,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "echo pipe";
      goto err;
    }

  /* Create security attribute.  Default permissions are 0620. */
  sd.malloc (sizeof (SECURITY_DESCRIPTOR));
  RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
  if (!create_object_sd_from_attribute (myself->uid, myself->gid,
					S_IFCHR | S_IRUSR | S_IWUSR | S_IWGRP,
					sd))
    sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) sd;

  /* Carefully check that the input_available_event didn't already exist.
     This is a measure to make sure that the event security descriptor
     isn't occupied by a malicious process.  We must make sure that the
     event's security descriptor is what we expect it to be. */
  if (!(input_available_event = t.get_event (errstr = INPUT_AVAILABLE_EVENT,
					     &sa, TRUE))
      || GetLastError () == ERROR_ALREADY_EXISTS)
    goto err;

  char buf[MAX_PATH];
  errstr = shared_name (buf, OUTPUT_MUTEX, unit);
  if (!(output_mutex = CreateMutex (&sa, FALSE, buf)))
    goto err;

  errstr = shared_name (buf, INPUT_MUTEX, unit);
  if (!(input_mutex = CreateMutex (&sa, FALSE, buf)))
    goto err;

  /* Create master control pipe which allows the master to duplicate
     the pty pipe handles to processes which deserve it. */
  __small_sprintf (buf, "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
		   &cygheap->installation_key, unit);
  master_ctl = CreateNamedPipe (buf, PIPE_ACCESS_DUPLEX
				     | FILE_FLAG_FIRST_PIPE_INSTANCE,
				PIPE_WAIT | PIPE_TYPE_MESSAGE
				| PIPE_READMODE_MESSAGE
				| PIPE_REJECT_REMOTE_CLIENTS,
				1, 4096, 4096, 0, &sec_all_nih);
  if (master_ctl == INVALID_HANDLE_VALUE)
    {
      errstr = "pty master control pipe";
      goto err;
    }
  master_thread = new cygthread (::pty_master_thread, this, "ptym");
  if (!master_thread)
    {
      errstr = "pty master control thread";
      goto err;
    }
  master_fwd_thread = new cygthread (::pty_master_fwd_thread, this, "ptymf");
  if (!master_fwd_thread)
    {
      errstr = "pty master forwarding thread";
      goto err;
    }
  get_ttyp ()->fwd_done = CreateEvent (&sec_none, true, false, NULL);

  setup_pseudoconsole ();

  t.set_from_master (from_master);
  t.set_from_master_cyg (from_master_cyg);
  t.set_to_master (to_master);
  t.set_to_master_cyg (to_master_cyg);
  t.winsize.ws_col = 80;
  t.winsize.ws_row = 25;
  t.master_pid = myself->pid;

  dev ().parse (DEV_PTYM_MAJOR, unit);

  termios_printf ("this %p, pty%d opened - from_pty <%p,%p>, to_pty %p",
	this, unit, from_slave, get_handle (),
	get_output_handle ());
  return true;

err:
  __seterrno ();
  close_maybe (from_slave);
  close_maybe (get_handle ());
  close_maybe (get_output_handle ());
  close_maybe (input_available_event);
  close_maybe (output_mutex);
  close_maybe (input_mutex);
  close_maybe (from_master);
  close_maybe (from_master_cyg);
  close_maybe (to_master);
  close_maybe (to_master_cyg);
  close_maybe (echo_r);
  close_maybe (echo_w);
  close_maybe (master_ctl);
  termios_printf ("pty%d open failed - failed to create %s", unit, errstr);
  return false;
}

void
fhandler_pty_master::fixup_after_fork (HANDLE parent)
{
  DWORD wpid = GetCurrentProcessId ();
  fhandler_pty_master *arch = (fhandler_pty_master *) archetype;
  if (arch->dwProcessId != wpid)
    {
      tty& t = *get_ttyp ();
      if (myself->pid == t.master_pid)
	{
	  t.set_from_master (arch->from_master);
	  t.set_from_master_cyg (arch->from_master_cyg);
	  t.set_to_master (arch->to_master);
	  t.set_to_master_cyg (arch->to_master_cyg);
	}
      arch->dwProcessId = wpid;
    }
  from_master = arch->from_master;
  from_master_cyg = arch->from_master_cyg;
  to_master = arch->to_master;
  to_master_cyg = arch->to_master_cyg;
#if 0 /* Not sure if this is necessary. */
  from_slave = arch->from_slave;
  to_slave = arch->to_slave;
#endif
  report_tty_counts (this, "inherited master", "");
}

void
fhandler_pty_master::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_after_fork (spawn_info->parent);
  else
    from_master = from_master_cyg = to_master = to_master_cyg =
      from_slave = to_slave = NULL;
}

BOOL
fhandler_pty_common::process_opost_output (HANDLE h, const void *ptr, ssize_t& len, bool is_echo)
{
  ssize_t towrite = len;
  BOOL res = TRUE;
  BOOL (WINAPI *WriteFunc)
    (HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
  WriteFunc = WriteFile_Orig ? WriteFile_Orig : WriteFile;
  while (towrite)
    {
      if (!is_echo)
	{
	  if (tc ()->output_stopped && is_nonblocking ())
	    {
	      if (towrite < len)
		break;
	      else
		{
		  set_errno (EAGAIN);
		  len = -1;
		  return TRUE;
		}
	    }
	  while (tc ()->output_stopped)
	    cygwait (10);
	}

      if (!(get_ttyp ()->ti.c_oflag & OPOST))	// raw output mode
	{
	  DWORD n = MIN (OUT_BUFFER_SIZE, towrite);
	  res = WriteFunc (h, ptr, n, &n, NULL);
	  if (!res)
	    break;
	  ptr = (char *) ptr + n;
	  towrite -= n;
	}
      else					// post-process output
	{
	  char outbuf[OUT_BUFFER_SIZE + 1];
	  char *buf = (char *)ptr;
	  DWORD n = 0;
	  ssize_t rc = 0;
	  while (n < OUT_BUFFER_SIZE && rc < towrite)
	    {
	      switch (buf[rc])
		{
		case '\r':
		  if ((get_ttyp ()->ti.c_oflag & ONOCR)
		      && get_ttyp ()->column == 0)
		    {
		      rc++;
		      continue;
		    }
		  if (get_ttyp ()->ti.c_oflag & OCRNL)
		    {
		      outbuf[n++] = '\n';
		      rc++;
		    }
		  else
		    {
		      outbuf[n++] = buf[rc++];
		      get_ttyp ()->column = 0;
		    }
		  break;
		case '\n':
		  if (get_ttyp ()->ti.c_oflag & ONLCR)
		    {
		      outbuf[n++] = '\r';
		      get_ttyp ()->column = 0;
		    }
		  if (get_ttyp ()->ti.c_oflag & ONLRET)
		    get_ttyp ()->column = 0;
		  outbuf[n++] = buf[rc++];
		  break;
		default:
		  outbuf[n++] = buf[rc++];
		  get_ttyp ()->column++;
		  break;
		}
	    }
	  res = WriteFunc (h, outbuf, n, &n, NULL);
	  if (!res)
	    break;
	  ptr = (char *) ptr + rc;
	  towrite -= rc;
	}
    }
  len -= towrite;
  return res;
}
