/*
 * sehfix.c
 *
 * A test program involving an exception handler that fixes the exception
 * causing condition.
 *
 * In this code we install an exception handler my_handler and then a piece
 * of inline assembly attempts to write at the address marked in eax, after
 * setting eax to 10. This should produce an exception. The handler then
 * changes the eax register of the exception context to be the address of
 * a static variable and restarts the code. This should allow everything
 * to continue.
 */

#include <windows.h>
#include <excpt.h>

#include "exutil.h"

int	x;

EXCEPTION_DISPOSITION
my_handler (
	struct _EXCEPTION_RECORD* pExceptionRec,
	void* pEstablisherFrame,
	struct _CONTEXT* pContextRecord,
	void* pDispatcherContext
	)
{
	printf ("In my exception handler!\n");
	DumpExceptionRecord (pExceptionRec);
	pContextRecord->Eax = (DWORD) &x;
	return ExceptionContinueExecution;
}

main ()
{
	x = 2;

	printf ("x = %d\n", x);

	WalkExceptionHandlers();

	__try1(my_handler)

	WalkExceptionHandlers();

	/* This assembly code should produce an exception. */
	__asm__("movl $10,%%eax;movl $1,(%%eax);" : : : "%eax");

	__except1

	WalkExceptionHandlers();

	printf ("x = %d\n", x);

	printf ("Finished!\n");
}


