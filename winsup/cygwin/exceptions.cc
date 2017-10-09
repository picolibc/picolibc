/* exceptions.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define CYGTLS_HANDLE
#include "winsup.h"
#include "miscfuncs.h"
#include <imagehlp.h>
#include <stdlib.h>
#include <stdarg.h>
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

PWCHAR debugger_command;
extern uint8_t _sigbe;
extern uint8_t _sigdelayed_end;

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
    return;
  if (!debugger_command &&
      !(debugger_command = (PWCHAR) malloc ((2 * NT_MAX_PATH + 20)
					    * sizeof (WCHAR))))
    return;

  PWCHAR cp = debugger_command
	      + sys_mbstowcs (debugger_command, NT_MAX_PATH, buf) - 1;
  cp = wcpcpy (cp, L" \"");
  wcpcpy (cp, global_progname);
  for (PWCHAR p = wcschr (cp, L'\\'); p; p = wcschr (p, L'\\'))
    *p = L'/';
  wcscat (cp, L"\"");
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

  for (int i = 0; status_info[i].name; i++)
    {
      if (status_info[i].code == (NTSTATUS) e->ExceptionCode)
	{
	  exception_name = status_info[i].name;
	  break;
	}
    }

#ifdef __x86_64__
  if (exception_name)
    small_printf ("Exception: %s at rip=%011X\r\n", exception_name, ctx->Rip);
  else
    small_printf ("Signal %d at rip=%011X\r\n", e->ExceptionCode, ctx->Rip);
  small_printf ("rax=%016X rbx=%016X rcx=%016X\r\n",
		ctx->Rax, ctx->Rbx, ctx->Rcx);
  small_printf ("rdx=%016X rsi=%016X rdi=%016X\r\n",
		ctx->Rdx, ctx->Rsi, ctx->Rdi);
  small_printf ("r8 =%016X r9 =%016X r10=%016X\r\n",
		ctx->R8, ctx->R9, ctx->R10);
  small_printf ("r11=%016X r12=%016X r13=%016X\r\n",
		ctx->R11, ctx->R12, ctx->R13);
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
		ctx->SegCs, ctx->SegDs, ctx->SegEs, ctx->SegFs,
		ctx->SegGs, ctx->SegSs);
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
  __tlsstack_t *sigstackptr;
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
  sigstackptr = _my_tls.stackptr;
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

#ifdef __x86_64__
static inline void
__unwind_single_frame (PCONTEXT ctx)
{
  PRUNTIME_FUNCTION f;
  ULONG64 imagebase;
  UNWIND_HISTORY_TABLE hist = {0};
  DWORD64 establisher;
  PVOID hdl;

  f = RtlLookupFunctionEntry (ctx->Rip, &imagebase, &hist);
  if (f)
    RtlVirtualUnwind (0, imagebase, ctx->Rip, f, ctx, &hdl, &establisher,
		      NULL);
  else
    {
      ctx->Rip = *(ULONG_PTR *) ctx->Rsp;
      ctx->Rsp += 8;
    }
}
#else
#define __unwind_single_frame(ctx)
#endif

/* Walk the stack.

   On 32 bit we're doing this by looking at successive stored 'ebp' frames.
   This is not foolproof. */
int
stack_info::walk ()
{
#ifdef __x86_64__
  if (!c.Rip)
    return 0;

  sf.AddrPC.Offset = c.Rip;
  sf.AddrStack.Offset = c.Rsp;
  sf.AddrFrame.Offset = c.Rbp;

  if ((c.Rip >= (DWORD64)&_sigbe) && (c.Rip < (DWORD64)&_sigdelayed_end))
    {
      /* _sigbe and sigdelayed don't have SEH unwinding data, so virtually
         unwind the tls sigstack */
      c.Rip = sigstackptr[-1];
      sigstackptr--;
      return 1;
    }
  __unwind_single_frame (&c);
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

  __try
    {
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
	    small_printf ("%s" _AFMT, j == 0 ? " (" : ", ",
			  thestack.sf.Params[j]);
	  small_printf (")\r\n");
	}
      small_printf ("End of stack trace%s\n",
		    i == 16 ? " (more stack frames may be present)" : "");
      if (h)
	NtClose (h);
    }
  __except (NO_ERROR) {}
  __endtry
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
  else if (!GetModuleFileNameW (h, checkdir,
				windows_system_directory_length + 6))
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
  if (!debugger_command)
    return 0;
  debug_printf ("debugger_command '%W'", debugger_command);
  if (being_debugged ())
    {
      extern void break_here ();
      break_here ();
      return 0;
    }

  PWCHAR dbg_end = wcschr (debugger_command, L'\0');
  __small_swprintf (dbg_end, L" %u", GetCurrentProcessId ());

  LONG prio = GetThreadPriority (GetCurrentThread ());
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
  PROCESS_INFORMATION pi = {NULL, 0, 0, 0};

  STARTUPINFOW si = {0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		     NULL, NULL, NULL, NULL};
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
  dbg = CreateProcessW (NULL,
			debugger_command,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
			NULL,
			NULL,
			&si,
			&pi);

  *dbg_end = L'\0';
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
static void __reg3 rtl_unwind (exception_list *, PEXCEPTION_RECORD)
		   __attribute__ ((noinline, regparm (3)));

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
/* myfault exception handler. */
EXCEPTION_DISPOSITION
exception::myfault (EXCEPTION_RECORD *e, exception_list *frame, CONTEXT *in,
		    PDISPATCHER_CONTEXT dispatch)
{
  PSCOPE_TABLE table = (PSCOPE_TABLE) dispatch->HandlerData;
  RtlUnwindEx (frame,
	       (char *) dispatch->ImageBase + table->ScopeRecord[0].JumpTarget,
	       e, 0, in, dispatch->HistoryTable);
  /* NOTREACHED, make gcc happy. */
  return ExceptionContinueSearch;
}

/* If another exception occurs while running a signal handler on an alternate
   signal stack, the normal SEH handlers are skipped, because the OS exception
   handling considers the current (alternate) stack "broken".  However, it
   still calls vectored exception handlers.

   TODO: What we do here is to handle only __try/__except blocks in Cygwin.
         "Normal" exceptions will simply exit the process.  Still, better
	 than nothing... */
LONG WINAPI
myfault_altstack_handler (EXCEPTION_POINTERS *exc)
{
  _cygtls& me = _my_tls;

  if (me.andreas)
    {
      CONTEXT *c = exc->ContextRecord;

      /* Unwind the stack manually and call RtlRestoreContext.  This
	 is necessary because RtlUnwindEx checks the stack for validity,
	 which, as outlined above, fails for the alternate stack. */
      while (c->Rsp < me.andreas->frame)
	__unwind_single_frame (c);
      c->Rip = me.andreas->ret;
      RtlRestoreContext (c, NULL);
    }
  return EXCEPTION_CONTINUE_SEARCH;
}

#endif

/* Main exception handler. */
EXCEPTION_DISPOSITION
exception::handle (EXCEPTION_RECORD *e, exception_list *frame, CONTEXT *in,
		   PDISPATCHER_CONTEXT dispatch)
{
  static int NO_COPY debugging = 0;
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

    case STATUS_STACK_OVERFLOW:
      /* If we encounter a stack overflow, and if the thread has no alternate
         stack, don't even try to call a signal handler.  This is in line with
	 Linux behaviour and also makes a lot of sense on Windows. */
      if (me.altstack.ss_flags)
	global_sigs[SIGSEGV].sa_handler = SIG_DFL;
      /*FALLTHRU*/
    case STATUS_ARRAY_BOUNDS_EXCEEDED:
    case STATUS_IN_PAGE_ERROR:
    case STATUS_NO_MEMORY:
    case STATUS_INVALID_DISPOSITION:
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

  debug_printf ("In cygwin_except_handler exception %y at %p sp %p",
		e->ExceptionCode, in->_GR(ip), in->_GR(sp));
  debug_printf ("In cygwin_except_handler signal %d at %p",
		si.si_signo, in->_GR(ip));

#ifdef __x86_64__
  PUINT_PTR framep = (PUINT_PTR) in->Rbp;
  /* Sometimes, when a stack is screwed up, Rbp tends to be NULL.  In that
     case, base the stacktrace on Rsp.  In most cases, it allows to generate
     useful stack trace. */
  if (!framep)
    framep = (PUINT_PTR) in->Rsp;
#else
  PUINT_PTR framep = (PUINT_PTR) in->_GR(sp);
  for (PUINT_PTR bpend = (PUINT_PTR) __builtin_frame_address (0);
       framep > bpend;
       framep--)
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
      debugging = 1;
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
  if ((NTSTATUS) e->ExceptionCode == STATUS_STACK_OVERFLOW)
    {
      /* If we catched a stack overflow, and if the signal handler didn't exit
	 or longjmp, we're back here and about to continue, supposed to run the
	 offending instruction again.  That works on Linux, but not on Windows.
	 In case of a stack overflow we're not immediately returning to the
	 system exception handler, but to NTDLL::__stkchk.  __stkchk will then
	 terminate the applicaton.  So what we do here is to signal our current
	 process again, but this time with SIG_DFL action.  This creates a
	 stackdump and then exits through our own means. */
      global_sigs[SIGSEGV].sa_handler = SIG_DFL;
      sig_send (NULL, si, &me);
    }
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
  push ((__tlsstack_t) sigdelayed);
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
  this->sig = si.si_signo; /* Should always be last thing set to avoid race */

  if (incyg)
    set_signal_arrived ();

  if (!have_execed)
    proc_subproc (PROC_CLEARWAIT, 1);
  sigproc_printf ("armed signal_arrived %p, signal %d",
		  signal_arrived, si.si_signo);
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
	      sigproc_printf ("controlled interrupt. stackptr %p, stack %p, "
			      "stackptr[-1] %p",
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
		 If the thread is already suspended (which can occur when a
		 program has called SuspendThread on itself) then just queue
		 the signal. */
	      sigproc_printf ("suspending thread, tls %p, _main_tls %p",
			      tls, _main_tls);
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
  sigproc_printf ("signal %d %sdelivered", si.si_signo,
		  interrupted ? "" : "not ");
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
  /* Ignore this if we're not the process group leader since it should be
     handled *by* the process group leader. */
  if (t && (!have_execed || have_execed_cygwin)
      && t->getpgid () == myself->pid &&
      (GetTickCount () - t->last_ctrl_c) >= MIN_CTRL_C_SLOP)
    /* Otherwise we just send a SIGINT to the process group and return TRUE
       (to indicate that we have handled the signal).  At this point, type
       should be a CTRL_C_EVENT or CTRL_BREAK_EVENT. */
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


DWORD WINAPI
dumpstack_overflow_wrapper (PVOID arg)
{
  cygwin_exception *exc = (cygwin_exception *) arg;
  SetThreadName (GetCurrentThreadId (), "__dumpstack_overflow");
  exc->dumpstack ();
  return 0;
}

/* Exit due to a signal.  Should only be called from the signal thread.  */
extern "C" {
static void
signal_exit (int sig, siginfo_t *si, void *)
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
	  {
	    cygwin_exception *exc = (cygwin_exception *) si->si_cyg;
	    if ((NTSTATUS) exc->exception_record ()->ExceptionCode
		== STATUS_STACK_OVERFLOW)
	      {
		/* We're handling a stack overflow so we're running low
		   on stack (surprise!)  The dumpstack method needs lots
		   of stack for buffers.  So what we do here is to run
		   dumpstack in another thread with its own stack. */
		HANDLE thread = CreateThread (&sec_none_nih, 0,
					      dumpstack_overflow_wrapper,
					      exc, 0, NULL);
		if (thread)
		  {
		    WaitForSingleObject (thread, INFINITE);
		    CloseHandle (thread);
		  }
	      }
	    else
	      ((cygwin_exception *) si->si_cyg)->dumpstack ();
	  }
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
	set_signal_arrived (); /* alert sig_handle_tty_stop */
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

  threadlist_t *tl_entry = NULL;
  _cygtls *tls = NULL;

  /* Don't try to send signals if we're just starting up since signal masks
     may not be available.  */
  if (!cygwin_finished_initializing)
    {
      rc = -1;
      goto done;
    }

  sigproc_printf ("signal %d processing", si.si_signo);

  myself->rusage_self.ru_nsignals++;

  if (si.si_signo == SIGCONT)
    {
      tl_entry = cygheap->find_tls (_main_tls);
      _main_tls->handle_SIGCONT ();
      cygheap->unlock_tls (tl_entry);
    }

  /* SIGKILL is special.  It always goes through.  */
  if (si.si_signo == SIGKILL)
    {
      tl_entry = cygheap->find_tls (_main_tls);
      tls = _main_tls;
    }
  else if (ISSTATE (myself, PID_STOPPED))
    {
      rc = -1;		/* Don't send signals when stopped */
      goto done;
    }
  else if (!sigtls)
    {
      tl_entry = cygheap->find_tls (si.si_signo, issig_wait);
      if (tl_entry)
	{
	  tls = tl_entry->thread;
	  sigproc_printf ("using tls %p", tls);
	}
    }
  else
    {
      tl_entry = cygheap->find_tls (sigtls);
      if (tl_entry)
	{
	  tls = tl_entry->thread;
	  if (sigismember (&tls->sigwait_mask, si.si_signo))
	    issig_wait = true;
	  else if (!sigismember (&tls->sigmask, si.si_signo))
	    issig_wait = false;
	  else
	    tls = NULL;
	}
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
  if (si.si_signo == SIGTSTP || si.si_signo == SIGTTIN
      || si.si_signo == SIGTTOU)
    sig_clear (SIGCONT);

  if (handler == (void *) SIG_DFL)
    {
      if (si.si_signo == SIGCHLD || si.si_signo == SIGIO
	  || si.si_signo == SIGCONT || si.si_signo == SIGWINCH
	  || si.si_signo == SIGURG)
	{
	  sigproc_printf ("signal %d default is currently ignore", si.si_signo);
	  goto done;
	}

      if (si.si_signo == SIGTSTP || si.si_signo == SIGTTIN
	  || si.si_signo == SIGTTOU)
	goto stop;

      goto exit_sig;
    }

  if (handler == (void *) SIG_ERR)
    goto exit_sig;

  goto dosig;

stop:
  if (tls != _main_tls)
    {
      cygheap->unlock_tls (tl_entry);
      tl_entry = cygheap->find_tls (_main_tls);
      tls = _main_tls;
    }
  handler = (void *) sig_handle_tty_stop;
  thissig = global_sigs[SIGSTOP];
  goto dosig;

exit_sig:
  handler = (void *) signal_exit;
  thissig.sa_flags |= SA_SIGINFO;
  /* Don't run signal_exit on alternate stack. */
  thissig.sa_flags &= ~SA_ONSTACK;

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
  cygheap->unlock_tls (tl_entry);
  sigproc_printf ("returning %d", rc);
  return rc;

}

static void
altstack_wrapper (int sig, siginfo_t *siginfo, ucontext_t *sigctx,
		  void (*handler) (int, siginfo_t *, void *))
{
  siginfo_t si = *siginfo;
  ULONG guard_size = 0;
  DWORD old_prot = (DWORD) -1;
  PTEB teb = NtCurrentTeb ();
  PVOID old_limit = NULL;

  /* Check if we're just handling a stack overflow.  If so... */
  if (sig == SIGSEGV && si.si_cyg
      && ((cygwin_exception *) si.si_cyg)->exception_record ()->ExceptionCode
	  == (DWORD) STATUS_STACK_OVERFLOW)
    {
      /* ...restore guard pages in original stack as if MSVCRT::_resetstkovlw
	 has been called.

	 Compute size of guard pages.  If SetThreadStackGuarantee returns 0,
	 use the default guard page size. */
      SetThreadStackGuarantee (&guard_size);
      if (!guard_size)
	guard_size = wincap.def_guard_page_size ();
      else
	guard_size += wincap.page_size ();
      old_limit = teb->Tib.StackLimit;
      /* Amazing but true: This VirtualProtect call automatically fixes the
	 value of teb->Tib.StackLimit on some systems.*/
      if (VirtualProtect (teb->Tib.StackLimit, guard_size,
			  PAGE_READWRITE | PAGE_GUARD, &old_prot)
	  && old_limit == teb->Tib.StackLimit)
	teb->Tib.StackLimit = (caddr_t) old_limit + guard_size;
    }
  handler (sig, &si, sigctx);
  if (old_prot != (DWORD) -1)
    {
      /* Typically the handler would exit or at least perform a siglongjmp
	 trying to overcome a SEGV condition.  However, if we return from a
	 segv handler after a stack overflow, we're dead.  While on Linux the
	 process returns to the offending code and thus the handler is called
	 ad infinitum, on Windows the NTDLL::__stkchk function will simply kill
	 the process.  So what we do here is to remove the guard pages again so
	 we can return to exception::handle.  exception::handle will then call
	 sig_send again, this time with SIG_DFL action, so at least we get a
	 stackdump. */
      if (VirtualProtect ((caddr_t) teb->Tib.StackLimit - guard_size,
			  guard_size, old_prot, &old_prot))
	teb->Tib.StackLimit = old_limit;
    }
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
      if (retaddr () == (__tlsstack_t) sigdelayed)
	pop ();

      debug_only_printf ("dealing with signal %d", sig);
      this_sa_flags = sa_flags;

      sigset_t this_oldmask = set_process_mask_delta ();

      /* Save information locally on stack to pass to handler. */
      int thissig = sig;
      siginfo_t thissi = infodata;
      void (*thisfunc) (int, siginfo_t *, void *) = func;

      ucontext_t *thiscontext = NULL;

      /* Only make a context for SA_SIGINFO handlers */
      if (this_sa_flags & SA_SIGINFO)
	{
	  context.uc_link = 0;
	  context.uc_flags = 0;
	  if (thissi.si_cyg)
	    memcpy (&context.uc_mcontext,
		    ((cygwin_exception *) thissi.si_cyg)->context (),
		    sizeof (CONTEXT));
	  else
	    {
	      /* Software-generated signal.  We're fetching the current
		 context, unwind to the caller and in case we're called
		 from sigdelayed, fix rip/eip accordingly. */
	      context.uc_mcontext.ctxflags = CONTEXT_FULL;
	      RtlCaptureContext ((PCONTEXT) &context.uc_mcontext);
	      __unwind_single_frame ((PCONTEXT) &context.uc_mcontext);
	      if (stackptr > stack)
		{
#ifdef __x86_64__
		  context.uc_mcontext.rip = retaddr ();
#else
		  context.uc_mcontext.eip = retaddr ();
#endif
		}
	    }

	  if (this_sa_flags & SA_ONSTACK
	      && !_my_tls.altstack.ss_flags
	      && _my_tls.altstack.ss_sp)
	    {
	      context.uc_stack = _my_tls.altstack;
	      context.uc_stack.ss_flags = SS_ONSTACK;
	    }
	  else
	    {
	      context.uc_stack.ss_sp = NtCurrentTeb ()->Tib.StackBase;
	      context.uc_stack.ss_flags = 0;
	      if (!NtCurrentTeb ()->DeallocationStack)
		context.uc_stack.ss_size
		  = (uintptr_t) NtCurrentTeb ()->Tib.StackLimit
		    - (uintptr_t) NtCurrentTeb ()->Tib.StackBase;
	      else
		context.uc_stack.ss_size
		  = (uintptr_t) NtCurrentTeb ()->DeallocationStack
		    - (uintptr_t) NtCurrentTeb ()->Tib.StackBase;
	    }
	  context.uc_sigmask = context.uc_mcontext.oldmask = this_oldmask;

	  context.uc_mcontext.cr2 = (thissi.si_signo == SIGSEGV
				     || thissi.si_signo == SIGBUS)
				    ? (uintptr_t) thissi.si_addr : 0;

	  thiscontext = &context;
	}

      int this_errno = saved_errno;
      reset_signal_arrived ();
      incyg = false;
      sig = 0;		/* Flag that we can accept another signal */
      unlock ();	/* unlock signal stack */

      /* Alternate signal stack requested for this signal and alternate signal
	 stack set up for this thread? */
      if (this_sa_flags & SA_ONSTACK
	  && !_my_tls.altstack.ss_flags
	  && _my_tls.altstack.ss_sp)
	{
	  /* Yes, use alternate signal stack.

	    NOTE:

	    We DO NOT change the TEB's stack addresses and we DO NOT move the
	    _cygtls area to the alternate stack.  This seems to work fine on
	    32 and 64 bit, but there may be Windows functions not working
	    correctly under these circumstances.  Especially 32 bit exception
	    handling may be broken.

	    On the other hand, if a Windows function crashed and we're handling
	    this here, moving the TEB stack addresses may be fatal.

	    If the current code does not work as expected in the "usual"
	    POSIX circumstances, this problem must be revisited. */

	  /* Compute new stackbase.  We start from the high address, aligned
	     to 16 byte. */
	  uintptr_t new_sp = ((uintptr_t) _my_tls.altstack.ss_sp
			      + _my_tls.altstack.ss_size) & ~0xf;
	  /* In assembler: Save regs on new stack, move to alternate stack,
	     call thisfunc, revert stack regs. */
#ifdef __x86_64__
	  /* Clobbered regs: rcx, rdx, r8, r9, r10, r11, rbp, rsp */
	  __asm__ ("\n\
		   movq  %[NEW_SP], %%rax  # Load alt stack into rax	\n\
		   subq  $0x60, %%rax      # Make room on alt stack	\n\
					   # for clobbered regs and	\n\
					   # required shadow space	\n\
		   movq  %%rcx, 0x20(%%rax)# Save clobbered regs	\n\
		   movq  %%rdx, 0x28(%%rax)				\n\
		   movq  %%r8,  0x30(%%rax)				\n\
		   movq  %%r9,  0x38(%%rax)				\n\
		   movq  %%r10, 0x40(%%rax)				\n\
		   movq  %%r11, 0x48(%%rax)				\n\
		   movq  %%rbp, 0x50(%%rax)				\n\
		   movq  %%rsp, 0x58(%%rax)				\n\
		   movl  %[SIG], %%ecx     # thissig to 1st arg reg	\n\
		   leaq  %[SI], %%rdx      # &thissi to 2nd arg reg	\n\
		   movq  %[CTX], %%r8      # thiscontext to 3rd arg reg	\n\
		   movq  %[FUNC], %%r9     # thisfunc to r9		\n\
		   leaq  %[WRAPPER], %%r10 # wrapper address to r10	\n\
		   movq  %%rax, %%rsp      # Move alt stack into rsp	\n\
		   call  *%%r10            # Call wrapper		\n\
		   movq  %%rsp, %%rax      # Restore clobbered regs	\n\
		   movq  0x58(%%rax), %%rsp				\n\
		   movq  0x50(%%rax), %%rbp				\n\
		   movq  0x48(%%rax), %%r11				\n\
		   movq  0x40(%%rax), %%r10				\n\
		   movq  0x38(%%rax), %%r9				\n\
		   movq  0x30(%%rax), %%r8				\n\
		   movq  0x28(%%rax), %%rdx				\n\
		   movq  0x20(%%rax), %%rcx				\n"
		   : : [NEW_SP]	"o" (new_sp),
		       [SIG]	"o" (thissig),
		       [SI]	"o" (thissi),
		       [CTX]	"o" (thiscontext),
		       [FUNC]	"o" (thisfunc),
		       [WRAPPER] "o" (altstack_wrapper)
		   : "memory");
#else
	  /* Clobbered regs: ecx, edx, ebp, esp */
	  __asm__ ("\n\
		   movl  %[NEW_SP], %%eax  # Load alt stack into eax	\n\
		   subl  $32, %%eax        # Make room on alt stack for	\n\
					   # clobbered regs and args to \n\
					   # signal handler             \n\
		   movl  %%ecx, 16(%%eax)  # Save clobbered regs	\n\
		   movl  %%edx, 20(%%eax)				\n\
		   movl  %%ebp, 24(%%eax)				\n\
		   movl  %%esp, 28(%%eax)				\n\
		   movl  %[SIG], %%ecx     # thissig to 1st arg slot	\n\
		   movl  %%ecx, (%%eax)					\n\
		   leal  %[SI], %%ecx      # &thissi to 2nd arg slot	\n\
		   movl  %%ecx, 4(%%eax)				\n\
		   movl  %[CTX], %%ecx     # thiscontext to 3rd arg slot\n\
		   movl  %%ecx, 8(%%eax)				\n\
		   movl  %[FUNC], %%ecx    # thisfunc to 4th arg slot	\n\
		   movl  %%ecx, 12(%%eax)				\n\
		   leal  %[WRAPPER], %%ecx # thisfunc to ecx		\n\
		   movl  %%eax, %%esp      # Move alt stack into esp	\n\
		   call  *%%ecx            # Call thisfunc		\n\
		   movl	 %%esp, %%eax      # Restore clobbered regs	\n\
		   movl  28(%%eax), %%esp				\n\
		   movl	 24(%%eax), %%ebp				\n\
		   movl	 20(%%eax), %%edx				\n\
		   movl	 16(%%eax), %%eax				\n"
		   : : [NEW_SP]	"o" (new_sp),
		       [SIG]	"o" (thissig),
		       [SI]	"o" (thissi),
		       [CTX]	"o" (thiscontext),
		       [FUNC]	"o" (thisfunc),
		       [WRAPPER] "o" (altstack_wrapper)
		   : "memory");
#endif
	}
      else
	/* No alternate signal stack requested or available, just call
	   signal handler. */
	thisfunc (thissig, &thissi, thiscontext);

      incyg = true;

      set_signal_mask (_my_tls.sigmask, this_oldmask);
      if (this_errno >= 0)
	set_errno (this_errno);
    }

  /* FIXME: Since 2011 this return statement always returned 1 (meaning
     SA_RESTART is effective) if the thread we're running in is not the
     main thread.  We're disabling this check to enable EINTR behaviour
     on system calls not running in the main thread.  It's not quite clear
     if that has undesired side-effects, therefore this comment. */
  return this_sa_flags & SA_RESTART;
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
	  memcpy (&context.uc_mcontext, &c, sizeof (CONTEXT));
	  /* Enough space for 32/64 bit addresses */
	  char sigmsg[2 * sizeof (_CYGWIN_SIGNAL_STRING
				  " ffffffff ffffffffffffffff")];
	  __small_sprintf (sigmsg, _CYGWIN_SIGNAL_STRING " %d %y %p",
			   si.si_signo, thread_id, &context.uc_mcontext);
	  OutputDebugString (sigmsg);
	}
      ResumeThread (th);
    }
}

extern "C" int
setcontext (const ucontext_t *ucp)
{
  PCONTEXT ctx = (PCONTEXT) &ucp->uc_mcontext;
  _my_tls.sigmask = ucp->uc_sigmask;
#ifdef __x86_64__
  /* Apparently a call to NtContinue works on 64 bit as well, but using
     RtlRestoreContext is the blessed way. */
  RtlRestoreContext (ctx, NULL);
#else
  NtContinue (ctx, FALSE);
#endif
  /* If we got here, something was wrong. */
  set_errno (EINVAL);
  return -1;
}

#ifdef __x86_64__

extern "C" int
getcontext (ucontext_t *ucp)
{
  PCONTEXT ctx = (PCONTEXT) &ucp->uc_mcontext;
  ctx->ContextFlags = CONTEXT_FULL;
  RtlCaptureContext (ctx);
  __unwind_single_frame (ctx);
  /* Successful getcontext is supposed to return 0.  If we don't set rax to 0
     here, there's a chance that code like this:

       if (getcontext (&ctx) != 0)

     assumes that getcontext failed after calling setcontext (&ctx).
     Same goes for eax on 32 bit, see assembler implementation below. */
  ucp->uc_mcontext.rax = 0;
  ucp->uc_sigmask = ucp->uc_mcontext.oldmask = _my_tls.sigmask;
  /* Do not touch any other member of ucontext_t. */
  return 0;
}

extern "C" int
swapcontext (ucontext_t *oucp, const ucontext_t *ucp)
{
  PCONTEXT ctx = (PCONTEXT) &oucp->uc_mcontext;
  ctx->ContextFlags = CONTEXT_FULL;
  RtlCaptureContext (ctx);
  __unwind_single_frame (ctx);
  /* See comment in getcontext. */
  oucp->uc_mcontext.rax = 0;
  oucp->uc_sigmask = oucp->uc_mcontext.oldmask = _my_tls.sigmask;
  return setcontext (ucp);
}

/* Trampoline function to set the context to uc_link.  The pointer to the
   address of uc_link is stored in the callee-saved register $rbx.  If uc_link
   is NULL, call exit. */
__asm__ ("				\n\
	.global	__cont_link_context	\n\
	.seh_proc __cont_link_context	\n\
__cont_link_context:			\n\
	.seh_endprologue		\n\
	movq	%rbx, %rsp		\n\
	movq	(%rsp), %rcx		\n\
	# align stack and subtract shadow space	\n\
	andq	$~0xf, %rsp		\n\
	subq	$0x20, %rsp		\n\
	testq	%rcx, %rcx		\n\
	je	1f			\n\
	call	setcontext		\n\
	movq	$0xff, %rcx		\n\
1:					\n\
	call	cygwin_exit		\n\
	nop				\n\
	.seh_endproc			\n\
	");

#else

/* On 32 bit it's crucial to call RtlCaptureContext in a way which makes sure
   the callee-saved registers, especially $ebx, are not changed by the calling
   function.  If so, makecontext/__cont_link_context would be broken.

   Amazing, but true:  While on 64 bit RtlCaptureContext returns the exact
   context of its own caller, as expected, on 32 bit RtlCaptureContext returns
   the context of the callers caller.  So while we have to unwind another frame
   on 64 bit, we can skip this step on 32 bit.

   Both functions are split into the first half in assembler, and the second
   half in C to allow easy access to _my_tls. */

extern "C" int
__getcontext (ucontext_t *ucp)
{
  ucp->uc_mcontext.eax = 0;
  ucp->uc_sigmask = ucp->uc_mcontext.oldmask = _my_tls.sigmask;
  return 0;
}

__asm__ ("				\n\
	.global	_getcontext		\n\
_getcontext:				\n\
	pushl	%ebp			\n\
	movl	%esp, %ebp		\n\
	movl	8(%esp), %eax		\n\
	pushl	%eax			\n\
	call	_RtlCaptureContext@4	\n\
	popl	%ebp			\n\
	jmp	___getcontext		\n\
	nop				\n\
	");

extern "C" int
__swapcontext (ucontext_t *oucp, const ucontext_t *ucp)
{
  oucp->uc_mcontext.eax = 0;
  oucp->uc_sigmask = oucp->uc_mcontext.oldmask = _my_tls.sigmask;
  return setcontext (ucp);
}

__asm__ ("				\n\
	.global	_swapcontext		\n\
_swapcontext:				\n\
	pushl	%ebp			\n\
	movl	%esp, %ebp		\n\
	movl	8(%esp), %eax		\n\
	pushl	%eax			\n\
	call	_RtlCaptureContext@4	\n\
	popl	%ebp			\n\
	jmp	___swapcontext		\n\
	nop				\n\
	");

/* Trampoline function to set the context to uc_link.  The pointer to the
   address of uc_link is stored in the callee-saved register $ebx.  If uc_link
   is NULL, call exit. */
__asm__ ("				\n\
	.global	___cont_link_context	\n\
___cont_link_context:			\n\
	movl	%ebx, %esp		\n\
	movl	(%esp), %eax		\n\
	testl	%eax, %eax		\n\
	je	1f			\n\
	call	_setcontext		\n\
	movl	$0xff, (%esp)		\n\
1:					\n\
	call	_cygwin_exit		\n\
	nop				\n\
	");
#endif

/* makecontext is modelled after GLibc's makecontext.  The stack from uc_stack
   is prepared so that it starts with a pointer to the linked context uc_link,
   followed by the arguments to func, and finally at the bottom the "return"
   address set to __cont_link_context.  In the ucp context, rbx/ebx is set to
   point to the stack address where the pointer to uc_link is stored.  The
   requirement to make this work is that rbx/ebx are callee-saved registers
   per the ABI.  If any function is called which doesn't follow the ABI
   conventions, e.g. assembler code, this method will break.  But that's ok. */
extern "C" void
makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  extern void __cont_link_context (void);
  uintptr_t *sp;
  va_list ap;

  /* Initialize sp to the top of the stack. */
  sp = (uintptr_t *) ((uintptr_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
  /* Subtract slots required for arguments and the pointer to uc_link. */
  sp -= (argc + 1);
  /* Align. */
  sp = (uintptr_t *) ((uintptr_t) sp & ~0xf);
  /* Subtract one slot for setting the return address. */
  --sp;
  /* Set return address to the trampolin function __cont_link_context. */
  sp[0] = (uintptr_t) __cont_link_context;
  /* Fetch arguments and store them on the stack.

     x86_64 only:

     - Store first four args in the AMD64 ABI arg registers.

     - Note that the stack is not short by these four register args.  The
       reason is the shadow space for these regs required by the AMD64 ABI.

     - The definition of makecontext only allows for "int" sized arguments to
       func, 32 bit, likely for historical reasons.  However, the argument
       slots on x86_64 are 64 bit anyway, so we can fetch and store the args
       as 64 bit values, and func can request 64 bit args without violating
       the definition.  This potentially allows porting 32 bit applications
       providing pointer values to func without additional porting effort. */
  va_start (ap, argc);
  for (int i = 0; i < argc; ++i)
#ifdef __x86_64__
    switch (i)
      {
      case 0:
	ucp->uc_mcontext.rcx = va_arg (ap, uintptr_t);
	break;
      case 1:
	ucp->uc_mcontext.rdx = va_arg (ap, uintptr_t);
	break;
      case 2:
	ucp->uc_mcontext.r8 = va_arg (ap, uintptr_t);
	break;
      case 3:
	ucp->uc_mcontext.r9 = va_arg (ap, uintptr_t);
	break;
      default:
	sp[i + 1] = va_arg (ap, uintptr_t);
	break;
      }
#else
    sp[i + 1] = va_arg (ap, uintptr_t);
#endif
  va_end (ap);
  /* Store pointer to uc_link at the top of the stack. */
  sp[argc + 1] = (uintptr_t) ucp->uc_link;
  /* Last but not least set the register in the context at ucp so that a
     subsequent setcontext or swapcontext picks up the right values:
     - Set rip/eip to the target function.
     - Set rsp/esp to the just computed stack pointer value.
     - Set rbx/ebx to the address of the pointer to uc_link. */
#ifdef __x86_64__
  ucp->uc_mcontext.rip = (uint64_t) func;
  ucp->uc_mcontext.rsp = (uint64_t) sp;
  ucp->uc_mcontext.rbx = (uint64_t) (sp + argc + 1);
#else
  ucp->uc_mcontext.eip = (uint32_t) func;
  ucp->uc_mcontext.esp = (uint32_t) sp;
  ucp->uc_mcontext.ebx = (uint32_t) (sp + argc + 1);
#endif
}
