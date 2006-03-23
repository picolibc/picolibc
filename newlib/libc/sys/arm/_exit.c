#include <_ansi.h>
#include "swi.h"

void _exit _PARAMS ((int));

void
_exit (int n)
{
  /* FIXME: return code is thrown away.  */
  
#ifdef ARM_RDI_MONITOR
  do_AngelSWI (AngelSWI_Reason_ReportException,
	      (void *) ADP_Stopped_ApplicationExit);
#else
  asm ("swi %a0" :: "i" (SWI_Exit));
#endif
  n = n;
}
