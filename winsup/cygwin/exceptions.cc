/* exceptions.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <imagehlp.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>

#include "exceptions.h"
#include "sync.h"
#include "pinfo.h"
#include "cygtls.h"
#include "sigproc.h"
#include "cygerrno.h"
#define NEED_VFORK
#include "perthread.h"
#include "shared_info.h"
#include "perprocess.h"
#include "security.h"
#include "cygthread.h"

#define CALL_HANDLER_RETRY 20

char debugger_command[2 * CYG_MAX_PATH + 20];

extern "C" {
static int handle_exceptions (EXCEPTION_RECORD *, void *, CONTEXT *, void *);
extern void sigdelayed ();
};

extern DWORD sigtid;

extern HANDLE hExeced;
extern DWORD dwExeced;

static BOOL WINAPI ctrl_c_handler (DWORD);
static void signal_exit (int) __attribute__ ((noreturn));
static char windows_system_directory[1024];
static size_t windows_system_directory_length;

/* This is set to indicate that we have already exited.  */

static NO_COPY int exit_already = 0;
static NO_COPY muto *mask_sync = NULL;

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

// Set up the exception handler for the current thread.  The PowerPC & Mips
// use compiler generated tables to set up the exception handlers for each
// region of code, and the kernel walks the call list until it finds a region
// of code that handles exceptions.  The x86 on the other hand uses segment
// register fs, offset 0 to point to the current exception handler.

extern exception_list *_except_list asm ("%fs:0");

void
init_exception_handler (exception_list *el, exception_handler *eh)
{
  el->handler = eh;
  el->prev = _except_list;
  _except_list = el;
}

extern "C" void
init_exceptions (exception_list *el)
{
  init_exception_handler (el, handle_exceptions);
}

void
init_console_handler ()
{
  (void) SetConsoleCtrlHandler (ctrl_c_handler, FALSE);
  if (!SetConsoleCtrlHandler (ctrl_c_handler, TRUE))
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

  char pgm[CYG_MAX_PATH + 1];
  if (!GetModuleFileName (NULL, pgm, CYG_MAX_PATH))
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
	  if (!myself->ppid_handle)
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

#ifdef __i386__
#define HAVE_STATUS
  if (exception_name)
    small_printf ("Exception: %s at eip=%08x\r\n", exception_name, in->Eip);
  else
    small_printf ("Exception %d at eip=%08x\r\n", e->ExceptionCode, in->Eip);
  small_printf ("eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\r\n",
	      in->Eax, in->Ebx, in->Ecx, in->Edx, in->Esi, in->Edi);
  small_printf ("ebp=%08x esp=%08x program=%s\r\n",
	      in->Ebp, in->Esp, myself->progname);
  small_printf ("cs=%04x ds=%04x es=%04x fs=%04x gs=%04x ss=%04x\r\n",
	      in->SegCs, in->SegDs, in->SegEs, in->SegFs, in->SegGs, in->SegSs);
#endif

#ifndef HAVE_STATUS
  system_printf ("Had an exception");
#endif
}

#ifdef __i386__
/* Print a stack backtrace. */

#define HAVE_STACK_TRACE

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
    /* The arguments follow the return address */
    for (unsigned i = 0; i < NPARAMS; i++)
      sf.Params[i] = (DWORD) *++ebp;

  return 1;
}

static void
stackdump (DWORD ebp, int open_file, bool isexception)
{
  extern unsigned long rlim_core;

  if (rlim_core == 0UL)
    return;

  if (open_file)
    open_stackdumpfile ();

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
  small_printf ("End of stack trace%s",
	      i == 16 ? " (more stack frames may be present)" : "");
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
  debug_printf ("debugger_command '%s'", debugger_command);
  if (*debugger_command == '\0' || being_debugged ())
    return 0;

  __small_sprintf (strchr (debugger_command, '\0'), " %u", GetCurrentProcessId ());

  LONG prio = GetThreadPriority (GetCurrentThread ());
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
  PROCESS_INFORMATION pi = {NULL, 0, 0, 0};

  STARTUPINFO si = {0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL};
  si.lpReserved = NULL;
  si.lpDesktop = NULL;
  si.dwFlags = 0;
  si.cb = sizeof (si);

  /* FIXME: need to know handles of all running threads to
     suspend_all_threads_except (current_thread_id);
  */

  /* if any of these mutexes is owned, we will fail to start any cygwin app
     until trapped app exits */

  ReleaseMutex (title_mutex);

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

  small_printf ("*** starting debugger for pid %u\n",
		cygwin_pid (GetCurrentProcessId ()));
  BOOL dbg;
  dbg = CreateProcess (NULL,
		       debugger_command,
		       NULL,
		       NULL,
		       FALSE,
		       CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
		       NULL,
		       NULL,
		       &si,
		       &pi);

  if (!dbg)
    system_printf ("Failed to start debugger: %E");
  else
    {
      if (!waitloop)
	return 1;
      SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_IDLE);
      while (!being_debugged ())
	low_priority_sleep (0);
      small_printf ("*** continuing pid %u from debugger call\n",
		    cygwin_pid (GetCurrentProcessId ()));
    }

  SetThreadPriority (GetCurrentThread (), prio);
  return 0;
}

/* Main exception handler. */

extern "C" DWORD __stdcall RtlUnwind (void *, void *, void *, DWORD);
static int
handle_exceptions (EXCEPTION_RECORD *e0, void *frame, CONTEXT *in0, void *)
{
  static bool NO_COPY debugging = false;
  static int NO_COPY recursed = 0;

  if (debugging && ++debugging < 500000)
    {
      SetThreadPriority (hMainThread, THREAD_PRIORITY_NORMAL);
      return 0;
    }

  /* If we've already exited, don't do anything here.  Returning 1
     tells Windows to keep looking for an exception handler.  */
  if (exit_already)
    return 1;

  EXCEPTION_RECORD e = *e0;
  CONTEXT in = *in0;

  extern DWORD ret_here[];
  RtlUnwind (frame, ret_here, e0, 0);
  __asm__ volatile (".equ _ret_here,.");

  int sig;
  /* Coerce win32 value to posix value.  */
  switch (e.ExceptionCode)
    {
    case STATUS_FLOAT_DENORMAL_OPERAND:
    case STATUS_FLOAT_DIVIDE_BY_ZERO:
    case STATUS_FLOAT_INEXACT_RESULT:
    case STATUS_FLOAT_INVALID_OPERATION:
    case STATUS_FLOAT_OVERFLOW:
    case STATUS_FLOAT_STACK_CHECK:
    case STATUS_FLOAT_UNDERFLOW:
    case STATUS_INTEGER_DIVIDE_BY_ZERO:
    case STATUS_INTEGER_OVERFLOW:
      sig = SIGFPE;
      break;

    case STATUS_ILLEGAL_INSTRUCTION:
    case STATUS_PRIVILEGED_INSTRUCTION:
    case STATUS_NONCONTINUABLE_EXCEPTION:
      sig = SIGILL;
      break;

    case STATUS_TIMEOUT:
      sig = SIGALRM;
      break;

    case STATUS_ACCESS_VIOLATION:
    case STATUS_DATATYPE_MISALIGNMENT:
    case STATUS_ARRAY_BOUNDS_EXCEEDED:
    case STATUS_GUARD_PAGE_VIOLATION:
    case STATUS_IN_PAGE_ERROR:
    case STATUS_NO_MEMORY:
    case STATUS_INVALID_DISPOSITION:
    case STATUS_STACK_OVERFLOW:
      sig = SIGSEGV;
      break;

    case STATUS_CONTROL_C_EXIT:
      sig = SIGINT;
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

  debug_printf ("In cygwin_except_handler exc %p at %p sp %p", e.ExceptionCode, in.Eip, in.Esp);
  debug_printf ("In cygwin_except_handler sig = %d at %p", sig, in.Eip);

  if (global_sigs[sig].sa_mask & SIGTOMASK (sig))
    syscall_printf ("signal %d, masked %p", sig, global_sigs[sig].sa_mask);

  debug_printf ("In cygwin_except_handler calling %p",
		 global_sigs[sig].sa_handler);

  DWORD *ebp = (DWORD *)in.Esp;
  for (DWORD *bpend = (DWORD *) __builtin_frame_address (0); ebp > bpend; ebp--)
    if (*ebp == in.SegCs && ebp[-1] == in.Eip)
      {
	ebp -= 2;
	break;
      }

  if (!myself->progname[0]
      || GetCurrentThreadId () == sigtid
      || (void *) global_sigs[sig].sa_handler == (void *) SIG_DFL
      || (void *) global_sigs[sig].sa_handler == (void *) SIG_IGN
      || (void *) global_sigs[sig].sa_handler == (void *) SIG_ERR)
    {
      /* Print the exception to the console */
      if (1)
	{
	  for (int i = 0; status_info[i].name; i++)
	    {
	      if (status_info[i].code == e.ExceptionCode)
		{
		  if (!myself->ppid_handle)
		    system_printf ("Exception: %s", status_info[i].name);
		  break;
		}
	    }
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
	  exception (&e, &in);
	  stackdump ((DWORD) ebp, 0, 1);
	}

      signal_exit (0x80 | sig);	// Flag signal + core dump
    }

  _my_tls.push ((__stack_t) ebp, true);
  sig_send (NULL, sig, &_my_tls);	// Signal myself
  return 1;
}
#endif /* __i386__ */

#ifndef HAVE_STACK_TRACE
void
stack (void)
{
  system_printf ("Stack trace not yet supported on this machine.");
}
#endif

/* Utilities to call a user supplied exception handler.  */

#define SIG_NONMASKABLE	(SIGTOMASK (SIGKILL) | SIGTOMASK (SIGSTOP))

#ifdef __i386__
#define HAVE_CALL_HANDLER

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
  sig_dispatch_pending ();
  sigset_t oldmask = myself->getsigmask ();	// Remember for restoration

  set_signal_mask (tempmask &= ~SIG_NONMASKABLE, oldmask);// Let signals we're
				//  interested in through.
  sigproc_printf ("oldmask %p, newmask %p", oldmask, tempmask);

  pthread_testcancel ();
  pthread::cancelable_wait (signal_arrived, INFINITE);

  set_sig_errno (EINTR);	// Per POSIX

  /* A signal dispatch function will have been added to our stack and will
     be hit eventually.  Set the old mask to be restored when the signal
     handler returns. */

  _my_tls.oldmask = oldmask;	// Will be restored by signal handler
  return -1;
}

extern DWORD exec_exit;		// Possible exit value for exec

extern "C" {
static void
sig_handle_tty_stop (int sig)
{
  /* Silently ignore attempts to suspend if there is no accomodating
     cygwin parent to deal with this behavior. */
  if (!myself->ppid_handle)
    {
      myself->process_state &= ~PID_STOPPED;
      return;
    }

  myself->stopsig = sig;
  /* See if we have a living parent.  If so, send it a special signal.
     It will figure out exactly which pid has stopped by scanning
     its list of subprocesses.  */
  if (my_parent_is_alive ())
    {
      pinfo parent (myself->ppid);
      if (NOTSTATE (parent, PID_NOCLDSTOP))
	sig_send (parent, SIGCHLD);
    }
  sigproc_printf ("process %d stopped by signal %d, myself->ppid_handle %p",
		  myself->pid, sig, myself->ppid_handle);
  if (WaitForSingleObject (sigCONT, INFINITE) != WAIT_OBJECT_0)
    api_fatal ("WaitSingleObject failed, %E");
  return;
}
}

bool
interruptible (DWORD pc)
{
  int res;
  MEMORY_BASIC_INFORMATION m;

  memset (&m, 0, sizeof m);
  if (!VirtualQuery ((LPCVOID) pc, &m, sizeof m))
    sigproc_printf ("couldn't get memory info, pc %p, %E", pc);

  char *checkdir = (char *) alloca (windows_system_directory_length + 4);
  memset (checkdir, 0, sizeof (checkdir));

# define h ((HMODULE) m.AllocationBase)
  /* Apparently Windows 95 can sometimes return bogus addresses from
     GetThreadContext.  These resolve to a strange allocation base.
     These should *never* be treated as interruptible. */
  if (!h || m.State != MEM_COMMIT)
    res = false;
  else if (h == user_data->hmodule)
    res = true;
  else if (!GetModuleFileName (h, checkdir, windows_system_directory_length + 2))
    res = false;
  else
    res = !strncasematch (windows_system_directory, checkdir,
			  windows_system_directory_length);
  sigproc_printf ("pc %p, h %p, interruptible %d", pc, h, res);
# undef h
  return res;
}
void __stdcall
_threadinfo::interrupt_setup (int sig, void *handler,
			      struct sigaction& siga, __stack_t retaddr)
{
  __stack_t *retaddr_in_tls = stackptr - 1;
  push ((__stack_t) sigdelayed);
  oldmask = myself->getsigmask ();
  newmask = oldmask | siga.sa_mask | SIGTOMASK (sig);
  sa_flags = siga.sa_flags;
  func = (void (*) (int)) handler;
  saved_errno = -1;		// Flag: no errno to save
  if (handler == sig_handle_tty_stop)
    {
      myself->stopsig = 0;
      myself->process_state |= PID_STOPPED;
    }
  this->sig = sig;			// Should ALWAYS be second to last setting set to avoid a race
  *retaddr_in_tls = retaddr;
  /* Clear any waiting threads prior to dispatching to handler function */
  int res = SetEvent (signal_arrived);	// For an EINTR case
  proc_subproc (PROC_CLEARWAIT, 1);
  sigproc_printf ("armed signal_arrived %p, sig %d, res %d", signal_arrived,
		  sig, res);
}

bool
_threadinfo::interrupt_now (CONTEXT *ctx, int sig, void *handler,
			    struct sigaction& siga)
{
  push (0);
  interrupt_setup (sig, handler, siga, (__stack_t) ctx->Eip);
  ctx->Eip = pop ();
  SetThreadContext (*this, ctx); /* Restart the thread in a new location */
  return 1;
}

void __stdcall
signal_fixup_after_fork ()
{
  if (_my_tls.sig)
    {
      _my_tls.sig = 0;
      _my_tls.stackptr = _my_tls.stack + 1;	// FIXME?
      set_signal_mask (_my_tls.oldmask);
    }
  sigproc_init ();
}

extern "C" void __stdcall
set_sig_errno (int e)
{
  *_my_tls.errno_addr = e;
  _my_tls.saved_errno = e;
  // sigproc_printf ("errno %d", e);
}

static int setup_handler (int, void *, struct sigaction&, _threadinfo *tls)
  __attribute__((regparm(3)));
static int
setup_handler (int sig, void *handler, struct sigaction& siga, _threadinfo *tls)
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
      __stack_t *retaddr_on_stack = tls->stackptr - 1;
      if (retaddr_on_stack >= tls->stack)
	{
	  __stack_t retaddr = InterlockedExchange ((LONG *) retaddr_on_stack, 0);
	  if (!retaddr)
	    continue;
	  tls->reset_exception ();
	  tls->interrupt_setup (sig, handler, siga, retaddr);
	  sigproc_printf ("interrupted known cygwin routine");
	  interrupted = true;
	  break;
	}

      DWORD res;
      HANDLE hth = (HANDLE) *tls;

      /* Suspend the thread which will receive the signal.  But first ensure that
	 this thread doesn't have any mutos.  (FIXME: Someday we should just grab
	 all of the mutos rather than checking for them)
	 For Windows 95, we also have to ensure that the addresses returned by GetThreadContext
	 are valid.
	 If one of these conditions is not true we loop for a fixed number of times
	 since we don't want to stall the signal handler.  FIXME: Will this result in
	 noticeable delays?
	 If the thread is already suspended (which can occur when a program has called
	 SuspendThread on itself then just queue the signal. */

#ifndef DEBUGGING
      sigproc_printf ("suspending mainthread");
#else
      cx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
      if (!GetThreadContext (hth, &cx))
	memset (&cx, 0, sizeof cx);
      sigproc_printf ("suspending mainthread PC %p", cx.Eip);
#endif
      res = SuspendThread (hth);
      /* Just release the lock now since we hav suspended the main thread and it
	 definitely can't be grabbing it now.  This will have to change, of course,
	 if/when we can send signals to other than the main thread. */

      /* Just set pending if thread is already suspended */
      if (res)
	{
	  (void) ResumeThread (hth);
	  break;
	}

      // FIXME - add check for reentering of DLL here

      cx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
      if (!GetThreadContext (hth, &cx))
	system_printf ("couldn't get context of main thread, %E");
      else if (interruptible (cx.Eip))
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
#endif /* i386 */

#ifndef HAVE_CALL_HANDLER
#error "Need to supply machine dependent setup_handler"
#endif

/* Keyboard interrupt handler.  */
static BOOL WINAPI
ctrl_c_handler (DWORD type)
{
  static bool saw_close;
  _my_tls.remove (INFINITE);

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
	  saw_close = true;
	  sig_send (NULL, SIGHUP);
	  return FALSE;
	}
      if (!saw_close && type == CTRL_LOGOFF_EVENT)
        {
	  /* Check if the process is actually associated with a visible
	     window station, one which actually represents a visible desktop.
	     If not, the CTRL_LOGOFF_EVENT doesn't concern this process. */
	  if (has_visible_window_station ())
	    sig_send (myself_nowait, SIGHUP);
	  return FALSE;
	}
    }

  /* If we are a stub and the new process has a pinfo structure, let it
     handle this signal. */
  if (dwExeced && pinfo (dwExeced))
    return TRUE;

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
      t->last_ctrl_c = GetTickCount ();
      kill (-myself->pid, SIGINT);
      t->last_ctrl_c = GetTickCount ();
      return TRUE;
    }

  return TRUE;
}

/* Function used by low level sig wrappers. */
extern "C" void __stdcall
set_process_mask (sigset_t newmask)
{
  set_signal_mask (newmask);
}

/* Set the signal mask for this process.
   Note that some signals are unmaskable, as in UNIX.  */
extern "C" void __stdcall
set_signal_mask (sigset_t newmask, sigset_t oldmask)
{
  mask_sync->acquire (INFINITE);
  newmask &= ~SIG_NONMASKABLE;
  sigset_t mask_bits = oldmask & ~newmask;
  sigproc_printf ("oldmask %p, newmask %p, mask_bits %p", oldmask, newmask,
		  mask_bits);
  myself->setsigmask (newmask);	// Set a new mask
  mask_sync->release ();
  if (mask_bits)
    sig_dispatch_pending ();
  else
    sigproc_printf ("not calling sig_dispatch_pending");
  return;
}

int __stdcall
sig_handle (int sig, sigset_t mask, int pid, _threadinfo *tls)
{
  if (sig == SIGCONT)
    {
      DWORD stopped = myself->process_state & PID_STOPPED;
      myself->stopsig = 0;
      myself->process_state &= ~PID_STOPPED;
      /* Clear pending stop signals */
      sig_clear (SIGSTOP);
      sig_clear (SIGTSTP);
      sig_clear (SIGTTIN);
      sig_clear (SIGTTOU);
      if (stopped)
	SetEvent (sigCONT);
    }

  int rc = 1;
  bool insigwait_mask = tls ? sigismember (&tls->sigwait_mask, sig) : false;
  bool special_case = ISSTATE (myself, PID_STOPPED) || VFORKPID;
  bool masked = sigismember (&mask, sig);
  if (sig != SIGKILL && sig != SIGSTOP
      && (special_case || masked || insigwait_mask
	  || (tls && sigismember (&tls->sigmask, sig))))
    {
      sigproc_printf ("signal %d blocked", sig);
      if ((!special_case && !masked)
	  && (insigwait_mask || (tls = _threadinfo::find_tls (sig)) != NULL))
	goto thread_specific;
      rc = -1;
      goto done;
    }

  /* Clear pending SIGCONT on stop signals */
  if (sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU)
    sig_clear (SIGCONT);

  sigproc_printf ("signal %d processing", sig);
  struct sigaction thissig = global_sigs[sig];
  void *handler;
  handler = (void *) thissig.sa_handler;

  myself->rusage_self.ru_nsignals++;

  if (sig == SIGKILL)
    goto exit_sig;

  if (sig == SIGSTOP)
    goto stop;

#if 0
  char sigmsg[24];
  __small_sprintf (sigmsg, "cygwin: signal %d\n", sig);
  OutputDebugString (sigmsg);
#endif

  if (handler == (void *) SIG_DFL)
    {
      if (insigwait_mask)
	goto thread_specific;
      if (sig == SIGCHLD || sig == SIGIO || sig == SIGCONT || sig == SIGWINCH
	  || sig == SIGURG)
	{
	  sigproc_printf ("default signal %d ignored", sig);
	  goto done;
	}

      if (sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU)
	goto stop;

      goto exit_sig;
    }

  if (handler == (void *) SIG_IGN)
    {
      sigproc_printf ("signal %d ignored", sig);
      goto done;
    }

  if (handler == (void *) SIG_ERR)
    goto exit_sig;

  goto dosig;

stop:
  /* Eat multiple attempts to STOP */
  if (ISSTATE (myself, PID_STOPPED))
    goto done;
  handler = (void *) sig_handle_tty_stop;
  thissig = global_sigs[SIGSTOP];

dosig:
  /* Dispatch to the appropriate function. */
  sigproc_printf ("signal %d, about to call %p", sig, handler);
  rc = setup_handler (sig, handler, thissig, tls ?: _main_tls);

done:
  sigproc_printf ("returning %d", rc);
  return rc;

thread_specific:
  tls->sig = sig;
  sigproc_printf ("releasing sigwait for thread");
  SetEvent (tls->event);
  goto done;

exit_sig:
  if (sig == SIGQUIT || sig == SIGABRT)
    {
      CONTEXT c;
      c.ContextFlags = CONTEXT_FULL;
      GetThreadContext (hMainThread, &c);
      if (!try_to_debug ())
	stackdump (c.Ebp, 1, 1);
      sig |= 0x80;
    }
  sigproc_printf ("signal %d, about to call do_exit", sig);
  signal_exit (sig);
  /* Never returns */
}

CRITICAL_SECTION NO_COPY exit_lock;

/* Cover function to `do_exit' to handle exiting even in presence of more
   exceptions.  We used to call exit, but a SIGSEGV shouldn't cause atexit
   routines to run.  */
static void
signal_exit (int rc)
{
  EnterCriticalSection (&exit_lock);
  rc = EXIT_SIGNAL | (rc << 8);
  if (exit_already++)
    myself->exit (rc);

  /* We'd like to stop the main thread from executing but when we do that it
     causes random, inexplicable hangs.  So, instead, we set up the priority
     of this thread really high so that it should do its thing and then exit. */
  (void) SetThreadPriority (hMainThread, THREAD_PRIORITY_IDLE);
  (void) SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);

  user_data->resourcelocks->Delete ();
  user_data->resourcelocks->Init ();

  if (hExeced)
    {
      sigproc_printf ("terminating captive process");
      TerminateProcess (hExeced, rc);
    }

  sigproc_printf ("about to call do_exit (%x)", rc);
  (void) SetEvent (signal_arrived);
  do_exit (rc);
}

HANDLE NO_COPY title_mutex = NULL;

void
events_init (void)
{
  char *name;
  char mutex_name[CYG_MAX_PATH];
  /* title_mutex protects modification of console title. It's necessary
     while finding console window handle */

  if (!(title_mutex = CreateMutex (&sec_all_nih, FALSE,
				   name = shared_name (mutex_name,
						       "title_mutex", 0))))
    api_fatal ("can't create title mutex '%s', %E", name);

  ProtectHandle (title_mutex);
  new_muto (mask_sync);
  windows_system_directory[0] = '\0';
  (void) GetSystemDirectory (windows_system_directory, sizeof (windows_system_directory) - 2);
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
  InitializeCriticalSection (&exit_lock);
}

void
events_terminate (void)
{
  exit_already = 1;
}

extern "C" {
int __stdcall
call_signal_handler_now ()
{
  int sa_flags = 0;
  while (_my_tls.sig && _my_tls.stackptr > _my_tls.stack)
    {
      sa_flags = _my_tls.sa_flags;
      int sig = _my_tls.sig;
      void (*sigfunc) (int) = _my_tls.func;

      (void) _my_tls.pop ();
      reset_signal_arrived ();
      sigset_t oldmask = _my_tls.oldmask;
      int this_errno = _my_tls.saved_errno;
      set_process_mask (_my_tls.newmask);
      _my_tls.sig = 0;
      sigfunc (sig);
      set_process_mask (oldmask);
      if (this_errno >= 0)
	set_errno (this_errno);
    }

  return sa_flags & SA_RESTART;
}

void __stdcall
reset_signal_arrived ()
{
  (void) ResetEvent (signal_arrived);
  sigproc_printf ("reset signal_arrived");
}
}
