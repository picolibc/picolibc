/* exceptions.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <errno.h>

#define Win32_Winsock
#include "winsup.h"
#include "exceptions.h"
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#include <imagehlp.h>
#include "autoload.h"

char debugger_command[2 * MAX_PATH + 20];

extern "C" {
static int handle_exceptions (EXCEPTION_RECORD *, void *, CONTEXT *, void *);
extern void sigreturn ();
extern void sigdelayed ();
extern void siglast ();
extern DWORD __sigfirst, __siglast;
};

static BOOL WINAPI ctrl_c_handler (DWORD);
static void really_exit (int);

/* This is set to indicate that we have already exited.  */

static NO_COPY int exit_already = 0;
static NO_COPY muto *mask_sync = NULL;

HANDLE NO_COPY console_handler_thread_waiter = NULL;

static const struct
{
  unsigned int code;
  const char *name;
} status_info[] NO_COPY =
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

#ifdef __i386__

// Set up the exception handler for the current thread.  The PowerPC & Mips
// use compiler generated tables to set up the exception handlers for each
// region of code, and the kernel walks the call list until it finds a region
// of code that handles exceptions.  The x86 on the other hand uses segment
// register fs, offset 0 to point to the current exception handler.

asm (".equ __except_list,0");

extern exception_list *_except_list asm ("%fs:__except_list");

static void
init_exception_handler (exception_list *el)
{
  el->handler = handle_exceptions;
  el->prev = _except_list;
  _except_list = el;
}

#define INIT_EXCEPTION_HANDLER(el) init_exception_handler (el)
#endif

void
set_console_handler ()
{
  /* Initialize global security attribute stuff */

  sec_none.nLength = sec_none_nih.nLength =
  sec_all.nLength = sec_all_nih.nLength = sizeof (SECURITY_ATTRIBUTES);
  sec_none.bInheritHandle = sec_all.bInheritHandle = TRUE;
  sec_none_nih.bInheritHandle = sec_all_nih.bInheritHandle = FALSE;
  sec_none.lpSecurityDescriptor = sec_none_nih.lpSecurityDescriptor = NULL;
  sec_all.lpSecurityDescriptor = sec_all_nih.lpSecurityDescriptor =
    get_null_sd ();

  /* Allocate the event needed for ctrl_c_handler synchronization with
     wait_sig. */
  if (!console_handler_thread_waiter)
    CreateEvent (&sec_none_nih, TRUE, TRUE, NULL);
  (void) SetConsoleCtrlHandler (ctrl_c_handler, FALSE);
  if (!SetConsoleCtrlHandler (ctrl_c_handler, TRUE))
    system_printf ("SetConsoleCtrlHandler failed, %E");
}

extern "C" void
init_exceptions (exception_list *el)
{
#ifdef INIT_EXCEPTION_HANDLER
  INIT_EXCEPTION_HANDLER (el);
#endif
}

extern "C" void
error_start_init (const char *buf)
{
  if (!buf || !*buf)
    {
      debugger_command[0] = '\0';
      return;
    }

  char myself_posix_name[MAX_PATH];

  /* FIXME: gdb cannot use win32 paths, but what if debugger isn't gdb? */
  cygwin_conv_to_posix_path (myself->progname, myself_posix_name);
  __small_sprintf (debugger_command, "%s %s", buf, myself_posix_name);
}

/* Utilities for dumping the stack, etc.  */

static void
exception (EXCEPTION_RECORD *e,  CONTEXT *in)
{
  const char *exception_name = 0;

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

extern "C" {
static LPVOID __stdcall
sfta(HANDLE, DWORD)
{
  return NULL;
}

static DWORD __stdcall
sgmb(HANDLE, DWORD)
{
  return 4;
}

#ifdef __i386__
/* Print a stack backtrace. */

#define HAVE_STACK_TRACE

/* Set from CYGWIN environment variable if want to use old method. */
BOOL NO_COPY oldstack = 0;

/* The function used to load the imagehlp DLL.  Returns TRUE if the
   DLL was found. */
static LoadDLLinitfunc (imagehlp)
{
  imagehlp_handle = LoadLibrary ("imagehlp.dll");
  return !!imagehlp_handle;
}

LoadDLLinit (imagehlp)	/* Set up storage for imagehlp.dll autoload */
LoadDLLfunc (StackWalk, StackWalk@36, imagehlp)

/* A class for manipulating the stack. */
class stack_info
{
  int first_time;		/* True if just starting to iterate. */
  HANDLE hproc;			/* Handle of process to inspect. */
  HANDLE hthread;		/* Handle of thread to inspect. */
  int (stack_info::*get) (HANDLE, HANDLE); /* Gets the next stack frame */
public:
  STACKFRAME sf;		/* For storing the stack information */
  int walk (HANDLE, HANDLE);	/* Uses the StackWalk function */
  int brute_force (HANDLE, HANDLE); /* Uses the "old" method */
  void init (CONTEXT *);	/* Called the first time that stack info is needed */

  /* The constructor remembers hproc and hthread and determines which stack walking
     method to use */
  stack_info (int use_old_stack, HANDLE hp, HANDLE ht): hproc(hp), hthread(ht)
  {
    if (!use_old_stack && LoadDLLinitnow (imagehlp))
      get = &stack_info::walk;
    else
      get = &stack_info::brute_force;
  }
  /* Postfix ++ iterates over the stack, returning zero when nothing is left. */
  int operator ++(int) { return (this->*get) (hproc, hthread); }
};

/* The number of parameters used in STACKFRAME */
#define NPARAMS (sizeof(thestack->sf.Params) / sizeof(thestack->sf.Params[0]))

/* This is the main stack frame info for this process. */
static stack_info *thestack = NULL;
static signal_dispatch sigsave;

/* Initialize everything needed to start iterating. */
void
stack_info::init (CONTEXT *cx)
{
  first_time = 1;
  memset (&sf, 0, sizeof(sf));
  sf.AddrPC.Offset = cx->Eip;
  sf.AddrPC.Mode = AddrModeFlat;
  sf.AddrStack.Offset = cx->Esp;
  sf.AddrStack.Mode = AddrModeFlat;
  sf.AddrFrame.Offset = cx->Ebp;
  sf.AddrFrame.Mode = AddrModeFlat;
}

/* Walk the stack by looking at successive stored 'bp' frames.
   This is not foolproof. */
int
stack_info::brute_force (HANDLE, HANDLE)
{
  char **ebp;
  if (first_time)
    /* Everything is filled out already */
    ebp = (char **) sf.AddrFrame.Offset;
  else if ((ebp = (char **) *(char **) sf.AddrFrame.Offset) != NULL)
    {
      sf.AddrFrame.Offset = (DWORD) ebp;
      sf.AddrPC.Offset = sf.AddrReturn.Offset;
    }
  else
    return 0;

  first_time = 0;
  if (!sf.AddrPC.Offset)
    return 0;		/* stack frames are exhausted */

  /* The return address always follows the stack pointer */
  sf.AddrReturn.Offset = (DWORD) *++ebp;

  /* The arguments follow the return address */
  for (unsigned i = 0; i < NPARAMS; i++)
    sf.Params[i] = (DWORD) *++ebp;
  return 1;
}

/* Use Win32 StackWalk() API to display the stack.  This is theoretically
   more foolproof than the brute force method above. */
int
stack_info::walk (HANDLE hproc, HANDLE hthread)
{
#ifdef SOMEDAY
  /* It would be nice to get more information (like DLL symbols and module name)
     for each stack frame but in order to do that we have to call SymInitialize.
     It doesn't seem to be possible to do this inside of an excaption handler for
     some reason. */
  static int initialized = 0;
  if (!initialized && !SymInitialize(hproc, NULL, TRUE))
    small_printf("SymInitialize error, %E\n");
  initialized = 1;
#endif

  return StackWalk (IMAGE_FILE_MACHINE_I386, hproc, hthread, &sf, NULL, NULL,
		    sfta, sgmb, NULL) && !!sf.AddrFrame.Offset;
}

/* Dump the stack using either the old method or the new Win32 API method */
void
stack (HANDLE hproc, HANDLE hthread, CONTEXT *cx)
{
  int i;

  /* Set this up if it's the first time. */
  if (!thestack)
    thestack = new stack_info (oldstack, hproc, hthread);

  thestack->init (cx);	/* Initialize from the input CONTEXT */
  small_printf ("Stack trace:\r\nFrame     Function  Args\r\n");
  for (i = 0; i < 16 && (*thestack)++ ; i++)
    {
      small_printf ("%08x  %08x ", thestack->sf.AddrFrame.Offset,
		    thestack->sf.AddrPC.Offset);
      for (unsigned j = 0; j < NPARAMS; j++)
	small_printf ("%s%08x", j == 0 ? " (" : ", ", thestack->sf.Params[j]);
      small_printf (")\r\n");
    }
  small_printf ("End of stack trace%s",
	      i == 16 ? " (more stack frames may be present)" : "");
}

/* Temporary (?) function for external callers to get a stack dump */
extern "C" void
cygwin_stackdump()
{
  CONTEXT c;
  c.ContextFlags = CONTEXT_FULL;
  HANDLE h1 = GetCurrentProcess ();
  HANDLE h2 = GetCurrentThread ();
  GetThreadContext (h2, &c);
  stack(h1, h2, &c);
}

static int NO_COPY keep_looping = 0;

extern "C" int
try_to_debug ()
{
  debug_printf ("debugger_command %s", debugger_command);
  if (*debugger_command == '\0')
    return 0;

  __small_sprintf (strchr (debugger_command, '\0'), " %u", GetCurrentProcessId ());

  BOOL dbg;

  PROCESS_INFORMATION pi = {0};

  STARTUPINFO si = {0};
  si.lpReserved = NULL;
  si.lpDesktop = NULL;
  si.dwFlags = 0;
  si.cb = sizeof (si);

  /* FIXME: need to know handles of all running threads to
     suspend_all_threads_except (current_thread_id);
  */

  /* if any of these mutexes is owned, we will fail to start any cygwin app
     until trapped app exits */

  ReleaseMutex (pinfo_mutex);
  ReleaseMutex (title_mutex);

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
    {
      system_printf ("Failed to start debugger: %E");
      /* FIXME: need to know handles of all running threads to
        resume_all_threads_except (current_thread_id);
      */
    }
  else
    {
      keep_looping = 1;
      while (keep_looping)
	Sleep (10000);
    }

  return 0;
}

void
stackdump (HANDLE hproc, HANDLE hthread, EXCEPTION_RECORD *e, CONTEXT *in)
{
  char *p;
  if (myself->progname[0])
    {
      /* write to progname.stackdump if possible */
      if ((p = strrchr (myself->progname, '\\')))
	p++;
      else
	p = myself->progname;
      char corefile[strlen(p) + sizeof(".stackdump")];
      __small_sprintf (corefile, "%s.stackdump", p);
      HANDLE h = CreateFile (corefile, GENERIC_WRITE, 0, &sec_none_nih,
			     CREATE_ALWAYS, 0, 0);
      if (h != INVALID_HANDLE_VALUE)
	{
	  system_printf ("Dumping stack trace to %s", corefile);
	  SetStdHandle (STD_ERROR_HANDLE, h);
	}
    }
  if (e)
    exception (e, in);
  stack (hproc, hthread, in);
}

/* Main exception handler. */

static int
handle_exceptions (EXCEPTION_RECORD *e, void *, CONTEXT *in, void *)
{
  int sig;

  /* If we've already exited, don't do anything here.  Returning 1
     tells Windows to keep looking for an exception handler.  */
  if (exit_already)
    return 1;

  /* Coerce win32 value to posix value.  */
  switch (e->ExceptionCode)
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

  debug_printf ("In cygwin_except_handler exc %p at %p sp %p", e->ExceptionCode, in->Eip, in->Esp);
  debug_printf ("In cygwin_except_handler sig = %d at %p", sig, in->Eip);

  if (myself->getsig(sig).sa_mask & SIGTOMASK (sig))
    syscall_printf ("signal %d, masked %p", sig, myself->getsig(sig).sa_mask);

  if (!myself->progname[0]
      || (void *) myself->getsig(sig).sa_handler == (void *) SIG_DFL
      || (void *) myself->getsig(sig).sa_handler == (void *) SIG_IGN
      || (void *) myself->getsig(sig).sa_handler == (void *) SIG_ERR)
    {
      static NO_COPY int traced = 0;

      /* Print the exception to the console */
      if (e)
	{
	  for (int i = 0; status_info[i].name; i++)
	    {
	      if (status_info[i].code == e->ExceptionCode)
		{
		  system_printf ("Exception: %s", status_info[i].name);
		  break;
		}
	    }
	}

      /* Another exception could happen while tracing or while exiting.
	 Only do this once.  */
      if (traced++)
        system_printf ("Error while dumping state (probably corrupted stack)");
      else
	{
	  HANDLE hthread;
	  DuplicateHandle (hMainProc, GetCurrentThread (),
			   hMainProc, &hthread, 0, FALSE, DUPLICATE_SAME_ACCESS);
	  stackdump (hMainProc, hthread, e, in);
	}
      try_to_debug ();
      really_exit (EXIT_SIGNAL | sig);
    }

  debug_printf ("In cygwin_except_handler calling %p",
		 myself->getsig(sig).sa_handler);

  DWORD *bp = (DWORD *)in->Esp;
  for (DWORD *bpend = bp - 8; bp > bpend; bp--)
    if (*bp == in->SegCs && bp[-1] == in->Eip)
      {
	bp -= 2;
	break;
      }
  
  in->Ebp = (DWORD) bp;
  sigsave.cx = in;
  sig_send (NULL, sig);		// Signal myself
  sigsave.cx = NULL;
  return 0;
}
#endif /* __i386__ */

#ifndef HAVE_STACK_TRACE
void
stack (void)
{
  system_printf ("Stack trace not yet supported on this machine.");
}
#endif
}

/* Utilities to call a user supplied exception handler.  */

#define SIG_NONMASKABLE	(SIGTOMASK (SIGCONT) | SIGTOMASK (SIGKILL) | SIGTOMASK (SIGSTOP))

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
  sigset_t oldmask = myself->getsigmask ();	// Remember for restoration

  set_process_mask (tempmask & ~SIG_NONMASKABLE);// Let signals we're
				//  interested in through.
  sigproc_printf ("old mask %x, new mask %x", oldmask, tempmask);

  sig_dispatch_pending (0);
  WaitForSingleObject (signal_arrived, INFINITE);

  set_sig_errno (EINTR);	// Per POSIX

  /* A signal dispatch function will have been added to our stack and will
     be hit eventually.  Set the old mask to be restored when the signal
     handler returns. */

  sigsave.oldmask = oldmask;	// Will be restored by signal handler
  return -1;
}

extern DWORD exec_exit;		// Possible exit value for exec
extern int pending_signals;

extern __inline int
interruptible (DWORD pc)
{
  DWORD pchigh = pc & 0xf0000000;
  return ((pc >= (DWORD) &__sigfirst) && (pc <= (DWORD) &__siglast)) ||
	 !(pchigh == 0xb0000000 || pchigh == 0x70000000 || pchigh == 0x60000000);
}

void
interrupt_now (CONTEXT *ctx, int sig, struct sigaction& siga, void *handler)
{
  DWORD oldmask = myself->getsigmask ();
  set_process_mask (myself->getsigmask () | siga.sa_mask | SIGTOMASK (sig));

  DWORD *sp = (DWORD *) ctx->Esp;
  *(--sp) = ctx->Eip; /*  ctxinal IP where program was suspended */
  *(--sp) = ctx->EFlags;
  *(--sp) = ctx->Esi;
  *(--sp) = ctx->Edi;
  *(--sp) = ctx->Edx;
  *(--sp) = ctx->Ecx;
  *(--sp) = ctx->Ebx;
  *(--sp) = ctx->Eax;
  *(--sp) = (DWORD)-1;	/* no saved errno. */
  *(--sp) = oldmask;
  *(--sp) = sig;
  *(--sp) = (DWORD) sigreturn;

  ctx->Esp = (DWORD) sp;
  ctx->Eip = (DWORD) handler;

  SetThreadContext (myself->getthread2signal(), ctx); /* Restart the thread */
}

int
interrupt_on_return (CONTEXT *ctx, int sig, struct sigaction& siga, void *handler)
{
  int i;

  if (sigsave.sig)
    return 0;	/* Already have a signal stacked up */

  /* Set this up if it's the first time. */
  /* FIXME: Eventually augment to handle more than one thread */
  if (!thestack)
    thestack = new stack_info (oldstack, hMainProc, hMainThread);

  thestack->init (ctx);  /* Initialize from the input CONTEXT */
  for (i = 0; i < 32 && (*thestack)++ ; i++)
    if (interruptible (thestack->sf.AddrReturn.Offset))
      {
	DWORD *addr_retaddr = ((DWORD *)thestack->sf.AddrFrame.Offset) + 1;
	if (*addr_retaddr  != thestack->sf.AddrReturn.Offset)
	  break;
	sigsave.retaddr = *addr_retaddr;
	*addr_retaddr = (DWORD) sigdelayed;
	sigsave.oldmask = myself->getsigmask ();	// Remember for restoration
	set_process_mask (myself->getsigmask () | siga.sa_mask | SIGTOMASK (sig));
	sigsave.func = (void (*)(int)) handler;
	sigsave.sig = sig;
	sigsave.saved_errno = -1;		// Flag: no errno to save
	break;
      }

  return 1;
}

extern "C" void __stdcall
set_sig_errno (int e)
{
  set_errno (e);
  sigsave.saved_errno = e;
}

static int
call_handler (int sig, struct sigaction& siga, void *handler)
{
  CONTEXT *cx, orig;
  int res;

  if (hExeced != NULL && hExeced != INVALID_HANDLE_VALUE)
    {
      SetEvent (signal_arrived);	// For an EINTR case
      sigproc_printf ("armed signal_arrived");
      exec_exit = sig;			// Maybe we'll exit with this value
      return 1;
    }

  /* Suspend the running thread, grab its context somewhere safe
     and run the exception handler in the context of the thread -
     we have to do that since sometimes they don't return - and if
     this thread doesn't return, you won't ever get another exception. */

  sigproc_printf ("Suspending %p (mainthread)", myself->getthread2signal());
  HANDLE hth = myself->getthread2signal ();
  res = SuspendThread (hth);
  sigproc_printf ("suspend said %d, %E", res);

  /* Clear any waiting threads prior to dispatching to handler function */
  proc_subproc(PROC_CLEARWAIT, 0);

  if (sigsave.cx)
    {
      cx = sigsave.cx;
      sigsave.cx = NULL;
    }
  else
    {
      cx = &orig;
      /* FIXME - this does not preserve FPU state */
      orig.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
      if (!GetThreadContext (hth, cx))
	{
	  system_printf ("couldn't get context of main thread, %E");
	  ResumeThread (hth);
	  goto out;
	}
    }

  if (cx == &orig && interruptible (cx->Eip))
    interrupt_now (cx, sig, siga, handler);
  else if (!interrupt_on_return (cx, sig, siga, handler))
    {
      pending_signals = 1;	/* FIXME: Probably need to be more tricky here */
      sig_set_pending (sig);
    }

  (void) ResumeThread (hth);
  (void) SetEvent (signal_arrived);	// For an EINTR case
  sigproc_printf ("armed signal_arrived %p, res %d", signal_arrived, res);

out:
  sigproc_printf ("returning");
  return 1;
}
#endif /* i386 */

#ifndef HAVE_CALL_HANDLER
#error "Need to supply machine dependent call_handler"
#endif

/* Keyboard interrupt handler.  */
static BOOL WINAPI
ctrl_c_handler (DWORD type)
{
  if (type == CTRL_LOGOFF_EVENT)
    return TRUE;

  /* Wait for sigproc_init to tell us that it's safe to send something.
     This event will always be in a signalled state when wait_sig is
     ready to process signals. */
  (void) WaitForSingleObject (console_handler_thread_waiter, 5000);

  if ((type == CTRL_CLOSE_EVENT) || (type == CTRL_SHUTDOWN_EVENT))
    /* Return FALSE to prevent an "End task" dialog box from appearing
       for each Cygwin process window that's open when the computer
       is shut down or console window is closed. */
    {
      sig_send (NULL, SIGHUP);
      return FALSE;
    }
  tty_min *t = cygwin_shared->tty.get_tty(myself->ctty);
  /* Ignore this if we're not the process group lead since it should be handled
     *by* the process group leader. */
  if (t->getpgid () != myself->pid ||
      (GetTickCount () - t->last_ctrl_c) < MIN_CTRL_C_SLOP)
    return TRUE;
  else
    /* Otherwise we just send a SIGINT to the process group and return TRUE (to indicate
       that we have handled the signal).  At this point, type should be
       a CTRL_C_EVENT or CTRL_BREAK_EVENT. */
    {
      t->last_ctrl_c = GetTickCount ();
      kill (-myself->pid, SIGINT);
      t->last_ctrl_c = GetTickCount ();
      return TRUE;
    }
}

/* Set the signal mask for this process.
 * Note that some signals are unmaskable, as in UNIX.
 */
extern "C" void __stdcall
set_process_mask (sigset_t newmask)
{
  mask_sync->acquire (INFINITE);
  newmask &= ~SIG_NONMASKABLE;
  sigproc_printf ("old mask = %x, new mask = %x", myself->getsigmask (), newmask);
  myself->setsigmask (newmask);	// Set a new mask
  mask_sync->release ();
  return;
}

extern "C" {
static void
sig_handle_tty_stop (int sig)
{
#if 0
  HANDLE waitbuf[2];

  /* Be sure that process's main thread isn't an owner of vital
     mutex to prevent cygwin subsystem lockups */
  waitbuf[0] = pinfo_mutex;
  waitbuf[1] = title_mutex;
  WaitForMultipleObjects (2, waitbuf, TRUE, INFINITE);
  ReleaseMutex (pinfo_mutex);
  ReleaseMutex (title_mutex);
#endif
  myself->stopsig = sig;
  myself->process_state |= PID_STOPPED;
  /* See if we have a living parent.  If so, send it a special signal.
   * It will figure out exactly which pid has stopped by scanning
   * its list of subprocesses.
   */
  if (my_parent_is_alive ())
    {
      pinfo *parent = procinfo(myself->ppid);
      sig_send (parent, __SIGCHILDSTOPPED);
    }
  sigproc_printf ("process %d stopped by signal %d, parent_alive %p",
		  myself->pid, sig, parent_alive);
  /* There is a small race here with the above two mutexes */
  SuspendThread (hMainThread);
  return;
}
}

int __stdcall
sig_handle (int sig)
{
  int rc = 0;

  sigproc_printf ("signal %d", sig);

  struct sigaction thissig = myself->getsig(sig);
  void *handler = (void *) thissig.sa_handler;

  myself->rusage_self.ru_nsignals++;

  /* Clear pending SIGCONT on stop signals */
  if (sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU)
    sig_clear (SIGCONT);

  if (sig == SIGKILL)
    goto exit_sig;

  if (sig == SIGSTOP)
    goto stop;

  /* FIXME: Should we still do this if SIGCONT has a handler? */
  if (sig == SIGCONT)
    {
      myself->stopsig = 0;
      myself->process_state &= ~PID_STOPPED;
      /* Clear pending stop signals */
      sig_clear (SIGSTOP);
      sig_clear (SIGTSTP);
      sig_clear (SIGTTIN);
      sig_clear (SIGTTOU);
      /* Windows 95 hangs on resuming non-suspended thread */
      SuspendThread (hMainThread);
      while (ResumeThread (hMainThread) > 1)
	;
      /* process pending signals */
      sig_dispatch_pending ();
    }

#if 0
  char sigmsg[24];
  __small_sprintf (sigmsg, "cygwin: signal %d\n", sig);
  OutputDebugString (sigmsg);
#endif

  if (handler == (void *) SIG_DFL)
    {
      if (sig == SIGCHLD || sig == SIGIO || sig == SIGCONT || sig == SIGWINCH)
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

  if ((sig == SIGCHLD) && (thissig.sa_flags & SA_NOCLDSTOP))
    goto done;

  goto dosig;

stop:
  handler = (void *) sig_handle_tty_stop;

dosig:
  /* Dispatch to the appropriate function. */
  sigproc_printf ("signal %d, about to call %p", sig, thissig.sa_handler);
  rc = call_handler (sig, thissig, handler);

done:
  sigproc_printf ("returning %d", rc);
  return rc;

exit_sig:
  if (sig == SIGQUIT || sig == SIGABRT)
    {
      stackdump (NULL, NULL, NULL, NULL);
      try_to_debug ();
    }
  sigproc_printf ("signal %d, about to call do_exit", sig);
  TerminateThread (hMainThread, 0);
  /* FIXME: This just works around the problem so that we don't attempt to
     use a resource lock when exiting.  */
  user_data->resourcelocks->Delete();
  user_data->resourcelocks->Init();
  do_exit (EXIT_SIGNAL | (sig << 8));
  /* Never returns */
}

/* Cover function to `do_exit' to handle exiting even in presence of more
   exceptions.  We use to call exit, but a SIGSEGV shouldn't cause atexit
   routines to run.  */

static void
really_exit (int rc)
{
  /* If the exception handler gets a trap, we could recurse awhile.
     If this is non-zero, skip the cleaning up and exit NOW.  */

  if (exit_already++)
    {
      /* We are going down - reset our process_state without locking. */
      myself->record_death (FALSE);
      ExitProcess (rc);
    }

  do_exit (rc);
}

HANDLE NO_COPY pinfo_mutex = NULL;
HANDLE NO_COPY title_mutex = NULL;

void
events_init (void)
{
  /* pinfo_mutex protects access to process table */

  if (!(pinfo_mutex = CreateMutex (&sec_all_nih, FALSE,
				   shared_name ("pinfo_mutex", 0))))
    api_fatal ("catastrophic failure - unable to create pinfo_mutex, %E");

  ProtectHandle (pinfo_mutex);

  /* title_mutex protects modification of console title. It's neccessary
     while finding console window handle */

  if (!(title_mutex = CreateMutex (&sec_all_nih, FALSE,
				   shared_name ("title_mutex", 0))))
    api_fatal ("can't create title mutex, %E");

  ProtectHandle (title_mutex);
  mask_sync = new_muto (FALSE, NULL);
}

void
events_terminate (void)
{
//CloseHandle (pinfo_mutex);	// Use implicit close on exit to avoid race
  ForceCloseHandle (title_mutex);
  exit_already = 1;
}

#define pid_offset (unsigned)(((pinfo *)NULL)->pid)
extern "C" {
void unused_sig_wrapper()
{
/* Signal cleanup stuff.  Cleans up stack (too bad that we didn't
   prototype signal handlers as __stdcall), calls _set_process_mask
   to restore any mask, restores any potentially clobbered registered
   and returns to orignal caller. */
__asm__ volatile ("
	.text
___sigfirst:
	.globl	__raise
__raise:
	pushl	%%ebp
	movl	%%esp,%%ebp
	movl	8(%%ebp),%%eax
	pushl	%%eax
	movl	$_myself,%%eax
	pushl	%6(%%eax)
	call	__kill
	mov	%%ebp,%%esp
	popl	%%ebp
	ret

_sigreturn:
	addl	$4,%%esp
	call	_set_process_mask@4
	popl	%%eax		# saved errno
	testl	%%eax,%%eax	# lt 0
	jl	1f		# yup.  ignore it
	movl	%1,%%ebx
	movl	%%eax,(%%ebx)
1:	popl	%%eax
	popl	%%ebx
	popl	%%ecx
	popl	%%edx
	popl	%%edi
	popl	%%esi
	popf
	ret

_sigdelayed:
	# addl	4,%%esp
	cmpl	$0,_pending_signals
	je	2f
	pushl	$0
	call	_sig_dispatch_pending@4
2:	pushl	%2	# original return address
	pushf
	pushl	%%esi
	pushl	%%edi
	pushl	%%edx
	pushl	%%ecx
	pushl	%%ebx
	pushl	%%eax
	pushl	%7	# saved errno
	pushl	%3	# oldmask
	pushl	%4	# signal argument
	pushl	$_sigreturn
	movl	$0,%0
	pushl	$_signal_arrived
	call	_ResetEvent@4
	jmp	*%5

___siglast:
" : "=m" (sigsave.sig) : "m" (&_impure_ptr->_errno),
  "g" (sigsave.retaddr), "g" (sigsave.oldmask), "g" (sigsave.sig),
    "g" (sigsave.func), "o" (pid_offset), "g" (sigsave.saved_errno)
  );
}
}
