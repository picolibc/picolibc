/*
 * sehsub.c
 *
 * In an attempt to see what might be going on inside CRTDLL, this program
 * walks the exception list after creating a new thread with _beginthread.
 *
 * It turns out that _beginthread DOES install an exception handler, as
 * expected, but this handler is NOT exported by CRTDLL (it is certainly
 * not _except_handler2 or _XcptFilter)... an odd and unpleasant turn of
 * events.
 */

#include <windows.h>
#include <excpt.h>
#include <process.h>

#include "exutil.h"

extern void* __imp__except_handler3;

unsigned
my_thread (void * p)
{
	printf ("In my thread.\n");
	WalkExceptionHandlers();
	return 0;
}

main ()
{
	unsigned long	h;
	unsigned	id;
	printf ("In main.\n");
	WalkExceptionHandlers();

	printf ("Except_handler3 %08x\n", __imp__except_handler3);
	h = _beginthreadex (NULL, 0, my_thread, NULL, 0, &id);

	WaitForSingleObject ((HANDLE) h, INFINITE);
	CloseHandle ((HANDLE) h);
	return;
}

