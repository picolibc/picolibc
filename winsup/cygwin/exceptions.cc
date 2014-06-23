/* exceptions.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define CYGTLS_HANDLE
#include "winsup.h"
#include "miscfuncs.h"
#include <imagehlp.h>
#include <stdlib.h>
#include <syslog.h>
#include <wchar.h>

#include "cygtls.h"
#include "pinfo.h"
#include "sigproc.h"
#include "shared_info.h"
#include "perprocess.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"
#include "ntdll.h"
#include "exception.h"

/* Definitions for code simplification */
#ifdef __x86_64__
# define _GR(reg)	R ## reg
# define _AFMT		"%011X"
# define _ADDR		DWORD64
#else
# define _GR(reg)	E ## reg
# define _AFMT		"%08x"
# define _ADDR		DWORD
#endif

#define CALL_HANDLER_RETRY_OUTER 10
#define CALL_HANDLER_RETRY_INNER 10

char debugger_command[2 * NT_MAX_PATH + 20];

static BOOL WINAPI ctrl_c_handler (DWORD);

static const struct
{
  NTSTATUS code;
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

void
cygwin_exception::open_stackdumpfile ()
{
  /* If we have no executable name, or if the CWD handle is NULL,
     which means, the CWD is a virtual path, don't even try to open
     a stackdump file. */
  if (myself->progname[0] && cygheap->cwd.get_handle ())
    {
      const WCHAR *p;
      /* write to progname.stackdump if possible */
      if (!myself->progname[0])
	p = L"unknown";
      else if ((p = wcsrchr (myself->progname, L'\\')))
	p++;
      else
	p = myself->progname;

      WCHAR corefile[wcslen (p) + sizeof (".stackdump")];
      wcpcpy (wcpcpy(corefile, p), L".stackdump");
      UNICODE_STRING ucore;
      OBJECT_ATTRIBUTES attr;
      /* Create the UNICODE variation of <progname>.stackdump. */
      RtlInitUnicodeString (&ucore, corefile);
      /* Create an object attribute which refers to <progname>.stackdump
	 in Cygwin's cwd.  Stick to caseinsensitivity. */
      InitializeObjectAttributes (&attr, &ucore, OBJ_CASE_INSENSITIVE,
				  cygheap->cwd.get_handle (), NULL);
      IO_STATUS_BLOCK io;
      NTSTATUS status;
      /* Try to open it to dump the stack in it. */
      status = NtCreateFile (&h, GENERIC_WRITE | SYNCHRONIZE, &attr, &io,
			     NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF,
			     FILE_SYNCHRONOUS_IO_NONALERT
			     | FILE_OPEN_FOR_BACKUP_INTENT, NULL, 0);
      if (NT_SUCCESS (status))
	{
	  if (!myself->cygstarted)
	    system_printf ("Dumping stack trace to %S", &ucore);
	  else
	    debug_printf ("Dumping stack trace to %S", &ucore);
	  SetStdHandle (STD_ERROR_HANDLE, h);
	}
    }
}

/* Utilities for dumping the stack, etc.  */

void
cygwin_exception::dump_exception ()
{
  const char *exception_name = NULL;

  if (e)
    {
      for (int i = 0; status_info[i].name; i++)
	{
	  if (status_info[i].code == (NTSTATUS) e->ExceptionCode)
	    {
	      exception_name = status_info[i].name;
	      break;
	    }
	}
    }

#ifdef __x86_64__
  if (exception_name)
    small_printf ("Exception: %s at rip=%011X\r\n", exception_name, ctx->Rip);
  else
    small_printf ("Signal %d at rip=%011X\r\n", e->ExceptionCode, ctx->Rip);
  small_printf ("rax=%016X rbx=%016X rcx=%016X\r\n", ctx->Rax, ctx->Rbx, ctx->Rcx);
  small_printf ("rdx=%016X rsi=%016X rdi=%016X\r\n", ctx->Rdx, ctx->Rsi, ctx->Rdi);
  small_printf ("r8 =%016X r9 =%016X r10=%016X\r\n", ctx->R8, ctx->R9, ctx->R10);
  small_printf ("r11=%016X r12=%016X r13=%016X\r\n", ctx->R11, ctx->R12, ctx->R13);
  small_printf ("r14=%016X r15=%016X\r\n", ctx->R14, ctx->R15);
  small_printf ("rbp=%016X rsp=%016X\r\n", ctx->Rbp, ctx->Rsp);
  small_printf ("program=%W, pid %u, thread %s\r\n",
		myself->progname, myself->pid, cygthread::name ());
#else
  if (exception_name)
    small_printf ("Exception: %s at eip=%08x\r\n", exception_name, ctx->Eip);
  else
    small_printf ("Signal %d at eip=%08x\r\n", e->ExceptionCode, ctx->Eip);
  small_printf ("eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\r\n",
		ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx, ctx->Esi, ctx->Edi);
  small_printf ("ebp=%08x esp=%08x program=%W, pid %u, thread %s\r\n",
		ctx->Ebp, ctx->Esp, myself->progname, myself->pid,
		cygthread::name ());
#endif
  small_printf ("cs=%04x ds=%04x es=%04x fs=%04x gs=%04x ss=%04x\r\n",
		ctx->SegCs, ctx->SegDs, ctx->SegEs, ctx->SegFs, ctx->SegGs, ctx->SegSs);
}

/* A class for manipulating the stack. */
class stack_info
{
  int walk ();			/* Uses the "old" method */
  char *next_offset () {return *((char **) sf.AddrFrame.Offset);}
  bool needargs;
  PUINT_PTR dummy_frame;
#ifdef __x86_64__
  CONTEXT c;
  UNWIND_HISTORY_TABLE hist;
#endif
public:
  STACKFRAME sf;		 /* For storing the stack information */
  void init (PUINT_PTR, bool, PCONTEXT); /* Called the first time that stack info is needed */

  /* Postfix ++ iterates over the stack, returning zero when nothing is left. */
  int operator ++(int) { return walk (); }
};

/* The number of parameters used in STACKFRAME */
#define NPARAMS (sizeof (thestack.sf.Params) / sizeof (thestack.sf.Params[0]))

/* This is the main stack frame info for this process. */
static NO_COPY stack_info thestack;

/* Initialize everything needed to start iterating. */
void
stack_info::init (PUINT_PTR framep, bool wantargs, PCONTEXT ctx)
{
#ifdef __x86_64__
  memset (&hist, 0, sizeof hist);
  if (ctx)
    memcpy (&c, ctx, sizeof c);
  else
    {
      memset (&c, 0, sizeof c);
      c.ContextFlags = CONTEXT_ALL;
    }
#endif
  memset (&sf, 0, sizeof (sf));
  if (ctx)
    sf.AddrFrame.Offset = (UINT_PTR) framep;
  else
    {
      dummy_frame = framep;
      sf.AddrFrame.Offset = (UINT_PTR) &dummy_frame;
    }
  if (framep)
    sf.AddrReturn.Offset = framep[1];
  sf.AddrFrame.Mode = AddrModeFlat;
  needargs = wantargs;
}

extern "C" void _cygwin_exit_return ();

/* Walk the stack by looking at successive stored 'bp' frames.
   This is not foolproof. */
int
stack_info::walk ()
{
#ifdef __x86_64__
  PRUNTIME_FUNCTION f;
  ULONG64 imagebase;
  DWORD64 establisher;
  PVOID hdl;

  if (!c.Rip)
    return 0;

  sf.AddrPC.Offset = c.Rip;
  sf.AddrStack.Offset = c.Rsp;
  sf.AddrFrame.Offset = c.Rbp;

  f = RtlLookupFunctionEntry (c.Rip, &imagebase, &hist);
  if (f)
    RtlVirtualUnwind (0, imagebase, c.Rip, f, &c, &hdl, &establisher, NULL);
  else
    {
      c.Rip = *(ULONG_PTR *) c.Rsp;
      c.Rsp += 8;
    }
  if (needargs && c.Rip)
    {
      PULONG_PTR p = (PULONG_PTR) c.Rsp;
      for (unsigned i = 0; i < NPARAMS; ++i)
	sf.Params[i] = p[i + 1];
    }
  return 1;
#else
  char **framep;

  if ((void (*) ()) sf.AddrPC.Offset == _cygwin_exit_return)
    return 0;		/* stack frames are exhausted */

  if (((framep = (char **) next_offset ()) == NULL)
      || (framep >= (char **) cygwin_hmodule))
    return 0;

  sf.AddrFrame.Offset = (_ADDR) framep;
  sf.AddrPC.Offset = sf.AddrReturn.Offset;

  /* The return address always follows the stack pointer */
  sf.AddrReturn.Offset = (_ADDR) *++framep;

  if (needargs)
    {
      unsigned nparams = NPARAMS;

      /* The arguments follow the return address */
      sf.Params[0] = (_ADDR) *++framep;
      /* Hack for XP/2K3 WOW64.  If the first stack param points to the
	 application entry point, we can only fetch one additional
	 parameter.  Accessing anything beyond this address results in
	 a SEGV.  This is fixed in Vista/2K8 WOW64. */
      if (wincap.has_restricted_stack_args () && sf.Params[0] == 0x401000)
	nparams = 2;
      for (unsigned i = 1; i < nparams; i++)
	sf.Params[i] = (_ADDR) *++framep;
    }
  return 1;
#endif
}

void
cygwin_exception::dumpstack ()
{
  static bool already_dumped;
  myfault efault;
  if (efault.faulted ())
    return;

  if (already_dumped || cygheap->rlim_core == 0Ul)
    return;
  already_dumped = true;
  open_stackdumpfile ();

  if (e)
    dump_exception ();

  int i;

  thestack.init (framep, 1, ctx);	/* Initialize from the input CONTEXT */
#ifdef __x86_64__
  small_printf ("Stack trace:\r\nFrame        Function    Args\r\n");
#else
  small_printf ("Stack trace:\r\nFrame     Function  Args\r\n");
#endif
  for (i = 0; i < 16 && thestack++; i++)
    {
      small_printf (_AFMT "  " _AFMT, thestack.sf.AddrFrame.Offset,
		    thestack.sf.AddrPC.Offset);
      for (unsigned j = 0; j < NPARAMS; j++)
	small_printf ("%s" _AFMT, j == 0 ? " (" : ", ", thestack.sf.Params[j]);
      small_printf (")\r\n");
    }
  small_printf ("End of stack trace%s\n",
	      i == 16 ? " (more stack frames may be present)" : "");
  if (h)
    NtClose (h);
}

bool
_cygtls::inside_kernel (CONTEXT *cx)
{
  int res;
  MEMORY_BASIC_INFORMATION m;

  if (!isinitialized ())
    return true;

  memset (&m, 0, sizeof m);
  if (!VirtualQuery ((LPCVOID) cx->_GR(ip), &m, sizeof m))
    sigproc_printf ("couldn't get memory info, pc %p, %E", cx->_GR(ip));

  size_t size = (windows_system_directory_length + 6) * sizeof (WCHAR);
  PWCHAR checkdir = (PWCHAR) alloca (size);
  memset (checkdir, 0, size);

# define h ((HMODULE) m.AllocationBase)
  if (!h || m.State != MEM_COMMIT)	/* Be defensive */
    res = true;
  else if (h == hntdll)
    res = true;				/* Calling GetModuleFilename on ntdll.dll
					   can hang */
  else if (h == user_data->hmodule)
    res = false;
  else if (!GetModuleFileNameW (h, checkdir, windows_system_directory_length + 6))
    res = false;
  else
    {
      /* Skip potential long path prefix. */
      if (!wcsncmp (checkdir, L"\\\\?\\", 4))
	checkdir += 4;
      res = wcsncasecmp (windows_system_directory, checkdir,
			 windows_system_directory_length) == 0;
#ifndef __x86_64__
      if (!res && system_wow64_directory_length)
	res = wcsncasecmp (system_wow64_directory, checkdir,
			   system_wow64_directory_length) == 0;

#endif
    }
  sigproc_printf ("pc %p, h %p, inside_kernel %d", cx->_GR(ip), h, res);
# undef h
  return res;
}

/* Temporary (?) function for external callers to get a stack dump */
extern "C" void
cygwin_stackdump ()
{
  CONTEXT c;
  c.ContextFlags = CONTEXT_FULL;
  RtlCaptureContext (&c);
  cygwin_exception exc ((PUINT_PTR) c._GR(bp), &c);
  exc.dumpstack ();
}

#define TIME_TO_WAIT_FOR_DEBUGGER 10000

extern "C" int
try_to_debug (bool waitloop)
{
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

  /* If the tty mutex is owned, we will fail to start any cygwin app
     until the trapped app exits.  However, this will only release any
     the mutex if it is owned by this thread so that may be problematic. */

  lock_ttys::release ();

  /* prevent recursive exception handling */
  PWCHAR rawenv = GetEnvironmentStringsW () ;
  for (PWCHAR p = rawenv; *p != L'\0'; p = wcschr (p, L'\0') + 1)
    {
      if (wcsncmp (p, L"CYGWIN=", wcslen (L"CYGWIN=")) == 0)
	{
	  PWCHAR q = wcsstr (p, L"error_start") ;
	  /* replace 'error_start=...' with '_rror_start=...' */
	  if (q)
	    {
	      *q = L'_' ;
	      SetEnvironmentVariableW (L"CYGWIN", p + wcslen (L"CYGWIN=")) ;
	    }
	  break;
	}
    }
  FreeEnvironmentStringsW (rawenv);

  console_printf ("*** starting debugger for pid %u, tid %u\n",
		  cygwin_pid (GetCurrentProcessId ()), GetCurrentThreadId ());
  BOOL dbg;
  WCHAR dbg_cmd[strlen(debugger_command) + 1];
  sys_mbstowcs (dbg_cmd, strlen(debugger_command) + 1, debugger_command);
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
	Sleep (1);
      Sleep (2000);
    }

  console_printf ("*** continuing pid %u from debugger call (%d)\n",
		  cygwin_pid (GetCurrentProcessId ()), dbg);

  SetThreadPriority (GetCurrentThread (), prio);
  return dbg;
}

#ifdef __x86_64__
/* Don't unwind the stack on x86_64.  It's not necessary to do that from the
   exception handler. */
#define rtl_unwind(el,er)
#else
static void __reg3 rtl_unwind (exception_list *, PEXCEPTION_RECORD) __attribute__ ((noinline, regparm (3)));
void __reg3
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
#endif /* __x86_64 */

#ifdef __x86_64__
/* myfault vectored exception handler */
LONG
exception::myfault_handle (LPEXCEPTION_POINTERS ep)
{
  _cygtls& me = _my_tls;

  if (me.andreas)
    {
      /* Only handle the minimum amount of exceptions the myfault handler
	 was designed for. */
      switch (ep->ExceptionRecord->ExceptionCode)
	{
	case STATUS_ACCESS_VIOLATION:
	case STATUS_DATATYPE_MISALIGNMENT:
	case STATUS_STACK_OVERFLOW:
	case STATUS_ARRAY_BOUNDS_EXCEEDED:
	  me.andreas->leave ();	/* Return from a "san" caught fault */
	default:
	  break;
	}
    }
  return EXCEPTION_CONTINUE_SEARCH;
}
#endif /* __x86_64 */

/* Main exception handler. */
int
exception::handle (EXCEPTION_RECORD *e, exception_list *frame, CONTEXT *in, void *)
{
  static bool NO_COPY debugging;
  _cygtls& me = _my_tls;

#ifndef __x86_64__
  if (me.andreas)
    me.andreas->leave ();	/* Return from a "san" caught fault */
#endif

  if (debugging && ++debugging < 500000)
    {
      SetThreadPriority (hMainThread, THREAD_PRIORITY_NORMAL);
      return ExceptionContinueExecution;
    }

  /* If we're exiting, tell Windows to keep looking for an
     exception handler.  */
  if (exit_state || e->ExceptionFlags)
    return ExceptionContinueSearch;

  siginfo_t si = {};
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
	  return ExceptionContinueExecution;
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
      return ExceptionContinueExecution;

    default:
      /* If we don't recognize the exception, we have to assume that
	 we are doing structured exception handling, and we let
	 something else handle it.  */
      return ExceptionContinueSearch;
    }

  debug_printf ("In cygwin_except_handler exception %y at %p sp %p", e->ExceptionCode, in->_GR(ip), in->_GR(sp));
  debug_printf ("In cygwin_except_handler signal %d at %p", si.si_signo, in->_GR(ip));

#ifdef __x86_64__
  PUINT_PTR framep = (PUINT_PTR) in->Rbp;
  /* Sometimes, when a stack is screwed up, Rbp tends to be NULL.  In that
     case, base the stacktrace on Rsp.  In most cases, it allows to generate
     useful stack trace. */
  if (!framep)
    framep = (PUINT_PTR) in->Rsp;
#else
  PUINT_PTR framep = (PUINT_PTR) in->_GR(sp);
  for (PUINT_PTR bpend = (PUINT_PTR) __builtin_frame_address (0); framep > bpend; framep--)
    if (*framep == in->SegCs && framep[-1] == in->_GR(ip))
      {
	framep -= 2;
	break;
      }

  /* Temporarily replace windows top level SEH with our own handler.
     We don't want any Windows magic kicking in.  This top level frame
     will be removed automatically after our exception handler returns. */
  _except_list->handler = handle;
#endif

  if (exit_state >= ES_SIGNAL_EXIT
      && (NTSTATUS) e->ExceptionCode != STATUS_CONTROL_C_EXIT)
    api_fatal ("Exception during process exit");
  else if (!try_to_debug (0))
    rtl_unwind (frame, e);
  else
    {
      debugging = true;
      return ExceptionContinueExecution;
    }

  /* FIXME: Probably should be handled in signal processing code */
  if ((NTSTATUS) e->ExceptionCode == STATUS_ACCESS_VIOLATION)
    {
      int error_code = 0;
      if (si.si_code == SEGV_ACCERR)	/* Address present */
	error_code |= 1;
      if (e->ExceptionInformation[0])	/* Write access */
	error_code |= 2;
      if (!me.inside_kernel (in))	/* User space */
	error_code |= 4;
      klog (LOG_INFO,
#ifdef __x86_64__
	    "%s[%d]: segfault at %011X rip %011X rsp %011X error %d",
#else
	    "%s[%d]: segfault at %08x rip %08x rsp %08x error %d",
#endif
		      __progname, myself->pid,
		      e->ExceptionInformation[1], in->_GR(ip), in->_GR(sp),
		      error_code);
    }
  cygwin_exception exc (framep, in, e);
  si.si_cyg = (void *) &exc;
  /* POSIX requires that for SIGSEGV and SIGBUS, si_addr should be set to the
     address of faulting memory reference.  For SIGILL and SIGFPE these should
     be the address of the faulting instruction.  Other signals are apparently
     undefined so we just set those to the faulting instruction too.  */ 
  si.si_addr = (si.si_signo == SIGSEGV || si.si_signo == SIGBUS)
	       ? (void *) e->ExceptionInformation[1] : (void *) in->_GR(ip);
  me.incyg++;
  sig_send (NULL, si, &me);	/* Signal myself */
  me.incyg--;
  e->ExceptionFlags = 0;
  return ExceptionContinueExecution;
}

/* Utilities to call a user supplied exception handler.  */

#define SIG_NONMASKABLE	(SIGTOMASK (SIGKILL) | SIGTOMASK (SIGSTOP))

/* Non-raceable sigsuspend
   Note: This implementation is based on the Single UNIX Specification
   man page.  This indicates that sigsuspend always returns -1 and that
   attempts to block unblockable signals will be silently ignored.
   This is counter to what appears to be documented in some UNIX
   man pages, e.g. Linux.  */
int __stdcall
handle_sigsuspend (sigset_t tempmask)
{
  sigset_t oldmask = _my_tls.sigmask;	// Remember for restoration

  set_signal_mask (_my_tls.sigmask, tempmask);
  sigproc_printf ("oldmask %ly, newmask %ly", oldmask, tempmask);

  pthread_testcancel ();
  cygwait (NULL, cw_infinite, cw_cancel | cw_cancel_self | cw_sig_eintr);

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
sig_handle_tty_stop (int sig, siginfo_t *, void *)
{
  /* Silently ignore attempts to suspend if there is no accommodating
     cygwin parent to deal with this behavior. */
  if (!myself->cygstarted)
    myself->process_state &= ~PID_STOPPED;
  else
    {
      _my_tls.incyg = 1;
      myself->stopsig = sig;
      myself->alert_parent (sig);
      sigproc_printf ("process %d stopped by signal %d", myself->pid, sig);
      /* FIXME! This does nothing to suspend anything other than the main
	 thread. */
      /* Use special cygwait parameter to handle SIGCONT.  _main_tls.sig will
	 be cleared under lock when SIGCONT is detected.  */
      DWORD res = cygwait (NULL, cw_infinite, cw_sig_cont);
      switch (res)
	{
	case WAIT_SIGNALED:
	  myself->stopsig = SIGCONT;
	  myself->alert_parent (SIGCONT);
	  break;
	default:
	  api_fatal ("WaitSingleObject returned %d", res);
	  break;
	}
      _my_tls.incyg = 0;
    }
}
} /* end extern "C" */

bool
_cygtls::interrupt_now (CONTEXT *cx, siginfo_t& si, void *handler,
			struct sigaction& siga)
{
  bool interrupted;

  /* Delay the interrupt if we are
     1) somehow inside the DLL
     2) in _sigfe (spinning is true) and about to enter cygwin DLL
     3) in a Windows DLL.  */
  if (incyg || spinning || inside_kernel (cx))
    interrupted = false;
  else
    {
      _ADDR &ip = cx->_GR(ip);
      push (ip);
      interrupt_setup (si, handler, siga);
      ip = pop ();
      SetThreadContext (*this, cx); /* Restart the thread in a new location */
      interrupted = true;
    }
  return interrupted;
}

void __reg3
_cygtls::interrupt_setup (siginfo_t& si, void *handler, struct sigaction& siga)
{
  push ((__stack_t) sigdelayed);
  deltamask = siga.sa_mask & ~SIG_NONMASKABLE;
  sa_flags = siga.sa_flags;
  func = (void (*) (int, siginfo_t *, void *)) handler;
  if (siga.sa_flags & SA_RESETHAND)
    siga.sa_handler = SIG_DFL;
  saved_errno = -1;		// Flag: no errno to save
  if (handler == sig_handle_tty_stop)
    {
      myself->stopsig = 0;
      myself->process_state |= PID_STOPPED;
    }

  infodata = si;
  this->sig = si.si_signo;		// Should always be last thing set to avoid a race

  if (incyg)
    SetEvent (get_signal_arrived (false));

  if (!have_execed)
    proc_subproc (PROC_CLEARWAIT, 1);
  sigproc_printf ("armed signal_arrived %p, signal %d", signal_arrived, si.si_signo);
}

extern "C" void __stdcall
set_sig_errno (int e)
{
  *_my_tls.errno_addr = e;
  _my_tls.saved_errno = e;
}

int
sigpacket::setup_handler (void *handler, struct sigaction& siga, _cygtls *tls)
{
  CONTEXT cx;
  bool interrupted = false;

  if (tls->sig)
    {
      sigproc_printf ("trying to send signal %d but signal %d already armed",
		      si.si_signo, tls->sig);
      goto out;
    }

  for (int n = 0; n < CALL_HANDLER_RETRY_OUTER; n++)
    {
      for (int i = 0; i < CALL_HANDLER_RETRY_INNER; i++)
	{
	  tls->lock ();
	  if (tls->incyg)
	    {
	      sigproc_printf ("controlled interrupt. stackptr %p, stack %p, stackptr[-1] %p",
			      tls->stackptr, tls->stack, tls->stackptr[-1]);
	      tls->interrupt_setup (si, handler, siga);
	      interrupted = true;
	      tls->unlock ();
	      goto out;
	    }

	  DWORD res;
	  HANDLE hth = (HANDLE) *tls;
	  if (!hth)
	    {
	      tls->unlock ();
	      sigproc_printf ("thread handle NULL, not set up yet?");
	    }
	  else
	    {
	      /* Suspend the thread which will receive the signal.
		 If one of these conditions is not true we loop.
		 If the thread is already suspended (which can occur when a program
		 has called SuspendThread on itself) then just queue the signal. */

	      sigproc_printf ("suspending thread, tls %p, _main_tls %p", tls, _main_tls);
	      res = SuspendThread (hth);
	      /* Just set pending if thread is already suspended */
	      if (res)
		{
		  tls->unlock ();
		  ResumeThread (hth);
		  goto out;
		}
	      cx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
	      if (!GetThreadContext (hth, &cx))
		sigproc_printf ("couldn't get context of thread, %E");
	      else
		interrupted = tls->interrupt_now (&cx, si, handler, siga);

	      tls->unlock ();
	      ResumeThread (hth);
	      if (interrupted)
		goto out;
	    }

	  sigproc_printf ("couldn't interrupt.  trying again.");
	  yield ();
	}
      /* Hit here if we couldn't deliver the signal.  Take a more drastic
	 action before trying again. */
      Sleep (1);
    }

out:
  sigproc_printf ("signal %d %sdelivered", si.si_signo, interrupted ? "" : "not ");
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
      && GetUserObjectInformationW (station_hdl, UOI_FLAGS, &uof,
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

  /* Remove early or we could overthrow the threadlist in cygheap.
     Deleting this line causes ash to SEGV if CTRL-C is hit repeatedly.
     I am not exactly sure why that is.  Maybe it's just because this
     adds some early serialization to ctrl_c_handler which prevents
     multiple simultaneous calls? */
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
	      sig_send (myself, SIGHUP);
	      return TRUE;
	    }
	  return FALSE;
	}
    }

  if (ch_spawn.set_saw_ctrl_c ())
    return TRUE;

  /* We're only the process group leader when we have a valid pinfo structure.
     If we don't have one, then the parent "stub" will handle the signal. */
  if (!pinfo (cygwin_pid (GetCurrentProcessId ())))
    return TRUE;

  tty_min *t = cygwin_shared->tty.get_cttyp ();
  /* Ignore this if we're not the process group leader since it should be handled
     *by* the process group leader. */
  if (t && (!have_execed || have_execed_cygwin)
      && t->getpgid () == myself->pid &&
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
      t->kill_pgrp (sig);
      t->last_ctrl_c = GetTickCount ();
      return TRUE;
    }

  return TRUE;
}

/* Function used by low level sig wrappers. */
extern "C" void __stdcall
set_process_mask (sigset_t newmask)
{
  set_signal_mask (_my_tls.sigmask, newmask);
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
  sigset_t mask = _my_tls.sigmask;
  sigaddset (&mask, sig);
  set_signal_mask (_my_tls.sigmask, mask);
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
  sigset_t mask = _my_tls.sigmask;
  sigdelset (&mask, sig);
  set_signal_mask (_my_tls.sigmask, mask);
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
  set_signal_mask (_my_tls.sigmask, mask);
  return prev;
}

extern "C" int
sigignore (int sig)
{
  return sigset (sig, SIG_IGN) == SIG_ERR ? -1 : 0;
}

/* Update the signal mask for this process and return the old mask.
   Called from call_signal_handler */
extern "C" sigset_t
set_process_mask_delta ()
{
  sigset_t newmask, oldmask;

  if (_my_tls.deltamask & SIG_NONMASKABLE)
    oldmask = _my_tls.oldmask; /* from handle_sigsuspend */
  else
    oldmask = _my_tls.sigmask;
  newmask = (oldmask | _my_tls.deltamask) & ~SIG_NONMASKABLE;
  sigproc_printf ("oldmask %lx, newmask %lx, deltamask %lx", oldmask, newmask,
		  _my_tls.deltamask);
  _my_tls.sigmask = newmask;
  return oldmask;
}

/* Set the signal mask for this process.
   Note that some signals are unmaskable, as in UNIX.  */

void
set_signal_mask (sigset_t& setmask, sigset_t newmask)
{
  newmask &= ~SIG_NONMASKABLE;
  sigset_t mask_bits = setmask & ~newmask;
  sigproc_printf ("setmask %lx, newmask %lx, mask_bits %lx", setmask, newmask,
		  mask_bits);
  setmask = newmask;
  if (mask_bits)
    sig_dispatch_pending (true);
}

/* Exit due to a signal.  Should only be called from the signal thread.  */
extern "C" {
static void
signal_exit (int sig, siginfo_t *si)
{
  debug_printf ("exiting due to signal %d", sig);
  exit_state = ES_SIGNAL_EXIT;

  if (cygheap->rlim_core > 0UL)
    switch (sig)
      {
      case SIGABRT:
      case SIGBUS:
      case SIGFPE:
      case SIGILL:
      case SIGQUIT:
      case SIGSEGV:
      case SIGSYS:
      case SIGTRAP:
      case SIGXCPU:
      case SIGXFSZ:
	sig |= 0x80;		/* Flag that we've "dumped core" */
	if (try_to_debug ())
	  break;
	if (si->si_code != SI_USER && si->si_cyg)
	  ((cygwin_exception *) si->si_cyg)->dumpstack ();
	else
	  {
	    CONTEXT c;
	    c.ContextFlags = CONTEXT_FULL;
#ifdef __x86_64__
	    RtlCaptureContext (&c);
	    cygwin_exception exc ((PUINT_PTR) __builtin_frame_address (0), &c);
#else
	    GetThreadContext (GetCurrentThread (), &c);
	    cygwin_exception exc ((PUINT_PTR) __builtin_frame_address (0), &c);
#endif
	    exc.dumpstack ();
	  }
	break;
      }

  lock_process until_exit (true);

  if (have_execed || exit_state > ES_PROCESS_LOCKED)
    {
      debug_printf ("recursive exit?");
      myself.exit (sig);
    }

  /* Starve other threads in a vain attempt to stop them from doing something
     stupid. */
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);

  sigproc_printf ("about to call do_exit (%x)", sig);
  do_exit (sig);
}
} /* extern "C" */

/* Attempt to carefully handle SIGCONT when we are stopped. */
void
_cygtls::handle_SIGCONT ()
{
  if (NOTSTATE (myself, PID_STOPPED))
    return;

  myself->stopsig = 0;
  myself->process_state &= ~PID_STOPPED;
  /* Carefully tell sig_handle_tty_stop to wake up.
     Make sure that any pending signal is handled before trying to
     send a new one.  Then make sure that SIGCONT has been recognized
     before exiting the loop.  */
  bool sigsent = false;
  while (1)
    if (sig)		/* Assume that it's ok to just test sig outside of a
			   lock since setup_handler does it this way.  */
      yield ();		/* Attempt to schedule another thread.  */
    else if (sigsent)
      break;		/* SIGCONT has been recognized by other thread */
    else
      {
	sig = SIGCONT;
	SetEvent (signal_arrived); /* alert sig_handle_tty_stop */
	sigsent = true;
      }
  /* Clear pending stop signals */
  sig_clear (SIGSTOP);
  sig_clear (SIGTSTP);
  sig_clear (SIGTTIN);
  sig_clear (SIGTTOU);
}

int __reg1
sigpacket::process ()
{
  int rc = 1;
  bool issig_wait = false;
  struct sigaction& thissig = global_sigs[si.si_signo];
  void *handler = have_execed ? NULL : (void *) thissig.sa_handler;

  /* Don't try to send signals if we're just starting up since signal masks
     may not be available.  */
  if (!cygwin_finished_initializing)
    {
      rc = -1;
      goto done;
    }

  sigproc_printf ("signal %d processing", si.si_signo);

  myself->rusage_self.ru_nsignals++;

  _cygtls *tls;
  if (si.si_signo == SIGCONT)
    _main_tls->handle_SIGCONT ();

  if (si.si_signo == SIGKILL)
    tls = _main_tls;	/* SIGKILL is special.  It always goes through.  */
  else if (ISSTATE (myself, PID_STOPPED))
    {
      rc = -1;		/* Don't send signals when stopped */
      goto done;
    }
  else if (!sigtls)
    {
      tls = cygheap->find_tls (si.si_signo, issig_wait);
      sigproc_printf ("using tls %p", tls);
    }
  else
    {
      tls = sigtls;
      if (sigismember (&tls->sigwait_mask, si.si_signo))
	issig_wait = true;
      else if (!sigismember (&tls->sigmask, si.si_signo))
	issig_wait = false;
      else
	tls = NULL;
    }
      
  /* !tls means no threads available to catch a signal. */
  if (!tls)
    {
      sigproc_printf ("signal %d blocked", si.si_signo);
      rc = -1;
      goto done;
    }

  /* Do stuff for gdb */
  if ((HANDLE) *tls)
    tls->signal_debugger (si);

  if (issig_wait)
    {
      tls->sigwait_mask = 0;
      goto dosig;
    }

  if (handler == SIG_IGN)
    {
      sigproc_printf ("signal %d ignored", si.si_signo);
      goto done;
    }

  if (si.si_signo == SIGKILL)
    goto exit_sig;
  if (si.si_signo == SIGSTOP)
    {
      sig_clear (SIGCONT);
      goto stop;
    }

  /* Clear pending SIGCONT on stop signals */
  if (si.si_signo == SIGTSTP || si.si_signo == SIGTTIN || si.si_signo == SIGTTOU)
    sig_clear (SIGCONT);

  if (handler == (void *) SIG_DFL)
    {
      if (si.si_signo == SIGCHLD || si.si_signo == SIGIO || si.si_signo == SIGCONT || si.si_signo == SIGWINCH
	  || si.si_signo == SIGURG)
	{
	  sigproc_printf ("signal %d default is currently ignore", si.si_signo);
	  goto done;
	}

      if (si.si_signo == SIGTSTP || si.si_signo == SIGTTIN || si.si_signo == SIGTTOU)
	goto stop;

      goto exit_sig;
    }

  if (handler == (void *) SIG_ERR)
    goto exit_sig;

  goto dosig;

stop:
  tls = _main_tls;
  handler = (void *) sig_handle_tty_stop;
  thissig = global_sigs[SIGSTOP];
  goto dosig;

exit_sig:
  handler = (void *) signal_exit;
  thissig.sa_flags |= SA_SIGINFO;

dosig:
  if (have_execed)
    {
      sigproc_printf ("terminating captive process");
      TerminateProcess (ch_spawn, sigExeced = si.si_signo);
    }
  /* Dispatch to the appropriate function. */
  sigproc_printf ("signal %d, signal handler %p", si.si_signo, handler);
  rc = setup_handler (handler, thissig, tls);

done:
  sigproc_printf ("returning %d", rc);
  return rc;

}

int
_cygtls::call_signal_handler ()
{
  int this_sa_flags = SA_RESTART;
  while (1)
    {
      lock ();
      if (!sig)
	{
	  unlock ();
	  break;
	}

      /* Pop the stack if the next "return address" is sigdelayed, since
	 this function is doing what sigdelayed would have done anyway. */
      if (retaddr () == (__stack_t) sigdelayed)
	pop ();

      debug_only_printf ("dealing with signal %d", sig);
      this_sa_flags = sa_flags;

      /* Save information locally on stack to pass to handler. */
      int thissig = sig;
      siginfo_t thissi = infodata;
      void (*thisfunc) (int, siginfo_t *, void *) = func;

      sigset_t this_oldmask = set_process_mask_delta ();
      int this_errno = saved_errno;
      reset_signal_arrived ();
      incyg = false;
      sig = 0;		/* Flag that we can accept another signal */
      unlock ();	/* unlock signal stack */

      /* no ucontext_t information provided yet, so third arg is NULL */
      thisfunc (thissig, &thissi, NULL);
      incyg = true;

      set_signal_mask (_my_tls.sigmask, this_oldmask);
      if (this_errno >= 0)
	set_errno (this_errno);
    }

  return this_sa_flags & SA_RESTART || (this != _main_tls);
}

void
_cygtls::signal_debugger (siginfo_t& si)
{
  HANDLE th;
  /* If si.si_cyg is set then the signal was already sent to the debugger. */
  if (isinitialized () && !si.si_cyg && (th = (HANDLE) *this)
      && being_debugged () && SuspendThread (th) >= 0)
    {
      CONTEXT c;
      c.ContextFlags = CONTEXT_FULL;
      if (GetThreadContext (th, &c))
	{
	  if (incyg)
#ifdef __x86_64__
	    c.Rip = retaddr ();
#else
	    c.Eip = retaddr ();
#endif
	  memcpy (&thread_context, &c, (&thread_context._internal -
					(unsigned char *) &thread_context));
	  /* Enough space for 32/64 bit addresses */
	  char sigmsg[2 * sizeof (_CYGWIN_SIGNAL_STRING " ffffffff ffffffffffffffff")];
	  __small_sprintf (sigmsg, _CYGWIN_SIGNAL_STRING " %d %y %p", si.si_signo,
			   thread_id, &thread_context);
	  OutputDebugString (sigmsg);
	}
      ResumeThread (th);
    }
}
