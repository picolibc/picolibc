#include <_ansi.h>
#include <signal.h>
#include "swi.h"

int _kill (int, int) __attribute__((__noreturn__));

int
_kill (int pid, int sig)
{
  (void) pid; (void) sig;
#ifdef ARM_RDI_MONITOR
  /* Note: The pid argument is thrown away.  */
  int block[2];
  block[1] = sig;
  int insn;

#if SEMIHOST_V2
  if (_has_ext_exit_extended ())
    {
      insn = AngelSWI_Reason_ReportExceptionExtended;
    }
  else
#endif
    {
      insn = AngelSWI_Reason_ReportException;
    }

  switch (sig)
    {
    case SIGABRT:
      {
	block[0] = ADP_Stopped_RunTimeError;
	break;
      }
    default:
      {
	block[0] = ADP_Stopped_ApplicationExit;
	break;
      }
    }

#if SEMIHOST_V2
if (_has_ext_exit_extended ())
  do_AngelSWI (insn, block);
else
#endif
  do_AngelSWI (insn, (void*)block[0]);

#else
  asm ("swi %a0" :: "i" (SWI_Exit));
#endif

  __builtin_unreachable();
}
