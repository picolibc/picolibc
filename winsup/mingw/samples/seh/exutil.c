
#include <stdlib.h>
#include <stdio.h>
#include <excpt.h>
#include <windows.h>

#include "exutil.h"

void
WalkExceptionHandlers ()
{
	PEXCEPTION_REGISTRATION_RECORD	p;
	int				i;

	__asm__("movl %%fs:0,%%eax;movl %%eax,%0" : "=g" (p) : : "%eax");

	i = 0;
	while (p != (PEXCEPTION_REGISTRATION_RECORD) -1 && p)
	{
		printf ("Registration %d at %08x : ", i, p);
		printf ("Handler = %08x ", p->handler);
		printf ("Next Registration = %08x\n", p->prev);
		p = p->prev;
		i++;
	}
	printf ("End of exception handler list.\n");
	fflush (stdout);
}

void
DumpExceptionRecord (struct _EXCEPTION_RECORD* pExRec)
{
	printf ("Exception: Code = %08x Flags %08x", pExRec->ExceptionCode,
		pExRec->ExceptionFlags);

	if (pExRec->ExceptionFlags)
	{
		printf (" ( ");
		if (pExRec->ExceptionFlags & EH_NONCONTINUABLE)
		{
			printf ("EH_NONCONTINUABLE ");
		}
		if (pExRec->ExceptionFlags & EH_UNWINDING)
		{
			printf ("EH_UNWINDING ");
		}
		if (pExRec->ExceptionFlags & EH_EXIT_UNWIND)
		{
			printf ("EH_EXIT_UNWIND ");
		}
		if (pExRec->ExceptionFlags & EH_STACK_INVALID)
		{
			printf ("EH_STACK_INVALID ");
		}
		if (pExRec->ExceptionFlags & EH_NESTED_CALL)
		{
			printf ("EH_NESTED_CALL ");
		}
		printf (")\n");
	}
	else
	{
		printf ("\n");
	}

	fflush(stdout);
}

