/*
 * This file tests some of the basics of structured exception handling as
 * implemented in excpt.h and the Windows API header files.
 *
 * The program installs two exception handlers, then attempts to write to
 * a pointer to an invalid address. This causes an exception which passes
 * through the exception handlers and on to the default system exception
 * handler. That handler brings up the dialog box all Windows users know
 * and love, and then the program is terminated.
 *
 * You might note that after the initial run up through our exception frames
 * we get a second run up through them with the exception code
 * STATUS_INVALID_DISPOSITION and the code EH_UNWINDING. This seems normal
 * except that the code got changed from the previous STATUS_ACCESS_VIOLATION.
 * I don't understand that bit particularly.
 */

#include <stdio.h>
#include <excpt.h>

#include "exutil.h"

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
	return ExceptionContinueSearch;
}

EXCEPTION_DISPOSITION
my_handler2 (
	struct _EXCEPTION_RECORD* pExceptionRec,
	void* pEstablisherFrame,
	struct _CONTEXT* pContextRecord,
	void* pDispatcherContext
	)
{
	printf ("In top exception handler!\n");
	DumpExceptionRecord (pExceptionRec);
	return ExceptionContinueSearch;
}

main ()
{
	char*	x;

	printf ("my_handler2 = %08x\n", my_handler2);
	printf ("my_handler = %08x\n", my_handler);

	WalkExceptionHandlers();

	__try1(my_handler2)
	x = (char*) 10;

	WalkExceptionHandlers();

	__try1(my_handler)

	WalkExceptionHandlers();

	*x = 1;
	__except1
	__except1
	printf ("Finished!\n");
}

