/*
 * This program attempts to load expexe.exe dynamically, get the address of the
 * ExportedFromExe function, and then call it.
 *
 * This example DOES NOT WORK! I don't know exactly what can be done, but
 * it simply seems that LoadLibrary refuses to load executables.
 */

#include <stdio.h>
#include <windows.h>

int (*ExportedFromExe)();

int main()
{
	HINSTANCE	hDll;
	int i, j, k;

	hDll = LoadLibrary ("expexe.exe");
	if (!hDll)
	{
		printf ("Error %d loading exe.\n", GetLastError());
		exit (-1);
	}

	if (!(ExportedFromExe = GetProcAddress (hDll, "ExportedFromExe")))
	{
		printf ("Error %d getting ExportedFromExe function.\n",
			GetLastError());
		exit (-1);
	}
	else
	{
		ExportedFromExe ();
	}

	/* NOTE: Unlike a DLL the exe doesn't have an entry point which
	 *       initializes global objects and adds __do_global_dtors to
	 *       the atexit list. Thus it should be safe(?) to free the
	 *       library. Of course, this also makes it unsafe to use
	 *       executables at all in this manner.
	 */
	FreeLibrary (hDll);

	return 0;
}

