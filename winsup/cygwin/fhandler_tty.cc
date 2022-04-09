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
#include "registry.h"
#include "tls_pbuf.h"
#include "winf.h"

#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE 0x00020016
#endif /* PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE */

extern "C" int sscanf (const char *, const char *, ...);

#define close_maybe(h) \
  do { \
    if (h && h != INVALID_HANDLE_VALUE) \
      CloseHandle (h); \
  } while (0)

/* pty master control pipe messages */
struct pipe_request {
  DWORD pid;
};

/* The name *nat* comes from 'native' which means non-cygwin
   (native windows). They are used for non-cygwin process. */
struct pipe_reply {
  HANDLE from_master_nat;
  HANDLE from_master;
  HANDLE to_master_nat;
  HANDLE to_master;
  HANDLE to_slave_nat;
  HANDLE to_slave;
  DWORD error;
};

HANDLE attach_mutex;

DWORD acquire_attach_mutex (DWORD t)
{
  if (!attach_mutex)
    return WAIT_OBJECT_0;
  return WaitForSingleObject (attach_mutex, t);
}

void release_attach_mutex (void)
{
  if (!attach_mutex)
    return;
  ReleaseMutex (attach_mutex);
}

inline static bool process_alive (DWORD pid);

/* This functions looks for a process which attached to the same console
   with current process and is matched to given conditions:
     match: If true, return given pid if the process pid attaches to the
	    same console, otherwise, return 0. If false, return pid except
	    for given pid.
     cygwin: return only process's pid which has cygwin pid.
     stub_only: return only stub process's pid of non-cygwin process. */
DWORD
fhandler_pty_common::get_console_process_id (DWORD pid, bool match,
					     bool cygwin, bool stub_only)
{
  tmp_pathbuf tp;
  DWORD *list = (DWORD *) tp.c_get ();
  const DWORD buf_size = NT_MAX_PATH / sizeof (DWORD);

  DWORD num = GetConsoleProcessList (list, buf_size);
  if (num == 0 || num > buf_size)
    return 0;

  DWORD res_pri = 0, res = 0;
  /* Last one is the oldest. */
  /* https://github.com/microsoft/terminal/issues/95 */
  for (int i = (int) num - 1; i >= 0; i--)
    if ((match && list[i] == pid) || (!match && list[i] != pid))
      {
	if (!cygwin)
	  {
	    res_pri = list[i];
	    break;
	  }
	else
	  {
	    pinfo p (cygwin_pid (list[i]));
	    if (!!p && p->exec_dwProcessId)
	      {
		res_pri = stub_only ? p->exec_dwProcessId : list[i];
		break;
	      }
	    if (!p && !res && process_alive (list[i]) && stub_only)
	      res = list[i];
	    if (!!p && !res && !stub_only)
	      res = list[i];
	  }
      }
  return res_pri ?: res;
}

static bool isHybrid; /* Set true if the active pipe is set to nat pipe
			 owned by myself even though the current process
			 is a cygwin process. */
static HANDLE h_gdb_inferior; /* Handle of GDB inferior process. */

static void
set_switch_to_nat_pipe (HANDLE *in, HANDLE *out, HANDLE *err)
{
  cygheap_fdenum cfd (false);
  int fd;
  fhandler_base *replace_in = NULL, *replace_out = NULL, *replace_err = NULL;
  fhandler_pty_slave *ptys = NULL;
  while ((fd = cfd.next ()) >= 0)
    {
      if (*in == cfd->get_handle () ||
	  (fd == 0 && *in == GetStdHandle (STD_INPUT_HANDLE)))
	replace_in = (fhandler_base *) cfd;
      if (*out == cfd->get_output_handle () ||
	  (fd == 1 && *out == GetStdHandle (STD_OUTPUT_HANDLE)))
	replace_out = (fhandler_base *) cfd;
      if (*err == cfd->get_output_handle () ||
	  (fd == 2 && *err == GetStdHandle (STD_ERROR_HANDLE)))
	replace_err = (fhandler_base *) cfd;
      if (cfd->get_device () == (dev_t) myself->ctty)
	{
	  fhandler_base *fh = cfd;
	  if (*in == fh->get_handle ()
	      || *out == fh->get_output_handle ()
	      || *err == fh->get_output_handle ())
	    ptys = (fhandler_pty_slave *) fh;
	}
    }
  if (ptys)
    ptys->set_switch_to_nat_pipe ();
  if (replace_in)
    *in = replace_in->get_handle_nat ();
  if (replace_out)
    *out = replace_out->get_output_handle_nat ();
  if (replace_err)
    *err = replace_err->get_output_handle_nat ();
}

/* Determine if the given path is cygwin binary. */
static bool
path_iscygexec_a_w (LPCSTR na, LPSTR ca, LPCWSTR nw, LPWSTR cw)
{
  path_conv path;
  tmp_pathbuf tp;
  char *prog =tp.c_get ();
  if (na)
    {
      __small_sprintf (prog, "%s", na);
      find_exec (prog, path);
    }
  else if (nw)
    {
      __small_sprintf (prog, "%W", nw);
      find_exec (prog, path);
    }
  else
    {
      if (ca)
	__small_sprintf (prog, "%s", ca);
      else if (cw)
	__small_sprintf (prog, "%W", cw);
      else
	return true;
      char *p = prog;
      char *p1;
      do
	if ((p1 = strchr (p, ' ')) || (p1 = p + strlen (p)))
	  {
	    p = p1;
	    if (*p == ' ')
	      {
		*p = '\0';
		find_exec (prog, path);
		*p = ' ';
		p ++;
	      }
	    else if (*p == '\0')
	      find_exec (prog, path);
	  }
      while (!path.exists() && *p);
    }
  const char *argv[] = {"", NULL}; /* Dummy */
  av av1;
  av1.setup ("", path, "", 1, argv, false);
  return path.iscygexec ();
}

bool
fhandler_termios::path_iscygexec_a (LPCSTR n, LPSTR c)
{
  return path_iscygexec_a_w (n, c, NULL, NULL);
}

bool
fhandler_termios::path_iscygexec_w (LPCWSTR n, LPWSTR c)
{
  return path_iscygexec_a_w (NULL, NULL, n, c);
}

static bool atexit_func_registered = false;
static bool debug_process = false;

static void
atexit_func (void)
{
  if (isHybrid)
    {
      cygheap_fdenum cfd (false);
      while (cfd.next () >= 0)
	if (cfd->get_device () == (dev_t) myself->ctty)
	  {
	    DWORD force_switch_to = 0;
	    if (WaitForSingleObject (h_gdb_inferior, 0) == WAIT_TIMEOUT
		&& !debug_process)
	      force_switch_to = GetProcessId (h_gdb_inferior);
	    fhandler_base *fh = cfd;
	    fhandler_pty_slave *ptys = (fhandler_pty_slave *) fh;
	    tty *ttyp = (tty *) ptys->tc ();
	    bool stdin_is_ptys =
		GetStdHandle (STD_INPUT_HANDLE) == ptys->get_handle ();
	    struct fhandler_pty_slave::handle_set_t handles =
	      {
		ptys->get_handle_nat (),
		ptys->get_input_available_event (),
		ptys->input_mutex,
		ptys->pipe_sw_mutex
	      };
	    fhandler_pty_slave::cleanup_for_non_cygwin_app (&handles, ttyp,
							    stdin_is_ptys,
							    force_switch_to);
	    break;
	  }
      CloseHandle (h_gdb_inferior);
    }
}

#define DEF_HOOK(name) static __typeof__ (name) *name##_Orig
/* CreateProcess() is hooked for GDB etc. */
DEF_HOOK (CreateProcessA);
DEF_HOOK (CreateProcessW);

static BOOL WINAPI
CreateProcessA_Hooked
     (LPCSTR n, LPSTR c, LPSECURITY_ATTRIBUTES pa, LPSECURITY_ATTRIBUTES ta,
      BOOL inh, DWORD f, LPVOID e, LPCSTR d,
      LPSTARTUPINFOA si, LPPROCESS_INFORMATION pi)
{
  STARTUPINFOEXA siex = {0, };
  if (si->cb == sizeof (STARTUPINFOEXA))
    siex = *(STARTUPINFOEXA *)si;
  else
    siex.StartupInfo = *si;
  STARTUPINFOA *siov = &siex.StartupInfo;
  if (!(si->dwFlags & STARTF_USESTDHANDLES))
    {
      siov->dwFlags |= STARTF_USESTDHANDLES;
      siov->hStdInput = GetStdHandle (STD_INPUT_HANDLE);
      siov->hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
      siov->hStdError = GetStdHandle (STD_ERROR_HANDLE);
    }
  bool path_iscygexec = fhandler_termios::path_iscygexec_a (n, c);
  if (!path_iscygexec)
    set_switch_to_nat_pipe (&siov->hStdInput, &siov->hStdOutput,
			    &siov->hStdError);
  BOOL ret = CreateProcessA_Orig (n, c, pa, ta, inh, f, e, d, siov, pi);
  h_gdb_inferior = pi->hProcess;
  DuplicateHandle (GetCurrentProcess (), h_gdb_inferior,
		   GetCurrentProcess (), &h_gdb_inferior,
		   0, 0, DUPLICATE_SAME_ACCESS);
  debug_process = !!(f & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS));
  if (debug_process)
    mutex_timeout = 0; /* to avoid deadlock in GDB */
  if (!atexit_func_registered && !path_iscygexec)
    {
      atexit (atexit_func);
      atexit_func_registered = true;
    }
  return ret;
}

static BOOL WINAPI
CreateProcessW_Hooked
     (LPCWSTR n, LPWSTR c, LPSECURITY_ATTRIBUTES pa, LPSECURITY_ATTRIBUTES ta,
      BOOL inh, DWORD f, LPVOID e, LPCWSTR d,
      LPSTARTUPINFOW si, LPPROCESS_INFORMATION pi)
{
  STARTUPINFOEXW siex = {0, };
  if (si->cb == sizeof (STARTUPINFOEXW))
    siex = *(STARTUPINFOEXW *)si;
  else
    siex.StartupInfo = *si;
  STARTUPINFOW *siov = &siex.StartupInfo;
  if (!(si->dwFlags & STARTF_USESTDHANDLES))
    {
      siov->dwFlags |= STARTF_USESTDHANDLES;
      siov->hStdInput = GetStdHandle (STD_INPUT_HANDLE);
      siov->hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
      siov->hStdError = GetStdHandle (STD_ERROR_HANDLE);
    }
  bool path_iscygexec = fhandler_termios::path_iscygexec_w (n, c);
  if (!path_iscygexec)
    set_switch_to_nat_pipe (&siov->hStdInput, &siov->hStdOutput,
			    &siov->hStdError);
  BOOL ret = CreateProcessW_Orig (n, c, pa, ta, inh, f, e, d, siov, pi);
  h_gdb_inferior = pi->hProcess;
  DuplicateHandle (GetCurrentProcess (), h_gdb_inferior,
		   GetCurrentProcess (), &h_gdb_inferior,
		   0, 0, DUPLICATE_SAME_ACCESS);
  debug_process = !!(f & (DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS));
  if (debug_process)
    mutex_timeout = 0; /* to avoid deadlock in GDB */
  if (!atexit_func_registered && !path_iscygexec)
    {
      atexit (atexit_func);
      atexit_func_registered = true;
    }
  return ret;
}

static void
convert_mb_str (UINT cp_to, char *ptr_to, size_t *len_to,
		UINT cp_from, const char *ptr_from, size_t len_from,
		mbstate_t *mbp)
{
  tmp_pathbuf tp;
  wchar_t *wbuf = tp.w_get ();
  int wlen = 0;
  char *tmpbuf = tp.c_get ();
  memcpy (tmpbuf, mbp->__value.__wchb, mbp->__count);
  if (mbp->__count + len_from > NT_MAX_PATH)
    len_from = NT_MAX_PATH - mbp->__count;
  memcpy (tmpbuf + mbp->__count, ptr_from, len_from);
  int total_len = mbp->__count + len_from;
  mbp->__count = 0;
  int mblen = 0;
  for (const char *p = tmpbuf; p < tmpbuf + total_len; p += mblen)
    /* Max bytes in multibyte char supported is 4. */
    for (mblen = 1; mblen <= 4; mblen ++)
      {
	/* Try conversion */
	int l = MultiByteToWideChar (cp_from, MB_ERR_INVALID_CHARS,
				     p, mblen,
				     wbuf + wlen, NT_MAX_PATH - wlen);
	if (l)
	  { /* Conversion Success */
	    wlen += l;
	    break;
	  }
	else if (mblen == 4)
	  { /* Conversion Fail */
	    l = MultiByteToWideChar (cp_from, 0, p, 1,
				     wbuf + wlen, NT_MAX_PATH - wlen);
	    wlen += l;
	    mblen = 1;
	    break;
	  }
	else if (p + mblen == tmpbuf + total_len)
	  { /* Multibyte char incomplete */
	    memcpy (mbp->__value.__wchb, p, mblen);
	    mbp->__count = mblen;
	    break;
	  }
	/* Retry conversion with extended length */
      }
  *len_to = WideCharToMultiByte (cp_to, 0, wbuf, wlen,
				 ptr_to, *len_to, NULL, NULL);
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
  return ::bytes_available (n, get_handle ());
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

void
fhandler_pty_master::discard_input ()
{
  DWORD bytes_in_pipe;
  char buf[1024];
  DWORD n;

  WaitForSingleObject (input_mutex, mutex_timeout);
  while (::bytes_available (bytes_in_pipe, from_master) && bytes_in_pipe)
    ReadFile (from_master, buf, sizeof(buf), &n, NULL);
  ResetEvent (input_available_event);
  if (!get_ttyp ()->pcon_activated)
    while (::bytes_available (bytes_in_pipe, from_master_nat) && bytes_in_pipe)
      ReadFile (from_master_nat, buf, sizeof(buf), &n, NULL);
  get_ttyp ()->discard_input = true;
  ReleaseMutex (input_mutex);
}

DWORD
fhandler_pty_common::__acquire_output_mutex (const char *fn, int ln,
					     DWORD ms)
{
  if (strace.active ())
    strace.prntf (_STRACE_TERMIOS, fn, "(%d): pty output_mutex (%p): waiting %d ms", ln, output_mutex, ms);
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
  if (!process_opost_output (echo_w, str, towrite, true,
			     get_ttyp (), is_nonblocking ()))
    termios_printf ("Write to echo pipe failed, %E");
}

int
fhandler_pty_master::accept_input ()
{
  DWORD bytes_left;
  int ret = 1;

  WaitForSingleObject (input_mutex, mutex_timeout);

  char *p = rabuf () + raixget ();
  bytes_left = eat_readahead (-1);

  HANDLE write_to = get_output_handle ();
  tmp_pathbuf tp;
  if (to_be_read_from_nat_pipe ()
      && get_ttyp ()->pty_input_state == tty::to_nat)
    {
      /* This code is reached if non-cygwin app is foreground and
	 pseudo console is not enabled. */
      write_to = to_slave_nat; /* write to nat pipe rather than cyg pipe. */

      /* Charset conversion for non-cygwin apps. */
      UINT cp_to;
      pinfo pinfo_target = pinfo (get_ttyp ()->invisible_console_pid);
      DWORD target_pid = 0;
      if (pinfo_target)
	target_pid = pinfo_target->dwProcessId;
      if (target_pid)
	{
	  /* Slave attaches to a different console than master.
	     Therefore reattach here. */
	  DWORD resume_pid = attach_console_temporarily (target_pid);
	  cp_to = GetConsoleCP ();
	  resume_from_temporarily_attach (resume_pid);
	}
      else
	cp_to = GetConsoleCP ();

      if (get_ttyp ()->term_code_page != cp_to)
	{
	  static mbstate_t mbp;
	  char *mbbuf = tp.c_get ();
	  size_t nlen = NT_MAX_PATH;
	  convert_mb_str (cp_to, mbbuf, &nlen,
			  get_ttyp ()->term_code_page, p, bytes_left, &mbp);
	  p = mbbuf;
	  bytes_left = nlen;
	}
    }

  if (!bytes_left)
    {
      termios_printf ("sending EOF to slave");
      get_ttyp ()->read_retval = 0;
    }
  else
    {
      BOOL rc = TRUE;
      DWORD written = 0;

      paranoid_printf ("about to write %u chars to slave", bytes_left);
      /* Write line by line for transfer input. */
      char *p0 = p;
      char *p_cr = (char *) memchr (p0, '\r', bytes_left - (p0 - p));
      char *p_lf = (char *) memchr (p0, '\n', bytes_left - (p0 - p));
      DWORD n;
      while (p_cr || p_lf)
	{
	  char *p1 =
	    p_cr ?  (p_lf ? ((p_cr + 1 == p_lf)
			     ?  p_lf : min(p_cr, p_lf)) : p_cr) : p_lf;
	  n = p1 - p0 + 1;
	  rc = WriteFile (write_to, p0, n, &n, NULL);
	  if (rc)
	    written += n;
	  p0 = p1 + 1;
	  p_cr = (char *) memchr (p0, '\r', bytes_left - (p0 - p));
	  p_lf = (char *) memchr (p0, '\n', bytes_left - (p0 - p));
	}
      if (rc && (n = bytes_left - (p0 - p)))
	{
	  rc = WriteFile (write_to, p0, n, &n, NULL);
	  if (rc)
	    written += n;
	}
      if (!rc && written == 0)
	{
	  debug_printf ("error writing to pipe %p %E", write_to);
	  get_ttyp ()->read_retval = -1;
	  puts_readahead (p, bytes_left);
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

  if (write_to == get_output_handle ())
    SetEvent (input_available_event); /* Set input_available_event only when
					 the data is written to cyg pipe. */
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
	  /* tclush can finish here. */
	  if (!buf)
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

      if (!buf || ((get_ttyp ()->ti.c_lflag & FLUSHO)
		   && !get_ttyp ()->mask_flusho))
	continue; /* Discard read data */

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
  if (buf)
    set_mask_flusho (false);
  termios_printf ("returning %d", rc);
  return rc;
}

/* pty slave stuff */

fhandler_pty_slave::fhandler_pty_slave (int unit)
  : fhandler_pty_common (), inuse (NULL), output_handle_nat (NULL),
  io_handle_nat (NULL), slave_reading (NULL), num_reader (0)
{
  if (unit >= 0)
    dev ().parse (DEV_PTYS_MAJOR, unit);
}

int
fhandler_pty_slave::open (int flags, mode_t)
{
  HANDLE pty_owner;
  HANDLE from_master_nat_local, from_master_local;
  HANDLE to_master_nat_local, to_master_local;
  HANDLE *handles[] =
  {
    &from_master_nat_local, &input_available_event, &input_mutex, &inuse,
    &output_mutex, &to_master_nat_local, &pty_owner, &to_master_local,
    &from_master_local, &pipe_sw_mutex,
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
  if (!(pipe_sw_mutex
	= get_ttyp ()->open_mutex (PIPE_SW_MUTEX, MAXIMUM_ALLOWED)))
    {
      errmsg = "open pipe switch mutex failed, %E";
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
    acquire_output_mutex (mutex_timeout);
    inuse = get_ttyp ()->create_inuse (&sa);
    get_ttyp ()->was_opened = true;
    release_output_mutex ();
  }

  if (!get_ttyp ()->from_master_nat () || !get_ttyp ()->from_master ()
      || !get_ttyp ()->to_master_nat () || !get_ttyp ()->to_master ())
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
      if (!DuplicateHandle (pty_owner, get_ttyp ()->from_master_nat (),
			    GetCurrentProcess (),
			    &from_master_nat_local, 0, TRUE,
			    DUPLICATE_SAME_ACCESS))
	{
	  termios_printf ("can't duplicate input from %u/%p, %E",
			  get_ttyp ()->master_pid,
			  get_ttyp ()->from_master_nat ());
	  __seterrno ();
	  goto err_no_msg;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->from_master (),
			    GetCurrentProcess (),
			    &from_master_local, 0, TRUE,
			    DUPLICATE_SAME_ACCESS))
	{
	  termios_printf ("can't duplicate input from %u/%p, %E",
			  get_ttyp ()->master_pid,
			  get_ttyp ()->from_master ());
	  __seterrno ();
	  goto err_no_msg;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->to_master_nat (),
			  GetCurrentProcess (),
			  &to_master_nat_local, 0, TRUE,
			  DUPLICATE_SAME_ACCESS))
	{
	  errmsg = "can't duplicate output, %E";
	  goto err;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->to_master (),
			    GetCurrentProcess (),
			    &to_master_local, 0, TRUE,
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
      from_master_nat_local = repl.from_master_nat;
      from_master_local = repl.from_master;
      to_master_nat_local = repl.to_master_nat;
      to_master_local = repl.to_master;
      if (!from_master_nat_local || !from_master_local
	  || !to_master_nat_local || !to_master_local)
	{
	  SetLastError (repl.error);
	  errmsg = "error duplicating pipes, %E";
	  goto err;
	}
    }
  VerifyHandle (from_master_nat_local);
  VerifyHandle (from_master_local);
  VerifyHandle (to_master_nat_local);
  VerifyHandle (to_master_local);

  termios_printf ("duplicated from_master_nat %p->%p from pty_owner",
		  get_ttyp ()->from_master_nat (), from_master_nat_local);
  termios_printf ("duplicated from_master %p->%p from pty_owner",
		  get_ttyp ()->from_master (), from_master_local);
  termios_printf ("duplicated to_master_nat %p->%p from pty_owner",
		  get_ttyp ()->to_master_nat (), to_master_nat_local);
  termios_printf ("duplicated to_master %p->%p from pty_owner",
		  get_ttyp ()->to_master (), to_master_local);

  set_handle_nat (from_master_nat_local);
  set_handle (from_master_local);
  set_output_handle_nat (to_master_nat_local);
  set_output_handle (to_master_local);

  if (_major (myself->ctty) == DEV_CONS_MAJOR
      && !(!pinfo (myself->ppid) && getenv ("ConEmuPID")))
    /* This process is supposed to be a master process which is
       running on console. Invisible console will be created in
       primary slave process to prevent overriding code page
       of root console by setup_locale(). */
    /* ... except for ConEmu cygwin-connector in which this
       code does not work as expected because it calls Win32
       API directly rather than cygwin read()/write(). Due to
       this behaviour, protection based on attach_mutex does
       not take effect. */
    get_ttyp ()->need_invisible_console = true;
  else if (_major (myself->ctty) != DEV_CONS_MAJOR
	   && (!get_ttyp ()->invisible_console_pid
	       || !pinfo (get_ttyp ()->invisible_console_pid)))
    /* Create a new invisible console for each pty to isolate
       CTRL_C_EVENTs between ptys. */
    get_ttyp ()->need_invisible_console = true;
  else
    {
      acquire_attach_mutex (mutex_timeout);
      fhandler_console::need_invisible ();
      release_attach_mutex ();
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

bool
fhandler_pty_slave::open_setup (int flags)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  myself->set_ctty (this, flags);
  report_tty_counts (this, "opened", "");
  return fhandler_base::open_setup (flags);
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
  if (!ForceCloseHandle (get_output_handle_nat ()))
    termios_printf ("CloseHandle (get_output_handle_nat ()<%p>), %E",
	get_output_handle_nat ());
  if (!ForceCloseHandle (get_handle_nat ()))
    termios_printf ("CloseHandle (get_handle_nat ()<%p>), %E",
	get_handle_nat ());
  fhandler_pty_common::close ();
  if (!ForceCloseHandle (output_mutex))
    termios_printf ("CloseHandle (output_mutex<%p>), %E", output_mutex);
  if (get_ttyp ()->invisible_console_pid
      && !pinfo (get_ttyp ()->invisible_console_pid))
    get_ttyp ()->invisible_console_pid = 0;
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

void
fhandler_pty_slave::set_switch_to_nat_pipe (void)
{
  if (!isHybrid)
    {
      isHybrid = true;
      setup_locale ();
      myself->exec_dwProcessId = myself->dwProcessId; /* Set this as a marker
							 for tty::nat_fg()
							 and process_sigs() */
      bool stdin_is_ptys = GetStdHandle (STD_INPUT_HANDLE) == get_handle ();
      setup_for_non_cygwin_app (false, NULL, stdin_is_ptys);
    }
}

inline static bool
process_alive (DWORD pid)
{
  /* This function is very similar to _pinfo::alive(), however, this
     can be used for non-cygwin process which is started from non-cygwin
     shell. In addition, this checks exit code as well. */
  if (pid == 0)
    return false;
  HANDLE h = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (h == NULL)
    return false;
  DWORD exit_code;
  BOOL r = GetExitCodeProcess (h, &exit_code);
  CloseHandle (h);
  if (r && exit_code == STILL_ACTIVE)
    return true;
  return false;
}

inline static bool
nat_pipe_owner_self (DWORD pid)
{
  return (pid == (myself->exec_dwProcessId ?: myself->dwProcessId));
}

/* This function is called from some pty slave functions. The purpose
   of this function is cleaning up the nat pipe state which is marked
   as active but actually not used anymore. This is needed only when
   non-cygwin process is not terminated cleanly. */
void
fhandler_pty_slave::reset_switch_to_nat_pipe (void)
{
  if (h_gdb_inferior)
    {
      if (WaitForSingleObject (h_gdb_inferior, 0) == WAIT_TIMEOUT)
	{
	  if (isHybrid)
	    get_ttyp ()->wait_fwd ();
	}
      else
	{
	  CloseHandle (h_gdb_inferior);
	  h_gdb_inferior = NULL;
	  mutex_timeout = INFINITE;
	  if (isHybrid)
	    {
	      if (get_ttyp ()->getpgid () == myself->pgid
		  && GetStdHandle (STD_INPUT_HANDLE) == get_handle ()
		  && get_ttyp ()->pty_input_state_eq (tty::to_nat))
		{
		  WaitForSingleObject (input_mutex, mutex_timeout);
		  acquire_attach_mutex (mutex_timeout);
		  transfer_input (tty::to_cyg, get_handle_nat (), get_ttyp (),
				  input_available_event);
		  release_attach_mutex ();
		  ReleaseMutex (input_mutex);
		}
	      if (get_ttyp ()->master_is_running_as_service
		  && get_ttyp ()->pcon_activated)
		/* If the master is running as service, re-attaching to
		   the console of the parent process will fail.
		   Therefore, never close pseudo console here. */
		return;
	      bool need_restore_handles = get_ttyp ()->pcon_activated;
	      WaitForSingleObject (pipe_sw_mutex, INFINITE);
	      if (get_ttyp ()->pcon_activated)
		{
		  acquire_attach_mutex (mutex_timeout);
		  close_pseudoconsole (get_ttyp ());
		  release_attach_mutex ();
		}
	      else
		hand_over_only (get_ttyp ());
	      ReleaseMutex (pipe_sw_mutex);
	      if (need_restore_handles)
		{
		  pinfo p (get_ttyp ()->master_pid);
		  HANDLE pty_owner =
		    OpenProcess (PROCESS_DUP_HANDLE, FALSE, p->dwProcessId);
		  if (pty_owner)
		    {
		      CloseHandle (get_handle_nat ());
		      DuplicateHandle (pty_owner,
				       get_ttyp ()->from_master_nat (),
				       GetCurrentProcess (), &get_handle_nat (),
				       0, TRUE, DUPLICATE_SAME_ACCESS);
		      CloseHandle (get_output_handle_nat ());
		      DuplicateHandle (pty_owner,
				       get_ttyp ()->to_master_nat (),
				       GetCurrentProcess (),
				       &get_output_handle_nat (),
				       0, TRUE, DUPLICATE_SAME_ACCESS);
		      CloseHandle (pty_owner);
		    }
		  else
		    {
		      char pipe[MAX_PATH];
		      __small_sprintf (pipe,
			       "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
			       &cygheap->installation_key, get_minor ());
		      pipe_request req = { GetCurrentProcessId () };
		      pipe_reply repl;
		      DWORD len;
		      if (!CallNamedPipe (pipe, &req, sizeof req,
					  &repl, sizeof repl, &len, 500))
			return; /* What can we do? */
		      CloseHandle (get_handle_nat ());
		      set_handle_nat (repl.from_master_nat);
		      CloseHandle (get_output_handle_nat ());
		      set_output_handle_nat (repl.to_master_nat);
		    }
		}
	      myself->exec_dwProcessId = 0;
	      isHybrid = false;
	    }
	}
    }
  if (isHybrid)
    return;
  if (get_ttyp ()->pcon_start) /* Pseudo console initialization is on going */
    return;
  DWORD wait_ret = WaitForSingleObject (pipe_sw_mutex, mutex_timeout);
  if (wait_ret == WAIT_TIMEOUT)
    return;
  if (!nat_pipe_owner_self (get_ttyp ()->nat_pipe_owner_pid)
      && process_alive (get_ttyp ()->nat_pipe_owner_pid))
    {
      /* There is a process which owns nat pipe. */
      ReleaseMutex (pipe_sw_mutex);
      return;
    }
  /* Clean up nat pipe state */
  get_ttyp ()->pty_input_state = tty::to_cyg;
  get_ttyp ()->nat_pipe_owner_pid = 0;
  get_ttyp ()->switch_to_nat_pipe = false;
  get_ttyp ()->pcon_activated = false;
  ReleaseMutex (pipe_sw_mutex);
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

  acquire_output_mutex (mutex_timeout);
  if (!process_opost_output (get_output_handle (), ptr, towrite, false,
			     get_ttyp (), is_nonblocking ()))
    {
      DWORD err = GetLastError ();
      termios_printf ("WriteFile failed, %E");
      switch (err)
	{
	case ERROR_NO_DATA:
	  err = ERROR_IO_DEVICE;
	  fallthrough;
	default:
	  __seterrno_from_win_error (err);
	}
      towrite = -1;
    }
  release_output_mutex ();

  return towrite;
}

/* This function is called from some slave pty reading functions
   to switch active pipe to cygwin pipe (not nat pipe) temporarily
   even though there is another process which owns nat pipe. This
   is needed if cygwin process reads input while another non-cygwin
   process is running at the same time.
   (e.g. Something like "cat | non-cygwin-app") */
void
fhandler_pty_slave::mask_switch_to_nat_pipe (bool mask, bool xfer)
{
  char name[MAX_PATH];
  shared_name (name, TTY_SLAVE_READING, get_minor ());
  HANDLE masked = OpenEvent (READ_CONTROL, FALSE, name);
  CloseHandle (masked);

  WaitForSingleObject (input_mutex, mutex_timeout);
  if (mask)
    {
      if (InterlockedIncrement (&num_reader) == 1)
	slave_reading = CreateEvent (&sec_none_nih, TRUE, FALSE, name);
    }
  else if (InterlockedDecrement (&num_reader) == 0)
    CloseHandle (slave_reading);

  /* This is needed when cygwin-app is started from non-cygwin app if
     pseudo console is disabled. */
  bool need_xfer = get_ttyp ()->nat_fg (get_ttyp ()->getpgid ())
    && get_ttyp ()->switch_to_nat_pipe && !get_ttyp ()->pcon_activated;

  /* In GDB, transfer input based on setpgid() does not work because
     GDB may not set terminal process group properly. Therefore,
     transfer input here if isHybrid is set. */
  bool need_gdb_xfer =
    isHybrid && GetStdHandle (STD_INPUT_HANDLE) == get_handle ();
  if (!!masked != mask && xfer && (need_gdb_xfer || need_xfer))
    {
      if (mask && get_ttyp ()->pty_input_state_eq (tty::to_nat))
	{
	  acquire_attach_mutex (mutex_timeout);
	  transfer_input (tty::to_cyg, get_handle_nat (), get_ttyp (),
			  input_available_event);
	  release_attach_mutex ();
	}
      else if (!mask && get_ttyp ()->pty_input_state_eq (tty::to_cyg))
	{
	  acquire_attach_mutex (mutex_timeout);
	  transfer_input (tty::to_nat, get_handle (), get_ttyp (),
			  input_available_event);
	  release_attach_mutex ();
	}
    }
  ReleaseMutex (input_mutex);
}

/* Return true when the non-cygwin app is reading the input. */
bool
fhandler_pty_common::to_be_read_from_nat_pipe (void)
{
  if (!get_ttyp ()->switch_to_nat_pipe)
    return false;

  char name[MAX_PATH];
  shared_name (name, TTY_SLAVE_READING, get_minor ());
  HANDLE masked = OpenEvent (READ_CONTROL, FALSE, name);
  CloseHandle (masked);

  if (masked) /* The foreground process is cygwin process */
    return false;

  if (!pinfo (get_ttyp ()->getpgid ()))
    /* GDB may set invalid process group for non-cygwin process. */
    return true;

  return get_ttyp ()->nat_fg (get_ttyp ()->getpgid ());
}

void __reg3
fhandler_pty_slave::read (void *ptr, size_t& len)
{
  ssize_t totalread = 0;
  int vmin = 0;
  int vtime = 0;	/* Initialized to prevent -Wuninitialized warning */
  size_t readlen;
  DWORD bytes_in_pipe;
  char buf[INP_BUFFER_SIZE];
  DWORD time_to_wait;
  char *ptr0 = (char *) ptr;

  bg_check_types bg = bg_check (SIGTTIN);
  if (bg <= bg_eof)
    {
      len = (size_t) bg;
      return;
    }

  termios_printf ("read(%p, %lu) handle %p", ptr, len, get_handle ());

  push_process_state process_state (PID_TTYIN);

  if (ptr) /* Indicating not tcflush(). */
    mask_switch_to_nat_pipe (true, true);

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

wait_retry:
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
      if (!IsEventSignalled (input_available_event))
	{ /* Maybe another thread has processed input. */
	  ReleaseMutex (input_mutex);
	  goto wait_retry;
	}

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
	  mask_switch_to_nat_pipe (false, false);
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
	  if (!ReadFile (get_handle (), buf, readlen, &n, NULL))
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
  if (ptr0)
    { /* Not tcflush() */
      bool saw_eol = totalread > 0 && strchr ("\r\n", ptr0[totalread -1]);
      mask_switch_to_nat_pipe (false, saw_eol || len == 0);
    }
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
  *t = get_ttyp ()->ti;

  /* Workaround for rlwrap */
  cygheap_fdenum cfd (false);
  while (cfd.next () >= 0)
    if (cfd->get_major () == DEV_PTYM_MAJOR
	&& cfd->get_minor () == get_minor ())
      {
	if (get_ttyp ()->pcon_start)
	  t->c_lflag &= ~(ICANON | ECHO);
	if (get_ttyp ()->pcon_activated)
	  t->c_iflag &= ~ICRNL;
	break;
      }
  return 0;
}

int
fhandler_pty_slave::tcsetattr (int, const struct termios *t)
{
  acquire_output_mutex (mutex_timeout);
  get_ttyp ()->ti = *t;
  release_output_mutex ();
  return 0;
}

int
fhandler_pty_slave::tcflush (int queue)
{
  int ret = 0;

  termios_printf ("tcflush(%d) handle %p", queue, get_handle ());

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

  acquire_output_mutex (mutex_timeout);

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
      if (get_ttyp ()->winsize.ws_row != ((struct winsize *) arg)->ws_row
	  || get_ttyp ()->winsize.ws_col != ((struct winsize *) arg)->ws_col
	  || get_ttyp ()->winsize.ws_ypixel != ((struct winsize *) arg)->ws_ypixel
	  || get_ttyp ()->winsize.ws_xpixel != ((struct winsize *) arg)->ws_xpixel
	 )
	{
	  if (get_ttyp ()->pcon_activated && get_ttyp ()->nat_pipe_owner_pid)
	    resize_pseudo_console ((struct winsize *) arg);
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
	fallthrough;
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
  pipe_sw_mutex = get_ttyp ()->open_mutex (PIPE_SW_MUTEX, write_access);
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
    master_thread (NULL), from_master_nat (NULL), to_master_nat (NULL),
    from_slave_nat (NULL), to_slave_nat (NULL), echo_r (NULL), echo_w (NULL),
    dwProcessId (0), to_master (NULL), from_master (NULL),
    master_fwd_thread (NULL)
{
  if (unit >= 0)
    dev ().parse (DEV_PTYM_MAJOR, unit);
  set_name ("/dev/ptmx");
}

int
fhandler_pty_master::open (int flags, mode_t)
{
  if (!setup ())
      return 0;
  set_open_status ();
  dwProcessId = GetCurrentProcessId ();
  return 1;
}

bool
fhandler_pty_master::open_setup (int flags)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  char buf[sizeof ("opened pty master for ptyNNNNNNNNNNN")];
  __small_sprintf (buf, "opened pty master for pty%d", get_minor ());
  report_tty_counts (this, buf, "");
  return fhandler_base::open_setup (flags);
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
  if (!ForceCloseHandle (pipe_sw_mutex))
    termios_printf ("CloseHandle (pipe_sw_mutex<%p>), %E", pipe_sw_mutex);
  if (!ForceCloseHandle1 (get_handle (), from_pty))
    termios_printf ("CloseHandle (get_handle ()<%p>), %E", get_handle ());
  if (!ForceCloseHandle1 (get_output_handle (), to_pty))
    termios_printf ("CloseHandle (get_output_handle ()<%p>), %E",
		    get_output_handle ());

  return 0;
}

void
fhandler_pty_common::resize_pseudo_console (struct winsize *ws)
{
  COORD size;
  size.X = ws->ws_col;
  size.Y = ws->ws_row;
  HPCON_INTERNAL hpcon_local;
  HANDLE pcon_owner =
    OpenProcess (PROCESS_DUP_HANDLE, FALSE, get_ttyp ()->nat_pipe_owner_pid);
  DuplicateHandle (pcon_owner, get_ttyp ()->h_pcon_write_pipe,
		   GetCurrentProcess (), &hpcon_local.hWritePipe,
		   0, TRUE, DUPLICATE_SAME_ACCESS);
  acquire_attach_mutex (mutex_timeout);
  ResizePseudoConsole ((HPCON) &hpcon_local, size);
  release_attach_mutex ();
  CloseHandle (pcon_owner);
  CloseHandle (hpcon_local.hWritePipe);
}

void
fhandler_pty_master::cleanup ()
{
  report_tty_counts (this, "closing master", "");
  if (archetype)
    from_master_nat = from_master =
      to_master_nat = to_master = from_slave_nat = to_slave_nat = NULL;
  fhandler_base::cleanup ();
}

int
fhandler_pty_master::close ()
{
  OBJECT_BASIC_INFORMATION obi;
  NTSTATUS status;

  termios_printf ("closing from_master_nat(%p)/from_master(%p)/to_master_nat(%p)/to_master(%p) since we own them(%u)",
		  from_master_nat, from_master,
		  to_master_nat, to_master, dwProcessId);
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
	  acquire_output_mutex (mutex_timeout);
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
	  get_ttyp ()->stop_fwd_thread = true;
	  WriteFile (to_master_nat, "", 0, &len, NULL);
	  master_fwd_thread->detach ();
	}
    }

  /* Check if the last master handle has been closed.  If so, set
     input_available_event to wake up potentially waiting slaves. */
  acquire_output_mutex (mutex_timeout);
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
      if (get_ttyp ()->getsid () > 0)
	kill (get_ttyp ()->getsid (), SIGHUP);
      SetEvent (input_available_event);
    }

  if (!ForceCloseHandle (from_master_nat))
    termios_printf ("error closing from_master_nat %p, %E", from_master_nat);
  if (!ForceCloseHandle (to_master_nat))
    termios_printf ("error closing to_master_nat %p, %E", to_master_nat);
  from_master_nat = to_master_nat = NULL;
  if (!ForceCloseHandle (from_slave_nat))
    termios_printf ("error closing from_slave_nat %p, %E", from_slave_nat);
  from_slave_nat = NULL;
  if (!ForceCloseHandle (to_master))
    termios_printf ("error closing to_master %p, %E", to_master);
  to_master = NULL;
  ForceCloseHandle (echo_r);
  ForceCloseHandle (echo_w);
  echo_r = echo_w = NULL;
  if (to_slave_nat)
    ForceCloseHandle (to_slave_nat);
  to_slave_nat = NULL;

  if (have_execed || get_ttyp ()->master_pid != myself->pid)
    termios_printf ("not clearing: %d, master_pid %d",
		    have_execed, get_ttyp ()->master_pid);
  if (!ForceCloseHandle (input_available_event))
    termios_printf ("CloseHandle (input_available_event<%p>), %E",
		    input_available_event);

  /* The from_master must be closed last so that the same pty is not
     allocated before cleaning up the other corresponding instances. */
  if (!ForceCloseHandle (from_master))
    termios_printf ("error closing from_master %p, %E", from_master);
  from_master = NULL;

  return 0;
}

ssize_t __stdcall
fhandler_pty_master::write (const void *ptr, size_t len)
{
  ssize_t ret;
  char *p = (char *) ptr;
  termios &ti = tc ()->ti;

  bg_check_types bg = bg_check (SIGTTOU);
  if (bg <= bg_eof)
    return (ssize_t) bg;

  push_process_state process_state (PID_TTYOU);

  if (get_ttyp ()->pcon_start)
    { /* Reaches here when pseudo console initialization is on going. */
      /* Pseudo condole support uses "CSI6n" to get cursor position.
	 If the reply for "CSI6n" is divided into multiple writes,
	 pseudo console sometimes does not recognize it.  Therefore,
	 put them together into wpbuf and write all at once. */
      static const int wpbuf_len = strlen ("\033[32768;32868R");
      static char wpbuf[wpbuf_len];
      static int ixput = 0;
      static int state = 0;

      DWORD n;
      WaitForSingleObject (input_mutex, mutex_timeout);
      for (size_t i = 0; i < len; i++)
	{
	  if (p[i] == '\033')
	    {
	      if (ixput)
		line_edit (wpbuf, ixput, ti, &ret);
	      ixput = 0;
	      state = 1;
	    }
	  if (state == 1)
	    {
	      if (ixput < wpbuf_len)
		wpbuf[ixput++] = p[i];
	      else
		{
		  if (!get_ttyp ()->req_xfer_input)
		    WriteFile (to_slave_nat, wpbuf, ixput, &n, NULL);
		  ixput = 0;
		  wpbuf[ixput++] = p[i];
		}
	    }
	  else
	    line_edit (p + i, 1, ti, &ret);
	  if (state == 1 && p[i] == 'R')
	    state = 2;
	}
      if (state == 2)
	{
	  /* req_xfer_input is true if "ESC[6n" was sent just for
	     triggering transfer_input() in master. In this case,
	     the responce sequence should not be written. */
	  if (!get_ttyp ()->req_xfer_input)
	    WriteFile (to_slave_nat, wpbuf, ixput, &n, NULL);
	  ixput = 0;
	  state = 0;
	  get_ttyp ()->req_xfer_input = false;
	  get_ttyp ()->pcon_start = false;
	}
      ReleaseMutex (input_mutex);

      if (!get_ttyp ()->pcon_start)
	{ /* Pseudo console initialization has been done in above code. */
	  pinfo pp (get_ttyp ()->pcon_start_pid);
	  bool pcon_fg = (pp && get_ttyp ()->getpgid () == pp->pgid);
	  /* GDB may set WINPID rather than cygwin PID to process group
	     when the debugged process is a non-cygwin process.*/
	  pcon_fg |= !pinfo (get_ttyp ()->getpgid ());
	  if (get_ttyp ()->switch_to_nat_pipe && pcon_fg
	      && get_ttyp ()->pty_input_state_eq (tty::to_cyg))
	    {
	      /* This accept_input() call is needed in order to transfer input
		 which is not accepted yet to non-cygwin pipe. */
	      if (get_readahead_valid ())
		accept_input ();
	      WaitForSingleObject (input_mutex, mutex_timeout);
	      acquire_attach_mutex (mutex_timeout);
	      fhandler_pty_slave::transfer_input (tty::to_nat, from_master,
						  get_ttyp (),
						  input_available_event);
	      release_attach_mutex ();
	      ReleaseMutex (input_mutex);
	    }
	  get_ttyp ()->pcon_start_pid = 0;
	}

      return len;
    }

  /* Write terminal input to to_slave_nat pipe instead of output_handle
     if current application is native console application. */
  WaitForSingleObject (input_mutex, mutex_timeout);
  if (to_be_read_from_nat_pipe () && get_ttyp ()->pcon_activated
      && get_ttyp ()->pty_input_state == tty::to_nat)
    { /* Reaches here when non-cygwin app is foreground and pseudo console
	 is activated. */
      tmp_pathbuf tp;
      char *buf = (char *) ptr;
      size_t nlen = len;
      if (get_ttyp ()->term_code_page != CP_UTF8)
	{
	  static mbstate_t mbp;
	  buf = tp.c_get ();
	  nlen = NT_MAX_PATH;
	  convert_mb_str (CP_UTF8, buf, &nlen,
			  get_ttyp ()->term_code_page, (const char *) ptr, len,
			  &mbp);
	}

      for (size_t i = 0; i < nlen; i++)
	{
	  process_sig_state r = process_sigs (buf[i], get_ttyp (), this);
	  if (r == done_with_debugger)
	    {
	      for (size_t j = i; j < nlen - 1; j++)
		buf[j] = buf[j + 1];
	      nlen--;
	      i--;
	    }
	  process_stop_start (buf[i], get_ttyp ());
	}

      DWORD n;
      WriteFile (to_slave_nat, buf, nlen, &n, NULL);
      ReleaseMutex (input_mutex);

      return len;
    }

  /* The code path reaches here when pseudo console is not activated
     or cygwin process is foreground even though pseudo console is
     activated. */

  /* This input transfer is needed when cygwin-app which is started from
     non-cygwin app is terminated if pseudo console is disabled. */
  if (to_be_read_from_nat_pipe () && !get_ttyp ()->pcon_activated
      && get_ttyp ()->pty_input_state == tty::to_cyg)
    {
      acquire_attach_mutex (mutex_timeout);
      fhandler_pty_slave::transfer_input (tty::to_nat, from_master,
					  get_ttyp (), input_available_event);
      release_attach_mutex ();
    }
  ReleaseMutex (input_mutex);

  line_edit_status status = line_edit (p, len, ti, &ret);
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
  /* Workaround for rlwrap v0.40 or later */
  if (get_ttyp ()->pcon_start)
    t->c_lflag &= ~(ICANON | ECHO);
  if (get_ttyp ()->pcon_activated)
    t->c_iflag &= ~ICRNL;
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
  if (queue == TCOFLUSH || queue == TCIOFLUSH)
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
      if (get_ttyp ()->winsize.ws_row != ((struct winsize *) arg)->ws_row
	  || get_ttyp ()->winsize.ws_col != ((struct winsize *) arg)->ws_col
	  || get_ttyp ()->winsize.ws_ypixel != ((struct winsize *) arg)->ws_ypixel
	  || get_ttyp ()->winsize.ws_xpixel != ((struct winsize *) arg)->ws_xpixel
	 )
	{
	  if (get_ttyp ()->pcon_activated && get_ttyp ()->nat_pipe_owner_pid)
	    resize_pseudo_console ((struct winsize *) arg);
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

void
fhandler_pty_slave::setup_locale (void)
{
  extern UINT __eval_codepage_from_internal_charset ();

  if (!get_ttyp ()->term_code_page)
    {
      get_ttyp ()->term_code_page = __eval_codepage_from_internal_charset ();
      acquire_attach_mutex (mutex_timeout);
      SetConsoleCP (get_ttyp ()->term_code_page);
      SetConsoleOutputCP (get_ttyp ()->term_code_page);
      release_attach_mutex ();
    }
}

bg_check_types
fhandler_pty_slave::bg_check (int sig, bool dontsignal)
{
  reset_switch_to_nat_pipe ();
  return fhandler_termios::bg_check (sig, dontsignal);
}

void
fhandler_pty_slave::fixup_after_fork (HANDLE parent)
{
  // fork_fixup (parent, inuse, "inuse");
  // fhandler_pty_common::fixup_after_fork (parent);
  report_tty_counts (this, "inherited", "");
}

void
fhandler_pty_slave::fixup_after_exec ()
{
  create_invisible_console ();

  if (!close_on_exec ())
    fixup_after_fork (NULL);	/* No parent handle required. */

  /* Hook Console API */
#define DO_HOOK(module, name) \
  if (!name##_Orig) \
    { \
      void *api = hook_api (module, #name, (void *) name##_Hooked); \
      name##_Orig = (__typeof__ (name) *) api; \
      /*if (api) system_printf (#name " hooked.");*/ \
    }
  /* CreateProcess() is hooked for GDB etc. */
  DO_HOOK (NULL, CreateProcessA);
  DO_HOOK (NULL, CreateProcessW);
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

/* The function pty_master_thread() should be static because the instance
   is deleted if the master is dup()'ed and the original is closed. In
   this case, dup()'ed instance still exists, therefore, master thread
   is also still alive even though the instance has been deleted. As a
   result, accesing member variables in this function causes access
   violation. */

DWORD
fhandler_pty_master::pty_master_thread (const master_thread_param_t *p)
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
  while (!exit && (ConnectNamedPipe (p->master_ctl, NULL)
		   || GetLastError () == ERROR_PIPE_CONNECTED))
    {
      pipe_reply repl = { NULL, NULL, NULL, NULL, NULL, NULL, 0 };
      bool deimp = false;
      NTSTATUS allow = STATUS_ACCESS_DENIED;
      ACCESS_MASK access = EVENT_MODIFY_STATE;
      HANDLE client = NULL;

      if (!ReadFile (p->master_ctl, &req, sizeof req, &len, NULL))
	{
	  termios_printf ("ReadFile, %E");
	  goto reply;
	}
      if (!GetNamedPipeClientProcessId (p->master_ctl, &pid))
	pid = req.pid;
      if (get_object_sd (p->input_available_event, sd))
	{
	  termios_printf ("get_object_sd, %E");
	  goto reply;
	}
      cygheap->user.deimpersonate ();
      deimp = true;
      if (!ImpersonateNamedPipeClient (p->master_ctl))
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
	  if (!DuplicateHandle (GetCurrentProcess (), p->from_master_nat,
			       client, &repl.from_master_nat,
			       0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (from_master_nat), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), p->from_master,
			       client, &repl.from_master,
			       0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (from_master), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), p->to_master_nat,
				client, &repl.to_master_nat,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_master_nat), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), p->to_master,
				client, &repl.to_master,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_master), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), p->to_slave_nat,
				client, &repl.to_slave_nat,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_slave_nat), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), p->to_slave,
				client, &repl.to_slave,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_slave), %E");
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
		      repl.from_master_nat, repl.to_master_nat, repl.error );
      if (!WriteFile (p->master_ctl, &repl, sizeof repl, &len, NULL))
	termios_printf ("WriteFile, %E");
      if (!DisconnectNamedPipe (p->master_ctl))
	termios_printf ("DisconnectNamedPipe, %E");
    }
  termios_printf ("Leaving");
  return 0;
}

static DWORD WINAPI
pty_master_thread (VOID *arg)
{
  fhandler_pty_master::master_thread_param_t p;
  ((fhandler_pty_master *) arg)->get_master_thread_param (&p);
  return fhandler_pty_master::pty_master_thread (&p);
}

/* The function pty_master_fwd_thread() should be static because the
   instance is deleted if the master is dup()'ed and the original is
   closed. In this case, dup()'ed instance still exists, therefore,
   master forwarding thread is also still alive even though the instance
   has been deleted. As a result, accesing member variables in this
   function causes access violation. */

DWORD
fhandler_pty_master::pty_master_fwd_thread (const master_fwd_thread_param_t *p)
{
  DWORD rlen;
  tmp_pathbuf tp;
  char *outbuf = tp.c_get ();
  char *mbbuf = tp.c_get ();
  static mbstate_t mbp;

  termios_printf ("Started.");
  for (;;)
    {
      p->ttyp->fwd_last_time = GetTickCount ();
      DWORD n;
      p->ttyp->fwd_not_empty =
	::bytes_available (n, p->from_slave_nat) && n;
      if (!ReadFile (p->from_slave_nat, outbuf, NT_MAX_PATH, &rlen, NULL))
	{
	  termios_printf ("ReadFile for forwarding failed, %E");
	  break;
	}
      if (p->ttyp->stop_fwd_thread)
	break;
      ssize_t wlen = rlen;
      char *ptr = outbuf;
      if (p->ttyp->pcon_activated)
	{
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
		const char *helper_str = "\\bin\\cygwin-console-helper.exe";
		if (memmem (&outbuf[start_at], i + 1 - start_at,
			    helper_str, strlen (helper_str)))
		  {
		    memmove (&outbuf[start_at], &outbuf[i+1], rlen-i-1);
		    rlen = wlen = start_at + rlen - i - 1;
		  }
		state = 0;
		continue;
	      }
	    else if (outbuf[i] == '\a')
	      {
		state = 0;
		continue;
	      }

	  /* Remove CSI > Pm m */
	  state = 0;
	  start_at = 0;
	  for (DWORD i = 0; i < rlen; i++)
	    if (outbuf[i] == '\033')
	      {
		start_at = i;
		state = 1;
		continue;
	      }
	    else if ((state == 1 && outbuf[i] == '[')
		     || (state == 2 && outbuf[i] == '>'))
	      {
		state ++;
		continue;
	      }
	    else if (state == 3 && (isdigit (outbuf[i]) || outbuf[i] == ';'))
	      continue;
	    else if (state == 3 && outbuf[i] == 'm')
	      {
		memmove (&outbuf[start_at], &outbuf[i+1], rlen-i-1);
		rlen = wlen = start_at + rlen - i - 1;
		state = 0;
		i = start_at - 1;
		continue;
	      }
	    else
	      state = 0;

	  /* Remove OSC Ps ; ? BEL/ST */
	  for (DWORD i = 0; i < rlen; i++)
	    if (state == 0 && outbuf[i] == '\033')
	      {
		start_at = i;
		state = 1;
		continue;
	      }
	    else if ((state == 1 && outbuf[i] == ']')
		     || (state == 2 && outbuf[i] == ';')
		     || (state == 3 && outbuf[i] == '?')
		     || (state == 4 && outbuf[i] == '\033'))
	      {
		state ++;
		continue;
	      }
	    else if (state == 2 && isdigit (outbuf[i]))
	      continue;
	    else if ((state == 4 && outbuf[i] == '\a')
		     || (state == 5 && outbuf[i] == '\\'))
	      {
		memmove (&outbuf[start_at], &outbuf[i+1], rlen-i-1);
		rlen = wlen = start_at + rlen - i - 1;
		state = 0;
		i = start_at - 1;
		continue;
	      }
	    else
	      state = 0;

	  if (p->ttyp->term_code_page != CP_UTF8)
	    {
	      size_t nlen = NT_MAX_PATH;
	      convert_mb_str (p->ttyp->term_code_page, mbbuf, &nlen,
			      CP_UTF8, ptr, wlen, &mbp);

	      ptr = mbbuf;
	      wlen = rlen = nlen;
	    }

	  /* OPOST processing was already done in pseudo console,
	     so just write it to to_master. */
	  DWORD written;
	  while (rlen>0)
	    {
	      if (!WriteFile (p->to_master, ptr, wlen, &written, NULL))
		{
		  termios_printf ("WriteFile for forwarding failed, %E");
		  break;
		}
	      ptr += written;
	      wlen = (rlen -= written);
	    }
	  continue;
	}

      UINT cp_from;
      pinfo pinfo_target = pinfo (p->ttyp->invisible_console_pid);
      DWORD target_pid = 0;
      if (pinfo_target)
	target_pid = pinfo_target->dwProcessId;
      if (target_pid)
	{
	  /* Slave attaches to a different console than master.
	     Therefore reattach here. */
	  DWORD resume_pid = attach_console_temporarily (target_pid);
	  cp_from = GetConsoleOutputCP ();
	  resume_from_temporarily_attach (resume_pid);
	}
      else
	cp_from = GetConsoleOutputCP ();

      if (p->ttyp->term_code_page != cp_from)
	{
	  size_t nlen = NT_MAX_PATH;
	  convert_mb_str (p->ttyp->term_code_page, mbbuf, &nlen,
			  cp_from, ptr, wlen, &mbp);

	  ptr = mbbuf;
	  wlen = rlen = nlen;
	}

      WaitForSingleObject (p->output_mutex, mutex_timeout);
      while (rlen>0)
	{
	  if (!process_opost_output (p->to_master, ptr, wlen,
				     true /* disable output_stopped */,
				     p->ttyp, false))
	    {
	      termios_printf ("WriteFile for forwarding failed, %E");
	      break;
	    }
	  ptr += wlen;
	  wlen = (rlen -= wlen);
	}
      ReleaseMutex (p->output_mutex);
    }
  return 0;
}

static DWORD WINAPI
pty_master_fwd_thread (VOID *arg)
{
  fhandler_pty_master::master_fwd_thread_param_t p;
  ((fhandler_pty_master *) arg)->get_master_fwd_thread_param (&p);
  return fhandler_pty_master::pty_master_fwd_thread (&p);
}

inline static bool
is_running_as_service (void)
{
  return check_token_membership (well_known_service_sid)
    || cygheap->user.saved_sid () == well_known_system_sid;
}

bool
fhandler_pty_master::setup ()
{
  int res;
  security_descriptor sd;
  SECURITY_ATTRIBUTES sa = { sizeof (SECURITY_ATTRIBUTES), NULL, TRUE };

  /* Find an unallocated pty to use. */
  /* Create a pipe for cygwin app (master to slave) simultaneously. */
  int unit = cygwin_shared->tty.allocate (from_master, get_output_handle ());
  if (unit < 0)
    return false;

  ProtectHandle1 (get_output_handle (), to_pty);

  tty& t = *cygwin_shared->tty[unit];
  _tc = (tty_min *) &t;

  tcinit (true);		/* Set termios information.  Force initialization. */

  const char *errstr = NULL;
  DWORD pipe_mode = PIPE_NOWAIT;

  if (!SetNamedPipeHandleState (get_output_handle (), &pipe_mode, NULL, NULL))
    termios_printf ("can't set output_handle(%p) to non-blocking mode",
		    get_output_handle ());

  /* Pipe for non-cygwin apps (slave to master) */
  char pipename[sizeof ("ptyNNNN-from-master-nat")];
  __small_sprintf (pipename, "pty%d-to-master-nat", unit);
  res = fhandler_pipe::create (&sec_none, &from_slave_nat, &to_master_nat,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "output pipe for non-cygwin apps";
      goto err;
    }

  /* Pipe for cygwin apps (slave to master) */
  __small_sprintf (pipename, "pty%d-to-master", unit);
  res = fhandler_pipe::create (&sec_none, &get_handle (), &to_master,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "output pipe";
      goto err;
    }

  /* Pipe for non-cygwin apps (master to slave) */
  __small_sprintf (pipename, "pty%d-from-master-nat", unit);
  /* FILE_FLAG_OVERLAPPED is specified here in order to prevent
     PeekNamedPipe() from blocking in transfer_input().
     Accordig to the official document, in order to access the handle
     opened with FILE_FLAG_OVERLAPPED, it is mandatory to pass the
     OVERLAPP structure, but in fact, it seems that the access will
     fallback to the blocking access if it is not specified. */
  res = fhandler_pipe::create (&sec_none, &from_master_nat, &to_slave_nat,
			       fhandler_pty_common::pipesize, pipename,
			       FILE_FLAG_OVERLAPPED);
  if (res)
    {
      errstr = "input pipe";
      goto err;
    }

  ProtectHandle1 (get_handle (), from_pty);

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

  errstr = shared_name (buf, PIPE_SW_MUTEX, unit);
  if (!(pipe_sw_mutex = CreateMutex (&sa, FALSE, buf)))
    goto err;

  if (!attach_mutex)
    attach_mutex = CreateMutex (&sa, FALSE, NULL);

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

  thread_param_copied_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  master_thread = new cygthread (::pty_master_thread, this, "ptym");
  if (!master_thread)
    {
      errstr = "pty master control thread";
      goto err;
    }
  WaitForSingleObject (thread_param_copied_event, INFINITE);
  master_fwd_thread = new cygthread (::pty_master_fwd_thread, this, "ptymf");
  if (!master_fwd_thread)
    {
      errstr = "pty master forwarding thread";
      goto err;
    }
  WaitForSingleObject (thread_param_copied_event, INFINITE);
  CloseHandle (thread_param_copied_event);

  t.set_from_master_nat (from_master_nat);
  t.set_from_master (from_master);
  t.set_to_master_nat (to_master_nat);
  t.set_to_master (to_master);
  t.set_to_slave_nat (to_slave_nat);
  t.set_to_slave (get_output_handle ());
  t.winsize.ws_col = 80;
  t.winsize.ws_row = 25;
  t.master_pid = myself->pid;

  dev ().parse (DEV_PTYM_MAJOR, unit);

  t.master_is_running_as_service = is_running_as_service ();

  termios_printf ("this %p, pty%d opened - from_pty <%p,%p>, to_pty %p",
	this, unit, from_slave_nat, get_handle (),
	get_output_handle ());
  return true;

err:
  __seterrno ();
  close_maybe (from_slave_nat);
  close_maybe (to_slave_nat);
  close_maybe (get_handle ());
  close_maybe (get_output_handle ());
  close_maybe (input_available_event);
  close_maybe (output_mutex);
  close_maybe (input_mutex);
  close_maybe (from_master_nat);
  close_maybe (to_master_nat);
  close_maybe (to_master);
  close_maybe (echo_r);
  close_maybe (echo_w);
  close_maybe (master_ctl);
  /* The from_master must be closed last so that the same pty is not
     allocated before cleaning up the other corresponding instances. */
  close_maybe (from_master);
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
	  t.set_from_master_nat (arch->from_master_nat);
	  t.set_from_master (arch->from_master);
	  t.set_to_master_nat (arch->to_master_nat);
	  t.set_to_master (arch->to_master);
	}
      arch->dwProcessId = wpid;
    }
  from_master_nat = arch->from_master_nat;
  from_master = arch->from_master;
  to_master_nat = arch->to_master_nat;
  to_master = arch->to_master;
#if 0 /* Not sure if this is necessary. */
  from_slave_nat = arch->from_slave_nat;
  to_slave_nat = arch->to_slave_nat;
#endif
  report_tty_counts (this, "inherited master", "");
}

void
fhandler_pty_master::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_after_fork (spawn_info->parent);
  else
    from_master_nat = from_master = to_master_nat = to_master =
      from_slave_nat = to_slave_nat = NULL;
}

BOOL
fhandler_pty_common::process_opost_output (HANDLE h, const void *ptr,
					   ssize_t& len, bool is_echo,
					   tty *ttyp, bool is_nonblocking)
{
  ssize_t towrite = len;
  BOOL res = TRUE;
  if (ttyp->ti.c_lflag & FLUSHO)
    return res; /* Discard write data */
  while (towrite)
    {
      if (!is_echo)
	{
	  if (ttyp->output_stopped && is_nonblocking)
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
	  while (ttyp->output_stopped)
	    cygwait (10);
	}

      if (!(ttyp->ti.c_oflag & OPOST))	// raw output mode
	{
	  DWORD n = MIN (OUT_BUFFER_SIZE, towrite);
	  res = WriteFile (h, ptr, n, &n, NULL);
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
		  if ((ttyp->ti.c_oflag & ONOCR)
		      && ttyp->column == 0)
		    {
		      rc++;
		      continue;
		    }
		  if (ttyp->ti.c_oflag & OCRNL)
		    {
		      outbuf[n++] = '\n';
		      rc++;
		    }
		  else
		    {
		      outbuf[n++] = buf[rc++];
		      ttyp->column = 0;
		    }
		  break;
		case '\n':
		  if (ttyp->ti.c_oflag & ONLCR)
		    {
		      outbuf[n++] = '\r';
		      ttyp->column = 0;
		    }
		  if (ttyp->ti.c_oflag & ONLRET)
		    ttyp->column = 0;
		  outbuf[n++] = buf[rc++];
		  break;
		default:
		  outbuf[n++] = buf[rc++];
		  ttyp->column++;
		  break;
		}
	    }
	  res = WriteFile (h, outbuf, n, &n, NULL);
	  if (!res)
	    break;
	  ptr = (char *) ptr + rc;
	  towrite -= rc;
	}
    }
  len -= towrite;
  return res;
}

  /* Pseudo console supprot is realized using a tricky technic.
     PTY need the pseudo console handles, however, they cannot
     be retrieved by normal procedure. Therefore, run a helper
     process in a pseudo console and get them from the helper.
     Slave process will attach to the pseudo console in the
     helper process using AttachConsole(). */
bool
fhandler_pty_slave::setup_pseudoconsole ()
{
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

  HANDLE hpConIn, hpConOut;
  if (get_ttyp ()->pcon_activated)
    { /* The pseudo console is already activated. */
      if (GetStdHandle (STD_INPUT_HANDLE) == get_handle ())
	{ /* Send CSI6n just for requesting transfer input. */
	  DWORD n;
	  WaitForSingleObject (input_mutex, mutex_timeout);
	  get_ttyp ()->req_xfer_input = true; /* indicates that this "ESC[6n"
						 is just for transfer input */
	  get_ttyp ()->pcon_start = true;
	  get_ttyp ()->pcon_start_pid = myself->pid;
	  WriteFile (get_output_handle (), "\033[6n", 4, &n, NULL);
	  ReleaseMutex (input_mutex);
	  while (get_ttyp ()->pcon_start_pid)
	    /* wait for completion of transfer_input() in master::write(). */
	    Sleep (1);
	}
      /* Attach to the pseudo console which already exits. */
      HANDLE pcon_owner = OpenProcess (PROCESS_DUP_HANDLE, FALSE,
				       get_ttyp ()->nat_pipe_owner_pid);
      if (pcon_owner == NULL)
	return false;
      DuplicateHandle (pcon_owner, get_ttyp ()->h_pcon_in,
		       GetCurrentProcess (), &hpConIn,
		       0, TRUE, DUPLICATE_SAME_ACCESS);
      DuplicateHandle (pcon_owner, get_ttyp ()->h_pcon_out,
		       GetCurrentProcess (), &hpConOut,
		       0, TRUE, DUPLICATE_SAME_ACCESS);
      CloseHandle (pcon_owner);
      FreeConsole ();
      AttachConsole (get_ttyp ()->nat_pipe_owner_pid);
      init_console_handler (false);
      goto skip_create;
    }

  STARTUPINFOEXW si;
  PROCESS_INFORMATION pi;
  HANDLE hello, goodbye;
  HANDLE hr, hw;
  HPCON hpcon;

  do
    { /* Create new pseudo console */
      COORD size = {
	(SHORT) get_ttyp ()->winsize.ws_col,
	(SHORT) get_ttyp ()->winsize.ws_row
      };
      const DWORD inherit_cursor = 1;
      hpcon = NULL;
      SetLastError (ERROR_SUCCESS);
      HRESULT res = CreatePseudoConsole (size, get_handle_nat (),
					 get_output_handle_nat (),
					 inherit_cursor, &hpcon);
      if (res != S_OK || GetLastError () == ERROR_PROC_NOT_FOUND)
	{
	  if (res != S_OK)
	    system_printf ("CreatePseudoConsole() failed. %08x %08x\n",
			   GetLastError (), res);
	  goto fallback;
	}

      SIZE_T bytesRequired;
      InitializeProcThreadAttributeList (NULL, 2, 0, &bytesRequired);
      ZeroMemory (&si, sizeof (si));
      si.StartupInfo.cb = sizeof (STARTUPINFOEXW);
      si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)
	HeapAlloc (GetProcessHeap (), 0, bytesRequired);
      if (si.lpAttributeList == NULL)
	goto cleanup_pseudo_console;
      if (!InitializeProcThreadAttributeList (si.lpAttributeList,
					      2, 0, &bytesRequired))
	goto cleanup_heap;
      if (!UpdateProcThreadAttribute (si.lpAttributeList,
				      0,
				      PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
				      hpcon, sizeof (hpcon), NULL, NULL))

	goto cleanup_heap;

      hello = CreateEvent (&sec_none, true, false, NULL);
      goodbye = CreateEvent (&sec_none, true, false, NULL);
      CreatePipe (&hr, &hw, &sec_none, 0);

      HANDLE handles_to_inherit[] = {hello, goodbye, hw};
      if (!UpdateProcThreadAttribute (si.lpAttributeList,
				      0,
				      PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
				      handles_to_inherit,
				      sizeof (handles_to_inherit),
				      NULL, NULL))
	goto cleanup_event_and_pipes;

      /* Execute helper process */
      WCHAR cmd[MAX_PATH];
      path_conv helper ("/bin/cygwin-console-helper.exe");
      size_t len = helper.get_wide_win32_path_len ();
      helper.get_wide_win32_path (cmd);
      __small_swprintf (cmd + len, L" %p %p %p", hello, goodbye, hw);
      si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
      si.StartupInfo.hStdInput = NULL;
      si.StartupInfo.hStdOutput = NULL;
      si.StartupInfo.hStdError = NULL;

      get_ttyp ()->pcon_activated = true;
      get_ttyp ()->pcon_start = true;
      get_ttyp ()->pcon_start_pid = myself->pid;
      if (!CreateProcessW (NULL, cmd, &sec_none, &sec_none,
			   TRUE, EXTENDED_STARTUPINFO_PRESENT,
			   NULL, NULL, &si.StartupInfo, &pi))
	goto cleanup_event_and_pipes;

      for (;;)
	{
	  DWORD wait_result = WaitForSingleObject (hello, 500);
	  if (wait_result == WAIT_OBJECT_0)
	    break;
	  if (wait_result != WAIT_TIMEOUT)
	    goto cleanup_helper_with_hello;
	  DWORD exit_code;
	  if (!GetExitCodeProcess (pi.hProcess, &exit_code))
	    goto cleanup_helper_with_hello;
	  if (exit_code == STILL_ACTIVE)
	    continue;
	  if (exit_code != 0 ||
	      WaitForSingleObject (hello, 500) != WAIT_OBJECT_0)
	    goto cleanup_helper_with_hello;
	  break;
	}
      CloseHandle (hello);
      CloseHandle (pi.hThread);

      /* Duplicate pseudo console handles */
      DWORD rlen;
      char buf[64];
      if (!ReadFile (hr, buf, sizeof (buf), &rlen, NULL))
	goto cleanup_helper_process;
      buf[rlen] = '\0';
      sscanf (buf, "StdHandles=%p,%p", &hpConIn, &hpConOut);
      if (!DuplicateHandle (pi.hProcess, hpConIn,
			    GetCurrentProcess (), &hpConIn, 0,
			    TRUE, DUPLICATE_SAME_ACCESS))
	goto cleanup_helper_process;
      if (!DuplicateHandle (pi.hProcess, hpConOut,
			    GetCurrentProcess (), &hpConOut, 0,
			    TRUE, DUPLICATE_SAME_ACCESS))
	goto cleanup_pcon_in;

      CloseHandle (hr);
      CloseHandle (hw);
      DeleteProcThreadAttributeList (si.lpAttributeList);
      HeapFree (GetProcessHeap (), 0, si.lpAttributeList);

      /* Attach to pseudo console */
      FreeConsole ();
      AttachConsole (pi.dwProcessId);
      init_console_handler (false);

      /* Terminate helper process */
      SetEvent (goodbye);
      WaitForSingleObject (pi.hProcess, INFINITE);
      CloseHandle (goodbye);
      CloseHandle (pi.hProcess);
    }
  while (false);

skip_create:
  do
    {
      /* Fixup handles */
      HANDLE orig_input_handle_nat = get_handle_nat ();
      HANDLE orig_output_handle_nat = get_output_handle_nat ();
      cygheap_fdenum cfd (false);
      while (cfd.next () >= 0)
	if (cfd->get_device () == get_device ())
	  {
	    fhandler_base *fh = cfd;
	    fhandler_pty_slave *ptys = (fhandler_pty_slave *) fh;
	    if (ptys->get_handle_nat () == orig_input_handle_nat)
	      ptys->set_handle_nat (hpConIn);
	    if (ptys->get_output_handle_nat () == orig_output_handle_nat)
	      ptys->set_output_handle_nat (hpConOut);
	  }
      CloseHandle (orig_input_handle_nat);
      CloseHandle (orig_output_handle_nat);
    }
  while (false);

  if (!process_alive (get_ttyp ()->nat_pipe_owner_pid))
    get_ttyp ()->nat_pipe_owner_pid = myself->exec_dwProcessId;

  if (hpcon && nat_pipe_owner_self (get_ttyp ()->nat_pipe_owner_pid))
    {
      HPCON_INTERNAL *hp = (HPCON_INTERNAL *) hpcon;
      get_ttyp ()->h_pcon_write_pipe = hp->hWritePipe;
      get_ttyp ()->h_pcon_condrv_reference = hp->hConDrvReference;
      get_ttyp ()->h_pcon_conhost_process = hp->hConHostProcess;
      DuplicateHandle (GetCurrentProcess (), hpConIn,
		       GetCurrentProcess (), &get_ttyp ()->h_pcon_in,
		       0, TRUE, DUPLICATE_SAME_ACCESS);
      DuplicateHandle (GetCurrentProcess (), hpConOut,
		       GetCurrentProcess (), &get_ttyp ()->h_pcon_out,
		       0, TRUE, DUPLICATE_SAME_ACCESS);
      /* Discard the pseudo console handler container here.
	 Reconstruct it temporary when it is needed. */
      HeapFree (GetProcessHeap (), 0, hp);
    }

  if (get_ttyp ()->previous_code_page)
    SetConsoleCP (get_ttyp ()->previous_code_page);
  if (get_ttyp ()->previous_output_code_page)
    SetConsoleOutputCP (get_ttyp ()->previous_output_code_page);

  if (get_ttyp ()->getpgid () == myself->pgid)
    {
      termios &t = get_ttyp ()->ti;
      DWORD mode;
      /* Set input mode */
      GetConsoleMode (hpConIn, &mode);
      mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
      if (t.c_lflag & ECHO)
	mode |= ENABLE_ECHO_INPUT;
      if (t.c_lflag & ICANON)
	mode |= ENABLE_LINE_INPUT;
      if (mode & ENABLE_ECHO_INPUT && !(mode & ENABLE_LINE_INPUT))
	/* This is illegal, so turn off the echo here, and fake it
	   when we read the characters */
	mode &= ~ENABLE_ECHO_INPUT;
      if (t.c_lflag & ISIG)
	mode |= ENABLE_PROCESSED_INPUT;
      SetConsoleMode (hpConIn, mode);
      /* Set output mode */
      GetConsoleMode (hpConOut, &mode);
      mode &= ~DISABLE_NEWLINE_AUTO_RETURN;
      if (!(t.c_oflag & OPOST) || !(t.c_oflag & ONLCR))
	mode |= DISABLE_NEWLINE_AUTO_RETURN;
      SetConsoleMode (hpConOut, mode);
    }

  return true;

cleanup_helper_with_hello:
  CloseHandle (hello);
  CloseHandle (pi.hThread);
  goto cleanup_helper_process;
cleanup_pcon_in:
  CloseHandle (hpConIn);
cleanup_helper_process:
  SetEvent (goodbye);
  WaitForSingleObject (pi.hProcess, INFINITE);
  CloseHandle (pi.hProcess);
  goto skip_close_hello;
cleanup_event_and_pipes:
  CloseHandle (hello);
skip_close_hello:
  get_ttyp ()->pcon_start = false;
  get_ttyp ()->pcon_start_pid = 0;
  get_ttyp ()->pcon_activated = false;
  CloseHandle (goodbye);
  CloseHandle (hr);
  CloseHandle (hw);
cleanup_heap:
  HeapFree (GetProcessHeap (), 0, si.lpAttributeList);
cleanup_pseudo_console:
  if (hpcon)
    {
      HPCON_INTERNAL *hp = (HPCON_INTERNAL *) hpcon;
      HANDLE tmp = hp->hConHostProcess;
      ClosePseudoConsole (hpcon);
      CloseHandle (tmp);
    }
fallback:
  return false;
}

/* Find a process to which the ownership of nat pipe should be handed over */
DWORD
fhandler_pty_slave::get_winpid_to_hand_over (tty *ttyp,
					     DWORD force_switch_to)
{
  DWORD switch_to = 0;
  if (force_switch_to)
    {
      switch_to = force_switch_to;
      ttyp->setpgid (force_switch_to + MAX_PID);
    }
  else if (nat_pipe_owner_self (ttyp->nat_pipe_owner_pid))
    {
      /* Search another native process which attaches to the same console */
      DWORD current_pid = myself->exec_dwProcessId ?: myself->dwProcessId;
      switch_to = get_console_process_id (current_pid, false, true, true);
      if (!switch_to)
	switch_to = get_console_process_id (current_pid, false, true, false);
    }
  return switch_to;
}

void
fhandler_pty_slave::hand_over_only (tty *ttyp, DWORD force_switch_to)
{
  if (nat_pipe_owner_self (ttyp->nat_pipe_owner_pid))
    {
      DWORD switch_to = get_winpid_to_hand_over (ttyp, force_switch_to);
      if (switch_to)
	/* The process switch_to takes over the ownership of the nat pipe. */
	ttyp->nat_pipe_owner_pid = switch_to;
      else
	{
	  /* Abandon the ownership of the nat pipe */
	  ttyp->nat_pipe_owner_pid = 0;
	  ttyp->switch_to_nat_pipe = false;
	}
    }
}

/* The function close_pseudoconsole() should be static so that it can
   be called even after the fhandler_pty_slave instance is deleted. */
void
fhandler_pty_slave::close_pseudoconsole (tty *ttyp, DWORD force_switch_to)
{
  DWORD switch_to = get_winpid_to_hand_over (ttyp, force_switch_to);
  ttyp->previous_code_page = GetConsoleCP ();
  ttyp->previous_output_code_page = GetConsoleOutputCP ();
  if (nat_pipe_owner_self (ttyp->nat_pipe_owner_pid))
    { /* I am owner of the nat pipe. */
      if (switch_to)
	{
	  /* Change pseudo console owner to another process (switch_to). */
	  HANDLE new_owner =
	    OpenProcess (PROCESS_DUP_HANDLE, FALSE, switch_to);
	  HANDLE new_write_pipe = NULL;
	  HANDLE new_condrv_reference = NULL;
	  HANDLE new_conhost_process = NULL;
	  HANDLE new_pcon_in = NULL, new_pcon_out = NULL;
	  DuplicateHandle (GetCurrentProcess (),
			   ttyp->h_pcon_write_pipe,
			   new_owner, &new_write_pipe,
			   0, TRUE, DUPLICATE_SAME_ACCESS);
	  DuplicateHandle (GetCurrentProcess (),
			   ttyp->h_pcon_condrv_reference,
			   new_owner, &new_condrv_reference,
			   0, TRUE, DUPLICATE_SAME_ACCESS);
	  DuplicateHandle (GetCurrentProcess (),
			   ttyp->h_pcon_conhost_process,
			   new_owner, &new_conhost_process,
			   0, TRUE, DUPLICATE_SAME_ACCESS);
	  DuplicateHandle (GetCurrentProcess (), ttyp->h_pcon_in,
			   new_owner, &new_pcon_in,
			   0, TRUE, DUPLICATE_SAME_ACCESS);
	  DuplicateHandle (GetCurrentProcess (), ttyp->h_pcon_out,
			   new_owner, &new_pcon_out,
			   0, TRUE, DUPLICATE_SAME_ACCESS);
	  CloseHandle (new_owner);
	  CloseHandle (ttyp->h_pcon_write_pipe);
	  CloseHandle (ttyp->h_pcon_condrv_reference);
	  CloseHandle (ttyp->h_pcon_conhost_process);
	  CloseHandle (ttyp->h_pcon_in);
	  CloseHandle (ttyp->h_pcon_out);
	  ttyp->nat_pipe_owner_pid = switch_to;
	  ttyp->h_pcon_write_pipe = new_write_pipe;
	  ttyp->h_pcon_condrv_reference = new_condrv_reference;
	  ttyp->h_pcon_conhost_process = new_conhost_process;
	  ttyp->h_pcon_in = new_pcon_in;
	  ttyp->h_pcon_out = new_pcon_out;
	  FreeConsole ();
	  pinfo p (myself->ppid);
	  if (!p || !AttachConsole (p->dwProcessId))
	    AttachConsole (ATTACH_PARENT_PROCESS);
	  init_console_handler (false);
	}
      else
	{ /* Close pseudo console and abandon the ownership of the nat pipe. */
	  FreeConsole ();
	  pinfo p (myself->ppid);
	  if (!p || !AttachConsole (p->dwProcessId))
	    AttachConsole (ATTACH_PARENT_PROCESS);
	  init_console_handler (false);
	  /* Reconstruct pseudo console handler container here for close */
	  HPCON_INTERNAL *hp =
	    (HPCON_INTERNAL *) HeapAlloc (GetProcessHeap (), 0,
					  sizeof (HPCON_INTERNAL));
	  hp->hWritePipe = ttyp->h_pcon_write_pipe;
	  hp->hConDrvReference = ttyp->h_pcon_condrv_reference;
	  hp->hConHostProcess = ttyp->h_pcon_conhost_process;
	  /* HeapFree() will be called in ClosePseudoConsole() */
	  ClosePseudoConsole ((HPCON) hp);
	  CloseHandle (ttyp->h_pcon_conhost_process);
	  ttyp->pcon_activated = false;
	  ttyp->switch_to_nat_pipe = false;
	  ttyp->nat_pipe_owner_pid = 0;
	  ttyp->pcon_start = false;
	  ttyp->pcon_start_pid = 0;
	}
    }
  else
    { /* Just detach from the pseudo console if I am not owner. */
      FreeConsole ();
      pinfo p (myself->ppid);
      if (!p || !AttachConsole (p->dwProcessId))
	AttachConsole (ATTACH_PARENT_PROCESS);
      init_console_handler (false);
    }
}

static bool
has_ansi_escape_sequences (const WCHAR *env)
{
  /* Retrieve TERM name */
  const char *term = NULL;
  char term_str[260];
  if (env)
    {
    for (const WCHAR *p = env; *p != L'\0'; p += wcslen (p) + 1)
      if (swscanf (p, L"TERM=%236s", term_str) == 1)
	{
	  term = term_str;
	  break;
	}
    }
  else
    term = getenv ("TERM");

  if (!term)
    return false;

  /* If cursor_home is not "\033[H", terminal is not supposed to
     support ANSI escape sequences. */
  char tinfo[260];
  __small_sprintf (tinfo, "/usr/share/terminfo/%02x/%s", term[0], term);
  path_conv path (tinfo);
  WCHAR wtinfo[260];
  path.get_wide_win32_path (wtinfo);
  HANDLE h;
  h = CreateFileW (wtinfo, GENERIC_READ, FILE_SHARE_READ,
		   NULL, OPEN_EXISTING, 0, NULL);
  if (h == NULL)
    return false;
  char terminfo[4096];
  DWORD n;
  ReadFile (h, terminfo, sizeof (terminfo), &n, 0);
  CloseHandle (h);

  int num_size = 2;
  if (*(int16_t *)terminfo == 01036 /* MAGIC2 */)
    num_size = 4;
  const int name_pos = 12; /* Position of terminal name */
  const int name_size = *(int16_t *) (terminfo + 2);
  const int bool_count = *(int16_t *) (terminfo + 4);
  const int num_count = *(int16_t *) (terminfo + 6);
  const int str_count = *(int16_t *) (terminfo + 8);
  const int str_size = *(int16_t *) (terminfo + 10);
  const int cursor_home = 12; /* cursor_home entry index */
  if (cursor_home >= str_count)
    return false;
  int str_idx_pos = name_pos + name_size + bool_count + num_size * num_count;
  if (str_idx_pos & 1)
    str_idx_pos ++;
  const int16_t *str_idx = (int16_t *) (terminfo + str_idx_pos);
  const char *str_table = (const char *) (str_idx + str_count);
  if (str_idx + cursor_home >= (int16_t *) (terminfo + n))
    return false;
  if (str_idx[cursor_home] == -1)
    return false;
  const char *cursor_home_str = str_table + str_idx[cursor_home];
  if (cursor_home_str >= str_table + str_size)
    return false;
  if (cursor_home_str >= terminfo + n)
    return false;
  if (strcmp (cursor_home_str, "\033[H") != 0)
    return false;
  return true;
}

bool
fhandler_pty_slave::term_has_pcon_cap (const WCHAR *env)
{
  if (get_ttyp ()->pcon_cap_checked)
    return get_ttyp ()->has_csi6n;

  DWORD n;
  char buf[1024];
  char *p;
  int len;
  int wait_cnt = 0;

  /* Check if terminal has ANSI escape sequence. */
  if (!has_ansi_escape_sequences (env))
    goto maybe_dumb;

  /* Check if terminal has CSI6n */
  WaitForSingleObject (pipe_sw_mutex, INFINITE);
  WaitForSingleObject (input_mutex, mutex_timeout);
  /* Set pcon_activated and pcon_start so that the response
     will sent to io_handle_nat rather than io_handle. */
  get_ttyp ()->pcon_activated = true;
  /* pcon_start will be cleared in master write() when CSI6n is responded. */
  get_ttyp ()->pcon_start = true;
  WriteFile (get_output_handle (), "\033[6n", 4, &n, NULL);
  ReleaseMutex (input_mutex);
  p = buf;
  len = sizeof (buf) - 1;
  do
    {
      if (::bytes_available (n, get_handle_nat ()) && n)
	{
	  ReadFile (get_handle_nat (), p, len, &n, NULL);
	  p += n;
	  len -= n;
	  *p = '\0';
	  char *p1 = strrchr (buf, '\033');
	  int x, y;
	  char c;
	  if (p1 == NULL || sscanf (p1, "\033[%d;%d%c", &y, &x, &c) != 3
	      || c != 'R')
	    continue;
	  wait_cnt = 0;
	  break;
	}
      else if (++wait_cnt > 100) /* Timeout */
	goto not_has_csi6n;
      else
	Sleep (1);
    }
  while (len);
  get_ttyp ()->pcon_activated = false;
  get_ttyp ()->nat_pipe_owner_pid = 0;
  ReleaseMutex (pipe_sw_mutex);
  if (len == 0)
    goto not_has_csi6n;

  get_ttyp ()->has_csi6n = true;
  get_ttyp ()->pcon_cap_checked = true;

  return true;

not_has_csi6n:
  WaitForSingleObject (input_mutex, mutex_timeout);
  /* If CSI6n is not responded, pcon_start is not cleared
     in master write(). Therefore, clear it here manually. */
  get_ttyp ()->pcon_start = false;
  get_ttyp ()->pcon_activated = false;
  ReleaseMutex (input_mutex);
  ReleaseMutex (pipe_sw_mutex);
maybe_dumb:
  get_ttyp ()->pcon_cap_checked = true;
  return false;
}

void
fhandler_pty_slave::create_invisible_console ()
{
  if (get_ttyp ()->need_invisible_console)
    {
      /* Detach from console device and create new invisible console. */
      acquire_attach_mutex (mutex_timeout);
      FreeConsole();
      fhandler_console::need_invisible (true);
      init_console_handler (false);
      release_attach_mutex ();
      get_ttyp ()->need_invisible_console = false;
      get_ttyp ()->invisible_console_pid = myself->pid;
    }
  if (get_ttyp ()->invisible_console_pid
      && !pinfo (get_ttyp ()->invisible_console_pid))
    /* If primary slave process does not exist anymore,
       this process becomes the primary. */
    get_ttyp ()->invisible_console_pid = myself->pid;
}

void
fhandler_pty_master::get_master_thread_param (master_thread_param_t *p)
{
  p->from_master_nat = from_master_nat;
  p->from_master = from_master;
  p->to_master_nat = to_master_nat;
  p->to_master = to_master;
  p->to_slave_nat = to_slave_nat;
  p->to_slave = get_output_handle ();
  p->master_ctl = master_ctl;
  p->input_available_event = input_available_event;
  SetEvent (thread_param_copied_event);
}

void
fhandler_pty_master::get_master_fwd_thread_param (master_fwd_thread_param_t *p)
{
  p->to_master = to_master;
  p->from_slave_nat = from_slave_nat;
  p->output_mutex = output_mutex;
  p->ttyp = get_ttyp ();
  SetEvent (thread_param_copied_event);
}

#define ALT_PRESSED (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)
#define CTRL_PRESSED (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)
void
fhandler_pty_slave::transfer_input (tty::xfer_dir dir, HANDLE from, tty *ttyp,
				    HANDLE input_available_event)
{
  HANDLE to;
  if (dir == tty::to_nat)
    to = ttyp->to_slave_nat ();
  else
    to = ttyp->to_slave ();

  pinfo p (ttyp->master_pid);
  HANDLE pty_owner = OpenProcess (PROCESS_DUP_HANDLE, FALSE, p->dwProcessId);
  if (pty_owner)
    {
      DuplicateHandle (pty_owner, to, GetCurrentProcess (), &to,
		       0, TRUE, DUPLICATE_SAME_ACCESS);
      CloseHandle (pty_owner);
    }
  else
    {
      char pipe[MAX_PATH];
      __small_sprintf (pipe,
		       "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
		       &cygheap->installation_key, ttyp->get_minor ());
      pipe_request req = { GetCurrentProcessId () };
      pipe_reply repl;
      DWORD len;
      if (!CallNamedPipe (pipe, &req, sizeof req,
			  &repl, sizeof repl, &len, 500))
	return; /* What can we do? */
      if (dir == tty::to_nat)
	to = repl.to_slave_nat;
      else
	to = repl.to_slave;
    }

  UINT cp_from = 0, cp_to = 0;

  if (dir == tty::to_nat)
    {
      cp_from = ttyp->term_code_page;
      if (ttyp->pcon_activated)
	cp_to = CP_UTF8;
      else
	cp_to = GetConsoleCP ();
    }
  else
    {
      cp_from = GetConsoleCP ();
      cp_to = ttyp->term_code_page;
    }

  tmp_pathbuf tp;
  char *buf = tp.c_get ();

  bool transfered = false;

  if (dir == tty::to_cyg && ttyp->pcon_activated)
    { /* from handle is console handle */
      /* Reaches here for nat->cyg case with pcon activated. */
      INPUT_RECORD r[INREC_SIZE];
      DWORD n;
      while (PeekConsoleInputA (from, r, INREC_SIZE, &n) && n)
	{
	  ReadConsoleInputA (from, r, n, &n);
	  if (ttyp->discard_input)
	    continue;
	  int len = 0;
	  char *ptr = buf;
	  for (DWORD i = 0; i < n; i++)
	    if (r[i].EventType == KEY_EVENT && r[i].Event.KeyEvent.bKeyDown)
	      {
		DWORD ctrl_key_state = r[i].Event.KeyEvent.dwControlKeyState;
		if (r[i].Event.KeyEvent.uChar.AsciiChar)
		  {
		    if ((ctrl_key_state & ALT_PRESSED)
			&& r[i].Event.KeyEvent.uChar.AsciiChar <= 0x7f)
		      buf[len++] = '\033'; /* Meta */
		    buf[len++] = r[i].Event.KeyEvent.uChar.AsciiChar;
		  }
		/* Allow Ctrl-Space to emit ^@ */
		else if (r[i].Event.KeyEvent.wVirtualKeyCode == '2'
			 && (ctrl_key_state & CTRL_PRESSED)
			 && !(ctrl_key_state & ALT_PRESSED))
		  buf[len++] = '\0';
		else
		  { /* arrow/function keys */
		    /* FIXME: The current code generates cygwin terminal
		       sequence rather than xterm sequence. */
		    char tmp[16];
		    const char *add =
		      fhandler_console::get_nonascii_key (r[i], tmp);
		    if (add)
		      {
			strcpy (buf + len, add);
			len += strlen (add);
		      }
		  }
	      }
	  if (cp_to != cp_from)
	    {
	      static mbstate_t mbp;
	      char *mbbuf = tp.c_get ();
	      size_t nlen = NT_MAX_PATH;
	      convert_mb_str (cp_to, mbbuf, &nlen, cp_from, buf, len, &mbp);
	      ptr = mbbuf;
	      len = nlen;
	    }
	  /* Call WriteFile() line by line */
	  char *p0 = ptr;
	  char *p_cr = (char *) memchr (p0, '\r', len - (p0 - ptr));
	  char *p_lf = (char *) memchr (p0, '\n', len - (p0 - ptr));
	  while (p_cr || p_lf)
	    {
	      char *p1 =
		p_cr ?  (p_lf ? ((p_cr + 1 == p_lf)
				 ?  p_lf : min(p_cr, p_lf)) : p_cr) : p_lf;
	      *p1 = '\n';
	      n = p1 - p0 + 1;
	      if (n && WriteFile (to, p0, n, &n, NULL) && n)
		transfered = true;
	      p0 = p1 + 1;
	      p_cr = (char *) memchr (p0, '\r', len - (p0 - ptr));
	      p_lf = (char *) memchr (p0, '\n', len - (p0 - ptr));
	    }
	  n = len - (p0 - ptr);
	  if (n && WriteFile (to, p0, n, &n, NULL) && n)
	    transfered = true;
	}
    }
  else
    { /* Reaches here when both cyg->nat and nat->cyg cases with
	 pcon not activated or cyg->nat case with pcon activated. */
      DWORD bytes_in_pipe;
      while (::bytes_available (bytes_in_pipe, from) && bytes_in_pipe)
	{
	  DWORD n = MIN (bytes_in_pipe, NT_MAX_PATH);
	  ReadFile (from, buf, n, &n, NULL);
	  if (ttyp->discard_input)
	    continue;
	  char *ptr = buf;
	  if (dir == tty::to_nat)
	    {
	      char *p = buf;
	      if (ttyp->pcon_activated)
		while ((p = (char *) memchr (p, '\n', n - (p - buf))))
		  *p = '\r';
	      else
		while ((p = (char *) memchr (p, '\r', n - (p - buf))))
		  *p = '\n';
	    }
	  if (cp_to != cp_from)
	    {
	      static mbstate_t mbp;
	      char *mbbuf = tp.c_get ();
	      size_t nlen = NT_MAX_PATH;
	      convert_mb_str (cp_to, mbbuf, &nlen, cp_from, buf, n, &mbp);
	      ptr = mbbuf;
	      n = nlen;
	    }
	  if (n && WriteFile (to, ptr, n, &n, NULL) && n)
	    transfered = true;;
	}
    }

  /* Fix input_available_event which indicates availability in cyg pipe. */
  if (dir == tty::to_nat) /* all data is transfered to nat pipe,
			     so no data available in cyg pipe. */
    ResetEvent (input_available_event);
  else if (transfered) /* There is data transfered to cyg pipe. */
    SetEvent (input_available_event);
  ttyp->pty_input_state = dir;
  ttyp->discard_input = false;
}

void
fhandler_pty_slave::cleanup_before_exit ()
{
  if (myself->process_state & PID_NOTCYGWIN)
    get_ttyp ()->wait_fwd ();
}

void
fhandler_pty_slave::get_duplicated_handle_set (handle_set_t *p)
{
  DuplicateHandle (GetCurrentProcess (), get_handle_nat (),
		   GetCurrentProcess (), &p->from_master_nat,
		   0, 0, DUPLICATE_SAME_ACCESS);
  DuplicateHandle (GetCurrentProcess (), input_available_event,
		   GetCurrentProcess (), &p->input_available_event,
		   0, 0, DUPLICATE_SAME_ACCESS);
  DuplicateHandle (GetCurrentProcess (), input_mutex,
		   GetCurrentProcess (), &p->input_mutex,
		   0, 0, DUPLICATE_SAME_ACCESS);
  DuplicateHandle (GetCurrentProcess (), pipe_sw_mutex,
		   GetCurrentProcess (), &p->pipe_sw_mutex,
		   0, 0, DUPLICATE_SAME_ACCESS);
}

void
fhandler_pty_slave::close_handle_set (handle_set_t *p)
{
  CloseHandle (p->from_master_nat);
  p->from_master_nat = NULL;
  CloseHandle (p->input_available_event);
  p->input_available_event = NULL;
  CloseHandle (p->input_mutex);
  p->input_mutex = NULL;
  CloseHandle (p->pipe_sw_mutex);
  p->pipe_sw_mutex = NULL;
}

void
fhandler_pty_slave::setup_for_non_cygwin_app (bool nopcon, PWCHAR envblock,
					      bool stdin_is_ptys)
{
  if (disable_pcon || !term_has_pcon_cap (envblock))
    nopcon = true;
  WaitForSingleObject (pipe_sw_mutex, INFINITE);
  /* Setting switch_to_nat_pipe is necessary even if pseudo console
     will not be activated. */
  fhandler_base *fh = ::cygheap->fdtab[0];
  if (fh && fh->get_major () == DEV_PTYS_MAJOR)
    {
      fhandler_pty_slave *ptys = (fhandler_pty_slave *) fh;
      ptys->get_ttyp ()->switch_to_nat_pipe = true;
      if (!process_alive (ptys->get_ttyp ()->nat_pipe_owner_pid))
	ptys->get_ttyp ()->nat_pipe_owner_pid = myself->exec_dwProcessId;
    }
  bool pcon_enabled = false;
  if (!nopcon)
    {
      acquire_attach_mutex (mutex_timeout);
      pcon_enabled = setup_pseudoconsole ();
      release_attach_mutex ();
    }
  ReleaseMutex (pipe_sw_mutex);
  /* For pcon enabled case, transfer_input() is called in master::write() */
  if (!pcon_enabled && get_ttyp ()->getpgid () == myself->pgid
      && stdin_is_ptys && get_ttyp ()->pty_input_state_eq (tty::to_cyg))
    {
      WaitForSingleObject (input_mutex, mutex_timeout);
      acquire_attach_mutex (mutex_timeout);
      transfer_input (tty::to_nat, get_handle (), get_ttyp (),
		      input_available_event);
      release_attach_mutex ();
      ReleaseMutex (input_mutex);
    }
}

void
fhandler_pty_slave::cleanup_for_non_cygwin_app (handle_set_t *p, tty *ttyp,
						bool stdin_is_ptys,
						DWORD force_switch_to)
{
  ttyp->wait_fwd ();
  if (ttyp->getpgid () == myself->pgid && stdin_is_ptys
      && ttyp->pty_input_state_eq (tty::to_nat))
    {
      WaitForSingleObject (p->input_mutex, mutex_timeout);
      acquire_attach_mutex (mutex_timeout);
      transfer_input (tty::to_cyg, p->from_master_nat, ttyp,
		      p->input_available_event);
      release_attach_mutex ();
      ReleaseMutex (p->input_mutex);
    }
  WaitForSingleObject (p->pipe_sw_mutex, INFINITE);
  if (ttyp->pcon_activated)
    {
      acquire_attach_mutex (mutex_timeout);
      close_pseudoconsole (ttyp, force_switch_to);
      release_attach_mutex ();
    }
  else
    hand_over_only (ttyp, force_switch_to);
  ReleaseMutex (p->pipe_sw_mutex);
}

void
fhandler_pty_slave::setpgid_aux (pid_t pid)
{
  reset_switch_to_nat_pipe ();

  WaitForSingleObject (pipe_sw_mutex, INFINITE);
  bool was_nat_fg = get_ttyp ()->nat_fg (tc ()->pgid);
  bool nat_fg = get_ttyp ()->nat_fg (pid);
  if (!was_nat_fg && nat_fg && get_ttyp ()->switch_to_nat_pipe
      && get_ttyp ()->pty_input_state_eq (tty::to_cyg))
    {
      WaitForSingleObject (input_mutex, mutex_timeout);
      acquire_attach_mutex (mutex_timeout);
      transfer_input (tty::to_nat, get_handle (), get_ttyp (),
		      input_available_event);
      release_attach_mutex ();
      ReleaseMutex (input_mutex);
    }
  else if (was_nat_fg && !nat_fg && get_ttyp ()->switch_to_nat_pipe
	   && get_ttyp ()->pty_input_state_eq (tty::to_nat))
    {
      bool attach_restore = false;
      HANDLE from = get_handle_nat ();
      DWORD resume_pid = 0;
      if (get_ttyp ()->pcon_activated && get_ttyp ()->nat_pipe_owner_pid
	  && !get_console_process_id (get_ttyp ()->nat_pipe_owner_pid, true))
	{
	  HANDLE pcon_owner = OpenProcess (PROCESS_DUP_HANDLE, FALSE,
					   get_ttyp ()->nat_pipe_owner_pid);
	  DuplicateHandle (pcon_owner, get_ttyp ()->h_pcon_in,
			   GetCurrentProcess (), &from,
			   0, TRUE, DUPLICATE_SAME_ACCESS);
	  CloseHandle (pcon_owner);
	  DWORD target_pid = get_ttyp ()->nat_pipe_owner_pid;
	  resume_pid = attach_console_temporarily (target_pid);
	  attach_restore = true;
	}
      else
	acquire_attach_mutex (mutex_timeout);
      WaitForSingleObject (input_mutex, mutex_timeout);
      transfer_input (tty::to_cyg, from, get_ttyp (), input_available_event);
      ReleaseMutex (input_mutex);
      if (attach_restore)
	resume_from_temporarily_attach (resume_pid);
      else
	release_attach_mutex ();
    }
  ReleaseMutex (pipe_sw_mutex);
}

bool
fhandler_pty_master::need_send_ctrl_c_event ()
{
  /* If pseudo console is activated, sending CTRL_C_EVENT to non-cygwin
     apps will be done in pseudo console, therefore, sending it in
     fhandler_pty_master::write() duplicates that event for non-cygwin
     apps. So return false if pseudo console is activated. */
  return !(to_be_read_from_nat_pipe () && get_ttyp ()->pcon_activated
    && get_ttyp ()->pty_input_state == tty::to_nat);
}

void
fhandler_pty_slave::release_ownership_of_nat_pipe (tty *ttyp,
						   fhandler_termios *fh)
{
  if (fh->get_major () == DEV_PTYM_MAJOR
      && nat_pipe_owner_self (ttyp->nat_pipe_owner_pid))
    {
      fhandler_pty_master *ptym = (fhandler_pty_master *) fh;
      WaitForSingleObject (ptym->pipe_sw_mutex, INFINITE);
      if (ttyp->pcon_activated)
	/* Do not acquire/release_attach_mutex() here because
	   it has done in fhandler_termios::process_sigs(). */
	close_pseudoconsole (ttyp);
      else
	hand_over_only (ttyp);
      ReleaseMutex (ptym->pipe_sw_mutex);
    }
}

DWORD
fhandler_pty_common::attach_console_temporarily (DWORD target_pid)
{
  DWORD resume_pid = 0;
  pinfo pinfo_resume (myself->ppid);
  if (pinfo_resume)
    resume_pid = pinfo_resume->dwProcessId;
  if (!resume_pid)
    resume_pid = get_console_process_id (myself->dwProcessId, false);
  bool console_exists = fhandler_console::exists ();
  acquire_attach_mutex (mutex_timeout);
  if (!console_exists || resume_pid)
    {
      FreeConsole ();
      AttachConsole (target_pid);
      init_console_handler (::cygheap->ctty
			    && ::cygheap->ctty->is_console ());
    }
  return console_exists ? resume_pid : (DWORD) -1;
}

void
fhandler_pty_common::resume_from_temporarily_attach (DWORD resume_pid)
{
  bool console_exists = (resume_pid != (DWORD) -1);
  if (!console_exists || resume_pid)
    {
      FreeConsole ();
      if (console_exists)
	if (!resume_pid || !AttachConsole (resume_pid))
	  AttachConsole (ATTACH_PARENT_PROCESS);
      init_console_handler (::cygheap->ctty
			    && ::cygheap->ctty->is_console ());
    }
  release_attach_mutex ();
}
