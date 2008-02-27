/* exceptions.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <wingdi.h>
#include <winuser.h>
#include <imagehlp.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>
#include <syslog.h>

#include "exceptions.h"
#include "sync.h"
#include "pinfo.h"
#include "cygtls.h"
#include "sigproc.h"
#include "cygerrno.h"
#include "shared_info.h"
#include "perprocess.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"

#define CALL_HANDLER_RETRY 20

char debugger_command[2 * NT_MAX_PATH + 20];

extern "C" {
extern void sigdelayed ();
};

extern child_info_spawn *chExeced;
int NO_COPY sigExeced;

static BOOL WINAPI ctrl_c_handler (DWORD);
char windows_system_directory[1024];
static size_t windows_system_directory_length;

/* This is set to indicate that we have already exited.  */

static NO_COPY int exit_already = 0;
static muto NO_COPY mask_sync;

NO_COPY static struct
{
  unsigned int code;
  const char *name;
} status_info[] =
{
#define X(s) s, #s
  { X (STATUS_ABANDONED_WAIT_0) },
  { X (STATUS_ACCESS_VIOLATION) },
  { X (STATUS_ARRAY_BOUNDS_EXCEEDED) },
  { X (STATUS_BREAKPOINT) },
  { X (STATUS_CONTROL_C_EXIT) },
  { X (STATUS_DATATYPE_MISALIGNMENT) },
  { X (STATUS_FLOAT_DENORMAL_OPERAND) },
  { X (STATUS_FLOAT_DIVIDE_BY_ZERO) },
  { X (STATUS_FLOAT_INEXACT_RESULT) },
  { X (STATUS_FLOAT_INVALID_OPERATION) },
  { X (STATUS_FLOAT_OVERFLOW) },
  { X (STATUS_FLOAT_STACK_CHECK) },
  { X (STATUS_FLOAT_UNDERFLOW) },
  { X (STATUS_GUARD_PAGE_VIOLATION) },
  { X (STATUS_ILLEGAL_INSTRUCTION) },
  { X (STATUS_INTEGER_DIVIDE_BY_ZERO) },
  { X (STATUS_INTEGER_OVERFLOW) },
  { X (STATUS_INVALID_DISPOSITION) },
  { X (STATUS_IN_PAGE_ERROR) },
  { X (STATUS_NONCONTINUABLE_EXCEPTION) },
  { X (STATUS_NO_MEMORY) },
  { X (STATUS_PENDING) },
  { X (STATUS_PRIVILEGED_INSTRUCTION) },
  { X (STATUS_SINGLE_STEP) },
  { X (STATUS_STACK_OVERFLOW) },
  { X (STATUS_TIMEOUT) },
  { X (STATUS_USER_APC) },
  { X (STATUS_WAIT_0) },
  { 0, 0 }
#undef X
};

/* Initialization code.  */

void
init_console_handler (bool install_handler)
{
  BOOL res;

  SetConsoleCtrlHandler (ctrl_c_handler, FALSE);
  SetConsoleCtrlHandler (NULL, FALSE);
  if (install_handler)
    res = SetConsoleCtrlHandler (ctrl_c_handler, TRUE);
  else
    res = SetConsoleCtrlHandler (NULL, TRUE);
  if (!res)
    system_printf ("SetConsoleCtrlHandler failed, %E");
}

extern "C" void
error_start_init (const char *buf)
{
  if (!buf || !*buf)
    {
      debugger_command[0] = '\0';
      return;
    }

  char pgm[NT_MAX_PATH];
  if (!GetModuleFileName (NULL, pgm, NT_MAX_PATH))
    strcpy (pgm, "cygwin1.dll");
  for (char *p = strchr (pgm, '\\'); p; p = strchr (p, '\\'))
    *p = '/';

  __small_sprintf (debugger_command, "%s \"%s\"", buf, pgm);
}

static void
open_stackdumpfile ()
{
  if (myself->progname[0])
    {
      const char *p;
      /* write to progname.stackdump if possible */
      if (!myself->progname[0])
	p = "unknown";
      else if ((p = strrchr (myself->progname, '\\')))
	p++;
      else
	p = myself->progname;
      char corefile[strlen (p) + sizeof (".stackdump")];
      __small_sprintf (corefile, "%s.stackdump", p);
      HANDLE h = CreateFile (corefile, GENERIC_WRITE, 0, &sec_none_nih,
			     CREATE_ALWAYS, 0, 0);
      if (h != INVALID_HANDLE_VALUE)
	{
	  if (!myself->cygstarted)
	    system_printf ("Dumping stack trace to %s", corefile);
	  else
	    debug_printf ("Dumping stack trace to %s", corefile);
	  SetStdHandle (STD_ERROR_HANDLE, h);
	}
    }
}

/* Utilities for dumping the stack, etc.  */

static void
exception (EXCEPTION_RECORD *e,  CONTEXT *in)
{
  const char *exception_name = NULL;

  if (e)
    {
      for (int i = 0; status_info[i].name; i++)
	{
	  if (status_info[i].code == e->ExceptionCode)
	    {
	      exception_name = status_info[i].name;
	      break;
	    }
	}
    }

  if (exception_name)
    small_printf ("Exception: %s at eip=%08x\r\n", exception_name, in->Eip);
  else
    small_printf ("Signal %d at eip=%08x\r\n", e->ExceptionCode, in->Eip);
  small_printf ("eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\r\n",
		in->Eax, in->Ebx, in->Ecx, in->Edx, in->Esi, in->Edi);
  small_printf ("ebp=%08x esp=%08x program=%s, pid %u, thread %s\r\n",
		in->Ebp, in->Esp, myself->progname, myself->pid, cygthread::name ());
  small_printf ("cs=%04x ds=%04x es=%04x fs=%04x gs=%04x ss=%04x\r\n",
		in->SegCs, in->SegDs, in->SegEs, in->SegFs, in->SegGs, in->SegSs);
}

/* A class for manipulating the stack. */
class stack_info
{
  int walk ();			/* Uses the "old" method */
  char *next_offset () {return *((char **) sf.AddrFrame.Offset);}
  bool needargs;
  DWORD dummy_frame;
public:
  STACKFRAME sf;		 /* For storing the stack information */
  void init (DWORD, bool, bool); /* Called the first time that stack info is needed */

  /* Postfix ++ iterates over the stack, returning zero when nothing is left. */
  int operator ++(int) { return walk (); }
};

/* The number of parameters used in STACKFRAME */
#define NPARAMS (sizeof (thestack.sf.Params) / sizeof (thestack.sf.Params[0]))

/* This is the main stack frame info for this process. */
static NO_COPY stack_info thestack;

/* Initialize everything needed to start iterating. */
void
stack_info::init (DWORD ebp, bool wantargs, bool goodframe)
{
# define debp ((DWORD *) ebp)
  memset (&sf, 0, sizeof (sf));
  if (!goodframe)
    sf.AddrFrame.Offset = ebp;
  else
    {
      dummy_frame = ebp;
      sf.AddrFrame.Offset = (DWORD) &dummy_frame;
    }
  sf.AddrReturn.Offset = debp[1];
  sf.AddrFrame.Mode = AddrModeFlat;
  needargs = wantargs;
# undef debp
}

/* Walk the stack by looking at successive stored 'bp' frames.
   This is not foolproof. */
int
stack_info::walk ()
{
  char **ebp;
  if ((ebp = (char **) next_offset ()) == NULL)
    return 0;

  sf.AddrFrame.Offset = (DWORD) ebp;
  sf.AddrPC.Offset = sf.AddrReturn.Offset;

  if (!sf.AddrPC.Offset)
    return 0;		/* stack frames are exhausted */

  /* The return address always follows the stack pointer */
  sf.AddrReturn.Offset = (DWORD) *++ebp;

  if (needargs)
    {
      unsigned nparams = NPARAMS;

      /* The arguments follow the return address */
      sf.Params[0] = (DWORD) *++ebp;
      /* Hack for XP/2K3 WOW64.  If the first stack param points to the
	 application entry point, we can only fetch one additional
	 parameter.  Accessing anything beyond this address results in
	 a SEGV.  This is fixed in Vista/2K8 WOW64. */
      if (wincap.has_restricted_stack_args () && sf.Params[0] == 0x401000)
	nparams = 2;
      for (unsigned i = 0; i < nparams; i++)
	sf.Params[i] = (DWORD) *++ebp;
    }

  return 1;
}

static void
stackdump (DWORD ebp, int open_file, bool isexception)
{
  extern unsigned long rlim_core;
  static bool already_dumped;

  if (rlim_core == 0UL || (open_file && already_dumped))
    return;

  if (open_file)
    open_stackdumpfile ();

  already_dumped = true;

  int i;

  thestack.init (ebp, 1, !isexception);	/* Initialize from the input CONTEXT */
  small_printf ("Stack trace:\r\nFrame     Function  Args\r\n");
  for (i = 0; i < 16 && thestack++; i++)
    {
      small_printf ("%08x  %08x ", thestack.sf.AddrFrame.Offset,
		    thestack.sf.AddrPC.Offset);
      for (unsigned j = 0; j < NPARAMS; j++)
	small_printf ("%s%08x", j == 0 ? " (" : ", ", thestack.sf.Params[j]);
      small_printf (")\r\n");
    }
  small_printf ("End of stack trace%s\n",
	      i == 16 ? " (more stack frames may be present)" : "");
}

bool
_cygtls::inside_kernel (CONTEXT *cx)
{
  int res;
  MEMORY_BASIC_INFORMATION m;

  if (!isinitialized ())
    return true;

  memset (&m, 0, sizeof m);
  if (!VirtualQuery ((LPCVOID) cx->Eip, &m, sizeof m))
    sigproc_printf ("couldn't get memory info, pc %p, %E", cx->Eip);

  char *checkdir = (char *) alloca (windows_system_directory_length + 4);
  memset (checkdir, 0, sizeof (checkdir));

# define h ((HMODULE) m.AllocationBase)
  /* Apparently Windows 95 can sometimes return bogus addresses from
     GetThreadContext.  These resolve to a strange allocation base.
     These should *never* be treated as interruptible. */
  if (!h || m.State != MEM_COMMIT)
    res = true;
  else if (h == user_data->hmodule)
    res = false;
  else if (!GetModuleFileName (h, checkdir, windows_system_directory_length + 2))
    res = false;
  else
    res = strncasematch (windows_system_directory, checkdir,
			 windows_system_directory_length);
  sigproc_printf ("pc %p, h %p, inside_kernel %d", cx->Eip, h, res);
# undef h
  return res;
}

/* Temporary (?) function for external callers to get a stack dump */
extern "C" void
cygwin_stackdump ()
{
  CONTEXT c;
  c.ContextFlags = CONTEXT_FULL;
  GetThreadContext (GetCurrentThread (), &c);
  stackdump (c.Ebp, 0, 0);
}

#define TIME_TO_WAIT_FOR_DEBUGGER 10000

extern "C" int
try_to_debug (bool waitloop)
{
  WCHAR dbg_cmd[sizeof debugger_command];

  debug_printf ("debugger_command '%s'", debugger_command);
  if (*debugger_command == '\0')
    return 0;
  if (being_debugged ())
    {
      extern void break_here ();
      break_here ();
      return 0;
    }

  __small_sprintf (strchr (debugger_command, '\0'), " %u", GetCurrentProcessId ());

  LONG prio = GetThreadPriority (GetCurrentThread ());
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
  PROCESS_INFORMATION pi = {NULL, 0, 0, 0};

  STARTUPINFOW si = {0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL};
  si.lpReserved = NULL;
  si.lpDesktop = NULL;
  si.dwFlags = 0;
  si.cb = sizeof (si);

  /* FIXME: need to know handles of all running threads to
     suspend_all_threads_except (current_thread_id);
  */

  /* if any of these mutexes is owned, we will fail to start any cygwin app
     until trapped app exits */

  lock_ttys::release ();

  /* prevent recursive exception handling */
  char* rawenv = GetEnvironmentStrings () ;
  for (char* p = rawenv; *p != '\0'; p = strchr (p, '\0') + 1)
    {
      if (strncmp (p, "CYGWIN=", strlen ("CYGWIN=")) == 0)
	{
	  char* q = strstr (p, "error_start") ;
	  /* replace 'error_start=...' with '_rror_start=...' */
	  if (q)
	    {
	      *q = '_' ;
	      SetEnvironmentVariable ("CYGWIN", p + strlen ("CYGWIN=")) ;
	    }
	  break ;
	}
    }

  console_printf ("*** starting debugger for pid %u, tid %u\n",
		  cygwin_pid (GetCurrentProcessId ()), GetCurrentThreadId ());
  BOOL dbg;
  sys_mbstowcs (dbg_cmd, sizeof debugger_command, debugger_command);
  dbg = CreateProcessW (NULL,
			dbg_cmd,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
			NULL,
			NULL,
			&si,
			&pi);

  if (!dbg)
    system_printf ("Failed to start debugger, %E");
  else
    {
      if (!waitloop)
	return dbg;
      SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_IDLE);
      while (!being_debugged ())
	low_priority_sleep (0);
      Sleep (2000);
    }

  console_printf ("*** continuing pid %u from debugger call (%d)\n",
		  cygwin_pid (GetCurrentProcessId ()), dbg);

  SetThreadPriority (GetCurrentThread (), prio);
  return dbg;
}

extern "C" DWORD __stdcall RtlUnwind (void *, void *, void *, DWORD);
static void __stdcall rtl_unwind (exception_list *, PEXCEPTION_RECORD) __attribute__ ((noinline, regparm (3)));
void __stdcall
rtl_unwind (exception_list *frame, PEXCEPTION_RECORD e)
{
  __asm__ ("\n\
  pushl		%%ebx					\n\
  pushl		%%edi					\n\
  pushl		%%esi					\n\
  pushl		$0					\n\
  pushl		%1					\n\
  pushl		$1f					\n\
  pushl		%0					\n\
  call		_RtlUnwind@16				\n\
1:							\n\
  popl		%%esi					\n\
  popl		%%edi					\n\
  popl		%%ebx					\n\
": : "r" (frame), "r" (e));
}

/* Main exception handler. */

extern "C" char *__progname;
int
_cygtls::handle_exceptions (EXCEPTION_RECORD *e, exception_list *frame, CONTEXT *in, void *)
{
  static bool NO_COPY debugging;
  static int NO_COPY recursed;
  _cygtls& me = _my_tls;

  if (debugging && ++debugging < 500000)
    {
      SetThreadPriority (hMainThread, THREAD_PRIORITY_NORMAL);
      return 0;
    }

  /* If we've already exited, don't do anything here.  Returning 1
     tells Windows to keep looking for an exception handler.  */
  if (exit_already || e->ExceptionFlags)
    return 1;

  siginfo_t si = {0};
  si.si_code = SI_KERNEL;
  /* Coerce win32 value to posix value.  */
  switch (e->ExceptionCode)
    {
    case STATUS_FLOAT_DENORMAL_OPERAND:
    case STATUS_FLOAT_DIVIDE_BY_ZERO:
    case STATUS_FLOAT_INVALID_OPERATION:
    case STATUS_FLOAT_STACK_CHECK:
      si.si_signo = SIGFPE;
      si.si_code = FPE_FLTSUB;
      break;
    case STATUS_FLOAT_INEXACT_RESULT:
      si.si_signo = SIGFPE;
      si.si_code = FPE_FLTRES;
      break;
    case STATUS_FLOAT_OVERFLOW:
      si.si_signo = SIGFPE;
      si.si_code = FPE_FLTOVF;
      break;
    case STATUS_FLOAT_UNDERFLOW:
      si.si_signo = SIGFPE;
      si.si_code = FPE_FLTUND;
      break;
    case STATUS_INTEGER_DIVIDE_BY_ZERO:
      si.si_signo = SIGFPE;
      si.si_code = FPE_INTDIV;
      break;
    case STATUS_INTEGER_OVERFLOW:
      si.si_signo = SIGFPE;
      si.si_code = FPE_INTOVF;
      break;

    case STATUS_ILLEGAL_INSTRUCTION:
      si.si_signo = SIGILL;
      si.si_code = ILL_ILLOPC;
      break;

    case STATUS_PRIVILEGED_INSTRUCTION:
      si.si_signo = SIGILL;
      si.si_code = ILL_PRVOPC;
      break;

    case STATUS_NONCONTINUABLE_EXCEPTION:
      si.si_signo = SIGILL;
      si.si_code = ILL_ILLADR;
      break;

    case STATUS_TIMEOUT:
      si.si_signo = SIGALRM;
      break;

    case STATUS_GUARD_PAGE_VIOLATION:
      si.si_signo = SIGBUS;
      si.si_code = BUS_OBJERR;
      break;

    case STATUS_DATATYPE_MISALIGNMENT:
      si.si_signo = SIGBUS;
      si.si_code = BUS_ADRALN;
      break;

    case STATUS_ACCESS_VIOLATION:
      switch (mmap_is_attached_or_noreserve ((void *)e->ExceptionInformation[1],
					     1))
	{
	case MMAP_NORESERVE_COMMITED:
	  return 0;
	case MMAP_RAISE_SIGBUS:	/* MAP_NORESERVE page, commit failed, or
				   access to mmap page beyond EOF. */
	  si.si_signo = SIGBUS;
	  si.si_code = BUS_OBJERR;
	  break;
	default:
	  MEMORY_BASIC_INFORMATION m;
	  VirtualQuery ((PVOID) e->ExceptionInformation[1], &m, sizeof m);
	  si.si_signo = SIGSEGV;
	  si.si_code = m.State == MEM_FREE ? SEGV_MAPERR : SEGV_ACCERR;
	  break;
	}
      break;

    case STATUS_ARRAY_BOUNDS_EXCEEDED:
    case STATUS_IN_PAGE_ERROR:
    case STATUS_NO_MEMORY:
    case STATUS_INVALID_DISPOSITION:
    case STATUS_STACK_OVERFLOW:
      si.si_signo = SIGSEGV;
      si.si_code = SEGV_MAPERR;
      break;

    case STATUS_CONTROL_C_EXIT:
      si.si_signo = SIGINT;
      break;

    case STATUS_INVALID_HANDLE:
      /* CloseHandle will throw this exception if it is given an
	 invalid handle.  We don't care about the exception; we just
	 want CloseHandle to return an error.  This can be revisited
	 if gcc ever supports Windows style structured exception
	 handling.  */
      return 0;

    default:
      /* If we don't recognize the exception, we have to assume that
	 we are doing structured exception handling, and we let
	 something else handle it.  */
      return 1;
    }

  rtl_unwind (frame, e);

  debug_printf ("In cygwin_except_handler exc %p at %p sp %p", e->ExceptionCode, in->Eip, in->Esp);
  debug_printf ("In cygwin_except_handler sig %d at %p", si.si_signo, in->Eip);

  if (global_sigs[si.si_signo].sa_mask & SIGTOMASK (si.si_signo))
    syscall_printf ("signal %d, masked %p", si.si_signo,
		    global_sigs[si.si_signo].sa_mask);

  debug_printf ("In cygwin_except_handler calling %p",
		 global_sigs[si.si_signo].sa_handler);

  DWORD *ebp = (DWORD *) in->Esp;
  for (DWORD *bpend = (DWORD *) __builtin_frame_address (0); ebp > bpend; ebp--)
    if (*ebp == in->SegCs && ebp[-1] == in->Eip)
      {
	ebp -= 2;
	break;
      }

  if (me.fault_guarded ())
    me.return_from_fault ();

  me.copy_context (in);
  if (!cygwin_finished_initializing
      || &me == _sig_tls
      || (void *) global_sigs[si.si_signo].sa_handler == (void *) SIG_DFL
      || (void *) global_sigs[si.si_signo].sa_handler == (void *) SIG_IGN
      || (void *) global_sigs[si.si_signo].sa_handler == (void *) SIG_ERR)
    {
      /* Print the exception to the console */
      if (!myself->cygstarted)
	for (int i = 0; status_info[i].name; i++)
	  if (status_info[i].code == e->ExceptionCode)
	    {
	      system_printf ("Exception: %s", status_info[i].name);
	      break;
	    }

      /* Another exception could happen while tracing or while exiting.
	 Only do this once.  */
      if (recursed++)
	system_printf ("Error while dumping state (probably corrupted stack)");
      else
	{
	  if (try_to_debug (0))
	    {
	      debugging = true;
	      return 0;
	    }

	  open_stackdumpfile ();
	  exception (e, in);
	  stackdump ((DWORD) ebp, 0, 1);
	}

      if (e->ExceptionCode == STATUS_ACCESS_VIOLATION)
	{
	  int error_code = 0;
	  if (si.si_code == SEGV_ACCERR)	/* Address present */
	    error_code |= 1;
	  if (e->ExceptionInformation[0])	/* Write access */
	    error_code |= 2;
	  if (!me.inside_kernel (in))		/* User space */
	    error_code |= 4;
	  klog (LOG_INFO, "%s[%d]: segfault at %08x rip %08x rsp %08x error %d",
			  __progname, myself->pid,
			  e->ExceptionInformation[1], in->Eip, in->Esp,
			  ((in->Eip >= 0x61000000 && in->Eip < 0x61200000)
			   ? 0 : 4) | (e->ExceptionInformation[0] << 1));
	}

      me.signal_exit (0x80 | si.si_signo);	// Flag signal + core dump
    }

  si.si_addr = (void *) in->Eip;
  si.si_errno = si.si_pid = si.si_uid = 0;
  me.incyg++;
  sig_send (NULL, si, &me);	// Signal myself
  me.incyg--;
  e->ExceptionFlags = 0;
  /* The OS adds an exception list frame to the stack.  It expects to be
     able to remove this entry after the exception handler returned.
     However, when unwinding to our frame, our frame becomes the uppermost
     frame on the stack (%fs:0 points to frame).  This way, our frame
     is removed from the exception stack and just disappears.  So, we can't
     just return here or things will be screwed up by the helpful function
     in (presumably) ntdll.dll.

     So, instead, we will do the equivalent of a longjmp here and return
     to the caller without visiting any of the helpful code installed prior
     to this function.  This should work ok, since a longjmp() out of here has
     to work if linux signal semantics are to be maintained. */

  SetThreadContext (GetCurrentThread (), in);
  return 0; /* Never actually returns.  This is just to keep gcc happy. */
}

/* Utilities to call a user supplied exception handler.  */

#define SIG_NONMASKABLE	(SIGTOMASK (SIGKILL) | SIGTOMASK (SIGSTOP))

/* Non-raceable sigsuspend
 * Note: This implementation is based on the Single UNIX Specification
 * man page.  This indicates that sigsuspend always returns -1 and that
 * attempts to block unblockable signals will be silently ignored.
 * This is counter to what appears to be documented in some UNIX
 * man pages, e.g. Linux.
 */
int __stdcall
handle_sigsuspend (sigset_t tempmask)
{
  if (&_my_tls != _main_tls)
    {
      cancelable_wait (signal_arrived, INFINITE, cw_cancel_self);
      return -1;
    }

  sigset_t oldmask = _my_tls.sigmask;	// Remember for restoration

  set_signal_mask (tempmask, _my_tls.sigmask);
  sigproc_printf ("oldmask %p, newmask %p", oldmask, tempmask);

  pthread_testcancel ();
  cancelable_wait (signal_arrived, INFINITE);

  set_sig_errno (EINTR);	// Per POSIX

  /* A signal dispatch function will have been added to our stack and will
     be hit eventually.  Set the old mask to be restored when the signal
     handler returns and indicate its presence by modifying deltamask. */

  _my_tls.deltamask |= SIG_NONMASKABLE;
  _my_tls.oldmask = oldmask;	// Will be restored by signal handler
  return -1;
}

extern DWORD exec_exit;		// Possible exit value for exec

extern "C" {
static void
sig_handle_tty_stop (int sig)
{
  _my_tls.incyg = 1;
  /* Silently ignore attempts to suspend if there is no accommodating
     cygwin parent to deal with this behavior. */
  if (!myself->cygstarted)
    {
      myself->process_state &= ~PID_STOPPED;
      return;
    }

  myself->stopsig = sig;
  myself->alert_parent (sig);
  sigproc_printf ("process %d stopped by signal %d", myself->pid, sig);
  HANDLE w4[2];
  w4[0] = sigCONT;
  w4[1] = signal_arrived;
  switch (WaitForMultipleObjects (2, w4, TRUE, INFINITE))
    {
    case WAIT_OBJECT_0:
    case WAIT_OBJECT_0 + 1:
      reset_signal_arrived ();
      myself->alert_parent (SIGCONT);
      break;
    default:
      api_fatal ("WaitSingleObject failed, %E");
      break;
    }
  _my_tls.incyg = 0;
}
}

bool
_cygtls::interrupt_now (CONTEXT *cx, int sig, void *handler,
			struct sigaction& siga)
{
  bool interrupted;

  if (incyg || spinning || locked () || inside_kernel (cx))
    interrupted = false;
  else
    {
      push ((__stack_t) cx->Eip);
      interrupt_setup (sig, handler, siga);
      cx->Eip = pop ();
      SetThreadContext (*this, cx); /* Restart the thread in a new location */
      interrupted = true;
    }
  return interrupted;
}

void __stdcall
_cygtls::interrupt_setup (int sig, void *handler, struct sigaction& siga)
{
  push ((__stack_t) sigdelayed);
  deltamask = siga.sa_mask & ~SIG_NONMASKABLE;
  sa_flags = siga.sa_flags;
  func = (void (*) (int)) handler;
  if (siga.sa_flags & SA_RESETHAND)
    siga.sa_handler = SIG_DFL;
  saved_errno = -1;		// Flag: no errno to save
  if (handler == sig_handle_tty_stop)
    {
      myself->stopsig = 0;
      myself->process_state |= PID_STOPPED;
    }

  this->sig = sig;			// Should always be last thing set to avoid a race

  if (!event)
    threadkill = false;
  else
    {
      HANDLE h = event;
      event = NULL;
      SetEvent (h);
    }

  /* Clear any waiting threads prior to dispatching to handler function */
  int res = SetEvent (signal_arrived);	// For an EINTR case
  proc_subproc (PROC_CLEARWAIT, 1);
  sigproc_printf ("armed signal_arrived %p, sig %d, res %d", signal_arrived,
		  sig, res);
}

extern "C" void __stdcall
set_sig_errno (int e)
{
  *_my_tls.errno_addr = e;
  _my_tls.saved_errno = e;
  // sigproc_printf ("errno %d", e);
}

static int setup_handler (int, void *, struct sigaction&, _cygtls *tls)
  __attribute__((regparm(3)));
static int
setup_handler (int sig, void *handler, struct sigaction& siga, _cygtls *tls)
{
  CONTEXT cx;
  bool interrupted = false;

  if (tls->sig)
    {
      sigproc_printf ("trying to send sig %d but signal %d already armed",
		      sig, tls->sig);
      goto out;
    }

  for (int i = 0; i < CALL_HANDLER_RETRY; i++)
    {
      tls->lock ();
      if (tls->incyg)
	{
	  sigproc_printf ("controlled interrupt. stackptr %p, stack %p, stackptr[-1] %p",
			  tls->stackptr, tls->stack, tls->stackptr[-1]);
	  tls->interrupt_setup (sig, handler, siga);
	  interrupted = true;
	  tls->unlock ();
	  break;
	}

      tls->unlock ();
      DWORD res;
      HANDLE hth = (HANDLE) *tls;

      /* Suspend the thread which will receive the signal.
	 For Windows 95, we also have to ensure that the addresses returned by
	 GetThreadContext are valid.
	 If one of these conditions is not true we loop for a fixed number of times
	 since we don't want to stall the signal handler.  FIXME: Will this result in
	 noticeable delays?
	 If the thread is already suspended (which can occur when a program has called
	 SuspendThread on itself) then just queue the signal. */

#ifndef DEBUGGING
      sigproc_printf ("suspending mainthread");
#else
      cx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
      if (!GetThreadContext (hth, &cx))
	memset (&cx, 0, sizeof cx);
      sigproc_printf ("suspending mainthread PC %p", cx.Eip);
#endif
      res = SuspendThread (hth);
      /* Just set pending if thread is already suspended */
      if (res)
	{
	  ResumeThread (hth);
	  break;
	}
      cx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
      if (!GetThreadContext (hth, &cx))
	system_printf ("couldn't get context of main thread, %E");
      else
	interrupted = tls->interrupt_now (&cx, sig, handler, siga);

      res = ResumeThread (hth);
      if (interrupted)
	break;

      sigproc_printf ("couldn't interrupt.  trying again.");
      low_priority_sleep (0);
    }

out:
  sigproc_printf ("signal %d %sdelivered", sig, interrupted ? "" : "not ");
  return interrupted;
}

static inline bool
has_visible_window_station ()
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

/* Keyboard interrupt handler.  */
static BOOL WINAPI
ctrl_c_handler (DWORD type)
{
  static bool saw_close;
  lock_process now;

  if (!cygwin_finished_initializing)
    {
      if (myself->cygstarted)	/* Was this process created by a cygwin process? */
	return TRUE;		/* Yes.  Let the parent eventually handle CTRL-C issues. */
      debug_printf ("exiting with status %p", STATUS_CONTROL_C_EXIT);
      ExitProcess (STATUS_CONTROL_C_EXIT);
    }

  _my_tls.remove (INFINITE);

#if 0
  if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT)
    proc_subproc (PROC_KILLFORKED, 0);
#endif

  /* Return FALSE to prevent an "End task" dialog box from appearing
     for each Cygwin process window that's open when the computer
     is shut down or console window is closed. */

  if (type == CTRL_SHUTDOWN_EVENT)
    {
#if 0
      /* Don't send a signal.  Only NT service applications and their child
	 processes will receive this event and the services typically already
	 handle the shutdown action when getting the SERVICE_CONTROL_SHUTDOWN
	 control message. */
      sig_send (NULL, SIGTERM);
#endif
      return FALSE;
    }

  if (myself->ctty != -1)
    {
      if (type == CTRL_CLOSE_EVENT)
	{
	  sig_send (NULL, SIGHUP);
	  saw_close = true;
	  return FALSE;
	}
      if (!saw_close && type == CTRL_LOGOFF_EVENT)
	{
	  /* The CTRL_LOGOFF_EVENT is sent when *any* user logs off.
	     The below code sends a SIGHUP only if it is not performing the
	     default activity for SIGHUP.  Note that it is possible for two
	     SIGHUP signals to arrive if a process group leader is exiting
	     too.  Getting this 100% right is saved for a future cygwin mailing
	     list goad.  */
	  if (global_sigs[SIGHUP].sa_handler != SIG_DFL)
	    {
	      sig_send (myself_nowait, SIGHUP);
	      return TRUE;
	    }
	  return FALSE;
	}
    }

  if (chExeced)
    {
      chExeced->set_saw_ctrl_c ();
      return TRUE;
    }

  /* We're only the process group leader when we have a valid pinfo structure.
     If we don't have one, then the parent "stub" will handle the signal. */
  if (!pinfo (cygwin_pid (GetCurrentProcessId ())))
    return TRUE;

  tty_min *t = cygwin_shared->tty.get_tty (myself->ctty);
  /* Ignore this if we're not the process group leader since it should be handled
     *by* the process group leader. */
  if (myself->ctty != -1 && t->getpgid () == myself->pid &&
       (GetTickCount () - t->last_ctrl_c) >= MIN_CTRL_C_SLOP)
    /* Otherwise we just send a SIGINT to the process group and return TRUE (to indicate
       that we have handled the signal).  At this point, type should be
       a CTRL_C_EVENT or CTRL_BREAK_EVENT. */
    {
      int sig = SIGINT;
      /* If intr and quit are both mapped to ^C, send SIGQUIT on ^BREAK */
      if (type == CTRL_BREAK_EVENT
	  && t->ti.c_cc[VINTR] == 3 && t->ti.c_cc[VQUIT] == 3)
	sig = SIGQUIT;
      t->last_ctrl_c = GetTickCount ();
      killsys (-myself->pid, sig);
      t->last_ctrl_c = GetTickCount ();
      return TRUE;
    }

  return TRUE;
}

/* Function used by low level sig wrappers. */
extern "C" void __stdcall
set_process_mask (sigset_t newmask)
{
  set_signal_mask (newmask, _my_tls.sigmask);
}

extern "C" int
sighold (int sig)
{
  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("signal %d out of range", sig);
      return -1;
    }
  mask_sync.acquire (INFINITE);
  sigset_t mask = _my_tls.sigmask;
  sigaddset (&mask, sig);
  set_signal_mask (mask, _my_tls.sigmask);
  mask_sync.release ();
  return 0;
}

extern "C" int
sigrelse (int sig)
{
  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("signal %d out of range", sig);
      return -1;
    }
  mask_sync.acquire (INFINITE);
  sigset_t mask = _my_tls.sigmask;
  sigdelset (&mask, sig);
  set_signal_mask (mask, _my_tls.sigmask);
  mask_sync.release ();
  return 0;
}

extern "C" _sig_func_ptr
sigset (int sig, _sig_func_ptr func)
{
  sig_dispatch_pending ();
  _sig_func_ptr prev;

  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigset (%d, %p)", sig, func);
      return (_sig_func_ptr) SIG_ERR;
    }

  mask_sync.acquire (INFINITE);
  sigset_t mask = _my_tls.sigmask;
  /* If sig was in the signal mask return SIG_HOLD, otherwise return the
     previous disposition. */
  if (sigismember (&mask, sig))
    prev = SIG_HOLD;
  else
    prev = global_sigs[sig].sa_handler;
  /* If func is SIG_HOLD, add sig to the signal mask, otherwise set the
     disposition to func and remove sig from the signal mask. */
  if (func == SIG_HOLD)
    sigaddset (&mask, sig);
  else
    {
      /* No error checking.  The test which could return SIG_ERR has already
	 been made above. */
      signal (sig, func);
      sigdelset (&mask, sig);
    }
  set_signal_mask (mask, _my_tls.sigmask);
  mask_sync.release ();
  return prev;
}

extern "C" int
sigignore (int sig)
{
  return sigset (sig, SIG_IGN) == SIG_ERR ? -1 : 0;
}

/* Update the signal mask for this process and return the old mask.
   Called from sigdelayed */
extern "C" sigset_t
set_process_mask_delta ()
{
  mask_sync.acquire (INFINITE);
  sigset_t newmask, oldmask;

  if (_my_tls.deltamask & SIG_NONMASKABLE)
    oldmask = _my_tls.oldmask; /* from handle_sigsuspend */
  else
    oldmask = _my_tls.sigmask;
  newmask = (oldmask | _my_tls.deltamask) & ~SIG_NONMASKABLE;
  sigproc_printf ("oldmask %p, newmask %p, deltamask %p", oldmask, newmask,
		  _my_tls.deltamask);
  _my_tls.sigmask = newmask;
  mask_sync.release ();
  return oldmask;
}

/* Set the signal mask for this process.
   Note that some signals are unmaskable, as in UNIX.  */
extern "C" void __stdcall
set_signal_mask (sigset_t newmask, sigset_t& oldmask)
{
#ifdef CGF
  if (&_my_tls == _sig_tls)
    small_printf ("********* waiting in signal thread\n");
#endif
  mask_sync.acquire (INFINITE);
  newmask &= ~SIG_NONMASKABLE;
  sigset_t mask_bits = oldmask & ~newmask;
  sigproc_printf ("oldmask %p, newmask %p, mask_bits %p", oldmask, newmask,
		  mask_bits);
  oldmask = newmask;
  if (mask_bits)
    sig_dispatch_pending (true);
  else
    sigproc_printf ("not calling sig_dispatch_pending");
  mask_sync.release ();
}

int __stdcall
sigpacket::process ()
{
  DWORD continue_now;
  struct sigaction dummy = global_sigs[SIGSTOP];

  if (si.si_signo != SIGCONT)
    continue_now = false;
  else
    {
      continue_now = myself->process_state & PID_STOPPED;
      myself->stopsig = 0;
      myself->process_state &= ~PID_STOPPED;
      /* Clear pending stop signals */
      sig_clear (SIGSTOP);
      sig_clear (SIGTSTP);
      sig_clear (SIGTTIN);
      sig_clear (SIGTTOU);
    }

  int rc = 1;

  sigproc_printf ("signal %d processing", si.si_signo);
  struct sigaction& thissig = global_sigs[si.si_signo];

  myself->rusage_self.ru_nsignals++;

  bool masked;
  void *handler;
  if (!hExeced || (void *) thissig.sa_handler == (void *) SIG_IGN)
    handler = (void *) thissig.sa_handler;
  else if (tls)
    return 1;
  else
    handler = NULL;

  if (si.si_signo == SIGKILL)
    goto exit_sig;
  if (si.si_signo == SIGSTOP)
    {
      sig_clear (SIGCONT);
      if (!tls)
	tls = _main_tls;
      goto stop;
    }

  bool insigwait_mask;
  if ((masked = ISSTATE (myself, PID_STOPPED)))
    insigwait_mask = false;
  else if (!tls)
    insigwait_mask = !handler && (tls = _cygtls::find_tls (si.si_signo));
  else
    insigwait_mask = sigismember (&tls->sigwait_mask, si.si_signo);

  if (insigwait_mask)
    goto thread_specific;

  if (masked)
    /* nothing to do */;
  else if (sigismember (mask, si.si_signo))
    masked = true;
  else if (tls)
    masked  = sigismember (&tls->sigmask, si.si_signo);

  if (!tls)
    tls = _main_tls;

  if (masked)
    {
      sigproc_printf ("signal %d blocked", si.si_signo);
      rc = -1;
      goto done;
    }

  /* Clear pending SIGCONT on stop signals */
  if (si.si_signo == SIGTSTP || si.si_signo == SIGTTIN || si.si_signo == SIGTTOU)
    sig_clear (SIGCONT);

#ifdef CGF
  if (being_debugged ())
    {
      char sigmsg[sizeof (_CYGWIN_SIGNAL_STRING " 0xffffffff")];
      __small_sprintf (sigmsg, _CYGWIN_SIGNAL_STRING " %p", si.si_signo);
      OutputDebugString (sigmsg);
    }
#endif

  if (handler == (void *) SIG_DFL)
    {
      if (insigwait_mask)
	goto thread_specific;
      if (si.si_signo == SIGCHLD || si.si_signo == SIGIO || si.si_signo == SIGCONT || si.si_signo == SIGWINCH
	  || si.si_signo == SIGURG)
	{
	  sigproc_printf ("default signal %d ignored", si.si_signo);
	  if (continue_now)
	    SetEvent (signal_arrived);
	  goto done;
	}

      if (si.si_signo == SIGTSTP || si.si_signo == SIGTTIN || si.si_signo == SIGTTOU)
	goto stop;

      goto exit_sig;
    }

  if (handler == (void *) SIG_IGN)
    {
      sigproc_printf ("signal %d ignored", si.si_signo);
      goto done;
    }

  if (handler == (void *) SIG_ERR)
    goto exit_sig;

  tls->set_siginfo (this);
  goto dosig;

stop:
  /* Eat multiple attempts to STOP */
  if (ISSTATE (myself, PID_STOPPED))
    goto done;
  handler = (void *) sig_handle_tty_stop;
  thissig = dummy;

dosig:
  /* Dispatch to the appropriate function. */
  sigproc_printf ("signal %d, about to call %p", si.si_signo, handler);
  rc = setup_handler (si.si_signo, handler, thissig, tls);

done:
  if (continue_now)
    SetEvent (sigCONT);
  sigproc_printf ("returning %d", rc);
  return rc;

thread_specific:
  tls->sig = si.si_signo;
  tls->set_siginfo (this);
  sigproc_printf ("releasing sigwait for thread");
  SetEvent (tls->event);
  goto done;

exit_sig:
  if (si.si_signo == SIGQUIT || si.si_signo == SIGABRT)
    {
      CONTEXT c;
      c.ContextFlags = CONTEXT_FULL;
      GetThreadContext (hMainThread, &c);
      tls->copy_context (&c);
      si.si_signo |= 0x80;
    }
  sigproc_printf ("signal %d, about to call do_exit", si.si_signo);
  tls->signal_exit (si.si_signo);	/* never returns */
}

/* Cover function to `do_exit' to handle exiting even in presence of more
   exceptions.  We used to call exit, but a SIGSEGV shouldn't cause atexit
   routines to run.  */
void
_cygtls::signal_exit (int rc)
{
  if (hExeced)
    {
      sigproc_printf ("terminating captive process");
      TerminateProcess (hExeced, sigExeced = rc);
    }

  signal_debugger (rc & 0x7f);
  if ((rc & 0x80) && !try_to_debug ())
    stackdump (thread_context.ebp, 1, 1);

  lock_process until_exit (true);
  if (hExeced || exit_state > ES_PROCESS_LOCKED)
    myself.exit (rc);

  /* Starve other threads in a vain attempt to stop them from doing something
     stupid. */
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);

  sigproc_printf ("about to call do_exit (%x)", rc);
  SetEvent (signal_arrived);
  do_exit (rc);
}

void
events_init ()
{
  mask_sync.init ("mask_sync");
  windows_system_directory[0] = '\0';
  GetSystemDirectory (windows_system_directory, sizeof (windows_system_directory) - 2);
  char *end = strchr (windows_system_directory, '\0');
  if (end == windows_system_directory)
    api_fatal ("can't find windows system directory");
  if (end[-1] != '\\')
    {
      *end++ = '\\';
      *end = '\0';
    }
  windows_system_directory_length = end - windows_system_directory;
  debug_printf ("windows_system_directory '%s', windows_system_directory_length %d",
		windows_system_directory, windows_system_directory_length);
}

void
events_terminate ()
{
  exit_already = 1;
}

int
_cygtls::call_signal_handler ()
{
  int this_sa_flags = 0;
  /* Call signal handler.  */
  while (sig)
    {
      lock ();
      this_sa_flags = sa_flags;
      int thissig = sig;

      pop ();
      reset_signal_arrived ();
      sigset_t this_oldmask = set_process_mask_delta ();
      int this_errno = saved_errno;
      sig = 0;
      unlock ();	// make sure synchronized
      incyg = 0;
      if (!(this_sa_flags & SA_SIGINFO))
	{
	  void (*sigfunc) (int) = func;
	  sigfunc (thissig);
	}
      else
	{
	  siginfo_t thissi = infodata;
	  void (*sigact) (int, siginfo_t *, void *) = (void (*) (int, siginfo_t *, void *)) func;
	  /* no ucontext_t information provided yet */
	  sigact (thissig, &thissi, NULL);
	}
      incyg = 1;
      set_signal_mask (this_oldmask, _my_tls.sigmask);
      if (this_errno >= 0)
	set_errno (this_errno);
    }

  return this_sa_flags & SA_RESTART;
}

extern "C" void __stdcall
reset_signal_arrived ()
{
  // NEEDED? WaitForSingleObject (signal_arrived, 10);
  ResetEvent (signal_arrived);
  sigproc_printf ("reset signal_arrived");
  if (_my_tls.stackptr > _my_tls.stack)
    debug_printf ("stackptr[-1] %p", _my_tls.stackptr[-1]);
}

void
_cygtls::copy_context (CONTEXT *c)
{
  memcpy (&thread_context, c, (&thread_context._internal - (unsigned char *) &thread_context));
}

void
_cygtls::signal_debugger (int sig)
{
  if (isinitialized () && being_debugged ())
    {
      char sigmsg[2 * sizeof (_CYGWIN_SIGNAL_STRING " ffffffff ffffffff")];
      __small_sprintf (sigmsg, _CYGWIN_SIGNAL_STRING " %d %p %p", sig, thread_id, &thread_context);
      OutputDebugString (sigmsg);
    }
}
