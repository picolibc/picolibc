//
// This is a C++ version of the code in dll.c. NOTE that you need to put
// extern "C" { ... } around DllMain or it will not be called when your
// Dll starts up! (It will get name mangled as a C++ function and the C
// default version in libmingw32.a will get called instead.)
//

#include <windows.h>

#include <iostream>

extern "C" {

BOOL WINAPI
DllMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			cout << "Dll Attached" << endl ;
			break;

		case DLL_PROCESS_DETACH:
			cout << "Dll Detached" << endl ;
			break;

		case DLL_THREAD_ATTACH:
			printf ("DLL Thread Attached.\n");
			break;

		case DLL_THREAD_DETACH:
			printf ("DLL Thread Detached.\n");
			break;
	}
	return TRUE;
}

void
Test ()
{
	printf ("Test Function called!\n");
}

};
